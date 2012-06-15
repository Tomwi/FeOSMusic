#ifndef VIDEO_H
#define VIDEO_H

#include "FeOSMusic.h"

#define SUB_SCREEN (0)
#define TOP_SCREEN (1)

#define MAX_STRLEN (1024)

enum VISUALIZER {
    NORMAL,
    BORKUALIZER,
};

void initVideo(void);
void deinitVideo(void);
void initConsole(void);
void hideConsole(void);
void showConsole(void);
void print(const char *format, ...);
void setConsoleCoo(int, int);
void setConsoleCooAbs(int , int );
void updateVideo(void);
void clearConsole(void);
void drawLine(int x, int y, int x2, int y2);
void visualizePlayingSMP(void);

extern int visualizer;
#endif
