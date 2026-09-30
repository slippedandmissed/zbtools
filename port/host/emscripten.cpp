/*
 * The web host: Emscripten's fibers (built on Asyncify, as is hostYield),
 * and IndexedDB for the files the game writes (web/pre.js mounts C: there).
 */

#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/fiber.h>
#include <stdlib.h>

#include <string>

#include "host/host.h"

/* Asyncify's stack holds a fiber's unwound calls while it's switched out:
   the game's deepest calls go through a few hundred frames. */
static const size_t ASYNCIFY_STACK = 256 * 1024;
static const size_t MIN_C_STACK = 256 * 1024;

struct HostFiber
{
    emscripten_fiber_t fiber;
    void (*entry)(void *);
    void *argument;
    void *cStack;
    void *asyncifyStack;
};

static HostFiber mainFiber;
static HostFiber *current;

HostFiber *hostFiberCurrent()
{
    if (!current) {
        mainFiber.asyncifyStack = malloc(ASYNCIFY_STACK);
        emscripten_fiber_init_from_current_context(&mainFiber.fiber, mainFiber.asyncifyStack,
                                                   ASYNCIFY_STACK);
        current = &mainFiber;
    }
    return current;
}

static void start(void *argument)
{
    HostFiber *fiber = (HostFiber *)argument;
    fiber->entry(fiber->argument);
    abort(); /* a fiber's entry never returns */
}

HostFiber *hostFiberCreate(size_t stackSize, void (*entry)(void *), void *argument)
{
    HostFiber *fiber = (HostFiber *)calloc(1, sizeof(HostFiber));

    hostFiberCurrent();
    if (stackSize < MIN_C_STACK)
        stackSize = MIN_C_STACK;
    fiber->entry = entry;
    fiber->argument = argument;
    fiber->cStack = malloc(stackSize);
    fiber->asyncifyStack = malloc(ASYNCIFY_STACK);
    emscripten_fiber_init(&fiber->fiber, start, fiber, fiber->cStack, stackSize,
                          fiber->asyncifyStack, ASYNCIFY_STACK);
    return fiber;
}

void hostFiberSwitch(HostFiber *to)
{
    HostFiber *from = hostFiberCurrent();

    if (to == from)
        return;
    current = to;
    emscripten_fiber_swap(&from->fiber, &to->fiber);
}

void hostFiberDelete(HostFiber *fiber)
{
    if (!fiber || fiber == &mainFiber || fiber == current)
        return;
    free(fiber->cStack);
    free(fiber->asyncifyStack);
    free(fiber);
}

void hostYield(unsigned ms)
{
    emscripten_sleep(ms);
}

void hostFilesChanged()
{
    /* web/pre.js syncs the mounted IndexedDB file system, at most once a
       second. */
    EM_ASM({
        if (Module.zbPersist)
            Module.zbPersist();
    });
}

/* (Under Node, a headless build, there's no one to ask: OK.) */
EM_JS(int, confirmBox, (const char *text), {
    if (typeof confirm !== 'function')
        return 1;
    return confirm(UTF8ToString(text)) ? 1 : 0;
});
EM_JS(void, alertBox, (const char *text), {
    if (typeof alert === 'function')
        alert(UTF8ToString(text));
});

/* The browser's alert and confirm (its OK the first button, Cancel the last). */
int hostMessageBox(const char *title, const char *text, const char *const *buttons, int count)
{
    std::string message = std::string(title) + "\n\n" + text;

    if (count <= 1) {
        alertBox(message.c_str());
        return 0;
    }
    message += std::string("\n\nOK: ") + buttons[0] + "    Cancel: " + buttons[count - 1];
    return confirmBox(message.c_str()) ? 0 : count - 1;
}

void hostTraceStack()
{
    emscripten_log(EM_LOG_C_STACK | EM_LOG_ERROR, "stack:");
}

/* Through the JavaScript console: C's stderr, redirected to a file under
   Node's raw file system, can stall writing. */
void hostTrace(const char *line)
{
    emscripten_err(line);
}
