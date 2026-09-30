/*
 * isle's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ISLE_H
#define ISLE_H

short isleAllowsFeature(short group, short feature);
extern short g_4b15ae;
extern ImageBank *g_4b159c;
extern char g_4a3386[];
extern char g_4a339c[];
extern short g_4b15b0;
extern short g_4b15b2;
void checkEnoughChosen();
void drawIsleImage(short which, short x, short y);
void addIsleSettingViews();
extern short g_4b15ac;
extern short g_4b15b6;
extern short g_4b15b8;
void resetZoombiniMade();
short zoombiniMadeAllowed();
void showIsleSetting(short keep);
extern short g_4b15a4; /* the scene is open */
extern long g_4b1590;
extern long g_4b158c;
extern long g_4b1594;
extern long g_4b1588;
void closeIsle();
extern ShortRect g_4a337e;
extern ImageBank *g_4b1598;
void drawFeatureButtons(short which, short lit, ShortRect *bounds);
extern short g_4a33b2;
extern short g_4a33b4;
void updateIsleButtons(View *, short region);
extern Point g_4a3324[16];
void isleQueue(Point *where, short *slot);
short isleKey(unsigned short key);
short leaveIsleIfAsked();
extern short isleBusy; /* @data 0x4a336c */
void isleFrame();
void drawZoombiniParts(Snoid *snoid);
extern SceneButton g_4a31cc[8]; /* [7]: the whole screen */
extern ShortRect g_4a3376;
void drawIsleButtons(short which, short lit, short show);
void countZoombiniMade(short add);
void pickZoombiniMade(short rename);
void drawIsleButtonsView(View *);
void featureButtonClicked(short button);
extern short g_4b15b4;
void isleButtonClicked(short button);
extern Point g_4a3364;
extern Point g_4a3368;
extern GroupList g_4a330c[2];
void openIsle();

#endif
