#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <feos.h>
#include "SndStream.h"

int fifoCh;
instance_t arm7_sndModule;
FILE * currentFile;

#define CLAMP(n,b,u) (n = (n >= u? b : n))
AUDIO_BUFFER outBuf;
AUDIO_BUFFER workBuf;
int frequency, nChans, smpNc;
/* Readbuffer variabelen */
unsigned char readBuf[READ_BUF_SIZE];
unsigned char * readOff;
int dataLeft;
unsigned int count;
hword_t  sampleCount[2];

char arm7Module[] = "/data/FeOS/arm7/arm7SndMod.fx2";

FIFO_AUD_MSG msg;

int initSoundStreamer(void)
{
	arm7_sndModule= FeOS_LoadARM7(arm7Module, &fifoCh);
	if(arm7_sndModule) {
		return 1;
	}
	return 0;
}

int startStream(CODEC_INTERFACE * cdc, char * codecFile, char * file)
{
	loadCodec(codecFile, cdc);
	if(!cdc)
		printf("codec %s not found!\n", codecFile);
	currentFile = cdc->openFile(file);
	sampleCount[0] = sampleCount[1] = 0;

	if(currentFile) {
		frequency 	= cdc->getSampleRate();
		nChans		= cdc->getnChannels();
		/* sample is 2 bytes */
		workBuf.buffer = malloc(STREAM_BUF_SIZE*2*nChans);
		outBuf.buffer = malloc(STREAM_BUF_SIZE*2*nChans);
		if(workBuf.buffer && outBuf.buffer) {

			readOff = readBuf;
			memset(readOff, READ_BUF_SIZE, 0);
			preFill(cdc);

			msg.type = FIFO_AUDIO_START;
			msg.property = (frequency | (nChans << 16));
			msg.buffer = outBuf.buffer;
			msg.bufLen = STREAM_BUF_SIZE;
			fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);

			/* Set the timer to start at a bunch of samples */
			FeOS_TimerWrite(0, (65536-(0x2000000)/frequency)|((TIMER_ENABLE)<<16));
			// cascade mode
			FeOS_TimerWrite(1, ((4|TIMER_ENABLE)<<16));
			return 1;
		}
	}
	printf("Stream failed to start!\n");
	return 0;
}

/*
 * Input: codec
 * Output: return value where nonzero indicates success
 */
int updateStream(CODEC_INTERFACE * cdc)
{
	sampleCount[0] = FeOS_TimerTick(1);
	int smpPlayed = sampleCount[0]-sampleCount[1];

	if(smpPlayed < 0) {
		smpPlayed += 65536;
	}

	sampleCount[1] = sampleCount[0];
	smpNc += smpPlayed;
 	int ret = 0;
	if(smpNc>0) {

		ret = cdc->decSamples((int)(smpNc), &readOff, workBuf.buffer, &dataLeft);
		/* Nothing decoded/decoder error:
		 * free decoder
		 * stop sound
		 * free buffers
		 */
		if(ret <0) {
			msg.type = FIFO_AUDIO_STOP;
			fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);
			cdc->freeDecoder();
			free(outBuf.buffer);
			free(workBuf.buffer);
			FeOS_FreeModule(arm7_sndModule);
			return 0;
		}
		count+=ret;
		int temp = ret;
		// Crossing boundary
		if((outBuf.bufOff + ret) >=STREAM_BUF_SIZE)
			temp = STREAM_BUF_SIZE - outBuf.bufOff;
		deInterleave(workBuf.buffer, &outBuf.buffer[outBuf.bufOff], temp);
		outBuf.bufOff+=temp;
		// still some left
		if(temp!=ret) {
			deInterleave(&workBuf.buffer[temp], outBuf.buffer, ret-temp);
			outBuf.bufOff = ret-temp;
		}
		smpNc -= ret;
	}
	return 1;
}

void deInterleave(void *in, void*out, int samples)
{
	
	short * right = out;
	short * left = right+STREAM_BUF_SIZE;

	for(; samples>0; samples--) {
		*right++ = *((unsigned int*)(in));
		*left++ = (*((unsigned int*)(in+=4))) >> 16;
	}

}

void preFill(CODEC_INTERFACE * cdc)
{
	smpNc = STREAM_BUF_SIZE;
	int ret = 0;
	while(smpNc > 0) {
		ret = cdc->decSamples((int)(smpNc), &readOff, workBuf.buffer, &dataLeft);
		if(ret<=0) {
			break;
		}
		deInterleave(workBuf.buffer, &outBuf.buffer[outBuf.bufOff], ret);
		outBuf.bufOff +=ret;
		smpNc -=ret;
		count +=ret;
	}
	CLAMP(outBuf.bufOff, 0, STREAM_BUF_SIZE);
}
