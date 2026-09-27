from zbtools.declarations import Global, globals_in, structs_as_c

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
