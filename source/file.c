#include "file.h"


int sizeofFile(FILE * fp)
{
	rewind(fp);
	fseek(fp, 0, SEEK_END);
	int ret = ftell(fp);
	rewind(fp);
	return ret;
}
void * bufferFile(char * file)
{
	FILE * fp;
	void * buf;
	if((fp=fopen(file, "rb"))) {
		int size = sizeofFile(fp);
		if((buf = malloc(size))) {
			fread(buf, 1, size, fp);
			fclose(fp);
			return buf;
		}
	}
	free(buf);
	return NULL;
}

int bufferTo(char * file, void * dest)
{
	FILE * fp;
	if(dest) {
		if((fp=fopen(file, "rb"))) {
			int size = sizeofFile(fp);
			fread(dest, 1, size, fp);
			fclose(fp);
			return 1;
		}
	}
	return 0;
}
