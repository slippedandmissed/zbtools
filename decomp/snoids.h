/*
 * snoids's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SNOIDS_H
#define SNOIDS_H

extern short spotCorner; /* @data 0x4a4ce6: where findSpot starts its grid (0-3) */
extern short heldPlace; /* @data 0x4b7b38 */
extern short placeHeld; /* @data 0x4b7b3a */
extern SnoidArrived arrivalHook; /* @data 0x4b7b68: told when a Zoombini arrives */
void showNameTag(const char *text, unsigned long duration, short large); /* 0x4589ce */
void recordParty(short ending, short all);
void freePaths();
/* snoids */
void resetSnoids(); /* 0x456c00 */
void loadSnoids(short files);
void closeSnoids();
short *loadShortTable(short id, long *resource); /* 0x456dbe */
void freeSnoidTables();
short countChosenSnoids(); /* 0x456e4c */
short countSnoidViews(); /* 0x456e7f */
void loadBaseSnoidScripts();
void freeBaseSnoidScripts();
void loadSnoidScripts(short first, short count, short limit);
void addSnoidScripts(short first, short count, short limit);
void findSnoidScript(short id, short *group, short *index);
void loadSnoidScript(short id);
void freeSnoidScripts(); /* 0x4571a8 */
void drawSnoid(Snoid *snoid); /* 0x4571d8 */
short placeSnoid(Snoid *snoid, unsigned long when, short x, short y, short targetX, short targetY);
short addSnoidView(Snoid *snoid, short placed); /* 0x4574ae */
short dragSnoid(View *view, Point where, const ShortRect *bounds, void (*track)(Point where)); /* 0x458059 */
void hideNameTag();
void drawNameTag(View *view);
void updateNameTag(View *view, short region);
void toggleShowPositions();
void setSnoidsRunning(short running);
void runSnoid(short id, short chosen);
View *idleSnoidView(short id);
Snoid *findSnoid(short id, short wake);
extern ShortRect nameTagRect; /* @data 0x4a4cc4 */
extern ShortRect largeNameTagRect; /* @data 0x4a4ccc */
extern Paths *paths; /* @data 0x4a4cd4 */
extern PathNodes *pathNodes; /* @data 0x4a4cd8 */
extern long pathsResource; /* @data 0x4a4cdc */
extern long pathNodesResource; /* @data 0x4a4ce0 */
extern short nextPathToDraw; /* @data 0x4a4b9c: the path drawPaths draws next */
extern ChosenSnoids chosenSnoids; /* @data 0x4b7b88 */
ChosenSnoids *listChosenSnoids();
void chooseSnoids(short chosen, short run);
short nearPlacedView(Point where);
void turnSnoid(View *view, short event);
short snoidSound(Snoid *snoid, short which);
short spotTaken(Point *where, View *ignore, short radius);
short spotNear(Point *where, short radius, short skip);
void drawFeature(short feature, short value, ShortRect *rect);
extern short feetImages[6]; /* @data 0x4a4ba4: by feature value (1-5) */
extern short noseImages[6]; /* @data 0x4a4bb0 */
extern short eyesImages[6]; /* @data 0x4a4bbc */
extern short hairImages[6]; /* @data 0x4a4bc8 */
extern short altFeetImages[6]; /* @data 0x4a4bd4: the same, when unknownF4 is 9 */
extern short altNoseImages[6]; /* @data 0x4a4be0 */
extern short altEyesImages[6]; /* @data 0x4a4bec */
extern short altHairImages[6]; /* @data 0x4a4bf8 */
extern short spotRadius; /* @data 0x4a4ce4 */
extern Point spots[32]; /* @data 0x4b7bdc: where idle Zoombinis stand */
extern short spotIds[32]; /* @data 0x4b7c5c */
extern short spotCount; /* @data 0x4b7c9c */
void makeName(char *name, short size);
void sortSnoids(short running);
extern char vowelSounds[]; /* @data 0x4a4c0c: pairs ("a ", "ee", ...) */
extern char consonantPairs[]; /* @data 0x4a4c73: pairs ("bl", "br", ...) */
extern short sortedCount; /* @data 0x4b75ac */
extern short sortedX[32]; /* @data 0x4b75ae */
short keepDragging();
extern short dragButtonDown; /* @data 0x4b7c9e */
extern short clickToDrag; /* @data 0x4b7ca0 */
short settleSnoid(View *view);
extern unsigned short showPositions; /* @data 0x4a4b9a: show the dragged Zoombini's position (a cheat) */
extern short dragX; /* @data 0x4b754e */
extern short dragY; /* @data 0x4b7550 */
void makePartySnoids(short all); /* 0x4572f0 */
void viewPlace(Point *where, short n); /* 0x457fff */
void placedViewPoint(Point *where, short n);
void releaseHeldPlace();
void drawSnoidView(View *view);
void updateSnoidView(View *view, short region); /* 0x4575e6 */
void setSnoidAction(Snoid *snoid, short action, Point *where);
void setSnoidMode(short);
void useAltSnoids(short restore);
extern long altSnoidResources[3]; /* @data 0x4b7ca4 */
extern ImageBank *savedSnoidImages; /* @data 0x4b7cb0 */
extern short *savedSnoidTables[2]; /* @data 0x4b7cb4 */
extern short savedFeetImages[6]; /* @data 0x4b7cbc */
extern short savedNoseImages[6]; /* @data 0x4b7cc8 */
extern short savedEyesImages[6]; /* @data 0x4b7cd4 */
extern short savedHairImages[6]; /* @data 0x4b7ce0 */
extern short otherFeetImages[6]; /* @data 0x4a4cec: the other look's */
extern short otherNoseImages[6]; /* @data 0x4a4cf8 */
extern short otherEyesImages[6]; /* @data 0x4a4d04 */
extern short otherHairImages[6]; /* @data 0x4a4d10 */
short stepAlongPath(Snoid *snoid);
void choosePath(Snoid *snoid, Point *target);
extern short dragInProgress; /* @data 0x4b7568 */
extern long snoidImages2Resource; /* @data 0x4b7b58 */
extern ImageBank *snoidImages; /* @data 0x4b7b5c */
extern ImageBank *snoidImages2; /* @data 0x4b7b60 */
extern ImageBank *snoidImages3; /* @data 0x4b7b64 */
extern long snoidImages3Resource; /* @data 0x4a4c08 */
extern short snoidTablesLoaded; /* @data 0x4a4c04 */
extern short *snoidTables[4]; /* @data 0x4b7b3c */
extern long snoidTableResources[4]; /* @data 0x4b7b6c */
extern long baseSnoidScriptResources[51]; /* @data 0x4b7630 */
extern long snoidScriptResources[110]; /* @data 0x4b76fc */
extern short snoidScriptGroupFirst[2]; /* @data 0x4b7b7c */
extern short snoidScriptGroupCount[2]; /* @data 0x4b7b80 */
extern short snoidScriptGroups; /* @data 0x4b7b84 */
void loadPaths(short);
void enterSnoids(short dy);
void staggerSnoids(unsigned long interval, unsigned long delay);
extern short staggerDue; /* @data 0x4b7b86 */
short sceneLevel();
short campHint(short *visits);
short heldPlaceNumber(); /* 0x457fbb */
void claimPlacedView(short n, short id); /* 0x45802e */
void markPlacedSnoids();
void sendSnoids(short x, short y, unsigned long interval);
void drawPaths();
void startSnoidScript(Snoid *snoid, short id, Point *anchor, char unknownF8);
void findSpot(View *view, ShortRect *area, short walk, short radius);
void pickFreePlace(Point *result, Point *places, short count, short radius);
extern Point spotOrigin; /* @data 0x4a4d1c */
void initSnoid(Snoid *snoid); /* 0x45bf41 */
void setSnoidFacing(Snoid *snoid, short facing);
short layOutSnoid(Snoid *snoid, short *event);
short countPresentTravellers();
void setSpotCorner(short value);
void setArrivalHook(SnoidArrived hook); /* 0x45bfc0 */

#endif
