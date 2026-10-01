/*
 * filelength (Mohawk engine): a file's length; the file layer's error
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x004845fc */
unsigned long fileLength(LONG_PTR handle)
{
    FileRecord *file = fileOf(handle, 0);

    if (!file) {
        setFileError(0x286f);
        return 0xffffffff;
    }
    if (file->volume->mount())
        return 0xffffffff;
    {
        DWORD at = SetFilePointer(file->handle, 0, 0, FILE_CURRENT);
        DWORD end = SetFilePointer(file->handle, 0, 0, FILE_END);
        SetFilePointer(file->handle, at, 0, FILE_BEGIN);
        setFileError(0);
        return end;
    }
}

/* @zoombi32 0x00484670 */
short fileError()
{
    return files.error;
}
