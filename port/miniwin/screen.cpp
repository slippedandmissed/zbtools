/*
 * The screen, through SDL: the 640x480 8-bit framebuffer shown through the
 * system palette, scaled to the window (whole multiples where they fit),
 * with the game's cursor drawn in the image (so it scales with it); and
 * SDL's input, turned into window messages and key state.
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "miniwin/internal.h"

namespace miniwin {

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static bool dirty = true;
static DWORD lastPresent;
static unsigned presentedPaletteVersion;
static uint8_t cursorAnd[128], cursorXor[128];
static int cursorHotX, cursorHotY;
static bool cursorShown;
static bool cursorInside = true;
static SDL_Rect viewport;

bool openScreen(const char *title)
{
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        trace("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        trace("SDL_CreateRenderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_ShowCursor(SDL_DISABLE);
    return texture != 0;
}

void closeScreen()
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    texture = 0;
    renderer = 0;
    window = 0;
}

void screenChanged()
{
    dirty = true;
}

void setCursorImage(const uint8_t *andMask, const uint8_t *xorMask, int hotX, int hotY)
{
    memcpy(cursorAnd, andMask, sizeof cursorAnd);
    memcpy(cursorXor, xorMask, sizeof cursorXor);
    cursorHotX = hotX;
    cursorHotY = hotY;
    dirty = true;
}

void setCursorShown(bool shown)
{
    if (shown != cursorShown)
        dirty = true;
    cursorShown = shown;
}

/* The window's pixels for the screen: centred, as big as fits, in whole
   multiples when it's at least twice the size. */
static void placeViewport()
{
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    int scale = std::min(width / SCREEN_WIDTH, height / SCREEN_HEIGHT);
    if (scale >= 2) {
        viewport.w = SCREEN_WIDTH * scale;
        viewport.h = SCREEN_HEIGHT * scale;
    } else if (width * SCREEN_HEIGHT < height * SCREEN_WIDTH) {
        viewport.w = width;
        viewport.h = width * SCREEN_HEIGHT / SCREEN_WIDTH;
    } else {
        viewport.h = height;
        viewport.w = height * SCREEN_WIDTH / SCREEN_HEIGHT;
    }
    viewport.x = (width - viewport.w) / 2;
    viewport.y = (height - viewport.h) / 2;
}

/* A headless build's view of the screen: a BMP, rewritten about once a
   second (setScreenshotPath). */
static std::string screenshotPath;
static DWORD lastScreenshot;

void setScreenshotPath(const char *path)
{
    screenshotPath = path;
}

static void put16(std::vector<uint8_t> &out, unsigned v)
{
    out.push_back((uint8_t)v);
    out.push_back((uint8_t)(v >> 8));
}

static void put32(std::vector<uint8_t> &out, unsigned long v)
{
    put16(out, (unsigned)(v & 0xffff));
    put16(out, (unsigned)(v >> 16));
}

static void writeScreenshot()
{
    std::vector<uint8_t> out;
    unsigned long pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    unsigned long offset = 14 + 40 + 256 * 4;

    out.push_back('B');
    out.push_back('M');
    put32(out, offset + pixels);
    put32(out, 0);
    put32(out, offset);
    put32(out, 40);
    put32(out, SCREEN_WIDTH);
    put32(out, SCREEN_HEIGHT);
    put16(out, 1);
    put16(out, 8);
    put32(out, 0);
    put32(out, pixels);
    put32(out, 2835);
    put32(out, 2835);
    put32(out, 256);
    put32(out, 0);
    for (int i = 0; i < 256; i++) {
        out.push_back(systemPalette[i].peBlue);
        out.push_back(systemPalette[i].peGreen);
        out.push_back(systemPalette[i].peRed);
        out.push_back(0);
    }
    for (int y = SCREEN_HEIGHT - 1; y >= 0; y--)
        out.insert(out.end(), screenBitmap->row(y), screenBitmap->row(y) + SCREEN_WIDTH);
    FILE *file = ::fopen(screenshotPath.c_str(), "wb");
    if (!file)
        return;
    fwrite(out.data(), 1, out.size(), file);
    fclose(file);
}

void presentIfDue(bool force)
{
    DWORD time = now();

    if (!screenshotPath.empty() && (force || time - lastScreenshot >= 1000)) {
        lastScreenshot = time;
        writeScreenshot();
    }
    if (!renderer || (!force && (time - lastPresent < 15 || (!dirty && presentedPaletteVersion == systemPaletteVersion))))
        return;
    lastPresent = time;
    dirty = false;
    presentedPaletteVersion = systemPaletteVersion;

    uint32_t colors[256];
    for (int i = 0; i < 256; i++)
        colors[i] = 0xff000000u | (uint32_t)systemPalette[i].peRed << 16
                    | (uint32_t)systemPalette[i].peGreen << 8 | systemPalette[i].peBlue;
    void *pixels;
    int pitch;
    if (SDL_LockTexture(texture, 0, &pixels, &pitch))
        return;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        const uint8_t *row = screenBitmap->row(y);
        uint32_t *out = (uint32_t *)((uint8_t *)pixels + y * pitch);
        for (int x = 0; x < SCREEN_WIDTH; x++)
            out[x] = colors[row[x]];
    }
    if (cursorShown && cursorInside) {
        /* AND then XOR, as a monochrome cursor is drawn on the screen. */
        for (int cy = 0; cy < 32; cy++) {
            int y = cursorPosition.y - cursorHotY + cy;
            if (y < 0 || y >= SCREEN_HEIGHT)
                continue;
            uint32_t *out = (uint32_t *)((uint8_t *)pixels + y * pitch);
            for (int cx = 0; cx < 32; cx++) {
                int x = cursorPosition.x - cursorHotX + cx;
                if (x < 0 || x >= SCREEN_WIDTH)
                    continue;
                uint8_t bit = (uint8_t)(0x80 >> (cx & 7));
                bool a = (cursorAnd[cy * 4 + cx / 8] & bit) != 0;
                bool b = (cursorXor[cy * 4 + cx / 8] & bit) != 0;
                uint32_t pixel = a ? out[x] : 0xff000000u;
                if (b)
                    pixel ^= 0x00ffffffu;
                out[x] = pixel;
            }
        }
    }
    SDL_UnlockTexture(texture);
    placeViewport();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, 0, &viewport);
    SDL_RenderPresent(renderer);
}

/* Input */

static int virtualKey(SDL_Keycode key)
{
    if (key >= SDLK_a && key <= SDLK_z)
        return 'A' + (key - SDLK_a);
    if (key >= SDLK_0 && key <= SDLK_9)
        return '0' + (key - SDLK_0);
    if (key >= SDLK_F1 && key <= SDLK_F12)
        return VK_F1 + (key - SDLK_F1);
    switch (key) {
    case SDLK_BACKSPACE: return VK_BACK;
    case SDLK_TAB: return VK_TAB;
    case SDLK_RETURN: case SDLK_KP_ENTER: return VK_RETURN;
    case SDLK_LSHIFT: case SDLK_RSHIFT: return VK_SHIFT;
    case SDLK_LCTRL: case SDLK_RCTRL: return VK_CONTROL;
    case SDLK_LALT: case SDLK_RALT: return VK_MENU;
    case SDLK_PAUSE: return VK_PAUSE;
    case SDLK_CAPSLOCK: return VK_CAPITAL;
    case SDLK_ESCAPE: return VK_ESCAPE;
    case SDLK_SPACE: return VK_SPACE;
    case SDLK_PAGEUP: return VK_PRIOR;
    case SDLK_PAGEDOWN: return VK_NEXT;
    case SDLK_END: return VK_END;
    case SDLK_HOME: return VK_HOME;
    case SDLK_LEFT: return VK_LEFT;
    case SDLK_UP: return VK_UP;
    case SDLK_RIGHT: return VK_RIGHT;
    case SDLK_DOWN: return VK_DOWN;
    case SDLK_INSERT: return VK_INSERT;
    case SDLK_DELETE: return VK_DELETE;
    case SDLK_SEMICOLON: return 0xBA;
    case SDLK_EQUALS: return 0xBB;
    case SDLK_COMMA: return 0xBC;
    case SDLK_MINUS: return 0xBD;
    case SDLK_PERIOD: return 0xBE;
    case SDLK_SLASH: return 0xBF;
    case SDLK_BACKQUOTE: return 0xC0;
    case SDLK_LEFTBRACKET: return 0xDB;
    case SDLK_BACKSLASH: return 0xDC;
    case SDLK_RIGHTBRACKET: return 0xDD;
    case SDLK_QUOTE: return 0xDE;
    }
    return 0;
}

static LPARAM mouseLParam()
{
    POINT origin = clientOrigin(mainWindow());
    return MAKELPARAM(cursorPosition.x - origin.x, cursorPosition.y - origin.y);
}

static WPARAM mouseKeys()
{
    WPARAM keys = 0;
    if (keyState[VK_LBUTTON] & 0x80)
        keys |= MK_LBUTTON;
    if (keyState[VK_RBUTTON] & 0x80)
        keys |= MK_RBUTTON;
    if (keyState[VK_MBUTTON] & 0x80)
        keys |= MK_MBUTTON;
    if (keyState[VK_SHIFT] & 0x80)
        keys |= MK_SHIFT;
    if (keyState[VK_CONTROL] & 0x80)
        keys |= MK_CONTROL;
    return keys;
}

static void moveCursor(int windowX, int windowY)
{
    int w, h, rw, rh;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GetRendererOutputSize(renderer, &rw, &rh);
    placeViewport();
    /* Window points to renderer pixels (they differ on high-DPI screens). */
    double px = (double)windowX * rw / (w ? w : 1), py = (double)windowY * rh / (h ? h : 1);
    int x = (int)((px - viewport.x) * SCREEN_WIDTH / viewport.w);
    int y = (int)((py - viewport.y) * SCREEN_HEIGHT / viewport.h);
    cursorInside = x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT;
    SetCursorPos(x, y);
    dirty = true;
}

static void button(Uint8 which, bool down)
{
    static const struct
    {
        int key;
        UINT down, up;
    } buttons[] = {
        {VK_LBUTTON, WM_LBUTTONDOWN, WM_LBUTTONUP},
        {VK_MBUTTON, WM_MBUTTONDOWN, WM_MBUTTONUP},
        {VK_RBUTTON, WM_RBUTTONDOWN, WM_RBUTTONUP},
    };
    if (which < SDL_BUTTON_LEFT || which > SDL_BUTTON_RIGHT)
        return;
    const auto &b = buttons[which - SDL_BUTTON_LEFT];
    keyState[b.key] = down ? 0x80 : 0;
    HWND main = mainWindow();
    if (main && down && !IsIconic(main) && !appIsActive())
        setActive(true); /* a click activates a window */
    if (main && !IsIconic(main)) {
        SendMessage(main, WM_SETCURSOR, (WPARAM)main, MAKELPARAM(HTCLIENT, down ? b.down : b.up));
        postToQueue(main, down ? b.down : b.up, mouseKeys(), mouseLParam());
    } else if (main && down) {
        /* A click on the minimised game brings it back. */
        ShowWindow(main, SW_RESTORE);
    }
}

/* A scripted click (for tests): at a point on the screen, as if the mouse
   had moved there and clicked. */
struct ScriptedClick
{
    DWORD at;
    int x, y;
};

static std::vector<ScriptedClick> scriptedClicks;
static DWORD scriptStart;

void scriptClick(DWORD at, int x, int y)
{
    if (!scriptStart)
        scriptStart = now();
    scriptedClicks.push_back({scriptStart + at, x, y});
}

static void runScript()
{
    DWORD time = now();
    for (size_t i = 0; i < scriptedClicks.size();) {
        ScriptedClick click = scriptedClicks[i];
        if ((LONG)(time - click.at) < 0) {
            i++;
            continue;
        }
        scriptedClicks.erase(scriptedClicks.begin() + i);
        trace("scripted click at %d, %d", click.x, click.y);
        SetCursorPos(click.x, click.y);
        HWND main = mainWindow();
        if (main)
            SendMessage(main, WM_SETCURSOR, (WPARAM)main, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        button(SDL_BUTTON_LEFT, true);
        button(SDL_BUTTON_LEFT, false);
    }
}

static void key(const SDL_KeyboardEvent &event, bool down)
{
    int vk = virtualKey(event.keysym.sym);
    if (!vk)
        return;
    bool wasDown = (keyState[vk] & 0x80) != 0;
    if (down) {
        if (!wasDown)
            keyState[vk] ^= 1; /* toggled */
        keyState[vk] |= 0x80;
    } else
        keyState[vk] &= (BYTE)~0x80;
    HWND main = mainWindow();
    if (!main || IsIconic(main))
        return;
    if (down && !appIsActive())
        setActive(true);
    bool alt = (keyState[VK_MENU] & 0x80) != 0 && vk != VK_MENU;
    UINT message = down ? (alt ? WM_SYSKEYDOWN : WM_KEYDOWN) : (alt ? WM_SYSKEYUP : WM_KEYUP);
    LPARAM lParam = 1 | (LPARAM)(event.keysym.scancode & 0xff) << 16
                    | (alt ? 1L << 29 : 0) | (wasDown ? 1L << 30 : 0) | (down ? 0 : 1L << 31);
    postToQueue(main, message, (WPARAM)vk, lParam);
}

void pumpEvents()
{
    SDL_Event event;

    if (!scriptedClicks.empty())
        runScript();
    while (SDL_PollEvent(&event)) {
        HWND main = mainWindow();
        switch (event.type) {
        case SDL_QUIT:
            if (main)
                PostMessage(main, WM_CLOSE, 0, 0);
            else
                exit(0);
            break;
        case SDL_MOUSEMOTION:
            moveCursor(event.motion.x, event.motion.y);
            if (main && !IsIconic(main)) {
                SendMessage(main, WM_SETCURSOR, (WPARAM)main, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
                postToQueue(main, WM_MOUSEMOVE, mouseKeys(), mouseLParam());
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            moveCursor(event.button.x, event.button.y);
            button(event.button.button, event.type == SDL_MOUSEBUTTONDOWN);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            key(event.key, event.type == SDL_KEYDOWN);
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_EXPOSED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                dirty = true;
                break;
            case SDL_WINDOWEVENT_LEAVE:
                cursorInside = false;
                dirty = true;
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
            case SDL_WINDOWEVENT_MINIMIZED:
                /* Switching away pauses the game, as it did on Windows. */
                memset(keyState, 0, sizeof keyState);
                setActive(false);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_RESTORED:
                if (main && IsIconic(main))
                    ShowWindow(main, SW_RESTORE);
                else
                    setActive(true);
                break;
            }
            break;
        }
    }
}

} /* namespace miniwin */
