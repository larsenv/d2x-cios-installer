#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libmatrice.h"
#include "libfile.h"
int fileCmp(const char *strFileName1,const char *strFileName2,unsigned long lngOffset,size_t intBytesCount) {
int varout=-1;
u8 *chDataBytes1,*chDataBytes2;
FILE *fh1,*fh2;
size_t intReadBytesCount1,intReadBytesCount2;
    if ((fh1=fopen(strFileName1,"rb"))!=NULL) {
        if (!fseek(fh1,0,SEEK_END)) {
            if (lngOffset<ftell(fh1)) {
                if (!fseek(fh1,lngOffset,SEEK_SET)) {
                    chDataBytes1=getUnsignedCharsVector(intBytesCount,0);
                    intReadBytesCount1=fread(chDataBytes1,1,intBytesCount,fh1);
                    if ((fh2=fopen(strFileName2,"rb"))!=NULL) {
                        if (!fseek(fh2,0,SEEK_END)) {
                            if (lngOffset<ftell(fh2)) {
                                if (!fseek(fh2,lngOffset,SEEK_SET)) {
                                    chDataBytes2=getUnsignedCharsVector(intBytesCount,0);
                                    intReadBytesCount2=fread(chDataBytes2,1,intBytesCount,fh2);
                                    if (!memcmp(chDataBytes1,chDataBytes2,intBytesCount)) {
                                        varout=abs(intReadBytesCount2-intReadBytesCount1);
                                    }
                                    free(chDataBytes2);
                                }
                            }
                        }
                        fclose(fh2);
                    }
                    free(chDataBytes1);
                }
            }
        }
        fclose(fh1);
    }
    return varout;
}
long getFilesize(const char *strFileName) {
FILE *fh;
long varout=-1;
    if ((fh=fopen(strFileName,"r"))!=NULL) {
        if (!fseek(fh,0,SEEK_END)) {
            varout=ftell(fh);
        }
        fclose(fh);
    }
    return varout;
}
char *getBaseName(const char *strFileName) {
char *varout;
    varout=strrchr(strFileName,'/');
    return (varout==NULL)?(char *) &strFileName[0]:varout+1;
}
char *getDeviceFileName(const char *strFileName) {
char *varout;
char *chDeviceLastCharOffset;
size_t intDeviceNameSize;
    if ((chDeviceLastCharOffset=strchr(strFileName,':'))==NULL) {
        varout=strdup("");
    }
    else {
        intDeviceNameSize=chDeviceLastCharOffset-strFileName;
        varout=getCharsVector(intDeviceNameSize+1,0);
        strncpy(varout,strFileName,intDeviceNameSize);
    }
    return varout;
}
char *getDirName(const char *strFileName) {
char *varout;
char *chLastSlashOffset;
size_t intDirNameSize;
    if ((chLastSlashOffset=strrchr(strFileName,'/'))==NULL) {
        varout=strdup("");
    }
    else {
        intDirNameSize=chLastSlashOffset-strFileName;
        varout=getCharsVector(intDirNameSize+1,0);
        strncpy(varout,strFileName,intDirNameSize);
    }
    return varout;
}
void writeBinaryFile(const char *strFileName,void *varContent,size_t intContentSize) {
FILE *fd;
    if ((fd=fopen(strFileName,"wb"))!=NULL) {
        fwrite(varContent,intContentSize,1,fd);
        fclose(fd);
    }
}
void *getFileContent(const char *strFileName,size_t *intReadBytesCount) {
FILE *fd;
void *varout=NULL;
long lngFileSize;
    if ((fd=fopen(strFileName,"rb"))!=NULL) {
        if (!fseek(fd,0,SEEK_END)) {
            lngFileSize=ftell(fd);
            if (lngFileSize>-1) {
                *intReadBytesCount=lngFileSize;
                if (!fseek(fd,0,SEEK_SET)) {
                    if ((varout=malloc(*intReadBytesCount))!=NULL) {
                        if (fread(varout,*intReadBytesCount,1,fd)!=1) {
                            free(varout);
                            varout=NULL;
                        }
                    }
                }
            }
        }
        fclose(fd);
    }
    return varout;
}
