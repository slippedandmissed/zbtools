/*
 * os_manager's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_MANAGER_H
#define OS_MANAGER_H

extern SystemState systemState; /* @data 0x4b9d20 */
void __cdecl debugPrintf(const char *format, ...); /* 0x46db93: to the debugger */
/* Initialises the Mohawk OS layer, with a work buffer. */
short osStartup(HINSTANCE instance, void *stacks, long size); /* 0x46ddaf */
short isMohawkThread(unsigned long thread); /* 0x46e00b: whether a thread runs the Mohawk OS */
BOOL CALLBACK findManagerWindow(HWND window, short *version); /* 0x46e02f */
long atomicDecrement(long *value);
void *atomicExchange(void **target, void *value);
long atomicIncrement(long *value);
short debugBreak(short value);
void enterAllLocks(); /* 0x46dc45 */
void leaveAllLocks(); /* 0x46dd02 */
short osSetActive(short active); /* 0x46da64 */
void deleteTimer(LONG_PTR timer); /* 0x46dbc2 */
void runTimers(unsigned long now, short all); /* 0x46dc64 */
LRESULT CALLBACK timerHook(int code, WPARAM wParam, LPARAM lParam); /* 0x46dd40 */
void osShutdown(); /* 0x46e081 */
void setTimerInterval(LONG_PTR timer, long interval); /* 0x46e0f8 */
LRESULT CALLBACK managerWindowProc(HWND window, UINT message, WPARAM wParam,
                                   LPARAM lParam); /* 0x46e164 */
void osIdle(); /* 0x46e1b2 */
unsigned short bcdVersion(unsigned short version); /* 0x46e21a */
HINSTANCE engineInstanceHandle();
unsigned long appThreadId();
HWND appWindowHandle();
unsigned long currentTimeMs();
short osLockMemory(void *address, unsigned long size);
short osUnlockMemory(void *address, unsigned long size);
short isAppActive();
short osVersion(); /* 0x46dff7: 0x500 once running */
ActivateHook setActivateHook(ActivateHook hook);
HINSTANCE osInstance(LONG_PTR);
short setOsError(short error);
LONG_PTR timerHandle(OsTimer *timer); /* 0x46e1f8 */
OsTimer *timerOf(LONG_PTR timer); /* 0x46e202: 0 if it isn't one */
char __cdecl lowByte(char value); /* 0x46e28e */
int __cdecl highByte(unsigned short value);

#endif
