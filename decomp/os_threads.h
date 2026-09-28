/*
 * os_threads's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_THREADS_H
#define OS_THREADS_H

extern thread *dyingThread; /* @data 0x4b9d88 */
/* Threads (the OS layer) */
long createThread(void (*proc)(long), long argument, unsigned short stackSize,
                  unsigned short priority); /* 0x46e302: suspended */
/* Threads, events and mutexes are OS-layer sync objects, deleted, waited
   for (an event set, a mutex acquired) the same way. */
short deleteSync(long sync); /* 0x46e463 */
void resumeThread(long thread); /* 0x46e857 */
void yieldThread(long); /* 0x46eb9f */
void setThreadPriority(long thread, unsigned short priority); /* 0x46eadd */
unsigned short threadPriority(long thread); /* 0x46e605 */
long currentThread(); /* 0x46e5dc */
long mainThread(); /* 0x46e5f4 */
void stopOtherThreads(); /* 0x46e2a4 */
void reschedule(unsigned long now); /* 0x46e5a4 */
void timesliceProc(long timer, long data); /* 0x46e71a */
void threadExit(); /* 0x46e8e0: where a thread's procedure returns to */
short schedule(unsigned long now); /* 0x46e90e: whether it switched threads */
sync *__cdecl syncOf(long sync, long kind); /* 0x46f442: 0 if it isn't one (of the kind) */
void disableScheduling(); /* 0x46e410 */
void enableScheduling(); /* 0x46e43a */
short initThreads(char *stacks, char *end); /* 0x46e658 */
void stopThreads(); /* 0x46e749 */
void suspendThread(long thread); /* 0x46ebca */
long newEvent(short); /* 0x46e380 */
void setEvent(long event); /* 0x46ea83 */
void resetEvent(long event); /* 0x46e7fd */
short waitSync(long sync, long timeout); /* 0x46ecb2: 0 when set (or acquired); 0x12e timed out */
long newMutex(short); /* 0x46e3c8 */
void releaseMutex(long mutex); /* 0x46e78c */
short threadError(); /* the OS layer's last error */
void resetEventCall(void *event); /* 0x46e842 */
void setEventCall(void *event); /* 0x46eac8 */
long __cdecl syncHandle(sync *object); /* 0x46f43a */

#endif
