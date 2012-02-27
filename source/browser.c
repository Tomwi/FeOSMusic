#include "FeosMusic.h"

#define ENTRY_TYPE 0
#define ENTRY_NAME 1

char ** list;
int numEnt;
int cursor;
char cwd[FILENAME_MAX];

const char * Codecs [5][2]= {
	{".ogg", "ogg"},
	{".m4a", "aac"},
	{".aac", "aac"},
	{".mp3", "mp3"},
	{".flac", "flac"},
};

#define NUM_EXT 5

CODEC_INTERFACE cur_codec;
int loadedCodec;

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

		cursor = 0;
		numEnt = 0;
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
					} else
						goto error;
				}

			}

		} else {
			printf ("opendir() failure; terminating\n");
		}
error:
		qsort(list, numEnt, sizeof(char*), compare);
		closedir(pdir);
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
		if(list[cursor][ENTRY_TYPE]==DT_DIR)
			retrieveDir(&list[cursor][ENTRY_NAME]);
		else {
			if(mixer_status == STATUS_STOP) {
				char * file = &list[cursor][ENTRY_NAME];
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
						mixer_status = STATUS_PLAY;
						return;
					}
				}
			}
		}
	}
	if(keysPres & KEY_B) {
		retrieveDir("..");
	}

	printf("\x1b[2J");
	int begin = ( cursor < 12? 0 : cursor-12);
	if(begin > (numEnt- 24)) {
		begin = (numEnt- 24);
		if(begin < 0)
			begin = 0;
	}
	int i;
	for(i=begin; i<(begin+24) && i<numEnt; i++) {
		printf("%s %.29s\n", (i==cursor? "*" : "-"), &list[i][ENTRY_NAME]);
	}
}
