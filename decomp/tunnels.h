/*
 * tunnels's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TUNNELS_H
#define TUNNELS_H

/* A record the caves keep a list of (fn_460527), 14 shorts: a view, and
   its kind last; the rest aren't known yet. */
struct TunnelEntry
{
    short view;
    short unknown2[12];
    short kind; /* +0x1a */
};

/* A list of up to five of them. */
struct TunnelList
{
    short count;
    TunnelEntry entries[5];
};

extern long g_4b7fb4; /* @data 0x4b7fb4: Tunnels.MHK */
extern short g_4b7fb8; /* @data 0x4b7fb8: the scene is open */
extern long g_4a7708; /* @data 0x4a7708 */
extern short g_4b7fd2; /* @data 0x4b7fd2 */
extern short g_4b7fee; /* @data 0x4b7fee */
extern short g_4b7fd4; /* @data 0x4b7fd4 */
extern short g_4b7fd6; /* @data 0x4b7fd6 */
extern short g_4b7fd8; /* @data 0x4b7fd8 */
extern short g_4b8000; /* @data 0x4b8000 */
extern short g_4b8004; /* @data 0x4b8004 */
void closeScene8();
void fn_45fa56(View *, short event);
void fn_45fb10(View *, short event);
void fn_460527(TunnelList *list, TunnelEntry entry);
extern TunnelList g_4b7ff0; /* @data 0x4b7ff0 */
extern short g_4b7fd0; /* @data 0x4b7fd0 */
extern short g_4b7fbe; /* @data 0x4b7fbe */
extern short g_4b8094; /* @data 0x4b8094 */
extern short g_4b7fba; /* @data 0x4b7fba: button 2 is live */
extern short g_4b7fda; /* @data 0x4b7fda: button 2 is drawn lit */
extern short g_4b7fdc; /* @data 0x4b7fdc: button 1 has been drawn */
extern SceneButton tunnelsButtons[2]; /* @data 0x4a766c: buttons 1 and 2 */
extern ImageBank *g_4a770c; /* @data 0x4a770c: the buttons' images */
void fn_45faa3(View *, short event);
void fn_45e9b9(View *, short region);
void drawTunnelsButton(short which, short lit, short show);
void fn_4622f5();

#endif
