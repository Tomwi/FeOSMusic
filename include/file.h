#ifndef FILE_H
#define FILE_H
#include "FeOSMusic.h"

void * bufferFile(const char * file, int * sz);
unsigned int getFileSize(FILE* fp);

extern far_t hArc;

#endif