#include <feos.h>
#include "input.h"

int keysPres, keysHold, keysReleased;
styluspos_t stylus;

void updateInput(void){
	scanKeys();
	FeOS_GetStylusPos(&stylus);
	keysPres = keysDown() | (keysDownRepeat() & (KEY_DOWN | KEY_UP | KEY_LEFT | KEY_RIGHT));
	keysHold = keysHeld();
	keysReleased = keysUp();
}

bool TouchinArea(int x, int y, int x2, int y2){
	if(stylus.x >= x && stylus.x <=x2){
		if(stylus.y >=y && stylus.y <= y2)
			return true;
	}
	return false;
}