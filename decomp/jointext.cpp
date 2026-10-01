/*
 * jointext (0x413c24-0x413dc0): error messages joined from parts, and reported
 */

#include <string.h>
#include "zoombinis.h"
#include "jointext.h"
#include "loading.h"
#include "platform.h"

char allocationFailed;
char outOfMemory;
char portFailed;
char loadFailed;
char joinedText[0x100];
short reportingJoinedError;

/* Joins two texts into *joined (a JoinNode) unless it's already set. */
/* @zoombi32 0x00413c24 */
void joinText(char **joined, const char *first, const char *second)
{
    JoinNode *join;

    if (!*joined) {
        if (!allocateBlock((void **)&join, sizeof(JoinNode)))
            allocationFailed = 0;
        else {
            join->tag = 0xff;
            join->first = first;
            join->second = second;
        }
        *joined = (char *)join;
    }
}

/* @zoombi32 0x00413c6d */
void freeText(void **block)
{
    freeAndClear(block);
}

/* Formats a joined text into joinedText, its parts separated by spaces.
   The original hands formatTextV the array of parts as the va_list (Borland's is
   a pointer to the arguments); where a va_list is something else, the parts are
   passed as the arguments (the unused ones are ignored), so this is functional,
   not byte-exact. */
/* @zoombi32-functional 0x00413c7c */
void __cdecl formatJoined(const char *text)
{
    const char *parts[10];
    char format[0x20];
    short count;
    short i;

    parts[0] = emptyString;
    count = collectParts(0, parts, text);
    strcpy(format, "%s");
    for (i = 1; i < count; i++)
        strcpy(&format[i * 3 - 1], " %s");
    for (i = count; i < 10; i++)
        parts[i] = emptyString;
    formatText(0x100, joinedText, format, parts[0], parts[1], parts[2], parts[3], parts[4], parts[5],
               parts[6], parts[7], parts[8], parts[9]);
}

/*
 * Adds a text's strings (at most 10 in all) to `parts`, from `count`; the
 * new count.
 *
 * Not exact: the original keeps count and text in eax and edx and gives parts
 * and node esi and ebx; every form tried puts count in a saved register.
 */
/* @zoombi32 0x00413cf1 */
short collectParts(short count, const char **parts, const char *text)
{
    JoinNode *node;

    if (count < 10 && text) {
        node = (JoinNode *)text;
        if (node->tag >= 0)
            parts[count++] = text;
        else
            return collectParts(collectParts(count, parts, node->first), parts, node->second);
    }
    return count;
}

/* Reports a joined error message (once), as the kind of failure flagged. */
/* @zoombi32 0x00413d33 */
void reportJoinedError(char *message)
{
    if (!reportingJoinedError) {
        reportingJoinedError = 1;
        formatJoined(message);
        if (outOfMemory)
            notEnoughMemory(joinedText);
        else if (allocationFailed)
            notEnoughNearMemory(joinedText);
        else if (portFailed)
            unableToAllocatePort(joinedText);
        else if (loadFailed)
            unableToLoad(joinedText);
        else
            fatalError(joinedText);
    }
}
