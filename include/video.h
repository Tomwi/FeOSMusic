#ifndef VIDEO_H
#define VIDEO_H

#include "FeosMusic.h"

#define SUB_SCREEN 0
#define TOP_SCREEN 1
#define GFX_PATH "/data/FeOSMusic/"

void initVideo(void);
void deinitVideo(void);
void initConsole(void);
void print(char * string, int limit);
void setConsoleCoo(int, int);
void updateVideo(void);
void clearConsole(void);
void visualize(s16 * buffer, int length, int ch);
extern u16 * iconFrames[2];
#endif