/*
 * tunnels's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TUNNELS_H
#define TUNNELS_H

/* A record of 14 shorts the caves keep a list of (fn_460527); its fields
   aren't known yet. */
struct TunnelEntry
{
    short fields[14];
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

#endif
