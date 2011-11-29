#include "FeosMusic.h"

int loadCodec(char * codecFile, CODEC_INTERFACE * cdc){
	instance_t mdl 		= 	FeOS_LoadModule(codecFile);
	if(mdl){
	cdc->openFile 		= 	FeOS_FindSymbol(mdl, "openFile");
	cdc->getSampleRate 	=	FeOS_FindSymbol(mdl, "getSampleRate");
	cdc->getnChannels 	= 	FeOS_FindSymbol(mdl, "getnChannels");
	cdc->seekPercentage	=	FeOS_FindSymbol(mdl, "seekPercentage");
	cdc->getPercentage	=	FeOS_FindSymbol(mdl, "getPercentage");
	cdc->freeDecoder	=	FeOS_FindSymbol(mdl, "freeDecoder");
	cdc->decSamples		= 	FeOS_FindSymbol(mdl, "decSamples");
	cdc->codecModule	=	mdl;
	cdc->deFragReadbuf	=	FeOS_FindSymbol(mdl, "deFragReadbuf");
	if(cdc->deFragReadbuf){
	*(int**)(cdc->deFragReadbuf) = 	(int*)deFragReadbuf;
	}
	return 1;
	}
	else{
		printf("%s module failed to load!\n", codecFile);
		return -1;
	}
}

void unloadCodec(CODEC_INTERFACE * cdc){
	FeOS_FreeModule(cdc->codecModule);
}