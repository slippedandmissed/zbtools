/*
 * module_484dc4 (Mohawk engine): seeking in files
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* Moves to `offset` from the start (whence 0), the current position (1) or
   the end (2), no further than the end; the new position, or -1. */
/* Not exact: the original keeps the file in ebx, the position in esi and
   the offset in edi; BCC32 4.5 rotates them. */
/* @zoombi32 0x00484dc4 */
unsigned long seekFile(long handle, unsigned long offset, long whence)
{
    FileRecord *file;
    unsigned long at;

    if ((file = fileOf(handle, 0)) == 0) {
        setFileError(0x286f);
        return 0xffffffff;
    }
    at = SetFilePointer(file->handle, 0, 0, FILE_CURRENT);
    if (whence == 1 && !offset) {
        setFileError(0);
        return at;
    }
    if (file->lock(1))
        return 0xffffffff;
    {
        unsigned long end = SetFilePointer(file->handle, 0, 0, FILE_END);
        offset = whence == 0 ? offset : whence == 1 ? offset + at : offset + end;
        if (end < offset) {
            at = end;
            offset = 0xffffffff;
            setFileError(0x283f);
        } else {
            at = offset;
            setFileError(0);
        }
    }
    SetFilePointer(file->handle, at, 0, FILE_BEGIN);
    {
        short error = files.error;
        file->lock(0);
        files.error = error;
    }
    return offset;
}
