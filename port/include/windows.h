/*
 * miniwin's windows.h: the Win32 functions the decompiled game calls (types
 * and constants in miniwin/types.h). Implemented in port/miniwin/.
 */

#ifndef MINIWIN_WINDOWS_H
#define MINIWIN_WINDOWS_H

#include "miniwin/types.h"

#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w) ((BYTE)(((DWORD_PTR)(w) >> 8) & 0xff))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#define MAKEINTRESOURCE(i) ((LPSTR)(uintptr_t)((WORD)(i)))
#define MAKEINTATOM(i) ((LPCSTR)(uintptr_t)((WORD)(i)))

namespace miniwin {

/* Kernel: errors and modules */
DWORD GetLastError();
void SetLastError(DWORD error);
UINT SetErrorMode(UINT mode);
DWORD GetVersion();
void GetSystemInfo(LPSYSTEM_INFO info);
HMODULE GetModuleHandle(LPCSTR name);
DWORD GetModuleFileName(HMODULE module, LPSTR name, DWORD size);
HMODULE LoadLibrary(LPCSTR name);
BOOL FreeLibrary(HMODULE module);
FARPROC GetProcAddress(HMODULE module, LPCSTR name);
void OutputDebugString(LPCSTR text);
void DebugBreak();

/* Kernel: files */
HANDLE CreateFile(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security,
                  DWORD creation, DWORD flags, HANDLE templateFile);
BOOL ReadFile(HANDLE file, LPVOID buffer, DWORD size, LPDWORD read, LPOVERLAPPED overlapped);
BOOL WriteFile(HANDLE file, LPCVOID buffer, DWORD size, LPDWORD written,
               LPOVERLAPPED overlapped);
DWORD SetFilePointer(HANDLE file, LONG distance, LPLONG distanceHigh, DWORD method);
BOOL SetEndOfFile(HANDLE file);
DWORD GetFileSize(HANDLE file, LPDWORD high);
BOOL CloseHandle(HANDLE object);
BOOL DeleteFile(LPCSTR name);
BOOL CreateDirectory(LPCSTR name, LPSECURITY_ATTRIBUTES security);
BOOL RemoveDirectory(LPCSTR name);
DWORD GetFileAttributes(LPCSTR name);
BOOL SetFileAttributes(LPCSTR name, DWORD attributes);
HANDLE FindFirstFile(LPCSTR pattern, LPWIN32_FIND_DATA found);
BOOL FindNextFile(HANDLE find, LPWIN32_FIND_DATA found);
BOOL FindClose(HANDLE find);
DWORD GetCurrentDirectory(DWORD size, LPSTR buffer);
BOOL SetCurrentDirectory(LPCSTR path);
DWORD GetTempPath(DWORD size, LPSTR buffer);
DWORD GetLogicalDrives();
UINT GetDriveType(LPCSTR root);
BOOL GetVolumeInformation(LPCSTR root, LPSTR name, DWORD nameSize, LPDWORD serial,
                          LPDWORD maxComponent, LPDWORD flags, LPSTR fileSystem,
                          DWORD fileSystemSize);
BOOL DeviceIoControl(HANDLE device, DWORD code, LPVOID in, DWORD inSize, LPVOID out,
                     DWORD outSize, LPDWORD returned, LPOVERLAPPED overlapped);
UINT GetPrivateProfileInt(LPCSTR section, LPCSTR key, int fallback, LPCSTR file);

/* Kernel: memory */
HGLOBAL GlobalAlloc(UINT flags, DWORD_PTR size);
HGLOBAL GlobalReAlloc(HGLOBAL block, DWORD_PTR size, UINT flags);
HGLOBAL GlobalFree(HGLOBAL block);
LPVOID GlobalLock(HGLOBAL block);
BOOL GlobalUnlock(HGLOBAL block);
HGLOBAL GlobalHandle(LPCVOID pointer);
DWORD_PTR GlobalSize(HGLOBAL block);
void GlobalMemoryStatus(LPMEMORYSTATUS status);
HLOCAL LocalAlloc(UINT flags, DWORD_PTR size);
HLOCAL LocalReAlloc(HLOCAL block, DWORD_PTR size, UINT flags);
HLOCAL LocalFree(HLOCAL block);

/* Kernel: atoms */
ATOM GlobalAddAtom(LPCSTR name);
ATOM GlobalFindAtom(LPCSTR name);
ATOM GlobalDeleteAtom(ATOM atom);

/* Kernel: threads and synchronisation (cooperative: see threads.cpp) */
HANDLE CreateThread(LPSECURITY_ATTRIBUTES security, DWORD_PTR stackSize,
                    LPTHREAD_START_ROUTINE start, LPVOID parameter, DWORD flags, LPDWORD id);
BOOL SetThreadPriority(HANDLE thread, int priority);
DWORD ResumeThread(HANDLE thread);
DWORD SuspendThread(HANDLE thread);
BOOL TerminateThread(HANDLE thread, DWORD code);
DWORD GetCurrentThreadId();
HANDLE CreateEvent(LPSECURITY_ATTRIBUTES security, BOOL manualReset, BOOL initialState,
                   LPCSTR name);
BOOL SetEvent(HANDLE event);
BOOL ResetEvent(HANDLE event);
DWORD WaitForSingleObject(HANDLE object, DWORD timeout);
void Sleep(DWORD ms);
void InitializeCriticalSection(LPCRITICAL_SECTION section);
void DeleteCriticalSection(LPCRITICAL_SECTION section);
void EnterCriticalSection(LPCRITICAL_SECTION section);
void LeaveCriticalSection(LPCRITICAL_SECTION section);
LONG InterlockedIncrement(LONG *value);
LONG InterlockedDecrement(LONG *value);
LONG InterlockedExchange(LONG *target, LONG value);
DWORD GetTickCount();
void GetLocalTime(SYSTEMTIME *time);

/* User: windows and messages */
ATOM RegisterClass(const WNDCLASS *windowClass);
BOOL UnregisterClass(LPCSTR name, HINSTANCE instance);
HWND CreateWindowEx(DWORD exStyle, LPCSTR className, LPCSTR title, DWORD style, int x, int y,
                    int width, int height, HWND parent, HMENU menu, HINSTANCE instance,
                    LPVOID parameter);
BOOL DestroyWindow(HWND window);
BOOL IsWindow(HWND window);
HWND FindWindow(LPCSTR className, LPCSTR title);
BOOL ShowWindow(HWND window, int command);
BOOL UpdateWindow(HWND window);
BOOL SetForegroundWindow(HWND window);
HWND GetActiveWindow();
HWND GetDesktopWindow();
BOOL IsIconic(HWND window);
BOOL GetClientRect(HWND window, LPRECT rect);
BOOL GetWindowRect(HWND window, LPRECT rect);
LONG GetWindowLong(HWND window, int index);
LONG SetWindowLong(HWND window, int index, LONG value);
LONG_PTR GetWindowLongPtr(HWND window, int index);
LONG_PTR SetWindowLongPtr(HWND window, int index, LONG_PTR value);
int GetWindowText(HWND window, LPSTR text, int size);
DWORD GetWindowThreadProcessId(HWND window, LPDWORD process);
BOOL EnumThreadWindows(DWORD thread, WNDENUMPROC proc, LPARAM data);
LRESULT DefWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CallWindowProc(WNDPROC proc, HWND window, UINT message, WPARAM wParam, LPARAM lParam);
BOOL PeekMessage(LPMSG message, HWND window, UINT first, UINT last, UINT flags);
BOOL GetMessage(LPMSG message, HWND window, UINT first, UINT last);
BOOL TranslateMessage(const MSG *message);
LRESULT DispatchMessage(const MSG *message);
BOOL PostMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT SendMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void PostQuitMessage(int code);
UINT_PTR SetTimer(HWND window, UINT_PTR id, UINT elapse, TIMERPROC proc);
BOOL KillTimer(HWND window, UINT_PTR id);
HHOOK SetWindowsHookEx(int kind, HOOKPROC proc, HINSTANCE instance, DWORD thread);
BOOL UnhookWindowsHookEx(HHOOK hook);
LRESULT CallNextHookEx(HHOOK hook, int code, WPARAM wParam, LPARAM lParam);
BOOL InvalidateRect(HWND window, LPCRECT rect, BOOL erase);
BOOL InvalidateRgn(HWND window, HRGN rgn, BOOL erase);
BOOL ValidateRect(HWND window, LPCRECT rect);
BOOL ValidateRgn(HWND window, HRGN rgn);
int GetUpdateRgn(HWND window, HRGN rgn, BOOL erase);
HDC BeginPaint(HWND window, LPPAINTSTRUCT paint);
BOOL EndPaint(HWND window, const PAINTSTRUCT *paint);
int MessageBox(HWND window, LPCSTR text, LPCSTR caption, UINT type);

/* User: input */
SHORT GetKeyState(int key);
SHORT GetAsyncKeyState(int key);
BOOL GetKeyboardState(BYTE *state);
BOOL SetKeyboardState(BYTE *state);
BOOL GetCursorPos(LPPOINT point);
BOOL SetCursorPos(int x, int y);
HCURSOR SetCursor(HCURSOR cursor);
HCURSOR LoadCursor(HINSTANCE instance, LPCSTR name);
HCURSOR CreateCursor(HINSTANCE instance, int hotX, int hotY, int width, int height,
                     const void *andPlane, const void *xorPlane);
BOOL DestroyCursor(HCURSOR cursor);
int ShowCursor(BOOL show);
HICON LoadIcon(HINSTANCE instance, LPCSTR name);

/* User: system */
int GetSystemMetrics(int index);
DWORD GetSysColor(int index);
BOOL SetSysColors(int count, const int *elements, const COLORREF *colors);
int FillRect(HDC dc, const RECT *rect, HBRUSH brush);
int InvertRect(HDC dc, const RECT *rect);
BOOL OffsetRect(LPRECT rect, int dx, int dy);
BOOL SetRect(LPRECT rect, int left, int top, int right, int bottom);
int DrawText(HDC dc, LPCSTR text, int count, LPRECT rect, UINT format);
LONG ChangeDisplaySettings(DEVMODE *mode, DWORD flags);
BOOL EnumDisplaySettings(LPCSTR device, DWORD index, DEVMODE *mode);

/* GDI: device contexts */
HDC GetDC(HWND window);
int ReleaseDC(HWND window, HDC dc);
HDC CreateDC(LPCSTR driver, LPCSTR device, LPCSTR output, const DEVMODE *mode);
HDC CreateIC(LPCSTR driver, LPCSTR device, LPCSTR output, const DEVMODE *mode);
HDC CreateCompatibleDC(HDC dc);
BOOL DeleteDC(HDC dc);
int GetDeviceCaps(HDC dc, int index);
HGDIOBJ SelectObject(HDC dc, HGDIOBJ object);
BOOL DeleteObject(HGDIOBJ object);
HGDIOBJ GetStockObject(int index);
BOOL UnrealizeObject(HGDIOBJ object);
BOOL GdiFlush();
int SetMapMode(HDC dc, int mode);
BOOL SetWindowOrgEx(HDC dc, int x, int y, LPPOINT previous);
BOOL SetWindowExtEx(HDC dc, int x, int y, LPSIZE previous);
BOOL SetViewportOrgEx(HDC dc, int x, int y, LPPOINT previous);
BOOL SetViewportExtEx(HDC dc, int x, int y, LPSIZE previous);
BOOL LPtoDP(HDC dc, LPPOINT points, int count);
BOOL DPtoLP(HDC dc, LPPOINT points, int count);
int SetBkMode(HDC dc, int mode);
COLORREF SetBkColor(HDC dc, COLORREF color);
COLORREF SetTextColor(HDC dc, COLORREF color);
UINT SetTextAlign(HDC dc, UINT align);
int SetROP2(HDC dc, int mode);
int SetPolyFillMode(HDC dc, int mode);
int SetStretchBltMode(HDC dc, int mode);
BOOL MoveToEx(HDC dc, int x, int y, LPPOINT previous);
BOOL GetCurrentPositionEx(HDC dc, LPPOINT position);
BOOL LineTo(HDC dc, int x, int y);
BOOL Rectangle(HDC dc, int left, int top, int right, int bottom);
BOOL Ellipse(HDC dc, int left, int top, int right, int bottom);
BOOL Polygon(HDC dc, const POINT *points, int count);
COLORREF GetPixel(HDC dc, int x, int y);

/* GDI: objects */
HBITMAP CreateCompatibleBitmap(HDC dc, int width, int height);
HBITMAP CreateDIBSection(HDC dc, const BITMAPINFO *info, UINT usage, void **bits,
                         HANDLE section, DWORD offset);
LONG GetBitmapBits(HBITMAP bitmap, LONG size, LPVOID bits);
int GetDIBits(HDC dc, HBITMAP bitmap, UINT start, UINT lines, LPVOID bits, LPBITMAPINFO info,
              UINT usage);
UINT SetDIBColorTable(HDC dc, UINT start, UINT count, const RGBQUAD *colors);
HBRUSH CreateSolidBrush(COLORREF color);
HBRUSH CreateDIBPatternBrush(HGLOBAL packed, UINT usage);
HPEN CreatePen(int style, int width, COLORREF color);
HFONT CreateFontIndirect(const LOGFONT *font);
BOOL GetTextMetrics(HDC dc, LPTEXTMETRIC metrics);
BOOL GetTextExtentPoint(HDC dc, LPCSTR text, int count, LPSIZE size);
int AddFontResource(LPCSTR file);
BOOL RemoveFontResource(LPCSTR file);
BOOL CreateScalableFontResource(DWORD hidden, LPCSTR resource, LPCSTR fontFile, LPCSTR path);

/* GDI: palettes */
HPALETTE CreatePalette(const LOGPALETTE *palette);
HPALETTE SelectPalette(HDC dc, HPALETTE palette, BOOL background);
UINT RealizePalette(HDC dc);
BOOL AnimatePalette(HPALETTE palette, UINT start, UINT count, const PALETTEENTRY *entries);
UINT SetPaletteEntries(HPALETTE palette, UINT start, UINT count, const PALETTEENTRY *entries);
UINT GetPaletteEntries(HPALETTE palette, UINT start, UINT count, LPPALETTEENTRY entries);
UINT GetSystemPaletteEntries(HDC dc, UINT start, UINT count, LPPALETTEENTRY entries);
UINT SetSystemPaletteUse(HDC dc, UINT use);
UINT GetSystemPaletteUse(HDC dc);

/* GDI: regions and clipping */
HRGN CreateRectRgn(int left, int top, int right, int bottom);
HRGN CreateEllipticRgn(int left, int top, int right, int bottom);
HRGN CreatePolygonRgn(const POINT *points, int count, int mode);
BOOL SetRectRgn(HRGN rgn, int left, int top, int right, int bottom);
int CombineRgn(HRGN target, HRGN a, HRGN b, int mode);
int SelectClipRgn(HDC dc, HRGN rgn);
int IntersectClipRect(HDC dc, int left, int top, int right, int bottom);
BOOL FillRgn(HDC dc, HRGN rgn, HBRUSH brush);

/* GDI: drawing */
BOOL BitBlt(HDC to, int x, int y, int width, int height, HDC from, int fromX, int fromY,
            DWORD rop);
BOOL StretchBlt(HDC to, int x, int y, int width, int height, HDC from, int fromX, int fromY,
                int fromWidth, int fromHeight, DWORD rop);
int StretchDIBits(HDC dc, int x, int y, int width, int height, int fromX, int fromY,
                  int fromWidth, int fromHeight, const void *bits, const BITMAPINFO *info,
                  UINT usage, DWORD rop);
BOOL PatBlt(HDC dc, int x, int y, int width, int height, DWORD rop);
BOOL ScrollDC(HDC dc, int dx, int dy, const RECT *scroll, const RECT *clip, HRGN update,
              LPRECT updateRect);

/* Registry */
LONG RegOpenKey(HKEY key, LPCSTR subKey, HKEY *result);
LONG RegCloseKey(HKEY key);
LONG RegQueryValueEx(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data,
                     LPDWORD size);

} /* namespace miniwin */

#endif
