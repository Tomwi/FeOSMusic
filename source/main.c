#include "FeosMusic.h"
#include "browser.h"

int main(int argc, char ** argv)
{
	initVideo();
	chdir("/");
	retrieveDir("");
	initSoundStreamer();

	while(1) {
		updateInput();
		updateVideo();
		glFlush(0);
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
			if(!updateStream(&cur_codec)) {
				mixer_status = STATUS_STOP;
			}
			if(keysPres & KEY_A)
				pauseStream(&cur_codec);
			if(keysPres & KEY_B) {
				stopStream(&cur_codec);
				mixer_status = STATUS_STOP;
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
