#include "FeOSMusic.h"

#define ICONX (256-16)
#define ICONY (0)
char prev[1024];
int state = SINGLE;
u16* playStateFrms[NUM_PLAYLIST_STATES];

static void onEof(void* context)
{
	if(getPlayLstState()==REPEAT) {
		cur_codec.seek(0);
	}
}

int playFile(const char* name)
{
	if(provideCodec(name)) {
		if(state==REPEAT)
			audioCallbacks.onEof = onEof;
		else
			audioCallbacks.onEof = NULL;
		strcpy(prev, name);
		startStream(name, streamIdx);
		return 1;
	}
	return 0;
}

void initPlayLstIcon(void)
{
	void * pal = bufferFile("playlst.pal.bin",NULL);
	u16* iconGfx = bufferFile("playlst.img.bin",NULL);
	if(pal && iconGfx) {
		loadPalette(0, pal, true, SUB_SCREEN);
		int i;
		for(i=0; i<NUM_PLAYLIST_STATES; i++) {
			playStateFrms[i] = loadFrame(iconGfx,  SpriteColorFormat_16Color, SpriteSize_16x16 , i, SUB_SCREEN);
		}
		free(iconGfx);
		free(pal);
		initSprite((ENTS_AL+1), 0, oamGfxPtrToOffset(states(SUB_SCREEN), playStateFrms[0]),SpriteSize_16x16 ,SpriteColorFormat_16Color,SUB_SCREEN);
		setSprXY((ENTS_AL+1), ICONX, ICONY, SUB_SCREEN);
		setSpriteVisiblity(true, (ENTS_AL+1), SUB_SCREEN);
	}
}

void shuffle(void)
{
	if(strlen(prev)>0) {
		int val, files = (numEnt-lastDir)-1;
		char* file = prev;
		if(files>=1) {
			if(files >= 2) {
select:
				val = rand() % files;
				if(!strcmp(prev, &list[val+lastDir+1][ENTRY_NAME])) {
					goto select;
				}
				strcpy(prev, &list[val+lastDir+1][ENTRY_NAME]);
			}
			playFile(file);
		}
	}
}

void updatePlayList(void)
{
	if(keysPres & KEY_TOUCH) {
		if(TouchinArea(ICONX, ICONY, ICONX+16, ICONY+16)) {
			int stat = (state+1)%NUM_PLAYLIST_STATES;
			setPlayLstState(stat);
		}
	}
	switch(state) {
	case SINGLE:
		return;
	case REPEAT:
		return; // handled automatically by onEof callback
	case SHUFFLE:
		if(getStreamState() == STREAM_STOP) {
			shuffle();
		}
		return;
	}
}

int getPlayLstState(void)
{
	return state;
}

void setPlayLstState(int stat)
{
	state = stat;
	setFrame(playStateFrms[stat], (ENTS_AL+1), SUB_SCREEN);
	if(state==REPEAT)
		audioCallbacks.onEof = onEof;
	else
		audioCallbacks.onEof = NULL;
}
