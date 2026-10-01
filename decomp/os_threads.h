/*
 * os_threads's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_THREADS_H
#define OS_THREADS_H

extern thread *dyingThread; /* @data 0x4b9d88 */
/* Threads (the OS layer) */
LONG_PTR createThread(void (*proc)(LONG_PTR), LONG_PTR argument, unsigned short stackSize,
                  unsigned short priority); /* 0x46e302: suspended */
/* Threads, events and mutexes are OS-layer sync objects, deleted, waited
   for (an event set, a mutex acquired) the same way. */
short deleteSync(LONG_PTR sync); /* 0x46e463 */
void resumeThread(LONG_PTR thread); /* 0x46e857 */
void yieldThread(long); /* 0x46eb9f */
void setThreadPriority(LONG_PTR thread, unsigned short priority); /* 0x46eadd */
unsigned short threadPriority(LONG_PTR thread); /* 0x46e605 */
LONG_PTR currentThread(); /* 0x46e5dc */
LONG_PTR mainThread(); /* 0x46e5f4 */
void stopOtherThreads(); /* 0x46e2a4 */
void reschedule(unsigned long now); /* 0x46e5a4 */
void timesliceProc(LONG_PTR timer, LONG_PTR data); /* 0x46e71a */
void threadExit(); /* 0x46e8e0: where a thread's procedure returns to */
short schedule(unsigned long now); /* 0x46e90e: whether it switched threads */
sync *__cdecl syncOf(LONG_PTR sync, long kind); /* 0x46f442: 0 if it isn't one (of the kind) */
void disableScheduling(); /* 0x46e410 */
void enableScheduling(); /* 0x46e43a */
short initThreads(char *stacks, char *end); /* 0x46e658 */
void stopThreads(); /* 0x46e749 */
void suspendThread(LONG_PTR thread); /* 0x46ebca */
LONG_PTR newEvent(short); /* 0x46e380 */
void setEvent(LONG_PTR event); /* 0x46ea83 */
void resetEvent(LONG_PTR event); /* 0x46e7fd */
short waitSync(LONG_PTR sync, long timeout); /* 0x46ecb2: 0 when set (or acquired); 0x12e timed out */
LONG_PTR newMutex(short); /* 0x46e3c8 */
void releaseMutex(LONG_PTR mutex); /* 0x46e78c */
short threadError(); /* the OS layer's last error */
void resetEventCall(void *event); /* 0x46e842 */
void setEventCall(void *event); /* 0x46eac8 */
LONG_PTR __cdecl syncHandle(sync *object); /* 0x46f43a */

#endif
