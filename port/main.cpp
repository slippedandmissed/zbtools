/*
 * The port's entry point: sets up miniwin's drives from the command line,
 * then runs the game's WinMain.
 *
 *   zoombinis --drive C=<directory> --cdrom D=<directory>[,<label>[,<serial>]]
 *             [--program <Windows path of the program>] [--screenshot <file.bmp>]
 *             [--run-for <milliseconds>] [--click <ms>:<x>,<y>[:press|move|release]]...
 *             [--soundfont <file.sf2>] [--record <file.wav>]
 *             [--cmd <debug commands>]... [--script <file>]   (ZB_DEBUG builds)
 *             [-- <game command line>]
 *
 * C: holds the installed game (and what it saves), D: the CD; `uv run port`
 * lays C: out from the user's copy of the game (build/port/data/c/).
 * --screenshot writes the screen to a BMP about once a second (for a headless
 * build, which has no window); --run-for quits after a while and --click
 * clicks at a point on the 640x480 screen, that many ms after starting (for
 * tests; with :press, :move or :release it only presses the button, moves the
 * pointer or releases the button, to script a drag). --soundfont is the General MIDI SoundFont the music plays with
 * (without one, it's silent); --record writes what's played to a WAV file
 * (for tests). --cmd and --script queue the debug tooling's commands (port/debug/).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "miniwin/internal.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showCommand);

#ifdef ZB_DEBUG
extern "C" void zbDebugRun(const char *commands); /* port/debug/zbdebug.cpp */
#endif

static void usage()
{
    fprintf(stderr, "usage: zoombinis --drive C=<directory> --cdrom D=<directory>[,<label>[,<serial>]]\n"
                    "                 [--program <path>] [-- <command line>]\n");
    exit(2);
}

/* LETTER=PATH[,LABEL[,SERIAL]] */
static void addDriveArgument(const char *argument, bool cdrom)
{
    std::string text(argument);
    if (text.size() < 3 || text[1] != '=')
        usage();
    std::string rest = text.substr(2), label = cdrom ? "CDROM" : "DISK", serial = "0";
    size_t comma = rest.find(',');
    std::string path = rest.substr(0, comma);
    if (comma != std::string::npos) {
        std::string more = rest.substr(comma + 1);
        size_t second = more.find(',');
        label = more.substr(0, second);
        if (second != std::string::npos)
            serial = more.substr(second + 1);
    }
    miniwin::addDrive(text[0], path.c_str(), label.c_str(), (DWORD)strtoul(serial.c_str(), 0, 0), cdrom);
}

int main(int argc, char **argv)
{
    std::string commandLine;
    bool drives = false;
    unsigned long runFor = 0;
    const char *soundFont = 0;
    struct Click
    {
        unsigned long at;
        int x, y;
        int action; /* 0 click, 1 press, 2 move, 3 release */
    };
    std::vector<Click> clicks;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--drive") && i + 1 < argc) {
            addDriveArgument(argv[++i], false);
            drives = true;
        } else if (!strcmp(argv[i], "--cdrom") && i + 1 < argc)
            addDriveArgument(argv[++i], true);
        else if (!strcmp(argv[i], "--screenshot") && i + 1 < argc)
            miniwin::setScreenshotPath(argv[++i]);
        else if (!strcmp(argv[i], "--click") && i + 1 < argc) {
            unsigned long at;
            int x, y, action = 0;
            char what[16] = "";
            int got = sscanf(argv[++i], "%lu:%d,%d:%15s", &at, &x, &y, what);
            if (got < 3)
                usage();
            if (got == 4) {
                action = !strcmp(what, "press") ? 1 : !strcmp(what, "move") ? 2 : !strcmp(what, "release") ? 3 : -1;
                if (action < 0)
                    usage();
            }
            clicks.push_back({at, x, y, action});
        } else if (!strcmp(argv[i], "--run-for") && i + 1 < argc)
            runFor = strtoul(argv[++i], 0, 10);
        else if (!strcmp(argv[i], "--soundfont") && i + 1 < argc)
            soundFont = argv[++i];
        else if (!strcmp(argv[i], "--record") && i + 1 < argc)
            miniwin::setRecordPath(argv[++i]);
#ifdef ZB_DEBUG
        else if (!strcmp(argv[i], "--cmd") && i + 1 < argc)
            zbDebugRun(argv[++i]);
        else if (!strcmp(argv[i], "--script") && i + 1 < argc) {
            std::ifstream file(argv[++i], std::ios::binary);
            if (!file) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
            std::stringstream text;
            text << file.rdbuf();
            zbDebugRun(text.str().c_str());
        }
#endif
        else if (!strcmp(argv[i], "--program") && i + 1 < argc)
            miniwin::setProgramPath(argv[++i]);
        else if (!strcmp(argv[i], "--")) {
            for (i++; i < argc; i++)
                commandLine += (commandLine.empty() ? "" : " ") + std::string(argv[i]);
        } else
            usage();
    }
    if (!drives)
        usage();
    if (!miniwin::initialize("Logical Journey of the Zoombinis"))
        return 1;
    if (soundFont)
        miniwin::loadSoundFont(soundFont);
    if (runFor)
        miniwin::setRunFor(runFor);
    for (const Click &click : clicks)
        miniwin::scriptClick(click.at, click.x, click.y, click.action);
    char *line = strdup(commandLine.c_str());
    int result = WinMain(miniwin::programInstance(), 0, line, SW_SHOWDEFAULT);
    miniwin::shutdown();
    return result;
}
