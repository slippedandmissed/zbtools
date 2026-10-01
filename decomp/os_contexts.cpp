/*
 * os_contexts (0x46f5c0-0x46f7a4): the threads' contexts: their stacks, and
 * switching between them (hand-written assembly)
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include <string.h>
#include "zoombinis.h"
#include "os_contexts.h"
#include "os_threads.h"

/*
 * Sets up a thread's context: a stack of `stackSize` bytes, first fit from
 * the threads' stack blocks (joining free neighbours, splitting off what's
 * left if that's 4 KB or more), which returns to threadExit with `argument`
 * above it, and a start at `proc`. Without a procedure (the main thread),
 * just clears it.
 *
 * The original is in assembly (it keeps the block in eax, the size in ecx);
 * this does the same in C++. The stack it builds is x86 (32-bit words); a
 * port that replaces the context switching (see resumeContext) would start
 * the thread its own way.
 */
/* @zoombi32-functional 0x0046f5c0 */
short initContext(Context *context, void (*proc)(LONG_PTR), LONG_PTR argument, unsigned short stackSize)
{
    unsigned long need;
    long *block;
    long *after;
    LONG_PTR *top;

    memset(context, 0, sizeof(Context));
    if (!proc)
        return setThreadError(0);
    stackSize &= ~3;
    need = stackSize;
    stackSize -= 4;
    for (block = threads.stacks; block < threads.stacksEnd;
         block = (long *)((char *)block + (*block & ~3))) {
        if (*block & 1)
            continue;
        while ((after = (long *)((char *)block + (*block & ~3))) < threads.stacksEnd
               && !(*after & 1))
            *block += *after;
        if (need <= (unsigned long)*block)
            break;
    }
    if (block >= threads.stacksEnd)
        return setThreadError(0x167);
    if (*block - need >= 0x1000) {
        block[need >> 2] = *block - need;
        *block = need;
    }
    *block |= 1;
    context->stack = block + 1;
    top = (LONG_PTR *)((char *)context->stack + stackSize) - 2;
    top[1] = argument;
    top[0] = (LONG_PTR)threadExit;
    context->eip = (UINT_PTR)proc;
    context->esp = (UINT_PTR)top;
    return setThreadError(0);
}

/*
 * The rest of the original switches contexts in assembly: it saves and loads
 * registers and flags and switches stacks (`mov esp, ...`, `popfd`, a `ret`
 * or `jmp` to the saved address), which only machine code does. Here each
 * context runs on a Win32 fiber instead: the same cooperative switching, on
 * the one Win32 thread (which the engine relies on: the scheduler pumps that
 * thread's messages while every context waits).
 *
 * A context's fiber is made when it's first resumed; it runs the context's
 * procedure (initContext's `eip`, its argument above the stack's return
 * address), then threadExit, as the context's stack would. The main thread
 * becomes a fiber at its first switch. A fiber can't delete itself, so a
 * context freed while its fiber runs (deleteSync, on a thread deleting
 * itself) leaves its fiber for the next fiber to run to delete.
 *
 * Borland C++ 4.5's headers predate fibers (Windows 98 and NT 4 have them),
 * so KERNEL32's are looked up. Without them, switching does nothing. A port
 * does this its own way (a coroutine library), as initContext says.
 */

typedef void(WINAPI *FiberStart)(void *parameter);
typedef void *(WINAPI *ConvertThreadToFiberProc)(void *parameter);
typedef void *(WINAPI *CreateFiberProc)(DWORD stackSize, FiberStart start, void *parameter);
typedef void(WINAPI *FiberProc)(void *fiber); /* SwitchToFiber, DeleteFiber */

struct Fibers
{
    short loaded; /* 1: looked up; -1: the system has none */
    ConvertThreadToFiberProc convertThread;
    CreateFiberProc create;
    FiberProc switchTo;
    FiberProc remove;
    void *current; /* the running fiber */
    void *dead; /* a freed context's, to delete once another runs */
    struct
    {
        Context *context;
        void *fiber;
    } bound[64]; /* contexts and their fibers */
};

static Fibers fibers;

/* Whether there are fibers to use: looks them up, and makes the thread one. */
static short useFibers()
{
    if (!fibers.loaded) {
        HMODULE kernel = GetModuleHandle("KERNEL32");

        fibers.convertThread =
            (ConvertThreadToFiberProc)GetProcAddress(kernel, "ConvertThreadToFiber");
        fibers.create = (CreateFiberProc)GetProcAddress(kernel, "CreateFiber");
        fibers.switchTo = (FiberProc)GetProcAddress(kernel, "SwitchToFiber");
        fibers.remove = (FiberProc)GetProcAddress(kernel, "DeleteFiber");
        fibers.loaded = -1;
        if (fibers.convertThread && fibers.create && fibers.switchTo && fibers.remove
            && (fibers.current = fibers.convertThread(0)) != 0)
            fibers.loaded = 1;
    }
    return fibers.loaded > 0;
}

static void *fiberOf(Context *context);
static void switchToFiber(void *fiber);

/* Deletes the fiber left by a context freed while it ran, now it doesn't. */
static void deleteDeadFiber()
{
    if (fibers.dead && fibers.dead != fibers.current) {
        fibers.remove(fibers.dead);
        fibers.dead = 0;
    }
}

/* The start of a context's fiber: its procedure, then threadExit, which
   resumes another thread. A fiber's start mustn't return (the Win32 thread
   would end), so should threadExit come back, this resumes the main thread. */
static void WINAPI startFiber(void *parameter)
{
    Context *context = (Context *)parameter;

    deleteDeadFiber();
    ((void (*)(LONG_PTR))context->eip)(((LONG_PTR *)context->esp)[1]);
    threadExit();
    for (;;)
        switchToFiber(fiberOf(&threads.main->context));
}

/* A context's slot in the table: the one binding it, or with `make`, a free
   one (0 if there's none). */
static short slotOf(Context *context, short make)
{
    short i;
    short free = -1;

    for (i = 0; i < 64; i++) {
        if (fibers.bound[i].context == context)
            return i;
        if (free < 0 && !fibers.bound[i].context)
            free = i;
    }
    return make ? free : -1;
}

/* Binds a context to a fiber (0: unbinds it). */
static void bindFiber(Context *context, void *fiber)
{
    short slot = slotOf(context, 1);

    if (slot >= 0) {
        fibers.bound[slot].context = fiber ? context : 0;
        fibers.bound[slot].fiber = fiber;
    }
}

/* A context's fiber, made for it on first use if it has a procedure. */
static void *fiberOf(Context *context)
{
    short slot = slotOf(context, 0);
    void *fiber;

    if (slot >= 0)
        return fibers.bound[slot].fiber;
    if (!context->eip || (fiber = fibers.create(0, startFiber, context)) == 0)
        return 0;
    bindFiber(context, fiber);
    return fiber;
}

/* Runs another fiber; returns when something switches back to this one. */
static void switchToFiber(void *fiber)
{
    if (!fiber || fiber == fibers.current)
        return;
    fibers.current = fiber;
    fibers.switchTo(fiber);
    deleteDeadFiber();
}

/*
 * Frees a context's stack, and its fiber (or, if it's the one running,
 * leaves it to be deleted once another runs).
 *
 * The original frees just the stack; the fiber is this version's.
 */
/* @zoombi32-functional 0x0046f68f */
short freeContext(Context *context)
{
    void *fiber;

    if (fibers.loaded > 0) {
        short slot = slotOf(context, 0);

        if (slot >= 0) {
            fiber = fibers.bound[slot].fiber;
            bindFiber(context, 0);
            if (fiber == fibers.current) {
                deleteDeadFiber();
                fibers.dead = fiber;
            } else {
                fibers.remove(fiber);
            }
        }
    }
    if (context->stack) {
        setThreadError(0);
        context->stack[-1] &= ~1;
    } else {
        setThreadError(0);
    }
    memset(context, 0, sizeof(Context));
    return threads.error;
}

/*
 * Continues a context where it left off: switches to that thread. What was
 * running is abandoned (a thread that has ended). A context resumed from
 * where it's running (recordReturn's) carries on.
 *
 * The original loads the context's registers and flags and returns to its
 * saved address on its stack.
 */
/* @zoombi32-functional 0x0046f6c9 */
void resumeContext(Context *context)
{
    if (useFibers())
        switchToFiber(fiberOf(context));
}

/*
 * The original switches to a context's stack and returns to its caller
 * there, so that deleteSync can delete a thread that's deleting itself from
 * elsewhere than that thread's stack. Here the thread deletes itself on its
 * own fiber, which freeContext leaves to be deleted once another runs.
 */
/* @zoombi32-functional 0x0046f6f9 */
void abandonContext(Context *)
{
}

/*
 * Saves the running thread in `save`, to carry on from this call's return
 * when it's resumed, and resumes `to`.
 *
 * The original saves the registers, flags and return address in `save` and
 * loads `to`'s (resumeContext).
 */
/* @zoombi32-functional 0x0046f70e */
void switchContext(Context *to, Context *save)
{
    if (!useFibers())
        return;
    bindFiber(save, fibers.current);
    switchToFiber(fiberOf(to));
}

/*
 * The original saves the current thread's stack pointer (unless it's `to`)
 * and returns on `to`'s. Nothing calls it; fibers keep their own stacks.
 */
/* @zoombi32-functional 0x0046f74f */
void switchStack(thread *)
{
}

/*
 * Records, as where a context resumes, the return address `depth` frames up
 * the stack (the original follows the saved frame pointers). Its one caller,
 * stopOtherThreads (never called), records its own return and resumes the
 * main thread there: carries on from its caller as the main thread. Here the
 * context becomes the running fiber's, so resuming it carries on.
 */
/* @zoombi32-functional 0x0046f771 */
void recordReturn(Context *context, unsigned short)
{
    if (useFibers())
        bindFiber(context, fibers.current);
}

/* @zoombi32 0x0046f78e */
short setThreadError(short error)
{
    return threads.error = error;
}
