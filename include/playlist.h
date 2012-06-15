#ifndef PLAYLIST_H
#define PLAYLIST_H

enum PLAYLST_STATE {
	SINGLE = 0,
	REPEAT,
	SHUFFLE,
	NUM_PLAYLIST_STATES,
};
void initPlayLstIcon(void);
int playFile(const char* name);
void updatePlayList(void);
int getPlayLstState(void);
void setPlayLstState(int stat);

#endif