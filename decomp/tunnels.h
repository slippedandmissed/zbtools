/*
 * tunnels's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef TUNNELS_H
#define TUNNELS_H

/* A record the caves keep a list of (addTunnelEntry), 28 bytes: a view, and
   its kind last; the rest aren't known yet. */
struct TunnelEntry
{
    short view; /* a Zoombini at a door (0 for a remark) */
    short back; /* +2: it is turned back */
    short step; /* +4: of a remark, the part being said (sayTunnelRemark) */
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
extern long tunnelsFile; /* @data 0x4b7fb4: Tunnels.MHK */
extern short tunnelsOpen; /* @data 0x4b7fb8: the scene is open */
extern long tunnelsButtonResource; /* @data 0x4a7708 */
extern short entryUnderway; /* @data 0x4b7fd2 */
extern short closingStep; /* @data 0x4b7fee */
extern short followingSpeaker; /* @data 0x4b7fd4 */
extern short followingLine; /* @data 0x4b7fd6 */
extern short followingDropsEntry; /* @data 0x4b7fd8 */
void closeTunnels();
void remarkEndNotify(View *, short event);
void firstLineNotify(View *, short event);
void addTunnelEntry(TunnelList *list, TunnelEntry entry);
extern TunnelList tunnelQueue; /* @data 0x4b7ff0 */
extern short closingRemarkDone; /* @data 0x4b7fd0 */
extern short tunnelsLevel; /* @data 0x4b7fbe */
extern short tunnelsPartySize; /* @data 0x4b8094 */
extern short tunnelsGoReady; /* @data 0x4b7fba: button 2 is live */
extern short tunnelsButton2Lit; /* @data 0x4b7fda: button 2 is drawn lit */
extern short tunnelsButton1Drawn; /* @data 0x4b7fdc: button 1 has been drawn */
extern SceneButton tunnelsButtons[3]; /* @data 0x4a766c: buttons 1 and 2, then the whole screen (the input group's items) */
extern ImageBank *tunnelsButtonImages; /* @data 0x4a770c: the buttons' images */
void tunnelRemarkNotify(View *, short event);
void updateTunnelsButtons(View *, short region);
void drawTunnelsButton(short which, short lit, short show);
void unghostDoorView();
extern FeatureRules tunnelRules; /* @data 0x4b7f18 */
extern short speaker3BackCount; /* @data 0x4b808a */
extern short speaker0BackCount; /* @data 0x4b8088 */
extern short pendingTunnelSound; /* @data 0x4b8096 */
extern short warningView; /* @data 0x4b8092 */
extern short pendingFacing; /* @data 0x4b7fe4 */
extern short sentThroughDoors; /* @data 0x4b7fce */
extern short warningPlaying; /* @data 0x4b808e */
extern short warningSound; /* @data 0x4b8090 */
extern short doorAnchorView; /* @data 0x4b808c */
extern short door3Count; /* @data 0x4b8086 */
extern short door2Count; /* @data 0x4b8084 */
extern short door4Count; /* @data 0x4b8082 */
extern short door1Count; /* @data 0x4b8080 */
extern short doorPassesInARow[4]; /* @data 0x4b7fe6 */
extern short door3Views[16]; /* @data 0x4b7f94 */
extern short door2Views[16]; /* @data 0x4b7f74 */
extern short door4Views[16]; /* @data 0x4b7f54 */
extern short door1Views[16]; /* @data 0x4b7f34 */
extern short fidgetsDone; /* @data 0x4b809a */
extern short fidgetsAllowed; /* @data 0x4b8098 */
extern unsigned long fidgetersUsed; /* @data 0x4b80a4: slots used (allocateSlot) */
extern long lastFidgetTime; /* @data 0x4b809c */
extern long fidgetInterval; /* @data 0x4b80a0 */
extern short closedDoorPair; /* @data 0x4b7fbc */
void resetTunnels();
extern short debugTunnelScript; /* @data 0x4b80a8: the script last shown (debugging) */
void queueRemark(short kind);
short tunnelsKey(unsigned short key);
void drawTunnelsButtons(View *);
short removeTunnelEntry(TunnelList *list, short view);

short turnedBackAtDoor(FeatureRules *rules, short door, Snoid *snoid, unsigned short *first);

void makeOneFeatureRule();
extern Point tunnelPlaces[16]; /* @data 0x4a7730: where the Zoombinis wait */
void findWaitingPlace(short *spot, short side);
void dropFirstTunnelEntry(TunnelList *list);
void sayTunnelRemark();
void dropRemarkNotify(View *, short event);
void sendThroughDoors();
void tunnelsSnoidNotify(View *view, short event);
extern Point backScriptAnchors[4]; /* @data 0x4a78a6: where tunnelsSnoidNotify anchors the first entry's script */
extern Point door1Places[16]; /* @data 0x4a7770: the places past door 1 */
extern Point door4Places[16]; /* @data 0x4a77b0: door 4 */
extern Point door2Places[16]; /* @data 0x4a77f0: door 2 */
extern Point door3Places[16]; /* @data 0x4a7830: door 3 */
void tunnelsFrame();
extern short inTunnelsFrame; /* @data 0x4a7888: tunnelsFrame is running */
extern short turnBacksLeft; /* @data 0x4b7fc0: how many more times a Zoombini can be turned back */
extern short view7000; /* @data 0x4b7fc2 */
extern unsigned long nextTunnelRemarkTime; /* @data 0x4b7fe0: when to make the next idle remark (view ticks) */
extern short tunnelsSpeakers[4]; /* @data 0x4b7fc4: the four views that make the remarks (queueRemark) */
extern short tunnelsButtonsView; /* @data 0x4b7fcc: the buttons' view */
extern GroupList tunnelsGroups[1]; /* @data 0x4a76e8 */
void openTunnels();
void tunnelsClicked(short which);
extern short doorSpeakers[8]; /* @data 0x4a7710: which of tunnelsSpeakers remarks on a Zoombini at a door (by door and result) */
extern short speaker0BackLines[10]; /* @data 0x4a75d0: remarks (speaker0BackLinesUsed picks) */
extern short speaker0Replies[11]; /* @data 0x4a75e8: (speaker0RepliesUsed) */
extern short speaker2BackLines[8]; /* @data 0x4a7604: (speaker2BackLinesUsed) */
extern short doors34Lines[8]; /* @data 0x4a7618: (doors34LinesUsed) */
extern short speaker3BackLines[7]; /* @data 0x4a762c: (speaker3BackLinesUsed) */
extern short speaker3Replies[6]; /* @data 0x4a7640: (speaker3RepliesUsed) */
extern short speaker1BackLines[4]; /* @data 0x4a7650: (speaker1BackLinesUsed) */
extern short doors16Lines[6]; /* @data 0x4a765c: (doors16LinesUsed) */
void makeOneValueRules();
void makeTwoFeatureRules();
void makeTwoValueRules();
void pickBestMaskPair(ChosenSnoids *chosen, unsigned long *masks, unsigned long *pair, short pairs, short n);

extern Group g_4a76d8[1]; /* pointed to by initialised data */
extern Scene g_4a76f4[1]; /* pointed to by initialised data */

#endif
