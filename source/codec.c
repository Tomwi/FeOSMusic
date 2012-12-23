#include "FeOSMusic.h"
#include "file.h"

CODEC_INTERFACE cur_codec;
//#define DEBUG

static int onRead(AUDIO_INFO* audioInfo, int length, void* buf, void * context);
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
		if(strstr(name, &cfgBuf[cdcLst[i].ext])) {
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
			if(!loadCodec(&cfgBuf[cdcLst[ret].cdc]))
				return 0;
		} else if(cdcLst[loadedCodec].cdc==cdcLst[ret].cdc) {
			unloadCodec();
			if(!loadCodec(&cfgBuf[(cdcLst[ret].cdc)]))
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
		setGuiState(GUI_STREAMING);
		printInfo();
		printFilterInfo();
		return 1;
	}
	return 0;
}

static int onRead(AUDIO_INFO* audioInfo, int length, void* buf, void* context)
{
	return cur_codec.decSamples(length, buf, context);
}

static void onClose(void* context)
{
	setGuiState(GUI_BROWSING);
	cur_codec.freeDecoder(context);
	markersX[0] = 0;
	markersX[1] = 256-8;
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
		cur_codec.getTrackCount  = FeOS_FindSymbol(mdl, "getTrackCount");
		cur_codec.setTrack		 = FeOS_FindSymbol(mdl, "setTrack");

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

void loadCdcList(void)
{
	/* scan for cfg files */
	DIR *pdir;
	struct dirent *pent;
	if((pdir=opendir(CFG_FILES_FOLDER))) {
		unsigned int toAlloc = 0; // terminating character
		while ((pent=readdir(pdir))!=NULL) {
			if(!strcmp(&pent->d_name[strlen(pent->d_name)-4], ".cfg")) {
				FILE* fp;
				/* Buffer a config file */
#ifdef DEBUG
				printf("Opening cfg: %s\n", pent->d_name);
#endif
				char filenBuf[256];
				snprintf(filenBuf, sizeof(filenBuf), "%s/%s", CFG_FILES_FOLDER, pent->d_name);
				if((fp=fopen(filenBuf, "rb"))) {
					unsigned sz = getFileSize(fp);
					/* cfgBuf may change, but we only keep track of the offsets of the strings in it*/
					void* tmp = realloc(cfgBuf, toAlloc+sz+1);
					/* Realloc failed, free memory */
					if(tmp==NULL) {
						free(cfgBuf);
						break;
					}
					cfgBuf = tmp;
					char* i=(cfgBuf+toAlloc+sz);
					fread(cfgBuf+toAlloc, 1, sz, fp);
					*i = 0;
					fclose(fp);

					/* Parse the buffer, as the buffer is reallocated, pointers may change,
					 * so this is why offsets are used
					 */
					char* tkn  = strtok(cfgBuf+toAlloc, "=\n");
					char* base = cfgBuf;
					toAlloc+=sz+1;
					while(tkn < i && tkn != NULL) {
						void* tmp = realloc(cdcLst, (numExts+1)*sizeof(CODECFILE));
						if(tmp) {
							cdcLst = tmp;
							cdcLst[numExts].ext = (unsigned int)(tkn-base);
							tkn = strtok(NULL, "=\n");
							if(tkn) {
								cdcLst[numExts].cdc = tkn-base;
								/* windows newlines */
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
				} else {
#ifdef DEBUG
					printf("File doesn't exist!\n");
#endif
				}
			}
		}
	}
#ifdef DEBUG
	int i;
	for(i=0; i<numExts; i++) {
		printf("%s=%s\n", &cfgBuf[cdcLst[i].ext], &cfgBuf[cdcLst[i].cdc]);
	}
#endif
	closedir(pdir);
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
