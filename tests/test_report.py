"""The progress report's `--no-embed-binary` mode: nothing of the original's code in it."""

from zbtools.exe import Instruction
from zbtools.report import AsmLine, Names, _original_asm

NAMES = Names({0x401000: "helper"}, {})
CALL = Instruction(0x402004, bytes.fromhex("e8f7ef ffff"), "call 0x401000")


def test_the_original_is_shown_by_default() -> None:
    line = _original_asm(CALL, 0x402000, 0x20, NAMES)
    assert line == AsmLine("0004", "e8 f7 ef ff ff", "call 0x401000", "helper")


def test_without_embedding_only_the_offset_is_kept() -> None:
    line = _original_asm(CALL, 0x402000, 0x20, NAMES, embed=False)
    assert line == AsmLine("0004", "", "", None)


def test_a_missing_instruction_stays_missing() -> None:
    assert _original_asm(None, 0x402000, 0x20, NAMES, embed=False) is None
