/*
 * game (0x44b550-0x455530): cheats ('Cheat on ', 'You have entered the psychedelic ZB Zone!'), memory statistics
 */

#include <windows.h>
#include <mmsystem.h>
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

/* A view's update: adds the rectangles g_4a4750 (when g_4b2792 changes)
   and g_4a472c (the first time) to the region to redraw. */
/* @zoombi32 0x0044f180 */
void fn_44f180(View *, short region)
{
    if (g_4b2792) {
        if (!g_4a483e) {
            g_4a483e = 1;
            unionRgnRect(region, &g_4a4750);
        }
    } else if (g_4a483e) {
        g_4a483e = 0;
        unionRgnRect(region, &g_4a4750);
    }
    if (!g_4a4840) {
        g_4a4840 = 1;
        unionRgnRect(region, &g_4a472c);
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
