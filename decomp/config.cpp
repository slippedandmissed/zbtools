/*
 * config (0x44695c-0x446bf8): 'Zoombi32.CFG', 'INSTALLFROMDIR', 'Zoombini CD must be inserted in drive.'
 */

#include <string.h>
#include "zoombinis.h"
#include "config.h"
#include "e2memory.h"
#include "loading.h"
#include "mainloop.h"

short dataFromInstallSource = 0;
char installFromDirKey[] = "INSTALLFROMDIR";
char dataDirName[] = "Data\\";
char installToDirKey[] = "INSTALLTODIR";
char configFileName[] = "Zoombi32.CFG";

char installDir[256];

/* @zoombi32 0x0044695c */
void refreshCursor()
{
    setModeCursor();
}

/* @zoombi32 0x00446962 */
void unusedPathHook(char *, const char *)
{
}

/* getIniString with the file as a reference: the calls below pass the file
   name, which makes a temporary fileSpec of it, as in the original. */
inline short getIniString(const fileSpec &file, const char *section, const char *key, char *buffer,
                          unsigned short size)
{
    return getIniString((fileSpec *)&file, section, key, buffer, size);
}

/*
 * Finds the game's data. Zoombi32.CFG (written by the installer) names the
 * directory the game was installed from (the CD) and the one it was installed
 * to. If the CD's Data directory has Zoombini.mhk the game uses that and sets
 * dataFromInstallSource; otherwise it uses the installed copy, and if that's missing too it
 * asks for the CD.
 */
/* @zoombi32 0x00446969 */
void findGameData()
{
    char path[256];

    if (getIniString(configFileName, "INSTALL", installFromDirKey, installDir, 0x100))
        fatalError("unable to read file Zoombi32.CFG");
    path[0] = 0;
    strcpy(path, installDir);
    strcat(path, dataDirName);
    setDataPath(path);
    strcat(path, "Zoombini.mhk");
    fileSpec archive(path);
    if (!fileMissing(archive)) {
        dataFromInstallSource = 1;
    } else {
        if (getIniString(configFileName, "INSTALL", installToDirKey, installDir, 0x100))
            fatalError("unable to read file Zoombi32.CFG");
        path[0] = 0;
        strcpy(path, installDir);
        setDataPath(path);
        strcat(path, "Zoombini.mhk");
        fileSpec installed(path);
        if (fileMissing(installed))
            fatalError("Zoombini CD must be inserted in drive.", path[0]);
    }
}

/*
 * Whether `first` can be opened; if not, `fallback` must be (or the game asks
 * for the CD).
 */
/* @zoombi32 0x00446b48 */
short preferFirstFile(const char *first, const char *fallback)
{
    short useFirst = 0;
    fileSpec firstSpec(first);
    fileSpec fallbackSpec(fallback);
    long file = openFile(&firstSpec, 1);
    if (file) {
        useFirst = 1;
        closeFile(file, 0);
    } else {
        file = openFile(&fallbackSpec, 1);
        if (!file)
            fatalError("Zoombini CD must be inserted in drive.");
        closeFile(file, 0);
    }
    return useFirst;
}
