/*
 * The host layer: what the port does differently per target. miniwin uses
 * SDL for everything SDL covers (the window, input, audio, timing); this is
 * the rest:
 *
 * - fibers (execution contexts switched cooperatively), which carry both the
 *   game engine's threads and miniwin's Win32 threads: Emscripten's fibers on
 *   the web, ucontext on POSIX systems, and Win32 fibers on Windows;
 * - giving the host its turn: the game runs its own blocking loops, which a
 *   browser can't wait out, so on the web hostYield returns to the browser
 *   (through Asyncify) and comes back; elsewhere it does nothing;
 * - persisting written files where the host needs telling (the browser's
 *   IndexedDB).
 *
 * Nothing here includes miniwin's headers, so a host file can use a host's
 * own Win32 API.
 */

#ifndef HOST_HOST_H
#define HOST_HOST_H

#include <stddef.h>

struct HostFiber;

/* The fiber running now; the first call makes the initial context one. */
HostFiber *hostFiberCurrent();
/* A new fiber that will run entry(argument) when first switched to. entry
   must never return. */
HostFiber *hostFiberCreate(size_t stackSize, void (*entry)(void *), void *argument);
/* Runs `to`, until something switches back to the current fiber. */
void hostFiberSwitch(HostFiber *to);
/* Frees a fiber that isn't running. */
void hostFiberDelete(HostFiber *fiber);

/* Lets the host run (the browser's event loop), for about `ms` milliseconds
   at most (0: just a turn). */
void hostYield(unsigned ms);

/* Shows a message and waits for one of the buttons (labels) to be chosen;
   its index. */
int hostMessageBox(const char *title, const char *text, const char *const *buttons, int count);

/* Prints a line of diagnostics (the host's console). */
void hostTrace(const char *line);

/* Prints the current call stack, where the host can (for diagnostics). */
void hostTraceStack();

/* Called once miniwin has flushed files it wrote (to persist them, where
   the host has to: the browser's IndexedDB). */
void hostFilesChanged();

#endif
