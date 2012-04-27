#include "FeOSMusic.h"

int loadCodec(const char * codecFile, CODEC_INTERFACE * cdc)
{
	instance_t mdl = FeOS_LoadModule(codecFile);

	if(mdl) {
		cdc->codecModule    = mdl;
		cdc->openFile       = FeOS_FindSymbol(mdl, "openFile");
		cdc->getSampleRate  = FeOS_FindSymbol(mdl, "getSampleRate");
		cdc->getnChannels   = FeOS_FindSymbol(mdl, "getnChannels");
		cdc->seekPercentage = FeOS_FindSymbol(mdl, "seekPercentage");
		cdc->getPercentage  = FeOS_FindSymbol(mdl, "getPercentage");
		cdc->freeDecoder    = FeOS_FindSymbol(mdl, "freeDecoder");
		cdc->decSamples     = FeOS_FindSymbol(mdl, "decSamples");
		cdc->deFragReadbuf  = FeOS_FindSymbol(mdl, "deFragReadbuf");

		if(cdc->deFragReadbuf) {
			*(int**)(cdc->deFragReadbuf) = (int*)deFragReadbuf;
		}
		return 1;
	}

	return 0;
}

void unloadCodec(CODEC_INTERFACE * cdc)
{
	if (cdc && cdc->codecModule)
	{
		FeOS_FreeModule(cdc->codecModule);
		memset(cdc, 0, sizeof(CODEC_INTERFACE));
	}
}
