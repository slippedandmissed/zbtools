"""Sort the near-misses by what differs: `uv run near-misses`.

A decompiled function that doesn't match (decomp/matching.txt) differs from
the original somewhere; whether that matters depends on how. The rebuilt
code and the original's are compared by what they compute, not
instruction by instruction (where the compiler orders code differently, an
alignment pairs unrelated instructions):

- **computations**: each version's operations (arithmetic, comparisons,
  branch conditions, calls, returns) and constants, counted, leaving out
  moving values about (between registers, the stack and memory) and
  writing equivalent forms alike (`inc` is `add 1`; a comparison is a
  comparison, whatever with; a condition, its inverse and its mirror are
  one). The same counts mean the same computations, differently allocated;
- **frame**: the stack slots whose address the code takes (`lea r, [ebp -
  n]`), which is how a local is handed to something that writes it: laid
  out differently, a local that's too small can be overrun (as
  `Drive::setLocked`'s IOCTL block was, by one byte, onto the saved frame
  pointer).

The verdicts: **allocation only** (the same computations and address-taken
locals); **frame layout** (the same computations, address-taken locals
elsewhere: check the locals' sizes); **needs a look** (different
computations: listed, with each version's extra operations and constants).
Each function is shown with its note in the source (`Not exact: ...`).
"""

import itertools
import re
from collections import Counter
from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from zbtools import inventory, match, paths
from zbtools.exe import Executable, Instruction, disassemble


class Verdict(StrEnum):
    ALLOCATION = "allocation only"
    FRAME = "frame layout (check the sizes of the locals whose address is taken)"
    LOOK = "needs a look"


_MOVES = {"mov", "movsx", "movzx", "xchg", "push", "pop", "lea", "nop", "cdq", "cwde", "cbw"}
# A condition, its inverse (a compiler may test either and swap the branches)
# and its mirror (`cmp a, b; jl` is `cmp b, a; jg`) are one.
_CONDITIONS = {
    "je": "eq", "jne": "eq", "jz": "eq", "jnz": "eq", "jl": "signed", "jge": "signed",
    "jg": "signed", "jle": "signed", "jb": "unsigned", "jae": "unsigned", "ja": "unsigned",
    "jbe": "unsigned", "js": "sign", "jns": "sign", "jo": "o", "jno": "o", "jp": "p", "jnp": "p",
}  # fmt: skip
_REGISTER = r"\b(?:e?[abcd]x|e?[sd]i|e?[sb]p|[abcd][hl])\b"
_NUMBER = re.compile(r"-?\b(?:0x[0-9a-f]+|\d+)\b")
_ADDRESS_TAKEN = re.compile(r"^lea \w+, \[ebp ([-+]) (0x[0-9a-f]+|\d+)\]$")
# Bytes of a switch table read as instructions: opcodes code never has here.
_DATA = {
    "in", "out", "insd", "outsd", "iretd", "retf", "into", "hlt", "arpl", "bound", "das", "aaa",
    "aas", "daa", "sahf", "lahf", "rcl", "rcr", "scasd", "loope", "loopne", "jecxz", "les", "lds",
}  # fmt: skip


# Instructions that aren't computations: moving values about, and how loops
# and exits are laid out.
_NOT_COMPUTING = _MOVES | {"jmp", "ret", "leave"}
# Written alike: a comparison is a comparison, whatever with (the branch tells).
_ALIKE = {"test": "compare", "cmp": "compare", "inc": "add 1", "dec": "sub 1", "call": "call"}


def _operation(ins: Instruction) -> str | None:
    """An instruction's computation, written so equivalent forms are alike;
    None for moving values about, the frame's bookkeeping or zeroing a
    register."""
    mnemonic, _, operands = ins.text.partition(" ")
    parts = [p.strip() for p in operands.split(",")]
    zeroing = mnemonic == "xor" and len(parts) == 2 and parts[0] == parts[1]
    if mnemonic in _NOT_COMPUTING or zeroing or parts[0] in ("esp", "ebp"):
        return None
    if mnemonic in _CONDITIONS:
        return f"branch {_CONDITIONS[mnemonic]}"
    if mnemonic in ("add", "sub") and len(parts) == 2 and _NUMBER.fullmatch(parts[1]):
        value = int(parts[1], 0)
        if value < 0:  # add -2 is sub 2
            mnemonic, value = {"add": "sub", "sub": "add"}[mnemonic], -value
        return f"{mnemonic} {value}"
    return _ALIKE.get(mnemonic, mnemonic)


@dataclass(frozen=True)
class Computations:
    operations: Counter[str]
    constants: Counter[int]
    address_taken: Counter[int]  # frame offsets whose address is taken


def _tables(pointers: dict[int, int], start: int, end: int) -> set[int]:
    """The bytes of a function's switch tables: runs of two or more pointers
    into the function (offset -> where it points)."""
    inside = sorted(o for o, target in pointers.items() if start <= target < end)
    found: set[int] = set()
    for offset in inside:
        if offset + 4 in pointers or offset - 4 in pointers:
            found.update(range(offset, offset + 4))
    return found


def _instructions(code: bytes, address: int, skip: set[int]) -> list[Instruction]:
    """A function's instructions, disassembling around the bytes to skip."""
    found: list[Instruction] = []
    offset = 0
    while offset < len(code):
        if offset in skip:
            offset += 1
            continue
        end = offset
        while end < len(code) and end not in skip:
            end += 1
        found += disassemble(code[offset:end], address + offset)
        offset = end
    return found


def computations(
    code: bytes, address: int, masked: frozenset[int], tables: set[int]
) -> Computations:
    """What a function computes: its operations, its constants (not addresses
    the linker fills in, branch targets or stack offsets) and the stack
    slots whose address it takes."""
    operations: Counter[str] = Counter()
    constants: Counter[int] = Counter()
    address_taken: Counter[int] = Counter()
    for ins in _instructions(code, address, tables):
        mnemonic = ins.text.partition(" ")[0]
        if ins.raw[0] in (0x00, 0x01) or mnemonic in _DATA:
            continue
        taken = _ADDRESS_TAKEN.match(ins.text)
        if taken:
            address_taken[int(taken.group(2), 0) * (-1 if taken.group(1) == "-" else 1)] += 1
        relocated = any(ins.address - address + k in masked for k in range(len(ins.raw)))
        operation = _operation(ins)
        if operation is not None:
            # an address the linker fills in isn't a constant, even added
            added = operation.startswith(("add ", "sub ")) and mnemonic not in ("inc", "dec")
            operations[mnemonic if relocated and added else operation] += 1
        is_branch = mnemonic in _CONDITIONS or mnemonic in ("jmp", "call")
        bookkeeping = ins.text.partition(" ")[2].split(",")[0].strip() in ("esp", "ebp")
        if relocated or is_branch or bookkeeping or mnemonic == "ret":
            continue
        # Constants: immediates and displacements, not stack offsets or scales.
        text = re.sub(r"\[e[sb]p(?: [-+] (?:0x[0-9a-f]+|\d+))?\]", "", ins.text.partition(" ")[2])
        text = re.sub(r"\[esp [-+] [^\]]*\]", "", text)
        text = re.sub(r"\*\d", "", text)
        text = re.sub(_REGISTER, "", text)
        for number in _NUMBER.findall(text):
            value = abs(int(number, 0))
            if value > 1:  # 0 and 1 come and go with the form (inc, xor)
                constants[value] += 1
    return Computations(operations, constants, address_taken)


@dataclass(frozen=True)
class NearMiss:
    name: str
    address: int
    source: Path
    note: str | None  # the source's "Not exact: ..."
    ours: Computations
    theirs: Computations

    def extra(self) -> tuple[list[str], list[str]]:
        """What only the rebuilt code computes, and what only the original's."""

        def only(a: Computations, b: Computations) -> list[str]:
            found = [f"{op} x{n}" for op, n in (a.operations - b.operations).items()]
            found += [f"constant {v:#x} x{n}" for v, n in (a.constants - b.constants).items()]
            return found

        return only(self.ours, self.theirs), only(self.theirs, self.ours)

    @property
    def verdict(self) -> Verdict:
        ours, theirs = self.extra()
        if ours or theirs:
            return Verdict.LOOK
        if self.ours.address_taken != self.theirs.address_taken:
            return Verdict.FRAME
        return Verdict.ALLOCATION


_NOTE = re.compile(r"Not exact:\s*(.*?)(?:\*/|$)", re.DOTALL)


def note(source: str, address: int) -> str | None:
    """The `Not exact: ...` note in the comment above a function's marker."""
    for marker in match.marker_positions(source):
        if marker.address != address:
            continue
        comments = source[: marker.start].rstrip()
        start = comments.rfind("/*")
        if start < 0 or not comments.endswith("*/"):
            return None
        found = _NOTE.search(comments[start:])
        return " ".join(found.group(1).replace("*", " ").split()) if found else None
    return None


def near_misses(sources: list[Path]) -> list[NearMiss]:
    exe = match.game_executable()
    baseline = match.load_baseline()
    # A function's extent in the original: Ghidra's size stops short at switch
    # tables, and the next function can be further on than the end (past
    # code it doesn't know), so the longer of Ghidra's and ours, up to the next.
    functions = inventory.load(exe)
    starts = sorted({f.address for f in functions})
    gaps = {a: b - a for a, b in itertools.pairwise(starts)}
    ghidra_sizes = {f.address: f.size for f in functions}
    found = []
    for checked in match.check(sources, exe):
        result = checked.result
        if result is None or result.matches or checked.target.address in baseline:
            continue
        if checked.target.marker == match.Marker.FUNCTIONAL:
            continue
        address = checked.target.address
        size = _extent(exe, address, ghidra_sizes.get(address, 0), len(result.compiled), gaps)
        original = exe.read(address, size)
        relocated = frozenset(
            k for k in range(size) if any(address + k - j in exe.relocations for j in range(4))
        )
        theirs = {
            k: exe.pointer(address + k) for k in range(size - 3) if address + k in exe.relocations
        }
        # Ours: the fixups into its own segment hold the offset they point to.
        ours = {
            k: address + int.from_bytes(result.compiled[k : k + 4], "little") - _start(checked)
            for k, target in result.references.items()
            if target.startswith("_TEXT") and k + 4 <= len(result.compiled)
        }
        found.append(
            NearMiss(
                checked.target.name,
                address,
                checked.source,
                note(checked.source.read_text(), address),
                computations(
                    result.compiled,
                    address,
                    result.masked,
                    _tables(ours, address, address + len(result.compiled)),
                ),
                computations(
                    original, address, relocated, _tables(theirs, address, address + size)
                ),
            )
        )
    return sorted(found, key=lambda n: n.address)


def _extent(exe: Executable, address: int, ghidra: int, ours: int, gaps: dict[int, int]) -> int:
    """How long a function is in the original: to its epilogue's `ret` (BCC32
    ends a function with one epilogue), the first at or after about the
    longer of Ghidra's size (which stops short at switch tables) and ours;
    else to the next function."""
    at_least = max(ghidra, ours)
    gap = gaps.get(address, at_least)
    for ins in disassemble(exe.read(address, gap), address):
        end = ins.address - address + len(ins.raw)
        if ins.text.startswith("ret") and end >= at_least - 16:
            return end
    return min(at_least, gap)


def _start(checked: match.Checked) -> int:
    """Where a function starts in its object's segment."""
    compiled = match.compile_sources([checked.source])[checked.source]
    assert not isinstance(compiled, str)
    return match.locate(compiled.obj, checked.target)[1]


app = typer.Typer(add_completion=False)


@app.command(help=__doc__)
def main(
    files: Annotated[
        list[Path] | None,
        typer.Argument(help="Source files to check (default: all of decomp/)", show_default=False),
    ] = None,
) -> None:
    match.require_toolchain()
    found = near_misses([f.resolve() for f in files] if files else match.decomp_sources())
    for verdict in Verdict:
        group = [n for n in found if n.verdict == verdict]
        print(f"{verdict}: {len(group)}")
        for n in group:
            where = n.source.relative_to(paths.REPO_ROOT)
            print(f"  {n.name} @ {n.address:#x} ({where})")
            if verdict == Verdict.ALLOCATION:
                continue
            if n.note:
                print(f"      note: {n.note}")
            ours, theirs = n.extra()
            if ours:
                print(f"      only ours:     {', '.join(ours)}")
            if theirs:
                print(f"      only original: {', '.join(theirs)}")
            if verdict == Verdict.FRAME:
                print(
                    f"      address taken: ours {sorted(n.ours.address_taken)}, "
                    f"original {sorted(n.theirs.address_taken)}"
                )
