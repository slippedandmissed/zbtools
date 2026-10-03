/*
 * Files, directories and drives. Each drive letter the port's main adds is
 * a host directory (C: holds the installed game and its saved state, D: the
 * CD); paths are Windows paths, matched against the host's names without
 * regard to case, as on Windows. There is one current directory, which may
 * be on any drive.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#ifdef __EMSCRIPTEN__
#include <unistd.h>
#endif
#include <string.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "miniwin/internal.h"

namespace fs = std::filesystem;

namespace miniwin {

struct Drive
{
    std::string root;
    std::string label;
    DWORD serial;
    bool cdrom;
};

static Drive *drives[26];
static std::string currentDirectory = "C:\\";

void addDrive(char letter, const char *path, const char *label, DWORD serial, bool cdrom)
{
    int index = toupper((unsigned char)letter) - 'A';

    if (index < 0 || index >= 26)
        return;
    delete drives[index];
    drives[index] = new Drive{path, label, serial, cdrom};
}

bool isCdrom(char drive)
{
    int index = toupper((unsigned char)drive) - 'A';
    return index >= 0 && index < 26 && drives[index] && drives[index]->cdrom;
}

static Drive *driveOf(const std::string &path)
{
    if (path.size() < 2 || path[1] != ':')
        return 0;
    int index = toupper((unsigned char)path[0]) - 'A';
    return index >= 0 && index < 26 ? drives[index] : 0;
}

/* The full path, "X:\a\b" (no trailing backslash but on a root), with "."
   and ".." resolved and slashes made backslashes. */
static std::string fullPath(const char *path)
{
    std::string text(path);
    for (char &c : text)
        if (c == '/')
            c = '\\';
    std::string full;
    if (text.size() >= 2 && text[1] == ':') {
        if (text.size() > 2 && text[2] == '\\')
            full = text;
        else if (toupper((unsigned char)text[0]) == toupper((unsigned char)currentDirectory[0]))
            full = currentDirectory + "\\" + text.substr(2);
        else
            full = text.substr(0, 2) + "\\" + text.substr(2);
    } else if (!text.empty() && text[0] == '\\')
        full = currentDirectory.substr(0, 2) + text;
    else
        full = currentDirectory + "\\" + text;

    std::vector<std::string> parts;
    size_t at = 3;
    while (at <= full.size()) {
        size_t end = full.find('\\', at);
        if (end == std::string::npos)
            end = full.size();
        std::string part = full.substr(at, end - at);
        if (part == "..") {
            if (!parts.empty())
                parts.pop_back();
        } else if (!part.empty() && part != ".")
            parts.push_back(part);
        at = end + 1;
    }
    std::string result = full.substr(0, 2);
    result[0] = (char)toupper((unsigned char)result[0]);
    for (const std::string &part : parts)
        result += "\\" + part;
    if (parts.empty())
        result += "\\";
    return result;
}

static bool sameName(const std::string &a, const std::string &b)
{
    return a.size() == b.size() && !stricmp(a.c_str(), b.c_str());
}

/* The host's name for `name` in `directory`: its own spelling if it has it
   in any case, else `name` as it is. */
static std::string hostName(const std::string &directory, const std::string &name)
{
    std::error_code error;

    if (fs::exists(fs::path(directory) / name, error))
        return name;
    for (const auto &entry : fs::directory_iterator(directory, error)) {
        std::string candidate = entry.path().filename().string();
        if (sameName(candidate, name))
            return candidate;
    }
    return name;
}

bool hostPath(const char *path, std::string &host)
{
    std::string full = fullPath(path);
    Drive *drive = driveOf(full);

    if (!drive)
        return false;
    host = drive->root;
    size_t at = 3;
    while (at < full.size()) {
        size_t end = full.find('\\', at);
        if (end == std::string::npos)
            end = full.size();
        host = (fs::path(host) / hostName(host, full.substr(at, end - at))).string();
        at = end + 1;
    }
    return true;
}

/* A failed file call, noted on the console. */
static void fail(const char *call, const char *path, DWORD error)
{
    trace("%s(\"%s\") failed: error %lu", call, path ? path : "", (unsigned long)error);
    SetLastError(error);
}

static DWORD errorOf(const std::error_code &error)
{
    if (error == std::errc::no_such_file_or_directory)
        return ERROR_FILE_NOT_FOUND;
    if (error == std::errc::permission_denied || error == std::errc::read_only_file_system)
        return ERROR_ACCESS_DENIED;
    if (error == std::errc::file_exists)
        return ERROR_ALREADY_EXISTS;
    if (error == std::errc::directory_not_empty)
        return ERROR_DIR_NOT_EMPTY;
    if (error == std::errc::no_space_on_device)
        return ERROR_DISK_FULL;
    return ERROR_INVALID_FUNCTION;
}

static DWORD errorOfErrno()
{
    return errorOf(std::error_code(errno, std::generic_category()));
}

/* Whether a path's directory exists (for telling ERROR_PATH_NOT_FOUND from
   ERROR_FILE_NOT_FOUND). */
static bool parentExists(const std::string &host)
{
    std::error_code error;
    return fs::is_directory(fs::path(host).parent_path(), error);
}

struct FileObject : KernelObject
{
    FILE *file;
    std::string path;
    bool written;
    bool deleteOnClose;
    FileObject() : KernelObject(OBJECT_FILE), file(0), written(false), deleteOnClose(false) {}
};

struct FindObject : KernelObject
{
    std::vector<WIN32_FIND_DATA> found;
    size_t next;
    FindObject() : KernelObject(OBJECT_FIND), next(0) {}
};

KernelObject *kernelObjectOf(HANDLE handle, int type)
{
    KernelObject *object = (KernelObject *)handle;

    if (!handle || handle == INVALID_HANDLE_VALUE || object->magic != 0x4d574b4f
        || (type && object->type != type))
        return 0;
    return object;
}

bool closeFileObject(KernelObject *object)
{
    if (object->type == OBJECT_FILE) {
        FileObject *file = (FileObject *)object;
        fclose(file->file);
        if (file->deleteOnClose) {
            std::error_code error;
            fs::remove(file->path, error);
        }
        if (file->written || file->deleteOnClose)
            hostFilesChanged();
        delete file;
        return true;
    }
    if (object->type == OBJECT_FIND) {
        delete (FindObject *)object;
        return true;
    }
    return false;
}

HANDLE CreateFile(LPCSTR name, DWORD access, DWORD, LPSECURITY_ATTRIBUTES, DWORD creation,
                  DWORD flags, HANDLE)
{
    std::string host;
    std::error_code error;

    if (!strncmp(name, "\\\\.\\", 4)) {
        SetLastError(ERROR_FILE_NOT_FOUND); /* no devices (the game probes VWIN32) */
        return INVALID_HANDLE_VALUE;
    }
    if (!hostPath(name, host)) {
        fail("CreateFile", name, ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    bool write = (access & GENERIC_WRITE) != 0;
    bool exists = fs::exists(host, error);
    if (fs::is_directory(host, error)) {
        fail("CreateFile", name, ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
    }
    if (write && isCdrom(fullPath(name)[0])) {
        fail("CreateFile", name, exists ? ERROR_ACCESS_DENIED : ERROR_WRITE_PROTECT);
        return INVALID_HANDLE_VALUE;
    }
    const char *mode = 0;
    switch (creation) {
    case CREATE_NEW:
        if (exists) {
            fail("CreateFile", name, ERROR_FILE_EXISTS);
            return INVALID_HANDLE_VALUE;
        }
        mode = "w+b";
        break;
    case CREATE_ALWAYS:
        mode = "w+b";
        break;
    case OPEN_EXISTING:
    case TRUNCATE_EXISTING:
        if (!exists) {
            fail("CreateFile", name, parentExists(host) ? ERROR_FILE_NOT_FOUND : ERROR_PATH_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
        }
        mode = creation == TRUNCATE_EXISTING ? "w+b" : write ? "r+b" : "rb";
        break;
    case OPEN_ALWAYS:
        mode = exists ? (write ? "r+b" : "rb") : "w+b";
        break;
    default:
        fail("CreateFile", name, ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    FILE *file = ::fopen(host.c_str(), mode);
    if (!file) {
        fail("CreateFile", name, errno == ENOENT && !parentExists(host) ? ERROR_PATH_NOT_FOUND
                                                             : errorOfErrno());
        return INVALID_HANDLE_VALUE;
    }
    FileObject *object = new FileObject;
    object->file = file;
    object->path = host;
    object->written = mode[0] == 'w';
    object->deleteOnClose = (flags & FILE_FLAG_DELETE_ON_CLOSE) != 0;
    SetLastError(creation == OPEN_ALWAYS && exists ? ERROR_ALREADY_EXISTS : 0);
    return object;
}

static FileObject *fileOf(HANDLE handle)
{
    FileObject *file = (FileObject *)kernelObjectOf(handle, OBJECT_FILE);
    if (!file)
        SetLastError(ERROR_INVALID_HANDLE);
    return file;
}

BOOL ReadFile(HANDLE handle, LPVOID buffer, DWORD size, LPDWORD read, LPOVERLAPPED)
{
    FileObject *file = fileOf(handle);

    if (read)
        *read = 0;
    if (!file)
        return FALSE;
    size_t count = fread(buffer, 1, size, file->file);
    if (read)
        *read = (DWORD)count;
    if (count < size && ferror(file->file)) {
        clearerr(file->file);
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    clearerr(file->file);
    return TRUE;
}

BOOL WriteFile(HANDLE handle, LPCVOID buffer, DWORD size, LPDWORD written, LPOVERLAPPED)
{
    FileObject *file = fileOf(handle);

    if (written)
        *written = 0;
    if (!file)
        return FALSE;
#ifdef __EMSCRIPTEN__
    /* Under Node on Linux, fwrite on the async worker's fiber sometimes never returns (it spins
       in musl's __stdio_write, writing nothing; whether it does depends on where the heap put
       the FILE), which hung practice mode's first save of its temporary roster. Writing the
       same bytes with write(2) doesn't. The cause isn't known. */
    fflush(file->file);
    size_t count = 0;
    while (count < size) {
        ssize_t n = ::write(fileno(file->file), (const char *)buffer + count, size - count);
        if (n <= 0)
            break;
        count += (size_t)n;
    }
#else
    size_t count = fwrite(buffer, 1, size, file->file);
#endif
    file->written = true;
    if (written)
        *written = (DWORD)count;
    if (count < size) {
        SetLastError(ERROR_DISK_FULL);
        return FALSE;
    }
    return TRUE;
}

DWORD SetFilePointer(HANDLE handle, LONG distance, LPLONG distanceHigh, DWORD method)
{
    FileObject *file = fileOf(handle);
    static const int whence[] = {SEEK_SET, SEEK_CUR, SEEK_END};

    if (!file || method > FILE_END)
        return 0xffffffff;
    if (distanceHigh)
        *distanceHigh = 0;
    if (fseek(file->file, distance, whence[method])) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0xffffffff;
    }
    return (DWORD)ftell(file->file);
}

DWORD GetFileSize(HANDLE handle, LPDWORD high)
{
    FileObject *file = fileOf(handle);

    if (high)
        *high = 0;
    if (!file)
        return 0xffffffff;
    long at = ftell(file->file);
    fseek(file->file, 0, SEEK_END);
    long size = ftell(file->file);
    fseek(file->file, at, SEEK_SET);
    return (DWORD)size;
}

BOOL SetEndOfFile(HANDLE handle)
{
    FileObject *file = fileOf(handle);
    std::error_code error;

    if (!file)
        return FALSE;
    fflush(file->file);
    long at = ftell(file->file);
    fs::resize_file(file->path, (uintmax_t)at, error);
    if (error) {
        SetLastError(errorOf(error));
        return FALSE;
    }
    file->written = true;
    return TRUE;
}

static bool writable(const char *name)
{
    if (isCdrom(fullPath(name)[0])) {
        SetLastError(ERROR_WRITE_PROTECT);
        return false;
    }
    return true;
}

BOOL DeleteFile(LPCSTR name)
{
    std::string host;
    std::error_code error;

    if (!writable(name))
        return FALSE;
    if (!hostPath(name, host) || !fs::is_regular_file(host, error)) {
        fail("DeleteFile", name, ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    if (!fs::remove(host, error)) {
        fail("DeleteFile", name, errorOf(error));
        return FALSE;
    }
    hostFilesChanged();
    return TRUE;
}

BOOL CreateDirectory(LPCSTR name, LPSECURITY_ATTRIBUTES)
{
    std::string host;
    std::error_code error;

    if (!writable(name))
        return FALSE;
    if (!hostPath(name, host)) {
        fail("CreateDirectory", name, ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    if (fs::exists(host, error)) {
        fail("CreateDirectory", name, ERROR_ALREADY_EXISTS);
        return FALSE;
    }
    if (!fs::create_directory(host, error)) {
        fail("CreateDirectory", name, error ? errorOf(error) : ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    hostFilesChanged();
    return TRUE;
}

BOOL RemoveDirectory(LPCSTR name)
{
    std::string host;
    std::error_code error;

    if (!writable(name))
        return FALSE;
    if (!hostPath(name, host) || !fs::is_directory(host, error)) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    if (!fs::remove(host, error)) {
        SetLastError(errorOf(error));
        return FALSE;
    }
    hostFilesChanged();
    return TRUE;
}

static DWORD attributesOf(const std::string &host, char drive)
{
    std::error_code error;
    fs::file_status status = fs::status(host, error);

    if (error || !fs::exists(status))
        return 0xffffffff;
    DWORD attributes = 0;
    if (fs::is_directory(status))
        attributes |= FILE_ATTRIBUTE_DIRECTORY;
    if (isCdrom(drive) || (status.permissions() & fs::perms::owner_write) == fs::perms::none)
        attributes |= FILE_ATTRIBUTE_READONLY;
    return attributes ? attributes : FILE_ATTRIBUTE_NORMAL;
}

DWORD GetFileAttributes(LPCSTR name)
{
    std::string host;

    if (!hostPath(name, host)) {
        fail("GetFileAttributes", name, ERROR_PATH_NOT_FOUND);
        return 0xffffffff;
    }
    DWORD attributes = attributesOf(host, fullPath(name)[0]);
    if (attributes == 0xffffffff)
        fail("GetFileAttributes", name, parentExists(host) ? ERROR_FILE_NOT_FOUND : ERROR_PATH_NOT_FOUND);
    return attributes;
}

/* Only the read-only attribute means anything here. */
BOOL SetFileAttributes(LPCSTR name, DWORD attributes)
{
    std::string host;
    std::error_code error;

    if (!writable(name))
        return FALSE;
    if (!hostPath(name, host) || !fs::exists(host, error)) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    fs::permissions(host, fs::perms::owner_write,
                    attributes & FILE_ATTRIBUTE_READONLY ? fs::perm_options::remove
                                                         : fs::perm_options::add,
                    error);
    return TRUE;
}

/* A wildcard match, without regard to case: * and ?, and "*.*" matches
   names without a dot too (as in DOS). */
static bool matches(const char *pattern, const char *name)
{
    if (!strcmp(pattern, "*.*"))
        return true;
    while (*pattern) {
        if (*pattern == '*') {
            pattern++;
            do
                if (matches(pattern, name))
                    return true;
            while (*name++);
            return false;
        }
        if (!*name)
            return !strcmp(pattern, ".") || !strcmp(pattern, ".*");
        if (*pattern != '?' && tolower((unsigned char)*pattern) != tolower((unsigned char)*name))
            return false;
        pattern++;
        name++;
    }
    return !*name;
}

static void describe(WIN32_FIND_DATA *found, const std::string &name, const std::string &host,
                     char drive)
{
    std::error_code error;

    memset(found, 0, sizeof *found);
    found->dwFileAttributes = attributesOf(host, drive);
    if (fs::is_regular_file(host, error))
        found->nFileSizeLow = (DWORD)fs::file_size(host, error);
    strncpy(found->cFileName, name.c_str(), MAX_PATH - 1);
}

HANDLE FindFirstFile(LPCSTR pattern, LPWIN32_FIND_DATA found)
{
    std::string full = fullPath(pattern);
    size_t slash = full.rfind('\\');
    std::string directory = full.substr(0, slash == 2 ? 3 : slash);
    std::string wildcard = full.substr(slash + 1);
    std::string host;
    std::error_code error;

    if (!hostPath(directory.c_str(), host) || !fs::is_directory(host, error)) {
        fail("FindFirstFile", pattern, ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    FindObject *find = new FindObject;
    WIN32_FIND_DATA entry;
    if (directory.size() > 3)
        for (const char *dots : {".", ".."})
            if (matches(wildcard.c_str(), dots)) {
                describe(&entry, dots, host, full[0]);
                entry.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                find->found.push_back(entry);
            }
    for (const auto &item : fs::directory_iterator(host, error)) {
        std::string name = item.path().filename().string();
        if (matches(wildcard.c_str(), name.c_str())) {
            describe(&entry, name, item.path().string(), full[0]);
            find->found.push_back(entry);
        }
    }
    if (find->found.empty()) {
        delete find;
        fail("FindFirstFile", pattern, ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    *found = find->found[find->next++];
    return find;
}

BOOL FindNextFile(HANDLE handle, LPWIN32_FIND_DATA found)
{
    FindObject *find = (FindObject *)kernelObjectOf(handle, OBJECT_FIND);

    if (!find) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if (find->next >= find->found.size()) {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }
    *found = find->found[find->next++];
    return TRUE;
}

BOOL FindClose(HANDLE handle)
{
    KernelObject *find = kernelObjectOf(handle, OBJECT_FIND);

    if (!find)
        return FALSE;
    return closeFileObject(find);
}

DWORD GetCurrentDirectory(DWORD size, LPSTR buffer)
{
    if (currentDirectory.size() + 1 > size)
        return (DWORD)currentDirectory.size() + 1;
    strcpy(buffer, currentDirectory.c_str());
    return (DWORD)currentDirectory.size();
}

BOOL SetCurrentDirectory(LPCSTR path)
{
    std::string full = fullPath(path);
    std::string host;
    std::error_code error;

    if (!hostPath(full.c_str(), host) || !fs::is_directory(host, error)) {
        fail("SetCurrentDirectory", path, ERROR_PATH_NOT_FOUND);
        return FALSE;
    }
    if (full.size() > 3 && full.back() == '\\')
        full.pop_back();
    currentDirectory = full;
    return TRUE;
}

DWORD GetTempPath(DWORD size, LPSTR buffer)
{
    static const char temp[] = "C:\\WINDOWS\\TEMP\\";
    std::string host;
    std::error_code error;

    if (hostPath(temp, host))
        fs::create_directories(host, error);
    if (sizeof temp > size)
        return sizeof temp;
    strcpy(buffer, temp);
    return sizeof temp - 1;
}

DWORD GetLogicalDrives()
{
    DWORD mask = 0;

    for (int i = 0; i < 26; i++)
        if (drives[i])
            mask |= 1u << i;
    return mask;
}

UINT GetDriveType(LPCSTR root)
{
    Drive *drive = driveOf(root ? fullPath(root) : currentDirectory);

    if (!drive)
        return DRIVE_NO_ROOT_DIR;
    return drive->cdrom ? DRIVE_CDROM : DRIVE_FIXED;
}

BOOL GetVolumeInformation(LPCSTR root, LPSTR name, DWORD nameSize, LPDWORD serial,
                          LPDWORD maxComponent, LPDWORD flags, LPSTR fileSystem,
                          DWORD fileSystemSize)
{
    Drive *drive = driveOf(root ? fullPath(root) : currentDirectory);

    if (!drive) {
        fail("GetVolumeInformation", root, ERROR_INVALID_DRIVE);
        return FALSE;
    }
    if (name && nameSize) {
        strncpy(name, drive->label.c_str(), nameSize - 1);
        name[nameSize - 1] = 0;
    }
    if (serial)
        *serial = drive->serial;
    if (maxComponent)
        *maxComponent = drive->cdrom ? 110 : 255;
    if (flags)
        *flags = drive->cdrom ? 0 : FS_CASE_IS_PRESERVED;
    if (fileSystem && fileSystemSize) {
        strncpy(fileSystem, drive->cdrom ? "CDFS" : "FAT", fileSystemSize - 1);
        fileSystem[fileSystemSize - 1] = 0;
    }
    return TRUE;
}

BOOL DeviceIoControl(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD returned, LPOVERLAPPED)
{
    if (returned)
        *returned = 0;
    SetLastError(ERROR_INVALID_FUNCTION);
    return FALSE;
}

BOOL CloseHandle(HANDLE handle)
{
    KernelObject *object = kernelObjectOf(handle);

    if (!object) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if (closeFileObject(object) || closeThreadObject(object))
        return TRUE;
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
}

} /* namespace miniwin */
