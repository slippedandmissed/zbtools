/*
 * sound's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SOUND_H
#define SOUND_H

extern short soundLevel; /* @data 0x4a0090 */
extern long g_4a0098;
extern unsigned long g_4a009c; /* sounds larger than this are loaded differently */
extern Entry *g_4a00a0;
extern short channelCounts[2]; /* @data 0x4a00a4 */
extern char currentChannel[2]; /* @data 0x4a00a8 */
extern long soundTypes[2]; /* @data 0x4a00dc */
extern char msgUnableToCreate[]; /* @data 0x4a0206 */
extern char textSound[]; /* @data 0x4a0217 */
extern char textMidi[]; /* @data 0x4a021d */
extern char textWaveform[]; /* @data 0x4a0222 */
extern char msgUnknownChunk[]; /* @data 0x4a022b */
extern char msgUnableToPrepare[]; /* @data 0x4a023f */
extern char msgSeekError[]; /* @data 0x4a0251 */
extern char msgUnableToStart[]; /* @data 0x4a025d */
extern char formatJoin[]; /* @data 0x4a027d */
extern char formatErrorNumber[]; /* @data 0x4a0282 */
extern char formatSoundId[]; /* @data 0x4a028d */
extern char msgDeviceFailed[]; /* @data 0x4a0297 */
extern short g_4aa42c;
extern char *g_4aa430;
extern char *g_4aa434;
extern char *g_4aa438;
extern short soundErrorsIgnored; /* @data 0x4aa43c */
unsigned short loadSoundByKey(short key, long type);
unsigned short fn_411382(short key, long type);
void unloadSound(short key, long type);
void fn_41158c(short key, long type);
short playSoundOn(short key, long type, short channel);
short fn_411bfe(short key, long type, short channel);
void unloadSounds();
Entry *fn_4115f5(short key, long tag);
void fn_411910(Entry *entry, short channel);
Entry *addSound(short key, long type);
void removeSound(Entry **entry);
void setSoundType(Entry **entry, short key, long type);
void reportSoundError(short id, long type, Entry *entry, const char *message);
short findChannel(short type);
short fn_4120c8(char value, long type);
Entry *findOrAddSound(short key, long type);
short isSoundPlaying(unsigned short id, long type);
short startSound(Entry *entry, short channel);
void stopSounds(unsigned short id, long type);
void fn_411e4c(unsigned short id, long type);
short fn_4120a2(unsigned short id, long type, short stop);
short waitForSound(unsigned short id, long type, short eventType, short discard);
short fn_412084(unsigned short id, long type, short eventType, short discard);
short waitForSoundValue(char value, long type, short eventType, short discard);
short fn_412159(char value, long type, short eventType, short discard);
short loadSound(Entry *entry);
Entry *getSound(short key, long type);
short prepareSound(Entry *entry, short channel);
void fn_4119f3(Entry *entry, short channel);
void fn_411d2c(long, SoundNotice *notice, long cookie);
void fn_412176(long type);
short fn_4121a5(short level);
void fn_4117a8(Entry *entry);

#endif
