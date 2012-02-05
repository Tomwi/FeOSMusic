#include <feos.h>
#include "input.h"

int keysPres, keysHold, keysReleased;

void updateInput(void){
	keysUpdate();
	keysPres = keysDown();
	keysHold = keysHeld();
	keysReleased = keysUp();
}