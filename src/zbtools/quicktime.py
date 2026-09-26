"""QuickTime for Windows' 32-bit glue, which the game links from Apple's SDK.

The glue loads QTIM32.DLL (QuickTime) and CMGR32.DLL (its component manager)
on demand and calls into each through a single `_EntryPoint` dispatcher. Every
API function is a tiny assembly stub that puts a selector in `bx` and calls the
dispatcher through a pointer (CMGR stubs also set `ax` = 1):

    push ebx / mov bx, <selector> / [mov ax, 1] / call dword ptr [<dispatcher>] / pop ebx / ret

Until the DLL is loaded the pointer holds a fallback that calls QTInitialize
and retries. The DLLs export nothing that maps selectors to API names (and the
Win16 build imports QuickTime by ordinal), so stubs are named by selector
(`qtim_39`) until the game's use of them shows which API they are. See
docs/findings.md.
"""

from dataclasses import dataclass

from zbtools.exe import Executable, disassemble

# The glue's code, from the CMGR fallback to QTTerminate, and its dispatchers.
GLUE_START, GLUE_END = 0x46CCA0, 0x46D754
_DISPATCHERS = {0x4A7F84: "qtim", 0x4A7F90: "cmgr"}
# The glue's own functions. QTInitialize and QTTerminate are the SDK's API
# names (they take and do what those do); the others are descriptive.
_FUNCTIONS = {
    0x46CCA0: "cmgrFallback",
    0x46CE80: "qtimFallback",
    0x46D3DC: "QTInitialize",
    0x46D454: "loadQtim32",
    0x46D4CB: "loadCmgr32",
    0x46D571: "qtim32FileVersion",
    0x46D693: "unloadQtim32",
    0x46D6FD: "unloadCmgr32",
    0x46D745: "QTTerminate",
}


@dataclass(frozen=True)
class GlueFunction:
    address: int
    name: str


def in_glue(address: int) -> bool:
    return GLUE_START <= address < GLUE_END


def functions(exe: Executable) -> list[GlueFunction]:
    """The glue's named functions and every dispatch stub, by address."""
    found = [GlueFunction(a, n) for a, n in _FUNCTIONS.items()]
    code = disassemble(exe.read(GLUE_START, GLUE_END - GLUE_START), GLUE_START)
    for i, ins in enumerate(code):
        if ins.text != "push ebx" or i + 2 >= len(code):
            continue
        selector, *rest = code[i + 1 : i + 4]
        if not selector.text.startswith("mov bx, "):
            continue
        call = next((r for r in rest if r.text.startswith("call dword ptr [0x")), None)
        if call is None:
            continue
        library = _DISPATCHERS.get(int(call.text.split("[")[1].rstrip("]"), 16))
        if library is not None:
            number = int(selector.text.removeprefix("mov bx, "), 0)
            found.append(GlueFunction(ins.address, f"{library}_{number:02x}"))
    return sorted(found, key=lambda f: f.address)
