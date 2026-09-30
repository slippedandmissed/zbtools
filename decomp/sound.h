/*
 * sound's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef SOUND_H
#define SOUND_H

extern short soundLevel; /* @data 0x4a0090 */
extern long g_4a0098;
extern unsigned long g_4a009c; /* sounds larger than this are loaded differently */
extern SoundEntry *g_4a00a0;
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
unsigned short findAndLoadSound(short key, long type);
void unloadSound(short key, long type);
void unloadSoundNow(short key, long type);
short playSoundOn(short key, long type, short channel);
short findAndPlaySound(short key, long type, short channel);
void unloadSounds();
SoundEntry *findSound(short key, long tag);
void closeSoundOnChannel(SoundEntry *entry, short channel);
SoundEntry *addSound(short key, long type);
void removeSound(SoundEntry **entry);
void setSoundType(SoundEntry **entry, short key, long type);
void reportSoundError(short id, long type, SoundEntry *entry, const char *message);
short findChannel(short type);
short soundValueReached(char value, long type);
SoundEntry *findOrAddSound(short key, long type);
short isSoundPlaying(unsigned short id, long type);
short startSound(SoundEntry *entry, short channel);
void stopSounds(unsigned short id, long type);
void endSoundLoops(unsigned short id, long type);
short soundPlayingOrStop(unsigned short id, long type, short stop);
short waitForSound(unsigned short id, long type, short eventType, short discard);
short awaitSound(unsigned short id, long type, short eventType, short discard);
short waitForSoundValue(char value, long type, short eventType, short discard);
short awaitSoundValue(char value, long type, short eventType, short discard);
short loadSound(SoundEntry *entry);
SoundEntry *getSound(short key, long type);
short prepareSound(SoundEntry *entry, short channel);
void stopSoundOnChannel(SoundEntry *entry, short channel);
void soundNoticeCallback(long, SoundNotice *notice, long cookie);
void resetSoundChannel(long type);
short soundAtMost(short level);
void disposeSoundHandle(SoundEntry *entry);

#endif
