#include <stdio.h>
#include "network.h"
#include "libfile.h"
void *getNusObject(const char *strCacheFolder,const char *strNusObjectFileName,const char *strNusObjectUrl,size_t *intNusObjectSize) {
char strCacheFileName[256];
void *varout=NULL;
    if (*strCacheFolder) {
        snprintf(strCacheFileName,sizeof(strCacheFileName),"%s/%s",strCacheFolder,strNusObjectFileName);
        varout=getFileContent(strCacheFileName,intNusObjectSize);
    }
    if (varout==NULL) {
        if ((varout=getHttpFileContent(strNusObjectUrl,intNusObjectSize,15,10))!=NULL) {
            if (*strCacheFolder) {
                writeBinaryFile(strCacheFileName,varout,*intNusObjectSize);
            }
        }
    }
    return varout;
}
