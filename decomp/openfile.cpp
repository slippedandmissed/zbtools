/*
 * openfile (Mohawk engine): opening files
 */

/* @flags -p -x- */

#include <string.h>
#include <stdlib.h>

#include "zoombinis.h"

/* @zoombi32 0x00484b50 */
long openFile(fileSpec *spec, short mode)
{
    FileRecord *file = new FileRecord;

    if (!file) {
        setFileError(0x2846);
        return 0;
    }
    if (file->open(*spec, mode)) {
        delete file;
        return 0;
    }
    {
        FileRecord *smaller = (FileRecord *)realloc(file, strlen(file->path) + 0x1d);
        if (smaller) {
            file = smaller;
            files.files = file;
            if (file->next)
                file->next->prev = file;
        }
    }
    return (long)file;
}

/* @zoombi32 0x00484bdc */
void *__cdecl FileRecord::operator new(size_t size)
{
    void *block = malloc(size);

    return block ? memset(block, 0, size) : 0;
}
