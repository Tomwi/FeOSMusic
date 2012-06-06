#include "FeOSMusic.h"
#include "fix_fft.h"

#define PRGRBAR_Y (SCREEN_HEIGHT/(8*2) - 1)
#define FFT_SAMP (8)
#define NUM_FREQS (16)



hword_t *consoleMap;
unsigned int row, col;
int consoleId, prgrBar, prgr;
int visualizer = NORMAL;

s16 FFT[(1<<FFT_SAMP)];
int frequencies[NUM_FREQS];

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
	int sz = 0;

	/* We need access to DS hardware */
	FeOS_DirectMode();
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	u16* sharedPal = bufferFile("shared.pal.bin", &sz);
	dmaCopy(sharedPal, BG_PALETTE_SUB, sz);
	free(sharedPal);
	initConsole();
	initPrgrBar();
	vramSetBankD(VRAM_D_SUB_SPRITE);
	oamEnable(states(SUB_SCREEN));
	oamInit(states(SUB_SCREEN), SpriteMapping_1D_128, true);

	/* Load sprites */
	void * pal = bufferFile("icon.pal.bin",NULL);
	iconGfx = bufferFile("icon.img.bin",NULL);
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
	int sz = 0;
	u16* consoleGfx = bufferFile("font.img.bin", &sz);

	consoleId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 2,0);

	dmaCopy(consoleGfx, bgGetGfxPtr(consoleId), sz);
	col = row = 0;
	consoleMap = bgGetMapPtr(consoleId);
	free(consoleGfx);
}

void hideConsole(void)
{
	bgHide(consoleId);
}

void showConsole(void)
{
	bgShow(consoleId);
}
void setConsoleCoo(int x, int y)
{
	col = x;
	row = y;
}

void setConsoleCooAbs(int x, int y)
{
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
	FeOS_ConsoleMode();
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

void visualizePlayingSMP(void)
{
	int off = getPlayingSample();
	short * buffer = &(getoutBuf()[off]);
	if(visualizer == NORMAL) {
		glBegin( GL_TRIANGLE_STRIP);
		glBindTexture( 0, 0 );
		int i, j = (cur_codec.getSampleRate()/60)/128;

		static int status = 0;
		static int clr[3] = { 31, 0, 0 };
		glColor(RGB15(clr[0], clr[1], clr[2]));

		int st_1 = (status + 1) % 3;
		clr[status] --;
		clr[st_1] ++;
		if (clr[status] == 0) status = st_1;

		for(i = 0; i<128; i++) {
			int val1 = (buffer[0]>>8);
			int val2 = (buffer[1]>>8);
			if(cur_codec.getnChannels()>1) {
				val1+=(buffer[STREAM_BUF_SIZE]>>8);
				val1>>=1;
				val2+=(buffer[STREAM_BUF_SIZE+1]>>8);
				val2>>=1;
			}
			drawLine(i*2, val1+96, i*2+2, val2+96);

			buffer+=j;
			if(buffer >= (getoutBuf() + STREAM_BUF_SIZE))
				buffer -= STREAM_BUF_SIZE;
		}
		glColor3b(255,255,255);
		glFlush(0);
	} else if(visualizer == BORKUALIZER) {
		if(getStreamInfo(streamIdx)->channelCount == 1)
			memcpy(FFT, buffer, (1<<FFT_SAMP)*2);
		else{
			int i;
			s16* out = FFT;
			for(i=0; i<(1<<FFT_SAMP); i++, buffer++){
				if(buffer >= (getoutBuf() + STREAM_BUF_SIZE))
					buffer -= STREAM_BUF_SIZE;
				*out++ = ((*buffer + *(buffer +STREAM_BUF_SIZE))>>1);
			}
		}
		fix_fftr(FFT, FFT_SAMP, 0);
		_visua(FFT, (1<<FFT_SAMP), frequencies);
		glBindTexture( 0, 0 );
		int i;
		//glBegin( GL_TRIANGLE_STRIP);
		for(i=0; i<(NUM_FREQS); i++) {
			glBegin(GL_QUAD);
			glColor3b(0,128,255);
			glVertex3v16(i*(256/(NUM_FREQS-1))+15, 191,0);
			glColor3b(frequencies[i]*2,128,255);
			glVertex3v16(i*(256/(NUM_FREQS-1))+15, 191-frequencies[i], 0);
			glVertex3v16(i*(256/(NUM_FREQS-1)), 191-frequencies[i], 0);
			glColor3b(0,128,255);
			glVertex3v16(i*(256/(NUM_FREQS-1)), 191, 0);
			glEnd();
			frequencies[i] = 0;
		}
		frequencies[NUM_FREQS-1] = 0;
		glFlush(0);
	}

}

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
