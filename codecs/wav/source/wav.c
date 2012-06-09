#include "wav.h"
#include "decoder.h"

WAV_HEADER curHdr;
FORMAT_CHUNK fmt;
DATA_CHUNK dat;

unsigned int bytSmp;

FILE * fp;
int dataOff;

int openFile(const char * name)
{
	if((fp = fopen(name, "rb"))) {
		fread(&curHdr, 1, sizeof(WAV_HEADER), fp);
		if(IS_RIFF((curHdr.Id))) {
			if(IS_WAVE(curHdr.Fmt)) {
				fread(&fmt, 1, sizeof(FORMAT_CHUNK), fp);
				fseek(fp, (sizeof(WAV_HEADER)+fmt.fmtSz), SEEK_SET);
				fread(&dat, 1, sizeof(DATA_CHUNK), fp);
				bytSmp = fmt.bitSmp>>3;
				return 1;
			}
		}
	}
	return 0;
}

int getSampleRate(void)
{
	return fmt.smpRate;
}

int getnChannels(void)
{
	return fmt.nChans;
}

int seek(int pos)
{
	return 0;
}

int getPosition(void)
{
	return 0;
}

int getResolution(void)
{
	return 1;
}

void freeDecoder(void)
{
	fclose(fp);
}

int decSamples(int length, short * destBuf, void * context)
{
	if(!feof(fp)) {
		int ret = fread(destBuf, 1, length*bytSmp*fmt.nChans, fp);
		if(ret){
		/* convert unsigned to signed PCM */
		if(fmt.bitSmp == 8){
			int i;
			u8* buf = (u8*)destBuf;
			for(i=0; i< ret/(bytSmp); i++){
				*buf++ ^= (1<<7);
			}
		}
		if(feof(fp)) {
			return DEC_EOF;
		}
		return ret/(bytSmp*fmt.nChans);
		}
	}
	return DEC_EOF;
}

void getFlags(int* flags)
{
	*flags = ((fmt.bitSmp == 16? AUDIO_16BIT : 0) |  AUDIO_INTERLEAVED);
}
