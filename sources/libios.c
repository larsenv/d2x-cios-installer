#include <stdio.h>
#include <mxml.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <gctypes.h>
#include "libstring.h"
#include "DIPP21_elf.h"
#include "DIPP21005_elf.h"
#include "DIPP21006_elf.h"
#include "EHCI21003_elf.h"
#include "EHCI21004_elf.h"
#include "EHCI21005_elf.h"
#include "EHCI21006_elf.h"
#include "ES21_elf.h"
#include "ES21004_elf.h"
#include "ES21005_elf.h"
#include "ES21006_elf.h"
#include "FAT21003_elf.h"
#include "FAT21004_elf.h"
#include "FAT21005_elf.h"
#include "FAT21006_elf.h"
#include "FFSP21003_elf.h"
#include "FFSP21004_elf.h"
#include "FFSP21005_elf.h"
#include "FFSP21006_elf.h"
#include "MLOAD21_elf.h"
#include "SDHC21_elf.h"
#include "USBS21_elf.h"
#include "debug.h"
#include "libios.h"
#include "libmatrice.h"
#include "libmath.h"
#include "d2x-cios-installer.h"
#include <ogc/machine/processor.h>
#include <ogc/cache.h>
#include "sha1.h"
#include "title.h"
#include "libfile.h"
#if TESTING_CODE == 0
#include "nand.h"
#endif
#define MEM_REG_BASE 0xd8b4000
#define MEM_PROT (MEM_REG_BASE + 0x20a)
static const u8 chDiReadlimitOriginalBytes[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x00,0x46,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x7E,0xD4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08};
static const u8 chDiReadlimitNewBytes[]={0x7e,0xd4};
static const u8 chIsfsPermissionsOriginalBytes[]={0x42,0x8B,0xD0,0x01,0x25,0x66};
static const u8 chIsfsPermissionsNewBytes[]={0x42,0x8B,0xE0,0x01,0x25,0x66};
static const u8 chEsSetuidOriginalBytes[]={0xD1,0x2A,0x1C,0x39};
static const u8 chEsSetuidNewBytes[]={0x46,0xC0};
static const u8 chEsIdentifyOriginalBytes[]={0x28,0x03,0xD1,0x23};
static const u8 chEsIdentifyNewBytes[]={0x00,0x00};
static const u8 chOldTruchaOriginalBytes[]={0x20,0x07,0x23,0xA2};
static const u8 chTruchaNewBytes[]={0x00};
static const u8 chNewTruchaOriginalBytes[]={0x20,0x07,0x4B,0x0B};
static unsigned short int intStubsMap[256]={-1,-1,-1,-1,65280,-1,-1,-1,-1,-1,768,256,-1,-1,-1,-1,512,-1,-1,-1,256,-1,-1,-1,-1,-1,-1,-1,-1,-1,2816,-1,-1,-1,-1,-1,-1,-1,-1,-1,3072,-1,-1,-1,-1,-1,-1,-1,-1,-1,5120,4864,5888,-1,-1,-1,-1,-1,-1,-1,6400,-1,-1,-1,-1,-1,-1,-1,-1,-1,6912,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,65280,65280,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,65280,65280,-1,-1,-1,65280,-1};
unsigned short int intExternalCiosModulesCount=0;
struct stExternalCiosModule *stExternalCiosModules=NULL;
u8 *getExternalCiosModule(const char *strModuleName,char *strCiosContentsFolder,u32 **intModuleSize) {
u8 *varout=NULL;
static char strCacheModuleFileName[256];
int i;
    if (*strCiosContentsFolder) {
        for (i=intExternalCiosModulesCount-1;i>=0;i--) {
            if (!strcmp(strModuleName,stExternalCiosModules[i].strModuleName)) {
                varout=stExternalCiosModules[i].chModuleContent;
                *intModuleSize=stExternalCiosModules[i].intModuleSize;
                break;
            }
        }
        if (varout==NULL) {
            stExternalCiosModules=(struct stExternalCiosModule *) realloc(stExternalCiosModules,(intExternalCiosModulesCount+1)*sizeof(struct stExternalCiosModule));
            snprintf(strCacheModuleFileName,sizeof(strCacheModuleFileName),"%s/%s.app",strCiosContentsFolder,strModuleName);
            stExternalCiosModules[intExternalCiosModulesCount].strModuleName=strdup(strModuleName);
            if ((stExternalCiosModules[intExternalCiosModulesCount].intModuleSize=(u32 *) malloc(sizeof(u32)))!=NULL) {
                stExternalCiosModules[intExternalCiosModulesCount].chModuleContent=getFileContent(strlwr(strCacheModuleFileName),stExternalCiosModules[intExternalCiosModulesCount].intModuleSize);
                varout=stExternalCiosModules[intExternalCiosModulesCount].chModuleContent;
                *intModuleSize=stExternalCiosModules[intExternalCiosModulesCount].intModuleSize;
                intExternalCiosModulesCount++;
            }
        }
    }
    return varout;
}
void freeExternalCiosModules() {
    if (intExternalCiosModulesCount) {
        free(stExternalCiosModules);
    }
}
u8 *getCiosModule(const char *strModuleName,char *strCiosContentsFolder,u32 **intModuleSize) {
u8 *varout=NULL;
    if (!strcmp(strModuleName,"DIPP21")) {
        varout=(u8 *) &DIPP21_elf[0];
        *intModuleSize=(u32 *) &DIPP21_elf_size;
    }
    else if (!strcmp(strModuleName,"DIPP21005")) {
        varout=(u8 *) &DIPP21005_elf[0];
        *intModuleSize=(u32 *) &DIPP21005_elf_size;
    }
    else if (!strcmp(strModuleName,"DIPP21006")) {
        varout=(u8 *) &DIPP21006_elf[0];
        *intModuleSize=(u32 *) &DIPP21006_elf_size;
    }
    else if (!strcmp(strModuleName,"EHCI21003")) {
        varout=(u8 *) &EHCI21003_elf[0];
        *intModuleSize=(u32 *) &EHCI21003_elf_size;
    }
    else if (!strcmp(strModuleName,"EHCI21004")) {
        varout=(u8 *) &EHCI21004_elf[0];
        *intModuleSize=(u32 *) &EHCI21004_elf_size;
    }
    else if (!strcmp(strModuleName,"EHCI21005")) {
        varout=(u8 *) &EHCI21005_elf[0];
        *intModuleSize=(u32 *) &EHCI21005_elf_size;
    }
    else if (!strcmp(strModuleName,"EHCI21006")) {
        varout=(u8 *) &EHCI21006_elf[0];
        *intModuleSize=(u32 *) &EHCI21006_elf_size;
    }
    else if (!strcmp(strModuleName,"ES21")) {
        varout=(u8 *) &ES21_elf[0];
        *intModuleSize=(u32 *) &ES21_elf_size;
    }
    else if (!strcmp(strModuleName,"ES21004")) {
        varout=(u8 *) &ES21004_elf[0];
        *intModuleSize=(u32 *) &ES21004_elf_size;
    }
    else if (!strcmp(strModuleName,"ES21005")) {
        varout=(u8 *) &ES21005_elf[0];
        *intModuleSize=(u32 *) &ES21005_elf_size;
    }
    else if (!strcmp(strModuleName,"ES21006")) {
        varout=(u8 *) &ES21006_elf[0];
        *intModuleSize=(u32 *) &ES21006_elf_size;
    }
    else if (!strcmp(strModuleName,"FAT21003")) {
        varout=(u8 *) &FAT21003_elf[0];
        *intModuleSize=(u32 *) &FAT21003_elf_size;
    }
    else if (!strcmp(strModuleName,"FAT21004")) {
        varout=(u8 *) &FAT21004_elf[0];
        *intModuleSize=(u32 *) &FAT21004_elf_size;
    }
    else if (!strcmp(strModuleName,"FAT21005")) {
        varout=(u8 *) &FAT21005_elf[0];
        *intModuleSize=(u32 *) &FAT21005_elf_size;
    }
    else if (!strcmp(strModuleName,"FAT21006")) {
        varout=(u8 *) &FAT21006_elf[0];
        *intModuleSize=(u32 *) &FAT21006_elf_size;
    }
    else if (!strcmp(strModuleName,"FFSP21003")) {
        varout=(u8 *) &FFSP21003_elf[0];
        *intModuleSize=(u32 *) &FFSP21003_elf_size;
    }
    else if (!strcmp(strModuleName,"FFSP21004")) {
        varout=(u8 *) &FFSP21004_elf[0];
        *intModuleSize=(u32 *) &FFSP21004_elf_size;
    }
    else if (!strcmp(strModuleName,"FFSP21005")) {
        varout=(u8 *) &FFSP21005_elf[0];
        *intModuleSize=(u32 *) &FFSP21005_elf_size;
    }
    else if (!strcmp(strModuleName,"FFSP21006")) {
        varout=(u8 *) &FFSP21006_elf[0];
        *intModuleSize=(u32 *) &FFSP21006_elf_size;
    }
    else if (!strcmp(strModuleName,"MLOAD21")) {
        varout=(u8 *) &MLOAD21_elf[0];
        *intModuleSize=(u32 *) &MLOAD21_elf_size;
    }
    else if (!strcmp(strModuleName,"SDHC21")) {
        varout=(u8 *) &SDHC21_elf[0];
        *intModuleSize=(u32 *) &SDHC21_elf_size;
    }
    else if (!strcmp(strModuleName,"USBS21")) {
        varout=(u8 *) &USBS21_elf[0];
        *intModuleSize=(u32 *) &USBS21_elf_size;
    }
    if (varout==NULL) {
        varout=getExternalCiosModule(strModuleName,strCiosContentsFolder,intModuleSize);
    }
    return varout;
}
struct stCiosGroup *getCiosMaps(const char *strXmlCiosMap,const char *strHomebrewAppFolder,unsigned int *intCiosCount) {
struct stCiosGroup *stCiosMaps=NULL;
char *chStopCharConversion,**strContentPatchOriginalBytesValues,**strContentPatchNewBytesValues,**strCiosContentsSubFolders,*strModuleName,*strContentPatchsCount,strCiosContentsFolder[256];
unsigned int intXmlCiosCount,intContentPatchSize,i;
u8 chXmlGroupCiosCount;
u16 intXmlCiosContentsCount,intXmlCiosModulesCount;
u32 intXmlContentPatchsCount;
    *intCiosCount=0;
    mxml_node_t *pXml=NULL;
    mxml_node_t *pXmlCiosGroups=NULL;
    mxml_node_t *pXmlCiosGroup=NULL;
    mxml_node_t *pXmlCiosBase=NULL;
    mxml_node_t *pXmlCiosContent=NULL;
    mxml_node_t *pXmlCiosContentPatch=NULL;
    if ((pXml=mxmlLoadString(NULL,strXmlCiosMap,MXML_NO_CALLBACK))) {
        if ((pXmlCiosGroups=mxmlFindElement(pXml,pXml,"ciosmaps",NULL,NULL,MXML_DESCEND_FIRST))) {
            intXmlCiosCount=atoi(mxmlElementGetAttr(pXmlCiosGroups,"ciosgroupscount"));
            stCiosMaps=(struct stCiosGroup *) malloc(intXmlCiosCount*sizeof(struct stCiosGroup));
            if (stCiosMaps==NULL) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to allocate stCiosMaps memory blocks");
                exit(2);
            }
            else {
                for (pXmlCiosGroup=mxmlFindElement(pXmlCiosGroups,pXmlCiosGroups,"ciosgroup",NULL,NULL,MXML_DESCEND_FIRST);pXmlCiosGroup!=NULL;pXmlCiosGroup=mxmlFindElement(pXmlCiosGroup,pXmlCiosGroups,"ciosgroup",NULL,NULL,MXML_NO_DESCEND)) {
                    if (*intCiosCount<intXmlCiosCount) {
                        chXmlGroupCiosCount=atoi(mxmlElementGetAttr(pXmlCiosGroup,"basescount"));
                        stCiosMaps[*intCiosCount].stCios=(struct stCiosMap *) malloc(chXmlGroupCiosCount*sizeof(struct stCiosMap));
                        if (stCiosMaps[*intCiosCount].stCios==NULL) {
                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to allocate stCiosMaps[%d].stCios memory blocks",*intCiosCount);
                            exit(2);
                        }
                        else {
                            stCiosMaps[*intCiosCount].intCiosRevision=atoi(mxmlElementGetAttr(pXmlCiosGroup,"version"));
                            stCiosMaps[*intCiosCount].strGroupName=strdup(mxmlElementGetAttr(pXmlCiosGroup,"name"));
                            stCiosMaps[*intCiosCount].chCiosCount=0;
                            for (pXmlCiosBase=mxmlFindElement(pXmlCiosGroup,pXmlCiosGroup,"base",NULL,NULL,MXML_DESCEND_FIRST);pXmlCiosBase!=NULL;pXmlCiosBase=mxmlFindElement(pXmlCiosBase,pXmlCiosGroup,"base",NULL,NULL,MXML_NO_DESCEND)) {
                                if (stCiosMaps[*intCiosCount].chCiosCount<chXmlGroupCiosCount) {
                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase=atoi(mxmlElementGetAttr(pXmlCiosBase,"ios"));
                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.intBaseRevision=atoi(mxmlElementGetAttr(pXmlCiosBase,"version"));
                                    intXmlCiosContentsCount=atoi(mxmlElementGetAttr(pXmlCiosBase,"contentscount"));
                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents=(struct stContent *) malloc(intXmlCiosContentsCount*sizeof(struct stContent));
                                    if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents==NULL) {
                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to allocate stCiosMaps[%d].stCios[%d].stContents memory blocks",*intCiosCount,stCiosMaps[*intCiosCount].chCiosCount);
                                        exit(2);
                                    }
                                    else {
                                        intXmlCiosModulesCount=atoi(mxmlElementGetAttr(pXmlCiosBase,"modulescount"));
                                        stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stCiosModules=(struct stModule *) malloc(intXmlCiosModulesCount*sizeof(struct stModule));
                                        if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stCiosModules==NULL) {
                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to allocate stCiosMaps[%d].stCios[%d].stCiosModules memory blocks",*intCiosCount,stCiosMaps[*intCiosCount].chCiosCount);
                                            exit(2);
                                        }
                                        else {
                                            stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount=0;
                                            stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount=0;
                                            for (pXmlCiosContent=mxmlFindElement(pXmlCiosBase,pXmlCiosBase,"content",NULL,NULL,MXML_DESCEND_FIRST);pXmlCiosContent!=NULL;pXmlCiosContent=mxmlFindElement(pXmlCiosContent,pXmlCiosBase,"content",NULL,NULL,MXML_NO_DESCEND)) {
                                                if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount<intXmlCiosContentsCount) {
                                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intContentId=strtoul(mxmlElementGetAttr(pXmlCiosContent,"id"),&chStopCharConversion,16);
                                                    if ((strContentPatchsCount=(char *) mxmlElementGetAttr(pXmlCiosContent,"patchscount"))==NULL) {
                                                        intXmlContentPatchsCount=0;
                                                    }
                                                    else {
                                                        intXmlContentPatchsCount=atoi(strContentPatchsCount);
                                                    }
                                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs=(struct stPatch *) malloc(intXmlContentPatchsCount*sizeof(struct stPatch));
                                                    if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs==NULL) {
                                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to allocate stCiosMaps[%d].stCios[%d].stContents[%d].stPatchs memory blocks",*intCiosCount,stCiosMaps[*intCiosCount].chCiosCount,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount);
                                                        exit(2);
                                                    }
                                                    else {
                                                        stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount=0;
                                                        for (pXmlCiosContentPatch=mxmlFindElement(pXmlCiosContent,pXmlCiosContent,"patch",NULL,NULL,MXML_DESCEND_FIRST);pXmlCiosContentPatch!=NULL;pXmlCiosContentPatch=mxmlFindElement(pXmlCiosContentPatch,pXmlCiosContent,"patch",NULL,NULL,MXML_NO_DESCEND)) {
                                                            if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount<intXmlContentPatchsCount) {
                                                                stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intOffset=strtoul(mxmlElementGetAttr(pXmlCiosContentPatch,"offset"),&chStopCharConversion,16);
                                                                stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize=atoi(mxmlElementGetAttr(pXmlCiosContentPatch,"size"));
                                                                intContentPatchSize=stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize+1;
                                                                strContentPatchOriginalBytesValues=getSplitStrings(mxmlElementGetAttr(pXmlCiosContentPatch,"originalbytes"),",",&intContentPatchSize);
                                                                if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize!=intContentPatchSize) {
                                                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid size attribute value %d for the %s cIOS base %d content %x patch %d",stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intContentId,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount);
                                                                    exit(2);
                                                                }
                                                                else {
                                                                    intContentPatchSize++;
                                                                    strContentPatchNewBytesValues=getSplitStrings(mxmlElementGetAttr(pXmlCiosContentPatch,"newbytes"),",",&intContentPatchSize);
                                                                    if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize!=intContentPatchSize) {
                                                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid size attribute value %d for the %s cIOS base %d content %x patch %d",stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].intPatchSize,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intContentId,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount);
                                                                        exit(2);
                                                                    }
                                                                    else {
                                                                        stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].chOriginalBytesValues=getUnsignedCharsVector(intContentPatchSize,0);
                                                                        stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].chNewBytesValues=getUnsignedCharsVector(intContentPatchSize,0);
                                                                        for (i=0;i<intContentPatchSize;i++) {
                                                                            stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].chOriginalBytesValues[i]=strtoul(strContentPatchOriginalBytesValues[i],&chStopCharConversion,16);
                                                                            stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].stPatchs[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount].chNewBytesValues[i]=strtoul(strContentPatchNewBytesValues[i],&chStopCharConversion,16);
                                                                        }
                                                                        free(strContentPatchOriginalBytesValues);
                                                                        free(strContentPatchNewBytesValues);
                                                                        stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount++;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount<intXmlContentPatchsCount) {
                                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nOnly %d/%d patchs extracted for the %s cIOS base %d content %x",stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intPatchsCount,intXmlContentPatchsCount,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stContents[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount].intContentId);
                                                            exit(2);
                                                        }
                                                        mxmlDelete(pXmlCiosContentPatch);
                                                    }
                                                    if ((strModuleName=(char *) mxmlElementGetAttr(pXmlCiosContent,"module"))!=NULL) {
                                                        if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount<intXmlCiosModulesCount) {
                                                            stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stCiosModules[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount].intTmdModuleId=atoi(mxmlElementGetAttr(pXmlCiosContent,"tmdmoduleid"));
                                                            strcpy(strCiosContentsFolder,strHomebrewAppFolder);
                                                            if (*strCiosContentsFolder) {
                                                                strCiosContentsSubFolders=getSplitStrings(stCiosMaps[*intCiosCount].strGroupName," ",&intContentPatchSize);
                                                                for (i=0;i<intContentPatchSize;i++) {
                                                                    snprintf(strCiosContentsFolder,sizeof(strCiosContentsFolder),"%s/%s",strCiosContentsFolder,strCiosContentsSubFolders[i]);
                                                                }
                                                                free(strCiosContentsSubFolders);
                                                            }
                                                            if ((stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stCiosModules[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount].chModuleContent=getCiosModule(strModuleName,strCiosContentsFolder,&stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stCiosModules[stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount].intModuleSize))==NULL) {
                                                                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nUnable to get the module %s for the %s cIOS base %d",strModuleName,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase);
                                                                exit(2);
                                                            }
                                                            else {
                                                                stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount++;
                                                            }
                                                        }
                                                        else {
                                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid modulescount attribute value %d for the %s cIOS base %d",intXmlCiosModulesCount,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase);
                                                            exit(2);
                                                        }
                                                    }
                                                    stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount++;
                                                }
                                                else {
                                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid contentscount attribute value %d for the %s cIOS base %d",intXmlCiosContentsCount,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase);
                                                    exit(2);
                                                }
                                            }
                                            if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount<intXmlCiosContentsCount) {
                                                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nOnly %d/%d contents extracted for the %s cIOS base %d",stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intContentsCount,intXmlCiosContentsCount,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase);
                                                exit(2);
                                            }
                                            if (stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount<intXmlCiosModulesCount) {
                                                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nOnly %d/%d modules extracted for the %s cIOS base %d",stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].intModulesCount,intXmlCiosModulesCount,stCiosMaps[*intCiosCount].strGroupName,stCiosMaps[*intCiosCount].stCios[stCiosMaps[*intCiosCount].chCiosCount].stBase.chBase);
                                                exit(2);
                                            }
                                            mxmlDelete(pXmlCiosContent);
                                        }
                                    }
                                    stCiosMaps[*intCiosCount].chCiosCount++;
                                }
                                else {
                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid basescount attribute value %d for the %s cIOS group",chXmlGroupCiosCount,stCiosMaps[*intCiosCount].strGroupName);
                                    exit(2);
                                }
                            }
                            if (stCiosMaps[*intCiosCount].chCiosCount<chXmlGroupCiosCount) {
                                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nOnly %d/%d %s cIOS extracted",stCiosMaps[*intCiosCount].chCiosCount,chXmlGroupCiosCount,stCiosMaps[*intCiosCount].strGroupName);
                                exit(2);
                            }
                            mxmlDelete(pXmlCiosBase);
                        }
                        *intCiosCount=*intCiosCount+1;
                    }
                    else {
                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInvalid ciosgroupscount attribute value %d",intXmlCiosCount);
                        exit(2);
                    }
                }
                if (*intCiosCount<intXmlCiosCount) {
                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nOnly %d/%d cIOS groups extracted",*intCiosCount,intXmlCiosCount);
                    exit(2);
                }
                mxmlDelete(pXmlCiosGroup);
            }
            mxmlDelete(pXmlCiosGroups);
        }
        mxmlDelete(pXml);
    }
    return stCiosMaps;
}
s32 addModule(tmd *pTmd,struct stModule *stCiosModule,u32 intContentId,const char *strNandContentFileName) {
sha1 hash;
tmd_content *pTmdContent=(tmd_content *)TMD_CONTENTS(pTmd);
u32 intContentSize=(*(stCiosModule->intModuleSize)+31)&~31;
u8 *chModuleContent=memalign(32,intContentSize);
    memset((void *) chModuleContent,0,intContentSize);
    memcpy((void *) chModuleContent,stCiosModule->chModuleContent,*(stCiosModule->intModuleSize));
    if (stCiosModule->intTmdModuleId<pTmd->num_contents) {
        pTmdContent[pTmd->num_contents]=pTmdContent[stCiosModule->intTmdModuleId];
    }
    else {
        stCiosModule->intTmdModuleId=pTmd->num_contents;
    }
    pTmdContent[stCiosModule->intTmdModuleId].cid=intContentId;
    pTmdContent[stCiosModule->intTmdModuleId].type=0x0001;
    pTmdContent[stCiosModule->intTmdModuleId].size=intContentSize;
    pTmdContent[stCiosModule->intTmdModuleId].index=pTmd->num_contents;
    SHA1(chModuleContent,intContentSize,hash);
    memcpy(pTmdContent[stCiosModule->intTmdModuleId].hash,hash,sizeof(hash));
    pTmd->num_contents++;
#if TESTING_CODE
    return intContentSize;
#else
    return writeNandFile(strNandContentFileName,chModuleContent,intContentSize);
#endif
}
u8 getRuntimeIos() {
    return *((volatile u32 *) 0x80003140)>>16;
}
u16 getRuntimeIosVersion() {
    return *((volatile u32 *) 0x80003140) & 0xffff;
}
static u32 applyOnTheFlyIosPatchs(const char *strVerboseMessage,const u8 *chOriginalBytes,u32 chOriginalBytesCount,const u8 *chNewBytes,u32 chNewBytesCount,u32 intOffsetPatch) {
u32 varout=0,i,intMemoryRangeSize=(chNewBytesCount >> 5 << 5)+64;
u8 *pStartPatchScanAddress=(u8 *) 0x93400000,*pPatchOffset=NULL,*pPatchOffsetMemoryRange=NULL;
    printf("%s",strVerboseMessage);
    while ((u32) pStartPatchScanAddress<(0x94000000-chNewBytesCount)) {
        if (!memcmp(pStartPatchScanAddress,chOriginalBytes,chOriginalBytesCount)) {
            varout++;
            pPatchOffset=pStartPatchScanAddress+intOffsetPatch;
            pPatchOffsetMemoryRange=(u8 *)(((u32)pPatchOffset) >> 5 << 5);
            for (i=0;i<chNewBytesCount;i++) {
                *pPatchOffset++ =chNewBytes[i];
            }
            DCFlushRange(pPatchOffsetMemoryRange,intMemoryRangeSize);
            ICInvalidateRange(pPatchOffsetMemoryRange,intMemoryRangeSize);
        }
        pStartPatchScanAddress++;
    }
    if (*strVerboseMessage) {
        if (varout) {
            printf(" OK\n");
        }
        else {
            printf(" FAILED\n");
        }
    }
    return varout;
}
u32 applyAhbProtPatchs(bool blnVerbose) {
u32 varout=0;
    if (HAVE_AHBPROT) {
        write32(MEM_PROT,read32(MEM_PROT) & 0x0000FFFF);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"di_readlimit..":"",chDiReadlimitOriginalBytes,sizeof(chDiReadlimitOriginalBytes),chDiReadlimitNewBytes,sizeof(chDiReadlimitNewBytes),12);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"isfs_permissions..":"",chIsfsPermissionsOriginalBytes,sizeof(chIsfsPermissionsOriginalBytes),chIsfsPermissionsNewBytes,sizeof(chIsfsPermissionsNewBytes),0);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"es_setuid":"",chEsSetuidOriginalBytes,sizeof(chEsSetuidOriginalBytes),chEsSetuidNewBytes,sizeof(chEsSetuidNewBytes),0);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"es_identify":"",chEsIdentifyOriginalBytes,sizeof(chEsIdentifyOriginalBytes),chEsIdentifyNewBytes,sizeof(chEsIdentifyNewBytes),2);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"hash_check":"",chOldTruchaOriginalBytes,sizeof(chOldTruchaOriginalBytes),chTruchaNewBytes,sizeof(chTruchaNewBytes),1);
        varout=varout+applyOnTheFlyIosPatchs(blnVerbose?"new_hash_check":"",chNewTruchaOriginalBytes,sizeof(chNewTruchaOriginalBytes),chTruchaNewBytes,sizeof(chTruchaNewBytes),1);
    }
    return varout;
}
u8 getSlotsMap(u8 *intSlotsMap) {
u8 varout=0;
u64 *intTitles;
signed_blob *sTmd;
u32 intTitlesCount,intMajorTitleId,intMinorTitleId,intTmdSize;
tmd *pTmd;
    memset((void *) intSlotsMap,0,256);
    intTitles=getTitles(&intTitlesCount);
    if (intTitles!=NULL) {
        while (varout<intTitlesCount) {
            intMajorTitleId =(intTitles[varout] >> 32);
            intMinorTitleId =(intTitles[varout] & 0xFFFFFFFF);
            if ((intMajorTitleId!=0x1) || (intMinorTitleId<3) || (intMinorTitleId>255)) {
                intTitlesCount--;
                memcpy(&intTitles[varout],&intTitles[varout+1],(intTitlesCount-varout)*sizeof(u64));
            }
            else {
                intSlotsMap[intMinorTitleId]=ACTIVE_IOS;
                if ((sTmd=getStoredTmd(intTitles[varout],&intTmdSize))!=NULL) {
                    pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
                    if (isInRange(pTmd->title_version,intStubsMap[intMinorTitleId],65535,true,false)) {
                        intSlotsMap[intMinorTitleId]=STUB_IOS;
                    }
                    free(sTmd);
                }
                varout++;
            }
        }
        free(intTitles);
	}
    return varout;
}