/*
 * snoids (0x456c00-0x45c0f4): 'Too many snoid NORMAL scripts', 'Zoombini.MHK', name syllables
 */

#include "zoombinis.h"

/* @zoombi32 0x00456c00 */
void resetSnoids()
{
    g_4b7564 = g_4b7558 = 0;
    g_4b7b68 = 0;
    g_4b754a = 0;
    g_4b7562 = g_4b7566 = g_4b7568 = 0;
    g_4b7552 = 0;
    g_4b7554 = 1;
    g_4b755e = 15;
    g_4b7560 = 1;
    fn_45aaff(1);
    g_4b7556 = 0;
}

/* Opens the sound files (with `files`), else loads the Zoombinis' images,
   scripts and hotspot tables. */
/* @zoombi32 0x00456c67 */
void loadSnoids(short files)
{
    if (files) {
        openGameFile(&g_4b7b50, "MidiMPC.MHK");
        openGameFile(&g_4b7b4c, "Zoombini.MHK");
        fn_46be2e(g_4b7b4c);
    } else {
        snoidImages = loadImageBank(3000, &snoidImagesResource);
        snoidImages2 = loadImageBank(3100, &snoidImages2Resource);
        if (!snoidTablesLoaded) {
            loadBaseSnoidScripts();
            snoidTables[0] = loadShortTable(100, &snoidTableResources[0]);
            snoidTables[1] = loadShortTable(101, &snoidTableResources[1]);
            snoidTables[2] = loadShortTable(102, &snoidTableResources[2]);
            snoidTables[3] = loadShortTable(103, &snoidTableResources[3]);
            snoidTablesLoaded = 1;
        }
        snoidImages3 = loadImageBank(3001, &snoidImages3Resource);
    }
}

/* @zoombi32 0x00456d3b */
void closeSnoids()
{
    short saved = fn_46bee9(1);

    if (g_4a4ba0) {
        disposePtr(g_4a4ba0);
        g_4a4ba0 = 0;
    }
    if (snoidTablesLoaded)
        snoidTablesLoaded = 0;
    fn_45bbba(1);
    freeBaseSnoidScripts();
    freeSnoidTables();
    fn_46c602(&snoidImagesResource);
    fn_46c602(&snoidImages2Resource);
    fn_46c602(&snoidImages3Resource);
    fn_46bee9(saved);
    fn_46ca9c(&g_4b7b4c);
    fn_46ca9c(&g_4b7b50);
}

/* Loads a table of big-endian words ('REGS'), swapping them. */
/* @zoombi32 0x00456dbe */
short *loadShortTable(short id, long *resource)
{
    short handle;
    short *at;
    short *data;

    *resource = 0;
    fn_46c4fe(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    handle = fn_46beac(*resource);
    at = (short *)fn_48ea00(handle);
    data = at;
    for (unsigned long size = handleSize(handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
    return data;
}

/* @zoombi32 0x00456e2e */
void freeSnoidTables()
{
    for (short i = 0; i < 4; i++)
        fn_46c602(&snoidTableResources[i]);
}

/* How many Zoombinis' views run with unknownF7 set. */
/* @zoombi32 0x00456e4c */
short countChosenSnoids()
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if ((view->flags & 1) && view->body.running && viewSnoid(view)->unknownF7)
            count++;
    return count;
}

/* How many Zoombinis' views there are. */
/* @zoombi32 0x00456e7f */
short countSnoidViews()
{
    short count = 0;

    for (View *view = viewListEnd(1); view; view = view->next)
        if (view->flags & 1)
            count++;
    return count;
}

/* Loads the Zoombinis' 51 base scripts ('SCRS' 100 on). */
/* @zoombi32 0x00456e9f */
void loadBaseSnoidScripts()
{
    for (short i = 0; i < 51; i++) {
        baseSnoidScriptResources[i] = 0;
        baseSnoidScripts[i] =
            loadSwappedResource(&baseSnoidScriptResources[i], i + 100, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    }
}

/* @zoombi32 0x00456edb */
void freeBaseSnoidScripts()
{
    for (short i = 0; i < 51; i++)
        fn_46c602(&baseSnoidScriptResources[i]);
}

/* Loads `count` Zoombini scripts from id `first` as the first group,
   loading `limit` of them (all if not 1 to count). */
/* @zoombi32 0x00456ef9 */
void loadSnoidScripts(short first, short count, short limit)
{
    short i;

    if (count > 110)
        fatalError("Too many snoid REJECT scripts");
    snoidScriptGroups = 0;
    for (i = 0; i < 2; i++) {
        snoidScriptGroupFirst[i] = 0;
        snoidScriptGroupCount[i] = 0;
    }
    for (i = 0; i < 110; i++) {
        snoidScripts[i] = 0;
        snoidScriptResources[i] = 0;
    }
    if (limit <= 0 || limit > count)
        limit = count;
    for (i = 0; i < limit && i < 110; i++)
        snoidScripts[i] =
            loadSwappedResource(&snoidScriptResources[i], first + i, RESOURCE_TYPE('S', 'C', 'R', 'S'));
    snoidScriptGroupFirst[snoidScriptGroups] = first;
    snoidScriptGroupCount[snoidScriptGroups] = count;
    snoidScriptGroups++;
}

/* @zoombi32 0x00456fd8 */
void addSnoidScripts(short first, short count, short limit)
{
    short i;
    short loaded;

    if (snoidScriptGroups < 2) {
        loaded = 0;
        for (i = 0; i < snoidScriptGroups; i++)
            loaded += snoidScriptGroupCount[i];
        if (loaded) {
            if (count + loaded > 110)
                fatalError("Too many snoid NORMAL scripts");
            if (limit <= 0 || limit > count)
                limit = count;
            for (i = loaded; i - loaded < limit && i < 110; i++)
                snoidScripts[i] = loadSwappedResource(&snoidScriptResources[i], first + i - loaded,
                                                      RESOURCE_TYPE('S', 'C', 'R', 'S'));
            snoidScriptGroupFirst[snoidScriptGroups] = first;
            snoidScriptGroupCount[snoidScriptGroups] = count;
            snoidScriptGroups++;
        }
    }
}

/* A Zoombini script id's index in snoidScripts (-1 if none) and group. */
/* @zoombi32 0x004570b5 */
void findSnoidScript(short id, short *group, short *index)
{
    short loaded;
    short i;

    *index = -1;
    loaded = 0;
    for (i = 0; i < snoidScriptGroups; i++) {
        short first = snoidScriptGroupFirst[i];
        short last = snoidScriptGroupCount[i] + first - 1;

        if (id >= first && id <= last) {
            *group = i;
            *index = loaded += id - first;
            return;
        }
        loaded += snoidScriptGroupCount[i];
    }
}

/* Loads a Zoombini script by id, if it isn't loaded. */
/* @zoombi32 0x00457120 */
void loadSnoidScript(short id)
{
    short loaded;
    short i;
    short index;

    loaded = 0;
    for (i = 0; i < snoidScriptGroups; i++) {
        short first = snoidScriptGroupFirst[i];
        short last = snoidScriptGroupCount[i] + first - 1;

        if (id >= first && id <= last) {
            index = id - first + loaded;
            if (!snoidScripts[index])
                snoidScripts[index] =
                    loadSwappedResource(&snoidScriptResources[index], id, RESOURCE_TYPE('S', 'C', 'R', 'S'));
            return;
        }
        loaded += snoidScriptGroupCount[i];
    }
}

/* @zoombi32 0x004571a8 */
void freeSnoidScripts()
{
    for (short i = 0; i < 110; i++)
        fn_46c602(&snoidScriptResources[i]);
    snoidScriptGroups = 0;
    g_4b7564 = 0;
}

/* Draws a Zoombini's cels. */
/* Functional: the original reads each cel's words as it pushes them. */
/* @zoombi32-functional 0x004571d8 */
void drawSnoid(Snoid *snoid)
{
    short *cel = (short *)snoid->body.cels;

    while (*cel && *cel <= snoidImages->count) {
        unsigned short *image = (unsigned short *)(snoidImages->offsets[*cel++] + (char *)snoidImages);
        short x = *cel++;
        short y = *cel++;

        drawImageData(image, x, y, 8);
    }
}

/* Adds a view for a Zoombini at a place, heading for another, first
   updated at `when`; returns its id. */
/* @zoombi32 0x00457221 */
short placeSnoid(Snoid *snoid, unsigned long when, short x, short y, short targetX, short targetY)
{
    Point saved = *(Point *)&snoid->body.x;
    short id;

    snoid->unknownF1 = 1;
    snoid->unknownF2 = 0;
    snoid->body.x = x;
    snoid->body.y = y;
    snoid->targetX = targetX;
    snoid->targetY = targetY;
    snoid->body.cels[0].image = 0;
    snoid->body.celsEnd = 0;
    snoid->unknownF8 = randomBetween(0, 0x40);
    id = addSnoidView(snoid, 1);
    {
        View *view = findView(id);

        if (view)
            view->nextUpdate = when;
    }
    snoid->unknownF1 = 0;
    *(Point *)&snoid->body.x = saved;
    return id;
}

/* Adds a view for a Zoombini (if it has one); returns its id. */
/* @zoombi32 0x004574ae */
short addSnoidView(Snoid *snoid, short placed)
{
    short id = 0;

    if (snoid->features[3]) {
        for (short i = 0; i < 16; i++)
            snoid->unknownC2[i] = 0;
        id = addView(1, drawSnoidView, updateSnoidView, 0, 6, snoid, 0, 0);
        {
            View *view = findView(id);

            if (view) {
                short action;

                if (!placed)
                    action = 0;
                else
                    action = 7;
                fn_45a75b(viewSnoid(view), action, 0);
                view->nextUpdate = 0;
            }
        }
    }
    return id;
}

/* A Zoombini view's drawing: its cels (from the second bank with
   unknownF4 9), clipped to its clip rectangle if it has one. */
/* Functional: as drawCels. */
/* @zoombi32-functional 0x00457527 */
void drawSnoidView(View *view)
{
    if (view->body.running) {
        if (view->body.clipped) {
            copyRgn(featureClipRgn, currentViewRgn);
            sectRgnWithRect(featureClipRgn, &view->body.clip);
            setClip(featureClipRgn);
        }
        short *cel = (short *)view->body.cels;
        ImageBank *bank = snoidImages;

        if (viewSnoid(view)->unknownF4 == 9)
            bank = snoidImages2;
        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
        if (view->body.clipped)
            setClip(currentViewRgn);
    }
}

/* @zoombi32 0x004572bf */
short fn_4572bf()
{
    int count = 0;
    for (short i = 0; i < *(short *)(g_4a4ba0 + 0xa92e); i++)
        if (*(g_4a4ba0 + 0xa93c + i * 0x13) != 0)
            count++;
    return count;
}

/* @zoombi32 0x00457fbb */
short fn_457fbb()
{
    if (g_4b7b3a)
        return g_4b7b38 + 1;
    return 0;
}

/* @zoombi32 0x0045b39a */
void fn_45b39a(short value)
{
    g_4a4ce6 = value & 3;
}

/* @zoombi32 0x0045bfc0 */
void fn_45bfc0(long value)
{
    g_4b7b68 = value;
}
