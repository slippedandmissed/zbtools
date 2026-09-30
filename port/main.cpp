/*
 * The port's entry point: sets up miniwin's drives from the command line,
 * then runs the game's WinMain.
 *
 *   zoombinis --drive C=<directory> --cdrom D=<directory>[,<label>[,<serial>]]
 *             [--program <Windows path of the program>] [--screenshot <file.bmp>]
 *             [--run-for <milliseconds>]
 *             [-- <game command line>]
 *
 * C: holds the installed game (and what it saves), D: the CD; `uv run port`
 * lays C: out from the user's copy of the game (build/port/data/c/).
 * --screenshot writes the screen to a BMP about once a second (for a headless
 * build, which has no window); --run-for quits after a while (for tests).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "miniwin/internal.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showCommand);

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

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--drive") && i + 1 < argc) {
            addDriveArgument(argv[++i], false);
            drives = true;
        } else if (!strcmp(argv[i], "--cdrom") && i + 1 < argc)
            addDriveArgument(argv[++i], true);
        else if (!strcmp(argv[i], "--screenshot") && i + 1 < argc)
            miniwin::setScreenshotPath(argv[++i]);
        else if (!strcmp(argv[i], "--run-for") && i + 1 < argc)
            runFor = strtoul(argv[++i], 0, 10);
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
    if (runFor)
        miniwin::setRunFor(runFor);
    char *line = strdup(commandLine.c_str());
    int result = WinMain(miniwin::programInstance(), 0, line, SW_SHOWDEFAULT);
    miniwin::shutdown();
    return result;
}
