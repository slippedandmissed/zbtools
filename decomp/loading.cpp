/*
 * loading (0x414f30-0x415514): 'Unable to load ', 'Not enough memory for ', 'Unable to allocate port for '
 *
 * MIDI versions of the sound functions, a small printf-like formatter
 * (with %L for text in locked resources), and error reporting.
 */

#include <ctype.h>
#include <stdlib.h>
#include "zoombinis.h"
#include "e2memory.h"
#include "loading.h"
#include "nthstring.h"
#include "platform.h"
#include "sound.h"

#define MIDI RESOURCE_TYPE('t', 'M', 'I', 'D')

/* @zoombi32 0x00414f30 */
unsigned short loadMidi(short key)
{
    return loadSoundByKey(key, MIDI);
}

/* @zoombi32 0x00414f46 */
void unloadMidi(short key)
{
    unloadSound(key, MIDI);
}

/* @zoombi32 0x00414f5c */
void unloadMidiNow(short key)
{
    unloadSoundNow(key, MIDI);
}

/* @zoombi32 0x00414f72 */
short playMidiOn(short key, short channel)
{
    return playSoundOn(key, MIDI, channel);
}

/* @zoombi32 0x00414f8d */
void stopMidi(unsigned short id)
{
    stopSounds(id, MIDI);
}

/* @zoombi32 0x00414fa3 */
void endMidiLoops(unsigned short id)
{
    endSoundLoops(id, MIDI);
}

/* @zoombi32 0x00414fb9 */
short isMidiPlaying(unsigned short id)
{
    return isSoundPlaying(id, MIDI);
}

/* @zoombi32 0x00414fcf */
short playMidi(short key, short channel, short eventType, short discard)
{
    return playSound(key, MIDI, channel, eventType, discard);
}

/* @zoombi32 0x00414ff4 */
short waitForMidi(unsigned short id, short eventType, short discard)
{
    return waitForSound(id, MIDI, eventType, discard);
}

/* @zoombi32 0x00415014 */
short awaitMidi(unsigned short id, short eventType, short discard)
{
    return awaitSound(id, MIDI, eventType, discard);
}

/* @zoombi32 0x00415034 */
short midiPlayingOrStop(unsigned short id, short stop)
{
    return soundPlayingOrStop(id, MIDI, stop);
}

/* @zoombi32 0x0041504f */
short midiValueReached(char value)
{
    return soundValueReached(value, MIDI);
}

/* @zoombi32 0x00415064 */
short waitForMidiValue(char value, short eventType, short discard)
{
    return waitForSoundValue(value, MIDI, eventType, discard);
}

/* @zoombi32 0x00415083 */
short awaitMidiValue(char value, short eventType, short discard)
{
    return awaitSoundValue(value, MIDI, eventType, discard);
}

/* @zoombi32 0x004150a2 */
void stopAllMidi()
{
    resetSoundChannel(MIDI);
}

/* @zoombi32 0x004150b0 */
void setFormatCharacters(short (*isSpecial)(char c), char *(*text)(char c))
{
    isFormatCharacter = isSpecial;
    formatCharacter = text;
}

/* Formats into `text` (at most `size` bytes, with the terminator): see
   formatString. */
/* @zoombi32 0x004150c7 */
char *__cdecl formatText(long size, char *text, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    return formatTextV(size, text, format, args);
}

/* @zoombi32 0x004150de */
char *formatTextV(long size, char *text, const char *format, va_list args)
{
    char *end;
    short i;

    lockedCount = 0;
    formatArgs = args;
    end = formatString(size, text, format);
    for (i = 0; i < lockedCount; i++)
        unlockResource(lockedResources[i]);
    return end;
}

/*
 * Copies `format` into `text`, converting %d, %u, %s and %L (any case; see
 * formatArgument) and the characters isFormatCharacter picks out; `%` before
 * anything else just keeps that character. The end of the text.
 */
/* @zoombi32 0x00415129 */
char *formatString(long size, char *text, const char *format)
{
    const char *from;
    char *to;
    long n;

    from = format;
    to = text;
    n = 0;
    while (n < size - 1 && *from) {
        if (*from == '%') {
            switch (toupper(from[1])) {
            case 'D':
            case 'L':
            case 'S':
            case 'U':
                formatArgument(size - n, &to, &from);
                size -= to - text;
                return formatString(size, to, from);
            }
            from++;
            *to = *from;
            from++;
            to++;
            n++;
        } else if (isFormatCharacter && formatCharacter && isFormatCharacter(*from)) {
            formatArgument(size - n, &to, &from);
            size -= to - text;
            return formatString(size, to, from);
        } else {
            *to = *from;
            from++;
            to++;
            n++;
        }
    }
    *to = 0;
    return to;
}

/*
 * Formats the conversion at *format into *text, moving both on: %s a string,
 * %d an int, %u an unsigned long; %L locks a resource (%L0) or takes a
 * character's text from the nth one locked so far (%L1 to %L9, via
 * nthString). Other characters go through formatCharacter.
 */
/* @zoombi32 0x0041522a */
void formatArgument(long size, char **text, const char **format)
{
    short index;
    char *from;
    char which;

    if (**format == '%') {
        (*format)++;
        switch (**format) {
        case 'S':
        case 's':
            from = va_arg(formatArgs, char *);
            break;
        case 'D':
        case 'd':
            intToDecimal(va_arg(formatArgs, int), numberText);
            from = numberText;
            break;
        case 'U':
        case 'u':
            unsignedToDecimal(va_arg(formatArgs, unsigned long), numberText);
            from = numberText;
            break;
        case 'L':
        case 'l':
            from = g_4a07a8 + 1;
            (*format)++;
            index = **format - '1';
            if (index < 0) {
                if (lockedCount < 10) {
                    index = lockedCount++;
                    lockedResources[index] = va_arg(formatArgs, long);
                    lockedData[index] = lockResource(lockedResources[index]);
                }
            } else if (index < lockedCount) {
                which = va_arg(formatArgs, char);
                from = nthString(lockedData[index], which);
            }
            break;
        default:
            g_4a07a8[0] = **format;
            from = g_4a07a8;
        }
    } else
        from = formatCharacter(**format);
    (*format)++;
    *text = formatString(size, *text, from);
}

/* @zoombi32 0x004153b0 */
void setFatalHook(Callback callback)
{
    fatalHook = callback;
}

/* @zoombi32 0x004153bf */
void setErrorReporter(void (*reporter)(const char *prefix, const char *format, va_list args))
{
    errorReporter = reporter;
}

/* @zoombi32 0x004153ce */
void setUsualFatalMessage(const char *message)
{
    usualFatalMessage = message;
}

/* Shows an error without stopping (stopping in the debugger, in debug mode,
   unless it's the usual fatal message). */
/* @zoombi32 0x004153dd */
void __cdecl warning(const char *format, ...)
{
    va_list args;

    if (debugMode && format != usualFatalMessage)
        debugging = 1;
    va_start(args, format);
    showError(emptyString, format, args);
    debugging = 0;
}

/* Reports an error and quits. */
/* @zoombi32 0x0041541a */
void __cdecl fatalError(const char *format, ...)
{
    va_list args;

    if (debugMode && format != usualFatalMessage)
        debugging = 1;
    va_start(args, format);
    reportFatalError(0, format, args);
}

/* @zoombi32 0x0041544b */
void __cdecl unableToLoad(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    reportFatalError("Unable to load ", format, args);
}

/* @zoombi32 0x00415461 */
void __cdecl notEnoughMemory(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    reportFatalError("Not enough memory for ", format, args);
}

/* @zoombi32 0x00415477 */
void __cdecl notEnoughNearMemory(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    reportFatalError("Not enough near memory for ", format, args);
}

/* @zoombi32 0x0041548d */
void __cdecl unableToAllocatePort(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    reportFatalError("Unable to allocate port for ", format, args);
}

/* Reports a fatal error (once), through fatalHook and the error reporter,
   then quits. */
/* @zoombi32 0x004154a3 */
void reportFatalError(const char *prefix, const char *format, va_list args)
{
    if (!reportingError) {
        reportingError = 1;
        if (fatalHook)
            fatalHook();
        if (!format) {
            format = prefix;
            prefix = emptyString;
        } else if (!prefix)
            prefix = emptyString;
        if (format && errorReporter)
            errorReporter(prefix, format, args);
        releaseControlKeys();
        exit(0);
    }
}
