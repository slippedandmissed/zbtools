/*
 * KERNEL32 (and ADVAPI32's registry), except files (files.cpp) and threads
 * (threads.cpp): errors, the version, modules, memory, atoms, time.
 */

#include <SDL.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "miniwin/internal.h"

namespace miniwin {

/* Declared by threads.cpp, for GetProcAddress. */
LPVOID WINAPI ConvertThreadToFiber(LPVOID parameter);
LPVOID WINAPI CreateFiber(DWORD stackSize, void(WINAPI *start)(LPVOID), LPVOID parameter);
void WINAPI SwitchToFiber(LPVOID fiber);
void WINAPI DeleteFiber(LPVOID fiber);

static DWORD lastError;
static std::string programPath = "C:\\ZOOMBI32\\ZOOMBI32.EXE";

/* Module handles: the program, and the system DLLs it looks things up in. */
static char programModule, kernelModule, userModule, gdiModule;

void trace(const char *format, ...)
{
    char line[1024] = "miniwin: ";
    va_list args;

    va_start(args, format);
    vsnprintf(line + 9, sizeof line - 9, format, args);
    va_end(args);
    hostTrace(line);
}

void unsupported(const char *what)
{
    static std::set<std::string> reported;

    if (reported.insert(what).second)
        trace("not supported: %s", what);
}

void setProgramPath(const char *windowsPath)
{
    programPath = windowsPath;
}

HINSTANCE programInstance()
{
    return (HINSTANCE)&programModule;
}

DWORD GetLastError()
{
    return lastError;
}

void SetLastError(DWORD error)
{
    lastError = error;
}

UINT SetErrorMode(UINT mode)
{
    static UINT current;
    UINT previous = current;
    current = mode;
    return previous;
}

/* Windows 98 (4.10), the least the game's fibers need: Windows 95's lowest
   byte, the major version, comes first; the high bit marks Windows 9x. */
DWORD GetVersion()
{
    return 0xC0000A04;
}

void GetSystemInfo(LPSYSTEM_INFO info)
{
    memset(info, 0, sizeof *info);
    info->dwPageSize = 4096;
    info->lpMinimumApplicationAddress = (LPVOID)0x10000;
    info->lpMaximumApplicationAddress = (LPVOID)0x7ffeffff;
    info->dwActiveProcessorMask = 1;
    info->dwNumberOfProcessors = 1;
    info->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
    info->dwAllocationGranularity = 0x10000;
    info->wProcessorLevel = 5;
}

HMODULE GetModuleHandle(LPCSTR name)
{
    if (!name)
        return (HMODULE)&programModule;
    std::string base(name);
    size_t dot = base.find('.');
    if (dot != std::string::npos)
        base.resize(dot);
    if (!stricmp(base.c_str(), "KERNEL32"))
        return (HMODULE)&kernelModule;
    if (!stricmp(base.c_str(), "USER32"))
        return (HMODULE)&userModule;
    if (!stricmp(base.c_str(), "GDI32"))
        return (HMODULE)&gdiModule;
    lastError = ERROR_FILE_NOT_FOUND;
    return 0;
}

DWORD GetModuleFileName(HMODULE module, LPSTR name, DWORD size)
{
    const char *path = programPath.c_str();

    if (module && module != (HMODULE)&programModule) {
        if (module == (HMODULE)&kernelModule)
            path = "C:\\WINDOWS\\SYSTEM\\KERNEL32.DLL";
        else if (module == (HMODULE)&userModule)
            path = "C:\\WINDOWS\\SYSTEM\\USER32.DLL";
        else
            path = "C:\\WINDOWS\\SYSTEM\\GDI32.DLL";
    }
    if (!size)
        return 0;
    strncpy(name, path, size - 1);
    name[size - 1] = 0;
    return (DWORD)strlen(name);
}

/* No DLL loads: not DirectSound (the game uses waveOut instead), nor the
   decompressors and helpers it looks for. The system's are there. */
HMODULE LoadLibrary(LPCSTR name)
{
    HMODULE module = GetModuleHandle(name);

    if (!module)
        trace("LoadLibrary(\"%s\"): no such library here", name);
    return module;
}

BOOL FreeLibrary(HMODULE)
{
    return TRUE;
}

FARPROC GetProcAddress(HMODULE module, LPCSTR name)
{
    static const struct
    {
        const char *name;
        FARPROC proc;
    } kernel[] = {
        {"ConvertThreadToFiber", (FARPROC)ConvertThreadToFiber},
        {"CreateFiber", (FARPROC)CreateFiber},
        {"SwitchToFiber", (FARPROC)SwitchToFiber},
        {"DeleteFiber", (FARPROC)DeleteFiber},
    };

    if (module == (HMODULE)&kernelModule && (uintptr_t)name > 0xffff)
        for (const auto &entry : kernel)
            if (!strcmp(entry.name, name))
                return entry.proc;
    if ((uintptr_t)name > 0xffff)
        trace("GetProcAddress(\"%s\"): not here", name);
    lastError = ERROR_INVALID_FUNCTION;
    return 0;
}

void OutputDebugString(LPCSTR text)
{
    std::string line(text);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();
    hostTrace(line.c_str());
}

void DebugBreak()
{
    trace("DebugBreak");
}

/* Memory. A fixed block's handle is its address. A moveable block's handle
   points at the block's address (its master pointer), as Win32's do: the
   engine relies on that. Each block has a header before it. */

struct BlockHeader
{
    size_t size;
    void **handle; /* a moveable block's; 0 for a fixed one */
    unsigned locks;
    uint32_t magic;
    uint32_t padding[2];
};

static const uint32_t BLOCK_MAGIC = 0x4d574d42; /* 'MWMB' */
static std::unordered_set<void **> moveableHandles;

static BlockHeader *headerOf(const void *data)
{
    if (!data)
        return 0;
    BlockHeader *header = (BlockHeader *)data - 1;
    return header->magic == BLOCK_MAGIC ? header : 0;
}

static void *allocateData(size_t size, bool zero)
{
    BlockHeader *header = (BlockHeader *)(zero ? calloc(1, sizeof(BlockHeader) + size)
                                               : malloc(sizeof(BlockHeader) + size));
    if (!header)
        return 0;
    header->size = size;
    header->handle = 0;
    header->locks = 0;
    header->magic = BLOCK_MAGIC;
    return header + 1;
}

static void freeData(void *data)
{
    BlockHeader *header = headerOf(data);

    if (header) {
        header->magic = 0;
        free(header);
    }
}

static bool isMoveable(HGLOBAL block)
{
    return moveableHandles.count((void **)block) != 0;
}

HGLOBAL GlobalAlloc(UINT flags, DWORD_PTR size)
{
    void *data = allocateData(size ? size : 1, (flags & GMEM_ZEROINIT) != 0);

    if (!data) {
        lastError = ERROR_NOT_ENOUGH_MEMORY;
        return 0;
    }
    if (!(flags & GMEM_MOVEABLE))
        return data;
    void **handle = new void *(data);
    headerOf(data)->handle = handle;
    moveableHandles.insert(handle);
    return handle;
}

HGLOBAL GlobalReAlloc(HGLOBAL block, DWORD_PTR size, UINT flags)
{
    bool moveable = isMoveable(block);
    void *data = moveable ? *(void **)block : block;
    BlockHeader *header = headerOf(data);

    if (!header) {
        lastError = ERROR_INVALID_HANDLE;
        return 0;
    }
    if (!size)
        size = 1;
    if (!moveable && !(flags & GMEM_MOVEABLE) && size > header->size) {
        /* A fixed block may only change where it is. */
        lastError = ERROR_NOT_ENOUGH_MEMORY;
        return 0;
    }
    if (size <= header->size && !moveable && !(flags & GMEM_MOVEABLE)) {
        header->size = size;
        return block;
    }
    BlockHeader *resized = (BlockHeader *)realloc(header, sizeof(BlockHeader) + size);
    if (!resized) {
        lastError = ERROR_NOT_ENOUGH_MEMORY;
        return 0;
    }
    if (size > resized->size && (flags & GMEM_ZEROINIT))
        memset((char *)(resized + 1) + resized->size, 0, size - resized->size);
    resized->size = size;
    if (moveable) {
        *(void **)block = resized + 1;
        return block;
    }
    return resized + 1;
}

HGLOBAL GlobalFree(HGLOBAL block)
{
    if (!block)
        return 0;
    if (isMoveable(block)) {
        void **handle = (void **)block;
        freeData(*handle);
        moveableHandles.erase(handle);
        delete handle;
        return 0;
    }
    if (!headerOf(block)) {
        lastError = ERROR_INVALID_HANDLE;
        return block;
    }
    freeData(block);
    return 0;
}

LPVOID GlobalLock(HGLOBAL block)
{
    if (!block)
        return 0;
    if (isMoveable(block)) {
        void *data = *(void **)block;
        headerOf(data)->locks++;
        return data;
    }
    return block;
}

BOOL GlobalUnlock(HGLOBAL block)
{
    if (isMoveable(block)) {
        BlockHeader *header = headerOf(*(void **)block);
        if (header->locks)
            header->locks--;
        return header->locks != 0;
    }
    return FALSE;
}

HGLOBAL GlobalHandle(LPCVOID pointer)
{
    BlockHeader *header = headerOf(pointer);

    if (!header)
        return 0;
    return header->handle ? (HGLOBAL)header->handle : (HGLOBAL)pointer;
}

DWORD_PTR GlobalSize(HGLOBAL block)
{
    BlockHeader *header = headerOf(isMoveable(block) ? *(void **)block : block);
    return header ? header->size : 0;
}

/* A machine of the day's better sort: 32 MB of memory. (The game wants 6 MB
   and 3.5 MB free.) */
void GlobalMemoryStatus(LPMEMORYSTATUS status)
{
    status->dwLength = sizeof *status;
    status->dwMemoryLoad = 25;
    status->dwTotalPhys = 32 << 20;
    status->dwAvailPhys = 24 << 20;
    status->dwTotalPageFile = 64 << 20;
    status->dwAvailPageFile = 56 << 20;
    status->dwTotalVirtual = 0x7fe00000;
    status->dwAvailVirtual = 0x70000000;
}

HLOCAL LocalAlloc(UINT flags, DWORD_PTR size)
{
    return GlobalAlloc(flags, size);
}

HLOCAL LocalReAlloc(HLOCAL block, DWORD_PTR size, UINT flags)
{
    return GlobalReAlloc(block, size, flags);
}

HLOCAL LocalFree(HLOCAL block)
{
    return GlobalFree(block);
}

/* Atoms: the game's "am I running already?" check. */

static std::map<std::string, ATOM> atoms;
static ATOM nextAtom = 0xC000;

static std::string upper(const char *text)
{
    std::string result(text);
    for (char &c : result)
        c = (char)toupper((unsigned char)c);
    return result;
}

ATOM GlobalAddAtom(LPCSTR name)
{
    std::string key = upper(name);
    auto found = atoms.find(key);
    if (found != atoms.end())
        return found->second;
    return atoms[key] = nextAtom++;
}

ATOM GlobalFindAtom(LPCSTR name)
{
    auto found = atoms.find(upper(name));
    return found == atoms.end() ? 0 : found->second;
}

ATOM GlobalDeleteAtom(ATOM atom)
{
    for (auto i = atoms.begin(); i != atoms.end(); ++i)
        if (i->second == atom) {
            atoms.erase(i);
            return 0;
        }
    return atom;
}

/* Interlocked operations: one thread runs at a time. */

LONG InterlockedIncrement(LONG *value)
{
    return ++*value;
}

LONG InterlockedDecrement(LONG *value)
{
    return --*value;
}

LONG InterlockedExchange(LONG *target, LONG value)
{
    LONG previous = *target;
    *target = value;
    return previous;
}

/* Time. The clock starts at a minute, as if Windows had been up that long
   (so no time the game computes is 0, which it takes for "none"). */

static const DWORD CLOCK_START = 60000;

DWORD now()
{
    return CLOCK_START + (DWORD)SDL_GetTicks();
}

DWORD GetTickCount()
{
    service();
    return now();
}

void GetLocalTime(SYSTEMTIME *local)
{
    time_t seconds = time(0);
    struct tm parts = *localtime(&seconds);

    local->wYear = (WORD)(parts.tm_year + 1900);
    local->wMonth = (WORD)(parts.tm_mon + 1);
    local->wDayOfWeek = (WORD)parts.tm_wday;
    local->wDay = (WORD)parts.tm_mday;
    local->wHour = (WORD)parts.tm_hour;
    local->wMinute = (WORD)parts.tm_min;
    local->wSecond = (WORD)parts.tm_sec;
    local->wMilliseconds = 0;
}

/* The registry: empty. (The game looks up MIDI settings there.) */

LONG RegOpenKey(HKEY, LPCSTR, HKEY *result)
{
    *result = 0;
    return ERROR_FILE_NOT_FOUND;
}

LONG RegCloseKey(HKEY)
{
    return ERROR_SUCCESS;
}

LONG RegQueryValueEx(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_FILE_NOT_FOUND;
}

} /* namespace miniwin */
