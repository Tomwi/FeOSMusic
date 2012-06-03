#include "FeOSMusic.h"

char cwd[1024];

int main(int argc, char ** argv)
{
	getcwd(cwd, sizeof(cwd));
	initVideo();
	chdir("/");
	retrieveDir("");
	initSoundStreamer();
	
	FeOS_SetAutoUpdate(AUTOUPD_KEYS, false);

	int oldSuspMode = FeOS_SetSuspendMode(SuspendMode_Headphones);

	while(1) {
		
		FeOS_WaitForVBlank();
		updateInput();

		int inSleepMode = keysHold & KEY_LID;

		if (!inSleepMode)
			drawList();
		
		/* Exit program */
		if(keysPres & KEY_START) {
			deinitSoundStreamer();
			unloadCodec();
			freeDir();
			deinitVideo();
			FeOS_SetSuspendMode(oldSuspMode);
			chdir(cwd);
			return 0;
		}
		switch(getStreamState()) {
		case STREAM_WAIT:
		case STREAM_PLAY:
			if (!inSleepMode)
				visualizePlayingSMP();

			if(updateStream()< 0){
				destroyStream(streamIdx);
				break;
			}
			
			if (inSleepMode)
				break;

			updatePrgrBar();
			
			if(keysPres & KEY_A){
				pauseStream();
				break;
			}
			if(keysPres & KEY_B) {
				destroyStream(streamIdx);
				break;
			}
			
			break;
		case STREAM_PAUSE:
			if(!inSleepMode && (keysPres & KEY_A))
				resumeStream();
			break;
		case STREAM_STOP:
			if (!inSleepMode){ 
				updateBrowser();
			}
		}
		
		
	}
	return 0;
}
