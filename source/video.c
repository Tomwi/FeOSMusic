#include "FeOSMusic.h"
#include "fix_fft.h"

#define FFT_SAMP (9)
#define NUM_FREQS (16)
#define SEPERATION ((256/NUM_FREQS))
#define PRECISION (16)

hword_t *consoleMap;
unsigned int row, col;

int consoleId;
int visualizer = NORMAL;

s16 FFT[(1<<FFT_SAMP)];
int frequencies[NUM_FREQS];
int curfreqs[NUM_FREQS];
int oldfreqs[NUM_FREQS];
int core;

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
	/* We need access to DS hardware */
	FeOS_DirectMode();
	vramSetPrimaryBanks (VRAM_A_LCD, VRAM_B_LCD, VRAM_C_SUB_BG_0x06200000, VRAM_D_LCD);
	/* Init video engine for the SUB_SCREEN */
	videoSetModeSub(MODE_5_2D);
	vramSetBankI(VRAM_I_SUB_SPRITE);
	oamEnable(states(SUB_SCREEN));
	oamInit(states(SUB_SCREEN), SpriteMapping_1D_128, false);
	/* Init video engine for the MAIN_SCREEN */
	init3D();
}

void initConsole(void)
{
	int sz = 0;
	u16* consoleGfx = bufferFile("font.img.bin", &sz);
	if(consoleGfx) {
		consoleId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 2,0);
		dmaCopy(consoleGfx, bgGetGfxPtr(consoleId), sz);
		col = row = 0;
		consoleMap = bgGetMapPtr(consoleId);
		free(consoleGfx);
	}
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
void consoleClearLine(int y){
	int i;
	hword_t* dst = &consoleMap[(y%32)*32];
	for(i=0; i<32; i++){
		*dst++ = 0;
	}
}
void putChar(char kar)
{
	if(col >= 0 && col <= 32) {
		if(row >= 0 && row<=31) {
			switch(kar) {
			case '\n':
				col = 0;
				row++;
				row &= 31;
				break;
			default:
				row += col >> 5;
				row &= 31;
				col &= 31;
				consoleMap[col+(row*32)] = kar;
				col++;
			}
		}
	}
}

void print(const char *fmt, ...)
{
	static char buffer[MAX_STRLEN];
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	vsnprintf(buffer, MAX_STRLEN, fmt, arg_ptr);
	va_end(arg_ptr);
	int i;
	for(i=0; i<strlen(buffer); i++)
		putChar(buffer[i]);
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

/* TODO:
 * add 8bit support
 */
void visualizePlayingSMP(void)
{
	int off = getPlayingSample();
	short * buffer = &((short*)(getoutBuf()))[off];
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
			if(buffer >= (((short*)getoutBuf()) + STREAM_BUF_SIZE))
				buffer -= STREAM_BUF_SIZE;
		}
		glColor3b(255,255,255);
		glFlush(0);
	}
	/* if it doesn't do what you expect well duh... it's b0rked!*/
	else if(visualizer == BORKUALIZER) {

		if(!(core & 3)) {
			int i;
			memcpy(oldfreqs, frequencies, sizeof(int)*NUM_FREQS);
			memcpy(curfreqs, frequencies, sizeof(int)*NUM_FREQS);
			memset(frequencies, 0, sizeof(int)*NUM_FREQS);
			if(getStreamInfo(streamIdx)->channelCount == 1)
				memcpy(FFT, buffer, (1<<FFT_SAMP)*2);
			else {
				s16* out = FFT;
				for(i=0; i<(1<<FFT_SAMP); i++, buffer++) {
					if(buffer >= (((short*)getoutBuf()) + STREAM_BUF_SIZE))
						buffer -= STREAM_BUF_SIZE;
					*out++ = ((*buffer + *(buffer +STREAM_BUF_SIZE))>>1);
				}
			}
			fix_fftr(FFT, FFT_SAMP, 0);
			for(i=0; i<(1<<FFT_SAMP); i++) {
				int ret;
				if((ret = binLog(abs(FFT[i])))>=0) {
					frequencies[ret] += (1<<PRECISION);
				}
			}
		}
		int i;
		glBindTexture( 0, 0 );
		for(i=0; i<(NUM_FREQS); i++) {
			glBegin(GL_QUAD);
			glColor3b(0,63,128);
			glVertex3v16((i*SEPERATION)+(SEPERATION), 192,0);
			glColor3b(curfreqs[i]>>PRECISION, 0, 128);
			glVertex3v16((i*SEPERATION)+(SEPERATION), 192-(curfreqs[i]>>PRECISION), 0);
			glVertex3v16(i*SEPERATION, 192-(curfreqs[i]>>PRECISION), 0);
			glColor3b(0,0,128);
			glVertex3v16(i*SEPERATION, 192, 0);
			//glEnd();
			curfreqs[i] += (frequencies[i] - oldfreqs[i])>>2;
		}
		glFlush(0);
		core++;
	}
}
