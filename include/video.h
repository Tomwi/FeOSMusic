#ifndef VIDEO_H
#define VIDEO_H

#include "FeosMusic.h"

#define SUB_SCREEN 0
#define TOP_SCREEN 1

void initVideo(void);
void initConsole(void);
void print(char * string, int limit);
void setConsoleCoo(int, int);
void updateVideo(void);
void clearConsole(void);

extern u16 * iconFrames[2];
#endif