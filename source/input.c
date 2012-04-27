#include <feos.h>
#include "input.h"

int keysPres, keysHold, keysReleased;

void updateInput(void){
	scanKeys();
	keysPres = keysDown() | (keysDownRepeat() & (KEY_DOWN | KEY_UP | KEY_LEFT | KEY_RIGHT));
	keysHold = keysHeld();
	keysReleased = keysUp();
}