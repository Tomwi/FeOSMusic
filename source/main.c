#include "FeOSMusic.h"

int main(int argc, char ** argv)
{
	initFeOSMusic(argc > 1 ? argv[1] : NULL);

	while(1) {
		FeOS_WaitForVBlank();
		updateGui();
	}
	return 0;
}
