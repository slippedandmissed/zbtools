/*
 * fileops (Mohawk engine): closing files, and creating and deleting
 * files and directories by name
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048266c */
short closeFile(long handle, short force)
{
    FileRecord *file = fileOf(handle, 0);

    if (!file)
        return setFileError(0x286f);
    if (!file->close() || force)
        delete file;
    return files.error;
}

/* @zoombi32 0x004826b4 */
short createPath(const fileSpec &spec, unsigned short mode)
{
    long volume;
    DWORD attributes;
    char path[0x100];

    if (!(mode & 3))
        mode |= 1;
    if (spec.locate(&volume, path))
        return files.error;
    switch (getAttributes(path, &attributes)) {
    case 0:
        return setFileError(attributes & FILE_ATTRIBUTE_DIRECTORY ? mode & 2 ? 0x283e : 0x2841
                                                                  : mode & 1 ? 0x283e : 0x2841);
    case 0x2845:
        attributes = FILE_ATTRIBUTE_NORMAL;
        break;
    default:
        return files.error;
    }
    if (mode & 2) {
        if (asyncCreateDirectory(path, 0).callFor(path))
            return files.error;
    } else {
        asyncCreateFile create(path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_NEW, attributes, 0);
        if (create.callFor(path))
            return files.error;
        asyncCloseHandle(create.file).callFor(path);
    }
    attributes &= ~3;
    attributes |= (mode & 4 ? FILE_ATTRIBUTE_HIDDEN : 0) | (mode & 8 ? FILE_ATTRIBUTE_READONLY : 0);
    return setAttributes(path, attributes);
}

/* @zoombi32 0x00482875 */
__cdecl asyncCreateDirectory::asyncCreateDirectory(const char *path, SECURITY_ATTRIBUTES *security)
{
    this->path = path;
    this->security = security;
}

/* @zoombi32-implicit 0x004828fa asyncCreateDirectory::~asyncCreateDirectory */

/* Deletes a file or (empty) directory, unless it's in use. */
/* @zoombi32 0x00482920 */
short deleteFile(const fileSpec &spec)
{
    long volume;
    DWORD attributes;
    char path[0x100];

    if (spec.locate(&volume, path))
        return files.error;
    if (getAttributes(path, &attributes))
        return files.error;
    if (fileInUse(volume, path, attributes))
        return setFileError(0x283d);
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        return asyncRemoveDirectory(path).callFor(path);
    return asyncDeleteFile(path).callFor(path);
}

/* @zoombi32 0x00482a0c */
__cdecl asyncRemoveDirectory::asyncRemoveDirectory(const char *path)
{
    this->path = path;
}

/* @zoombi32 0x00482a2b */
__cdecl asyncDeleteFile::asyncDeleteFile(const char *path)
{
    this->path = path;
}

/* @zoombi32-implicit 0x00482b02 asyncRemoveDirectory::~asyncRemoveDirectory */

/* @zoombi32-implicit 0x00482b28 asyncDeleteFile::~asyncDeleteFile */
