/*
 * os_threads (0x46e2a4-0x46f5c0): the Mohawk OS layer's cooperative threads,
 * mutexes and events (see `thread` in zoombinis.h), and their scheduler
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-); this module
   without exception handling, like the engine (its constructors don't count
   the objects they make). */
/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

ThreadState threads;
thread *dyingThread;

#define SYNC_TAG RESOURCE_TYPE('s', 'y', 'n', 'c')
#define THREAD_KIND RESOURCE_TYPE('t', 'h', 'r', 'd')
#define MUTEX_KIND RESOURCE_TYPE('m', 'u', 't', 'x')
#define EVENT_KIND RESOURCE_TYPE('e', 'v', 'n', 't')

/* Ends every thread but the main one, and carries on as the main thread
   from where this was called. */
/* @zoombi32 0x0046e2a4 */
void stopOtherThreads()
{
    thread *t;
    short elsewhere = threads.current != threads.main;

    threads.schedulingOff = 0;
    for (t = threads.main->next; t != threads.main; t = t->next)
        t->setSignaled(1);
    if (elsewhere) {
        recordReturn(&threads.main->context, 1);
        resumeContext(&threads.main->context);
    }
}

/* A thread running proc(argument) on a stack of its own, suspended. */
/* @zoombi32 0x0046e302 */
long createThread(void (*proc)(long), long argument, unsigned short stackSize,
                  unsigned short priority)
{
    thread *t = new thread;

    if (!t)
        return 0;
    if (initContext(&t->context, proc, argument, stackSize)) {
        delete t;
        return 0;
    }
    t->priority = priority;
    t->suspendCount = 1;
    t->setSignaled(0);
    setThreadError(0);
    return syncHandle(t);
}

/* @zoombi32 0x0046e380 */
long newEvent(short set)
{
    event *e = new event;

    if (!e)
        return 0;
    e->setSignaled(set);
    setThreadError(0);
    return syncHandle(e);
}

/* @zoombi32 0x0046e3c8 */
long newMutex(short free)
{
    mutex *m = new mutex;

    if (!m)
        return 0;
    m->setSignaled(free);
    setThreadError(0);
    return syncHandle(m);
}

/* Keeps the current thread running (nestable). */
/* @zoombi32 0x0046e410 */
void disableScheduling()
{
    if (threads.schedulingOff == 0xffff) {
        setThreadError(0x164);
        return;
    }
    threads.schedulingOff++;
    setThreadError(0);
}

/* @zoombi32 0x0046e43a */
void enableScheduling()
{
    if (!threads.schedulingOff) {
        setThreadError(0x164);
        return;
    }
    threads.schedulingOff--;
    setThreadError(0);
}

/* Deletes a sync object; a thread deleting itself ends there. Not exact:
   the original keeps `object` in esi and `t` in ebx; BCC32 4.5 swaps them. */
/* @zoombi32 0x0046e463 */
short deleteSync(long handle)
{
    thread *t;
    sync *object;

    if ((object = syncOf(handle, 0)) == 0)
        return setThreadError(0x161);
    if (object->kind == THREAD_KIND) {
        t = (thread *)object;
        if (t == threads.main)
            return setThreadError(0x15e);
        if (threads.schedulingOff && t == threads.current)
            return setThreadError(0x15f);
        if (!t->suspendCount && !t->signaled && threads.runnable == 1)
            return setThreadError(0x162);
        short self = t == threads.current;
        t->setSignaled(1);
        if (self) {
            /* Carry on on the next thread's stack, and delete this one's
               there. */
            dyingThread = t;
            schedule(currentTimeMs());
            abandonContext(&threads.current->context);
            delete dyingThread;
            resumeContext(&threads.current->context);
        }
    } else {
        thread *holder = object->owner();

        if (holder && holder != threads.current)
            return setThreadError(0x165);
    }
    setThreadError(0);
    delete object;
    return threads.error;
}

/* Lets another thread run, if one should. */
/* @zoombi32 0x0046e5a4 */
void reschedule(unsigned long now)
{
    thread *from;

    if (!threads.scheduling) {
        from = threads.current;
        if (schedule(now))
            switchContext(&threads.current->context, &from->context);
    }
}

/* @zoombi32 0x0046e5dc */
long currentThread()
{
    return syncHandle(threads.current);
}

/* @zoombi32 0x0046e5ed */
short threadError()
{
    return threads.error;
}

/* @zoombi32 0x0046e5f4 */
long mainThread()
{
    return syncHandle(threads.main);
}

/* @zoombi32 0x0046e605 */
unsigned short threadPriority(long handle)
{
    thread *t = (thread *)syncOf(handle, 0);

    if (!t || t->kind != THREAD_KIND) {
        setThreadError(0x161);
        return 0;
    }
    if (t->signaled) {
        setThreadError(0x166);
        return 0;
    }
    setThreadError(0);
    return t->priority;
}

/* Starts the threads: the main one is the caller; stacks for the others
   come from `stacks` to `end`. */
/* @zoombi32 0x0046e658 */
short initThreads(char *stacks, char *end)
{
    if (threads.initialized)
        return setThreadError(0x15e);
    memset(&threads, 0, sizeof(threads));
    threads.timeSlice = 20;
    stacks = (char *)((unsigned long)(stacks + 3) & ~3);
    end = (char *)((unsigned long)end & ~3);
    threads.stacks = (long *)stacks;
    threads.stacksEnd = (long *)end;
    *threads.stacks = end - stacks;
    if ((threads.main = new thread) != 0)
        initContext(&threads.main->context, 0, 0, 0);
    else
        return threads.error;
    threads.main->setSignaled(0);
    threads.current = threads.main;
    initLock(&threads.lock, 0);
    threads.initialized = 1;
    return setThreadError(0);
}

/* The time-slice timer: switches threads when the slice is up (or an
   urgent thread may be waiting). */
/* @zoombi32 0x0046e71a */
void timesliceProc(long, long)
{
    unsigned long now = currentTimeMs();

    if (threads.urgent || threads.current->sliceStart + threads.timeSlice <= now)
        reschedule(now);
}

/* @zoombi32 0x0046e749 */
void stopThreads()
{
    enterLock(&threads.lock);
    while (threads.objects)
        delete threads.objects;
    removeLock(&threads.lock);
    threads.initialized = 0;
}

/* @zoombi32 0x0046e78c */
void releaseMutex(long handle)
{
    mutex *m = (mutex *)syncOf(handle, 0);

    if (!m || m->kind != MUTEX_KIND) {
        setThreadError(0x161);
        return;
    }
    if (m->owner() != threads.current) {
        setThreadError(0x163);
        return;
    }
    if (!m->count) {
        setThreadError(0x164);
        return;
    }
    m->setSignaled(1);
    setThreadError(0);
}

/* @zoombi32 0x0046e7fd */
void resetEvent(long handle)
{
    event *e = (event *)syncOf(handle, 0);

    if (!e || e->kind != EVENT_KIND) {
        setThreadError(0x161);
        return;
    }
    deferCall(&threads.lock, &e->resetCall);
    setThreadError(0);
}

/* @zoombi32 0x0046e842 */
void resetEventCall(void *e)
{
    ((sync *)e)->setSignaled(0);
}

/* @zoombi32 0x0046e857 */
void resumeThread(long handle)
{
    thread *t = (thread *)syncOf(handle, 0);

    if (!t || t->kind != THREAD_KIND) {
        setThreadError(0x161);
        return;
    }
    if (!t->suspendCount) {
        setThreadError(0x164);
        return;
    }
    if (t->signaled) {
        setThreadError(0x166);
        return;
    }
    if (!--t->suspendCount) {
        threads.runnable++;
        if (t->priority >= 2) {
            threads.urgent++;
            setTimerInterval(threads.timer, -1);
        }
    }
    setThreadError(0);
}

/* Where a thread's procedure returns to: ends the thread. */
/* @zoombi32 0x0046e8e0 */
void threadExit()
{
    thread *t = threads.current;

    t->setSignaled(1);
    schedule(currentTimeMs());
    resumeContext(&threads.current->context);
}

/*
 * Picks the thread to run: round the ring from the current one, the first
 * of the highest priority that isn't sleeping, suspended or waiting (ending
 * waits that have timed out), preferring the one that has waited longest
 * since its slice; if none can run, idles until one can. Keeps the current
 * thread while its slice lasts unless one of higher priority can run.
 * Returns whether it changed threads.
 */
/* @zoombi32 0x0046e90e */
short schedule(unsigned long now)
{
    thread *t = threads.current;
    thread *best;

    if (!threads.scheduling++
        && (now >= t->sliceEnd || t->suspendCount > 0 || t->waitingOn
            || t->priority < 2 && threads.urgent > 0)) {
        best = 0;
        for (;;) {
            do {
                if (!threads.schedulingOff)
                    t = t->next;
                enterLock(&threads.lock);
                if (t->waitingOn && t->waitUntil && now > t->waitUntil)
                    t->waitingOn->dequeue(t, 0x12e);
                leaveLock(&threads.lock);
                if (now >= t->wakeTime && !t->suspendCount && !t->waitingOn
                    && (!best || best->priority < t->priority
                        || best->priority == t->priority && best->sliceStart > t->sliceStart))
                    best = t;
                else
                    t->sliceEnd = 0;
            } while (t != threads.current);
            if (best) {
                short switched = best != threads.current;

                if (switched) {
                    if (now < threads.current->sliceEnd
                        && threads.current->priority == best->priority) {
                        threads.scheduling--;
                        return 0;
                    }
                    threads.current->sliceEnd = 0;
                }
                threads.current = best;
                best->sliceStart = now;
                best->sliceEnd = now + threads.timeSlice;
                best->wakeTime = 0;
                threads.scheduling--;
                return switched;
            }
            threads.current = 0;
            osIdle();
            threads.current = t;
            now = currentTimeMs();
        }
    }
    threads.scheduling--;
    return 0;
}

/* @zoombi32 0x0046ea83 */
void setEvent(long handle)
{
    event *e = (event *)syncOf(handle, 0);

    if (!e || e->kind != EVENT_KIND) {
        setThreadError(0x161);
        return;
    }
    deferCall(&threads.lock, &e->setCall);
    setThreadError(0);
}

/* @zoombi32 0x0046eac8 */
void setEventCall(void *e)
{
    ((sync *)e)->setSignaled(1);
}

/* @zoombi32 0x0046eadd */
void setThreadPriority(long handle, unsigned short priority)
{
    thread *t = (thread *)syncOf(handle, 0);

    if (!t || t->kind != THREAD_KIND) {
        setThreadError(0x161);
        return;
    }
    if (t->signaled) {
        setThreadError(0x166);
        return;
    }
    if (priority != t->priority) {
        if (!t->suspendCount) {
            if (t->priority >= 2 && priority < 2 && !--threads.urgent)
                setTimerInterval(threads.timer, threads.timeSlice);
            else if (t->priority < 2 && priority >= 2 && !threads.urgent++)
                setTimerInterval(threads.timer, -1);
        }
        t->priority = priority;
    }
    setThreadError(0);
}

/* Lets other threads run for at least `ms`. */
/* @zoombi32 0x0046eb9f */
void yieldThread(long ms)
{
    thread *t = threads.current;

    if (t) {
        unsigned long now = currentTimeMs();

        t->sliceEnd = 0;
        t->wakeTime = ms + now;
        reschedule(now);
    }
}

/* @zoombi32 0x0046ebca */
void suspendThread(long handle)
{
    thread *t = (thread *)syncOf(handle, 0);

    if (!t || t->kind != THREAD_KIND) {
        setThreadError(0x161);
        return;
    }
    if (threads.schedulingOff && t == threads.current) {
        setThreadError(0x15f);
        return;
    }
    if (t->suspendCount == 0xffff) {
        setThreadError(0x164);
        return;
    }
    if (t->signaled) {
        setThreadError(0x166);
        return;
    }
    if (!t->suspendCount) {
        if (threads.runnable == 1) {
            setThreadError(0x162);
            return;
        }
        threads.runnable--;
        if (t->priority >= 2 && !--threads.urgent)
            setTimerInterval(threads.timer, threads.timeSlice);
    }
    t->suspendCount++;
    if (t == threads.current)
        reschedule(currentTimeMs());
    setThreadError(0);
}

/* @zoombi32 0x0046ecb2 */
short waitSync(long handle, long timeout)
{
    sync *object = syncOf(handle, 0);

    if (!object)
        return setThreadError(0x161);
    return object->wait(threads.current, timeout);
}

/* @zoombi32 0x0046ece8 */
__cdecl event::event()
{
    kind = EVENT_KIND;
    resetCall.proc = resetEventCall;
    resetCall.data = this;
    setCall.proc = setEventCall;
    setCall.data = this;
}

/* @zoombi32 0x0046ed1c */
void __cdecl event::setSignaled(unsigned short set)
{
    enterLock(&threads.lock);
    sync::setSignaled(set);
    leaveLock(&threads.lock);
}

/* @zoombi32 0x0046ed4a */
__cdecl mutex::mutex()
{
    kind = MUTEX_KIND;
}

/* @zoombi32 0x0046ed6a */
__cdecl mutex::~mutex()
{
    if (!signaled)
        detach(holder);
}

/* @zoombi32 0x0046eda9 */
void __cdecl mutex::attach(thread *owner)
{
    if (owner->mutexes) {
        next = owner->mutexes;
        prev = next->prev;
        prev->next = this;
        next->prev = this;
    } else {
        next = this;
        prev = this;
        owner->mutexes = this;
    }
    holder = owner;
}

/* @zoombi32 0x0046ede3 */
thread *__cdecl mutex::owner()
{
    return signaled ? 0 : holder;
}

/* @zoombi32 0x0046edfb */
void __cdecl mutex::detach(thread *owner)
{
    if (this == next) {
        owner->mutexes = 0;
    } else {
        prev->next = next;
        next->prev = prev;
        if (this == owner->mutexes)
            owner->mutexes = next;
    }
    holder = 0;
}

/* Acquired by the current thread (0), or released (1): the first waiter
   gets it once its holder has released it as often as it acquired it. */
/* @zoombi32 0x0046ee36 */
void __cdecl mutex::setSignaled(unsigned short free)
{
    if (free) {
        if (!signaled && !--count) {
            signaled = 1;
            detach(holder);
            if (waiters)
                acquire(waiters);
        }
    } else {
        acquire(threads.current);
    }
}

/* Waits, unless that would deadlock: with scheduling off, or when the
   holder is waiting for something the waiter holds. */
/* @zoombi32 0x0046ee91 */
short __cdecl mutex::wait(thread *waiter, unsigned long timeout)
{
    if (waiter == owner() && count == 0xffff)
        return setThreadError(0x164);
    if (isBlocked(waiter)) {
        if (threads.schedulingOff > 0) {
            if (timeout == -1)
                return setThreadError(0x160);
            return setThreadError(0x12e);
        }
        if (holder->waitingOn && waiter == holder->waitingOn->owner()) {
            if (timeout == -1)
                return setThreadError(0x160);
            return setThreadError(0x12e);
        }
    }
    return sync::wait(waiter, timeout);
}

/* @zoombi32 0x0046ef41 */
void __cdecl mutex::acquire(thread *waiter)
{
    if (this == waiter->waitingOn)
        dequeue(waiter, 0);
    if (signaled) {
        signaled = 0;
        attach(waiter);
        count = 1;
    } else {
        count++;
    }
}

/* @zoombi32 0x0046ef84 */
short __cdecl mutex::isBlocked(thread *waiter)
{
    return !signaled && holder != waiter;
}

/* @zoombi32 0x0046efa4 */
__cdecl sync::sync()
{
    tag = SYNC_TAG;
    signaled = 1;
    prev = 0;
    if ((next = threads.objects) != 0)
        next->prev = this;
    threads.objects = this;
}

/* Ends every wait (with 300) and forgets the object. */
/* @zoombi32 0x0046efdc */
__cdecl sync::~sync()
{
    tag = 0;
    while (waiters)
        dequeue(waiters, 300);
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        threads.objects = next;
}

/* @zoombi32 0x0046f043 */
void *__cdecl sync::operator new(size_t size)
{
    void *block;

    if ((block = localAlloc(size)) == 0) {
        setThreadError(localMemError());
        return 0;
    }
    setThreadError(0);
    memset(block, 0, size);
    return block;
}

/* @zoombi32 0x0046f07f */
void __cdecl sync::operator delete(void *block)
{
    localFree(block);
}

/* @zoombi32 0x0046f08c */
thread *__cdecl sync::owner()
{
    return 0;
}

/* @zoombi32 0x0046f093 */
void __cdecl sync::setSignaled(unsigned short set)
{
    if ((signaled = set) != 0)
        while (waiters)
            acquire(waiters);
}

/* @zoombi32 0x0046f0be */
short __cdecl sync::wait(thread *waiter, unsigned long timeout)
{
    unsigned long now;

    enterLock(&threads.lock);
    if (!isBlocked(waiter)) {
        acquire(waiter);
        leaveLock(&threads.lock);
        return 0;
    }
    now = currentTimeMs();
    waiter->waitUntil = timeout == -1 ? 0 : timeout + now;
    waiter->waitResult = 0;
    enqueue(waiter);
    leaveLock(&threads.lock);
    reschedule(now);
    return setThreadError(waiter->waitResult);
}

/* @zoombi32 0x0046f146 */
void __cdecl sync::acquire(thread *waiter)
{
    if (this == waiter->waitingOn)
        dequeue(waiter, 0);
}

/* @zoombi32 0x0046f162 */
void __cdecl sync::enqueue(thread *waiter)
{
    if (waiters) {
        waiter->waitNext = waiters;
        waiter->waitPrev = waiters->waitPrev;
        waiters->waitPrev->waitNext = waiter;
        waiters->waitPrev = waiter;
    } else {
        waiter->waitNext = waiter;
        waiter->waitPrev = waiter;
        waiters = waiter;
    }
    waiter->waitingOn = this;
}

/* @zoombi32 0x0046f19f */
short __cdecl sync::isBlocked(thread *)
{
    return !signaled;
}

/* @zoombi32 0x0046f1b2 */
void __cdecl sync::dequeue(thread *waiter, short result)
{
    if (waiter == waiter->waitNext) {
        waiters = 0;
    } else {
        waiter->waitPrev->waitNext = waiter->waitNext;
        waiter->waitNext->waitPrev = waiter->waitPrev;
        if (waiter == waiters)
            waiters = waiter->waitNext;
    }
    waiter->waitingOn = 0;
    waiter->waitResult = result;
}

/* @zoombi32 0x0046f1f5 */
__cdecl thread::thread()
{
    kind = THREAD_KIND;
    priority = 1;
}

/* @zoombi32 0x0046f21b */
__cdecl thread::~thread()
{
    setSignaled(1);
    freeContext(&context);
}

/*
 * Starts (0) or ends (1) the thread: joins or leaves the ring of threads
 * (the time-slice timer runs while there are two or more), releases its
 * mutexes and ends its wait, and keeps the counts of runnable and urgent
 * threads.
 */
/* @zoombi32 0x0046f25b */
void __cdecl thread::setSignaled(unsigned short ended)
{
    if (ended == signaled)
        return;
    if (ended) {
        if (this == next) {
            threads.ring = 0;
        } else {
            prev->next = next;
            next->prev = prev;
            if (this == threads.ring)
                threads.ring = next;
            if (this == threads.current)
                threads.current = next;
            if (threads.ring->next == threads.ring) {
                deleteTimer(threads.timer);
                threads.timer = 0;
            }
        }
        while (mutexes)
            mutexes->setSignaled(1);
        enterLock(&threads.lock);
        if (waitingOn)
            waitingOn->dequeue(this, 0);
        leaveLock(&threads.lock);
    } else if ((next = threads.ring) != 0) {
        if (threads.ring->next == threads.ring)
            threads.timer = newTimer(timesliceProc, 0, threads.timeSlice);
        prev = next->prev;
        next->prev = this;
        prev->next = this;
    } else {
        next = this;
        prev = this;
        threads.ring = this;
    }
    if (!suspendCount) {
        threads.runnable += ended ? 0xffff : 1; /* one less, or one more */
        if (priority >= 2) {
            if (ended && !--threads.urgent && threads.timer)
                setTimerInterval(threads.timer, threads.timeSlice);
            else if (!ended && !threads.urgent++ && threads.timer)
                setTimerInterval(threads.timer, -1);
        }
    }
    sync::setSignaled(ended);
}

/* Waits for the thread to end. */
/* @zoombi32 0x0046f3cd */
short __cdecl thread::wait(thread *waiter, unsigned long timeout)
{
    if (this == waiter || this == threads.main)
        return setThreadError(0x15e);
    if (isBlocked(waiter) && threads.schedulingOff > 0) {
        if (timeout == -1)
            return setThreadError(0x160);
        return setThreadError(0x12e);
    }
    return sync::wait(waiter, timeout);
}

/* @zoombi32 0x0046f43a */
long __cdecl syncHandle(sync *object)
{
    return (long)object;
}

/* The sync object a handle is, if it is one (of the kind, unless 0). */
/* @zoombi32 0x0046f442 */
sync *__cdecl syncOf(long handle, long kind)
{
    sync *object = (sync *)handle;

    if (!object || object->tag != SYNC_TAG || kind && kind != object->kind)
        return 0;
    return object;
}

/* @zoombi32-implicit 0x0046f599 event::~event */
