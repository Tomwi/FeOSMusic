#include "FeOSMusic.h"
#include "browser.h"

int main(int argc, char ** argv)
{
	initVideo();
	chdir("/");
	retrieveDir("");
	initSoundStreamer();
	keysSetRepeat(15, 6);

	while(1) {
		updateInput();
		updateVideo();
		
		FeOS_WaitForVBlank();

		/* Exit program */
		if(keysPres & KEY_START) {
			deinitSoundStreamer(&cur_codec);
			freeDir();
			deinitVideo();
			return 0;
		}

		switch(mixer_status) {
		case STATUS_STOP:
			updateBrowser();
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
	}
	return 0;
}
