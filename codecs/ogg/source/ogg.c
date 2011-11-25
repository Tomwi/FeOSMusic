#include "ivorbisfile.h"
#include "ogg_t.h"


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
		printf("opened file %s\n", name);
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

int decSamples(int length, unsigned char ** readBuf, short int * destBuf, int *dataLeft){

	short *target = destBuf;
	int tlength = length;

	while(tlength) {
		/* Read enough bytes, 4* for stereo, 2*for mono */
		int ret=ov_read(&vf,target,tlength*vi->channels*2, &current_section);
		/* Decoding error or EOF*/
		if(ret <= 0) {
			ov_clear(&vf);
			return 0;
		}
		tlength -= ret/(vi->channels*2);
		target +=ret/2; // we increase a s16 pointer so half the byte size
	}
	return length; /* Return how many samples are decoded */
}

void freeDecoder(void) {
	ov_clear(&vf);
}