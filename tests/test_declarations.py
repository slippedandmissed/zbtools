from zbtools.declarations import (
    Global,
    Prototype,
    declared_twice,
    globals_declared_twice,
    globals_in,
    headers,
    prototypes_in,
    structs_as_c,
)

HEADER = """
struct Link
{
    Link *prev;
    Link *next;
};

extern long g_4a7f58;
extern short g_4b83e4[];
extern Link *g_4a00a0;
extern time_t g_4a07b8;
extern long mousePresent; /* @data 0x4aa000 */
/* @data 0x4aa004 */
extern short cursorShown;
extern long unplaced;
extern long g_4a0010; /* @data 0x4a0020 */
extern long g_4a0014;
typedef void (*Callback)();
extern Callback g_4a0018;
extern void (*g_4a001c)(short active); /* told when it changes */
extern const char *g_4a0024;
"""


def test_globals_by_name_and_marker() -> None:
    assert globals_in(HEADER) == [
        Global(0x4A7F58, "g_4a7f58", "long", array=False),
        Global(0x4B83E4, "g_4b83e4", "short", array=True),
        Global(0x4A00A0, "g_4a00a0", "Link *", array=False),
        Global(0x4A07B8, "g_4a07b8", "long", array=False),
        Global(0x4AA000, "mousePresent", "long", array=False),
        Global(0x4AA004, "cursorShown", "short", array=False),
        Global(0x4A0020, "g_4a0010", "long", array=False),
        Global(0x4A0014, "g_4a0014", "long", array=False),
        Global(0x4A0018, "g_4a0018", "void *", array=False),
        Global(0x4A001C, "g_4a001c", "void *", array=False),
        Global(0x4A0024, "g_4a0024", "char *", array=False),
    ]


def test_structs_get_typedefs_and_packing() -> None:
    c = structs_as_c(HEADER)
    assert c.startswith(
        "#pragma pack(push, 1)\n"
        "typedef struct Link Link;\n"
        "typedef void (*Callback)();\n"
        "struct Link\n{"
    )
    assert c.endswith("};\n#pragma pack(pop)\n")


def test_classes_are_declared_not_defined() -> None:
    c = structs_as_c(
        "class basePort;\n"
        "class Palette\n{\npublic:\n    virtual ~Palette();\n};\n"
        "struct Holder\n{\n    basePort *port;\n    Palette *palette;\n};\n"
    )
    assert "typedef struct Holder Holder;\n" in c
    assert "typedef struct basePort basePort;\n" in c
    assert "typedef struct Palette Palette;\n" in c
    assert "virtual" not in c


def test_typedefs_and_anonymous_unions() -> None:
    c = structs_as_c(
        "struct Chunk\n{\n    short magic;\n};\n"
        "typedef Chunk **Block;\n"
        "struct Entry\n{\n    short bits;\n    union {\n        Block block;\n"
        "        short next;\n    };\n};\n"
    )
    assert "typedef Chunk **Block;\n" in c
    assert "    } u1;\n" in c
    assert c.index("typedef Chunk **Block;") < c.index("struct Entry\n{")


PROTOTYPES = """
short addView(unsigned long flags, ViewDraw draw,
              short target);
long newTimer(long data); /* 0x46daca */
class Color
{
public:
    Color(short index);
};
inline short twice(short x)
{
    return x * 2;
}
extern short g_4a0000;
void fn_41d1b1(View *view, short event);
"""


def test_prototypes_at_the_top_level() -> None:
    assert prototypes_in(PROTOTYPES) == [
        Prototype("addView", None),
        Prototype("newTimer", 0x46DACA),
        Prototype("fn_41d1b1", None),
    ]


def test_declared_twice() -> None:
    once = "void f(short a);\n"
    assert declared_twice([once, "void g();\n"]) == []
    # The same function in two headers, or a conflicting copy.
    assert declared_twice([once, once]) == ["f"]
    assert declared_twice([once, "void f(long a); /* 0x401000 */\n"]) == ["f"]
    # Two functions sharing a name, each with its address, are fine.
    assert (
        declared_twice(["void f(short a); /* 0x401000 */\n", "void f(long a); /* 0x402000 */\n"])
        == []
    )


def test_decomp_declares_each_function_once() -> None:
    assert declared_twice([path.read_text() for path in headers()]) == []


def test_globals_declared_twice() -> None:
    assert globals_declared_twice(["extern short a;\n", "extern long b[4]; /* x */\n"]) == []
    assert globals_declared_twice(
        ["extern short a;\n", "extern short a; /* @data 0x4a0000 */\n"]
    ) == ["a"]


def test_decomp_declares_each_global_once() -> None:
    assert globals_declared_twice([path.read_text() for path in headers()]) == []
