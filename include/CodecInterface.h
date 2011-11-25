#ifndef CODEC_INTERFACE_H
#define CODEC_INTERFACE_H

#include <stdio.h>

typedef struct {
	FILE * (*openFile)(char * name);
	int (*getSampleRate)(void);
	int (*getnChannels)(void);
	int (*seekPercentage)(int perc);
	int (*getPercentage) (void);
	void (*freeDecoder)(void);
	int (*decSamples)(int length, unsigned char ** readBuf, short int * destBuf, int *dataLeft);
	instance_t codecModule;
}CODEC_INTERFACE;

int loadCodec(char * codecFile, CODEC_INTERFACE * cdc);

#endif /* CODEC_INTERFACE_H */