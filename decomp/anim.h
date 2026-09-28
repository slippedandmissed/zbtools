/*
 * anim's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef ANIM_H
#define ANIM_H

extern basePort **screenPortRef; /* @data 0x4a0070: animations show on *screenPortRef */
extern unsigned char animOpcodes[13]; /* @data 0x4a0074 */
extern unsigned char animOperandSizes[13]; /* @data 0x4a0081 */
extern short keepFrameRate; /* @data 0x4aa412: frames stay on the beat when late */
extern ShortRect animArea; /* @data 0x4aa414 */
extern char *scriptText; /* @data 0x4aa41c */
extern long castInfo; /* @data 0x4aa420 */
extern short animDrawing; /* @data 0x4aa424: stepping (not skipping) */
/* anim */
void freeAnim(Anim **anim);
short stepAnim(Anim *anim);
short skipAnim(Anim *anim, short frames);
Anim *showAnim(Anim *anim);
void drawAnim(Anim *anim);
void drawSprite(Anim *anim, Sprite *sprite);
void markSprite(Anim *anim, short index, short which);
void resetSprite(Sprite *sprite);
void addRect(Anim *anim, ShortRect *rect, short which);
short playAnimation(AnimSpec *spec);
void setupAnim(AnimSpec *spec);
void loadCast(Anim *anim, short first, const char *name);
short playAnim(AnimSpec *spec);
void freeAnimSpec(AnimSpec *spec);
void restartAnim(Anim *anim, short run);
void runFrame(Anim *anim);
void loadScript(long *script, short id, const char *name);
void freeScript(long *script);
short fn_411212();
void setupAnimOffscreen(AnimSpec *spec);
void spritesBounds(Anim *anim, ShortRect *into);

#endif
