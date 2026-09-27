/*
 * module_4800c4 (Mohawk engine): settings files (MOHAWK.INI), read into memory
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

IniState iniState;

/*
 * Opens a settings file (iniState.path without one), reading it into memory, or
 * finds it among the open ones. Its handle, or 0 (the error in
 * iniState.error).
 *
 * Not exact: the original keeps `end` in a saved register (edi); this keeps
 * it in eax.
 */
/* @zoombi32 0x004800c4 */
short openIni(fileSpec *path)
{
    char *text;
    long file;
    unsigned long size;
    fileSpec *name;
    short handle;
    IniFile *ini;
    short error;
    register char *end;

    name = path;
    if (!name)
        name = &iniState.path;
    for (handle = iniState.files; handle; handle = ini->next) {
        ini = (IniFile *)lockHandle(handle);
        error = name->compare(ini->path);
        if (!error) {
            if (handleSize(ini->text) > 0) {
                unlockHandle(handle);
                setIniError(0);
                return handle;
            }
            unlockHandle(handle);
            closeIni(handle);
            break;
        }
        if (error != 0x2844) {
            unlockHandle(handle);
            goto fail;
        }
        unlockHandle(handle);
    }
    size = fileSize(name);
    if (size == -1) {
        error = fileError();
        if (error == 0x2845)
            error = 0x2968;
        goto fail;
    }
    if (size >= 0x10000) {
        error = 0x296c;
        goto fail;
    }
    if ((handle = newHandle(sizeof(IniFile))) == 0) {
        error = memError();
        goto fail;
    }
    ini = (IniFile *)lockHandle(handle);
    memset(ini, 0, sizeof(IniFile));
    ini->users = 1;
    if ((ini->text = newHandle(size + 1)) == 0) {
        error = memError();
        goto freeHeader;
    }
    text = (char *)lockHandle(ini->text);
    file = openFile(name, 1);
    if (!file) {
        error = fileError();
        goto freeText;
    }
    error = readFile(file, text, (long *)&size);
    closeFile(file, 0);
    if (error) {
    freeText:
        unlockHandle(ini->text);
        disposeHandle(ini->text);
    freeHeader:
        unlockHandle(handle);
        disposeHandle(handle);
    fail:
        setIniError(error);
        return 0;
    }
    ini->size = size;
    text[ini->size] = 0;
    if (strchr(text, 0) < text + ini->size) {
        error = 0x2969;
        goto freeText;
    }
    end = strpbrk(text, "\x1a");
    if (end) {
        switch (*end) {
        case 0x1a:
            if (end < text + ini->size - 1) {
                error = 0x2969;
                goto freeText;
            }
            *end = 0;
            ini->size--;
        }
    }
    ini->prev = 0;
    if ((ini->next = iniState.files) != 0)
        ((IniFile *)handleData(iniState.files))->prev = handle;
    iniState.files = handle;
    ini->path = *name;
    unlockHandle(ini->text);
    fn_48f464(ini->text, 1);
    unlockHandle(handle);
    setIniError(0);
    return handle;
}

/* Where the line holding the parse position starts. */
/* @zoombi32 0x00480330 */
unsigned short lineStart(IniFile *ini, char *text)
{
    unsigned short pos;

    for (pos = ini->pos; pos && text[pos - 1] != '\r' && text[pos - 1] != '\n'; pos--)
        ;
    return pos;
}

/* Where the line after the parse position starts. Not exact: the original
   keeps `end` in ebx (text in edi, ini in esi); this keeps it in eax. */
/* @zoombi32 0x00480363 */
unsigned short nextLine(IniFile *ini, char *text)
{
    register char *end;

    end = (char *)memchr(text + ini->pos, '\r', ini->size - ini->pos);
    if (end) {
        end++;
        if (*end == '\n')
            end++;
    }
    if (end)
        return end - text;
    return ini->size;
}

/* @zoombi32 0x004803b0 */
void closeIni(short handle)
{
    IniFile *ini;

    ini = (IniFile *)lockHandle(handle);
    if (ini->prev)
        ((IniFile *)handleData(ini->prev))->next = ini->next;
    else
        iniState.files = ini->next;
    if (ini->next)
        ((IniFile *)handleData(ini->next))->prev = ini->prev;
    ini->path.~fileSpec();
    disposeHandle(ini->text);
    unlockHandle(handle);
    disposeHandle(handle);
}

/*
 * Moves on to the next "key=" line of the section (skipping comments),
 * leaving the parse position at the key. The key's length, or 0 at the
 * section's end.
 *
 * Not exact: the original keeps `equals` in edi and `next` on the stack;
 * this does the opposite.
 */
/* @zoombi32 0x00480424 */
unsigned short nextKey(IniFile *ini, char *text)
{
    unsigned short next;
    char *equals;

    do {
        next = nextLine(ini, text);
        ini->pos = skipSpaces(ini, text);
        switch (text[ini->pos]) {
        case ';':
            break;
        case '[':
            return 0;
        default:
            equals = (char *)memchr(text + ini->pos, '=', ini->size - ini->pos);
            if (equals && (unsigned short)(equals - text) < next) {
                while (isSpace(*--equals))
                    ;
                return equals + 1 - (text + ini->pos);
            }
        }
    } while ((ini->pos = next) < ini->size);
    return 0;
}

/* Moves on to the next "[section]" line, leaving the parse position at the
   name. The name's length, or 0 at the end. */
/* @zoombi32 0x004804bb */
unsigned short nextSection(IniFile *ini, char *text)
{
    char *line;
    char *close;
    unsigned short start;
    unsigned short next;

    do {
        start = skipSpaces(ini, text);
        next = nextLine(ini, text);
        line = text + start;
        if (*line == '[' && (close = (char *)memchr(line, ']', ini->size - start)) != 0
            && (next == ini->size || next > (unsigned short)(close - text))) {
            ini->pos = start + 1;
            return close - (line + 1);
        }
    } while ((ini->pos = next) < ini->size);
    return 0;
}

/* Finds a key in a section, leaving the parse position after its '='. */
/* @zoombi32 0x0048054c */
short findKey(IniFile *ini, char *text, const char *section, const char *key)
{
    unsigned short length;
    unsigned short keyLength;

    if (!findSection(ini, text, section))
        return 0;
    keyLength = strlen(key);
    ini->pos = nextLine(ini, text);
    while ((length = nextKey(ini, text)) != 0) {
        if (keyLength == length && !memicmp(key, text + ini->pos, keyLength)) {
            ini->pos += keyLength;
            while (text[ini->pos++] != '=')
                ;
            return 1;
        }
        ini->pos = nextLine(ini, text);
    }
    return 0;
}

/* @zoombi32 0x004805e7 */
short findSection(IniFile *ini, char *text, const char *section)
{
    unsigned short length;
    unsigned short found;

    length = strlen(section);
    ini->pos = 0;
    while ((found = nextSection(ini, text)) != 0)
        if (length == found && !memicmp(section, text + ini->pos, length))
            return 1;
    return 0;
}

/* Finds the settings file: MOHAWK.W32 in the program's directory, else
   MOHAWK.INI. */
/* @zoombi32 0x00480642 */
short initIni()
{
    memset(&iniState, 0, sizeof(iniState));
    fileSpec directory;
    programDirectory(&directory);
    if (fileMissing(iniState.path = fileSpec(directory, "MOHAWK.W32")))
        iniState.path = fileSpec(directory, "MOHAWK.INI");
    iniState.ready = 1;
    return setIniError(0);
}

/* How big a buffer settings need (0 before initIni). */
/* @zoombi32 0x004806ff */
short iniBufferSize()
{
    return iniState.ready ? 0x500 : 0;
}

/* @zoombi32 0x00480713 */
void closeAllIni()
{
    while (iniState.files)
        closeIni(iniState.files);
    iniState.ready = 0;
}

/* Skips spaces and tabs from the parse position; where they end. */
/* @zoombi32 0x00480738 */
unsigned short skipSpaces(IniFile *ini, char *text)
{
    char *p;

    p = text + ini->pos;
    while (isSpace(*p))
        p++;
    return p - text;
}
