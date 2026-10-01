"""The toolchain check's lookup of the files a Borland tool wrote."""

from pathlib import Path

from zbtools.toolchain import _output_file


def test_an_output_is_found_whatever_the_case_of_its_name(tmp_path: Path) -> None:
    # BCC32 names its output after the source file's case, so on a case-sensitive file system
    # (Linux) `hello.c` gives `hello.exe`, and the check once looked for HELLO.EXE.
    (tmp_path / "hello.exe").write_bytes(b"MZ")
    (tmp_path / "other.obj").write_bytes(b"")
    assert _output_file(tmp_path, "HELLO.EXE") == tmp_path / "hello.exe"
    assert _output_file(tmp_path, "hello.exe") == tmp_path / "hello.exe"
    assert _output_file(tmp_path, "hello.obj") is None
