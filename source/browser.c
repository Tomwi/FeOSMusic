#include "FeOSMusic.h"

char** list;
int numEnt, lastDir;
int scrollY;
int drgY[2];
int drgTime;
int kinEn;
char cwd[FILENAME_MAX];

static int scrolly = 0, beginY = 0, begin = 0;

#define SCHUIF (2)
#define MAX_EN (10<<SCHUIF)

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
		chdir(path);
		getcwd(cwd, FILENAME_MAX);
		/* Free old directory BEFORE we set numEnt to 0 */
		freeDir();
		numEnt = 0;
		scrollY = 0;
		drgY[1] = drgY[0] = 0;
		struct dirent *pent;

		pdir=opendir(cwd);

		if (pdir) {
			chdir(cwd);
			while ((pent=readdir(pdir))!=NULL) {

				if(strcmp(".", pent->d_name) == 0)

					continue;
				if(pent->d_type==DT_REG) {
					if(isPlayable(pent->d_name)<0)
						continue;
				}
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
		int i;
		for(i=0; i<numEnt; i++) {
			if(list[i][ENTRY_TYPE] == DT_REG)
				break;
			lastDir = i;
		}
	}
}

void setListedDir(void){
	chdir(cwd);
}

void updateIcons()
{
	int i;
	for(i=0; i<7; i++) {
		if((i+begin) < numEnt) {
			setSprXY(i, 0, (beginY+ICON_SZ*i), SUB_SCREEN);
			if(list[begin + i][ENTRY_TYPE]==DT_DIR) {
				setFrame(iconFrames[0], false, i, SUB_SCREEN);
			} else {
				setFrame(iconFrames[1], false, i, SUB_SCREEN);
			}

		} else {
			setSpriteVisiblity(true, i, SUB_SCREEN);
		}
	}
}

void drawList()
{
	clearConsole();
	if(numEnt) {
		int i, j=((scrolly % ICON_SZ) ? 7 : 6);
		CLAMP(j, 0, numEnt);

		for(i=0; i<j; i++) {
			setConsoleCoo((ICON_SZ/8), i * (ICON_SZ/8) + 2);
			if(list) {
				print("%.*s", 32-(ICON_SZ/8), &list[(begin+i)][ENTRY_NAME]);
			}
		}
	} else {
		print("EMPTY\n");
	}
}

void updateBrowser(void)
{
	if(keysPres & KEY_TOUCH) {
		drgY[0] = stylus.y;
	}
	if(keysHold & KEY_TOUCH) {
		drgY[1] = stylus.y;
		drgTime++;
	}

	if(keysReleased & KEY_TOUCH) {
		if(drgTime < 30 && abs(drgY[1]-drgY[0])<3) {
			if(!(TouchedArea(256-FL_ICONSZ, 0, 256, FL_ICONSZ))){
				int selected = (scrollY + drgY[0])/ICON_SZ;
				CLAMP(selected, 0, numEnt);
				if(list[selected][ENTRY_TYPE]==DT_DIR)
					retrieveDir(&list[selected][ENTRY_NAME]);
				else {
					if(getStreamState() == STREAM_STOP) {
						char * file = &list[selected][ENTRY_NAME];
						if(playFile(file))
							return;
					}
				}
			}
		}
		scrollY+=(drgY[0]-drgY[1]);
		CLAMP(scrollY, 0, ((numEnt+1)<=(192/ICON_SZ))? 0 :  ((numEnt * ICON_SZ)-192));
		kinEn = ((drgY[0]-drgY[1])/(drgTime))<<SCHUIF;
		if(abs(kinEn) > MAX_EN ) {
			if(kinEn >0)
				kinEn = MAX_EN;
			else
				kinEn = -MAX_EN;
		}
		drgY[1] = drgY[0] = 0;
		drgTime = 0;
	}
	scrollY += (kinEn>>SCHUIF);
	if(kinEn > 0)
		kinEn--;
	else if(kinEn < 0)
		kinEn++;
	// Update scrolling variables used by drawList() in the next frame
	CLAMP(scrollY, 0, ((numEnt+1)<=(192/ICON_SZ))? 0 :  ((numEnt * ICON_SZ)-192));
	scrolly = (scrollY + (drgY[0]-drgY[1]));
	CLAMP(scrolly, 0, ((numEnt+1)<=(192/ICON_SZ))? 0 :  ((numEnt * ICON_SZ)-192));
	beginY = -(scrolly%ICON_SZ);
	begin = (scrolly/ICON_SZ);
	CLAMP(begin, 0, numEnt);
	setConsoleCooAbs(0, -beginY);
	updateIcons();
}
