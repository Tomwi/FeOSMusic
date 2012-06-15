#ifndef GUI_H
#define GUI_H

enum SUB_ICONS {
	FILEBROWSER_ICON 	= 0,
	PLAYLIST_ICON		= MAX_ENTRIES,
};

enum SUB_ICONSZ {
	FB_ICONSZ			= 32,
	PL_ICONSZ			= 16,
};

enum SUB_FRAMES {
	FILEBROWSER_FRAMES  = 0,
	PLAYLIST_FRAMES 	= 2,
	NUM_FRAMES			= 5,
};

typedef enum {
	GUI_STREAMING = 0,
	GUI_BROWSING = 1,
}GUI_STATE;

extern u16* iconFrames[NUM_FRAMES];
extern int prgrBar;

void initGui(void);
void setGuiState(GUI_STATE stat);
void updateGui(void);
void deinitGui(void);

void initPrgrBar(void);
void updatePrgrBar(void);

#endif