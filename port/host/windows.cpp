/*
 * The host for Windows: the system's own fibers. (This file sees the real
 * Win32 API; the game sees miniwin's, whatever the host.)
 */

#include <windows.h>

#include <SDL.h>
#include <stdio.h>

#include "host/host.h"

struct HostFiber
{
    void *fiber;
    void (*entry)(void *);
    void *argument;
};

static HostFiber mainFiber;
static HostFiber *current;

HostFiber *hostFiberCurrent()
{
    if (!current) {
        mainFiber.fiber = ConvertThreadToFiber(0);
        current = &mainFiber;
    }
    return current;
}

static void WINAPI start(void *argument)
{
    HostFiber *fiber = (HostFiber *)argument;
    fiber->entry(fiber->argument);
    ExitProcess(3); /* a fiber's entry never returns */
}

HostFiber *hostFiberCreate(size_t stackSize, void (*entry)(void *), void *argument)
{
    HostFiber *fiber = new HostFiber;

    hostFiberCurrent();
    fiber->entry = entry;
    fiber->argument = argument;
    fiber->fiber = CreateFiber(stackSize < 256 * 1024 ? 256 * 1024 : stackSize, start, fiber);
    return fiber;
}

void hostFiberSwitch(HostFiber *to)
{
    if (to == hostFiberCurrent())
        return;
    current = to;
    SwitchToFiber(to->fiber);
}

void hostFiberDelete(HostFiber *fiber)
{
    if (!fiber || fiber == &mainFiber || fiber == current)
        return;
    DeleteFiber(fiber->fiber);
    delete fiber;
}

/* Nothing else needs the thread: just don't spin. */
void hostYield(unsigned ms)
{
    if (ms)
        Sleep(ms);
}

void hostFilesChanged()
{
}

/* SDL's message box, with the buttons in order. */
int hostMessageBox(const char *title, const char *text, const char *const *buttons, int count)
{
    SDL_MessageBoxButtonData data[8];
    SDL_MessageBoxData box;
    int choice = count - 1;

    if (count > 8)
        count = 8;
    for (int i = 0; i < count; i++) {
        data[i].flags = i == 0 ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT
                               : i == count - 1 ? SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0;
        data[i].buttonid = i;
        data[i].text = buttons[i];
    }
    SDL_zero(box);
    box.flags = SDL_MESSAGEBOX_WARNING;
    box.title = title;
    box.message = text;
    box.numbuttons = count;
    box.buttons = data;
    if (SDL_ShowMessageBox(&box, &choice))
        choice = count - 1;
    return choice;
}

void hostTraceStack()
{
}

void hostTrace(const char *line)
{
    fprintf(stderr, "%s\n", line);
}
