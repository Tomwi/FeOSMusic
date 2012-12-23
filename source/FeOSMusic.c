#include "FeOSMusic.h"

char savedCwd[1024];
int oldSuspMode;

void initFeOSMusic(const char* cmdLineArg)
{
	srand(time(NULL));
	getcwd(savedCwd, sizeof(savedCwd));
	initGui();
	loadFilters();
	loadCdcList();
	if(!cmdLineArg) {
		chdir("/");
	}
	retrieveDir("");
	initSoundStreamer();
	FeOS_SetAutoUpdate(AUTOUPD_KEYS, false);
	oldSuspMode = FeOS_SetSuspendMode(SuspendMode_Headphones);
	if((streamIdx = createStream(&audioCallbacks))<0) {
		deinitFeOSMusic();
	}
	if(cmdLineArg) {
		playFile(cmdLineArg);
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
	unloadFilters();
	chdir(savedCwd);
	getcwd(savedCwd, sizeof(savedCwd));
	FeOS_SetSuspendMode(oldSuspMode);
	deinitGui();
}
