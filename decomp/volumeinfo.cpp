/*
 * volumeinfo (Mohawk engine): volumes' information, and finding the
 * volume in a drive
 */

/* @flags -p -x- */

#include <string.h>

#include "zoombinis.h"

/* @zoombi32 0x004849ac */
short volumeInfo(LONG_PTR id, VolumeInfo *info)
{
    Volume *volume = volumeOf(id);

    if (!volume)
        return setFileError(0x2870);
    memset(info, 0, sizeof *info);
    strcpy(info->name, volume->info.name);
    info->maxPath = volume->info.maxPath;
    info->maxName = volume->info.maxName;
    info->async = 1;
    info->casePreserved = volume->info.casePreserved;
    info->caseSensitive = volume->info.caseSensitive;
    if (volume->drive) {
        info->drive = volume->drive->number;
        info->readOnly = volume->drive->cdrom;
    } else
        info->share = volume->share;
    return setFileError(0);
}

/* Reads the label of the disk in a drive and adds it as a volume (unless
   it's known already, or one is known to be in the drive). */
/* @zoombi32 0x00484a4c */
LONG_PTR mountDrive(long number)
{
    DiskInfo info;
    Drive *drive = driveAt(number, 1);

    if (!drive) {
        setFileError(0x2870);
        return 0;
    }
    if (drive->removable && !drive->locked)
        drive->volume = 0;
    else if (drive->volume) {
        setFileError(0x284b);
        return 0;
    }
    if (drive->readInfo(&info))
        return 0;
    {
        LONG_PTR id = findVolume(&info, number, 0);
        if (id) {
            volumeOf(id)->mount();
            setFileError(0x284b);
            return 0;
        }
    }
    {
        Volume *volume = new Volume(number, &info);
        if (!volume) {
            setFileError(0x2846);
            return 0;
        }
        if (volume->mount()) {
            delete volume;
            return 0;
        }
        setFileError(0);
        return volume->id;
    }
}
