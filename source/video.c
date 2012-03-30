#include "video.h"
#include "sprite.h"


u16 * iconFrames[2];
u16 * iconGfx;
char * gfxPath = "/data/FeOSMusic/";
int consoleId;
hword_t *consoleMap;
unsigned int row, col;


void initVideo(void)
{
	/* We need access to DS hardware */
	FeOS_DirectMode();
	videoSetModeSub(MODE_0_2D);
	initConsole();
	vramSetBankD(VRAM_D_SUB_SPRITE);
	oamEnable(states(SUB_SCREEN));
	oamInit(states(SUB_SCREEN), SpriteMapping_1D_128, true);

	/* Load sprites */
	chdir(gfxPath);
	void * pal = bufferFile("icon.pal.bin");
	iconGfx = bufferFile("icon.img.bin");
	if(pal && iconGfx) {
		loadExtPalette(0, pal, SUB_SCREEN);
		iconFrames[0] = loadFrame(iconGfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 0, SUB_SCREEN);
		iconFrames[1] = loadFrame(iconGfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 1, SUB_SCREEN);
		int i;
		for(i =0; i<6; i++) {
			initSprite(i, 0, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[0]),SpriteSize_32x32 ,SpriteColorFormat_256Color,SUB_SCREEN);
			setSprXY(i, 0, i*32, SUB_SCREEN);
		}
	} else {

	}
}

void initConsole(void){
	// Try to load custom console font
	chdir(gfxPath);
	u16* consoleGfx = bufferFile("font.img.bin");
	u16* consolePal = bufferFile("font.pal.bin");
	
	vramSetBankC(VRAM_C_SUB_BG);
	consoleId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 20,0);
	
	dmaCopy(consolePal, BG_PALETTE_SUB, 16*2);
	dmaCopy(consoleGfx, bgGetGfxPtr(consoleId), 16348*2);
	col = row = 0;
	consoleMap = bgGetMapPtr(consoleId);
}

void setConsoleCoo(int x, int y){
	col = x;
	row = y;
}

void putChar(char kar){
	
	row += col >> 5;
	if(row > 23){
		row = 0;
	}
	col &= 31;
	consoleMap[col+(row*32)] = kar;
	col++;
}

void print(char * string, int limit){
	
	int i;
	if(limit < 0 || limit >strlen(string)){
		limit = strlen(string);
	}
	limit+=col;
	for(i = 0; col<limit && i<strlen(string); i++){
		int k = string[i];
		switch(k){
			case '\n':
			row++;
			col = 0;
			break;
			default:
			putChar(k);
			break;
		}
	}
}

void clearConsole(void){
	dmaFillHalfWords(0, consoleMap, 32*32*2);
}

void updateVideo(void){
}