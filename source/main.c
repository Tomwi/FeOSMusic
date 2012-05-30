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

	while(1) {
		
		FeOS_WaitForVBlank();
		updateInput();
		drawList();
		
		/* Exit program */
		if(keysPres & KEY_START) {
			deinitSoundStreamer();
			freeDir();
			deinitVideo();
			
			chdir(cwd);
			return 0;
		}
		switch(getStreamState()) {
		case STREAM_STOP:
			
			break;
		case STREAM_WAIT:
		case STREAM_PLAY:
		visualizePlayingSMP();
			if(!updateStream()) {
				setStreamState(STREAM_STOP);
				break;
			}
			if(keysPres & KEY_A){
				pauseStream();
				break;
			}
			if(keysPres & KEY_B) {
				stopStream();
				setStreamState(STREAM_STOP);
				break;
			}
			
			break;
		case STREAM_PAUSE:
			if(keysPres & KEY_A)
				resumeStream();
			break;
		}
		updateBrowser();
	}
	return 0;
}
