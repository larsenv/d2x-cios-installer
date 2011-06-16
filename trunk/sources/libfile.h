#ifndef _LIBFILE_H_
#define _LIBFILE_H_
#include <gctypes.h>
int fileCmp(const char *strFileName1,const char *strFileName2,unsigned long lngOffset,size_t intBytesCount);
long getFilesize(const char *strFileName);
char *getBaseName(const char *strFileName);
char *getDeviceFileName(const char *strFileName);
char *getDirName(const char *strFileName);
void writeBinaryFile(const char *strFileName,void *varContent,size_t intContentSize);
void *getFileContent(const char *strFileName,size_t *intReadBytesCount);
#endif
