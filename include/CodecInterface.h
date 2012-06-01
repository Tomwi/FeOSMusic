#ifndef CODEC_INTERFACE_H
#define CODEC_INTERFACE_H

#include <stdio.h>

typedef struct {
	int (*openFile)(char*);
	int (*getSampleRate)(void);
	int (*getnChannels)(void);
	int (*seekPercentage)(int perc);
	int (*getPercentage) (void);
	void (*freeDecoder)(void * context);
	int (*decSamples)(int length, short * buf, void * context);
	void (*deFragReadbuf)(unsigned char * readBuf, unsigned char ** readOff, int dataLeft);
	instance_t codecModule;
} CODEC_INTERFACE;

int loadCodec(const char * codecFile);
void unloadCodec(void);

extern AUDIO_CALLBACKS audioCallbacks;
extern int streamIdx;
extern CODEC_INTERFACE cur_codec;

#endif /* CODEC_INTERFACE_H */
