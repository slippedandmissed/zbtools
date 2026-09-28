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

/* One of the caves' two rules (13 bytes): how many features, the features
   and their values; the rest isn't known yet. */
struct TunnelRule
{
    unsigned char count; /* +0 */
    unsigned char features[5]; /* +1 */
    unsigned char values[5]; /* +6 */
    short unknownB; /* +0xb */
};

/* The caves' rules (0x4b7f18). */
struct TunnelRules
{
    short unknown0;
    short unknown2;
    TunnelRule rules[2]; /* +4 */
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
extern TunnelRules g_4b7f18; /* @data 0x4b7f18 */
extern short g_4b808a; /* @data 0x4b808a */
extern short g_4b8088; /* @data 0x4b8088 */
extern short g_4b8096; /* @data 0x4b8096 */
extern short g_4b8092; /* @data 0x4b8092 */
extern short g_4b7fe4; /* @data 0x4b7fe4 */
extern short g_4b7fce; /* @data 0x4b7fce */
extern short g_4b808e; /* @data 0x4b808e */
extern short g_4b8090; /* @data 0x4b8090 */
extern short g_4b808c; /* @data 0x4b808c */
extern short g_4b8086; /* @data 0x4b8086 */
extern short g_4b8084; /* @data 0x4b8084 */
extern short g_4b8082; /* @data 0x4b8082 */
extern short g_4b8080; /* @data 0x4b8080 */
extern short g_4b7fe6[4]; /* @data 0x4b7fe6 */
extern short g_4b7f94[16]; /* @data 0x4b7f94 */
extern short g_4b7f74[16]; /* @data 0x4b7f74 */
extern short g_4b7f54[16]; /* @data 0x4b7f54 */
extern short g_4b7f34[16]; /* @data 0x4b7f34 */
extern short g_4b809a; /* @data 0x4b809a */
extern short g_4b8098; /* @data 0x4b8098 */
extern long g_4b80a4; /* @data 0x4b80a4 */
extern long g_4b809c; /* @data 0x4b809c */
extern long g_4b80a0; /* @data 0x4b80a0 */
extern short g_4b7548; /* @data 0x4b7548 */
extern long g_4b7544; /* @data 0x4b7544 */
extern short g_4b7fbc; /* @data 0x4b7fbc */
void resetScene8();

#endif
