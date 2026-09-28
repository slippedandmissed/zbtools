/*
 * os_46f5c0's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_46F5C0_H
#define OS_46F5C0_H

short initContext(Context *context, void (*proc)(long), long argument,
                  unsigned short stackSize); /* 0x46f5c0 */
short freeContext(Context *context); /* 0x46f68f */
void resumeContext(Context *context); /* 0x46f6c9 */
void abandonContext(Context *context); /* 0x46f6f9 */
void switchContext(Context *to, Context *save); /* 0x46f70e */
void fn_46f74f(thread *to); /* 0x46f74f */
void recordReturn(Context *context, unsigned short depth); /* 0x46f771 */
short setThreadError(short error); /* 0x46f78e */

#endif
