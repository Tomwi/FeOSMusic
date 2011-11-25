#ifndef OGG_H
#define OGG_H

#include <feos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


FEOS_EXPORT FILE* openFile(char * name);
FEOS_EXPORT int getSampleRate(void);
FEOS_EXPORT int getnChannels(void);
FEOS_EXPORT int seekPercentage(int perc);
FEOS_EXPORT int getPercentage(void);
FEOS_EXPORT void freeDecoder(void);
FEOS_EXPORT int decSamples(int length, unsigned char ** readBuf, short int * destBuf, int *dataLeft);

#endif /* OGG_H */
