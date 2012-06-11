#include "FeOSMusic.h"

char cwd[1024];
int oldSuspMode;

void initFeOSMusic(void)
{
	srand(time(NULL));
	getcwd(cwd, sizeof(cwd));
	initVideo();
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
	deinitSoundStreamer();
	destroyStream(streamIdx);
	unloadCodec();
	freeDir();
	deinitVideo();
	FeOS_SetSuspendMode(oldSuspMode);
	chdir(cwd);
}
