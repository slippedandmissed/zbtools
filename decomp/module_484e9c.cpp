/*
 * module_484e9c (Mohawk engine): the current directory; truncating files
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00484e9c */
short setCurrentDirectory(const fileSpec *spec)
{
    long volume;
    DWORD attributes;
    char path[0x100];

    if (spec->locate(&volume, path) || getAttributes(path, &attributes))
        return files.error;
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return setFileError(0x2841);
    while (!SetCurrentDirectory(path))
        if (!askAboutError(path, GetLastError()))
            return files.error;
    files.currentDirectory = *spec;
    return setFileError(0);
}

/* @zoombi32 0x00484f3c */
short setFileSize(long handle, unsigned long size)
{
    FileRecord *file = fileOf(handle, 0);
    short error;

    if (!file)
        return setFileError(0x286f);
    if (file->volume->mount())
        return files.error;
    if (file->lock(1))
        return files.error;
    {
        DWORD at = SetFilePointer(file->handle, 0, 0, FILE_CURRENT);
        SetFilePointer(file->handle, size, 0, FILE_BEGIN);
        if ((error = asyncSetEndOfFile(file->handle).callFor(file->path)) != 0) {
            SetFilePointer(file->handle, at, 0, FILE_BEGIN);
            file->lock(0);
            return files.error = error;
        }
        if (at < size)
            SetFilePointer(file->handle, at, 0, FILE_BEGIN);
    }
    file->lock(0);
    return setFileError(0);
}

/* @zoombi32 0x00485030 */
__cdecl asyncSetEndOfFile::asyncSetEndOfFile(HANDLE file)
{
    this->file = file;
}
