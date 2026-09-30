/*
 * QuickTime for Windows' SDK glue (0x46cca0-0x46d754 in the original, see
 * src/zbtools/quicktime.py), which the game links from Apple's SDK and we
 * don't have: loads QTIM32.DLL and CMGR32.DLL when QTInitialize is called and
 * forwards each API call to the one dispatcher a DLL exports, `_EntryPoint`.
 *
 * A call is a stub that puts its selector in bx (the CMGR32 ones also set
 * ax to 1) and calls the dispatcher with the caller's arguments still on the
 * stack, so it can't be written in C: each stub is raw bytes (__emit__), and
 * is exactly
 *
 *     push ebx / mov bx, selector / [mov ax, 1] / call [entry] / pop ebx / ret
 *
 * after the frame BCC adds to a function with arguments, which its first byte
 * undoes (pop ebp). The loader
 * is the SDK's as the original has it (the same library is in the codec,
 * qb32.qtc, which Ghidra decompiles). Until a DLL is loaded its entry is a
 * function that says QuickTime isn't there; the original's, instead,
 * initialises QuickTime and retries, which the game never needs, as it
 * calls QTInitialize first.
 *
 * Only the calls the game makes are here (and the two the loader makes).
 */

#include "../decomp/zoombinis.h"

#pragma warn -rvl

enum
{
    notInitialised = 0x8001
};

typedef long (__cdecl *Entry)();

static long __cdecl qtimUnavailable()
{
    return notInitialised;
}

static long __cdecl cmgrUnavailable()
{
    return 0x80008001L;
}

static Entry qtimEntry = qtimUnavailable;
static HMODULE qtimModule = 0;
static long qtimUsers = 0;
static Entry cmgrEntry = cmgrUnavailable;
static HMODULE cmgrModule = 0;
static long cmgrUsers = 0;

/* The stubs. `entry` is the address of the dispatcher's variable. */
#define QTIM_STUB(selector) \
    __emit__((char)0x5D, (char)0x53, (char)0x66, (char)0xBB, (char)(selector), (char)0, \
             (char)0xFF, (char)0x15, &qtimEntry, (char)0x5B, (char)0xC3)
/* (A function with no arguments gets no frame, so its stub doesn't undo one.) */
#define QTIM_STUB_NO_ARGUMENTS(selector) \
    __emit__((char)0x53, (char)0x66, (char)0xBB, (char)(selector), (char)0, \
             (char)0xFF, (char)0x15, &qtimEntry, (char)0x5B, (char)0xC3)
#define CMGR_STUB(selector) \
    __emit__((char)0x5D, (char)0x53, (char)0x66, (char)0xBB, (char)(selector), (char)0, \
             (char)0x66, (char)0xB8, (char)1, (char)0, (char)0xFF, (char)0x15, &cmgrEntry, \
             (char)0x5B, (char)0xC3)

/* cmgr_0b: MCIsPlayerEvent. */
long __cdecl cmgr_0b(long, HWND, UINT, WPARAM, LPARAM)
{
    CMGR_STUB(0x0B);
}

long __cdecl cmgr_00(long, HWND, long)
{
    CMGR_STUB(0x00);
}

long __cdecl cmgr_01(long, long, long)
{
    CMGR_STUB(0x01);
}

long __cdecl cmgr_05(long, long *)
{
    CMGR_STUB(0x05);
}

long __cdecl cmgr_09(long)
{
    CMGR_STUB(0x09);
}

long __cdecl cmgr_0d(long, long, HWND, POINT)
{
    CMGR_STUB(0x0D);
}

long __cdecl cmgr_0e(long, RECT *, long, long)
{
    CMGR_STUB(0x0E);
}

long __cdecl qtim_02(long)
{
    QTIM_STUB(0x02);
}

long __cdecl qtim_07(long)
{
    QTIM_STUB(0x07);
}

/* EnterMovies. */
long qtim_0b()
{
    QTIM_STUB_NO_ARGUMENTS(0x0B);
}

/* ExitMovies. */
long __cdecl qtim_0c()
{
    QTIM_STUB_NO_ARGUMENTS(0x0C);
}

/* GetMovieBox. */
long __cdecl qtim_0f(long, RECT *)
{
    QTIM_STUB(0x0F);
}

long __cdecl qtim_2a(long *, long, long *, long, long, long)
{
    QTIM_STUB(0x2A);
}

/* OpenMovieFile. */
long __cdecl qtim_2c(const char *, long *, long)
{
    QTIM_STUB(0x2C);
}

long __cdecl qtim_2f(long, long, long)
{
    QTIM_STUB(0x2F);
}

long __cdecl qtim_31(long, long)
{
    QTIM_STUB(0x31);
}

long __cdecl qtim_37(long)
{
    QTIM_STUB(0x37);
}

long __cdecl qtim_38(long, RECT *, long, HWND)
{
    QTIM_STUB(0x38);
}

/* Called once, when QuickTime is first loaded, and once when it's unloaded
   (the SDK's initialisation and termination of the library). */
static long __cdecl qtim_39()
{
    QTIM_STUB_NO_ARGUMENTS(0x39);
}

static long __cdecl qtim_3a()
{
    QTIM_STUB_NO_ARGUMENTS(0x3A);
}

/* GetMoviesError. */
long __cdecl qtim_5e()
{
    QTIM_STUB_NO_ARGUMENTS(0x5E);
}

long __cdecl qtim_62(long, long)
{
    QTIM_STUB(0x62);
}

/* The loader */

enum
{
    loaded = 0,
    dllMissing = 1,
    noEntryPoint = 2,
    quickTimeVersionTooOld = 0x2300 /* (unused) */
};

static long loadQtim32()
{
    if (!qtimModule) {
        qtimModule = LoadLibrary("QTIM32.DLL");
        if (!qtimModule) {
            qtimEntry = qtimUnavailable;
            return dllMissing;
        }
        qtimEntry = (Entry)GetProcAddress(qtimModule, "_EntryPoint");
        if (!qtimEntry) {
            qtimEntry = qtimUnavailable;
            qtimModule = 0;
            return noEntryPoint;
        }
        /* (the original doesn't free the library here either) */
    }
    qtimUsers++;
    return loaded;
}

static long loadCmgr32()
{
    typedef long (__cdecl *Initialize)();
    Initialize initialize;

    if (!cmgrModule) {
        cmgrModule = LoadLibrary("CMGR32.DLL");
        if (!cmgrModule) {
            cmgrEntry = cmgrUnavailable;
            cmgrModule = 0;
            return dllMissing;
        }
        cmgrEntry = (Entry)GetProcAddress(cmgrModule, "_EntryPoint");
        initialize = (Initialize)GetProcAddress(cmgrModule, "_CMgrInitialize");
        if (!cmgrEntry || !initialize || initialize()) {
            cmgrEntry = cmgrUnavailable;
            FreeLibrary(cmgrModule);
            cmgrModule = 0;
            return noEntryPoint;
        }
    }
    cmgrUsers++;
    return loaded;
}

/* QTIM32.DLL's version, from its file's version resource, as the SDK packs
   it: major.minor.build.patch as hex digits (0x0230 0000 for 2.3.0.0, say). */
static long qtim32FileVersion()
{
    typedef DWORD (__stdcall *GetSize)(const char *, DWORD *);
    typedef BOOL (__stdcall *GetInfo)(const char *, DWORD, DWORD, void *);
    typedef BOOL (__stdcall *Query)(const void *, const char *, void **, UINT *);
    char path[260];
    char block[2048];
    DWORD handle;
    DWORD size;
    void *fixed;
    UINT length;
    long version = 0;
    HMODULE library = LoadLibrary("version.dll");
    GetSize getSize;
    GetInfo getInfo;
    Query query;
    DWORD *numbers;

    if (!library)
        return 0;
    getSize = (GetSize)GetProcAddress(library, "GetFileVersionInfoSizeA");
    getInfo = (GetInfo)GetProcAddress(library, "GetFileVersionInfoA");
    query = (Query)GetProcAddress(library, "VerQueryValueA");
    if (GetModuleFileName(qtimModule, path, sizeof path) && getSize && getInfo && query) {
        size = getSize(path, &handle);
        if (size && size <= sizeof block && getInfo(path, handle, size, block)
            && query(block, "\\", &fixed, &length)) {
            numbers = (DWORD *)fixed; /* a VS_FIXEDFILEINFO: the file version is [4] and [5] */
            version = (long)((((numbers[4] >> 16) * 16 + (numbers[5] >> 16)
                               + (numbers[4] & 0xFFFF) * 10) * 256)
                             + (numbers[5] & 0xFFFF));
        }
    }
    FreeLibrary(library);
    return version;
}

static void unloadQtim32()
{
    OSVERSIONINFO system;

    if (qtimUsers > 0 && --qtimUsers == 0) {
        qtim_3a();
        system.dwOSVersionInfoSize = sizeof system;
        GetVersionEx(&system);
        if (system.dwPlatformId == VER_PLATFORM_WIN32_NT)
            FreeLibrary(qtimModule);
        else
            OutputDebugString("Library not freed ... call microsoft\n");
        qtimModule = 0;
        qtimEntry = qtimUnavailable;
    }
}

static void unloadCmgr32()
{
    typedef void (__cdecl *Terminate)();
    Terminate terminate;

    if (cmgrUsers > 0 && --cmgrUsers == 0) {
        terminate = (Terminate)GetProcAddress(cmgrModule, "_CMgrTerminate");
        if (terminate)
            terminate();
        FreeLibrary(cmgrModule);
        cmgrModule = 0;
        cmgrEntry = cmgrUnavailable;
    }
}

/* Loads QuickTime (and its component manager): 0 and its version, or why not
   (1: a DLL isn't there, 2: one isn't what it should be). */
long __cdecl QTInitialize(long *version)
{
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    long result = loadQtim32();

    if (result == loaded) {
        result = loadCmgr32();
        if (result == loaded) {
            if (version)
                *version = qtim32FileVersion();
            if (qtimUsers == 1)
                qtim_39();
        } else {
            unloadQtim32();
        }
    }
    SetErrorMode(errorMode);
    return result;
}

void __cdecl QTTerminate()
{
    unloadCmgr32();
    unloadQtim32();
}
