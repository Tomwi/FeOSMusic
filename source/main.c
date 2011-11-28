#include "FeosMusic.h"
#include <filebrowser.h>

#define TYPE_DIR(n) (n == DT_DIR ? 1 : 0)

int filter(const struct dirent *dent) {
	int len;

	if(TYPE_DIR(dent->d_type)
	&& strcmp(dent->d_name, ".") != 0)
		return 1;

	len = strlen(dent->d_name);
	return stricmp(dent->d_name+len-4, ".ogg") == 0
	    || stricmp(dent->d_name+len-4, ".m4a") == 0;
}

int compar(const struct dirent **dent1, const struct dirent **dent2) {
	char isDir[2];

	isDir[0] = TYPE_DIR((*dent1)->d_type);
	isDir[1] = TYPE_DIR((*dent2)->d_type);

	if(isDir[0] == isDir[1]) // sort by name
		return stricmp((*dent1)->d_name, (*dent2)->d_name);
	else
		return isDir[1] - isDir[0]; // put directories first
}

CODEC_INTERFACE codec;
int main(int argc, char ** argv)
{
	char *path;
	char *type;
	int   len;
	initSoundStreamer();

	path = pickFile("/", filter, compar);
	if(path == NULL)
		return 0;

	len = strlen(path);
	if(strcmp(path+len-4, ".ogg") == 0)
		type = "ogg";
	else if(strcmp(path+len-4, ".m4a") == 0)
		type = "mp4";
	else {
		free(path);
		return 0;
	}
	
	startStream(&codec, type, path);
	while(1) {
		FeOS_WaitForVBlank();
		if(!updateStream(&codec)) {
			free(path);
			return 0;
		}
	}

	free(path);
	return 0;
}
