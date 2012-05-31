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
			unloadCodec(&cur_codec);
			freeDir();
			deinitVideo();
			FeOS_SetSuspendMode(oldSuspMode);
			chdir(cwd);
			return 0;
		}
		switch(getStreamState()) {
		case STREAM_STOP:
			
			break;
		case STREAM_WAIT:
		case STREAM_PLAY:
			if (!inSleepMode)
				visualizePlayingSMP();

			if(!updateStream()) {
				cur_codec.freeDecoder();
				glFlush(0);
				break;
			}

			if (inSleepMode)
				break;

			if(keysPres & KEY_A){
				pauseStream();
				break;
			}
			if(keysPres & KEY_B) {
				stopStream();
				glFlush(0);
				cur_codec.freeDecoder();
				break;
			}
			
			break;
		case STREAM_PAUSE:
			if(!inSleepMode && (keysPres & KEY_A))
				resumeStream();
			break;
		}
		if (!inSleepMode)
			updateBrowser();
	}
	return 0;
}
