#include "FeosMusic.h"

int fifoCh;
instance_t arm7_sndModule;

AUDIO_BUFFER outBuf;
AUDIO_BUFFER workBuf;
int frequency, nChans, smpNc;
hword_t  sampleCount[2];
int test;
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
			int temp = ret;
			// Crossing boundary
			if((outBuf.bufOff + ret) > STREAM_BUF_SIZE)
				temp = STREAM_BUF_SIZE - outBuf.bufOff;

			_deInterleave(workBuf.buffer, &outBuf.buffer[outBuf.bufOff], temp);
			outBuf.bufOff+=temp;

			// still some left
			if(temp!=ret) {
				_deInterleave(&workBuf.buffer[temp*cdc->getnChannels()], outBuf.buffer, ret-temp);
				outBuf.bufOff = ret-temp;
			}
			DC_FlushAll();
			FeOS_DrainWriteBuffer();
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
		_deInterleave(workBuf.buffer, &outBuf.buffer[outBuf.bufOff], ret);
		outBuf.bufOff +=ret;
		smpNc -=ret;
	}
	CLAMP(outBuf.bufOff, 0, STREAM_BUF_SIZE);
}

void deFragReadbuf(unsigned char * readBuf, unsigned char ** readOff, int dataLeft)
{
	memmove(readBuf, *readOff, dataLeft);
	*readOff = readBuf;
}
