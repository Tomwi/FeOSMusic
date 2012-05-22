#include "FeOSMusic.h"

#define ENTRY_TYPE (0)
#define ENTRY_NAME (1)

char ** list;
int numEnt;
int scrollY;
int drgY[2];
int drgTime;
int kinEn;
char cwd[FILENAME_MAX];

static int scrolly = 0, beginY = 0, begin = 0;

const char * Codecs [][2]= {
	{".ogg", "ogg"},
	{".m4a", "aac"},
	{".aac", "aac"},
	{".mp3", "mp3"},
	{".flac", "flac"},
};

#define NUM_EXT (sizeof(Codecs)/sizeof(Codecs[0]))

CODEC_INTERFACE cur_codec;
int loadedCodec = -1;

u16 * iconFrames[2] = {
	NULL,
	NULL,
};

bool isRoot(char * path)
{
	int i;
	int counter = 0;
	for(i=0; i<strlen(path); i++) {
		if(path[i]=='/')
			counter++;
		if(counter == 2)
			return false;
	}
	return true;
}
int compare(const void * a, const void * b)
{
	char * c = *((char**)a);
	char * d = *((char**)b);
	// Directory goes before file
	if(c[ENTRY_TYPE] !=d[ENTRY_TYPE])
		if(c[ENTRY_TYPE] == DT_DIR)
			return -1;
		else
			return 1;
	else {
		return stricmp(&c[ENTRY_NAME], &d[ENTRY_NAME]);
	}
	return 0;
}

void freeDir(void)
{
	int i;
	for(i=0; i<numEnt; i++) {
		free(list[i]);
	}
	free(list);
	list = NULL;
}

void retrieveDir(char * path)
{
	DIR *pdir;
	if(path) {
		getcwd(cwd, FILENAME_MAX);
		if(strcmp(path, "..")) {
			strcat(cwd, path);
		} else {
			int i;
			if(!isRoot(cwd)) {
				for(i=strlen(cwd)-2; i>1; i--) {
					if(cwd[i] == '/')
						break;
					cwd[i] = 0;
				}

			} else
				return;
		}
		numEnt = 0;
		scrollY = 0;
		struct dirent *pent;

		pdir=opendir(cwd);

		if (pdir) {
			chdir(cwd);
			freeDir();
			while ((pent=readdir(pdir))!=NULL) {

				if(strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0)
					continue;

				void * temp = realloc(list, sizeof(char**)*(numEnt+1));
				if(temp) {
					list = temp;
					int toAlloc = ((strlen(pent->d_name) + sizeof(char)*2));
					list[numEnt] = malloc(toAlloc);
					if(list[numEnt]) {
						list[numEnt][ENTRY_TYPE] = pent->d_type;
						strcpy(&list[numEnt][ENTRY_NAME], pent->d_name);
						numEnt++;
					} else {
						freeDir();
						closedir(pdir);
						return;
					}
				}
			}

		} else {
			closedir(pdir);
			return;
		}
		if(numEnt == 0) {
			free(list);
			list = NULL;
		}
		qsort(list, numEnt, sizeof(char*), compare);
		closedir(pdir);
	}
}

void updateIcons()
{
	int i;

	for(i=0; i<7; i++, beginY+=ICON_SZ) {
		if((i+begin) < numEnt) {
			setSprXY(i, 0, beginY, SUB_SCREEN);
			if(list[begin + i][ENTRY_TYPE]==DT_DIR) {
				setFrame(iconFrames[0], i, SUB_SCREEN);
			} else {
				setFrame(iconFrames[1], i, SUB_SCREEN);
			}

		} else {
			hideSprite(i, SUB_SCREEN);
		}
	}
}

void drawList()
{
	clearConsole();
	if(numEnt) {
		int i, j=((scrolly % ICON_SZ) ? 7 : 6);
		CLAMP(j, 0, numEnt);

		for(i=0; i<j; i++, beginY+=ICON_SZ) {

			setConsoleCoo((ICON_SZ/8), i * (ICON_SZ/8) + 2);
			if(list) {
				print(&list[(begin+i)][ENTRY_NAME], 32-(ICON_SZ/8));
			}
		}

	} else {
		print("EMPTY\n",-1);
	}

}

void updateBrowser(void)
{
	if(keysPres & KEY_TOUCH) {
		drgY[0] = stylus.y;
	}
	if(keysPres & KEY_B) {
		retrieveDir("..");
	}
	if(keysHold & KEY_TOUCH) {
		drgY[1] = stylus.y;
		drgTime++;
	}

	if(keysReleased & KEY_TOUCH) {
		if(drgTime < 30 && drgY[1] == drgY[0]) {
			int selected = (scrollY + drgY[1])/ICON_SZ;
			if(list[selected][ENTRY_TYPE]==DT_DIR)
				retrieveDir(&list[selected][ENTRY_NAME]);
			else {
				if(mixer_status == STATUS_STOP) {
					char * file = &list[selected][ENTRY_NAME];
					int i;
					for(i =0; i<NUM_EXT; i++) {
						if(strstr(file, Codecs[i][0])) {
							if(loadedCodec != -1 && strcmp(Codecs[loadedCodec][1],(Codecs[i][1]))) {
								unloadCodec(&cur_codec);
							}
							if(i != loadedCodec) {
								if(!loadCodec((Codecs[i][1]), &cur_codec))
									return;
								loadedCodec = i;
							}

							startStream(&cur_codec, (const char*)(Codecs[i][1]), file);
							mixer_status = STATUS_PLAY;
							return;
						}
					}
				}
			}
		}
		scrollY+=(drgY[0]-drgY[1]);
		kinEn = ((drgY[0]-drgY[1])/(drgTime));
		drgY[1] = drgY[0] = 0;
		drgTime = 0;
	}

	// Update scrolling variables used by drawList() in the next frame
	CLAMP(scrollY, 0, (numEnt < (192 / ICON_SZ-192)? numEnt * ICON_SZ : (numEnt*ICON_SZ-192)));
	scrolly = (scrollY + (drgY[0]-drgY[1])) + kinEn;
	kinEn-= (kinEn>>31);
	CLAMP(kinEn, 0, 64);
	CLAMP(scrolly, 0, (numEnt < (192 / ICON_SZ-192)? numEnt * ICON_SZ : (numEnt*ICON_SZ-192)));
	beginY = -(scrolly%ICON_SZ);
	begin = (scrolly/ICON_SZ);
	CLAMP(begin, 0, numEnt);
	setConsoleCooAbs(0, -beginY);
	updateIcons();
}
