#include "FeOSMusic.h"

CODEC_INTERFACE cur_codec;

static int onRead(int length, void* buf, void * context);
static int onOpen(const char* name, AUDIO_INFO* inf, void** context);
static void onClose(void * context);

AUDIO_CALLBACKS audioCallbacks = {
	onOpen,
	onRead,
	NULL,
	onClose,
	NULL,
};

CODECFILE* cdcLst;
char* cfgBuf;
int streamIdx, numExts;

int loadedCodec = -1;

int isPlayable(const char* name)
{
	int i;
	for(i =0; i<numExts; i++) {
		if(strstr(name, cdcLst[i].ext)) {
			return i;
		}
	}
	return -1;
}

int provideCodec(const char* name)
{
	int ret;
	if((ret=isPlayable(name))>=0) {
		if(loadedCodec < 0) {
			if(!loadCodec(cdcLst[ret].cdc))
				return 0;
		} else if(strcmp(cdcLst[loadedCodec].cdc,cdcLst[ret].cdc)) {
			unloadCodec();
			if(!loadCodec((cdcLst[ret].cdc)))
				return 0;
		}
		loadedCodec = ret;
		return 1;
	}
	return 0;
}

void deFragReadbuf(unsigned char * readBuf, unsigned char ** readOff, int dataLeft)
{
	memmove(readBuf, *readOff, dataLeft);
	*readOff = readBuf;
}

static int onOpen(const char* name, AUDIO_INFO* inf, void** context)
{
	if(cur_codec.openFile(name)) {
		inf->channelCount = cur_codec.getnChannels();
		inf->frequency = cur_codec.getSampleRate();
		cur_codec.getFlags(&inf->flags);
		hideConsole();
		int i;
		for(i =0; i<(ENTS_AL+1); i++) {
			setSpriteVisiblity(true, i, SUB_SCREEN);
		}
		bgShow(prgrBar);
		return 1;
	}
	return 0;
}

static int onRead(int length, void* buf, void* context)
{
	return cur_codec.decSamples(length, buf, context);
}

static void onClose(void* context)
{
	cur_codec.freeDecoder(context);
	glFlush(0);
	bgSetScroll(prgrBar, 0, 0);
	bgHide(prgrBar);
	showConsole();
}

int loadCodec(const char * codecFile)
{
	instance_t mdl = FeOS_LoadModule(codecFile);

	if(mdl) {
		cur_codec.codecModule    = mdl;
		cur_codec.getFlags		 = FeOS_FindSymbol(mdl, "getFlags");
		cur_codec.openFile       = FeOS_FindSymbol(mdl, "openFile");
		cur_codec.getSampleRate  = FeOS_FindSymbol(mdl, "getSampleRate");
		cur_codec.getnChannels   = FeOS_FindSymbol(mdl, "getnChannels");
		cur_codec.seek			 = FeOS_FindSymbol(mdl, "seek");
		cur_codec.getPosition  	 = FeOS_FindSymbol(mdl, "getPosition");
		cur_codec.getResolution	 = FeOS_FindSymbol(mdl, "getResolution");
		cur_codec.freeDecoder    = FeOS_FindSymbol(mdl, "freeDecoder");
		cur_codec.decSamples     = FeOS_FindSymbol(mdl, "decSamples");
		cur_codec.deFragReadbuf  = FeOS_FindSymbol(mdl, "deFragReadbuf");
		if(cur_codec.deFragReadbuf) {
			*(int**)(cur_codec.deFragReadbuf) = (int*)deFragReadbuf;
		}
		return 1;
	}

	return 0;
}

void unloadCodec(void)
{
	if (cur_codec.codecModule) {
		FeOS_FreeModule(cur_codec.codecModule);
		memset(&cur_codec, 0, sizeof(CODEC_INTERFACE));
	}
}

void loadCdcList(const char* name)
{
	FILE* fp;
	if((fp=fopen(name, "rb"))) {
		unsigned int toAlloc = getFileSize(fp);
		cfgBuf = malloc(toAlloc+1);
		if(cfgBuf) {
			cfgBuf[toAlloc] = '\n';
			char* i=(cfgBuf+toAlloc);
			fread(cfgBuf, 1, toAlloc, fp);
			fclose(fp);
			char* tkn = strtok(cfgBuf, "=\n");
			while(tkn < i && tkn != NULL) {
				void* tmp = realloc(cdcLst, (numExts+1)*sizeof(CODECFILE));
				if(tmp) {
					cdcLst = tmp;
					cdcLst[numExts].ext = tkn;
					tkn = strtok(NULL, "=\n");
					if(tkn) {
						cdcLst[numExts].cdc = tkn;
						if(tkn[strlen(tkn)-1]=='\r')
							tkn[strlen(tkn)-1] = 0;
					} else
						break;
					numExts++;

					if((tkn+strlen(tkn) + 1) < i)
						tkn = strtok(NULL, "=\n");
					else
						break;

				} else {
					freeCdcLst();
					break;
				}
			}
		}
	}
}

void freeCdcLst(void)
{
	if(cdcLst) {
		free(cdcLst);
		cdcLst = NULL;
	}
	if(cfgBuf) {
		free(cfgBuf);
		cfgBuf = NULL;
	}
	numExts = 0;
}
