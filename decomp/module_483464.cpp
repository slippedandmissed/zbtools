/*
 * module_483464 (Mohawk engine): the file layer: starting and stopping it,
 * errors (and asking the user about them), attributes, and opening and
 * closing files
 */

/* @flags -p -x- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zoombinis.h"

/* The application was activated or deactivated. */
/* @zoombi32 0x00483464 */
void filesActivated(short active)
{
    unsigned short i;

    if (files.previousHook)
        files.previousHook(active);
    files.active = active;
    for (i = 0; i < files.drives->count; i++)
        files.drives->drives[i].activate(active);
}

/* @zoombi32 0x004834aa */
short readDiskInfo(const char *root, DiskInfo *info)
{
    DWORD flags;
    DWORD maxComponent;
    DWORD serial;

    memset(info, 0, sizeof *info);
    if (asyncGetVolumeInformation(root, info->name, sizeof info->name - 2, &info->serial,
                                  &maxComponent, &flags, info->fileSystem, sizeof info->fileSystem)
            .callFor(root))
        return files.error;
    info->casePreserved = (flags & FS_CASE_IS_PRESERVED) != 0;
    info->caseSensitive = (flags & FS_CASE_SENSITIVE) != 0;
    info->maxPath = maxComponent + 1;
    info->maxName = maxComponent == 12 ? 0x4e : maxComponent + 2;
    return setFileError(0);
}

/* A root directory ("C:\") counts as a directory. */
/* @zoombi32 0x00483557 */
short getAttributes(const char *path, DWORD *attributes)
{
    if (!strcmp(path + 1, ":\\"))
        *attributes = FILE_ATTRIBUTE_DIRECTORY;
    else {
        asyncGetFileAttributes get(path);
        if (get.callFor(path))
            return files.error;
        *attributes = get.attributes;
    }
    return setFileError(0);
}

/* @zoombi32 0x004835d8 */
short askAboutError(const char *path, DWORD error)
{
    char message[0x20];
    FileRequest request;
    char share[0x100];
    char volumeName[0x70];
    short mapped;
    short answer;

    memset(&request, 0, sizeof request);
    mapped = fileErrorOf(error);
    switch (mapped) {
    case 0x2846:
        request.kind = 1;
        request.message = message;
        sprintf(message, "General system error #%ld.", error);
        break;
    case 0x284a:
        request.kind = 2;
        break;
    case 0x284c:
        request.kind = 3;
        break;
    case 0x284d:
        request.kind = 4;
        break;
    default:
        setFileError(mapped);
        return 0;
    }
    if (path[1] == ':') {
        request.drive = driveNumber(path[0]);
        request.volume = driveAt(request.drive, 0)->volume;
    } else if (path[0] == '\\' && path[1] == '\\') {
        char *name;
        path += 2;
        strcpy(share, path);
        name = strchr(share, '\\');
        *name = 0;
        *strchr(++name, '\\') = 0;
        memset(volumeName, 0, sizeof volumeName);
        strcpy(volumeName, name);
        request.volume = findVolume((DiskInfo *)volumeName, 0, share);
    }
    if ((answer = askFileUser(&request)) == 0)
        setFileError(mapped);
    return answer;
}

/* Starts the file layer: the drives, the current, program and temporary
   directories, and the activate hook. */
/* @zoombi32 0x00483732 */
short initFiles(long)
{
    char path[0x100];
    unsigned short count;
    unsigned short i;
    DWORD drives;

    SetErrorMode(SEM_FAILCRITICALERRORS);
    memset(&files, 0, sizeof files);
    drives = GetLogicalDrives();
    count = 0;
    i = 0;
    do
        if (1 << i & drives)
            count = i + 1;
    while (++i < 32);
    if ((files.drives = (DriveTable *)malloc(count * sizeof(Drive) + 4)) != 0)
        memset(files.drives, 0, count * sizeof(Drive) + 4);
    else {
        setFileError(0x2846);
    fail:
        if (files.drives) {
            for (i = 0; i < files.drives->count; i++)
                files.drives->drives[i].~Drive();
            free(files.drives);
        }
        return files.error;
    }
    files.drives->count = count;
    for (i = 0; i < count; i++) {
        if (!files.drives->drives[i].init(i)) {
            files.drives->drives[i].setLocked(0);
            if (!files.drives->drives[i].removable && !files.drives->drives[i].remote)
                mountDrive(files.drives->drives[i].number);
        } else if (files.error)
            goto fail;
    }
    GetCurrentDirectory(sizeof path, path);
    files.currentDirectory = fileSpec(path);
    if (files.error)
        goto fail;
    GetModuleFileName(engineInstanceHandle(), path, sizeof path);
    files.programDirectory = fileSpec(path, "..");
    if (files.error)
        goto fail;
    GetTempPath(sizeof path, path);
    if (strlen(path) > 3)
        *strrchr(path, '\\') = 0;
    files.tempDirectory = fileSpec(path);
    if (files.error)
        goto fail;
    files.appDirectory = fileSpec(files.programDirectory);
    files.canAsk = 1;
    files.ready = 1;
    if (isAppActive())
        filesActivated(1);
    files.previousHook = setActivateHook(filesActivated);
    return setFileError(0);
}

/* @zoombi32 0x00483a22 */
short __fastcall fileLayerVersion()
{
    return files.ready ? 0x500 : 0;
}

/* @zoombi32 0x00483a36 */
void __cdecl closeFiles()
{
    unsigned short i;

    files.askUser = 0;
    files.canAsk = 1;
    fn_46e410();
    setActivateHook(files.previousHook);
    files.previousHook = 0;
    filesActivated(0);
    while (files.volumes)
        delete files.volumes;
    for (i = 0; i < files.drives->count; i++)
        files.drives->drives[i].~Drive();
    free(files.drives);
    fn_46e43a();
    {
        AsyncWorker *worker;
        while ((worker = asyncWorkers) != 0)
            delete worker;
    }
    SetErrorMode(0);
    files.ready = 0;
}

/* A root directory's attributes can't be set. */
/* @zoombi32 0x00483ad5 */
short setAttributes(const char *path, DWORD attributes)
{
    if (!strcmp(path + 1, ":\\"))
        return setFileError(0x2841);
    return asyncSetFileAttributes(path, attributes).callFor(path);
}

/* @zoombi32 0x00483b2e */
short fileInUse(long volume, const char *path, DWORD attributes)
{
    Volume *on = volumeOf(volume);

    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return findOpenFile(volume, path) != 0;
    {
        fileSpec directory(path);
        unsigned short length;
        FileRecord *file;
        if (!strcmp(path + 2, "\\") || !directory.compare(files.currentDirectory)
            || !directory.compare(files.programDirectory) || !directory.compare(files.appDirectory)
            || !directory.compare(files.tempDirectory))
            return 1;
        length = strlen(path);
        for (file = files.files; file; file = file->next)
            if (file->volume->id == volume
                && (on->info.caseSensitive ? !memcmp(path, file->path, length)
                                           : !memicmp(path, file->path, length)))
                return 1;
        return 0;
    }
}

/* @zoombi32 0x00483c8a */
FileRecord *findOpenFile(long volume, const char *path)
{
    Volume *on = volumeOf(volume);
    FileRecord *file;

    for (file = files.files; file; file = file->next)
        if (volume == file->volume->id
            && (on->info.caseSensitive ? !strcmp(path, file->path) : !stricmp(path, file->path)))
            break;
    return file;
}

/* @zoombi32 0x00483cfb */
short fileErrorOf(DWORD error)
{
    switch (error) {
    case ERROR_ACCESS_DENIED:
    case ERROR_DIR_NOT_EMPTY:
        return 0x283c;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return 0x283e;
    case ERROR_BUFFER_OVERFLOW:
    case ERROR_FILENAME_EXCED_RANGE:
        return 0x2843;
    case ERROR_CURRENT_DIRECTORY:
        return 0x283d;
    case ERROR_DIRECTORY:
        return 0x2842;
    case ERROR_HANDLE_DISK_FULL:
    case ERROR_DISK_FULL:
        return 0x2849;
    case ERROR_DRIVE_LOCKED:
        return 0x284a;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_NO_MORE_FILES:
        return 0x2845;
    case ERROR_HANDLE_EOF:
        return 0x283f;
    case ERROR_INVALID_ACCESS:
        return 0x2841;
    case ERROR_INVALID_DRIVE:
        return 0x2870;
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        return 0x2848;
    case ERROR_NOT_READY:
    case ERROR_WRONG_DISK:
    case ERROR_DISK_CHANGE:
        return 0x284c;
    case ERROR_PATH_NOT_FOUND:
        return 0x2847;
    case ERROR_WRITE_PROTECT:
        return 0x284d;
    default:
        return 0x2846;
    }
}

/* @zoombi32 0x00483e4c */
__cdecl FileRecord::FileRecord()
{
    tag = 0x46696c65;
    if ((next = files.files) != 0)
        files.files->prev = this;
    files.files = this;
}

/* @zoombi32 0x00483e75 */
__cdecl FileRecord::~FileRecord()
{
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        files.files = next;
    tag = 0;
}

/* Opens the file: mode 1 read, 2 write, 0x10/0x20 share reading/writing,
   0x100 create if need be, 0x200 create (it mustn't exist), 0x1000 at the
   end, 0x4000 truncate. */
/* @zoombi32 0x00483ebb */
short FileRecord::open(const fileSpec &spec, unsigned short mode)
{
    DWORD access;
    DWORD share;
    DWORD creation;
    long id;

    if (!(mode & 3))
        mode |= 1;
    if (mode & 0x2000)
        return setFileError(0x2840);
    access = (mode & 1 ? GENERIC_READ : 0) | (mode & 2 ? GENERIC_WRITE : 0);
    share = (mode & 0x10 ? FILE_SHARE_READ : 0) | (mode & 0x20 ? FILE_SHARE_WRITE : 0);
    creation = mode & 0x100 ? OPEN_ALWAYS : mode & 0x200 ? CREATE_NEW : OPEN_EXISTING;
    if (spec.locate(&id, path))
        return files.error;
    volume = volumeOf(id);
    this->mode = mode;
    volume->drive->lock(1);
    asyncCreateFile create(path, access, share, 0, creation, FILE_ATTRIBUTE_NORMAL, 0);
    if (create.callFor(path)) {
        volume->drive->lock(0);
        return files.error;
    }
    handle = create.file;
    if (mode & 0x4000)
        if (asyncSetEndOfFile(handle).callFor(path))
            goto close;
    if (mode & 0x1000)
        SetFilePointer(handle, 0, 0, FILE_END);
    mutex = newMutex(1);
    if (!mutex) {
        setFileError(threadError());
    close:
        CloseHandle(handle);
    } else
        setFileError(0);
    volume->drive->lock(0);
    return files.error;
}

/* Holds the file (and its drive) for a call, or lets it go. */
/* @zoombi32 0x0048409b */
short FileRecord::lock(short on)
{
    if (!on)
        releaseMutex(mutex);
    else if (waitSync(mutex, -1))
        return setFileError(threadError());
    volume->drive->lock(on);
    return setFileError(0);
}

/* @zoombi32 0x004840ed */
short FileRecord::close()
{
    short error;

    if ((error = lock(1)) != 0)
        return files.error;
    if ((error = volume->mount()) != 0) {
        lock(0);
        return files.error = error;
    }
    if ((error = asyncCloseHandle(handle).callFor(path)) != 0) {
        lock(0);
        return files.error = error;
    }
    lock(0);
    deleteSync(mutex);
    return setFileError(0);
}

/* @zoombi32 0x00484203 */
__cdecl asyncCreateFile::asyncCreateFile(const char *path, DWORD access, DWORD share,
                                         SECURITY_ATTRIBUTES *security, DWORD creation,
                                         DWORD flags, HANDLE templateFile)
{
    this->path = path;
    this->access = access;
    this->share = share;
    this->security = security;
    this->creation = creation;
    this->flags = flags;
    this->templateFile = templateFile;
}

/* @zoombi32 0x00484246 */
__cdecl asyncCloseHandle::asyncCloseHandle(HANDLE handle)
{
    this->handle = handle;
}

/* Asks the user (through FileState.askUser) what to do; without a way to
   ask, gives up only if asking is allowed. */
/* @zoombi32 0x00484365 */
short askFileUser(FileRequest *request)
{
    if (files.askUser) {
        request->canAsk = files.canAsk;
        return files.askUser(request) || !files.canAsk;
    }
    return !files.canAsk;
}

/* @zoombi32 0x004843b3 */
__cdecl asyncGetVolumeInformation::asyncGetVolumeInformation(
    const char *root, char *name, DWORD nameSize, DWORD *serial, DWORD *maxComponent,
    DWORD *flags, char *fileSystem, DWORD fileSystemSize)
{
    this->root = root;
    this->name = name;
    this->nameSize = nameSize;
    this->serial = serial;
    this->maxComponent = maxComponent;
    this->flags = flags;
    this->fileSystem = fileSystem;
    this->fileSystemSize = fileSystemSize;
}

/* @zoombi32 0x004843fc */
__cdecl asyncGetFileAttributes::asyncGetFileAttributes(const char *path)
{
    this->path = path;
}

/* @zoombi32 0x0048441b */
__cdecl asyncSetFileAttributes::asyncSetFileAttributes(const char *path, DWORD attributes)
{
    this->path = path;
    this->attributes = attributes;
}

/* @zoombi32 0x00484440 */
void __cdecl FileRecord::operator delete(void *block)
{
    free(block);
}

/* @zoombi32 0x004845e4 */
void currentDirectory(fileSpec *directory)
{
    *directory = files.currentDirectory;
}
