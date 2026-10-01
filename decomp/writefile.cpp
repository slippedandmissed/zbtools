/*
 * writefile (Mohawk engine): holding files, writing to them; the file
 * layer's error
 */

/* @flags -p -x- */

#include "zoombinis.h"
#include "os_manager.h"
#include "os_threads.h"

/* Holds the file (for a sequence of calls), waiting up to `timeout` ms;
   0x283d if that runs out. */
/* @zoombi32 0x004860cc */
short lockFile(LONG_PTR handle, long timeout)
{
    FileRecord *file = fileOf(handle, 0);

    if (!file)
        return setFileError(0x286f);
    {
        short error = waitSync(file->mutex, timeout);
        return setFileError(error == 0x12e ? 0x283d : error);
    }
}

/* Writes *size bytes; *size is set to how many were written (0x283f if
   fewer). */
/* @zoombi32 0x0048610c */
short writeFile(LONG_PTR handle, const void *buffer, long *size)
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
    if ((error = asyncWriteFile(file->handle, buffer, wanted, (DWORD *)size, 0).callFor(file->path))
        != 0) {
        file->lock(0);
        return files.error = error;
    }
    file->lock(0);
    file->volume->touch(currentTimeMs());
    return setFileError(wanted == *size ? 0 : 0x283f);
}

/* @zoombi32 0x004861e7 */
short setFileError(short error)
{
    return files.error = error;
}

/* @zoombi32 0x00486240 */
FileRecord *fileOf(LONG_PTR handle, short kind)
{
    FileRecord *file = (FileRecord *)handle;

    if (!file || file->tag != 0x46696c65 || file->kind != kind)
        file = 0;
    return file;
}

/* @zoombi32 0x00486262 */
__cdecl asyncWriteFile::asyncWriteFile(HANDLE file, const void *buffer, DWORD size,
                                       DWORD *written, OVERLAPPED *overlapped)
{
    this->file = file;
    this->buffer = buffer;
    this->size = size;
    this->written = written;
    this->overlapped = overlapped;
}

/* @zoombi32-implicit 0x004862f1 asyncWriteFile::~asyncWriteFile */
