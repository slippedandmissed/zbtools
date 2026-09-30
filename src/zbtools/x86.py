"""A 32-bit x86 emulator (Unicorn) behind typed methods. Unicorn's own are
mostly unannotated, so this is the one module where mypy may call untyped
functions (`pyproject.toml`); its users stay fully typed."""

from collections.abc import Callable
from enum import Enum

from unicorn.unicorn import Uc
from unicorn.unicorn_const import UC_ARCH_X86, UC_HOOK_CODE, UC_MODE_32
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_EIP, UC_X86_REG_ESP


class Register(Enum):
    EAX = UC_X86_REG_EAX
    EBX = UC_X86_REG_EBX
    EIP = UC_X86_REG_EIP
    ESP = UC_X86_REG_ESP


class Emulator:
    def __init__(self) -> None:
        self._uc = Uc(UC_ARCH_X86, UC_MODE_32)

    def map(self, address: int, size: int) -> None:
        """Zeroed memory (the address and size a multiple of 4 KB)."""
        self._uc.mem_map(address, size)

    def write(self, address: int, data: bytes) -> None:
        self._uc.mem_write(address, data)

    def read(self, address: int, size: int) -> bytes:
        return bytes(self._uc.mem_read(address, size))

    def read32(self, address: int) -> int:
        return int.from_bytes(self.read(address, 4), "little")

    def write32(self, address: int, value: int) -> None:
        self.write(address, (value & 0xFFFFFFFF).to_bytes(4, "little"))

    def register(self, register: Register) -> int:
        return int(self._uc.reg_read(register.value))

    def set_register(self, register: Register, value: int) -> None:
        self._uc.reg_write(register.value, value & 0xFFFFFFFF)

    def on_execute(self, callback: Callable[[int], None], begin: int, end: int) -> None:
        """Calls `callback` with the address of each instruction run from `begin` to `end`."""

        def hook(uc: Uc, address: int, size: int, user_data: object) -> None:
            callback(address)

        self._uc.hook_add(UC_HOOK_CODE, hook, begin=begin, end=end)

    def run(self, start: int, stop: int, limit: int) -> None:
        """Runs from `start` until `stop` is reached or `limit` instructions have."""
        self._uc.emu_start(start, stop, count=limit)
