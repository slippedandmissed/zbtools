/*
 * drive (Mohawk engine): drives: what kind each is, locking and
 * ejecting removable media, and making sure the right disk is in
 */

/* @flags -p -x- */

#include <stdio.h>

#include "zoombinis.h"
#include "os_manager.h"
#include "os_threads.h"

/* IOCTL_DISK_GET_MEDIA_TYPES's DISK_GEOMETRY. */
struct MediaType
{
    char unknown0[8];
    DWORD type;
    char unknownC[0xc];
};

/* @zoombi32 0x00482b50 */
__cdecl Drive::~Drive()
{
    if (number) {
        setLocked(0);
        deleteSync(mutex);
    }
}

/* The application was activated or deactivated: media are unlocked while
   it's inactive. */
/* @zoombi32 0x00482b80 */
void Drive::activate(short active)
{
    if (!active && locked) {
        setLocked(0);
        if (volume)
            volumeOf(volume)->touch(currentTimeMs());
    }
}

/* Ejects a CD, and forgets the volume in it. */
/* @zoombi32 0x00482bbc */
void Drive::eject()
{
    DWORD ntReturned;
    DWORD returned;
    char name[0x10];
    DiocRegisters regs;
    HANDLE device;

    setLocked(0);
    if (type == 1) {
        if (systemState.windowsNT) {
            sprintf(name, "\\\\.\\%c:", letter);
            device = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                OPEN_EXISTING, 0, 0);
            if (device != INVALID_HANDLE_VALUE) {
                DeviceIoControl(device, 0x74808, 0, 0, 0, 0, &ntReturned, 0);
                CloseHandle(device);
            }
        } else {
            memset(&regs, 0, sizeof regs);
            regs.eax = 0x440d;
            regs.ebx = drive;
            regs.ecx = 0x849;
            device = CreateFile("\\\\.\\VWIN32", 0, 0, 0, 0, FILE_FLAG_DELETE_ON_CLOSE, 0);
            DeviceIoControl(device, 1, &regs, sizeof regs, &regs, sizeof regs, &returned, 0);
            CloseHandle(device);
        }
    }
    volume = 0;
}

/* Finds out what kind of drive the letter `index` ('A' + index) is. */
/* @zoombi32 0x00482cb3 */
short Drive::init(long index)
{
    DWORD size;
    DWORD returned;
    BOOL ok;
    char root[0x10];
    MediaType media[0x10];
    unsigned char mapInfo[0x10];
    DiocRegisters regs;
    HANDLE device;

    number = 0;
    setFileError(0);
    letter = index + 'A';
    drive = index + 1;
    sprintf(root, "%c:\\", letter);
    switch (GetDriveType(root)) {
    case DRIVE_NO_ROOT_DIR:
        return 1;
    case DRIVE_FIXED:
        type = 2;
        break;
    case DRIVE_REMOTE:
        remote = 1;
        type = 4;
        break;
    case DRIVE_CDROM:
        cdrom = 1;
        removable = 1;
        type = 1;
        break;
    case DRIVE_RAMDISK:
        type = 5;
        break;
    default:
        type = 0;
    case DRIVE_REMOVABLE:
        removable = 1;
        type = 0;
        if (systemState.windowsNT) {
            sprintf(root, "\\\\.\\%c:", letter);
            device = CreateFile(root, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
            DeviceIoControl(device, 0x70c00, 0, 0, media, sizeof media, &size, 0);
            CloseHandle(device);
            {
                /* Anything but unknown, removable (not a floppy) or fixed
                   media is a floppy. */
                MediaType *entry = media;
                if (size)
                    do {
                        if (entry->type != 0 && entry->type != 0xb && entry->type != 0xc) {
                            type = 3;
                            break;
                        }
                        entry++;
                        size -= sizeof *entry;
                    } while (size);
            }
        } else {
            memset(mapInfo, 0, sizeof mapInfo);
            mapInfo[0] = sizeof mapInfo;
            memset(&regs, 0, sizeof regs);
            regs.eax = 0x440d;
            regs.ebx = drive;
            regs.ecx = 0x86f;
            regs.edx = (DWORD)mapInfo;
            regs.flags = 1;
            device = CreateFile("\\\\.\\VWIN32", 0, 0, 0, 0, FILE_FLAG_DELETE_ON_CLOSE, 0);
            ok = DeviceIoControl(device, 1, &regs, sizeof regs, &regs, sizeof regs, &returned, 0);
            CloseHandle(device);
            /* A BIOS unit below 0x80 is a floppy. */
            if (ok && !(regs.flags & 1) && mapInfo[3] <= 0x7f)
                type = 3;
        }
        break;
    }
    mutex = newMutex(1);
    if (!mutex) {
        setFileError(threadError());
        return 1;
    }
    valid = 1;
    number = index + 1;
    setLocked(0);
    return 0;
}

/* Makes sure the volume `id` is in the drive, asking for it if need be. */
/* @zoombi32 0x00482f2c */
short Drive::use(long id)
{
    DiskInfo info;
    FileRequest request;
    Volume *wanted = volumeOf(id);
    unsigned long now;

    if (!wanted)
        return setFileError(0x2870);
    if (volume && id != volume)
        eject();
    now = currentTimeMs();
    if (id == volume && (!removable || locked || now <= wanted->lastUsed + 2000)) {
    found:
        volume = id;
        wanted->touch(now);
        setLocked(1);
        return setFileError(0);
    }
    while (!readInfo(&info)) {
        if (wanted->info.serial == info.serial && !stricmp(wanted->info.name, info.name)) {
            now = currentTimeMs();
            goto found;
        }
        request.kind = 3;
        request.drive = number;
        request.volume = id;
        if (!askFileUser(&request))
            return setFileError(0x284c);
    }
    return files.error;
}

/* Locks the media in the drive, or unlocks it (removable drives, on
   Windows 95; never while the application is inactive). */
/* Not exact: the original keeps `unlock` at ebp-2, BCC32 4.5 at ebp-1. */
/* @zoombi32 0x00483028 */
void Drive::setLocked(short on)
{
    unsigned char unlock; /* the IOCTL's parameter */
    DWORD returned;
    BOOL ok;
    DiocRegisters regs;
    HANDLE device;

    if (!files.active)
        on = 0;
    if (type == 1 && !systemState.windowsNT && on != locked) {
        unlock = on ? 0 : 1;
        memset(&regs, 0, sizeof regs);
        regs.eax = 0x440d;
        regs.ebx = drive;
        regs.ecx = 0x848;
        regs.edx = (DWORD)&unlock;
        regs.flags = 1;
        device = CreateFile("\\\\.\\VWIN32", 0, 0, 0, 0, FILE_FLAG_DELETE_ON_CLOSE, 0);
        ok = DeviceIoControl(device, 1, &regs, sizeof regs, &regs, sizeof regs, &returned, 0);
        CloseHandle(device);
        if (ok && !(regs.flags & 1))
            locked = on;
    }
}

/* @zoombi32 0x00483102 */
void Drive::lock(short on)
{
    if (on)
        waitSync(mutex, -1);
    else
        releaseMutex(mutex);
}

/* Reads the label of the disk in the drive (making one up for a
   non-removable disk without one). */
/* @zoombi32 0x00483127 */
short Drive::readInfo(DiskInfo *info)
{
    char root[4];

    sprintf(root, "%c:\\", letter);
    lock(1);
    readDiskInfo(root, info);
    lock(0);
    if (!files.error) {
        if (!info->name[0] && !removable)
            sprintf(info->name, "NONAME @%c:", letter);
        if (!info->serial && !removable)
            info->serial = letter;
    }
    return files.error;
}
