#include "FeosMusic.h"

#define TIMER_CASCADE   (1<<2)
#define TIMER_IRQ_REQ   (1<<6)

int fifoCh;
instance_t arm7_sndModule;

AUDIO_BUFFER outBuf;
AUDIO_BUFFER workBuf;
int frequency, nChans, smpNc;
hword_t  sampleCount[2];
char arm7Module[] = "/data/FeOS/arm7/arm7SndMod.fx2";

FIFO_AUD_MSG msg;
char mixer_status;

void fifoValHandler(u32 value32, void *userdata)
{
	switch(value32) {
		// Sound channels are enabled, enable timers
	case FIFO_AUDIO_START:
		/* Set the timer to start at a bunch of samples */
		FeOS_TimerWrite(0, (65536-(0x2000000)/frequency)|((TIMER_ENABLE)<<16));
		// cascade mode
		FeOS_TimerWrite(1, ((4|TIMER_ENABLE)<<16));
		mixer_status = STATUS_PLAY;
		break;
	default:
		printf("FIFO returned %d\n", value32);
		break;
	}
}
int initSoundStreamer(void)
{
	arm7_sndModule= FeOS_LoadARM7(arm7Module, &fifoCh);

	if(arm7_sndModule) {
		// For handling states returned by arm7
		fifoSetValue32Handler(fifoCh, fifoValHandler, NULL);
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
			return 1;
		}
	}
	return 0;
}

/*
 * Input: -
 * Output: Stream is paused
 * Details:
 * Stream is paused by moving unplayed samples to the start of the buffer(s)
 * and then resetting every timer so that when the stream is resumed no
 * samples are skipped
 */
void pauseStream(CODEC_INTERFACE * cdc)
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
	memcpy(outBuf.buffer, workBuf.buffer, size*2);
	if(nChans == 2)
		memcpy(outBuf.buffer+STREAM_BUF_SIZE, workBuf.buffer+STREAM_BUF_SIZE, size*2);

	outBuf.bufOff = size;
	mixer_status = STATUS_PAUSE;
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
 * Input: -
 * Output: Stopped stream, however the used codec is not free'ed!
 */
void stopStream(CODEC_INTERFACE * cdc)
{
	msg.type = FIFO_AUDIO_STOP;
	fifoSendDatamsg(fifoCh, sizeof(FIFO_AUD_MSG), &msg);
	free(outBuf.buffer);
	free(workBuf.buffer);
	FeOS_TimerWrite(0, 0);
	FeOS_TimerWrite(1, 0);
	cdc->freeDecoder();
}

/*
 * Input: codec
 * Output: return value where nonzero indicates success
 */
int updateStream(CODEC_INTERFACE * cdc)
{
	sampleCount[0] = FeOS_TimerTick(1);
	int smpPlayed = sampleCount[0]-sampleCount[1];
	smpPlayed += (smpPlayed < 0 ? 65536 : 0);

	sampleCount[1] = sampleCount[0];
	smpNc += smpPlayed;
	int ret = DEC_EOF;

	if(smpNc>0) {
decode:
		if(mixer_status != STATUS_WAIT)
			ret = cdc->decSamples((smpNc&(~3)), workBuf.buffer);
		switch(ret) {
		case DEC_ERR:
			stopStream(cdc);
			return 0;
		case DEC_EOF:
			mixer_status = STATUS_WAIT;
			if(smpNc >= STREAM_BUF_SIZE) {
				stopStream(cdc);
				return 0;
			}
			int i,j;
			/* No more samples to decode, but still playing, fill zeroes */
			for(j=0; j<nChans; j++) {
				for(i=outBuf.bufOff; i<(outBuf.bufOff+smpPlayed); i++) {
					/* STREAM_BUF_SIZE is a power of 2 */
					outBuf.buffer[(i%STREAM_BUF_SIZE)+STREAM_BUF_SIZE*j] = 0;
				}
			}
			outBuf.bufOff = (outBuf.bufOff + smpPlayed)%STREAM_BUF_SIZE;
			break;
		default:
			if(ret > 0) {
				copySamples(workBuf.buffer, 1, ret);
				smpNc -= ret;
				if(smpNc>0)
					goto decode;
			}
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
	/* I don't care about the lag*/
	visualize(&inBuf[toEnd], toEnd, nChans);
	
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