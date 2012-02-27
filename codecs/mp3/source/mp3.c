/*
 * Sources:
 * http://www.codeproject.com/KB/audio-video/mpegaudioinfo.aspx
 */

#include "mp3dec.h"
#include "mp3common.h"
#include "mp3.h"
#include "decoder.h"

HMP3Decoder * mdecoder;
MP3FrameInfo inf;

unsigned int firstFrame;	// offset in file of first frameheader
unsigned int fileSize;
unsigned int bitrate;
FILE * fp;

unsigned char readBuf[READ_BUF_SIZE];
unsigned char *readOff;
int dataLeft;

FEOS_EXPORT void (*deFragReadbuf)(unsigned char * readBuf, unsigned char ** readOff, int dataLeft); 

unsigned int get_fileSize(FILE * fp)
{
	unsigned int offset = ftell(fp);
	fseek(fp, 0, SEEK_END);
	unsigned int size = ftell(fp);
	fseek(fp, offset, SEEK_SET);
	return size;
}

int findValidSync(unsigned char ** offset, int * bytes)
{

	int ret;
	while(UnpackFrameHeader((MP3DecInfo *)mdecoder, *offset)<0) {
		// search for another syncword
findsync:
		*offset +=2;
		*bytes -=2;
		// we did enough this mp3 is likely corrupt
		if((ret=MP3FindSyncWord(*offset, *bytes))<0) {
			return -1;
		}
		*offset += ret;
		*bytes -=ret;
	}
	// eh we're decoding mp3, get back!
	if(((MP3DecInfo *)mdecoder)->layer!=3){
		goto findsync;
	}
		

	return 0;
}

void parseID3_V2(FILE * fp)
{
	rewind(fp);
	/* Jump to the sync save size field (4 bytes, where bit 7 is left unused
	 * to avoid finding keywords (which need to have that bit set)
	 */
	fseek(fp, 6, SEEK_SET);
	char temp[4];
	fread(&temp, 1, 4, fp);
	// extract header size to skip it
	firstFrame =  (temp[0]<<21|temp[1]<<14|temp[2]<<7|temp[3])+ID3_HDR_SIZE;
	fseek(fp, firstFrame, SEEK_SET);
	fread(&temp, 1, 3, fp);
	// some mp3's appear to have double headers
	if(IS_ID3_V2(temp)) {
		// get to the size field again
		fseek(fp, firstFrame+6, SEEK_SET);
		fread(&temp, 1, 4, fp);
		firstFrame +=  (temp[0]<<21|temp[1]<<14|temp[2]<<7|temp[3])+ID3_HDR_SIZE;
	}
	fseek(fp, firstFrame, SEEK_SET);
	fileSize -= firstFrame;
}

int openFile(const char * name)
{
	memset(&inf, 0, sizeof(inf));
	readOff = readBuf;
	
	if((fp = fopen(name, "rb"))) {
		fileSize = get_fileSize(fp);
		char magic[3];
		fread(&magic, 1, 3, fp);
		if(IS_ID3_V2(magic)) {
			parseID3_V2(fp);
		}
		if((dataLeft = fread(readBuf, 1, READ_BUF_SIZE, fp))==READ_BUF_SIZE) {
			mdecoder = MP3InitDecoder();
			if(!findValidSync(&readOff, &dataLeft)) {
				/* Get info for mm stream init */
				MP3GetLastFrameInfo(mdecoder, &inf);
				bitrate = inf.bitrate;
				return 1;
			}
		}
	}
	return 0;
}

int getSampleRate(void)
{
	return inf.samprate;
}
int getnChannels(void)
{
	return inf.nChans;
}
int seekPercentage(int perc)
{
	return 0;
}

int getPercentage()
{
	u32 current = ftell(fp);
	current -= firstFrame;
	return ((current*16)/(fileSize/100));
}

void freeDecoder(void)
{
	MP3FreeDecoder(mdecoder);
	fclose(fp);
}

int decSamples(int length, short * destBuf)
{
	int samps =0;
	/* Otherwhise we can possibly overwrite the data behind the buffer */
	if(length >= MAX_NGRAN*inf.nChans*MAX_NSAMP) {
		int ret = 0;
		/* Possible buffer-underrun */
		if(dataLeft < READ_BUF_SIZE) {
			deFragReadbuf(readBuf, &readOff, dataLeft);
			ret = fread(readBuf+dataLeft, 1, READ_BUF_SIZE-dataLeft, fp);
			dataLeft += ret;
			if(feof(fp) && !dataLeft)
				return DEC_EOF;
		}
		/* check for errors */
		if((ret = MP3Decode(mdecoder, &readOff, &dataLeft, destBuf,0))) {
			switch(ret) {
			case ERR_MP3_INDATA_UNDERFLOW :
			case ERR_MP3_MAINDATA_UNDERFLOW:
				return 0;
			case ERR_MP3_INVALID_FRAMEHEADER:
				findValidSync(&readOff, &dataLeft);
				return 0;
			default:
				printf("HELIX MP3 ERROR: %d", ret);
				return DEC_ERR;
			}
		}
		/* GCC can't know channels being only 1 or 2 */
		samps = inf.outputSamps >> (inf.nChans-1);
	}

	return samps;
}
