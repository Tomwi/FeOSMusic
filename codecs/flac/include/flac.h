FEOS_EXPORT int openFile(const char * name);
FEOS_EXPORT int getSampleRate(void);
FEOS_EXPORT int getnChannels(void);
FEOS_EXPORT int seek(int pos);
FEOS_EXPORT int getPosition(void);
FEOS_EXPORT int getResolution(void);
FEOS_EXPORT void freeDecoder(void);
FEOS_EXPORT int decSamples(int length, short * destBuf, void * context);
FEOS_EXPORT void getFlags(int* flags);
#define READ_BUF_SIZE 1940
