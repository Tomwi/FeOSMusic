#ifndef WAV_H
#define WAV_H

#include <feos.h>
#include <SndStream.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IS_RIFF(p)		((p)[0] == 'R' && (p)[1] == 'I' && (p)[2] == 'F' && (p)[3] == 'F')
#define IS_WAVE(p)		((p)[0] == 'W' && (p)[1] == 'A' && (p)[2] == 'V' && (p)[3] == 'E')
#define IS_FMT(p)		((p)[0] == 'f' && (p)[1] == 'm' && (p)[2] == 't')
#define IS_DAT(p)		((p)[0] == 'd' && (p)[1] == 'a' && (p)[2] == 't' && (p)[3] == 'a')	

enum AUDIO_FORMAT{
	PCM = 1,
};

typedef struct{
	int8_t 	 fmtId[4];
	int32_t	 fmtSz;
	uint16_t audFmt;
	uint16_t nChans;
	int32_t  smpRate;
	int32_t  bytRate;
	uint16_t blckAlgn;
	uint16_t bitSmp;
	uint16_t extSz;
}FORMAT_CHUNK;

typedef struct{
	int8_t	datId[4];
	int32_t datSz;
}DATA_CHUNK;

typedef struct{
	int8_t	Id[4];
	int32_t chunkSize;
	int8_t	Fmt[4];
}WAV_HEADER;



FEOS_EXPORT int openFile(const char * name);
FEOS_EXPORT int getSampleRate(void);
FEOS_EXPORT int getnChannels(void);
FEOS_EXPORT int seek(int pos);
FEOS_EXPORT int getPosition(void);
FEOS_EXPORT int getResolution(void);
FEOS_EXPORT void freeDecoder(void);
FEOS_EXPORT int decSamples(int length, short * destBuf, void * context);
FEOS_EXPORT void getFlags(int* flags);

#endif /* WAV_H */
