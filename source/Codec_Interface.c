#include "FeOSMusic.h"

CODEC_INTERFACE cur_codec;
AUDIO_CALLBACKS audioCallbacks;
int streamIdx;

int onOpen(const char* name, AUDIO_INFO* inf, void** context)
{
	if(cur_codec.openFile(name)) {
		inf->channelCount = cur_codec.getnChannels();
		inf->frequency = cur_codec.getSampleRate();

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

int onRead(int length, short * buf, void * context)
{
	return cur_codec.decSamples(length, buf, context);
}

void onClose(void * context)
{
	cur_codec.freeDecoder(context);
	glFlush(0);
	bgSetScroll(prgrBar, 0, 0);
	bgHide(prgrBar);
	showConsole();
	int i;
	for(i =0; i<(ENTS_AL+1); i++) {
		setSpriteVisiblity(false, i, SUB_SCREEN);
	}
}

int loadCodec(const char * codecFile)
{
	instance_t mdl = FeOS_LoadModule(codecFile);

	if(mdl) {
		cur_codec.codecModule    = mdl;
		cur_codec.openFile       = FeOS_FindSymbol(mdl, "openFile");
		cur_codec.getSampleRate  = FeOS_FindSymbol(mdl, "getSampleRate");
		cur_codec.getnChannels   = FeOS_FindSymbol(mdl, "getnChannels");
		cur_codec.seek			 = FeOS_FindSymbol(mdl, "seek");
		cur_codec.getPosition  	 = FeOS_FindSymbol(mdl, "getPosition");
		cur_codec.getResolution	 = FeOS_FindSymbol(mdl, "getResolution");
		cur_codec.freeDecoder    = FeOS_FindSymbol(mdl, "freeDecoder");
		cur_codec.decSamples     = FeOS_FindSymbol(mdl, "decSamples");
		cur_codec.deFragReadbuf  = FeOS_FindSymbol(mdl, "deFragReadbuf");
		audioCallbacks.onOpen = onOpen;
		audioCallbacks.onClose = onClose;
		audioCallbacks.onRead = onRead;
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
