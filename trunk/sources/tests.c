#include <stdio.h>
#include <ogc/gu.h>
#include <gctypes.h>
#include <ogc/es.h>
#include "debug.h"
#include "libfile.h"
#include "libios.h"
#include "controllers.h"
bool validCiosContents(struct stCiosMap *stCios,u16 intCiosRevision,const char *strModWiiTestFolder,const char *strPrebuildCiosFolder) {
bool varout=true;
u16 i;
int intFileCmp;
char strPrebuildCiosContentFileName[256];
char strModWiiCiosContentFileName[256];
    for (i=0;i<stCios->intContentsCount;i++) {
        snprintf(strPrebuildCiosContentFileName,sizeof(strPrebuildCiosContentFileName),"%s/%08x.app",strPrebuildCiosFolder,i);
        snprintf(strModWiiCiosContentFileName,sizeof(strModWiiCiosContentFileName),"%s/%08x.app",strModWiiTestFolder,stCios->stContents[i].intContentId);
        if ((intFileCmp=fileCmp(strPrebuildCiosContentFileName,strModWiiCiosContentFileName,0,MAX(getFileSize(strPrebuildCiosContentFileName),getFileSize(strModWiiCiosContentFileName))))) {
            if (intFileCmp<0) {
                printDebugMsg(NORMAL_DEBUG_MESSAGE,"files %s and %s differ in content\n",strPrebuildCiosContentFileName,strModWiiCiosContentFileName);
                varout=(varout && false);
            }
            else {
                printDebugMsg(NORMAL_DEBUG_MESSAGE,"files %s and %s differ in size (%d bytes)\n",strPrebuildCiosContentFileName,strModWiiCiosContentFileName,intFileCmp);
            }
        }
    }
    return varout;
}
void showTmdDetails(const tmd *pTmd) {
const tmd_content *pTmdContent;
u16 i;
    pTmdContent=TMD_CONTENTS(pTmd);
    for (i=0;i<pTmd->num_contents;i++) {
        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Content Index: %d, ID: %d, Type: %d, Size: %llu\n",pTmdContent[i].index,pTmdContent[i].cid,pTmdContent[i].type,pTmdContent[i].size);
    }
}
void showTmdsDiffs(const tmd *pTmd,const tmd *pPrebuildTmd) {
const tmd_content *pTmdContent,*pPrebuildTmdContent;
u16 i;
    pTmdContent=TMD_CONTENTS(pTmd);
    pPrebuildTmdContent=TMD_CONTENTS(pPrebuildTmd);
    if (pTmd->num_contents==pPrebuildTmd->num_contents) {
        for (i=0;i<pTmd->num_contents;i++) {
            printDebugMsg(NORMAL_DEBUG_MESSAGE,"Content Index: %d, ID: %d, Type: %d, Size: %llu\n",pTmdContent[i].index-pPrebuildTmdContent[i].index,pTmdContent[i].cid-pPrebuildTmdContent[i].cid,pTmdContent[i].type-pPrebuildTmdContent[i].type,pTmdContent[i].size-pPrebuildTmdContent[i].size);
        }
    }
    else {
        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Tmds differ\n");
    }
}
void showCiosMaps(struct stCiosGroup *stCiosMaps,unsigned int intCiosCount) {
unsigned int i,l,m;
unsigned char j;
u16 k;
    for (i=0;i<intCiosCount;i++) {
        for (j=0;j<stCiosMaps[i].chCiosCount;j++) {
            printDebugMsg(NORMAL_DEBUG_MESSAGE,"%s cIOS rev%d base %d v%d: %d contents with %d modules\n",stCiosMaps[i].strGroupName,stCiosMaps[i].intCiosRevision,stCiosMaps[i].stCios[j].stBase.chBase,stCiosMaps[i].stCios[j].stBase.intBaseRevision,stCiosMaps[i].stCios[j].intContentsCount,stCiosMaps[i].stCios[j].intModulesCount);
            for (k=0;k<stCiosMaps[i].stCios[j].intContentsCount;k++) {
                printDebugMsg(NORMAL_DEBUG_MESSAGE,"Content %x: %d patches\n",stCiosMaps[i].stCios[j].stContents[k].intContentId,stCiosMaps[i].stCios[j].stContents[k].intPatchsCount);
                for (l=0;l<stCiosMaps[i].stCios[j].stContents[k].intPatchsCount;l++) {
                    printDebugMsg(NORMAL_DEBUG_MESSAGE,"Patch %d: %d bytes at offset %x\nOriginal byte-->New byte:",l,stCiosMaps[i].stCios[j].stContents[k].stPatchs[l].intPatchSize,stCiosMaps[i].stCios[j].stContents[k].stPatchs[l].intOffset);
                    for (m=0;m<stCiosMaps[i].stCios[j].stContents[k].stPatchs[l].intPatchSize;m++) {
                        printDebugMsg(NORMAL_DEBUG_MESSAGE,"%x-->%x ",stCiosMaps[i].stCios[j].stContents[k].stPatchs[l].chOriginalBytesValues[m],stCiosMaps[i].stCios[j].stContents[k].stPatchs[l].chNewBytesValues[m]);
                    }
                    printDebugMsg(NORMAL_DEBUG_MESSAGE,"\n");
                }
                waitPadsKeyPressed("Press any button to continue...\n");
            }
            for (k=0;k<stCiosMaps[i].stCios[j].intModulesCount;k++) {
                printDebugMsg(NORMAL_DEBUG_MESSAGE,"Module %d: %d bytes at offset %p\n",stCiosMaps[i].stCios[j].stCiosModules[k].intTmdModuleId,*(stCiosMaps[i].stCios[j].stCiosModules[k].intModuleSize),stCiosMaps[i].stCios[j].stCiosModules[k].chModuleContent);
            }
            waitPadsKeyPressed("Press any button to continue...\n");
        }
    }
}
