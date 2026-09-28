from zbtools.includes import declared_names, needed, with_includes

HEADER = """
#ifndef PICKER_H
#define PICKER_H

struct PickerData
{
    short view;
};

extern PickerData pickerData; /* @data 0x4af8ac */
extern short g_4afb14;
void closeScene19();
short fn_43297f();

#endif
"""


def test_declared_names() -> None:
    assert declared_names(HEADER) == {
        "PickerData",
        "pickerData",
        "g_4afb14",
        "closeScene19",
        "fn_43297f",
    }


def test_needed() -> None:
    headers = {"picker": {"closeScene19"}, "roster": {"rosterKey"}, "net": {"netFrame"}}
    source = "void f()\n{\n    closeScene19();\n    rosterKey(1);\n}\n"
    assert needed(source, "game", headers) == ["picker", "roster"]
    assert needed("", "net", headers) == ["net"]  # a module always includes its own


def test_with_includes() -> None:
    source = '#include <stdio.h>\n\n#include "zoombinis.h"\n#include "old.h"\n\nvoid f();\n'
    assert with_includes(source, ["a", "b"]) == (
        '#include <stdio.h>\n\n#include "zoombinis.h"\n'
        '#include "a.h"\n#include "b.h"\n\nvoid f();\n'
    )
    assert with_includes("void f();\n", ["a"]) == "void f();\n"  # no shared header: left alone
