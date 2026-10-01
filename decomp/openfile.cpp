/*
 * openfile (Mohawk engine): opening files
 */

/* @flags -p -x- */

#include <string.h>
#include <stdlib.h>

#include <stddef.h>
#include "zoombinis.h"

/* @zoombi32 0x00484b50 */
LONG_PTR openFile(fileSpec *spec, short mode)
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
        FileRecord *smaller = (FileRecord *)realloc(file, offsetof(FileRecord, path) + strlen(file->path) + 1);
        if (smaller) {
            file = smaller;
            files.files = file;
            if (file->next)
                file->next->prev = file;
        }
    }
    return (LONG_PTR)file;
}

/* @zoombi32 0x00484bdc */
void *__cdecl FileRecord::operator new(size_t size)
{
    void *block = malloc(size);

    return block ? memset(block, 0, size) : 0;
}
