#include "FeOSMusic.h"

#define ICONX (256-16)
#define ICONY (0)
char prev[1024];
static int state = SINGLE;
int track;

u16* playStateFrms[NUM_PLAYLIST_STATES];

static void onEof(void* context)
{
	if(getPlayLstState()==REPEAT) {
		cur_codec.seek(0);
	}
}

int playFile(const char* name)
{
	if(provideCodec(name)) {
		if(state==REPEAT)
			audioCallbacks.onEof = onEof;
		else
			audioCallbacks.onEof = NULL;
		strcpy(prev, name);
		startStream(name, streamIdx);
		return 1;
	}
	return 0;
}

void printInfo(void)
{
	clearConsole();
	AUDIO_INFO* inf = getStreamInfo(streamIdx);
	print("Sample rate  : %d\n", inf->frequency);
	print("Channel count: %d\n", inf->channelCount);
	print("Current track: %d\n", track);
}

void shuffle(void)
{
	if(strlen(prev)>0) {
		int val, files = (numEnt-lastDir)-1;
		char* file = prev;
		if(files>=1) {
			if(files >= 2) {
select:
				val = rand() % files;
				if(!strcmp(prev, &list[val+lastDir+1][ENTRY_NAME])) {
					goto select;
				}
				strcpy(prev, &list[val+lastDir+1][ENTRY_NAME]);
			}
			playFile(file);
		}
	}
}

void updatePlayList(void)
{
	if(keysPres & KEY_TOUCH) {
		if(TouchinArea(ICONX, ICONY, ICONX+16, ICONY+16)) {
			int stat = (state+1)%NUM_PLAYLIST_STATES;
			setPlayLstState(stat);
		}
	}
	if(getStreamState() == STREAM_STOP)
		track = 0;
	switch(state) {
	case SINGLE:
		return;
	case REPEAT:
		return; // handled automatically by onEof callback
	case SHUFFLE:
		if(getStreamState() == STREAM_STOP) {
			shuffle();
		}
		return;
	}
}

int getPlayLstState(void)
{
	return state;
}

void setPlayLstState(int stat)
{
	state = stat;
	setFrame(iconFrames[PLAYLIST_FRAMES+stat], false, MAX_ENTRIES, SUB_SCREEN);
	if(state==REPEAT)
		audioCallbacks.onEof = onEof;
	else
		audioCallbacks.onEof = NULL;
}

void selectTrack(int var)
{
	/* Pretty useless to select another track when there's only one
	 * or when selecting is unsupported.
	 */
	if(cur_codec.getTrackCount && cur_codec.setTrack) {
		printInfo();
		track+= var;
		CYCLE(track, 0, cur_codec.getTrackCount());
		cur_codec.setTrack(track);
	}
}
