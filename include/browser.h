#ifndef BROWSER_H
#define BROWSER_H
#include "FeOSMusic.h"
#include "input.h"


#define ICON_SZ (32)
#define ENTS_AL (SCREEN_HEIGHT/ICON_SZ)

#define ENTRY_TYPE (0)
#define ENTRY_NAME (1)

extern char** list;
extern int numEnt, lastDir;

void retrieveDir(char * path);
void freeDir(void);
void updateBrowser(void);
void drawList(void);

#endif

