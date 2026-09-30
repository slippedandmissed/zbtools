/*
 * Threads, cooperatively: each Win32 thread is a host fiber, and one runs
 * at a time, as on the uniprocessor PCs the game was written for. A thread
 * runs until it waits (WaitForSingleObject, Sleep, GetMessage with nothing
 * to get) or wakes a thread of higher priority, which then runs at once, as
 * Windows would preempt it (the game's file worker, above normal priority,
 * relies on that to finish its call before the caller looks). While every
 * thread waits, service() does what the system would meanwhile.
 *
 * Win32 fibers, which the game's engine runs its own threads on (looking
 * them up in KERNEL32), are host fibers within the main thread.
 */

#include <stdlib.h>
#include <string.h>

#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

struct Thread : KernelObject
{
    HostFiber *fiber; /* what it runs on (the main thread's changes with the game's fibers) */
    LPTHREAD_START_ROUTINE start;
    LPVOID parameter;
    DWORD id;
    int priority;
    int suspended;
    bool finished;
    DWORD exitCode;
    /* While it waits: until ready(readyData), or deadline (with timed). */
    bool waiting;
    bool (*ready)(void *);
    void *readyData;
    bool timed;
    DWORD deadline;
    Thread() : KernelObject(OBJECT_THREAD), fiber(0), start(0), parameter(0), id(0), priority(0),
               suspended(0), finished(false), exitCode(0), waiting(false), ready(0),
               readyData(0), timed(false), deadline(0) {}
};

struct Event : KernelObject
{
    bool manualReset;
    bool signaled;
    Event() : KernelObject(OBJECT_EVENT), manualReset(false), signaled(false) {}
};

static Thread mainThread;
static Thread *current = &mainThread;
static std::vector<Thread *> threads;
static DWORD nextThreadId = 0x100;

void initThreads()
{
    mainThread.id = nextThreadId++;
    mainThread.fiber = hostFiberCurrent();
    threads.push_back(&mainThread);
}

bool isMainThread()
{
    return current == &mainThread;
}

static bool timedOut(Thread *thread)
{
    return thread->timed && (LONG)(now() - thread->deadline) >= 0;
}

static bool runnable(Thread *thread)
{
    if (thread->finished || thread->suspended > 0)
        return false;
    if (!thread->waiting)
        return true;
    return (thread->ready && thread->ready(thread->readyData)) || timedOut(thread);
}

/* The thread to run instead of the current one: the runnable one of the
   highest priority (at least `least`), the first after the current one
   among equals. */
static Thread *pick(int least)
{
    Thread *best = 0;
    size_t count = threads.size();
    size_t at = 0;

    for (size_t i = 0; i < count; i++)
        if (threads[i] == current)
            at = i;
    for (size_t i = 1; i <= count; i++) {
        Thread *thread = threads[(at + i) % count];
        if (thread != current && thread->priority >= least && runnable(thread)
            && (!best || thread->priority > best->priority))
            best = thread;
    }
    return best;
}

static void runThread(void *argument);

static void switchTo(Thread *thread)
{
    if (thread == current)
        return;
    current->fiber = hostFiberCurrent();
    current = thread;
    if (!thread->fiber)
        thread->fiber = hostFiberCreate(0, runThread, thread);
    hostFiberSwitch(thread->fiber);
}

static void runThread(void *argument)
{
    Thread *thread = (Thread *)argument;

    thread->exitCode = thread->start(thread->parameter);
    thread->finished = true;
    for (;;) {
        Thread *next = pick(-100);
        switchTo(next ? next : &mainThread);
    }
}

/* Something may have made a thread runnable: one of higher priority than
   the current one runs now. */
static void preempt()
{
    Thread *thread = pick(current->priority + 1);
    if (thread)
        switchTo(thread);
}

void yieldThreads(bool all)
{
    Thread *thread = pick(all ? -100 : current->priority);
    if (thread)
        switchTo(thread);
}

bool inBackground();

bool waitFor(bool (*ready)(void *), void *data, DWORD timeout)
{
    Thread *self = current;

    if (ready && ready(data))
        return true;
    if (!timeout)
        return false;
    if (inBackground()) {
        static bool reported;
        if (!reported) {
            reported = true;
            trace("a timer or sound callback waits (as it couldn't on Windows's own thread):");
            hostTraceStack();
        }
    }
    self->waiting = true;
    self->ready = ready;
    self->readyData = data;
    self->timed = timeout != INFINITE;
    self->deadline = now() + timeout;
    for (;;) {
        if (ready && ready(data))
            break;
        if (timedOut(self))
            break;
        Thread *other = pick(-100);
        if (other)
            switchTo(other);
        else
            service(true);
    }
    self->waiting = false;
    return ready && ready(data);
}

/* Threads */

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, DWORD_PTR, LPTHREAD_START_ROUTINE start,
                    LPVOID parameter, DWORD flags, LPDWORD id)
{
    Thread *thread = new Thread;

    thread->start = start;
    thread->parameter = parameter;
    thread->id = nextThreadId++;
    thread->suspended = flags & CREATE_SUSPENDED ? 1 : 0;
    threads.push_back(thread);
    if (id)
        *id = thread->id;
    return thread;
}

static Thread *threadOf(HANDLE handle)
{
    return (Thread *)kernelObjectOf(handle, OBJECT_THREAD);
}

bool closeThreadObject(KernelObject *object)
{
    if (object->type == OBJECT_EVENT) {
        delete (Event *)object;
        return true;
    }
    if (object->type != OBJECT_THREAD)
        return false;
    /* A thread's object lasts as long as the thread (it's never freed while
       the thread may still run: a fiber can't free itself). */
    return true;
}

BOOL SetThreadPriority(HANDLE handle, int priority)
{
    Thread *thread = threadOf(handle);

    if (!thread)
        return FALSE;
    thread->priority = priority;
    return TRUE;
}

DWORD ResumeThread(HANDLE handle)
{
    Thread *thread = threadOf(handle);

    if (!thread)
        return 0xffffffff;
    DWORD previous = (DWORD)thread->suspended;
    if (thread->suspended > 0 && --thread->suspended == 0)
        preempt();
    return previous;
}

DWORD SuspendThread(HANDLE handle)
{
    Thread *thread = threadOf(handle);

    if (!thread)
        return 0xffffffff;
    DWORD previous = (DWORD)thread->suspended++;
    if (thread == current)
        yieldThreads(true);
    return previous;
}

BOOL TerminateThread(HANDLE handle, DWORD code)
{
    Thread *thread = threadOf(handle);

    if (!thread || thread == current || thread == &mainThread)
        return FALSE;
    thread->finished = true;
    thread->exitCode = code;
    return TRUE;
}

DWORD GetCurrentThreadId()
{
    return current->id;
}

/* Events and waiting */

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL manualReset, BOOL initialState, LPCSTR)
{
    Event *event = new Event;

    event->manualReset = manualReset != 0;
    event->signaled = initialState != 0;
    return event;
}

BOOL SetEvent(HANDLE handle)
{
    Event *event = (Event *)kernelObjectOf(handle, OBJECT_EVENT);

    if (!event)
        return FALSE;
    event->signaled = true;
    preempt();
    return TRUE;
}

BOOL ResetEvent(HANDLE handle)
{
    Event *event = (Event *)kernelObjectOf(handle, OBJECT_EVENT);

    if (!event)
        return FALSE;
    event->signaled = false;
    return TRUE;
}

static bool objectSignaled(void *data)
{
    KernelObject *object = (KernelObject *)data;

    if (object->type == OBJECT_EVENT)
        return ((Event *)object)->signaled;
    if (object->type == OBJECT_THREAD)
        return ((Thread *)object)->finished;
    return true;
}

DWORD WaitForSingleObject(HANDLE handle, DWORD timeout)
{
    KernelObject *object = kernelObjectOf(handle);

    if (!object || (object->type != OBJECT_EVENT && object->type != OBJECT_THREAD)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return WAIT_FAILED;
    }
    if (!waitFor(objectSignaled, object, timeout))
        return WAIT_TIMEOUT;
    if (object->type == OBJECT_EVENT && !((Event *)object)->manualReset)
        ((Event *)object)->signaled = false;
    return WAIT_OBJECT_0;
}

static bool never(void *)
{
    return false;
}

void Sleep(DWORD ms)
{
    service();
    if (ms)
        waitFor(never, 0, ms);
    else
        yieldThreads(true);
}

/* Critical sections: owner and recursion count kept in the structure. */

void InitializeCriticalSection(LPCRITICAL_SECTION section)
{
    memset(section, 0, sizeof *section);
}

void DeleteCriticalSection(LPCRITICAL_SECTION section)
{
    memset(section, 0, sizeof *section);
}

static bool sectionFree(void *data)
{
    return !((LPCRITICAL_SECTION)data)->owner;
}

void EnterCriticalSection(LPCRITICAL_SECTION section)
{
    if (section->owner != current)
        waitFor(sectionFree, section, INFINITE);
    section->owner = current;
    section->recursion++;
}

void LeaveCriticalSection(LPCRITICAL_SECTION section)
{
    if (section->owner == current && --section->recursion <= 0) {
        section->owner = 0;
        section->recursion = 0;
        preempt();
    }
}

/* Win32 fibers, for GetProcAddress (kernel.cpp). */

LPVOID WINAPI ConvertThreadToFiber(LPVOID)
{
    return hostFiberCurrent();
}

LPVOID WINAPI CreateFiber(DWORD stackSize, void(WINAPI *start)(LPVOID), LPVOID parameter)
{
    return hostFiberCreate(stackSize, start, parameter);
}

void WINAPI SwitchToFiber(LPVOID fiber)
{
    hostFiberSwitch((HostFiber *)fiber);
}

void WINAPI DeleteFiber(LPVOID fiber)
{
    hostFiberDelete((HostFiber *)fiber);
}

} /* namespace miniwin */
