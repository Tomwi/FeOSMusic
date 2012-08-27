#include "FeOSMusic.h"

char cwd[1024];
int oldSuspMode;

void initFeOSMusic(void)
{
	srand(time(NULL));
	getcwd(cwd, sizeof(cwd));
	initGui();
	loadFilters();
	chdir(CFG_FILES_FOLDER);
	loadCdcList();
	chdir("/");
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
	unloadFilters();
	deinitGui();
	FeOS_SetSuspendMode(oldSuspMode);
	chdir(cwd);
}
