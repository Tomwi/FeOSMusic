#ifndef MP3_H
#define MP3_H

#include <feos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IS_ID3_V2(p)		((p)[0] == 'I' && (p)[1] == 'D' && (p)[2] == '3')
#define ID3_HDR_SIZE 10

#define READ_BUF_SIZE  1940

FEOS_EXPORT FILE* openFile(char * name);
FEOS_EXPORT int getSampleRate(void);
FEOS_EXPORT int getnChannels(void);
FEOS_EXPORT int seekPercentage(int perc);
FEOS_EXPORT int getPercentage(void);
FEOS_EXPORT void freeDecoder(void);
FEOS_EXPORT int decSamples(int length, short * destBuf);

#endif /* MP3_H */