#ifndef PLAYLIST_H
#define PLAYLIST_H

extern int markersX[2];
extern int dragging;
enum PLAYLST_STATE {
	SINGLE = 0,
	REPEAT,
	SHUFFLE,
	REPEAT_DIR,
	MARKER,
	NUM_PLAYLIST_STATES,
};
int playFile(const char* name);
void printInfo(void);
void updatePlayList(void);
int getPlayLstState(void);
void setPlayLstState(int stat);
void selectTrack(int var);

#endif
