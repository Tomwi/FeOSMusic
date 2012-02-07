#include "FeosMusic.h"

int fifoCh;
instance_t arm7_sndModule;

AUDIO_BUFFER outBuf;
AUDIO_BUFFER workBuf;
int frequency, nChans, smpNc;
hword_t  sampleCount[2];
char arm7Module[] = "/data/FeOS/arm7/arm7SndMod.fx2";

FIFO_AUD_MSG msg;
char mixer_status;

int initSoundStreamer(void)
{
	arm7_sndModule= FeOS_LoadARM7(arm7Module, &fifoCh);
	if(arm7_sndModule) {
		return 1;
	}
	return 0;
}

void deinitSoundStreamer(CODEC_INTERFACE * cdc)
{
	if(mixer_status == STATUS_PLAY) {
		msg.type = FIFO_AUDIO_STOP;
		fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);
		cdc->freeDecoder();
		free(outBuf.buffer);
		free(workBuf.buffer);
		unloadCodec(cdc);
		FeOS_TimerWrite(0, 0);
		FeOS_TimerWrite(1, 0);
	}
	FeOS_FreeARM7(arm7_sndModule, fifoCh);
}

int startStream(CODEC_INTERFACE * cdc, const char * codecFile, const char * file)
{
	if(!loadCodec(codecFile, cdc)) {
		printf("codec %s not found!\n", codecFile);
		return 0;
	}
	int ret = cdc->openFile(file);
	sampleCount[0] = sampleCount[1] = 0;

	if(ret) {
		frequency 	= cdc->getSampleRate();
		nChans		= cdc->getnChannels();
		/* sample is 2 bytes */
		workBuf.buffer = malloc(STREAM_BUF_SIZE*2*nChans);
		outBuf.buffer = malloc(STREAM_BUF_SIZE*2*nChans);
		if(workBuf.buffer && outBuf.buffer) {
			outBuf.bufOff = 0;
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
 * Input: -
 * Output: Stream is paused
 */
void pauseStream(void)
{
	msg.type = FIFO_AUDIO_PAUSE;
	fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);

	sampleCount[0] = sampleCount[1] = 0;
	FeOS_TimerWrite(0, 0);
	FeOS_TimerWrite(1, 0);

	int i,j;
	int size = STREAM_BUF_SIZE-smpNc;
	int start = outBuf.bufOff-size;
	if(start < 0)
		start +=STREAM_BUF_SIZE;

	for(j=0; j<nChans; j++) {
		for(i = 0; i<STREAM_BUF_SIZE-smpNc; i++) {
			workBuf.buffer[STREAM_BUF_SIZE*j + i] = outBuf.buffer[STREAM_BUF_SIZE*j + (start + i)%8192];
		}
	}
	memset(outBuf.buffer, 0, STREAM_BUF_SIZE*2);
	mixer_status = STATUS_PAUSE;
	memcpy(outBuf.buffer, workBuf.buffer, (STREAM_BUF_SIZE-smpNc)*2);
	if(nChans == 2)
		memcpy(outBuf.buffer+STREAM_BUF_SIZE, workBuf.buffer+STREAM_BUF_SIZE, (STREAM_BUF_SIZE-smpNc)*2);

	outBuf.bufOff = STREAM_BUF_SIZE-smpNc;
	DC_FlushAll();
	FeOS_DrainWriteBuffer();
}

void resumeStream(void)
{
	msg.type = FIFO_AUDIO_RESUME;
	fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);

	FeOS_TimerWrite(0, (65536-(0x2000000)/frequency)|((TIMER_ENABLE)<<16));
	FeOS_TimerWrite(1, ((4|TIMER_ENABLE)<<16));
	mixer_status = STATUS_PLAY;
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

		ret = cdc->decSamples(smpNc, workBuf.buffer);

		if(ret <0) {
			msg.type = FIFO_AUDIO_STOP;
			fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);
			cdc->freeDecoder();
			free(outBuf.buffer);
			free(workBuf.buffer);
			unloadCodec(cdc);
			FeOS_TimerWrite(0, 0);
			FeOS_TimerWrite(1, 0);
			return 0;
		}
		if(ret) {
			copySamples(workBuf.buffer, 1, ret);
			smpNc -= ret;
		}

	}
	return 1;
}

void preFill(CODEC_INTERFACE * cdc)
{
	smpNc = STREAM_BUF_SIZE-outBuf.bufOff;
	int ret = 0;
	while(smpNc > 0) {

		ret = cdc->decSamples(smpNc, workBuf.buffer);
		if(ret<=0) {
			break;
		}
		copySamples(workBuf.buffer, 1, ret);
		smpNc -=ret;
	}
}

void deFragReadbuf(unsigned char * readBuf, unsigned char ** readOff, int dataLeft)
{
	memmove(readBuf, *readOff, dataLeft);
	*readOff = readBuf;
}

/*
 * Input:
 * 	-Pointer to buffer with decoded samples
 * 	-Interleave flag (only set when stereo, but even then not mandatory
 * 						depending on the decoders output)
 *	Output:
 * 	-Decoded samples in the outBuffer(s)
 */
void copySamples(short * inBuf, int deinterleave, int samples)
{
	// Deinterleave will fail otherwise (deinterleaves 4n samples)
	samples &= (~3); // bic
	int toEnd = ((outBuf.bufOff + samples) > STREAM_BUF_SIZE? STREAM_BUF_SIZE - outBuf.bufOff : samples);
	toEnd  	&= (~3);

copy:

	if(toEnd) {

		switch(nChans) {
			// Right channel
		case 2:
			if(!deinterleave)
				memcpy(&outBuf.buffer[STREAM_BUF_SIZE+outBuf.bufOff], &inBuf[toEnd], toEnd*2);
			// has to be stereo
			else {
				_deInterleave(inBuf, &outBuf.buffer[outBuf.bufOff], toEnd);
				break;
			}
			//Left channel
		case 1:
			memcpy(&outBuf.buffer[outBuf.bufOff], inBuf, toEnd*2);
			break;
		}
	}
	samples -= toEnd;
	/* There was a split */
	if(samples) {
		outBuf.bufOff = 0;
		inBuf += toEnd*nChans;
		toEnd = samples;
		goto copy;
	}
	outBuf.bufOff += toEnd;

	DC_FlushAll();
	FeOS_DrainWriteBuffer();
}
