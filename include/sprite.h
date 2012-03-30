#ifndef T_SPRITE_H
#define T_SPRITE_H

#include <FeosMusic.h>
#define states(n) (n>0? FeOS_GetMainOAM():FeOS_GetSubOAM())

/* Straight from libnds */
#define SPRITE_SIZE_SHAPE(size) (((size) >> 12) & 0x3)
#define SPRITE_SIZE_SIZE(size)  (((size) >> 14) & 0x3)
#define SPRITE_SIZE_PIXELS(size) (((size) & 0xFFF) << 5)

extern hword_t * spritePalettes[2];

typedef struct {
	/* ATTR0 */
	struct{
		hword_t y				:8;
		union{
			struct{
				u8				:1;
				bool disabled	:1;
				u8 mode			:2;
				bool mosaic		:1;
				bool color		:1;	// 16 if not set
				u8	 shape 		:2;	
			};
		};
	};
	/* ATTR1 */
	struct{
		hword_t x				:9;
		u8						:3;
		bool hFlip				:1;
		bool vFlip				:1;
		u8	size				:2;
	};
	/* ATTR2 */
	struct{
		hword_t name 			:10;
		u8 prio 				:2;
		u8 pal 					:4;
	};
	struct{
		hword_t fill			:6;
	};
}SPRITE_ENTRY;

typedef struct {
    u8 * tileaddr;
    hword_t counter;
    hword_t size;
    u8 curFrame, animating;
    u8 start, stop, time;
} SPRITE_INFO;

hword_t * loadFrame(hword_t * gfx, int color, int size, int index, bool screen);
bool loadPalette(u8 no, void * data, bool small, bool screen);
bool loadExtPalette(u8 no, void * data, bool screen);
void initSprite(u8 no, u8 pal, hword_t name, int size, int format,  bool screen);
void setSprXY(int no, hword_t x, hword_t y, bool screen);
void setFrame(hword_t * ptr, int no, int screen);
void hideSprite(int no, int screen);
#endif



