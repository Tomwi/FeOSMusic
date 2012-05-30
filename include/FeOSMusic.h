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

#include "CodecInterface.h"

#include "input.h"
#include "video.h"
#include "sprite.h"
#include "file.h"
#include "decoder.h"
#include "browser.h"

#define CLAMP(n,l,u) ((n) = ((n) > (u) ? (u) : ((n)<l ? (l) : (n))))

#endif