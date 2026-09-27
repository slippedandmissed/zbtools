/*
 * config (0x44695c-0x446bf8): 'Zoombi32.CFG', 'INSTALLFROMDIR', 'Zoombini CD must be inserted in drive.'
 */

#include <string.h>
#include "zoombinis.h"

char installFromDirKey[] = "INSTALLFROMDIR";
char dataDirName[] = "Data\\";
char installToDirKey[] = "INSTALLTODIR";

/* @zoombi32 0x0044695c */
void fn_44695c()
{
    fn_46258a();
}

/* @zoombi32 0x00446962 */
void fn_446962(char *, const char *)
{
}

/*
 * Finds the game's data. Zoombi32.CFG (written by the installer) names the
 * directory the game was installed from (the CD) and the one it was installed
 * to. If the CD's Data directory has Zoombini.mhk the game uses that and sets
 * g_4a3e5c; otherwise it uses the installed copy, and if that's missing too it
 * asks for the CD.
 */
/* @zoombi32 0x00446969 */
void findGameData()
{
    char path[256];

    if (fn_480790(configFileName, "INSTALL", installFromDirKey, installDir, 0x100))
        fatalError("unable to read file Zoombi32.CFG");
    path[0] = 0;
    strcpy(path, installDir);
    strcat(path, dataDirName);
    setDataPath(path);
    strcat(path, "Zoombini.mhk");
    fileSpec archive(path);
    if (!fn_483420(archive)) {
        g_4a3e5c = 1;
    } else {
        if (fn_480790(configFileName, "INSTALL", installToDirKey, installDir, 0x100))
            fatalError("unable to read file Zoombi32.CFG");
        path[0] = 0;
        strcpy(path, installDir);
        setDataPath(path);
        strcat(path, "Zoombini.mhk");
        fileSpec installed(path);
        if (fn_483420(installed))
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
    long file = fn_484b50(firstSpec, 1);
    if (file) {
        useFirst = 1;
        fn_48266c(file, 0);
    } else {
        file = fn_484b50(fallbackSpec, 1);
        if (!file)
            fatalError("Zoombini CD must be inserted in drive.");
        fn_48266c(file, 0);
    }
    return useFirst;
}
