/*
*	MP4 files can contain aac tracks, but they need to be proccessed
*	differently than just raw aac (.aac) files, which are easier
*	to handle.
* 	The streaming functions are based upon the example found in the helix
* 	aac decoder source
*/
#include <feos.h>
#include "aacdec.h"
#include "aaccommon.h"
#include "mp4ff.h"
#include "aac.h"

FILE * fp;

/* Helix variabelen*/
HAACDecoder * decoder;
AACFrameInfo inf;

/* MP4ff variabelen */
int trackSample;
static int samples;
int track;

unsigned char readBuf[READ_BUF_SIZE];
unsigned char *readOff;
int dataLeft;

FEOS_EXPORT void (*deFragReadbuf)(unsigned char * readBuf, unsigned char ** readOff, int dataLeft);

uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
	return fread(buffer, 1, length, (FILE*)user_data);
}
uint32_t seek_callback(void *user_data, uint64_t position)
{
	return fseek((FILE*)user_data, position, SEEK_SET);
}
mp4ff_t *infile;
mp4ff_callback_t mp4cb = {
	.read = read_callback,
	.seek = seek_callback
};

int findAudioTrack(mp4ff_t * f)
{
	int i, numTracks = mp4ff_total_tracks(f);
	for(i =0; i<numTracks; i++) {
		/* Found an audio track */
		if(mp4ff_get_track_type(f, i) == MP4_TRACK_AUDIO)
			return i;
	}
	return -1;
}
/*
 * Input	:	Filename
 * Output	: 	1 if succesful 0 if not
 */
int openFile(const char * name)
{
	readOff = readBuf;
	if((fp = fopen(name, "rb"))) {
		mp4cb.user_data = fp;
		if((infile = mp4ff_open_read(&mp4cb))) {
			if ((track = findAudioTrack(infile)) >= 0) {
				/* Decoder failed to initialize */
				if((decoder = AACInitDecoder())) {
					/* Decoder should be updated to decode raw blocks in the mp4 */
					inf.sampRateCore = mp4ff_get_sample_rate(infile, track);
					inf.nChans = mp4ff_get_channel_count(infile,track);
					/* AACSetRawBlockParams will fail if not set */
					inf.profile = AAC_PROFILE_LC;
					samples = mp4ff_num_samples(infile, track);
					if(!AACSetRawBlockParams(decoder, 0, &inf))
						return 1;
				}
			}
		}
	}
	return 0;
}

/*
 * Note	: Use only after you've opened up a valid AAC file
 */
int getSampleRate(void)
{
	return inf.sampRateCore;
}
/*
 * Note	: Use only after you've opened up a valid AAC file
 */
int getnChannels(void)
{
	return inf.nChans;
}

int getPercentage(void)
{
	return 0;
}
int seekPercentage(int perc)
{
	return 0;
}

void freeDecoder(void)
{
	trackSample = 0;
	AACFreeDecoder(decoder);
	memset(&inf, 0, sizeof(AACFrameInfo));
	fclose(fp);
}

int decSamples(int length, short * destBuf)
{

	int ret = 0;
	/* We'll always output 1024 (nah almost) samples */
	if(length >=1024) {
		int read = 0;
		/* The decoder is always fed one aac frame */
		if(trackSample < samples) {
			/* Move old data to the beginning of the buffer */
			deFragReadbuf(readBuf, &readOff, dataLeft);
			/* Read sample for decoding*/
			read = mp4ff_read_sample_v2(infile, track, trackSample++, readBuf+dataLeft);
			/* Update amount of data in readBuffer */
			dataLeft+=read;
			memset(readBuf+dataLeft, 0, READ_BUF_SIZE - dataLeft);
			/* Decode sample, after return the external application should move the oldest
			 * data to the beginning of the readbuffer */
			ret = AACDecode(decoder, &readOff, &dataLeft, destBuf);
			return 1024;
		}
		/* Either a sample read error occured , an decoding error occured or simply EOF */
		if(!read || ret) {
			mp4ff_close(infile);
			return -1;
		}
	}
	/* Nothing is decoded: too few samples requested */
	return 0;
}
