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
 * function, and every 12 ticks steps through g_4a4976 (fn_46251c; an
 * animated cursor?).
 */
/* @zoombi32 0x00454f61 */
void gameFrame()
{
    if (currentScene != -1 && scenes[currentScene]->frame) {
        long saved = getPort();
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
            fn_46251c(g_4a4976[g_4b2aee]);
            g_4b2aee++;
        }
    }
}

/* @zoombi32 0x00455013 */
long fn_455013(long, long)
{
    g_4aa4c9 = 1;
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

    fn_48ac68(&mode, 0xffff, 0xffff, -1, 0);
    g_4b2aea = 0;
    quickTimeReady = 0;
    appName = "Zoombini";
    g_4aa42a = 1;
    g_4ab482 = 1;
    g_4aa428 = 0;
    g_4a4a0c = 1;
    g_4aa7cc = 0;
    fn_415604(gameFrame);
    fn_4153b0(fn_454caa);
    fn_415a11(fn_44695c);
    fn_456a2f(fn_4625b8);
    g_4b2aec = addModifierKeys(0) != 0x800;

    if (fn_46ddaf(instance, osBuffer, sizeof osBuffer))
        fatalError(msgInitOs);
    if (fn_493096())
        fatalError(msgInitTimer);
    if (fn_48ec85(0, 0))
        fatalError(msgInitHeap);
    fn_48f2b0(fn_455013);
    unsigned long free = fn_48e7ec();
    if (free < 0x189c40 || aboveWindows311 && free < 0x389c40)
        fatalError(msgNotEnoughMemory);
    if (aboveWindows311) {
        fn_48e928(&memory);
        if (memory.freePhysical < 0x600000)
            fatalError(msgNotEnoughPhysicalMemory);
    }
    if (fn_483732(0))
        fatalError(msgInitFileManager);
    if (fn_480642())
        fatalError(msgInitConfiguration);
    if (fn_4922c6())
        fatalError(msgInitResourceManager);
    if (fn_476d0a())
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
    mode.unknown8 = 1;
    mode.colors = 256;
    fn_4144d0(&mode, 1);
    if (instanceAtom) {
        GlobalDeleteAtom(instanceAtom);
        instanceAtom = 0;
    }
    fn_48da48(0);
    fn_48cab4(fn_48b4a8(), 1);

    for (i = 0; i < 3; i++)
        fonts[i] = 0;
    fn_46cb10(&fonts[1], "CornerStone", 13, 0);
    fn_46cb10(&fonts[2], "CornerStone", 18, 0);
    fn_48d4c4(fonts[1]);

    g_4b754a = 0;
    g_4a4ba0 = (char *)fn_48e6b4(0xae05);
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
