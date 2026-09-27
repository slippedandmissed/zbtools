/*
 * module_4831bc (Mohawk engine): listing the current directory
 */

/* @flags -p -x- */

#include <string.h>

#include "zoombinis.h"

/* The original doesn't check FindFirstFile's result when there's nothing to
   find (error 2): it goes on to look at an unset entry. */
/* Not exact: the original keeps the entry pointer in edi and the find
   handle in esi; BCC32 4.5 swaps them. */
/* @zoombi32 0x004831bc */
short forEachDirectory(FileCallback callback, void *data)
{
    long volume;
    char path[0x100];
    WIN32_FIND_DATA found;
    DWORD error;
    HANDLE find;
    WIN32_FIND_DATA *entry = &found;

    if (files.currentDirectory.locate(&volume, path))
        return files.error;
    if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");
    strcat(path, "*.*");
    error = 0;
    while ((find = FindFirstFile(path, entry)) == INVALID_HANDLE_VALUE
           && (error = GetLastError()) != ERROR_FILE_NOT_FOUND)
        if (!askAboutError(path, error))
            return files.error;
    if (error != ERROR_NO_MORE_FILES)
        do {
            if (entry->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && stricmp(entry->cFileName, ".")
                && stricmp(entry->cFileName, "..") && callback(entry->cFileName, data))
                break;
            while (!FindNextFile(find, entry) && (error = GetLastError()) != ERROR_NO_MORE_FILES)
                if (!askAboutError(path, error)) {
                    FindClose(find);
                    return files.error;
                }
        } while (error != ERROR_NO_MORE_FILES);
    FindClose(find);
    return setFileError(0);
}

/* Not exact: the original keeps the error in ebx, the find handle in esi
   and the path in edi; BCC32 4.5 assigns them differently. */
/* @zoombi32 0x00483310 */
short forEachFile(FileCallback callback, void *data)
{
    long volume;
    char buffer[0x100];
    WIN32_FIND_DATA found;
    HANDLE find;
    DWORD error;
    char *path = buffer;

    if (files.currentDirectory.locate(&volume, path))
        return files.error;
    if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");
    strcat(path, "*.*");
    error = 0;
    while ((find = FindFirstFile(path, &found)) == INVALID_HANDLE_VALUE
           && (error = GetLastError()) != ERROR_FILE_NOT_FOUND)
        if (!askAboutError(path, error))
            return files.error;
    if (error != ERROR_NO_MORE_FILES)
        do {
            if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && callback(found.cFileName, data))
                break;
            while (!FindNextFile(find, &found) && (error = GetLastError()) != ERROR_NO_MORE_FILES)
                if (!askAboutError(path, error)) {
                    FindClose(find);
                    return files.error;
                }
        } while (error != ERROR_NO_MORE_FILES);
    FindClose(find);
    return setFileError(0);
}

/* @zoombi32 0x00483420 */
short fileMissing(const fileSpec &spec)
{
    long volume;
    DWORD attributes;
    char path[0x100];

    if (spec.locate(&volume, path))
        return files.error;
    return getAttributes(path, &attributes);
}
