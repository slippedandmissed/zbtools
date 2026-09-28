/*
 * jointext's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef JOINTEXT_H
#define JOINTEXT_H

extern char portFailed; /* @data 0x4aa4ca: "Unable to allocate port for" */
extern char joinedText[0x100]; /* @data 0x4aa4cc */
extern short reportingJoinedError; /* @data 0x4aa5cc */
/* Joins two strings into a new block at *joined. */
void joinText(char **joined, const char *first, const char *second);
void reportJoinedError(char *message);
void __cdecl formatJoined(const char *text);
short collectParts(short count, const char **parts, const char *text);
void freeText(void **block);

#endif
