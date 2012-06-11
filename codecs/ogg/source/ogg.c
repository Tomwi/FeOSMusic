#include "ivorbisfile.h"
#include "ogg_t.h"
#include <feos.h>
#include "decoder.h"
#define RESOLUTION (256)
FEOS_EXPORT unsigned char readBuf[READ_BUF_SIZE];
FEOS_EXPORT unsigned char *readOff;
FEOS_EXPORT int dataLeft;
OggVorbis_File vf;
static int current_section;
vorbis_info *vi = NULL;
/*
Opens an OGG file
*/
int openFile(const char * name)
{
	FILE * fp = fopen(name, "rb");
	if(fp) {
		int ret = ov_open(fp, &vf, NULL,0);
		if(!ret) {
			vi=ov_info(&vf,-1);
			return 1;
		}
	}
	ov_clear(&vf);
	return 0;
}
void getFlags(int* flags)
{
	*flags = (AUDIO_INTERLEAVED | AUDIO_16BIT);
}
int getSampleRate(void)
{
	return vi->rate;
}
int getnChannels(void)
{
	return vi->channels;
}
int getPosition(void)
{
	return (int)((ov_time_tell(&vf))/(ov_time_total(&vf, -1)/RESOLUTION));
}
int seek(int pos)
{
	int ret = ov_time_seek(&vf,pos*(ov_time_total(&vf, -1)/RESOLUTION));
	if(ret == 0)
		return 0;
	return -1;
}
int getResolution(void)
{
	return RESOLUTION;
}
int decSamples(int length, short * destBuf, void * context)
{
	char *target = (char*)destBuf;
	if(length >= 1024) {
		int tlength = length*vi->channels*2;
		while(tlength) {
			/* Read enough bytes, 4* for stereo, 2*for mono */
			int ret=ov_read(&vf,target,tlength, &current_section);
			/* Decoding error or EOF*/
			if(ret <= 0) {
				if(!ret)
					return DEC_EOF;
				ov_clear(&vf);
				return DEC_ERR;
			}
			tlength -= ret;
			target += ret;
		}
		return length; /* Return how many samples are decoded */
	}
	return 0;
}
void freeDecoder(void)
{
	ov_clear(&vf);
}
