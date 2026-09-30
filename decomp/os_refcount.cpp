/*
 * os_refcount (0x46d7f8-0x46d95c): locks that defer calls (DeferLock)
 */

/* The Mohawk OS layer was compiled with stack frames (no -k-). */
/* @flags -p */

#include "zoombinis.h"
#include "os_manager.h"
#include "os_refcount.h"

DeferLock *locks = 0;

/* Drops the calls waiting on a lock. */
/* Not exact: the original computes &lock->queue in ebx (a saved register);
   BCC32 4.5 uses eax. */
/* @zoombi32 0x0046d7f8 */
void __cdecl clearLock(DeferLock *lock)
{
    Deferred *next = (Deferred *)atomicExchange((void **)&lock->queue, 0);
    Deferred *call;

    for (call = next; call; call = next) {
        next = call->next;
        call->queued = 0;
        call->count = 0;
    }
}

/* @zoombi32 0x0046d827 */
long __cdecl enterLock(DeferLock *object)
{
    return atomicIncrement((long *)&object->depth);
}

/* Releases a lock; the last release runs the calls that waited (in the order
   they were posted, each as many times as it was). */
/* @zoombi32 0x0046d838 */
void __cdecl leaveLock(DeferLock *lock)
{
    Deferred *queue;
    Deferred *call;
    Deferred *reversed;

    if (lock->depth > 1)
        atomicDecrement((long *)&lock->depth);
    else
        for (;;) {
            if ((queue = (Deferred *)atomicExchange((void **)&lock->queue, 0)) != 0) {
                reversed = 0;
                do {
                    call = queue;
                    queue = queue->next;
                    call->next = reversed;
                    reversed = call;
                } while (queue);
                do {
                    queue = call->next;
                    call->queued = 0;
                    do
                        call->proc(call->data);
                    while ((unsigned long)atomicDecrement((long *)&call->count) > 0 && !call->queued);
                    call = queue;
                } while (call);
            }
            if (lock->depth <= 0)
                break;
            if (!lock->queue)
                lock->depth = 0;
        }
}

/* @zoombi32 0x0046d8af */
void __cdecl initLock(DeferLock *lock, short listed)
{
    lock->depth = 0;
    lock->queue = 0;
    lock->next = 0;
    lock->prev = 0;
    if (listed) {
        if ((lock->next = locks) != 0)
            lock->next->prev = lock;
        locks = lock;
    }
}

/* @zoombi32 0x0046d8e8 */
void __cdecl removeLock(DeferLock *lock)
{
    if (lock->next)
        lock->next->prev = lock->prev;
    if (lock->prev)
        lock->prev->next = lock->next;
    else if (lock == locks)
        locks = lock->next;
}

/* Makes a call now, or if the lock is held, when it's released. */
/* @zoombi32 0x0046d91c */
void __cdecl deferCall(DeferLock *lock, Deferred *call)
{
    if (!lock->depth)
        call->proc(call->data);
    else if ((unsigned long)atomicIncrement((long *)&call->count) == 1) {
        call->queued = 1;
        call->next = (Deferred *)atomicExchange((void **)&lock->queue, call);
    }
}
