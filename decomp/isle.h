/*
 * isle's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ISLE_H
#define ISLE_H

short isleAllowsFeature(short group, short feature);
extern short enoughToLeave; /* @data 0x4b15ae */
extern ImageBank *isleImages; /* @data 0x4b159c */
extern char isleImageHotX[]; /* @data 0x4a3386 */
extern char isleImageHotY[]; /* @data 0x4a339c */
extern short settingView1; /* @data 0x4b15b0 */
extern short settingView2; /* @data 0x4b15b2 */
void checkEnoughChosen();
void drawIsleImage(short which, short x, short y);
void addIsleSettingViews();
extern short isleButton22Due; /* @data 0x4b15ac */
extern short isleRemark; /* @data 0x4b15b6 */
extern short isleSendingOff; /* @data 0x4b15b8 */
void resetZoombiniMade();
short zoombiniMadeAllowed();
void showIsleSetting(short keep);
extern short isleOpen; /* @data 0x4b15a4: the scene is open */
extern long isleImagesResource; /* @data 0x4b1590 */
extern long featureButtonResource; /* @data 0x4b158c */
extern long isleButtonResource; /* @data 0x4b1594 */
extern long isleFile; /* @data 0x4b1588 */
void closeIsle();
extern ShortRect featureButtonsRect; /* @data 0x4a337e */
extern ImageBank *featureButtonImages; /* @data 0x4b1598 */
void drawFeatureButtons(short which, short lit, ShortRect *bounds);
extern short isleEnoughDrawn; /* @data 0x4a33b2 */
extern short isleMakeAllowedDrawn; /* @data 0x4a33b4 */
void updateIsleButtons(View *, short region);
extern Point isleQueuePlaces[16]; /* @data 0x4a3324 */
void isleQueue(Point *where, short *slot);
short isleKey(unsigned short key);
short leaveIsleIfAsked();
extern short isleBusy; /* @data 0x4a336c */
void isleFrame();
void drawZoombiniParts(Snoid *snoid);
extern ShortRect isleButtonsRect; /* @data 0x4a3376 */
void drawIsleButtons(short which, short lit, short show);
void countZoombiniMade(short add);
void pickZoombiniMade(short rename);
void drawIsleButtonsView(View *);
void featureButtonClicked(short button);
extern short view4100; /* @data 0x4b15b4 */
void isleButtonClicked(short button);
extern Point isleEntry; /* @data 0x4a3364 */
extern Point isleExit; /* @data 0x4a3368 */
extern GroupList isleGroups[2]; /* @data 0x4a330c */
void openIsle();

extern Scene g_4a2f0c[1]; /* pointed to by initialised data */
extern Group g_4a32ec[2]; /* pointed to by initialised data */

#endif
