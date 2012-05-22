#include "FeOSMusic.h"

hword_t *consoleMap;
unsigned int row, col;
int consoleId;

void init3D(void)
{
	videoSetMode(MODE_0_3D);
	glInit();
	glEnable( GL_TEXTURE_2D | GL_ANTIALIAS );
	glClearColor( 0, 0, 0, 31 ); 	// BG must be opaque for AA to work
	glClearPolyID( 63 ); 			// BG must have a unique polygon ID for AA to work
	glClearDepth( GL_MAX_DEPTH );
	glViewport(0,0,255,191);		// set the viewport to screensize
	glColor( 0x7FFF ); 				// max color
	glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE );  // geen dingen laten verdwijnen
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();				// reset view
	glOrthof32( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1 << 12, 1 << 12 );
	gluLookAt(	0.0, 0.0, 1.0,		//camera possition
	            0.0, 0.0, 0.0,		//look at
	            0.0, 1.0, 0.0);		//up
}

void deinit3D(void)
{
	glDeinit();
}
void initVideo(void)
{
	u16 * iconGfx;

	/* We need access to DS hardware */
	FeOS_DirectMode();
	videoSetModeSub(MODE_0_2D);
	initConsole();
	vramSetBankD(VRAM_D_SUB_SPRITE);
	oamEnable(states(SUB_SCREEN));
	oamInit(states(SUB_SCREEN), SpriteMapping_1D_128, true);

	/* Load sprites */
	void * pal = bufferFile("icon.pal.bin");
	iconGfx = bufferFile("icon.img.bin");
	if(pal && iconGfx) {
		loadExtPalette(0, pal, SUB_SCREEN);
		iconFrames[0] = loadFrame(iconGfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 0, SUB_SCREEN);
		iconFrames[1] = loadFrame(iconGfx,  SpriteColorFormat_256Color, SpriteSize_32x32 , 1, SUB_SCREEN);
		int i;
		for(i =0; i<(ENTS_AL+1); i++) {
			initSprite(i, 0, oamGfxPtrToOffset(states(SUB_SCREEN), iconFrames[0]),SpriteSize_32x32 ,SpriteColorFormat_256Color,SUB_SCREEN);
			setSprXY(i, 0, i*ICON_SZ, SUB_SCREEN);
		}
		free(pal);
		free(iconGfx);
	} else {
		FeOS_ConsoleMode();
		abort();
	}

	init3D();
}

void initConsole(void)
{
	u16* consoleGfx = bufferFile("font.img.bin");
	u16* consolePal = bufferFile("font.pal.bin");

	vramSetBankC(VRAM_C_SUB_BG);
	consoleId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 20,0);

	dmaCopy(consolePal, BG_PALETTE_SUB, 16*2);
	dmaCopy(consoleGfx, bgGetGfxPtr(consoleId), 16348*2);
	col = row = 0;
	consoleMap = bgGetMapPtr(consoleId);
	free(consolePal);
	free(consoleGfx);
}

void setConsoleCoo(int x, int y)
{
	col = x;
	row = y;
}

void setConsoleCooAbs(int x, int y){
	bgSetScroll(consoleId, x, y);
}
void putChar(char kar)
{
	if(col >= 0 && col <= 31) {
		if(row >= 0 && row<=31) {
			row += col >> 5;
			col &= 31;
			row &= 31;
			consoleMap[col+(row*32)] = kar;
			col++;
		}
	}
}

void print(const char * string, int limit)
{

	int i;
	if(limit < 0 || limit >strlen(string)) {
		limit = strlen(string);
	}
	limit+=col;
	for(i = 0; col<limit && i<strlen(string); i++) {
		int k = string[i];
		switch(k) {
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

void clearConsole(void)
{
	dmaFillHalfWords(0, consoleMap, 32*32*2);
	row = col = 0;
}

void updateVideo(void)
{
	oamUpdate(states(SUB_SCREEN));
	bgUpdate();
}

void deinitVideo(void)
{
	deinit3D();
	
}

void glVertex2v16(int x, int y)
{
	GFX_VERTEX16 = ((x & 0xffff) | (y<<16));
}

void drawLine(int x, int y, int x2, int y2)
{
	glVertex3v16(x,y,0);
	glVertex2v16(x2,y2);
	glVertex2v16(x2,y2);
}
