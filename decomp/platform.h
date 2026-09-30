/*
 * platform's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef PLATFORM_H
#define PLATFORM_H

extern short deactivateOnNcActivate; /* @data 0x4a4ad6 */
extern Callback aboutHook; /* @data 0x4a4a14 */
extern long platformPair1; /* @data 0x4a4a18 */
extern long platformPair2; /* @data 0x4a4a1c */
extern char minimumOfText[]; /* @data 0x4a4a20 */
extern char colors256Text[]; /* @data 0x4a4a2e */
extern char svgaRequiredFormat[]; /* @data 0x4a4a39 */
extern char color16Text[]; /* @data 0x4a4a8d */
extern char color24Text[]; /* @data 0x4a4a9a */
extern long savedDisk; /* @data 0x4a4aa8 */
extern unsigned short resolutionWidths[4]; /* @data 0x4a4aac */
extern unsigned short resolutionHeights[4]; /* @data 0x4a4ab4 */
extern char messageLogName[]; /* @data 0x4a4ad8 */
extern unsigned short appActive; /* @data 0x4a4ae4 */
extern ShortRect paletteChartRect; /* @data 0x4a4ae6 */
extern short keepDisplayMode; /* @data 0x4b2b04 */
extern char programPath[0x100]; /* @data 0x4b2b06 */
extern char savedDirectory[]; /* @data 0x4b2c06 */
extern WNDCLASS windowClass; /* @data 0x4b2d06 */
extern short classRegistered; /* @data 0x4b2d2e */
extern short wasActivated; /* @data 0x4b2d30 */
extern short inputIgnored; /* @data 0x4b2d36: keys and clicks are dropped */
extern short pauseLoopRunning; /* @data 0x4b2d3c */
extern short messageLogCount; /* @data 0x4b2d42 */
extern long loggedMessages[0x400]; /* @data 0x4b2d44 */
extern long loggedWParams[0x400]; /* @data 0x4b3d44 */
extern long loggedLParams[0x400]; /* @data 0x4b4d44 */
extern long loggedResults[0x400]; /* @data 0x4b5d44 */
extern short loggedAfter[0x400]; /* @data 0x4b6d44 */
LRESULT CALLBACK mainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void drawPaletteChart();
void mouseButtonDown(short button, long keys, long where);
short handleNextMessage();
void flushInput(short which);
void handleMessagesIgnoringInput();
void activateApp(long active);
void placeGamePort();
void destroyMainWindow();
void showError(const char *prefix, const char *format, va_list args);
void releaseControlKeys();
void gameActivated(short active);
void checkDisplayMode(DisplayMode *mode);
void handleWaitingMessage();
void waitWhilePaused();
short pumpMessage(MSG *message, unsigned short first, unsigned short last, unsigned short flags);
void handleSystemKey(MSG *message);
void handleMessage(MSG *message);
short createMainWindow(short width, short height);
short addModifierKeys(short modifiers);
void getCursorPosition(Point *where);
void setCursorPosition(short x, short y);
short isButtonStillDown(unsigned short button);
short allocateBlock(void **block, unsigned long size);
void getClockTime(char *hour, char *minute, char *second);
void enterProgramDirectory();
void restoreDirectory();
void brightenPalette(PALETTEENTRY *entries, short first, short count);
short isInputWaiting(short which);
void logMessage(long message, long wParam, long lParam, short after, long result);
void dumpMessages();
int isMousePresent();
void freeAndClear(void **block);
char *intToDecimal(int value, char *buffer);
char *unsignedToDecimal(unsigned long value, char *buffer);
void unusedPlatformHook1(long);
void unusedPlatformHook2(long);
short platformHandlesMouse(Point *where, short button);
void setAboutHook(Callback callback);
short realizeFullScreenPalette();
void setPlatformPair(long first, long second);
void setGameActivateHook(void (*callback)(short active));
short isWindowed();

#endif
