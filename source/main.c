#include "FeosMusic.h"
#include "browser.h"

int main(int argc, char ** argv)
{
	retrieveDir("/");
	initSoundStreamer();

	while(1) {
		updateInput();
		FeOS_WaitForVBlank();
		/* Exit program */
		if(keysPres & KEY_START){
			deinitSoundStreamer(&cur_codec);
			freeDir();
			return 0;
		}
		
		switch(mixer_status) {
		case STATUS_STOP:
			updateBrowser();
			break;
		case STATUS_PLAY:
			if(!updateStream(&cur_codec)) {
				mixer_status = STATUS_STOP;
			}
			if(keysPres & KEY_A)
				pauseStream();
			break;
		case STATUS_PAUSE:
			if(keysPres & KEY_A)
				resumeStream();
			break;
		}

	}
	return 0;
}
