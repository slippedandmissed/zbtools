/*
 * loading's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef LOADING_H
#define LOADING_H

extern char g_4a07a8[2]; /* a one-character string (the second byte is an empty one) */
extern Callback fatalHook; /* called before a fatal error is reported */
/* reports an error (showError, as the game sets it up) */
extern void (*errorReporter)(const char *prefix, const char *format, va_list args); /* @data 0x4a07b0 */
extern va_list formatArgs; /* @data 0x4ab40c: formatString's arguments */
extern short lockedCount; /* @data 0x4ab410 */
extern long lockedResources[10]; /* @data 0x4ab414: resources %L locked */
extern char *lockedData[10]; /* @data 0x4ab43c */
/* extra conversions for formatString: whether a character starts one, and its text */
extern short (*isFormatCharacter)(char c); /* @data 0x4ab464 */
extern char *(*formatCharacter)(char c); /* @data 0x4ab468 */
extern char numberText[]; /* @data 0x4ab46c */
extern short reportingError; /* @data 0x4ab478 */
/* Reports an error, printf-style. */
void __cdecl fatalError(const char *format, ...);
/* Formats into `buffer` (of `size` bytes), printf-style. */
void __cdecl fn_4150c7(long size, char *buffer, const char *format, ...);
/* loading */
unsigned short loadMidi(short key);
void unloadMidi(short key);
void unloadMidiNow(short key);
short playMidiOn(short key, short channel);
void stopMidi(unsigned short id);
void endMidiLoops(unsigned short id);
short isMidiPlaying(unsigned short id);
short playMidi(short key, short channel, short eventType, short discard);
short waitForMidi(unsigned short id, short eventType, short discard);
short awaitMidi(unsigned short id, short eventType, short discard);
short midiPlayingOrStop(unsigned short id, short stop);
short midiValueReached(char value);
short waitForMidiValue(char value, short eventType, short discard);
short awaitMidiValue(char value, short eventType, short discard);
void stopAllMidi();
void setFormatCharacters(short (*isSpecial)(char c), char *(*text)(char c));
char *__cdecl formatText(long size, char *text, const char *format, ...);
char *formatTextV(long size, char *text, const char *format, va_list args);
char *formatString(long size, char *text, const char *format);
void formatArgument(long size, char **text, const char **format);
void __cdecl warning(const char *format, ...);
void __cdecl unableToLoad(const char *format, ...);
void __cdecl notEnoughMemory(const char *format, ...);
void __cdecl notEnoughNearMemory(const char *format, ...);
void __cdecl unableToAllocatePort(const char *format, ...);
void reportFatalError(const char *prefix, const char *format, va_list args);
void setFatalHook(Callback callback);
void setErrorReporter(void (*reporter)(const char *prefix, const char *format, va_list args));
void setUsualFatalMessage(const char *message);

#endif
