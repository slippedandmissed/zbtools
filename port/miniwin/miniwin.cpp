/*
 * Starting miniwin, and service(): what goes on while the program runs,
 * called from the functions a program waits or polls in.
 */

#include <SDL.h>
#include <stdlib.h>

#include "miniwin/internal.h"

namespace miniwin {

void initGdi();

bool initialize(const char *title)
{
    /* Without a display (a headless build), the game runs unseen. */
#ifdef ZB_HEADLESS
    bool display = false;
    SDL_Init(SDL_INIT_TIMER);
    SDL_SetError("a headless build");
#else
    bool display = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == 0;
#endif
    if (!display)
        trace("no display (%s): running headless", SDL_GetError());
    initThreads();
    initPalette();
    initGdi();
    loadSystemFonts();
    if (display) {
        if (!openScreen(title))
            return false;
        openAudio();
    }
    return true;
}

void shutdown()
{
    closeAudio();
    closeScreen();
    SDL_Quit();
}

/* How often the host gets its turn while the program keeps busy, and how
   long each wait gives it. */
static const DWORD HOST_TURN_MS = 16;

static int inCallbacks;
static DWORD quitAt;

void setRunFor(DWORD ms)
{
    quitAt = now() + ms;
}

bool inBackground()
{
    return inCallbacks > 0;
}

/*
 * The background jobs each guard themselves against running inside
 * themselves (a callback that reads the clock), but not service() as a
 * whole: a callback that waits (a critical section, say) must still let the
 * rest go on, or nothing would ever wake it.
 */
void service(bool waiting)
{
    static bool pumping, timing, mixing;
    static DWORD last, lastTurn;
    DWORD time = now();

    if (time == last && !waiting)
        return;
    last = time;
    if (quitAt && (LONG)(time - quitAt) >= 0) {
        trace("ran for the time asked: quitting");
        presentIfDue(true);
        exit(0);
    }
    if (!pumping) {
        pumping = true;
        pumpEvents();
        pumping = false;
    }
    if (!timing) {
        timing = true;
        inCallbacks++;
        serviceTimers();
        inCallbacks--;
        timing = false;
    }
    if (!mixing) {
        mixing = true;
        inCallbacks++;
        serviceAudio();
        inCallbacks--;
        mixing = false;
    }
    presentIfDue();
    if (waiting) {
        lastTurn = now();
        hostYield(1);
    } else if (now() - lastTurn >= HOST_TURN_MS) {
        lastTurn = now();
        hostYield(0);
    }
}

} /* namespace miniwin */
