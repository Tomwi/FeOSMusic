#ifndef CODEC_INTERFACE_H
#define CODEC_INTERFACE_H

#include <stdio.h>

#define CFG_FILES_FOLDER ("/data/FeOSMusic/cfg")

typedef struct {
	int (*openFile)(const char*);
	int (*getSampleRate)(void);
	void(*getFlags)(int* flags);
	int (*getnChannels)(void);
	int (*getPosition)(void);
	int (*getResolution)(void);
	int (*seek)(int pos);
	int (*getPercentage) (void);
	int (*getTrackCount)(void);
	void (*setTrack)(int);
	void (*freeDecoder)(void * context);
	int (*decSamples)(int length, short * buf, void * context);
	void (*deFragReadbuf)(unsigned char * readBuf, unsigned char ** readOff, int dataLeft);
	instance_t codecModule;
} CODEC_INTERFACE;

typedef struct{
	unsigned int ext;
	unsigned int cdc;
}CODECFILE;

void loadCdcList(void);
void freeCdcLst(void);
extern AUDIO_CALLBACKS audioCallbacks;
extern int streamIdx;
extern CODEC_INTERFACE cur_codec;

int isPlayable(const char* name);
int provideCodec(const char* name);
int loadCodec(const char * codecFile);
void unloadCodec(void);


#endif /* CODEC_INTERFACE_H */
