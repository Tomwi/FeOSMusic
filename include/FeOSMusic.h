#ifndef FEOS_MUSIC_H
#define FEOS_MUSIC_H

#include <feos.h>
#include <feos3d.h>
#include <SndStream.h>
#include <far.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>

#include "browser.h"
#include "codec.h"
#include "decoder.h"
#include "file.h"
#include "gui.h"
#include "input.h"
#include "playlist.h"
#include "sprite.h"
#include "video.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CYCLE(n,l,u) ((n) = ((n)>(u) ? (l) : ((n)<(l) ? (u) : (n))))
#define CLAMP(n,l,u) ((n) = ((n) > (u) ? (u) : ((n)<(l) ? (l) : (n))))

void initFeOSMusic(void);
void deinitFeOSMusic(void);
int binLog(int no);

#endif