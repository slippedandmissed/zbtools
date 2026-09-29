/*
 * xfer's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef XFER_H
#define XFER_H

extern long g_4b98d4; /* @data 0x4b98d4: xfer.MHK */
extern short g_4b98d8; /* @data 0x4b98d8: the scene is open */
extern short g_4b9928; /* @data 0x4b9928 */
extern unsigned long g_4b9934; /* @data 0x4b9934 */
extern Point g_4b9944[24]; /* @data 0x4b9944 */
extern char g_4b99a4[24]; /* @data 0x4b99a4: g_4b9944's in use */
extern char g_4b99c0; /* @data 0x4b99c0 */
extern char g_4b99c1; /* @data 0x4b99c1 */
extern char g_4b99c2; /* @data 0x4b99c2 */
extern char g_4b99c3; /* @data 0x4b99c3 */

void closeScene2();
void fn_46bb0c(char *cell, short x, short y);
void fn_46bbce(View *view);
void fn_46bdde(View *view, short region);
long fn_46b07b(long);
void fn_46b747(long, short id);

#endif
