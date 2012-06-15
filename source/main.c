#include "FeOSMusic.h"

int main(int argc, char ** argv)
{
	initFeOSMusic();
	
	while(1) {

		FeOS_WaitForVBlank();
		updateInput();

		int inSleepMode = keysHold & KEY_LID;

		/* Exit program */
		if(keysPres & KEY_START) {
			deinitFeOSMusic();
			return 0;
		}
		switch(getStreamState()) {
		case STREAM_WAIT:
		case STREAM_PLAY:
			if (!inSleepMode)
				visualizePlayingSMP();

			if(updateStream()< 0) {
				stopStream();
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
				setPlayLstState(SINGLE);
				stopStream();
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
				drawList();
				updateBrowser();
			}
		}
	updatePlayList();
	}
	return 0;
}
