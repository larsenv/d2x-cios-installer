#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libmatrice.h"
#include "libfile.h"
#include "libmath.h"
#include "libstring.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dirent.h>
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
                                    chDataBytes2=NULL;
                                }
                            }
                        }
                        fclose(fh2);
                    }
                    free(chDataBytes1);
                    chDataBytes1=NULL;
                }
            }
        }
        fclose(fh1);
    }
    return varout;
}
long getFilePointerSize(FILE *fp,bool blnRestoreFilePointer) {
long varout=-1,lngCurrentFilePointerOffset=ftell(fp);
    if (!fseek(fp,0,SEEK_END)) {
        varout=ftell(fp);
    }
    if (blnRestoreFilePointer) {
        if (lngCurrentFilePointerOffset>=0) {
            fseek(fp,lngCurrentFilePointerOffset,SEEK_SET);
        }
    }
    return varout;
}
long getFileSize(const char *strFileName) {
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
        varout=getCloneString("");
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
        varout=getCloneString("");
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
void *getFileDataBlock(FILE *fp,unsigned int intOffset,size_t *intDataSize) {
void *varout=NULL;
long lngDataSize=getMinValue(getFilePointerSize(fp,false)-intOffset,*intDataSize);
    if (lngDataSize>0) {
        *intDataSize=lngDataSize;
        if ((varout=malloc(*intDataSize))!=NULL) {
            if (!fseek(fp,intOffset,SEEK_SET)) {
                if ((fread(varout,*intDataSize,1,fp))!=1) {
                    *intDataSize=0;
                }
            }
            else {
                *intDataSize=0;
            }
            if (*intDataSize==0) {
                free(varout);
                varout=NULL;
            }
        }
    }
    return varout;
}
bool isDirectory(const char *strDirectory) {
bool varout=false;
struct stat stFileInfos;
    if (stat(strDirectory,&stFileInfos)==0) {
        varout=((stFileInfos.st_mode & S_IFDIR)!=0);
    }
    return varout;
}
bool removeDirectory(const char *strDirectory) {
bool varout=true;
DIR *stRootDir;
char strDirItemPath[256];
struct dirent *stDirItem;
    if ((stRootDir=opendir(strDirectory))!=NULL) {
		while (((stDirItem=readdir(stRootDir))) && (varout)) {
		    snprintf(strDirItemPath,sizeof(strDirItemPath),"%s/%s",strDirectory,stDirItem->d_name);
            if (((strcmp(stDirItem->d_name,"."))) && ((strcmp(stDirItem->d_name,"..")))) {
                if (isDirectory(strDirItemPath)) {
                    varout=removeDirectory(strDirItemPath);
                }
                else {
                    varout=(remove(strDirItemPath)==0);
                }
            }
        }
        closedir(stRootDir);
        varout=(remove(strDirectory)==0);
	}
	return varout;
}
