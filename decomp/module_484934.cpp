/*
 * module_484934 (Mohawk engine): the name of an open file
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00484934 */
short fileSpecOf(long handle, fileSpec *spec)
{
    FileRecord *file = fileOf(handle, 0);

    if (!file)
        return setFileError(0x286f);
    *spec = fileSpec(file->volume->id, file->path + 2);
    return files.error;
}
