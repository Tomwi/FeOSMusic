#include "FeOSMusic.h"

#define FILTER_PATH ("/data/FeOSMusic/filters/")
const char msg[] = "No filter selected";

char* nameBlock;
char** nameList;
int blockSize;
int numFilters;
int selected = -1;
FILTER filter;

void loadFilters(void)
{
	DIR *pdir;
	struct dirent *pent;
	if((pdir=opendir(FILTER_PATH))) {
		chdir(FILTER_PATH);
		while ((pent=readdir(pdir))!=NULL) {
			if(strstr(pent->d_name, ".fx2")) {
				void* tmp = realloc(nameBlock, blockSize+strlen(pent->d_name)+1);
				if(tmp) {
					nameBlock = tmp;
					char* dst = &nameBlock[blockSize];
					strcpy(dst, pent->d_name);
					blockSize+=strlen(pent->d_name)+1;
					numFilters++;
					tmp = realloc(nameList, sizeof(char*)*numFilters);
					if(tmp) {
						nameList = tmp;
						nameList[numFilters-1] = dst;
						printf("%s\n", nameList[0]);
					}
				}
			}
		}
	}
	closedir(pdir);
}

void unloadFilters(void)
{
	free(nameBlock);
	free(nameList);
	/* Don't leak filters */
	if(selected >= 0)
			unloadFilter(&filter);
	numFilters = 0;
}

void updateFilters(void)
{
	
	if(keysPres & KEY_SELECT) {
		int prev = selected;
		selected++;
		CYCLE(selected, -1, (numFilters-1));
		/* unload filter if it became useless */
		if(prev!=selected){
			if(prev>=0)
				unloadFilter(&filter);
		}
			
		/* Load a new filter */
		if(selected>=0 && selected!=prev) {
			chdir(FILTER_PATH);
			if(!loadFilter(&filter, nameList[selected]))
				selected = -1;
			setListedDir();
		}
	}
	setConsoleCoo(0, 23);
	char* string = (char*)msg;
	if(selected>=0)
		string = nameList[selected];
	consoleClearLine(23);
	print("%s\n", string);
}
