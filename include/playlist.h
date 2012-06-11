#ifndef PLAYLIST_H
#define PLAYLIST_H

enum PLAYLST_STATE {
	SINGLE = 0,
	REPEAT,
	SHUFFLE,
};

int playFile(const char* name);
void updatePlayList(void);
int getPlayLstState(void);
void setPlayLstState(int stat);

#endif