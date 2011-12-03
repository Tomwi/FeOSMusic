#include "FeosMusic.h"

int fifoCh;
instance_t arm7_sndModule;

AUDIO_BUFFER outBuf;
AUDIO_BUFFER workBuf;
int frequency, nChans, smpNc;
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

		ret = cdc->decSamples(((smpNc)&(~3)), workBuf.buffer);

		if(ret <0) {
			msg.type = FIFO_AUDIO_STOP;
			fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);
			cdc->freeDecoder();
			free(outBuf.buffer);
			free(workBuf.buffer);
			unloadCodec(cdc);
			FeOS_FreeARM7(arm7_sndModule, fifoCh);
			FeOS_TimerWrite(0, 0);
			FeOS_TimerWrite(1, 0);
			printf("Playback stopped\n");
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
	smpNc = STREAM_BUF_SIZE;
	int ret = 0;
	while(smpNc > 0) {

		ret = cdc->decSamples(((smpNc)&(~3)), workBuf.buffer);
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
	int toEnd = ((outBuf.bufOff + samples) > STREAM_BUF_SIZE? STREAM_BUF_SIZE - outBuf.bufOff : samples);

	DC_FlushAll();
	FeOS_DrainWriteBuffer();

copy:

	if(toEnd) {

		switch(nChans) {
			//. Right channel
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

	if(samples) {
		outBuf.bufOff = 0;
		inBuf += toEnd*nChans;
		toEnd = samples;
		goto copy;
	}
	outBuf.bufOff += toEnd;

}
