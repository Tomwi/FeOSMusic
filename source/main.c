#include "FeOSMusic.h"

int main(int argc, char ** argv)
{
	initFeOSMusic();
	
	while(1) {

		FeOS_WaitForVBlank();
		updateInput();

		int inSleepMode = keysHold & KEY_LID;

		if (!inSleepMode)
			drawList();

		/* Exit program */
		if(keysPres & KEY_START) {
			deinitFeOSMusic();
		}
		switch(getStreamState()) {
		case STREAM_WAIT:
		case STREAM_PLAY:
			if (!inSleepMode)
				visualizePlayingSMP();

			if(updateStream()< 0) {
				destroyStream(streamIdx);
				break;
			}

			if (inSleepMode)
				break;

			updatePrgrBar();

			if(keysPres & KEY_A) {
				pauseStream();
				break;
			}
			if(keysPres & KEY_B) {
				destroyStream(streamIdx);
				break;
			}
			if(keysPres & KEY_X) {
				if(visualizer == BORKUALIZER)
					visualizer = NORMAL;
				else
					visualizer = BORKUALIZER;
			}
			break;
		case STREAM_PAUSE:
			if(!inSleepMode && (keysPres & KEY_A))
				resumeStream();
			break;
		case STREAM_STOP:
			if (!inSleepMode) {
				updateBrowser();
			}
		}


	}
	return 0;
}
