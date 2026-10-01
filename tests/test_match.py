from pathlib import Path

from zbtools.match import (
    DEFAULT_RELEASE,
    Checked,
    Marker,
    Outcome,
    Result,
    Sibling,
    Target,
    _local_calls,
    _misplaced_self_references,
    _silent,
    cache_key,
    find_targets,
    load_baseline,
    local_headers,
    outcome,
    parameter_types,
    release_for,
    write_baseline,
)
from zbtools.omf import Fixup


def _call(source: int, destination: int) -> bytes:
    return b"\xe8" + (destination - source - 5).to_bytes(4, "little", signed=True)


def test_local_call_to_the_right_sibling() -> None:
    # Compiled at offset 0x20 of CODE, calling a sibling at offset 0x00; the
    # original (at 0x401000) calls that sibling's address, 0x402000.
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x402000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(),
        siblings=siblings,
    )
    assert found == {2: "@sibling$qv"}


def test_local_call_to_another_function() -> None:
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x403000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(),
        siblings=siblings,
    )
    assert found == {}


def test_calls_with_fixups_are_left_to_the_linker() -> None:
    compiled = b"\x55" + _call(0x21, 0x00)
    original = b"\x55" + _call(0x401001, 0x402000)
    siblings = {("CODE", 0x00): Sibling(0x402000, "@sibling$qv")}
    found = _local_calls(
        compiled,
        original,
        address=0x401000,
        at=("CODE", 0x20),
        masked=frozenset(range(2, 6)),
        siblings=siblings,
    )
    assert found == {}


def test_markers() -> None:
    source = """
/* @zoombi32 0x00401000 */
void exact(long) {}
/* @zoombi32-functional 0x00401020 */
long portable(long *value) { return 0; }
/* @zoombi32 0x00401040 */
void *__cdecl operator new(size_t size, void *where) { return where; }
/* @zoombi32 0x00401060 */
Spec &__cdecl Spec::operator=(const Spec &from) { return *this; }
/* @zoombi32-implicit 0x00401080 Spec::~Spec */
/* @zoombi32-implicit 0x004010a0 <startup 2> */
"""
    assert find_targets(source) == [
        Target("exact", 0x401000, Marker.DECOMPILED, "long"),
        Target("portable", 0x401020, Marker.FUNCTIONAL, "long*"),
        Target("operator new", 0x401040, Marker.DECOMPILED, "size_t,void*"),
        Target("Spec::operator =", 0x401060, Marker.DECOMPILED, "constSpec&"),
        Target("Spec::~Spec", 0x401080),
        Target("<startup 2>", 0x4010A0),
    ]


def _checked(marker: Marker, *, exact: bool | None) -> Checked:
    """A checked function at 0x401000: exact, differing, or not compiled (None)."""
    target = Target("f", 0x401000, marker)
    if exact is None:
        return Checked(Path("f.cpp"), target, None, "failed")
    mismatches = () if exact else (0,)
    result = Result(target, b"\xc3", b"\xc3", frozenset(), frozenset(), mismatches, {})
    return Checked(Path("f.cpp"), target, result)


def test_outcomes() -> None:
    recorded, fresh = {0x401000}, set[int]()
    decompiled, functional = Marker.DECOMPILED, Marker.FUNCTIONAL
    assert outcome(_checked(decompiled, exact=True), recorded) == Outcome.MATCH
    assert outcome(_checked(decompiled, exact=True), fresh) == Outcome.NEW_MATCH
    assert outcome(_checked(decompiled, exact=False), recorded) == Outcome.REGRESSED
    assert outcome(_checked(decompiled, exact=False), fresh) == Outcome.NONMATCHING
    assert outcome(_checked(functional, exact=False), fresh) == Outcome.FUNCTIONAL
    assert outcome(_checked(functional, exact=True), fresh) == Outcome.FUNCTIONAL_EXACT
    assert outcome(_checked(decompiled, exact=None), fresh) == Outcome.ERROR
    assert Outcome.REGRESSED.failure
    assert Outcome.ERROR.failure
    assert not Outcome.NEW_MATCH.failure
    assert Outcome.NEW_MATCH.discrepancy
    assert not Outcome.NONMATCHING.discrepancy


def test_baseline_round_trip(tmp_path: Path) -> None:
    path = tmp_path / "matching.txt"
    assert load_baseline(path) == {}
    write_baseline({0x402000: "second", 0x401000: "first"}, path)
    assert path.read_text().splitlines()[-2:] == ["0x00401000 first", "0x00402000 second"]
    assert load_baseline(path) == {0x401000: "first", 0x402000: "second"}


def test_cache_key_follows_the_source_and_its_headers(tmp_path: Path) -> None:
    header = tmp_path / "zoombinis.h"
    header.write_text('#include "types.h"\nlong g;\n')
    (tmp_path / "types.h").write_text("typedef long LONG;\n")
    source = tmp_path / "config.cpp"
    source.write_text('#include <windows.h>\n#include "zoombinis.h"\nvoid f() {}\n')
    assert local_headers(source) == [(tmp_path / "types.h").resolve(), header.resolve()]

    key = cache_key("4.5", source, "-p -k-")
    assert cache_key("4.5", source, "-p -k-") == key
    assert cache_key("4.52", source, "-p -k-") != key
    assert cache_key("4.5", source, "-p") != key
    (tmp_path / "types.h").write_text("typedef long LONG; /* changed */\n")
    assert cache_key("4.5", source, "-p -k-") != key


def test_parameter_types_drop_names() -> None:
    assert parameter_types("short index") == "short"
    assert parameter_types("const Color &color, unsigned char kind") == "constColor&,unsignedchar"
    assert parameter_types("unsigned short") == "unsignedshort"
    assert parameter_types("void") == ""
    assert parameter_types("") == ""
    assert parameter_types("fileSpec *path, short") == "fileSpec*,short"
    # decomp/zoombinis.h's pointer-sized types are what BCC32 mangles: long
    assert parameter_types("LONG_PTR volume, const char *path") == "long,constchar*"
    assert parameter_types("UINT_PTR, DWORD_PTR data") == "unsignedlong,unsignedlong"


def test_release_by_directive_or_override() -> None:
    assert release_for("int f();") == DEFAULT_RELEASE
    assert release_for("/* @release 5.02 */\nint f();") == "5.02"
    assert release_for("/* @release 5.02 */", override="4.52") == "4.52"


def test_silent_compile_failures_are_told_apart() -> None:
    banner = (
        "Borland C++ 4.5 for Win32 Copyright (c) 1993, 1994 Borland International\nR:\\a.cpp:\n"
    )
    assert _silent(RuntimeError("compiling a.cpp failed:\n" + banner))
    assert not _silent(RuntimeError(banner + "Error R:\\a.cpp 3: Undefined symbol 'x'\n"))


def _table(*entries: int) -> bytes:
    return b"".join(e.to_bytes(4, "little") for e in entries)


def test_jump_table_entries_must_lead_to_the_same_code() -> None:
    # A function at offset 0x100 of CODE (0x40 bytes) and at 0x401000 in the
    # original, whose jump table (at +0x10) lists two cases.
    fixups = [
        Fixup(offset=0x110, size=4, self_relative=False, target="CODE"),
        Fixup(offset=0x114, size=4, self_relative=False, target="CODE"),
    ]
    compiled = bytes(0x10) + _table(0x120, 0x130) + bytes(0x28)
    same = bytes(0x10) + _table(0x401020, 0x401030) + bytes(0x28)
    swapped = bytes(0x10) + _table(0x401030, 0x401020) + bytes(0x28)
    at = ("CODE", 0x100, 0x140)
    assert _misplaced_self_references(compiled, same, fixups, at=at, address=0x401000) == set()
    assert _misplaced_self_references(compiled, swapped, fixups, at=at, address=0x401000) == set(
        range(0x10, 0x18)
    )


def test_references_elsewhere_are_left_to_the_linker() -> None:
    fixups = [
        Fixup(offset=0x110, size=4, self_relative=False, target="DATA"),
        Fixup(offset=0x114, size=4, self_relative=False, target="CODE", displacement=0x200),
    ]
    compiled = bytes(0x10) + _table(0x10, 0x0) + bytes(0x28)
    original = bytes(0x10) + _table(0x4A0000, 0x402000) + bytes(0x28)
    at = ("CODE", 0x100, 0x140)
    assert _misplaced_self_references(compiled, original, fixups, at=at, address=0x401000) == set()
