#ifndef OGG_H
#define OGG_H

#include <feos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define READ_BUF_SIZE 		1940

FEOS_EXPORT FILE* openFile(char * name);
FEOS_EXPORT int getSampleRate(void);
FEOS_EXPORT int getnChannels(void);
FEOS_EXPORT int seekPercentage(int perc);
FEOS_EXPORT int getPercentage(void);
FEOS_EXPORT void freeDecoder(void);
FEOS_EXPORT int decSamples(int length, short * destBuf);

#endif /* OGG_H */
