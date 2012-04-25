#ifndef SND_STREAM_H
#define SND_STREAM_H


#include "CodecInterface.h"

#define CLAMP(n,b,u) (n = (n >= u? b : n))
#define BUS_CLOCK   (33513982)
#define TIMER_IRQ_ENABLE	(1<<6)	
#define TIMER_ENABLE		(1<<7)

/* amount of samples to be counted every timer0 irq */
#define SAMPLE_PACKET		32		
#define READ_BUF_SIZE 		1940
// samples per channel
#define STREAM_BUF_SIZE		8192	
/* Message types */
#define FIFO_AUDIO_START	1
#define FIFO_AUDIO_STOP		2
#define FIFO_AUDIO_PAUSE	3
#define FIFO_AUDIO_RESUME	4

/* Mixer status */
#define STATUS_STOP			0
#define STATUS_PLAY			1
#define STATUS_PAUSE		2
#define STATUS_WAIT 		3

/* decoder status */
#define DEC_ERR				-1
#define DEC_EOF				-2


typedef struct {
	int type;			// kind of audio message
	unsigned int property;	// (tmr_value & (n_Channels << 16))
	int bufLen;		// Length of the buffer for just one channel
	void * buffer;	// pointer to sample buffer
} FIFO_AUD_MSG;

typedef struct{
	short * buffer;
	int bufLen;
	int bufOff;
}AUDIO_BUFFER;

int startStream(CODEC_INTERFACE * cdc, const char * codecFile, const char * file);
void pauseStream(CODEC_INTERFACE * cdc);
void resumeStream(void);
void stopStream(CODEC_INTERFACE * cdc);
int updateStream(CODEC_INTERFACE * cdc);
void _deInterleave(short *in, short *out, int samples);
void preFill(CODEC_INTERFACE * cdc);
void deFragReadbuf(unsigned char * readBuf, unsigned char ** readOff, int dataLeft);
void copySamples(short * inBuf, int deinterleave, int samples);
void visualizePlayingSMP(void);
/*
 * Initalize arm7 module for sound playback
 * Returns:
 * 0 on failure (Module couldn't be loaded)
 * 1 on success
 */
int initSoundStreamer(void);
void deinitSoundStreamer(CODEC_INTERFACE * cdc);

extern char mixer_status;

#endif /* SND_STREAM_H */
