from pathlib import Path

import pytest

from zbtools.define_data import (
    Array,
    Definition,
    Global,
    Pointer,
    Scalar,
    Types,
    Unrenderable,
    _braces,
    _definition_start,
    _number,
    _text,
    insert,
    size_of,
)

HEADER = """
typedef void (*Callback)();
typedef Chunk **Block;
struct Point
{
    short x;
    short y;
};
struct Thing
{
    Point where; /* a comment */
    short a, b[2];
    void (*changed)(short value);
    char *name;
};
struct Odd
{
    union {
        short s;
        long l;
    };
};
"""


def test_declarations() -> None:
    types = Types(HEADER)
    assert types.declaration("short primes[5]") == (
        "primes",
        Array(Scalar("short", 2, True), 5),
    )
    assert types.declaration("const char *texts[]") == ("texts", Array(Pointer("char"), None))
    assert types.declaration("short (*grid)[12]") == ("grid", Pointer("short"))
    assert types.declaration("Callback hook") == ("hook", Pointer(""))
    assert types.declaration("Block chunks") == ("chunks", Pointer("Chunk *"))
    assert types.declaration("unsigned long table[2][3]")[1] == Array(
        Array(Scalar("unsigned long", 4, False), 3), 2
    )


def test_struct_layout() -> None:
    types = Types(HEADER)
    thing = types.declaration("Thing thing")[1]
    assert size_of(thing) == 4 + 2 + 4 + 4 + 4  # byte packing
    with pytest.raises(Unrenderable):
        types.declaration("Odd odd")


def test_numbers() -> None:
    short = Scalar("short", 2, True)
    long_ = Scalar("long", 4, True)
    assert _number(11, short) == "11"
    assert _number(-1, short) == "-1"
    assert _number(0x4E79, short) == "0x4e79"
    assert _number(0x53484C50, long_) == "RESOURCE_TYPE('S', 'H', 'L', 'P')"


def test_text() -> None:
    assert _text(b'say "hi"\r\0\0\0') == '"say \\"hi\\"\\r"'
    assert _text(b"ab\0c") is None  # more after the text
    assert _text(b"ab\0c", whole=False) == '"ab"'
    assert _text(b"\x01\x02\0") is None


def test_long_lists_wrap() -> None:
    items = [str(n) for n in range(1000, 1040)]
    wrapped = _braces(items, top=True)
    assert wrapped.startswith("{\n    1000, 1001,")
    assert all(len(line) <= 100 for line in wrapped.splitlines())
    assert _braces(["1", "2"], top=True) == "{1, 2}"


def _definition(name: str, address: int, text: str, initialised: bool = True) -> Definition:
    glob = Global(
        name, address, text.split(" = ", maxsplit=1)[0].rstrip(";"), Scalar("short", 2, True)
    )
    return Definition(glob, Path("x.cpp"), initialised, text)


SOURCE = """#include "zoombinis.h"

short a = 1;
/* About c. */
short c = 3;

/* The first function. */
/* @zoombi32 0x00401000 */
void f()
{
}
"""


def test_insert_between_definitions_by_address() -> None:
    existing = [(0x100, True, "a"), (0x104, True, "c")]
    text = insert(SOURCE, _definition("b", 0x102, "short b = 2;"), existing)
    assert "short a = 1;\nshort b = 2;\n/* About c. */\nshort c = 3;" in text
    text = insert(SOURCE, _definition("d", 0x106, "short d = 4;"), existing)
    assert "short c = 3;\nshort d = 4;\n" in text


def test_insert_before_the_first_function() -> None:
    text = insert(SOURCE, _definition("u", 0x500, "short u;", initialised=False), [])
    assert "short u;\n\n/* The first function. */\n/* @zoombi32" in text


def test_function_pointer_definitions_are_found() -> None:
    text = "void (*hook)(short active) = 0;\nshort (*grid)[12];\n"
    assert _definition_start("hook").search(text)
    assert _definition_start("grid").search(text)
    assert not _definition_start("hook").search("extern void (*hook)(short active);")


def test_text_beyond_ascii_is_escaped_in_octal() -> None:
    # A hex escape would swallow the "d" after it.
    assert _text(b"Br\xd8derbund\0") == '"Br\\330derbund"'
    assert _text(b"\0", whole=False) is None
    assert _text(b"\0", whole=False, empty=True) == '""'
