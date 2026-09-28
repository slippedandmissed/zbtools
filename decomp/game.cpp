/*
 * game (0x44b550-0x455530): cheats ('Cheat on ', 'You have entered the psychedelic ZB Zone!'), memory statistics
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zoombinis.h"

/*
 * The game's part of each pass of the main loop (WinMain registers it with
 * fn_415604; mainLoopEvents calls it): runs the current scene's frame
 * function, and every 12 ticks steps through g_4a4976 (setCursorMode; an
 * animated cursor?).
 */
/* @zoombi32 0x00454f61 */
void gameFrame()
{
    if (currentScene != -1 && scenes[currentScene]->frame) {
        basePort *saved = getPort();
        setPort(workPort);
        scenes[currentScene]->frame();
        setPort(saved);
    }
    if (g_4a4974)
        fn_455023(1);
    else
        fn_455023(0);
    if (g_4b80d2 >= 1) {
        unsigned long now = fn_41571f();
        if (now >= g_4b80d4) {
            g_4b80d4 = now + 12;
            if (g_4b2aee >= 12)
                g_4b2aee = 0;
            setCursorMode(g_4a4976[g_4b2aee]);
            g_4b2aee++;
        }
    }
}

/* @zoombi32 0x00455013 */
short noteOutOfMemory(unsigned long, short)
{
    outOfMemory = 1;
    return 0;
}

/*
 * The program: checks it's the only copy running, starts the engine (each
 * step fatal if it fails), checks for enough memory and for sound devices,
 * finds the game data, opens the 640x480, 256-colour display, loads fonts,
 * cursors and QuickTime, then runs the main loop until it's told to quit.
 * Its messages come from a table of named strings (msg...), not literals.
 * The version check swaps the version's bytes with swapShort (the original
 * evidently had Mac-style byte-swapping helpers; see zoombinis.h).
 */
/* @zoombi32 0x004546f8 */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showCommand)
{
    char osBuffer[0x5f50];
    MemoryInfo memory;
    DisplayMode mode;
    HWND window;
    short i;

    appInstance = instance;
    appPreviousInstance = previous;
    appCommandLine = commandLine;
    appShowCommand = showCommand;
    aboveWindows311 = swapShort((WORD)GetVersion()) > 0x30b;
    atexit(fn_454ca4);

    /* The window class is named after the program's file: if a window of
       that class exists, the game is running already. */
    GetModuleFileName(appInstance, moduleFileName, 0x100);
    window = FindWindow(moduleFileName, 0);
    if (window) {
        ShowWindow(window, SW_SHOWMAXIMIZED);
        SetForegroundWindow(window);
        return 0;
    }
    moduleFileName[0] = 0;
    if (previous)
        return 0;
    if (GlobalFindAtom("Zoombini"))
        return 0;
    instanceAtom = GlobalAddAtom("Zoombini");
    if (*appCommandLine == 'd')
        debugMode = 1;

    initDisplayMode(&mode, 0xffff, 0xffff, -1, 0);
    g_4b2aea = 0;
    quickTimeReady = 0;
    appName = "Zoombini";
    g_4aa42a = 1;
    clockInTicks = 1;
    g_4aa428 = 0;
    g_4a4a0c = 1;
    g_4aa7cc = 0;
    fn_415604(gameFrame);
    fn_4153b0(fn_454caa);
    fn_415a11(fn_44695c);
    fn_456a2f(fn_4625b8);
    g_4b2aec = addModifierKeys(0) != 0x800;

    if (osStartup(instance, osBuffer, sizeof osBuffer))
        fatalError(msgInitOs);
    if (fn_493096())
        fatalError(msgInitTimer);
    if (initMemory(0, 0))
        fatalError(msgInitHeap);
    setGrowProc(noteOutOfMemory);
    unsigned long free = availableVirtualMemory();
    if (free < 0x189c40 || aboveWindows311 && free < 0x389c40)
        fatalError(msgNotEnoughMemory);
    if (aboveWindows311) {
        getMemoryInfo(&memory);
        if (memory.totalPhysical < 0x600000)
            fatalError(msgNotEnoughPhysicalMemory);
    }
    if (initFiles(0))
        fatalError(msgInitFileManager);
    if (fn_480642())
        fatalError(msgInitConfiguration);
    if (initResources())
        fatalError(msgInitResourceManager);
    if (initSound())
        fatalError(msgInitSound);
    if (!waveOutGetNumDevs())
        fatalError(msgNoWaveDevices);
    if (!midiOutGetNumDevs())
        fatalError(msgNoMidiDevices);

    fn_415910();
    fn_446962(g_4b29d4, rosterFileName);
    fn_41f2c8(0, 0);
    strcat(userFileName, ".txt");
    fn_446962(moduleFileName, userFileName);
    findGameData();

    mode.width = 640;
    mode.height = 480;
    mode.palettized = 1;
    mode.colors = 256;
    fn_4144d0(&mode, 1);
    if (instanceAtom) {
        GlobalDeleteAtom(instanceAtom);
        instanceAtom = 0;
    }
    setTakeStatic(0);
    realizePalette(getPortPalette(), 1);

    for (i = 0; i < 3; i++)
        fonts[i] = 0;
    fn_46cb10(&fonts[1], "CornerStone", 13, 0);
    fn_46cb10(&fonts[2], "CornerStone", 18, 0);
    setFont(fonts[1]);

    g_4b754a = 0;
    g_4a4ba0 = (char *)newPtr(0xae05);
    if (!g_4a4ba0)
        fn_41f195(msgOutOfMemory);
    fn_41f6fc(1);
    fn_41f668();
    g_4b0d52 = 0;
    g_4b0d56 = -1;
    g_4b0d54 = -1;
    currentScene = -1;
    fn_46310c();
    fn_456c67(1);

    /* Cursors 1-5 ('CURS' resources). */
    for (i = 0; i < 6; i++) {
        cursors[i] = 0;
        g_4b80c4[i] = 0;
        if (i) {
            fn_46c4fe(&cursors[i], RESOURCE_TYPE('C', 'U', 'R', 'S'), i, 0, 1);
            g_4b80c4[i] = fn_46beac(cursors[i]);
            fn_48ea00(g_4b80c4[i]);
        }
    }
    fn_46be2e(0);

    long quickTimeVersion = 0;
    if (QTInitialize(&quickTimeVersion) || quickTimeVersion < 0x2300)
        fatalError(msgRequiresQuickTime);
    if (qtim_0b())
        fatalError(msgRequiresQuickTime);
    else
        quickTimeReady = 1;

    g_4b0d50 = 0;
    while (mainLoopUpdate() && !g_4b80e0)
        mainLoopEvents();
    fn_454c8e();
    return 0;
}

/* Registered with atexit, which needs the C convention. */
/* @zoombi32 0x00454ca4 */
void __cdecl fn_454ca4()
{
    fn_454caa();
}

/* Picks a free spot (0-19) for something at g_4a4846: finds, for each of
   the 20 places in g_4a47ec, the nearest spot no earlier place has taken,
   then picks the first place without one, from either end at random (-1:
   none). */
/* @zoombi32 0x00450540 */
void fn_450540(short *result)
{
    Point where = g_4a4846;
    short i;
    short skip;
    short spot;
    short j;

    spotTaken(&where, 0, 500);
    for (i = 0; i < 20; i++) {
        skip = 0;
        spot = spotNear(&g_4a47ec[i], 500, skip);
        for (j = 0; spot && j < i; j++)
            if (spot == g_4b75ee[j]) {
                skip++;
                spot = spotNear(&g_4a47ec[i], 500, skip);
                j = 0;
            }
        g_4b75ee[i] = spot;
    }
    spot = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 0; spot == -1 && i <= 19; i++)
            if (!g_4b75ee[i])
                spot = i;
    } else {
        for (i = 19; spot == -1 && i >= 0; i--)
            if (!g_4b75ee[i])
                spot = i;
    }
    *result = spot;
}

/* @zoombi32 0x0045062d */
void fn_45062d(short script)
{
    View *view = findView(g_4b25b0);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, script, 1);
    }
}

/* @zoombi32 0x00450658 */
void fn_450658(short script, short running)
{
    View *view = findView(g_4b25b0);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, running);
        view->flags = 0x4188000;
        view->notify = fn_45174e;
    }
}

/* @zoombi32 0x004506a9 */
void fn_4506a9(short script)
{
    View *view = findView(g_4b25b0);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, 0);
        view->flags = 0x4188000;
    }
}

/* @zoombi32 0x004506f0 */
void fn_4506f0()
{
    View *view;

    if (g_4b26b4 < 3) {
        view = findView(g_4b25a8);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11006, 1);
        }
    }
    if (g_4b26b6 < 3) {
        view = findView(g_4b25aa);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11007, 1);
        }
    }
}

/* @zoombi32 0x0045074d */
void fn_45074d()
{
    View *view;

    view = findView(g_4b25a8);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11006, 0);
    }
    view = findView(g_4b25aa);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11007, 0);
    }
}

/* @zoombi32 0x00450796 */
void fn_450796()
{
    View *view = findView(g_4b2588);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, 11076, 1);
    }
}

/* @zoombi32 0x004507bb */
void fn_4507bb()
{
    View *view = findView(g_4b2588);

    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11076, 0);
    }
}

/* Whether any of the first g_4b2414 entries of g_4b2430 is set (not -1). */
/* @zoombi32 0x0044d102 */
short fn_44d102()
{
    short any = 0;
    short i;

    for (i = 0; i < g_4b2414; i++)
        if (g_4b2430[i] != -1)
            any = 1;
    return any;
}

/* Resets a cell: state 500, no links. */
/* @zoombi32 0x0044d5ad */
void fn_44d5ad(short cell)
{
    short i;

    g_4b1aea[cell].state = 500;
    for (i = 0; i <= 5; i++)
        g_4b1aea[cell].links[i] = -1;
    g_4b2324[cell] = 0;
}

/* Cuts a cell's link in one direction, clearing its bit. */
/* @zoombi32 0x0044dca0 */
void fn_44dca0(short cell, short direction, short bit)
{
    g_4b1aea[cell].links[direction] = -1;
    g_4b2324[cell] |= bit;
    g_4b2324[cell] ^= bit;
}

/* Counts the cells in state 508, and once there are g_4b2414 of them
   (the first time only), plays a sound. */
/* @zoombi32 0x0044e092 */
void fn_44e092()
{
    short count = 0;
    short i;

    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508)
            count++;
    if (!g_4b2542 && count >= g_4b2414) {
        g_4b2540++;
        queueViewSound(randomBetween(20055, 20063), 0);
    }
}

/* Unlinks a cell from its neighbours (clearing the opposite direction's
   bit on each) and resets it. */
/* @zoombi32 0x0044e314 */
void fn_44e314(short cell)
{
    short i;
    short other;

    g_4b1aea[cell].state = 500;
    for (i = 0; i <= 5; i++) {
        switch (i) {
        case 0:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 8;
            g_4b2324[other] ^= 8;
            break;
        case 1:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 0x10;
            g_4b2324[other] ^= 0x10;
            break;
        case 2:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 0x20;
            g_4b2324[other] ^= 0x20;
            break;
        case 5:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 4;
            g_4b2324[other] ^= 4;
            break;
        case 4:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 2;
            g_4b2324[other] ^= 2;
            break;
        case 3:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 1;
            g_4b2324[other] ^= 1;
            break;
        }
        g_4b1aea[cell].links[i] = -1;
    }
    g_4b2324[cell] = 0;
}

/* @zoombi32 0x00451238 */
void fn_451238(short n)
{
    g_4b26cc[n][0] = 0;
    g_4b26cc[n][1] = 0;
    g_4b26cc[n][2] = 0;
    g_4b26cc[n][3] = 0;
}

/* @zoombi32 0x00451276 */
void fn_451276()
{
    short i;

    for (i = 0; i < 8; i++) {
        g_4b26cc[i][0] = 0;
        g_4b26cc[i][1] = 0;
        g_4b26cc[i][2] = 0;
        g_4b26cc[i][3] = 0;
    }
}

/* Starts a Zoombini view's script (see startSnoidScript), moving it into
   `group`, with `notify` if given. */
/* @zoombi32 0x0045170a */
void fn_45170a(short id, short script, short group, ViewNotify notify, char unknownF8)
{
    View *view = findView(id);

    if (view) {
        startSnoidScript(viewSnoid(view), script, 0, unknownF8);
        view->body.group = group;
        if (notify)
            view->notify = notify;
    }
}

/* Records the features of the Zoombini in view `id` as slot n's. */
/* @zoombi32 0x00450d00 */
void fn_450d00(short id, short n)
{
    View *view = findView(id);

    if (view) {
        Snoid *snoid = viewSnoid(view);
        char *features = snoid->features;

        g_4b26cc[n][0] = features[0];
        g_4b26cc[n][1] = features[1];
        g_4b26cc[n][2] = features[2];
        g_4b26cc[n][3] = features[3];
    }
}

/* Deletes the temporary file (ZBtemp), if one was made. */
/* @zoombi32 0x00454f03 */
void fn_454f03()
{
    fileSpec temp("ZBtemp");

    if (g_4a48e8) {
        g_4a48e8 = 0;
        deleteFile(temp);
    }
}

/* @zoombi32 0x00454c8e */
void fn_454c8e()
{
    g_4a48e6 = 1;
    fatalError(0, 0);
}

/* Empties one of the Zoombini views (0 and 1, or 7 and 8): no features. */
/* @zoombi32 0x004511c1 */
void fn_4511c1(short n)
{
    View *view;

    if (n == 0 || n == 1)
        view = findView(g_4b26a6[n]);
    else if (n == 7 || n == 8)
        view = findView(g_4b26ba[n]);
    if (view) {
        view->changed = 1;
        Snoid *snoid = viewSnoid(view);

        snoid->features[0] = 0;
        snoid->features[1] = 0;
        snoid->features[2] = 0;
        snoid->features[3] = 0;
        snoid->unknownF4 = 4;
    }
}

/* Stops and empties the two Zoombini views g_4b26ac. */
/* @zoombi32 0x004512ac */
void fn_4512ac()
{
    ShortRect unused[1];
    short i;
    View *view;

    for (i = 0; i < 2; i++) {
        view = findView(g_4b26ac[i]);
        if (view) {
            view->body.running = 0;
            view->changed = 1;
            Snoid *snoid = viewSnoid(view);

            snoid->features[0] = 0;
            snoid->features[1] = 0;
            snoid->features[2] = 0;
            snoid->features[3] = 0;
            unionRgnRect(removedRgn, &snoid->body.bounds);
        }
    }
}

/* A view's update: adds buttons 2 (when g_4b2792 changes) and 1 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0044f180 */
void fn_44f180(View *, short region)
{
    if (g_4b2792) {
        if (!g_4a483e) {
            g_4a483e = 1;
            unionRgnRect(region, &g_4a4708[2].rect);
        }
    } else if (g_4a483e) {
        g_4a483e = 0;
        unionRgnRect(region, &g_4a4708[2].rect);
    }
    if (!g_4a4840) {
        g_4a4840 = 1;
        unionRgnRect(region, &g_4a4708[1].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x0044f1f2 */
void fn_44f1f2()
{
    if (g_4b2790) {
        g_4b2790 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a47c8);
        fn_46c602(&g_4b2638);
        fn_46c602(&g_4b2650);
        fn_46c602(&g_4b2654);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b278c);
        fadeOutViews();
        fn_4624fc();
    }
}

/* A view's drawing: its cels from the bank g_4b2634, while it runs and
   stands in g_4aa7a8. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x004541bf */
void fn_4541bf(View *view)
{
    if (view->body.running && ptInRect(&g_4aa7a8, *(Point *)&view->body.x)) {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = g_4b2634;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
}

/* The scene to go back to: the current one if it is the town, the camp, a
   waiting place or a puzzle with a party (setting g_4b7562), else 3. */
/* @zoombi32 0x00454c10 */
short fn_454c10()
{
    short scene;

    if (savedScene() == 3 || savedScene() == 4 || savedScene() == 5 || savedScene() == 6 || savedScene() == 1
        || savedScene() >= 7 && savedScene() <= 18 && party()->count > 0) {
        scene = savedScene();
        g_4b7562 = 1;
    } else {
        scene = 3;
    }
    return scene;
}

/* Stands each Zoombini on a cell in state 508 at its cell. */
/* @zoombi32 0x0044e0e2 */
void fn_44e0e2()
{
    Point where;
    short i;
    View *view;

    g_4b2540 = 0;
    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508) {
            view = findView(g_4b1aea[i].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            setSnoidAction((Snoid *)&findView(g_4b1aea[i].snoid)->body, 0, &where);
        }
}

/* Stands the placed Zoombinis (g_4b2544) on their cells (state 507). */
/* @zoombi32 0x0044e161 */
void fn_44e161()
{
    Point where;
    short i;
    short cell;
    View *view;

    for (i = 0; i < g_4b2414; i++)
        if (g_4b2544[i].cell) {
            cell = g_4b2544[i].cell;
            view = findView(g_4b1aea[g_4b2544[i].cell].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            view = findView(g_4b2544[i].snoid);
            if (view) {
                setSnoidAction(viewSnoid(view), 0, &where);
                g_4b1aea[cell].state = 507;
                g_4b1aea[cell].snoid = g_4b2544[i].snoid;
            }
        }
}

/* Loads a QuickTime movie from a file (0: failed). */
/* @zoombi32 0x004552fd */
long fn_4552fd(const char *path)
{
    long movie;
    long id;
    long file;
    long error;

    error = qtim_2c(path, &file, 0);
    if (qtim_5e() || error)
        return 0;
    id = 0;
    error = qtim_2a(&movie, file, &id, 0, 0, 0);
    if (qtim_5e() || error)
        return 0;
    error = qtim_02(file);
    if (qtim_5e() || error)
        return 0;
    return movie;
}

/* Stops the movie playing, if one is, and with `shutdown` closes
   QuickTime. */
/* @zoombi32 0x00455273 */
void fn_455273(short shutdown)
{
    if (g_4b2ad4) {
        g_4b2ad6 = 1;
        g_4b2ad4 = 0;
        qtim_31(g_4b2ad8, 0);
        qtim_07(g_4b2ad8);
        setPort(g_4b2ae4);
    }
    if (shutdown && quickTimeReady) {
        quickTimeReady = 0;
        if (g_4b2adc) {
            qtim_37(g_4b2adc);
            g_4b2adc = 0;
        }
        qtim_0c();
        QTTerminate();
    }
}

/* Copies the features of the Zoombini in view `id` to one of the slot
   views (0 and 1, or 7 and 8); for any but slot 0, while g_4b2630 is
   below 3, the Zoombini's view also stops and moves to the slot view's
   place in the list (moveView). */
/* @zoombi32 0x00450c24 */
void fn_450c24(short id, short n)
{
    View *to;
    char *features;
    View *from;

    from = findView(id);
    Snoid *fromSnoid = viewSnoid(from);
    features = fromSnoid->features;
    switch (n) {
    case 0:
        to = findView(g_4b26a6[0]);
        break;
    case 1:
        to = findView(g_4b26a6[1]);
        break;
    case 7:
        to = findView(g_4b26ba[7]);
        break;
    case 8:
        to = findView(g_4b26ba[8]);
        break;
    }
    if (to) {
        to->changed = 1;
        Snoid *snoid = viewSnoid(to);
        char *copy = snoid->features;

        copy[0] = features[0];
        copy[1] = features[1];
        copy[2] = features[2];
        copy[3] = features[3];
        snoid->unknownF4 = 4;
        if (n && g_4b2630 < 3) {
            from->changed = 1;
            from->body.running = 0;
            moveView(from->id, 0, to->id);
        }
    }
}

/* Records the features of the Zoombinis in the views g_4b2776[1-3] as
   slots 1-3's (none if a view is gone). */
/* @zoombi32 0x00450d5d */
void fn_450d5d()
{
    short i;
    View *view;

    for (i = 1; i < 4; i++) {
        view = findView(g_4b2776[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            g_4b26cc[i][0] = snoid->features[0];
            g_4b26cc[i][1] = snoid->features[1];
            g_4b26cc[i][2] = snoid->features[2];
            g_4b26cc[i][3] = snoid->features[3];
        } else {
            g_4b26cc[i][0] = 0;
            g_4b26cc[i][1] = 0;
            g_4b26cc[i][2] = 0;
            g_4b26cc[i][3] = 0;
        }
    }
}

/* The same for slots 4-6. */
/* @zoombi32 0x00450df2 */
void fn_450df2()
{
    short i;
    View *view;

    for (i = 4; i < 7; i++) {
        view = findView(g_4b2776[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            g_4b26cc[i][0] = snoid->features[0];
            g_4b26cc[i][1] = snoid->features[1];
            g_4b26cc[i][2] = snoid->features[2];
            g_4b26cc[i][3] = snoid->features[3];
        } else {
            g_4b26cc[i][0] = 0;
            g_4b26cc[i][1] = 0;
            g_4b26cc[i][2] = 0;
            g_4b26cc[i][3] = 0;
        }
    }
}

/* Copies the features of the Zoombinis in the next two views of
   g_4b2604 (from g_4b2734, up to g_4b262e) into g_4b263c; returns how many
   there were. */
/* @zoombi32 0x00452035 */
short fn_452035()
{
    short count = 0;
    View *view;

    if (g_4b2734 < g_4b262e) {
        count++;
        view = findView(g_4b2604[g_4b2734]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            g_4b263c[0] = features[0];
            g_4b263c[1] = features[1];
            g_4b263c[2] = features[2];
            g_4b263c[3] = features[3];
        }
        if (g_4b2734 + 1 < g_4b262e) {
            count++;
            view = findView(g_4b2604[g_4b2734 + 1]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                g_4b263c[4] = features[0];
                g_4b263c[5] = features[1];
                g_4b263c[6] = features[2];
                g_4b263c[7] = features[3];
            }
        } else {
            g_4b263c[4] = 0;
            g_4b263c[5] = 0;
            g_4b263c[6] = 0;
            g_4b263c[7] = 0;
        }
    }
    return count;
}

/* Starts view g_4b25ac's script (11036 on, by g_4b266c) and, with it, the
   Zoombini in view g_4b26b2 (script 12020 on, from g_4a44ac), grouping
   them. */
/* @zoombi32 0x0045162e */
void fn_45162e(short)
{
    View *view;
    View *other;

    view = findView(g_4b25ac);
    if (view) {
        setViewScript(view, g_4b266c + 11036, 1);
        view->notify = fn_45174e;
        moveView(view->id, 0, g_4b25ae);
    }
    other = findView(g_4b26b2);
    if (other && view) {
        Snoid *snoid = viewSnoid(other);

        snoid->unknownF2 = 1;
        *(Point *)&snoid->body.x = g_4a44ac;
        startSnoidScript(viewSnoid(other), g_4b266c + 12020, 0, 0);
        other->notify = fn_45174e;
        moveView(other->id, 1, view->id);
    }
    if (other)
        groupViews(other->id, view->id, 0, 0, 0, 0);
    else
        groupViews(view->id, view->id, 0, 0, 0, 0);
}

/* Whether two of the Zoombinis (g_4b2430) share a feature; g_4b2516 is
   set to the first they share (0-3). */
/* @zoombi32 0x0044cd71 */
short fn_44cd71(short first, short second)
{
    short hair;
    short eyes;
    short nose;
    short feet;
    short hair2;
    short eyes2;
    short nose2;
    short feet2;

    {
        Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[first]])->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    {
        Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[second]])->body;

        hair2 = snoid->features[0];
        eyes2 = snoid->features[1];
        nose2 = snoid->features[2];
        feet2 = snoid->features[3];
    }
    if (hair == hair2) {
        g_4b2516 = 0;
        return 1;
    }
    if (eyes == eyes2) {
        g_4b2516 = 1;
        return 1;
    }
    if (nose == nose2) {
        g_4b2516 = 2;
        return 1;
    }
    if (feet2 == feet) {
        g_4b2516 = 3;
        return 1;
    }
    return 0;
}

/* Picks a random Zoombini of the views g_4b2604 and copies its features
   into g_4b263c, and from g_4b2630 3 on, the next one's after it; returns
   how many views there were. */
/* @zoombi32 0x00451f4e */
short fn_451f4e()
{
    short i;
    short count;
    short ids[22];
    View *view;

    for (i = 0, count = 0; i < g_4b262e; i++)
        if (g_4b2604[i])
            ids[count++] = g_4b2604[i];
    if (count) {
        i = randomBetween(0, count - 1);
        view = findView(ids[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            g_4b263c[0] = features[0];
            g_4b263c[1] = features[1];
            g_4b263c[2] = features[2];
            g_4b263c[3] = features[3];
        }
        if (g_4b2630 >= 3 && count > 1) {
            i++;
            if (i == count)
                i = 0;
            view = findView(ids[i]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                g_4b263c[4] = features[0];
                g_4b263c[5] = features[1];
                g_4b263c[6] = features[2];
                g_4b263c[7] = features[3];
            }
        } else {
            g_4b263c[4] = 0;
            g_4b263c[5] = 0;
            g_4b263c[6] = 0;
            g_4b263c[7] = 0;
        }
    }
    return count;
}

/* The cheat's message: "You have entered the psychedelic ZB Zone!", in a
   box at g_4a447a. */
/* @zoombi32 0x0044dcdc */
void fn_44dcdc()
{
    ShortRect rect = g_4a447a;
    Color saved;
    char text[64];

    sprintf(text, "You have entered the psychedelic ZB Zone!");
    saved = setForeColor(Color(0x19));
    fillPortRect(Rect(rect), Color(0x1f), 0);
    frameRect(Rect(rect));
    drawText(Rect(rect), 0x22, text, 0xffff);
    setForeColor(saved);
    showRect(&rect);
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   g_4b2792), lit or not, and with `show` shows it. */
/* @zoombi32 0x0044f066 */
void fn_44f066(short which, short lit, short show)
{
    short image = 0;
    short handle;
    ImageBank *bank;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b2792) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        handle = fn_46beac(g_4a47c8);
        lockHandle(handle);
        bank = (ImageBank *)handleData(handle);
        unsigned short *data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);

        drawImageData(data, g_4a4708[which].rect.left, g_4a4708[which].rect.top, 8);
        unlockHandle(handle);
        if (show)
            showRect(&g_4a4708[which].rect);
    }
}

/* Gives the Zoombinis in the views g_4b26ac the features set in slots 0-3
   and 7-4 (in that order, so later slots win), and returns 2 if the two
   then differ, else 0. */
/* @zoombi32 0x004513ac */
short fn_4513ac()
{
    char *first;
    char *second;
    short i;
    View *view;
    short result;

    view = findView(g_4b26ac[0]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->unknownF4 = 4;
        first = snoid->features;
        for (i = 0; i < 4; i++) {
            if (g_4b26cc[i][0])
                first[0] = g_4b26cc[i][0];
            if (g_4b26cc[i][1])
                first[1] = g_4b26cc[i][1];
            if (g_4b26cc[i][2])
                first[2] = g_4b26cc[i][2];
            if (g_4b26cc[i][3])
                first[3] = g_4b26cc[i][3];
        }
    }
    view = findView(g_4b26ac[1]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->unknownF4 = 4;
        second = snoid->features;
        for (i = 7; i > 3; i--) {
            if (g_4b26cc[i][0])
                second[0] = g_4b26cc[i][0];
            if (g_4b26cc[i][1])
                second[1] = g_4b26cc[i][1];
            if (g_4b26cc[i][2])
                second[2] = g_4b26cc[i][2];
            if (g_4b26cc[i][3])
                second[3] = g_4b26cc[i][3];
        }
    }
    result = 0;
    if (second[0] != first[0])
        result = 2;
    else if (second[1] != first[1])
        result = 2;
    else if (second[2] != first[2])
        result = 2;
    else if (second[3] != first[3])
        result = 2;
    return result;
}

/* Starts the scene's next move: view g_4b25a4 or g_4b25a6's script, and
   the pair of views g_4b258c and g_4b258e (scripts from g_4b2714), grouped. */
/* @zoombi32 0x004514f6 */
void fn_4514f6()
{
    View *view;
    View *first;
    View *second;

    g_4b2788 = 0;
    if (!g_4b2742)
        view = findView(g_4b25a4);
    else
        view = findView(g_4b25a6);
    if (view) {
        view->flags = 0x5188000;
        if (g_4b2630 == 1 || g_4b2630 == 2) {
            setViewScript(view, g_4b2724, 0);
        } else {
            g_4b2742 = !g_4b2742;
            setViewScript(view, g_4b2726, 0);
        }
    }
    first = findView(g_4b258c);
    if (first) {
        setViewScript(first, g_4b2714[g_4b273c], 1);
        loadViewSounds(first->id, 1);
        first->notify = fn_45174e;
    }
    second = findView(g_4b258e);
    if (second) {
        setViewScript(second, g_4b2714[g_4b273c + 1], 1);
        second->notify = fn_45174e;
    }
    view = findView(g_4b26b2);
    if (view && second)
        moveView(view->id, 0, second->id);
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Gives the eight Zoombinis in the views g_4b2672 random features at their
   places (g_4a44cc), except that one of the first `count`, at random, gets
   the features set in g_4b263c. */
/* @zoombi32 0x004520ec */
void fn_4520ec(short count)
{
    short j;
    char *features;
    short i;
    Snoid *snoid;
    short chosen;
    View *view;

    if (count) {
        if (count >= 8)
            chosen = randomBetween(0, 7);
        else
            chosen = randomBetween(0, count - 1);
        for (i = 0; i < 8; i++) {
            view = findView(g_4b2672[i]);
            if (view) {
                view->body.running = 1;
                view->changed = 1;
                snoid = (Snoid *)&view->body;
                snoid->unknownF4 = 4;
                *(Point *)&snoid->body.x = g_4a44cc[i];
                features = snoid->features;
                for (j = 0; j < 4; j++)
                    features[j] = randomBetween(1, 4);
                snoid->unknownF1 = 1;
                if (i == chosen) {
                    if (g_4b263c[0])
                        features[0] = g_4b263c[0];
                    else
                        features[0] = randomBetween(1, 4);
                    if (g_4b263c[1])
                        features[1] = g_4b263c[1];
                    else
                        features[1] = randomBetween(1, 4);
                    if (g_4b263c[2])
                        features[2] = g_4b263c[2];
                    else
                        features[2] = randomBetween(1, 4);
                    if (g_4b263c[3])
                        features[3] = g_4b263c[3];
                    else
                        features[3] = randomBetween(1, 4);
                }
            }
        }
    }
}

/* Moves on the one feature (unknownF5) of the Zoombinis in the views
   g_4b2776[1-3] that change it: each takes the next value after its slot's (or
   an earlier slot's, if its is unset), wrapping 5 round to 1, and records
   it in the next slot. */
/* @zoombi32 0x00450e87 */
void fn_450e87()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 3; i++) {
        view = findView(g_4b2776[i + 1]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->unknownF4 = 4;
            if (snoid->unknownF5) {
                switch (i) {
                case 0:
                    snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                case 1:
                    if (g_4b26cc[1][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[1][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                case 2:
                    if (g_4b26cc[2][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[2][snoid->unknownF5 - 1] + 1;
                    else if (g_4b26cc[1][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[1][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->unknownF5 - 1] > 5)
                    snoid->features[snoid->unknownF5 - 1] = 1;
                g_4b26cc[i + 1][snoid->unknownF5 - 1] = snoid->features[snoid->unknownF5 - 1];
                view->unknown1e = snoid->features[snoid->unknownF5 - 1];
                snoid->unknownF8 = 0;
            }
        }
    }
}

/* The same from the other side: the views g_4b2776[6-4], from slots 7-5,
   recording in slots 6-4. */
/* @zoombi32 0x00451020 */
void fn_451020()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 5; i > 2; i--) {
        view = findView(g_4b2776[i + 1]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->unknownF4 = 4;
            if (snoid->unknownF5) {
                switch (i) {
                case 3:
                    if (g_4b26cc[5][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[5][snoid->unknownF5 - 1] + 1;
                    else if (g_4b26cc[6][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[6][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                case 4:
                    if (g_4b26cc[6][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[6][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                case 5:
                    snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->unknownF5 - 1] > 5)
                    snoid->features[snoid->unknownF5 - 1] = 1;
                g_4b26cc[i + 1][snoid->unknownF5 - 1] = snoid->features[snoid->unknownF5 - 1];
                view->unknown1e = snoid->features[snoid->unknownF5 - 1];
                snoid->unknownF8 = 0;
            }
        }
    }
}

/* The scene's keys (with g_4b8803, the cheat keys; else only 0x16f): L shows
   the level, 0x172 and 0x173 turn cheating (g_4b2798) on and off, 0x16f
   calls fn_466b93. Returns whether the key was used. */
/* Not exact: the original keeps `key` in esi and `show` in ebx (saving esi
   around the arrays' initial copies); this swaps them. */
/* @zoombi32 0x00450a58 */
short fn_450a58(unsigned short key)
{
    Color saved;
    char digits[52] = "01234";
    char level[52] = "Level x ";
    short show = 0;
    ShortRect rect = g_4a48b2;
    char text[52];

    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 'L':
        level[6] = digits[g_4b2630];
        strcpy(text, level);
        show = 1;
        key = 1;
        break;
    case ' ':
        key = 1;
        break;
    case 0x172:
        strcpy(text, " Cheat on ");
        show = 1;
        g_4b2798 = 1;
        key = 1;
        break;
    case 0x173:
        show = 0;
        g_4b2798 = 0;
        key = 1;
        break;
    case 0x16f:
        fn_466b93();
        key = 1;
        break;
    default:
        key = 0;
        break;
    }
    if (show) {
        saved = setForeColor(Color(0xb));
        fillPortRect(Rect(rect), Color(0xe), 0);
        drawText(Rect(rect), 0x22, text, 0xffff);
        showRect(&rect);
        setForeColor(saved);
    }
    return key;
}

/* Looks, two cells from `cell` in `direction`, for a waiting Zoombini
   (g_4b2430) that shares a feature with the one on `cell` (starting from a
   random feature); places it on the far cell (state 507) with the middle
   one showing the feature (state 501, view 510-513), and returns its
   index. -2: no such cells; -1: none shares a feature. (It reuses `cell`
   as the index.) */
/* Not exact: the original leaves `direction` on the stack; this gives it
   edi until `differ` takes it. */
/* @zoombi32 0x0044d3b8 */
short fn_44d3b8(short cell, short direction)
{
    short feature;
    short differ;
    short next;
    short further;
    short feet;
    short hair;
    short eyes;
    short nose;
    short hair2;
    short feet2;
    short eyes2;
    short nose2;
    short tries;

    feature = randomUpTo(3);
    next = g_4b1aea[cell].links[direction];
    if (next < 0)
        return -2;
    further = g_4b1aea[next].links[direction];
    if (further < 0)
        return -2;
    {
        Snoid *snoid = (Snoid *)&findView(g_4b1aea[cell].snoid)->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    differ = 1;
    for (cell = 0; cell < g_4b2414; cell++)
        if (g_4b2430[cell] != -1) {
            Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[cell]])->body;

            hair2 = snoid->features[0];
            eyes2 = snoid->features[1];
            nose2 = snoid->features[2];
            feet2 = snoid->features[3];
            tries = 4;
            do {
                if (feature == 0 && hair2 == hair)
                    differ = 0;
                if (feature == 1 && eyes == eyes2)
                    differ = 0;
                if (feature == 2 && nose == nose2)
                    differ = 0;
                if (feature == 3 && feet == feet2)
                    differ = 0;
                if (differ) {
                    tries--;
                    feature++;
                    if (feature > 3)
                        feature = 0;
                }
            } while (differ && tries);
            if (!differ) {
                g_4b1aea[further].state = 507;
                g_4b1aea[further].snoid = partyViews[g_4b2430[cell]];
                g_4b1aea[next].state = 501;
                g_4b1aea[next].snoid = feature + 510;
                g_4b2430[cell] = -1;
                return cell;
            }
        }
    return -1;
}

/* The memory statistics line (with g_4a48e4): free, purgeable and the
   least free seen (g_4a48e0), in thousands; with `clear`, unloads the
   sounds and blanks the line instead. */
/* Not exact: the original gives `port` esi and `freeThousands` ebx (shared
   with `available`); this gives them the other way round. */
/* @zoombi32 0x00455023 */
void fn_455023(short clear)
{
    unsigned long freeUnits;
    unsigned long freeThousands;
    unsigned long purgeableUnits;
    unsigned long purgeableThousands;
    unsigned long leastUnits;
    unsigned long leastThousands;
    Color saved;
    Font *font;
    char text[256];
    unsigned long available;
    unsigned long purgeable;
    basePort *port;

    if (clear) {
        unloadSounds();
        port = getPort();
        setPort(workPort);
        text[0] = 0;
        drawText(Rect(g_4a498e), 0x11, text, 0xffff);
        setPort(port);
    } else if (g_4a48e4) {
        available = availableVirtualMemory();
        purgeable = purgeMemory(0, 0);
        if (available < g_4a48e0)
            g_4a48e0 = available;
        freeUnits = available % 1000;
        freeThousands = available / 1000;
        purgeableUnits = purgeable % 1000;
        purgeableThousands = purgeable / 1000;
        leastUnits = g_4a48e0 % 1000;
        leastThousands = g_4a48e0 / 1000;
        port = getPort();
        setPort(workPort);
        font = getFont();
        setFont(fonts[1]);
        sprintf(text, "Free:%d,%03d  Purg:%d,%03d  Min:%d,%03d", (short)freeThousands, (short)freeUnits,
                (short)purgeableThousands, (short)purgeableUnits, (short)leastThousands, (short)leastUnits);
        fillPortRect(Rect(g_4a498e), Color(0xff), 0);
        saved = setForeColor(Color(0));
        drawText(Rect(g_4a498e), 0x11, text, 0xffff);
        setForeColor(saved);
        setFont(font);
        setPort(port);
        showRect(&g_4a498e);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x0044f163 */
void fn_44f163(View *)
{
    fn_44f066(1, 0, 0);
    fn_44f066(2, 0, 0);
}

/* Lets the movie play (cmgr_09; MCIdle?); once it has stopped (flag 0x40
   clear; mcInfoIsPlaying?), closes it and returns 1. 0: still playing;
   -1: no movie. */
/* @zoombi32 0x00455229 */
short fn_455229()
{
    long flags;

    if (!g_4b2adc)
        return -1;
    cmgr_05(g_4b2adc, &flags);
    cmgr_09(g_4b2adc);
    if (flags & 0x40)
        return 0;
    fn_455273(0);
    return 1;
}

/* Sets the Zoombinis' features from the slots (fn_4513ac; g_4b2740 if the
   two differ) and starts the views g_4b2590 and g_4b2592 (11018, 11019),
   grouped. */
/* @zoombi32 0x00451315 */
void fn_451315()
{
    View *first;
    View *second;

    g_4b273c = fn_4513ac();
    g_4b2740 = 0;
    if (g_4b273c)
        g_4b2740 = 1;
    first = findView(g_4b2590);
    if (first) {
        setViewScript(first, 11018, 1);
        first->notify = fn_45174e;
    }
    second = findView(g_4b2592);
    if (second) {
        setViewScript(second, 11019, 1);
        second->notify = fn_45174e;
    }
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Fills the free cells of a set of 20 with waiting Zoombinis, while any
   are left. */
/* Its loop runs to 22, past the end of the cells, and each Zoombini's view
   is read after its entry in g_4b2430 is cleared (partyViews[-1]); both as
   in the original. */
/* @zoombi32 0x0044d5f5 */
void fn_44d5f5()
{
    short cells[20] = {55, 57, 59, 61, 38, 74, 40, 76, 42, 78, 44, 80, 21, 93, 23, 95, 25, 97, 4, 112};
    short i;
    short j;

    for (i = 0; i < 22; i++)
        if (g_4b1aea[cells[i]].state != 507) {
            g_4b1aea[cells[i]].state = 507;
            for (j = 0; j < g_4b2414; j++)
                if (g_4b2430[j] != -1) {
                    g_4b2430[j] = -1;
                    g_4b1aea[cells[i]].snoid = partyViews[g_4b2430[j]];
                    break;
                }
            if (!fn_44d102())
                break;
        }
}
