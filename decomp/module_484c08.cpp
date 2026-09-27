/*
 * module_484c08 (Mohawk engine): reading files
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Reads up to *size bytes; *size is set to how many were read (0x283f if
   fewer). */
/* @zoombi32 0x00484c08 */
short readFile(long handle, void *buffer, long *size)
{
    long wanted = *size;
    FileRecord *file;
    short error;

    *size = 0;
    if ((file = fileOf(handle, 0)) == 0)
        return setFileError(0x286f);
    if (file->volume->mount())
        return files.error;
    if (file->lock(1))
        return files.error;
    if ((error = asyncReadFile(file->handle, buffer, wanted, (DWORD *)size, 0).callFor(file->path))
        != 0) {
        file->lock(0);
        return files.error = error;
    }
    file->lock(0);
    file->volume->touch(currentTimeMs());
    return setFileError(wanted == *size ? 0 : 0x283f);
}

/* @zoombi32 0x00484ce3 */
__cdecl asyncReadFile::asyncReadFile(HANDLE file, void *buffer, DWORD size, DWORD *read,
                                     OVERLAPPED *overlapped)
{
    this->file = file;
    this->buffer = buffer;
    this->size = size;
    this->read = read;
    this->overlapped = overlapped;
}

/* @zoombi32 0x00484d98 */
void unlockFile(long handle)
{
    FileRecord *file = fileOf(handle, 0);

    if (!file)
        setFileError(0x286f);
    else
        releaseMutex(file->mutex);
}
