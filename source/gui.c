#include "FeOSMusic.h"
#include <assert.h>

#define PRGRBAR_Y (SCREEN_HEIGHT/(8*2) - 1)

static int state = GUI_BROWSING;
u16* iconFrames[NUM_FRAMES];
int prgrBar, prgr;

void initPrgrBar(void)
{
	int sz = 0;
	u16* prgrGfx = bufferFile("prgr.img.bin", &sz);

	prgrBar = bgInitSub(1, BgType_Text4bpp, BgSize_T_512x256, 3,1);

	dmaCopy(prgrGfx, bgGetGfxPtr(prgrBar), sz);
	free(prgrGfx);
	u16 * map = bgGetMapPtr(prgrBar);
	dmaFillHalfWords(0, map, 64*32*2);
	map += PRGRBAR_Y*32 + 32*32;
	int i,j;
	for(i=1; i<3; i++) {
		for(j = 0; j<32; j++) {
			map[j] = i;
		}
		map+=32;
	}
	bgHide(prgrBar);
}

void updatePrgrBar(void)
{
	if(keysHold & KEY_TOUCH) {
		if(stylus.y > PRGRBAR_Y*8 && stylus.y < (PRGRBAR_Y*8 + 2*8)) {
			prgr = stylus.x;
			bgSetScroll(prgrBar, -stylus.x, 0);
		}
	} else if(keysReleased & KEY_TOUCH) {
		if(prgr) {
			int seek = ((cur_codec.getResolution())*prgr)>>8;
			cur_codec.seek(seek);
			prgr = 0;
		}
	} else {
		u64 pos = (cur_codec.getPosition()<<8)/(cur_codec.getResolution());
		bgSetScroll(prgrBar, -pos, 0);
	}
}

void initGui(void)
{
	initVideo();
	/* Initialize console, the copied palette is a shared palette */
	int sz;
	u16* sharedPal = bufferFile("shared.pal.bin", &sz);
	assert(sharedPal);
	dmaCopy(sharedPal, BG_PALETTE_SUB, sz);
	free(sharedPal);
	initConsole();

	/* Initialize the filebrowser icons */
	void* pal 	= bufferFile("icon.pal.bin",NULL);
	void* gfx	= bufferFile("icon.img.bin",NULL);
	assert(pal && gfx);
	loadExtPalette(0, pal, SUB_SCREEN);
	iconFrames[FILEBROWSER_FRAMES] = loadFrame(gfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 0, SUB_SCREEN);
	iconFrames[FILEBROWSER_FRAMES+1] = loadFrame(gfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 1, SUB_SCREEN);
	int i;
	for(i =0; i<(MAX_ENTRIES); i++) {
		initSprite(i, 0, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[FILEBROWSER_FRAMES]),SpriteSize_32x32 ,SpriteColorFormat_256Color,SUB_SCREEN);
		setSprXY(i, 0, i*FB_ICONSZ, SUB_SCREEN);
	}
	free(pal);
	free(gfx);
	/* Initialize the playlist icon */
	pal = bufferFile("playlst.pal.bin",NULL);
	gfx = bufferFile("playlst.img.bin",NULL);
	assert(pal && gfx);
	loadPalette(0, pal, true, SUB_SCREEN);
	for(i=0; i<NUM_PLAYLIST_STATES; i++) {
		iconFrames[PLAYLIST_FRAMES+i] = loadFrame(gfx,  SpriteColorFormat_16Color, SpriteSize_16x16 , i, SUB_SCREEN);
	}
	free(gfx);
	free(pal);
	initSprite(MAX_ENTRIES, 0, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[PLAYLIST_FRAMES]), SpriteSize_16x16, SpriteColorFormat_16Color, SUB_SCREEN);
	setSprXY(MAX_ENTRIES, SCREEN_WIDTH-PL_ICONSZ, 0, SUB_SCREEN);
	setSpriteVisiblity(true, MAX_ENTRIES, SUB_SCREEN);
	/* Initialize the progress bar */
	initPrgrBar();
}

void setGuiState(GUI_STATE stat)
{
	if(state!= stat) {
		state = stat;
		switch(state) {
		case GUI_BROWSING:
			setSpriteVisiblity(true, MAX_ENTRIES, SUB_SCREEN);
			glFlush(0);
			bgSetScroll(prgrBar, 0, 0);
			bgHide(prgrBar);
			break;
		case GUI_STREAMING:
			clearConsole();
			setConsoleCooAbs(0,0);

			int i;
			for(i=0; i<MAX_ENTRIES; i++) {
				setSpriteVisiblity(true, i, SUB_SCREEN);
			}
			bgShow(prgrBar);
			setSpriteVisiblity(false, MAX_ENTRIES, SUB_SCREEN);
			break;
		}
	}
}

void updateGui(void)
{
	updateInput();
	updateVideo();

	int inSleepMode = keysHold & KEY_LID;

	/* Exit program */
	if(keysPres & KEY_START) {
		deinitFeOSMusic();
		exit(0);
	}
	switch(state) {
	case GUI_STREAMING:
		switch(getStreamState()) {
		case STREAM_WAIT:
		case STREAM_PLAY:
			if (!inSleepMode){
				visualizePlayingSMP();
				updatePrgrBar();
			}
			
			if(updateStream()< 0) {
				stopStream();
				break;
			}
			if(keysPres & KEY_A) {
				pauseStream();
				break;
			}
			if(keysPres & KEY_RIGHT)
				selectTrack(1);
			else if(keysPres & KEY_LEFT)
				selectTrack(-1);
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
			if(keysPres & KEY_A)
				resumeStream();
			break;
		}
		updatePlayList();
		break;
	case GUI_BROWSING:
		if (!inSleepMode) {
			drawList();
			updateBrowser();
		}
		break;
	}
}

void deinitGui(void)
{
	deinitVideo();
}
