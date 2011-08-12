#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "libfile.h"
#include "wad.h"
#include "nus.h"
#include "title.h"
void *getNusObject(const char *strCacheFolder,const char *strWadFileName,const char *strNusObjectFileName,const char *strNusObjectUrl,size_t *intNusObjectSize) {
char strCacheFileName[256],*strDeviceName;
void *varout=NULL;
static struct stWadMap *stWadFileMap;
static u16 intTmdModuleId=0;
static signed char chOpenWadFileId=-1;
    if (*strCacheFolder) {
        if (chOpenWadFileId<0) {
            strDeviceName=getDeviceFileName(strCacheFolder);
            snprintf(strCacheFileName,sizeof(strCacheFileName),"%s:/%s",strDeviceName,strWadFileName);
            free(strDeviceName);
            strDeviceName=NULL;
            if ((chOpenWadFileId=openWad(strCacheFileName,&stWadFileMap))>-1) {
                intTmdModuleId=0;
                varout=getNusObject(strCacheFolder,strWadFileName,strNusObjectFileName,strNusObjectUrl,intNusObjectSize);
            }
        }
        else {
            if (!strcmp(strNusObjectFileName,"cetk")) {
                varout=getWadData(chOpenWadFileId,WAD_TICKET_DATA,intNusObjectSize);
            }
            else if (!strncmp(strNusObjectFileName,"tmd.",4)) {
                varout=getWadData(chOpenWadFileId,WAD_TMD_DATA,intNusObjectSize);
            }
            else {
                varout=getWadData(chOpenWadFileId,intTmdModuleId,intNusObjectSize);
                intTmdModuleId++;
                if (intTmdModuleId==stWadFileMap->intTmdContentsCount) {
                    closeWad(chOpenWadFileId);
                    chOpenWadFileId=-1;
                }
            }
        }
        if (varout==NULL) {
            snprintf(strCacheFileName,sizeof(strCacheFileName),"%s/%s",strCacheFolder,strNusObjectFileName);
            varout=getFileContent(strCacheFileName,intNusObjectSize);
        }
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
unsigned int writeNusScript(const char *strNusScriptFile,struct stNusTitle *stNusTitles,unsigned int intNusTitlesCount) {
unsigned int i,j,varout=0;
FILE *fh;
    if ((fh=fopen(strNusScriptFile,"w"))!=NULL) {
        for (i=0;i<intNusTitlesCount;i++) {
            if (stNusTitles[i].intTitleId) {
                stNusTitles[i].intMajorTitleId=getMajorTitleId(stNusTitles[i].intTitleId);
                stNusTitles[i].intMinorTitleId=getMinorTitleId(stNusTitles[i].intTitleId);
            }
            else {
                stNusTitles[i].intTitleId=getFullTitleId(stNusTitles[i].intMajorTitleId,stNusTitles[i].intMinorTitleId);
            }

            for (j=0;j<i;j++) {
                if ((stNusTitles[i].intTitleId==stNusTitles[j].intTitleId) && (stNusTitles[i].intTitleVersion==stNusTitles[j].intTitleVersion)) {
                    break;
                }
            }
            if (j==i) {
                if (fprintf(fh,"%08x%08x %04x\n",stNusTitles[i].intMajorTitleId,stNusTitles[i].intMinorTitleId,stNusTitles[i].intTitleVersion)>0) {
                    varout++;
                }
            }
        }
        fclose(fh);
    }
    return varout;
}
