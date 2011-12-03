#ifndef CODEC_INTERFACE_H
#define CODEC_INTERFACE_H

#include <stdio.h>

typedef struct {
	int (*openFile)(const char * name);
	int (*getSampleRate)(void);
	int (*getnChannels)(void);
	int (*seekPercentage)(int perc);
	int (*getPercentage) (void);
	void (*freeDecoder)(void);
	int (*decSamples)(int length, short int * destBuf);
	void (*deFragReadbuf)(unsigned char * readBuf, unsigned char ** readOff, int dataLeft);
	instance_t codecModule;
} CODEC_INTERFACE;

int loadCodec(const char * codecFile, CODEC_INTERFACE * cdc);
void unloadCodec(CODEC_INTERFACE * cdc);

#endif /* CODEC_INTERFACE_H */
