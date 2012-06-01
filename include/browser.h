#ifndef BROWSER_H
#define BROWSER_H
#include "FeOSMusic.h"
#include "input.h"


#define ICON_SZ (32)
#define ENTS_AL (SCREEN_HEIGHT/ICON_SZ)

void retrieveDir(char * path);
void freeDir(void);
void updateBrowser(void);
void drawList(void);

#endif

