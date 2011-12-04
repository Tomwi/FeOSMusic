#include "FeosMusic.h"
#include <filebrowser.h>

#define TYPE_DIR(n) (n == DT_DIR ? 1 : 0)

#define CHECK(path,ext) (stricmp(path + len - strlen(ext), ext) == 0)

const struct {
	const char * const ext;
	const char * const mod;
} extensions[] = {
	{ ".ogg",  "ogg"  },
	{ ".m4a",  "mp4"  },
	{ ".mp3",  "mp3"  },
};
#define NUM_EXT (sizeof(extensions)/sizeof(extensions[0]))

int filter(const struct dirent *dent) {
	int len;
	int i;

	if(TYPE_DIR(dent->d_type)
	&& strcmp(dent->d_name, ".") != 0)
		return 1;

	len = strlen(dent->d_name);
	for(i = 0; i < NUM_EXT; i++) {
		if(CHECK(dent->d_name, extensions[i].ext))
			return 1;
	}

	return 0;
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
	const char *type = NULL;
	int   len;
	int   i;
	initSoundStreamer();

	if(argc == 1)
		path = pickFile("/", filter, compar);
	else
		path = strdup(argv[1]);

	if(path == NULL)
		return 0;

	len = strlen(path);
	for(i = 0; type == NULL && i < NUM_EXT; i++) {
		if(CHECK(path, extensions[i].ext))
			type = extensions[i].mod;
	}
	if(type == NULL) {
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
