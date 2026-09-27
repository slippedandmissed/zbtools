/*
 * module_48213c (Mohawk engine): the async API: Win32 calls made on worker
 * threads, so a slow or missing disk doesn't stop the game
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x0048213c */
__cdecl AsyncWorker::AsyncWorker()
{
    thread = 0;
    prev = 0;
    if ((next = asyncWorkers) != 0)
        next->prev = this;
    asyncWorkers = this;
}

/* Stops the thread (after its call, if one is running). */
/* @zoombi32 0x00482164 */
__cdecl AsyncWorker::~AsyncWorker()
{
    if (thread) {
        if (caller)
            suspendThread(caller);
        waitSync(done, -1);
        run(0, 0);
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        CloseHandle(wake);
        deleteSync(done);
    }
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        asyncWorkers = next;
}

/* Has the thread (started the first time) call `proc`, and waits for it. */
/* @zoombi32 0x004821ec */
short AsyncWorker::run(void (*proc)(void *data), void *data)
{
    if (!thread) {
        done = newEvent(0);
        if (!done)
            return setFileError(threadError());
        wake = CreateEvent(0, 0, 0, 0);
        if (!wake) {
            deleteSync(done);
            return setFileError(0x2846);
        }
        thread = CreateThread(0, 0, asyncThread, this, 0, &threadId);
        if (!thread) {
            CloseHandle(wake);
            deleteSync(done);
            return setFileError(0x2846);
        }
        SetThreadPriority(thread, THREAD_PRIORITY_ABOVE_NORMAL);
    }
    caller = fn_46e5dc();
    this->proc = proc;
    this->data = data;
    resetEvent(done);
    SetEvent(wake);
    return setFileError(waitSync(done, -1));
}

/* A worker's thread: runs each call it's woken for, until told to stop (a
   null procedure). */
/* @zoombi32 0x004822c4 */
DWORD WINAPI asyncThread(void *data)
{
    AsyncWorker *worker = (AsyncWorker *)data;

    while (WaitForSingleObject(worker->wake, INFINITE) == 0 && worker->proc) {
        worker->proc(worker->data);
        setEvent(worker->done);
        worker->caller = 0;
        worker->proc = 0;
    }
    setEvent(worker->done);
    return 0;
}

/* @zoombi32 0x00482308 */
__cdecl asyncAPI::asyncAPI()
{
    worker = 0;
}

/* Keeps one idle worker for the next call. */
/* @zoombi32 0x0048231b */
__cdecl asyncAPI::~asyncAPI()
{
    if (worker) {
        if (files.spareWorker)
            delete worker;
        else
            files.spareWorker = worker;
    }
}

/* @zoombi32 0x00482362 */
void asyncRun(void *call)
{
    ((asyncAPI *)call)->run();
}

/* @zoombi32 0x00482371 */
short asyncAPI::call()
{
    error = 0;
    if (!worker) {
        if ((worker = files.spareWorker) != 0)
            files.spareWorker = 0;
        else {
            worker = new AsyncWorker;
            if (!worker)
                return setFileError(0x2846);
        }
    }
    return worker->run(asyncRun, this);
}

/* The original leaves `drive` unset (it's never used) for a relative path.
   Not exact: the original keeps `this` in ebx and `path` in esi; BCC32 4.5
   swaps them. */
/* @zoombi32 0x004823db */
short asyncAPI::callFor(const char *path)
{
    Drive *drive;

    if (path[0] == '\\' && path[1] == '\\')
        drive = 0;
    else if (path[1] == ':') {
        long number = driveNumber(path[0]);
        if (!number)
            return files.error;
        drive = driveAt(number, 0);
        drive->lock(1);
    }
    do {
    } while (!call() && error && askAboutError(path, error));
    if (drive)
        drive->lock(0);
    return files.error;
}

/* @zoombi32 0x00482463 */
void asyncCloseHandle::run()
{
    error = CloseHandle(handle) ? 0 : GetLastError();
}

/* @zoombi32 0x00482487 */
void asyncCreateDirectory::run()
{
    error = CreateDirectory(path, security) ? 0 : GetLastError();
}

/* @zoombi32 0x004824ae */
void asyncCreateFile::run()
{
    file = CreateFile(path, access, share, security, creation, flags, templateFile);
    error = file == INVALID_HANDLE_VALUE ? GetLastError() : 0;
}

/* @zoombi32 0x004824e9 */
void asyncDeleteFile::run()
{
    error = DeleteFile(path) ? 0 : GetLastError();
}

/* @zoombi32 0x0048250d */
void asyncFindFirstFile::run()
{
    find = FindFirstFile(pattern, found);
    error = find == INVALID_HANDLE_VALUE ? GetLastError() : 0;
}

/* @zoombi32 0x00482539 */
void asyncGetFileAttributes::run()
{
    attributes = GetFileAttributes(path);
    error = attributes == 0xffffffff ? GetLastError() : 0;
}

/* @zoombi32 0x00482562 */
void asyncGetVolumeInformation::run()
{
    error = GetVolumeInformation(root, name, nameSize, serial, maxComponent, flags, fileSystem,
                                 fileSystemSize)
        ? 0
        : GetLastError();
}

/* @zoombi32 0x0048259b */
void asyncReadFile::run()
{
    error = ReadFile(file, buffer, size, read, overlapped) ? 0 : GetLastError();
}

/* @zoombi32 0x004825cb */
void asyncRemoveDirectory::run()
{
    error = RemoveDirectory(path) ? 0 : GetLastError();
}

/* @zoombi32 0x004825ef */
void asyncSetFileAttributes::run()
{
    error = SetFileAttributes(path, attributes) ? 0 : GetLastError();
}

/* @zoombi32 0x00482616 */
void asyncSetEndOfFile::run()
{
    error = SetEndOfFile(file) ? 0 : GetLastError();
}

/* @zoombi32 0x0048263a */
void asyncWriteFile::run()
{
    error = WriteFile(file, buffer, size, written, overlapped) ? 0 : GetLastError();
}
