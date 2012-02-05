#ifndef BROWSER_H
#define BROWSER_H
#include "FeosMusic.h"
#include "input.h"

extern CODEC_INTERFACE cur_codec;

void retrieveDir(char * path);
void freeDir(void);
void updateBrowser(void);

#endif

