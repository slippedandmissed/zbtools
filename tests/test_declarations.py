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
"""


def test_globals_by_name_and_marker() -> None:
    assert globals_in(HEADER) == [
        Global(0x4A7F58, "g_4a7f58", "long", array=False),
        Global(0x4B83E4, "g_4b83e4", "short", array=True),
        Global(0x4A00A0, "g_4a00a0", "Link *", array=False),
        Global(0x4A07B8, "g_4a07b8", "long", array=False),
        Global(0x4AA000, "mousePresent", "long", array=False),
        Global(0x4AA004, "cursorShown", "short", array=False),
    ]


def test_structs_get_typedefs_and_packing() -> None:
    c = structs_as_c(HEADER)
    assert c.startswith("#pragma pack(push, 1)\ntypedef struct Link Link;\nstruct Link\n{")
    assert c.endswith("};\n#pragma pack(pop)\n")
