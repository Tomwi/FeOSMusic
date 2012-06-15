#include "FeOSMusic.h"

int main(int argc, char ** argv)
{
	initFeOSMusic();

	while(1) {
		FeOS_WaitForVBlank();
		updateGui();
	}
	return 0;
}
