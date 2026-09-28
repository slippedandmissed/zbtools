/*
 * config's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef CONFIG_H
#define CONFIG_H

extern short g_4a3e5c; /* set when the game data is found in INSTALLFROMDIR */
extern char dataDirName[]; /* @data 0x4a3f15 */
extern char installToDirKey[]; /* @data 0x4a3f1b */
extern char configFileName[]; /* @data 0x4a5149 */
void fn_44695c();
void fn_446962(char *, const char *);
void findGameData();
short preferFirstFile(const char *first, const char *fallback);

#endif
