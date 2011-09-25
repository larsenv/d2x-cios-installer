#include <ogc/isfs.h>
#include <ogc/gu.h>
#include <stdio.h>
#include <gcutil.h>
#include <string.h>
#include "nand.h"
#include "debug.h"
#include "title.h"
bool haveNandAccess() {
bool varout=true;
int intFd;
    if ((intFd=ISFS_Open("/title/00000001/00000002/content/title.tmd",ISFS_OPEN_RW))<0) {
        varout=false;
    }
    else {
        ISFS_Close(intFd);
    }
    return varout;
}
bool existNandFile(const char *strFileName) {
bool varout=true;
int fd;
    if ((fd=ISFS_Open(strFileName,ISFS_OPEN_READ))<0) {
        varout=false;
    }
    else {
        ISFS_Close(fd);
    }
    return varout;
}
s32 deleteNandFile(const char *strFileName) {
s32 varout=ISFS_OK;
    if (existNandFile(strFileName)) {
        varout=ISFS_Delete(strFileName);
    }
    return varout;
}
s32 createNandFile(const char *strFileName) {
s32 varout=-1;
    if (deleteNandFile(strFileName)==ISFS_OK) {
        varout=ISFS_CreateFile(strFileName,0,ISFS_OPEN_RW,ISFS_OPEN_RW,ISFS_OPEN_RW);
    }
    return varout;
}
s32 writeNandFile(const char *strNandFileName,u8 *chFileContent,u32 intContentSize) {
s32 varout,intWriteBytes;
static u8 chBuffer[1024] ATTRIBUTE_ALIGN(0x20);
int fd;
u32 intReadBytes;
    if ((varout=createNandFile(strNandFileName))==ISFS_OK) {
        if ((fd=ISFS_Open(strNandFileName,ISFS_OPEN_WRITE))<0) {
            varout=fd;
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_Open(%s) returned %d",strNandFileName,fd);
        }
        else {
            varout=0;
            while (varout<intContentSize) {
                intReadBytes=MIN(intContentSize-varout,1024);
                memcpy(chBuffer,chFileContent+varout,intReadBytes);
                if ((intWriteBytes=ISFS_Write(fd,chBuffer,intReadBytes))<0) {
                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_Write(%d,%p,%d) returned %d at offset %d",fd,chBuffer,intReadBytes,intWriteBytes,varout);
                    varout=intWriteBytes;
                    break;
                }
                else {
                    varout=varout+intWriteBytes;
                }
            }
            ISFS_Close(fd);
        }
    }
    else {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\ncreateNandFile(%s) returned %d",strNandFileName,varout);
    }
    return varout;
}
s32 createNandDirectory(const char *strFolderName) {
s32 varout;
    if ((varout=ISFS_CreateDir(strFolderName,0,ISFS_OPEN_RW,ISFS_OPEN_RW,ISFS_OPEN_RW))) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_CreateDir(/tmp/patchmii) returned %d",varout);
    }
    return varout;
}
s32 writeNandTmdFile(u64 intTitleId,u8 *sTmd,u32 intTmdSize) {
static char strNandTmdFileName[43];
    sprintf(strNandTmdFileName,"/title/%08x/%08x/content/title.tmd",getMajorTitleId(intTitleId),getMinorTitleId(intTitleId));
    return writeNandFile(strNandTmdFileName,sTmd,intTmdSize);
}
