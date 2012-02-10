#include <feos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "browser.h"


const char * Codecs [5][2]= {
	{".ogg", "ogg"},
	{".m4a", "aac"},
	{".aac", "aac"},
	{".mp3", "mp3"},
	{".flac", "flac"},
};

#define NUM_EXT 5

typedef struct DIRENTRY {
	char * type_name; // type = char, name = string, concatenated
} DIRENTRY;

DIRENTRY *list;
unsigned int numEnt;
int cursor;
CODEC_INTERFACE cur_codec;
int loadedCodec;

int compare(const void * a, const void * b)
{
	DIRENTRY * c = (DIRENTRY*)a;
	DIRENTRY * d = (DIRENTRY*)b;
	// Directory goes before file
	if(c->type_name[0] != d->type_name[0])
		if(c->type_name[0] == DT_DIR)
			return -1;
		else
			return 1;
	else {
		return strcmp(c->type_name+1, d->type_name+1);
	}
	return 0;
}

void freeDir(void)
{
	int i;
	for(i=0; i<numEnt; i++) {
		free(list[i].type_name);
	}
	free(list);
}

void retrieveDir(char * path)
{
	freeDir();
	DIR * dir;
	struct dirent * entry;
	numEnt = 0;
	cursor = 0;
	if((dir = opendir(path))) {
		while((entry = readdir(dir))) {
			// Realloc list
			void * temp = realloc(list, sizeof(DIRENTRY)*(numEnt+1));
			if(temp) {
				list = temp;
				list[numEnt].type_name = malloc(strlen(entry->d_name)+2);
				if(list[numEnt].type_name) {
					list[numEnt].type_name[0]=entry->d_type;
					memcpy(list[numEnt].type_name+1, entry->d_name, strlen(entry->d_name) );
					list[numEnt].type_name[strlen(entry->d_name)+1]=0;
					numEnt++;
					continue;
				}
			}
			printf("Couldn't open %s\n", path);
			freeDir();
			closedir(dir);
			return;
		}
		qsort(list, numEnt, sizeof(DIRENTRY), compare);
		cursor = 0;
		closedir(dir);
	}
}

void updateBrowser(void)
{
	if(keysPres & KEY_DOWN) {
		if(cursor < (numEnt-1))
			cursor++;
	}
	if(keysPres & KEY_UP) {
		if(cursor > 0)
			cursor--;
	}
	if(keysPres & KEY_A) {
		if(list[cursor].type_name[0] == DT_DIR) {

		} else {
			if(mixer_status == STATUS_STOP) {
				char * file = list[cursor].type_name +1;
				int i;
				for(i =0; i<NUM_EXT; i++) {
					if(strstr(file, Codecs[i][0])) {
						if(strcmp(Codecs[loadedCodec][1],(Codecs[i][1]))) {
							unloadCodec(&cur_codec);
							if(!loadCodec((Codecs[i][1]), &cur_codec))
								return;

						}
						loadedCodec = i;
						startStream(&cur_codec, (const char*)(Codecs[i][1]), file);
						return;
					}
				}
			}
		}
	}
	/* Print files on screen */
	int i;
	printf("\x1b[2J");
	for(i=0; i<numEnt; i++) {
		printf((cursor == i? "*" : "-"));
		printf("%s\n", list[i].type_name+1);
	}
}
