/*
 * module_485e08 (Mohawk engine): volumes, and the drive table
 */

/* @flags -p -x- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "zoombinis.h"

/* A disk seen in drive `number`. */
/* @zoombi32 0x00485e08 */
__cdecl Volume::Volume(long number, DiskInfo *info)
{
    tag = 0x566f6c6d;
    id = (long)this;
    this->info = *info;
    drive = driveAt(number, 0);
    share = 0;
    if ((next = files.volumes) != 0)
        files.volumes->prev = this;
    files.volumes = this;
}

/* A network share. */
/* @zoombi32 0x00485e62 */
__cdecl Volume::Volume(const char *share, DiskInfo *info)
{
    tag = 0x566f6c6d;
    id = (long)this;
    this->info = *info;
    drive = 0;
    this->share = strdup(share);
    if ((next = files.volumes) != 0)
        files.volumes->prev = this;
    files.volumes = this;
}

/* @zoombi32 0x00485ebb */
__cdecl Volume::~Volume()
{
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
    else
        files.volumes = next;
    if (share)
        free(share);
    tag = 0;
}

/* @zoombi32 0x00485f17 */
void Volume::touch(unsigned long time)
{
    lastUsed = time;
}

/* Not exact: the original keeps the drive number in edi and `info` in esi;
   BCC32 4.5 allocates the registers differently. */
/* @zoombi32 0x00485f2a */
long findVolume(DiskInfo *info, long drive, const char *share)
{
    Drive *in = driveAt(drive, 0);
    Volume *volume;

    for (volume = files.volumes; volume; volume = volume->next)
        if (!stricmp(info->name, volume->info.name) && info->serial == volume->info.serial
            && (!drive && !share || drive && in == volume->drive
                || drive && in->removable && volume->drive && volume->drive->removable
                || share && volume->share && !stricmp(share, volume->share))) {
            setFileError(0);
            return volume->id;
        }
    setFileError(0x2845);
    return 0;
}

/* The volume's root directory ("C:\" or "\\server\share\"). */
/* @zoombi32 0x00486007 */
void Volume::rootPath(char *path)
{
    if (drive)
        sprintf(path, "%c:\\", drive->letter);
    else
        sprintf(path, "\\\\%s\\%s\\", share, info.name);
}

/* Not exact: the original keeps `this` in ebx; BCC32 4.5 uses eax. */
/* @zoombi32 0x00486050 */
short Volume::mount()
{
    if (drive)
        return drive->use(id);
    return setFileError(0);
}

/* The drive numbered `number` (1-based); checked, 0 if there's none. */
/* @zoombi32 0x0048607c */
Drive *driveAt(long number, short check)
{
    long index = number - 1;

    return check ? index >= 0 ? &files.drives->drives[index] : 0 : &files.drives->drives[index];
}

/* @zoombi32 0x004860bc */
void __cdecl Volume::operator delete(void *block)
{
    free(block);
}
