/*
 * tunnels's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TUNNELS_H
#define TUNNELS_H

/* A record the caves keep a list of (fn_460527), 28 bytes: a view, and
   its kind last; the rest aren't known yet. */
struct TunnelEntry
{
    short view; /* a Zoombini at a door (0 for a remark) */
    short back; /* +2: it is turned back */
    short step; /* +4: of a remark, the part being said (fn_460571) */
    long from; /* +6: where it stood (a Point) */
    short script; /* +0xa: a Zoombini's, as it goes to its door */
    short backScript; /* +0xc: its script for being turned back (8000 on) */
    short speaker; /* +0xe: a remark: the view saying `line` then `lineThen` */
    short line;
    short lineThen;
    short replier; /* +0x14: and the one replying (0 for none) */
    short reply;
    short replyThen;
    short kind; /* +0x1a: the door (1-4) */
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
    short side; /* which way it sends a Zoombini that passes (the first rule:
                   left, else right; the second: top, else bottom) */
    unsigned char count; /* +2: of features */
    unsigned char features[5]; /* +3 */
    unsigned char values[5]; /* +8 */
};

/* The caves' rules (0x4b7f18): one or two. */
struct TunnelRules
{
    short count;
    TunnelRule rules[2]; /* +2 */
};

extern long g_4b7fb4; /* @data 0x4b7fb4: Tunnels.MHK */
extern short g_4b7fb8; /* @data 0x4b7fb8: the scene is open */
extern long g_4a7708; /* @data 0x4a7708 */
extern short g_4b7fd2; /* @data 0x4b7fd2 */
extern short g_4b7fee; /* @data 0x4b7fee */
extern short g_4b7fd4; /* @data 0x4b7fd4 */
extern short g_4b7fd6; /* @data 0x4b7fd6 */
extern short g_4b7fd8; /* @data 0x4b7fd8 */
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
extern unsigned long g_4b80a4; /* @data 0x4b80a4: slots used (allocateSlot) */
extern long g_4b809c; /* @data 0x4b809c */
extern long g_4b80a0; /* @data 0x4b80a0 */
extern short g_4b7548; /* @data 0x4b7548 */
extern long g_4b7544; /* @data 0x4b7544 */
extern short g_4b7fbc; /* @data 0x4b7fbc */
void resetScene8();
extern short g_4b80a8; /* @data 0x4b80a8: the script last shown (debugging) */
void fn_460642(short kind);
short scene8Key(unsigned short key);
void drawTunnelsButtons(View *);
short removeTunnelEntry(TunnelList *list, short view);

short fn_460c41(TunnelRules *rules, short door, Snoid *snoid, unsigned short *first);

void fn_460e3d();
extern Point tunnelPlaces[16]; /* @data 0x4a7730: where the Zoombinis wait */
void fn_460021(short *spot, short side);
void dropFirstTunnelEntry(TunnelList *list);
void sayTunnelRemark();
void fn_45fa80(View *, short event);
void fn_45f9c9();
void fn_45fb50(View *view, short event);
extern Point g_4a78a6[4]; /* @data 0x4a78a6: where fn_45fb50 anchors the first entry's script */
extern Point g_4a7770[16]; /* @data 0x4a7770: the places past door 1 */
extern Point g_4a77b0[16]; /* @data 0x4a77b0: door 4 */
extern Point g_4a77f0[16]; /* @data 0x4a77f0: door 2 */
extern Point g_4a7830[16]; /* @data 0x4a7830: door 3 */
void scene8Frame();
extern short g_4a7888; /* @data 0x4a7888: scene8Frame is running */
extern short turnBacksLeft; /* @data 0x4b7fc0: how many more times a Zoombini can be turned back */
extern short g_4b7fc2; /* @data 0x4b7fc2 */
extern unsigned long g_4b7fe0; /* @data 0x4b7fe0: when to make the next idle remark (view ticks) */
extern short tunnelsSpeakers[4]; /* @data 0x4b7fc4: the four views that make the remarks (fn_460642) */
extern short g_4b7fcc; /* @data 0x4b7fcc: the buttons' view */
extern GroupList tunnelsGroups[1]; /* @data 0x4a76e8 */
void openScene8();
void scene8Clicked(short which);
extern short doorSpeakers[8]; /* @data 0x4a7710: which of tunnelsSpeakers remarks on a Zoombini at a door (by door and result) */
extern short g_4a75d0[10]; /* @data 0x4a75d0: remarks (g_4a75e4 picks) */
extern short g_4a75e8[11]; /* @data 0x4a75e8: (g_4a7600) */
extern short g_4a7604[8]; /* @data 0x4a7604: (g_4a7614) */
extern short g_4a7618[8]; /* @data 0x4a7618: (g_4a7628) */
extern short g_4a762c[7]; /* @data 0x4a762c: (g_4a763c) */
extern short g_4a7640[6]; /* @data 0x4a7640: (g_4a764c) */
extern short g_4a7650[4]; /* @data 0x4a7650: (g_4a7658) */
extern short g_4a765c[6]; /* @data 0x4a765c: (g_4a7668) */
void fn_461135();
void fn_461bec();
void fn_4612b1();
void fn_461e1a(ChosenSnoids *chosen, unsigned long *masks, unsigned long *pair, short pairs, short n);

#endif
