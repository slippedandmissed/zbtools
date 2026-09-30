"""The original executable: bytes at virtual addresses, relocation sites and
disassembly. Typed wrappers around pefile and capstone, neither of which ships
type information."""

from dataclasses import dataclass
from pathlib import Path

import capstone
import pefile

_IMAGE_REL_BASED_HIGHLOW = 3  # a 32-bit absolute address the loader relocates
_IMAGE_SCN_MEM_EXECUTE = 0x20000000


@dataclass(frozen=True)
class PeResource:
    """A resource in the executable's .rsrc section."""

    type: int | str  # a number (3: ICON, 14: GROUP_ICON) or a name
    name: int | str
    language: int
    data: bytes


@dataclass(frozen=True)
class Instruction:
    address: int
    raw: bytes
    text: str  # e.g. "mov eax, dword ptr [ebp + 8]"


def disassemble(code: bytes, address: int) -> list[Instruction]:
    cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    return [
        Instruction(int(i.address), bytes(i.bytes), f"{i.mnemonic} {i.op_str}".strip())
        for i in cs.disasm(code, address)
    ]


class Executable:
    def __init__(self, path: Path) -> None:
        pe = pefile.PE(str(path))
        self.base = int(pe.OPTIONAL_HEADER.ImageBase)
        self.image = bytes(pe.get_memory_mapped_image())
        self.relocations = frozenset(
            self.base + int(entry.rva)
            for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", [])
            for entry in block.entries
            if int(entry.type) == _IMAGE_REL_BASED_HIGHLOW
        )
        self.sections = {
            section.Name.rstrip(b"\0").decode(): (
                self.base + int(section.VirtualAddress),
                self.base + int(section.VirtualAddress) + int(section.Misc_VirtualSize),
            )
            for section in pe.sections
        }
        # The part of each section stored in the file; the rest (a data
        # section's uninitialised globals) is zero-filled when loaded.
        self.initialised = {
            section.Name.rstrip(b"\0").decode(): (
                self.base + int(section.VirtualAddress),
                self.base
                + int(section.VirtualAddress)
                + min(int(section.SizeOfRawData), int(section.Misc_VirtualSize)),
            )
            for section in pe.sections
        }
        # The imported functions by the address of their slot in the import table.
        self.imports = {
            int(imp.address): imp.name.decode()
            for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", [])
            for imp in entry.imports
            if imp.name is not None
        }
        self.resources: list[PeResource] = []
        root = getattr(pe, "DIRECTORY_ENTRY_RESOURCE", None)
        for kind in root.entries if root is not None else []:
            for name in kind.directory.entries:
                for language in name.directory.entries:
                    struct = language.data.struct
                    data = pe.get_data(int(struct.OffsetToData), int(struct.Size))
                    self.resources.append(
                        PeResource(
                            str(kind.name) if kind.name is not None else int(kind.id),
                            str(name.name) if name.name is not None else int(name.id),
                            int(language.id),
                            bytes(data),
                        )
                    )
        code = next(s for s in pe.sections if int(s.Characteristics) & _IMAGE_SCN_MEM_EXECUTE)
        start = self.base + int(code.VirtualAddress)
        self.code_range = (start, start + int(code.Misc_VirtualSize))

    def pointer(self, address: int) -> int:
        """The 32-bit value stored at an address."""
        return int.from_bytes(self.read(address, 4), "little")

    def read(self, address: int, size: int) -> bytes:
        offset = address - self.base
        if offset < 0 or offset + size > len(self.image):
            raise ValueError(f"{address:#x}+{size} is outside the executable")
        return self.image[offset : offset + size]
