/*
 * module_484690 (Mohawk engine): the size of a file, or of a directory's
 * files (recursively)
 */

/* @flags -p -x- */

#include <string.h>

#include "zoombinis.h"

/* fileSize's running total for a directory. */
struct DirectorySize
{
    __cdecl DirectorySize(const fileSpec *directory);

    short error;
    short unknown2;
    long total;
    const fileSpec *directory;
};

short directorySizeCallback(const char *name, void *data);

/* @zoombi32 0x00484690 */
long fileSize(const fileSpec &spec)
{
    long volume;
    char path[0x100];
    WIN32_FIND_DATA found;

    if (spec.locate(&volume, path))
        return -1;
    asyncFindFirstFile find(path, &found);
    if (strcmp(path + 1, ":\\")) {
        if (find.callFor(path))
            return files.error;
        FindClose(find.find);
        if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            goto file;
    }
    {
        DirectorySize total(&spec);
        fileSpec current;
        currentDirectory(&current);
        if (setCurrentDirectory(&spec) || forEachFile(directorySizeCallback, &total)
            || forEachDirectory(directorySizeCallback, &total) || total.error)
            total.total = -1;
        setCurrentDirectory(&current);
        setFileError(total.error);
        return total.total;
    }
file:
    setFileError(0);
    return found.nFileSizeLow;
}

/* @zoombi32 0x00484816 */
short directorySizeCallback(const char *name, void *data)
{
    DirectorySize *total = (DirectorySize *)data;
    long size = fileSize(fileSpec(*total->directory, name));

    if (size == -1) {
        total->error = fileError();
        return 1;
    }
    total->total += size;
    return 0;
}

/* @zoombi32 0x00484874 */
__cdecl asyncFindFirstFile::asyncFindFirstFile(const char *pattern, WIN32_FIND_DATA *found)
{
    this->pattern = pattern;
    this->found = found;
}

/* @zoombi32 0x00484899 */
__cdecl DirectorySize::DirectorySize(const fileSpec *directory)
{
    this->directory = directory;
    error = 0;
    total = 0;
}

/* @zoombi32-implicit 0x0048490d asyncFindFirstFile::~asyncFindFirstFile */
