#include "FeOSMusic.h"

char cwd[1024];
int oldSuspMode;

#define CFG_PATH ("data/FeOS/audiocodecs.cfg")
void initFeOSMusic(void)
{
	srand(time(NULL));
	getcwd(cwd, sizeof(cwd));
	initVideo();
	chdir("/");
	loadCdcList(CFG_PATH);
	retrieveDir("");
	initSoundStreamer();

	FeOS_SetAutoUpdate(AUTOUPD_KEYS, false);
	oldSuspMode = FeOS_SetSuspendMode(SuspendMode_Headphones);
	if((streamIdx = createStream(&audioCallbacks))<0) {
		deinitFeOSMusic();
	}
}

void deinitFeOSMusic(void)
{
	if (streamIdx >= 0)
		destroyStream(streamIdx);
	deinitSoundStreamer();
	unloadCodec();
	freeCdcLst();
	freeDir();
	deinitVideo();
	FeOS_SetSuspendMode(oldSuspMode);
	chdir(cwd);
}
