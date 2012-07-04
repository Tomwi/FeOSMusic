#include "FeOSMusic.h"

#define PRGRBAR_Y (SCREEN_HEIGHT/(8*2) - 1)

static int state = GUI_BROWSING;
u16* iconFrames[NUM_FRAMES];
int prgrBar, prgr, sbitmap;

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
			unsigned int seek = ((unsigned int)(cur_codec.getResolution())*prgr)>>8;
			cur_codec.seek(seek);
			prgr = 0;
		}
	} else {
		u64 pos = ((unsigned int)(cur_codec.getPosition())<<8)/((unsigned int)(cur_codec.getResolution()));
		bgSetScroll(prgrBar, -pos, 0);
	}
}

void loadIcon(char* tls, char* pl, int palNo, int frameIdx, int numFrames, int size){
	int i;
	
	if(tls){
		void* gfx	= bufferFile(tls, NULL);
		if(!gfx)
			return;
		if(pl){
			void* pal 	= bufferFile(pl, NULL);
			if(pal){
				loadPalette(palNo, pal, true, SUB_SCREEN);
				free(pal);
			}
		}
	
		for(i=0; i<numFrames; i++){
			iconFrames[frameIdx+i] = loadFrame(gfx,  SpriteColorFormat_16Color, size, i, SUB_SCREEN);
		}
	free(gfx);
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
	loadIcon("icon.img.bin", "icon.pal.bin", 0, FILEBROWSER_FRAMES, PLAYLIST_FRAMES, SpriteSize_32x32);
	int i;
	for(i =0; i<(MAX_ENTRIES); i++) {
		initSprite(i, 0, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[FILEBROWSER_FRAMES]),SpriteSize_32x32 ,SpriteColorFormat_16Color,SUB_SCREEN);
		setSprXY(i, 0, i*FB_ICONSZ, SUB_SCREEN);
	}

	loadIcon("filtericon.img.bin", "filtericon.pal.bin", 1, FILTER_FRAMES, NUM_FRAMES-FILTER_FRAMES, SpriteSize_16x16);
	initSprite(FILTER_ICON, 1, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[FILTER_FRAMES]), SpriteSize_16x16, SpriteColorFormat_16Color, SUB_SCREEN);
	setSprXY(FILTER_ICON, SCREEN_WIDTH-FL_ICONSZ, 0, SUB_SCREEN);

	/* Initialize the playlist icon */
	loadIcon("playlst.img.bin", "playlst.pal.bin", 2, PLAYLIST_FRAMES, FILTER_FRAMES-PLAYLIST_FRAMES, SpriteSize_16x16);
	initSprite(PLAYLIST_ICON, 2, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[PLAYLIST_FRAMES]), SpriteSize_16x16, SpriteColorFormat_16Color, SUB_SCREEN);
	setSprXY(PLAYLIST_ICON, SCREEN_WIDTH-PL_ICONSZ, 0, SUB_SCREEN);
	setSpriteVisiblity(true, PLAYLIST_ICON, SUB_SCREEN);
	 
	/* Initialize the progress bar */
	initPrgrBar();
	
	u16* sbmpGfx = bufferFile("sbitmap.img.bin", &sz);
	sbitmap = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 2,2);
	dmaCopy(sbmpGfx, bgGetGfxPtr(sbitmap), 192*256*2);
	free(sbmpGfx);
}

void setGuiState(GUI_STATE stat)
{
	if(state!= stat) {
		state = stat;
		switch(state) {
		case GUI_BROWSING:
			setSpriteVisiblity(true, PLAYLIST_ICON, SUB_SCREEN);
			setSpriteVisiblity(false, FILTER_ICON, SUB_SCREEN);
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
			setSpriteVisiblity(false, PLAYLIST_ICON, SUB_SCREEN);
			setSpriteVisiblity(true, FILTER_ICON, SUB_SCREEN);
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
	updateFilters(state);
}

void deinitGui(void)
{
	deinitVideo();
}


