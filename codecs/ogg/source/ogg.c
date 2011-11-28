#include "ivorbisfile.h"
#include "ogg_t.h"
#include <feos.h>

FEOS_EXPORT unsigned char readBuf[READ_BUF_SIZE];
FEOS_EXPORT unsigned char *readOff;
FEOS_EXPORT int dataLeft;

OggVorbis_File vf;
static int current_section;
vorbis_info *vi = NULL;

/*
Opens an OGG file
*/
FILE* openFile(char * name) {
	FILE * fp = fopen(name, "rb");
	if(fp) {
		int ret = ov_open(fp, &vf, NULL,0);
		vi=ov_info(&vf,-1);
		return fp;
	}
	ov_clear(&vf);
	return NULL;
}

int getSampleRate(void) {
	return vi->rate;
}

int getnChannels(void) {
	return vi->channels;
}
int getPercentage(void){
	return (int)((ov_time_tell(&vf)*16)/(ov_time_total(&vf, -1)/100));
}
int seekPercentage(int perc){
	int ret = ov_time_seek(&vf,perc*(ov_time_total(&vf, -1)/100));
	if(ret == 0)
		return 0;
	return -1;
}

int decSamples(int length, short * destBuf){

	short *target = destBuf;
	if(length >= 1024){
	int tlength = 1024;

	while(tlength) {
		/* Read enough bytes, 4* for stereo, 2*for mono */
		int ret=ov_read(&vf,target,tlength*vi->channels*2, &current_section);
		/* Decoding error or EOF*/
		if(ret <= 0) {
			ov_clear(&vf);
			return -1;
		}
		tlength -= ret/(vi->channels*2);
		target +=ret/2; // we increase a s16 pointer so half the byte size
	}
	return 1024; /* Return how many samples are decoded */
	}
	return 0;
}

void freeDecoder(void) {
	ov_clear(&vf);
}