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
			deinitSoundStreamer(&cur_codec);
			freeDir();
			FeOS_SetAutoUpdate(AUTOUPD_KEYS, true);
			deinitVideo();
			
			chdir(cwd);
			FeOS_ConsoleMode();
			return 0;
		}
		switch(mixer_status) {
		case STATUS_STOP:
			
			break;
		case STATUS_WAIT:
		case STATUS_PLAY:
		visualizePlayingSMP();
			if(!updateStream(&cur_codec)) {
				mixer_status = STATUS_STOP;
				break;
			}
			if(keysPres & KEY_A){
				pauseStream(&cur_codec);
				break;
			}
			if(keysPres & KEY_B) {
				stopStream(&cur_codec);
				mixer_status = STATUS_STOP;
				break;
			}
			
			break;
		case STATUS_PAUSE:
			if(keysPres & KEY_A)
				resumeStream();
			break;
		}
		updateBrowser();
	}
	return 0;
}
