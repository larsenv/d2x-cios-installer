#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ogc/gu.h>
#include <ogc/es.h>
#include "d2x-cios-installer.h"
#include "haxx_certs.h"
#include "patchmii.h"
#include "debug.h"
#include "nus.h"
#include "title.h"
#include "sha1.h"
#include "libutils.h"
#include "rijndael.h"
#include "controllers.h"
#include "video.h"
#if TESTING_CODE
#include "libfile.h"
#include "tests.h"
extern char strModWiiTestFolder[256];
extern char strPrebuildCiosFolder[256];
#else
#include "nand.h"
#include "gui.h"
#include "rijndael.h"
#endif
int patchmii(struct stCiosMap *stSelectedCios,u8 chSelectedCiosSlot,u16 intCiosRevision,u32 intMajorTitleId,const char *strCacheFolder,const char *strNandWorkFolder) {
#if TESTING_CODE
char strContentFileName[256],strTikFileName[256],strTmdFileName[256],strPrebuildTmdFileName[256],*strDeviceFileName;
signed_blob *sPrebuildTmd=NULL;
tmd *pPrebuildTmd;
#else
int intConsoleRow;
#endif
int intReturnValue,varout=0,intConsoleColumnsCount;
u16 i;
signed_blob *sTmd=NULL,*sTik=NULL,*sCerts=NULL;
u8 *pStreamTmdBuffer=NULL,*pStreamTikBuffer=NULL,chTitleKey[16],*chCryptedContent,*chDecryptedContent,chCryptParameter[16];
static u8 pTmdAlignedbuffer[MAX_SIGNED_TMD_SIZE*2] ATTRIBUTE_ALIGN(0x20);
static u8 pTikAlignedBuffer[STD_SIGNED_TIK_SIZE*2] ATTRIBUTE_ALIGN(0x20);
u32 intTmdSize,intTicketSize,intFreeContentId=0,intContentSize,j;
tmd *pTmd;
tmd_content *pTmdContent;
char strContentId[32];
char strNusObjectFileName[256],strNusObjectUrl[256];
sha1 hash;
bool blnFixTmd=false;
    intConsoleColumnsCount=getConsoleColumnsCount()-1;
#if TESTING_CODE
    printDebugMsg(NORMAL_DEBUG_MESSAGE,"[Downloading IOS%d metadata]\n",stSelectedCios->stBase.chBase);
#else
    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"[*] Downloading IOS%d metadata",stSelectedCios->stBase.chBase);
    intConsoleRow=getConsoleRow();
    drawProgressBar(intConsoleRow,getConsoleColumn(),10,intConsoleColumnsCount,0,intConsoleRow+1,CONSOLE_FONT_WHITE,CONSOLE_FONT_GREEN,3,"");
#endif
	snprintf(strNusObjectUrl,sizeof(strNusObjectUrl),"http://nus.cdn.shop.wii.com/ccs/download/%08x%08x/tmd.%d",intMajorTitleId,stSelectedCios->stBase.chBase,stSelectedCios->stBase.intBaseRevision);
    snprintf(strNusObjectFileName,sizeof(strNusObjectFileName),"tmd.%d",stSelectedCios->stBase.intBaseRevision);
#if TESTING_CODE
	printDebugMsg(NORMAL_DEBUG_MESSAGE,"Getting tmd.%d file...",stSelectedCios->stBase.intBaseRevision);
#else
	updateProgressBar("Getting tmd.%d file...",stSelectedCios->stBase.intBaseRevision);
#endif
	if ((pStreamTmdBuffer=getNusObject(strCacheFolder,strNusObjectFileName,strNusObjectUrl,&intTmdSize))==NULL) {
		printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to get NUS object tmd.%d",stSelectedCios->stBase.intBaseRevision);
		varout=1;
	}
	else {
        memcpy(pTmdAlignedbuffer,pStreamTmdBuffer,MIN(intTmdSize,sizeof(pTmdAlignedbuffer)));
        free(pStreamTmdBuffer);
        sTmd =(signed_blob *)pTmdAlignedbuffer;
        if (IS_VALID_SIGNATURE(sTmd)) {
#if TESTING_CODE
            printDebugMsg(NORMAL_DEBUG_MESSAGE," OK\n");
            printDebugMsg(NORMAL_DEBUG_MESSAGE,"Getting cetk file...");
#else
            updateProgressBar("Getting cetk file...");
#endif
            snprintf(strNusObjectUrl,sizeof(strNusObjectUrl),"http://nus.cdn.shop.wii.com/ccs/download/%08x%08x/cetk",intMajorTitleId,stSelectedCios->stBase.chBase);
            if ((pStreamTikBuffer=getNusObject(strCacheFolder,"cetk",strNusObjectUrl,&intTicketSize))==NULL) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to get NUS object cetk");
                varout=2;
            }
            else {
                memcpy(pTikAlignedBuffer,pStreamTikBuffer,MIN(intTicketSize,sizeof(pTikAlignedBuffer)));
                free(pStreamTikBuffer);
                sTik=(signed_blob *)pTikAlignedBuffer;
                if (IS_VALID_SIGNATURE(sTik)) {
                    sCerts=(signed_blob *)HAXX_certs;
                    if(IS_VALID_SIGNATURE(sCerts)) {
#if TESTING_CODE
                        printDebugMsg(NORMAL_DEBUG_MESSAGE," OK\n");
#endif
                        aes_set_key(getTitleKey(sTik,chTitleKey));
                        pTmd=(tmd*)SIGNATURE_PAYLOAD(sTmd);
                        pTmdContent=TMD_CONTENTS(pTmd);
                        if (pTmd->title_version==stSelectedCios->stBase.intBaseRevision) {
                            if (pTmd->title_version!=intCiosRevision) {
                                blnFixTmd=true;
                                pTmd->title_version=intCiosRevision;
                            }
#if TESTING_CODE
                            printDebugMsg(NORMAL_DEBUG_MESSAGE,"[Downloading contents (%d)]\n",pTmd->num_contents);
#else
                            updateProgressBar("Done.");
                            printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n[*] Downloading and patching contents (%d)",pTmd->num_contents);
                            intConsoleRow=getConsoleRow();
                            drawProgressBar(intConsoleRow,getConsoleColumn(),10,intConsoleColumnsCount,0,intConsoleRow+1,CONSOLE_FONT_WHITE,CONSOLE_FONT_GREEN,pTmd->num_contents+1,"");
#endif
                            for (i=0;i<pTmd->num_contents;i++) {
                                intFreeContentId=MAX(intFreeContentId,pTmdContent[i].cid);
                                snprintf(strContentId,sizeof(strContentId),"%08x",pTmdContent[i].cid);
#if TESTING_CODE
                                printDebugMsg(NORMAL_DEBUG_MESSAGE,"Getting %s file (%lld bytes)...",strContentId,pTmdContent[i].size);
#else
                                updateProgressBar("Getting %s file (%lld bytes)...",strContentId,pTmdContent[i].size);
#endif
                                snprintf(strNusObjectUrl,sizeof(strNusObjectUrl),"http://nus.cdn.shop.wii.com/ccs/download/%08x%08x/%s",intMajorTitleId,stSelectedCios->stBase.chBase,strContentId);
                                if ((chCryptedContent=getNusObject(strCacheFolder,strContentId,strNusObjectUrl,&intContentSize))==NULL) {
                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to get NUS object %s",strContentId);
                                    varout=3;
                                    break;
                                }
                                else {
                                    if (intContentSize % 16) {
                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nERROR: downloaded %s content size %u is not a multiple of 16",strContentId,intContentSize);
                                        free(chCryptedContent);
                                        varout=4;
                                        break;
                                    }
                                    else {
                                        if (intContentSize<pTmdContent[i].size) {
                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nERROR: only downloaded %u/%llu bytes",intContentSize,pTmdContent[i].size);
                                            free(chCryptedContent);
                                            varout=5;
                                            break;
                                        }
                                        else {
                                            if ((chDecryptedContent=malloc(intContentSize))==NULL) {
                                                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nERROR: failed to allocate chDecryptedContent (%u bytes)",intContentSize);
                                                free(chCryptedContent);
                                                varout=6;
                                                break;
                                            }
                                            else {
                                                decryptContent(getContentCryptParameter(chCryptParameter,i),chCryptedContent,chDecryptedContent,intContentSize);
                                                SHA1(chDecryptedContent,pTmdContent[i].size,hash);
                                                if (memcmp(pTmdContent[i].hash,hash,sizeof(hash))) {
                                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nhash BAD");
                                                    free(chDecryptedContent);
                                                    free(chCryptedContent);
                                                    varout=7;
                                                    break;
                                                }
                                                else {
                                                    if (stSelectedCios->stContents[i].intPatchsCount) {
                                                        blnFixTmd=true;
                                                        if (stSelectedCios->stContents[i].intContentId==pTmdContent[i].cid) {
                                                            for (j=0;j<stSelectedCios->stContents[i].intPatchsCount;j++) {
                                                                if (patchBytesData(chDecryptedContent,stSelectedCios->stContents[i].stPatchs[j].intOffset,stSelectedCios->stContents[i].stPatchs[j].chOriginalBytesValues,stSelectedCios->stContents[i].stPatchs[j].chNewBytesValues,stSelectedCios->stContents[i].stPatchs[j].intPatchSize)) {
                                                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to patch content %s",strContentId);
                                                                    free(chDecryptedContent);
                                                                    free(chCryptedContent);
                                                                    varout=8;
                                                                    break;
                                                                }
                                                            }
                                                            if (varout) {
                                                                break;
                                                            }
                                                            else {
#if TESTING_CODE
                                                                printDebugMsg(NORMAL_DEBUG_MESSAGE," (%d patchs applied)",stSelectedCios->stContents[i].intPatchsCount);
#endif
                                                                SHA1(chDecryptedContent,pTmdContent[i].size,hash);
                                                                memcpy(pTmdContent[i].hash,hash,sizeof(hash));
                                                                if (pTmdContent[i].type == 0x8001) {
                                                                    pTmdContent[i].type = 1;
                                                                }
                                                            }
                                                        }
                                                        else {
                                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nUnexpected content id detected (%s) during patching process",strContentId);
                                                            free(chDecryptedContent);
                                                            free(chCryptedContent);
                                                            varout=10;
                                                            break;
                                                        }
                                                    }
#if TESTING_CODE
                                                    printDebugMsg(NORMAL_DEBUG_MESSAGE," OK\n");
                                                    if (*strCacheFolder) {
                                                        snprintf(strContentFileName,sizeof(strContentFileName),"%s/%s.app",strModWiiTestFolder,strContentId);
                                                        writeBinaryFile(strContentFileName,chDecryptedContent,intContentSize);
                                                        snprintf(strTmdFileName,sizeof(strTmdFileName),"%s/%s.hsh",strModWiiTestFolder,strContentId);
                                                        writeBinaryFile(strTmdFileName,pTmdContent[i].hash,sizeof(hash));
                                                    }
#else
                                                    snprintf(strNusObjectFileName,sizeof(strNusObjectFileName),"%s/%08x",strNandWorkFolder,pTmdContent[i].cid);
                                                    if ((intReturnValue=writeNandFile(strNusObjectFileName,chDecryptedContent,intContentSize))<0) {
                                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nwriteNandFile(%x) returned error %d",pTmdContent[i].cid,intReturnValue);
                                                        free(chDecryptedContent);
                                                        free(chCryptedContent);
                                                        varout=9;
                                                        break;
                                                    }
#endif
                                                }
                                                free(chDecryptedContent);
                                                free(chCryptedContent);
                                            }
                                        }
                                    }
                                }
                            }
                            if (!varout) {
#if TESTING_CODE
                                printDebugMsg(NORMAL_DEBUG_MESSAGE,"[Adding contents (%d)]\n",stSelectedCios->intModulesCount);
#else
                                updateProgressBar("Done.");
                                printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n[*] Adding contents (%d)",stSelectedCios->intModulesCount);
                                intConsoleRow=getConsoleRow();
                                drawProgressBar(intConsoleRow,getConsoleColumn(),10,intConsoleColumnsCount,0,intConsoleRow+1,CONSOLE_FONT_WHITE,CONSOLE_FONT_GREEN,stSelectedCios->intModulesCount+1,"");
#endif
                                if (stSelectedCios->intModulesCount) {
                                    blnFixTmd=true;
                                    for (i=0;i<stSelectedCios->intModulesCount;i++) {
                                        intFreeContentId++;
#if TESTING_CODE
                                        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Adding content %d (%u bytes)...",intFreeContentId,*(stSelectedCios->stCiosModules[i].intModuleSize));
#else
                                        updateProgressBar("Adding content %d (%u bytes)...",intFreeContentId,*(stSelectedCios->stCiosModules[i].intModuleSize));
#endif
                                        snprintf(strNusObjectFileName,sizeof(strNusObjectFileName),"%s/%08x",strNandWorkFolder,intFreeContentId);
                                        if ((intReturnValue=addModule(pTmd,&(stSelectedCios->stCiosModules[i]),intFreeContentId,strNusObjectFileName))<0) {
                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\naddModule(%d) returned error %d",intFreeContentId,intReturnValue);
                                            varout=11;
                                            break;
                                        }
                                        else {
#if TESTING_CODE
                                            printDebugMsg(NORMAL_DEBUG_MESSAGE," OK\n");
                                            if (*strCacheFolder) {
                                                snprintf(strContentFileName,sizeof(strContentFileName),"%s/%08x.app",strModWiiTestFolder,intFreeContentId);
                                                writeBinaryFile(strContentFileName,(void *) stSelectedCios->stCiosModules[i].chModuleContent,(size_t) *stSelectedCios->stCiosModules[i].intModuleSize);
                                                snprintf(strTmdFileName,sizeof(strTmdFileName),"%s/%08x.hsh",strModWiiTestFolder,intFreeContentId);
                                                writeBinaryFile(strTmdFileName,pTmdContent[stSelectedCios->stCiosModules[i].intTmdModuleId].hash,sizeof(hash));
                                            }
#endif
                                        }
                                    }
                                }
                                if (!varout) {
#if TESTING_CODE
                                    printDebugMsg(NORMAL_DEBUG_MESSAGE,"[Fixing tmd and ticket files]\n");
#else
                                    updateProgressBar("Done.");
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n[*] Fixing tmd and ticket files");
                                    intConsoleRow=getConsoleRow();
                                    drawProgressBar(intConsoleRow,getConsoleColumn(),10,intConsoleColumnsCount,0,intConsoleRow+1,CONSOLE_FONT_WHITE,CONSOLE_FONT_GREEN,3,"");
#endif
                                    if (stSelectedCios->stBase.chBase!=chSelectedCiosSlot) {
                                        blnFixTmd=true;
#if TESTING_CODE
                                        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Changing titleid from %08x-%08x to %08x-%08x...\n",intMajorTitleId,stSelectedCios->stBase.chBase,intMajorTitleId,chSelectedCiosSlot);
#endif
                                        changeTicketTitleId(sTik,intMajorTitleId,chSelectedCiosSlot);
                                        changeTmdTitleId(sTmd,intMajorTitleId,chSelectedCiosSlot);
#if TESTING_CODE
                                        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Forging ticket signature...\n");
#endif
                                        forgeTicket(sTik);
                                    }
#if TESTING_CODE == 0
                                    updateProgressBar("Fixing ticket...");
#endif
                                    if (blnFixTmd) {
#if TESTING_CODE
                                        printDebugMsg(NORMAL_DEBUG_MESSAGE,"Forging tmd signature...\n");
#endif
                                        forgeTmd(sTmd);
                                    }
#if TESTING_CODE
                                    printDebugMsg(NORMAL_DEBUG_MESSAGE,"[Installing cIOS]\n");
                                    showTmdDetails(pTmd);
                                    waitPadsKeyPressed("Press any button to continue...\n");
                                    if (*strCacheFolder) {
                                        snprintf(strTikFileName,sizeof(strTikFileName),"%s/%08x%08x.tik",strModWiiTestFolder,intMajorTitleId,stSelectedCios->stBase.chBase);
                                        snprintf(strTmdFileName,sizeof(strTmdFileName),"%s/%08x%08x.tmd",strModWiiTestFolder,intMajorTitleId,stSelectedCios->stBase.chBase);
                                        writeBinaryFile(strTikFileName,sTik,SIGNED_TIK_SIZE(sTik));
                                        writeBinaryFile(strTmdFileName,sTmd,SIGNED_TMD_SIZE(sTmd));
                                        strDeviceFileName=getDeviceFileName(strCacheFolder);
                                        snprintf(strPrebuildCiosFolder,sizeof(strPrebuildCiosFolder),"%s:/%d-%d",strDeviceFileName,stSelectedCios->stBase.chBase,intCiosRevision);
                                        free(strDeviceFileName);
                                        if (!validCiosContents(stSelectedCios,intCiosRevision,strModWiiTestFolder,strPrebuildCiosFolder)) {
                                            printDebugMsg(NORMAL_DEBUG_MESSAGE,"ModWii and prebuild cIOS contents differ\n");
                                        }
                                        waitPadsKeyPressed("Press any button to continue...\n");
                                        snprintf(strPrebuildTmdFileName,sizeof(strPrebuildTmdFileName),"%s/%08x%08x.tmd",strPrebuildCiosFolder,intMajorTitleId,chSelectedCiosSlot);
                                        if ((pStreamTmdBuffer=getFileContent(strPrebuildTmdFileName,&intTmdSize))!=NULL) {
                                            sPrebuildTmd=(signed_blob *) pStreamTmdBuffer;
                                            pPrebuildTmd=(tmd *)SIGNATURE_PAYLOAD(sPrebuildTmd);
                                            showTmdDetails(pPrebuildTmd);
                                            waitPadsKeyPressed("Press any button to continue...\n");
                                            showTmdsDiffs(pTmd,pPrebuildTmd);
                                            free(pStreamTmdBuffer);
                                        }
                                        else {
                                            printDebugMsg(NORMAL_DEBUG_MESSAGE,"%s file not found\n",strPrebuildTmdFileName);
                                        }
                                        waitPadsKeyPressed("Press any button to continue...\n");
                                    }
#else
                                    updateProgressBar("Fixing tmd...");
                                    updateProgressBar("Done.");
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n[*] Installing Title");
                                    intConsoleRow=getConsoleRow();
                                    drawProgressBar(intConsoleRow,getConsoleColumn(),10,intConsoleColumnsCount,0,intConsoleRow+1,CONSOLE_FONT_WHITE,CONSOLE_FONT_GREEN,2+pTmd->num_contents,"");
                                    updateProgressBar("Installing ticket...");
                                    if ((intReturnValue=installTicket(sTik,sCerts,HAXX_certs_size))<0) {
                                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\ninstallTicket returned %d", intReturnValue);
                                        varout=12;
                                    }
                                    else {
                                        aes_set_key(getTitleKey(sTik,chTitleKey));
                                        if ((intReturnValue=installTmdContents(sTmd,sCerts,HAXX_certs_size,strNandWorkFolder,true))<0) {
                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\ninstallTmdContents returned %d",intReturnValue);
                                            varout=13;
                                        }
                                        else {
                                            updateProgressBar("Done.");
                                        }
                                    }
#endif
                                }
                            }
                        }
                        else {
                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nTMD Version wrong %d != %d.\n", pTmd->title_version,stSelectedCios->stBase.intBaseRevision);
                            varout=14;
                        }
                    }
                    else {
                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nBad cert signature!");
                        varout=15;
                    }
                }
                else {
                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nBad tik signature!");
                    varout=16;
                }
            }
        }
        else {
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nBad TMD signature!");
            varout=17;
        }
	}
	return varout;
}
