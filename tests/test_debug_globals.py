"""The debug tools' table of game globals (zbtools.debug_globals)."""

from pathlib import Path

from zbtools import debug_globals


def test_finds_integer_globals_with_dimensions() -> None:
    headers = {
        "a.h": "extern short level; /* @data 0x4a0000 */\n"
        "extern unsigned char flags[12];\n"
        "extern long grid[3][5];\n"
        "extern char cube[2][2][2];\n"  # three dimensions: left out
        "extern Point where;\n"  # not an integer
        "extern char *text;\n"  # a pointer
        "extern short unknown[];\n",  # no length
    }
    found = {g.name: g for g in debug_globals.declared(headers)}
    assert set(found) == {"level", "flags", "grid"}
    assert (found["level"].size, found["level"].signed, found["level"].dimensions) == (2, True, ())
    assert (found["flags"].size, found["flags"].signed, found["flags"].dimensions) == (
        1,
        False,
        (12,),
    )
    assert found["grid"].dimensions == (3, 5)


def test_definitions_are_what_the_sources_define() -> None:
    source = (
        "short level = 3;\n"
        "unsigned char flags[12];\n"
        "extern short elsewhere;\n"
        "static short hidden;\n"
        "short counted(short x)\n{\n    return x;\n}\n"
        "  short indented = 0;\n"
    )
    assert debug_globals.defined([source]) == {"level", "flags"}


def test_the_table_leaves_out_what_no_source_defines(tmp_path: Path) -> None:
    (tmp_path / "a.h").write_text("extern short level;\nextern short missing;\n")
    (tmp_path / "a.cpp").write_text('#include "a.h"\nshort level = 1;\n')
    text = debug_globals.generate(tmp_path)
    assert '{"level", (void *)&level, 2, 1, 0, 0, 0},' in text
    assert "missing" not in text
    assert '#include "a.h"' in text


def test_write_says_whether_it_changed_anything(tmp_path: Path) -> None:
    (tmp_path / "a.h").write_text("extern short level;\n")
    (tmp_path / "a.cpp").write_text("short level;\n")
    target = tmp_path / "out" / "debug_globals.inc"
    assert debug_globals.write(target, tmp_path)
    assert not debug_globals.write(target, tmp_path)
