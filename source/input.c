#include <feos.h>
#include "input.h"

int keysPres, keysHold, keysReleased;

void updateInput(void){
	scanKeys();
	keysPres = keysDown();
	keysHold = keysHeld();
	keysReleased = keysUp();
}