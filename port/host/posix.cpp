/*
 * The host for POSIX systems (Linux, macOS): fibers with ucontext, which
 * POSIX has deprecated but every such system still has.
 */

#define _XOPEN_SOURCE 700
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include <SDL.h>
#include <stdio.h>

#include "host/host.h"

static const size_t MIN_STACK = 256 * 1024;

struct HostFiber
{
    ucontext_t context;
    void (*entry)(void *);
    void *argument;
    void *stack;
};

static HostFiber mainFiber;
static HostFiber *current;

HostFiber *hostFiberCurrent()
{
    if (!current)
        current = &mainFiber;
    return current;
}

/* makecontext passes ints: the fiber's address goes in two halves. */
static void start(unsigned int high, unsigned int low)
{
    HostFiber *fiber = (HostFiber *)(size_t)((unsigned long long)high << 32 | low);
    fiber->entry(fiber->argument);
    abort(); /* a fiber's entry never returns */
}

HostFiber *hostFiberCreate(size_t stackSize, void (*entry)(void *), void *argument)
{
    HostFiber *fiber = (HostFiber *)calloc(1, sizeof(HostFiber));
    unsigned long long address = (unsigned long long)(size_t)fiber;

    hostFiberCurrent();
    if (stackSize < MIN_STACK)
        stackSize = MIN_STACK;
    fiber->entry = entry;
    fiber->argument = argument;
    fiber->stack = malloc(stackSize);
    getcontext(&fiber->context);
    fiber->context.uc_stack.ss_sp = fiber->stack;
    fiber->context.uc_stack.ss_size = stackSize;
    fiber->context.uc_link = 0;
    makecontext(&fiber->context, (void (*)())start, 2, (unsigned int)(address >> 32),
                (unsigned int)address);
    return fiber;
}

void hostFiberSwitch(HostFiber *to)
{
    HostFiber *from = hostFiberCurrent();

    if (to == from)
        return;
    current = to;
    swapcontext(&from->context, &to->context);
}

void hostFiberDelete(HostFiber *fiber)
{
    if (!fiber || fiber == &mainFiber || fiber == current)
        return;
    free(fiber->stack);
    free(fiber);
}

/* Nothing else needs the thread: just don't spin. */
void hostYield(unsigned ms)
{
    if (ms)
        usleep(ms * 1000);
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
