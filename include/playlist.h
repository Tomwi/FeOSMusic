#ifndef PLAYLIST_H
#define PLAYLIST_H

enum PLAYLST_STATE {
	SINGLE = 0,
	REPEAT,
	REPEAT_DIR,
	SHUFFLE,
	NUM_PLAYLIST_STATES,
};
int playFile(const char* name);
void printInfo(void);
void updatePlayList(void);
int getPlayLstState(void);
void setPlayLstState(int stat);
void selectTrack(int var);

#endif
