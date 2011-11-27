#include <feos.h>
#include <stdio.h>
#include "CodecInterface.h"
#include "SndStream.h"

CODEC_INTERFACE codec;
int main(int argc, char ** argv)
{
	initSoundStreamer();
	startStream(&codec, argv[2], argv[1]);
	while(1) {
		FeOS_WaitForVBlank();
		if(!updateStream(&codec))
			return 0;
	}

	return 0;
}
