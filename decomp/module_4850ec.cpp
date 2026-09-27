/*
 * module_4850ec (Mohawk engine): file names (fileSpec)
 */

/* @flags -p -x- */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "zoombinis.h"

/* @zoombi32 0x004850ec */
__cdecl fileSpec::fileSpec()
{
    name = 0;
}

/* @zoombi32 0x004850f8 */
__cdecl fileSpec::fileSpec(const char *path)
{
    unsigned short length = path ? strlen(path) : 0;

    if (!length || length >= 0x100) {
        setFileError(0x2843);
        name = 0;
    } else if ((name = (FileName *)malloc(sizeof(FileName))) == 0)
        setFileError(0x2846);
    else {
        memset(name, 0, 9);
        name->references = 1;
        strcpy(name->path, path);
        if (canonicalize()) {
            free(name);
            name = 0;
        }
    }
}

/* @zoombi32 0x0048518c */
__cdecl fileSpec::fileSpec(const fileSpec &directory, const char *file)
{
    unsigned short length = directory.name ? strlen(directory.name->path) : 0;
    unsigned short fileLength = file ? strlen(file) : 0;

    if (!length || !fileLength || length + fileLength + 1 >= 0x100) {
        setFileError(0x2843);
        name = 0;
    } else if ((name = (FileName *)malloc(sizeof(FileName))) == 0)
        setFileError(0x2846);
    else {
        memset(name, 0, 9);
        name->references = 1;
        name->volume = directory.name->volume;
        memcpy(name->path, directory.name->path, length);
        if (name->path[length - 1] != '\\')
            name->path[length++] = '\\';
        strcpy(name->path + length, file);
        if (canonicalize()) {
            free(name);
            name = 0;
        }
    }
}

/* @zoombi32 0x00485291 */
__cdecl fileSpec::fileSpec(long volume, const char *path)
{
    unsigned short length = path ? strlen(path) : 0;

    if (!volumeOf(volume) || !length || length >= 0x100) {
        setFileError(0x2843);
        name = 0;
    } else if ((name = (FileName *)malloc(sizeof(FileName))) == 0)
        setFileError(0x2846);
    else {
        memset(name, 0, 9);
        name->references = 1;
        name->volume = volume;
        strcpy(name->path, path);
        if (canonicalize()) {
            free(name);
            name = 0;
        }
    }
}

/* @zoombi32 0x0048533e */
__cdecl fileSpec::~fileSpec()
{
    if (name && --name->references == 0)
        free(name);
}

/* @zoombi32 0x00485373 */
__cdecl fileSpec::fileSpec(const fileSpec &from)
{
    if ((name = from.name) != 0) {
        name->references++;
        setFileError(0);
    } else
        setFileError(0x2843);
}

/* @zoombi32 0x004853a2 */
fileSpec &__cdecl fileSpec::operator=(const fileSpec &from)
{
    if (name != from.name) {
        if (name && --name->references == 0)
            free(name);
        if ((name = from.name) != 0) {
            name->references++;
            setFileError(0);
        } else
            setFileError(0x2843);
    }
    return *this;
}

/* Not exact: the original keeps the result in edi (set to 0 before the
   comparison, then 0x2844 added); BCC32 4.5 computes it in eax. */
/* @zoombi32 0x004853f3 */
short __cdecl fileSpec::compare(const fileSpec &with) const
{
    short result;

    if (!name || !with.name)
        return setFileError(0x2843);
    if (name->volume == with.name->volume) {
        if (volumeOf(name->volume)->info.caseSensitive)
            result = strcmp(name->path, with.name->path) ? 0x2844 : 0;
        else
            result = stricmp(name->path, with.name->path) ? 0x2844 : 0;
    } else
        result = 0x2844;
    return setFileError(result);
}

/* @zoombi32 0x00485485 */
short __cdecl fileSpec::fileName(char *file) const
{
    if (!name)
        return setFileError(0x2843);
    strcpy(file, strrchr(name->path, '\\') + 1);
    return setFileError(0);
}

/* @zoombi32 0x004854c5 */
short __cdecl fileSpec::getPath(char *path) const
{
    if (!name)
        return setFileError(0x2843);
    volumeOf(name->volume)->rootPath(path);
    strcat(path, name->path + 1);
    return setFileError(0);
}

/* Not exact: the original keeps `this` in ebx and `path` in esi; BCC32 4.5
   swaps them. */
/* @zoombi32 0x0048550c */
short __cdecl fileSpec::directory(char *path) const
{
    if (!name)
        return setFileError(0x2843);
    char *last = strrchr(name->path, '\\');
    if (last == name->path)
        strcpy(path, "\\");
    else {
        unsigned short length = last - name->path;
        memcpy(path, name->path, length);
        path[length] = 0;
    }
    for (; *path; path++)
        if (*path == '\\')
            *path = '/';
    return setFileError(0);
}

/* @zoombi32 0x00485596 */
short __cdecl fileSpec::volume(long *volume) const
{
    if (!name)
        return setFileError(0x2843);
    *volume = name->volume;
    return setFileError(0);
}

/* @zoombi32 0x004855c0 */
short __cdecl fileSpec::locate(long *volume, char *path) const
{
    if (this->volume(volume) || getPath(path))
        return files.error;
    return volumeOf(*volume)->mount();
}

/* @zoombi32 0x00485604 */
short __cdecl fileSpec::canonicalize()
{
    Drive *drive;
    char *first;
    char *second;
    Volume *volume;
    char c;
    char *found;
    unsigned short length;
    char unc[0x102];
    DiskInfo info;
    char upper[0x100];
    char root[0x100];
    char *p;
    long at;

    if (!name->path[0])
        return setFileError(0x2843);
    for (p = name->path; *p; p++)
        switch (*p) {
        case '/':
            *p = '\\';
        }
    if (name->path[1] == ':') {
        long number = driveNumber(name->path[0]);
        if (name->volume || (drive = driveAt(number, 1)) == 0)
            return setFileError(0x2843);
        if ((name->volume = drive->volume) == 0 && (name->volume = mountDrive(number)) == 0
            && (name->volume = drive->volume) == 0)
            return files.error;
        at = 2;
    } else if (name->path[0] == '\\' && name->path[1] == '\\') {
        strcpy(unc, name->path);
        if (name->volume || (first = strchr(unc + 2, '\\')) == 0
            || (second = strchr(first + 1, '\\')) == 0)
            return setFileError(0x2843);
        second[1] = 0;
        if (readDiskInfo(unc, &info))
            return files.error;
        *first = 0;
        if ((name->volume = findVolume(&info, 0, unc + 2)) == 0) {
            Volume *share = new Volume(unc + 2, &info);
            if (!share)
                return setFileError(0x2846);
            name->volume = share->id;
        }
        at = second - unc;
    } else {
        if (!name->volume)
            files.currentDirectory.volume(&name->volume);
        at = 0;
    }
    volume = volumeOf(name->volume);
    p = upper;
    do {
        c = name->path[at++];
        if (!volume->info.caseSensitive)
            c = toupper(c);
    } while ((*p++ = c) != 0);
    at = 0;
    p = upper;
    if (*p != '\\') {
        if (volume->mount())
            return files.error;
        volume->rootPath(root);
        at = strlen(root) - 1;
        root[at] = 0;
        SetCurrentDirectory(root);
        GetCurrentDirectory(sizeof root, root);
        strcpy(name->path, root + at);
        at = strlen(name->path);
        if (*p && name->path[at - 1] != '\\')
            name->path[at++] = '\\';
    }
    while ((name->path[at] = *p) != 0) {
        found = strchr(p, '\\');
        length = found ? found - p : strlen(p);
        if (!length) {
            if (at > 0 && (!p[1] || name->path[at - 1] == '\\'))
                return setFileError(0x2843);
            at++;
            p++;
        } else {
            if (length == 1 && !memcmp(p, ".", 1))
                at--;
            else if (length == 2 && !memcmp(p, "..", 2)) {
                name->path[at - 1] = 0;
                if ((found = strrchr(name->path, '\\')) != 0)
                    at = found - name->path;
                else
                    return setFileError(0x2843);
            } else {
                if (validName(p, length, name->volume)) {
                    memcpy(name->path + at, p, length);
                    at += length;
                } else
                    return setFileError(0x2843);
            }
            p += length;
        }
        if (at + 1 >= volume->info.maxName)
            return setFileError(0x2843);
    }
    if (!at)
        strcpy(name->path + at++, "\\");
    name = (FileName *)realloc(name, at + 9);
    return setFileError(0);
}

/* Not exact: the original shares ebx between `on` and `c` and keeps the
   length in edi and the index in esi; BCC32 4.5 allocates them
   differently. */
/* @zoombi32 0x00485a63 */
unsigned short __cdecl validName(const char *file, unsigned short length, long volume)
{
    short longNames;
    short inExtension;
    unsigned short extension;
    unsigned short base;
    Volume *on;
    long i;
    char c;

    if (!volume)
        files.currentDirectory.volume(&volume);
    if ((on = volumeOf(volume)) == 0) {
        setFileError(0x2870);
        return -1;
    }
    if (!length)
        length = strlen(file);
    if (!length && length >= on->info.maxPath)
        return 0;
    if (!strcmp(on->info.fileSystem, "NTFS") || !strcmp(on->info.fileSystem, "HPFS")) {
        for (i = 0; i < length; i++) {
            c = toupper(file[i]);
            switch (c) {
            case ' ':
            case '.':
                if (i == length)
                    return 0;
                break;
            default:
                if ((c < 'A' || c > 'Z') && (c < '0' || c > '9') && (unsigned short)c < 0x80)
                    return 0;
                break;
            case '!':
            case '#':
            case '$':
            case '%':
            case '&':
            case '\'':
            case '(':
            case ')':
            case '+':
            case ',':
            case '-':
            case ';':
            case '=':
            case '@':
            case '[':
            case ']':
            case '^':
            case '_':
            case '`':
            case '{':
            case '}':
            case '~':
                break;
            }
        }
    } else {
        longNames = on->info.maxPath > 0xd;
        inExtension = 0;
        extension = 0;
        base = 0;
        for (i = 0; i < length; i++) {
            c = toupper(file[i]);
            switch (c) {
            case '.':
                if (!longNames) {
                    if (inExtension || !base)
                        return 0;
                    inExtension = 1;
                    continue;
                }
            case ' ':
                if (i == length)
                    return 0;
                break;
            case '+':
            case ',':
            case ';':
            case '=':
            case '[':
            case ']':
                if (!longNames)
                    return 0;
                break;
            default:
                if ((c < 'A' || c > 'Z') && (c < '0' || c > '9') && (unsigned short)c < 0x80)
                    return 0;
                break;
            case '!':
            case '#':
            case '$':
            case '%':
            case '&':
            case '\'':
            case '(':
            case ')':
            case '-':
            case '@':
            case '^':
            case '_':
            case '`':
            case '{':
            case '}':
            case '~':
                break;
            }
            if (!longNames && (!inExtension && ++base > 8 || inExtension && ++extension > 3))
                return 0;
        }
    }
    return 1;
}

/* @zoombi32 0x00485d70 */
long driveNumber(char letter)
{
    letter = toupper(letter) - 'A';
    if (letter >= 0 && letter < files.drives->count && files.drives->drives[letter].number)
        return files.drives->drives[letter].number;
    return 0;
}

/* @zoombi32 0x00485dc5 */
Volume *volumeOf(long id)
{
    Volume *volume = (Volume *)id;

    if (!volume || volume->tag != 0x566f6c6d)
        volume = 0;
    return volume;
}

/* @zoombi32 0x00485ddd */
void *__cdecl Volume::operator new(size_t size)
{
    void *block = malloc(size);

    return block ? memset(block, 0, size) : 0;
}
