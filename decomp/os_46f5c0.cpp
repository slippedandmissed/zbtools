/*
 * os_46f5c0 (0x46f5c0-0x46f7a4): the threads' contexts: their stacks, and
 * switching between them (hand-written assembly)
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include <string.h>
#include "zoombinis.h"
#include "os_46f5c0.h"
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
short initContext(Context *context, void (*proc)(long), long argument, unsigned short stackSize)
{
    unsigned long need;
    long *block;
    long *after;
    long *top;

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
    top = (long *)((char *)context->stack + stackSize - 4);
    top[0] = argument;
    top[-1] = (long)threadExit;
    context->eip = (unsigned long)proc;
    context->esp = (unsigned long)&top[-1];
    return setThreadError(0);
}

/* Frees a context's stack. */
/* @zoombi32 0x0046f68f */
short freeContext(Context *context)
{
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
 * The rest of the context switching loads and saves registers and switches
 * stacks (`mov esp, ...`, `popfd`, a `ret` or `jmp` to the saved address),
 * which only machine code does: left as documented stubs. A port replaces
 * this layer (e.g. with Windows fibers or a coroutine library).
 */

/* Loads a context's registers and flags and continues where it left off:
   switches to that thread. */
/* @zoombi32 0x0046f6c9 */
void resumeContext(Context *)
{
}

/* Switches to a context's stack and returns to this function's caller on
   it (deleteSync uses it to leave a thread that is deleting itself). */
/* @zoombi32 0x0046f6f9 */
void abandonContext(Context *)
{
}

/* Saves the registers in `save`, to resume from this call's return, and
   resumes `to`. */
/* @zoombi32 0x0046f70e */
void switchContext(Context *, Context *)
{
}

/* Saves the current thread's stack pointer (unless it is `to`) and returns
   on `to`'s. */
/* @zoombi32 0x0046f74f */
void switchStack(thread *)
{
}

/*
 * Records as a context's resume address the return address `depth` frames
 * up the stack: a hand-written loop follows the saved frame pointers
 * (`mov eax, [eax]`), then reads the return address above the frame
 * reached. That depends on the x86 stack layout, so it has no portable
 * equivalent (a documented stub).
 */
/* @zoombi32 0x0046f771 */
void recordReturn(Context *, unsigned short)
{
}

/* @zoombi32 0x0046f78e */
short setThreadError(short error)
{
    return threads.error = error;
}
