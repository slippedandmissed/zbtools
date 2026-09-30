/*
 * hotel's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef HOTEL_H
#define HOTEL_H

void roomViewNotify(View *view, short event);
short footSound(View *view, short which);
void startView9002(short n);
void startView9007(short n);
void deleteHotelTalker();
void setRowAndColumn(short b, short a, short n);
void setRowLayerColumn(short a, short b, short c, short n);
void startRoomAnimations();
void updateHotelButtons(View *view, short region);
extern short roomsFilled; /* @data 0x4ac0e4 */
extern short roomDoorViews[]; /* @data 0x4abfc0 */
extern short roomAnimStage; /* @data 0x4ac0d6 */
extern short hotelGoReady; /* @data 0x4abec2 */
extern short hotelButton2Lit; /* @data 0x4a1aac */
extern short hotelButton1Drawn; /* @data 0x4a1aae */
void startSnoidInRoom(short id);
void hotelSnoidNotify(View *view, short event);
void drawHotelButton(short which, short lit, short show);
extern short hotelLevel; /* @data 0x4ac0d8 */
extern short hotelRoom; /* @data 0x4ac0ec */
extern short roomColumnViews[]; /* @data 0x4abec6 */
extern short roomGroup; /* @data 0x4ac0d0 */
extern ImageBank *hotelButtonImages; /* @data 0x4a1a18 */
void drawRoomView(View *view);
void startRoomColumnViews();
extern ImageBank *roomImages; /* @data 0x4ac178 */
extern ImageBank *roomImages3d; /* @data 0x4ac17c */
extern short roomCount; /* @data 0x4ac0ee */
void darkenPalette();
void countFeatureValues();
void drawIdBox(short id);
void clearWay(short x);
extern ShortRect viewIdBoxRect; /* @data 0x4a1ae6 */
short fitsRoom(short a, short b, short n);
short fitsRoom3d(short a, short b, short c, short n);
void sendSnoidToRoom(short id);
void addHotelViews();
void drawHotelButtons(View *);
short hotelKey(unsigned short key);
extern short hotelFacing; /* @data 0x4ac13e */
extern short hotelWalker; /* @data 0x4ac136 */
extern short snoidRejected; /* @data 0x4ac0ce */
void closeHotel();
void hotelFrame();
extern short inHotelFrame; /* @data 0x4a1ab0 */
extern unsigned long hotelIdleSince; /* @data 0x4ac140 */
extern ShortRect hotelBlankRect1; /* @data 0x4a19e4 */
extern ShortRect hotelBlankRect2; /* @data 0x4a19ec */
extern ShortRect hotelBlankRect3; /* @data 0x4a19f4 */
extern ShortRect hotelBlankRect4; /* @data 0x4a19fc */
extern Point hotelPlaces[16]; /* @data 0x4a1a1c */
extern short hotelOpen; /* @data 0x4abec0 */
extern long hotelButtonResource; /* @data 0x4ac144 */
extern long roomImagesResource; /* @data 0x4ac148 */
extern long hotelPlaceXResource; /* @data 0x4ac14c */
extern long hotelPlaceYResource; /* @data 0x4ac150 */
extern long roomViewXResource; /* @data 0x4ac154 */
extern long roomViewYResource; /* @data 0x4ac158 */
extern long roomColumnXResource; /* @data 0x4ac160 */
extern long roomColumnYResource; /* @data 0x4ac164 */
extern long layerRowXResource; /* @data 0x4ac168 */
extern long layerRowYResource; /* @data 0x4ac16c */
extern long roomViewHotXResource; /* @data 0x4ac170 */
extern long roomViewHotYResource; /* @data 0x4ac174 */
extern long hotelFile; /* @data 0x4abebc */
extern short debugTalkerScript1; /* @data 0x4ac0c4 */
extern short debugTalkerScript2; /* @data 0x4ac0c6 */
extern short guideStep; /* @data 0x4ac0fe */
extern short guideLastStep; /* @data 0x4ac100 */
extern short savedGuideStep; /* @data 0x4ac102 */
extern short savedGuideLastStep; /* @data 0x4ac104 */
extern short guideView; /* @data 0x4ac0fa */
extern PALETTEENTRY savedPalette[]; /* @data 0x4ac51c */
extern short *roomViewX; /* @data 0x4ac188 */
extern short *roomViewY; /* @data 0x4ac18c */
extern short *layerRowX; /* @data 0x4ac198 */
extern short *layerRowY; /* @data 0x4ac19c */
extern short roomViews[25]; /* @data 0x4ac216 */
extern short roomViewScripts[]; /* @data 0x4ac10e */
extern short roomAnchorView; /* @data 0x4ac0bc */
extern Point roomPlaces[25]; /* @data 0x4a1788 */
extern Point roomPlaces3d[25]; /* @data 0x4a17f0 */
extern short standX; /* @data 0x4ac510 */
extern short standY; /* @data 0x4ac512 */
extern short roomOccupancy[25]; /* @data 0x4abdc0 */
extern ShortRect standArea; /* @data 0x4ac514 */
extern short *roomColumnX; /* @data 0x4ac190 */
extern short *roomColumnY; /* @data 0x4ac194 */
extern short roomColumnDx[5]; /* @data 0x4a1a04 */
extern short roomColumnDy[5]; /* @data 0x4a1a0e */
extern short arrivingSnoid; /* @data 0x4ac0ea */
extern short arrivingGroup; /* @data 0x4ac0f8 */
extern short hotelAnyFits; /* @data 0x4abec4 */
void drawFeatureLabels();
void layOutRoomView(View *view, short region);
extern short *roomViewHotX; /* @data 0x4ac1a0 */
extern short *roomViewHotY; /* @data 0x4ac1a4 */
void setUpHotelPuzzle();
void hotelClicked(short action);
void openHotel();
extern short talkerPending; /* @data 0x4ac0c0 */
extern short talkerStage; /* @data 0x4ac0be */
extern short talkerDoneGroup; /* @data 0x4ac0f6 */
extern short guideStepGroup; /* @data 0x4ac0fc */
extern short guideRemarkGroup; /* @data 0x4ac0f0 */
extern short unusedHotel1; /* @data 0x4ac138 */
extern short hotelLabelView; /* @data 0x4ac0c8 */
extern short view11800; /* @data 0x4ac0ca */
extern GroupList hotelGroups[1]; /* @data 0x4a1764 */
extern short talkerStarted; /* @data 0x4ac13a */
extern short skipGuide; /* @data 0x4ac13c */
extern short roundResetGroup; /* @data 0x4ac0f2 */
extern short talkerGroup; /* @data 0x4ac0f4 */
extern short hotelFails; /* @data 0x4ac0e6 */
extern short snoidArriving; /* @data 0x4ac0dc */
extern short heldRoomPlace; /* @data 0x4ac0d4 */
extern short firstPlacementFree; /* @data 0x4ac0da */
extern short droppedSnoid; /* @data 0x4ac504 */
extern short *hotelPlaceX; /* @data 0x4ac180 */
extern short *hotelPlaceY; /* @data 0x4ac184 */
extern short rowSortFeature; /* @data 0x4ac0de */
extern short columnSortFeature; /* @data 0x4ac0e0 */
extern short layerSortFeature; /* @data 0x4ac0e2 */
extern short hotelValueCounts[4]; /* @data 0x4ac106 */
extern ChosenSnoids *hotelChosen; /* @data 0x4ac508 */
extern short hotelPartySize; /* @data 0x4ac0e8 */
extern short hotelTalkerView; /* @data 0x4ac0ba */
extern short roomRowValues[25]; /* @data 0x4ac1a8 */
extern short roomLayerValues[25]; /* @data 0x4ac1da */
extern short roomColumnValues[25]; /* @data 0x4ac20c */
extern short room9002Views[]; /* @data 0x4ac310 */
extern short room9007Views[]; /* @data 0x4ac40a */

#endif
