/*
 * lilly's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef LILLY_H
#define LILLY_H

extern short lillyStage; /* @data 0x4af350 */
extern short lillyClaim; /* @data 0x4af35a */
void lillyNoDraw(View *);
void lillyNoUpdate(View *, short);
void setLillyStage(short value);
short lillyClaimIndex();
void releaseLillyClaim();
void freeResourcePair(long *resources);
short lillyKey(unsigned short event);
void freeLockedResource(long *resource, short *handle);
void updateLillyBackdrop(View *view, short region);
void loadTablePair(long *resources, short id, short **first, short **second);
void placeByHotSpot(View *view);
void freeLockedResources(long *resources, short *handles, short count);
void hopperNotify(View *view, short event);
void placeActorCels3(View *view);
extern ShortRect lillyArea; /* @data 0x4a1dfc */
void setLillyLevel(short level);
void updateLillyButtons(View *view, short region);
void lillyActorNotify(View *view, short event);
void loadLockedTable(long *resource, short *handle, short id, short **locked);
extern short unusedLillyLevel; /* @data 0x4ac91e */
extern short lillyLevelParam; /* @data 0x4ac920 */
extern short startCount; /* @data 0x4af0f8 */
void lillyNotify60(View *view, short event);
void lillyNotify49(View *view, short event);
void showViewOnSquare(short id, short row, short column);
void updateMarkerView(View *view, short region);
extern short event60Views[20]; /* @data 0x4acdf4 */
extern short event60Count; /* @data 0x4ace1c */
extern short jumperBusy; /* @data 0x4acfee */
extern short landedJumper; /* @data 0x4acfe8 */
extern ShortRect markerArea; /* @data 0x4a1e32 */
void drawLillyButton(short which, short lit, short show);
void checkLillyArrivals();
void updateSquareHighlight(View *view, short region);
extern SceneButton lillyButtons[3]; /* @data 0x4a1b28 */
extern ImageBank *lillyButtonImages; /* @data 0x4a1d68 */
extern short padsArrived; /* @data 0x4af0ea */
extern short actorCount; /* @data 0x4af102 */
extern short actorViews[]; /* @data 0x4aed64 */
extern short lillyPartySize; /* @data 0x4af0e8 */
extern short lillyLevel; /* @data 0x4a1b1c */
extern short cursorColumn; /* @data 0x4af344 */
extern short cursorRow; /* @data 0x4af346 */
extern ShortRect cursorRect; /* @data 0x4af5a8 */
void placeActorCels(View *view);
void placeActorCelsLow(View *view);
void lillyNotify54(View *view, short event);
void loadLillyScripts(long *resources, short *handles, short count);
extern short landerBusy; /* @data 0x4acff0 */
void lillyNotify44(View *view, short event);
void drawNumberBox(ShortRect rect, short number);
void placeActorCelsHidden(View *view);
void mirrorGrid(short (*grid)[12], short how);
void drawLillyButtons(View *);
void flashSquare(View *view);
void closeLilly();
extern short lillyOpen; /* @data 0x4af368 */
extern long unusedLillyResource; /* @data 0x4a1b48 */
extern long lillyImagesResource; /* @data 0x4a1b44 */
extern long lillyScriptResources[91]; /* @data 0x4af108 */
extern short lillyScriptHandles[91]; /* @data 0x4af278 */
extern long rowPlaceResources[2]; /* @data 0x4ac928 */
extern long squareHotSpotResources[2]; /* @data 0x4ac930 */
extern long actorHotSpotResources[2]; /* @data 0x4ac938 */
extern long grid1Resource; /* @data 0x4ac98c */
extern short grid1Handle; /* @data 0x4ac994 */
extern long grid2Resource; /* @data 0x4ac998 */
extern short grid2Handle; /* @data 0x4ac9a0 */
extern long grid3Resource; /* @data 0x4ac9a4 */
extern short grid3Handle; /* @data 0x4ac9ac */
extern long lillyButtonResource; /* @data 0x4a1be8 */
extern long lillyFile; /* @data 0x4af364 */
extern short flashCount; /* @data 0x4af352 */
extern short flashRow; /* @data 0x4af34a */
extern short flashColumn; /* @data 0x4af348 */
extern char flashFrame; /* @data 0x4a1e3a */
void placeViewOnSquare(short id, short row, short column);
extern short *squareHotSpotsX; /* @data 0x4ac948 */
extern short *squareHotSpotsY; /* @data 0x4ac94c */
void drawSquareImage(short row, short column, char offset);
short addLillyActor(short value);
void drawSquare(short row, short column);
void drawBoard(View *);
void drawCursorSquare(View *view);
void placeJumper(View *view);
void placeJumperAt(View *view);
void placeLander(View *view);
void addLillyActors();
short pickNextSquare(View *view);
void hopNotify(View *view, short event);
extern short *rowLeft; /* @data 0x4ac940 */
extern short *rowTop; /* @data 0x4ac944 */
extern short columnDy[12]; /* @data 0x4a1d70 */
extern short actorDealLast; /* @data 0x4af5a6 */
extern short actorDealOrder[13]; /* @data 0x4a1db2 */
extern short actorDealKinds[12]; /* @data 0x4a1dcc */
extern short actorDealValues[12]; /* @data 0x4a1de4 */
extern short dealtKinds[12]; /* @data 0x4ac95a */
extern short dealtValues[12]; /* @data 0x4ac972 */
extern Point jumpPlaces[]; /* @data 0x4a1ca4 */
void dealSquares();
extern short squareSetA[12]; /* @data 0x4a1e3c */
extern short squareSetB[12]; /* @data 0x4a1e56 */
extern short *squareSets[3]; /* @data 0x4af5b0 */
extern short squareSetA3[4]; /* @data 0x4af5bc */
extern short squareSetB3[4]; /* @data 0x4af5c4 */
extern short squareSetC3[4]; /* @data 0x4af5cc */
extern short squareSetA4[5]; /* @data 0x4af5d4 */
extern short squareSetB4[5]; /* @data 0x4af5de */
extern short squareSetC4[5]; /* @data 0x4af5e8 */
extern short squareSetA5[6]; /* @data 0x4af5f2 */
extern short squareSetB5[6]; /* @data 0x4af5fe */
extern short squareSetC5[6]; /* @data 0x4af60a */
extern LillyDeal squareDeals[13]; /* @data 0x4af616 */
void searchStep(short attribute, short layer, short row, short column);
void searchLayer(short attribute, short layer);
void swapSquares();
void flashSwap(View *view);
void lillyViewNotify3(View *view, short event);
void lillyClicked(short action);
extern short cursorSquareView; /* @data 0x4af33c */
extern short lillyDragState; /* @data 0x4af664 */
extern ShortRect rowEntryRects[12]; /* @data 0x4a1cd8 */
extern ShortRect lillyArea1; /* @data 0x4a1e04 */
extern ShortRect lillyArea2; /* @data 0x4a1e0c */
extern short swapSound; /* @data 0x4a1f16 */
extern short swapsPerStage; /* @data 0x4ac924 */
extern short swapsThisStage; /* @data 0x4ac926 */
void dragLillyPiece(View *view, Point where);
extern short claimedRow; /* @data 0x4af332 */
extern short rowAnchorViews[]; /* @data 0x4aed22 */
extern short padsPlaced; /* @data 0x4af0ec */
extern short swapToolStage; /* @data 0x4ac922 */
extern short snoidPadViews[]; /* @data 0x4aed3a */
extern short event4Pad; /* @data 0x4af33a */
extern short swapToolView; /* @data 0x4af354 */
extern Point presetSwapColumns[5]; /* @data 0x4a1d40 */
extern Point presetSwapRows[5]; /* @data 0x4a1d54 */
extern short presetSwapIndex; /* @data 0x4af104 */
extern short swapFirstMarker; /* @data 0x4af33e */
extern short swapSecondMarker; /* @data 0x4af340 */
extern short swapColumn; /* @data 0x4af34c */
extern short swapRow; /* @data 0x4af34e */
extern short layersToSearchCount; /* @data 0x4af342 */
extern short swapPending; /* @data 0x4af360 */
extern short swapStep; /* @data 0x4af358 */
extern char swapFlashFrame; /* @data 0x4a1e3b */
extern short searchQueueStart; /* @data 0x4af8aa */
extern short cursorImageBase[]; /* @data 0x4a1e16 */
extern short cursorFrameOffsets[4]; /* @data 0x4a1e28 */
extern short cursorSquareFrame; /* @data 0x4a1e30 */
extern LillySearch layerSearches[]; /* @data 0x4ad7e0 */
extern Point searchQueue[]; /* @data 0x4af668 */
extern short searchQueueEnd; /* @data 0x4af8a8 */
void lillyNotify30(View *view, short event);
extern short firstArrivals; /* @data 0x4ac91c */
extern short landerQueue[]; /* @data 0x4acdca */
extern short landerQueueCount; /* @data 0x4acdf2 */
extern short jumperQueue[]; /* @data 0x4acda0 */
extern short jumperQueueCount; /* @data 0x4acdc8 */
extern short lillyLayerView3; /* @data 0x4aed14 */
void turnGrid(short (*grid)[12], short how);
short moveActorDown(View *view);
extern short squareImageBase[]; /* @data 0x4a1e20 */
extern ImageBank *lillyImages; /* @data 0x4af5a0 */
extern short replanQueue[]; /* @data 0x4acec6 */
extern short replanQueueCount; /* @data 0x4acfe6 */
extern LillyStart lillyStarts[3]; /* @data 0x4aece6 */
void setUpBoard();
void openLilly();
void lillyFrame();
extern short inLillyFrame; /* @data 0x4a1d88 */
extern short planTick; /* @data 0x4af5a4 */
extern short lillyLayerView1; /* @data 0x4aed10 */
extern short unusedLilly1; /* @data 0x4ac958 */
extern short unusedLilly2; /* @data 0x4af36c */
extern short unusedLilly3; /* @data 0x4af100 */
extern short unusedLilly4; /* @data 0x4af0f0 */
extern unsigned long nextActorTime; /* @data 0x4af0fc */
extern short unusedLilly5; /* @data 0x4af0fa */
extern short unusedLilly6; /* @data 0x4af0f2 */
extern short unusedLilly7; /* @data 0x4af0f4 */
extern short unusedLilly8; /* @data 0x4af35e */
extern short unusedLilly9; /* @data 0x4af0f6 */
extern short unusedLillyTable1[144]; /* @data 0x4aefc0 */
extern short lillyLayerViews[10]; /* @data 0x4aed0e */
extern short unusedLillyTable2[20]; /* @data 0x4ac9bc */
extern short planWaiting[20]; /* @data 0x4acd76 */
extern short unusedLillyTable3[20]; /* @data 0x4ace48 */
extern short unusedLillyTable4[144]; /* @data 0x4acb08 */
extern short hopperWaiting[144]; /* @data 0x4acc2a */
extern short unusedLilly10; /* @data 0x4af0e2 */
extern short nextStart; /* @data 0x4af0e4 */
extern short unusedLilly11; /* @data 0x4acc28 */
extern short hopperWaitingCount; /* @data 0x4acd4a */
extern short unusedLilly12; /* @data 0x4acfea */
extern short jumper2Busy; /* @data 0x4acfec */
extern short unusedLilly13; /* @data 0x4ac9e4 */
extern short planWaitingCount; /* @data 0x4acd9e */
extern short unusedLilly14; /* @data 0x4ace70 */
extern short boardView; /* @data 0x4a1b40 */
extern Point padPlaces[]; /* @data 0x4a1bfc */
extern short lillyView11000; /* @data 0x4af338 */
extern GroupList lillyGroups[1]; /* @data 0x4a1bc8 */
extern short valueUses[12]; /* @data 0x4a1ec6 */
extern short rowValueUsed[14]; /* @data 0x4a1ede */
extern short columnValueUsed[14]; /* @data 0x4a1efa */
extern short levelLeftOut[]; /* @data 0x4a1e84 */
extern short (*grid1)[12]; /* @data 0x4ac9b0 */
extern short (*grid2)[12]; /* @data 0x4ac9b4 */
extern short (*grid3)[12]; /* @data 0x4ac9b8 */
extern short rowColumnAllowed[12]; /* @data 0x4a1eae */
extern short overlayImageBase[]; /* @data 0x4a1e70 */
extern short squareSetC[]; /* @data 0x4a1b1e */
extern short attributeImageIndex[]; /* @data 0x4a1b38 */
void lillyNotify70(View *view, short event);
extern short hopperQueue[]; /* @data 0x4ac9e6 */
extern short hopperQueueCount; /* @data 0x4acb06 */
extern short event80Views[]; /* @data 0x4aed80 */
extern short event80Count; /* @data 0x4af0e0 */
extern short actorsOut[]; /* @data 0x4aeea0 */
extern short actorsOutCount; /* @data 0x4af0e6 */
extern short event44Views[20]; /* @data 0x4ace1e */
extern short event44Count; /* @data 0x4ace46 */
extern short finishedLander; /* @data 0x4acff2 */
extern short lillyGoReady; /* @data 0x4af36a */
extern short lillyButton2Lit; /* @data 0x4a1d6c */
extern short lillyButton1Drawn; /* @data 0x4a1d6e */
extern short event2Views[20]; /* @data 0x4ace72 */
extern short event2Count; /* @data 0x4ace9a */
extern short *actorHotSpotsX; /* @data 0x4ac950 */

#endif
