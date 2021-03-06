#include <ogc/isfs.h>
#include <ogc/gu.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <gcutil.h>
#include <stdlib.h>
#include "string.h"
#include "rijndael.h"
#include "title.h"
#include "debug.h"
#include "nand.h"
#include "sha1.h"
#include "macro.h"
#include "libios.h"
#include "stubs.h"
#include "d2x-cios-installer.h"
const u8 aesCommonKey[16]={0xeb,0xe4,0x2a,0x22,0x5e,0x85,0x93,0xe4,0x48,0xd9,0xc5,0x45,0x73,0x81,0xaa,0xf7};
u32 getMajorTitleId(u64 intTitleId) {
    return (u32) (intTitleId >> 32);
}
u32 getMinorTitleId(u64 intTitleId) {
    return (u32) (intTitleId & 0xFFFFFFFF);
}
u64 getFullTitleId(u32 intMajorTitleId,u32 intMinorTitleId) {
    return (u64) intMajorTitleId << 32 | (u64) intMinorTitleId;
}
u8 *getContentCryptParameter(u8 *chCryptParameter,u16 intTmdModuleId) {
    memset(chCryptParameter,0,16);
    return (u8 *) memcpy(chCryptParameter,&intTmdModuleId,2);
}
void decryptContent(u8 *chCryptParameter, u8 *chEncryptedBuffer, u8 *chDecryptedBuffer,u32 intContentSize) {
    aes_decrypt(chCryptParameter,chEncryptedBuffer,chDecryptedBuffer,intContentSize);
}
void encryptContent(u8 *chCryptParameter,u8 *chDecryptedBuffer,u8 *chEncryptedBuffer,u32 intContentSize) {
    aes_encrypt(chCryptParameter,chDecryptedBuffer,chEncryptedBuffer,intContentSize);
}
s32 installTmdContent(tmd *pTmd,u16 intTmdModuleId,const char *strNandContentLocation) {
s32 varout=0,intWriteBytes;
u32 intReadBytes;
static char strNandContentFilename[256];
static u8 chDecryptedBuffer[1024] ATTRIBUTE_ALIGN(0x20);
static u8 chEncryptedBuffer[1024] ATTRIBUTE_ALIGN(0x20);
static u8 chCryptParameter[16];
const tmd_content *pTmdContent=TMD_CONTENTS(pTmd);
int intReturnValue,fd,intContentFd;
    snprintf(strNandContentFilename,sizeof(strNandContentFilename),"%s/%08x",strNandContentLocation,pTmdContent[intTmdModuleId].cid);
    if ((fd=ISFS_Open(strNandContentFilename,ISFS_OPEN_READ))<0) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_OpenFile(%s) returned %d",strNandContentFilename,fd);
        varout=fd;
    }
    else {
        if ((intContentFd=ES_AddContentStart(pTmd->title_id,pTmdContent[intTmdModuleId].cid))<0) {
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddContentStart(%016llx, %x) failed: %d",pTmd->title_id,pTmdContent[intTmdModuleId].cid,intContentFd);
            varout=intContentFd;
        }
        else {
            getContentCryptParameter(chCryptParameter,pTmdContent[intTmdModuleId].index);
            while (varout<pTmdContent[intTmdModuleId].size) {
                intReadBytes=MIN(pTmdContent[intTmdModuleId].size-varout,1024);
                intReadBytes=ALIGN(intReadBytes,32);
                if ((intWriteBytes=ISFS_Read(fd,chDecryptedBuffer,intReadBytes))<0) {
                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_Read(%d, %p, %d) returned %d at offset %d",fd,chDecryptedBuffer,intReadBytes,intWriteBytes,varout);
                    varout=intWriteBytes;
                    break;
                }
                else {
                    encryptContent(chCryptParameter,chDecryptedBuffer,chEncryptedBuffer,sizeof(chDecryptedBuffer));
                    if ((intReturnValue=ES_AddContentData(intContentFd,chEncryptedBuffer,intWriteBytes))<0) {
                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddContentData(%d, %p, %d) returned %d",intContentFd,chEncryptedBuffer,intWriteBytes,intReturnValue);
                        varout=intReturnValue;
                        break;
                    }
                    else {
                        varout=varout+intWriteBytes;
                    }
                }
            }
            if ((intReturnValue=ES_AddContentFinish(intContentFd))<0) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddContentFinish failed: %d",intReturnValue);
                varout=intReturnValue;
            }
        }
        ISFS_Close(fd);
        deleteNandFile(strNandContentFilename);
    }
    return varout;
}
u8 *getTitleKey(signed_blob *sTik,u8 *chTitlekey) {
static u8 chTitleId[16] ATTRIBUTE_ALIGN(32);
static u8 chCryptedKey[16] ATTRIBUTE_ALIGN(32);
static u8 chDecryptedkey[16] ATTRIBUTE_ALIGN(32);
const tik *pTik=(tik*)SIGNATURE_PAYLOAD(sTik);
    memcpy(chCryptedKey,(u8*) &pTik->cipher_title_key,sizeof(chCryptedKey));
	memset(chDecryptedkey,0,sizeof(chDecryptedkey));
	memset(chTitleId,0,sizeof(chTitleId));
	memcpy(chTitleId,&pTik->titleid,sizeof(pTik->titleid));
	aes_set_key((u8*)aesCommonKey);
	aes_decrypt(chTitleId,chCryptedKey,chDecryptedkey,sizeof(chCryptedKey));
	return (u8 *) memcpy(chTitlekey,chDecryptedkey,sizeof(chDecryptedkey));
}
void changeTicketTitleId(signed_blob *sTik,u32 intMajorTitleId,u32 intMinorTitleId) {
static u8 chTitleId[16] ATTRIBUTE_ALIGN(32);
static u8 chCryptedKey[16] ATTRIBUTE_ALIGN(32);
static u8 chDecryptedkey[16] ATTRIBUTE_ALIGN(32);
tik *pTik = (tik*)SIGNATURE_PAYLOAD(sTik);
u8 *pEnckey = (u8*) &pTik->cipher_title_key;
    memcpy(chCryptedKey,pEnckey,sizeof(chCryptedKey));
	memset(chDecryptedkey,0,sizeof(chDecryptedkey));
	memset(chTitleId,0,sizeof(chTitleId));
	memcpy(chTitleId,&pTik->titleid,sizeof(pTik->titleid));
	aes_set_key((u8*)aesCommonKey);
	aes_decrypt(chTitleId,chCryptedKey,chDecryptedkey,sizeof(chCryptedKey));
	pTik->titleid=getFullTitleId(intMajorTitleId,intMinorTitleId);
	memset(chTitleId,0,sizeof(chTitleId));
	memcpy(chTitleId,&pTik->titleid,sizeof(pTik->titleid));
	aes_encrypt(chTitleId,chDecryptedkey,chCryptedKey,sizeof(chDecryptedkey));
	memcpy(pEnckey,chCryptedKey,sizeof(chCryptedKey));
}
void changeTmdTitleId(signed_blob *sTmd,u32 intMajorTitleId,u32 intMinorTitleId) {
	((tmd*)SIGNATURE_PAYLOAD(sTmd))->title_id=getFullTitleId(intMajorTitleId,intMinorTitleId);
}
void setZeroSignature(signed_blob *sSig) {
  u8 *pSig=(u8 *) sSig;
  memset(pSig+4,0,SIGNATURE_SIZE(sSig)-4);
}
void bruteTmd(tmd *pTmd) {
u16 fill;
sha1 hash;
    for (fill=0;fill<65535;fill++) {
        pTmd->fill3=fill;
        SHA1((u8 *)pTmd,TMD_SIZE(pTmd),hash);
        if (hash[0]==0) return;
    }
    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nUnable to fix tmd");
    exit(4);
}
void bruteTicket(tik *pTik) {
u16 fill;
sha1 hash;
    for (fill=0;fill<65535;fill++) {
        pTik->padding=fill;
        SHA1((u8 *)pTik,sizeof(tik),hash);
        if (hash[0]==0) return;
    }
    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nUnable to fix tik");
    exit(5);
}
void forgeTmd(signed_blob *sTmd) {
    setZeroSignature(sTmd);
    bruteTmd((tmd *) SIGNATURE_PAYLOAD(sTmd));
}
void forgeTicket(signed_blob *sTik) {
    setZeroSignature(sTik);
    bruteTicket((tik *) SIGNATURE_PAYLOAD(sTik));
}
s32 installTicket(const signed_blob *sTik, const signed_blob *sCerts,u32 intCertsSize,const signed_blob *sCrl,u32 intCrlsize) {
s32 varout;
    if ((varout=ES_AddTicket(sTik,SIGNED_TIK_SIZE(sTik),sCerts,intCertsSize,sCrl,intCrlsize))<0) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddTicket failed: %d",varout);
    }
    return varout;
}
s32 installTmdContents(const signed_blob *sTmd,const signed_blob *sCerts,u32 intCertsSize,const signed_blob *sCrl,u32 intCrlsize,const char *strNandContentLocation,struct stProgressBar *stProgressBarSettings) {
s32 varout;
u16 i;
tmd *pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
    if ((varout=ES_AddTitleStart(sTmd,SIGNED_TMD_SIZE(sTmd),sCerts,intCertsSize,sCrl,intCrlsize))<0) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddTitleStart failed: %d",varout);
    }
    else {
        for (i=0;i<pTmd->num_contents;i++) {
            if (stProgressBarSettings!=NULL) {
                updateProgressBar(stProgressBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Installing content %08x...",i);
            }
            if ((varout=installTmdContent(pTmd,i,strNandContentLocation))<0) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\ninstallTmdContent(%d) failed: %d",i,varout);
                break;
            }
        }
        if (i==pTmd->num_contents) {
            if((varout=ES_AddTitleFinish())<0) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddTitleFinish failed: %d",varout);
            }
        }
    }
    if (varout<0) {
        ES_AddTitleCancel();
    }
    return varout;
}
u64 *getTitles(u32 *intTitlesCount) {
u64 *varout=NULL;
    if (ES_GetNumTitles(intTitlesCount)>=0) {
        if (*intTitlesCount>0) {
            if ((varout=memalign(32,round_up((*intTitlesCount+1)*sizeof(u64),32)))) {
                if (ES_GetTitles(varout,*intTitlesCount)<0) {
                    free(varout);
                    varout=NULL;
                }
            }
        }
    }
    return varout;
}
bool existTitle(u64 intTitleId) {
static char strNandTmdFileName[43];
    snprintf(strNandTmdFileName,sizeof(strNandTmdFileName),"/title/%08x/%08x/content/title.tmd",getMajorTitleId(intTitleId),getMinorTitleId(intTitleId));
    return existNandFile(strNandTmdFileName);
}
s32 deleteTicket(u64 intTitleId) {
static char strNandTicketFileName[30];
	snprintf(strNandTicketFileName,sizeof(strNandTicketFileName),"/ticket/%08x/%08x.tik",getMajorTitleId(intTitleId),getMinorTitleId(intTitleId));
	return deleteNandFile(strNandTicketFileName);
}
s32 deleteTitle(u64 intTitleId) {
static char strNandTitleFolder[25];
	snprintf(strNandTitleFolder,sizeof(strNandTitleFolder),"/title/%08x/%08x",getMajorTitleId(intTitleId),getMinorTitleId(intTitleId));
	return ISFS_Delete(strNandTitleFolder);
}
signed_blob *getStoredTmd(u64 intTitleId,u32 *intTmdSize) {
signed_blob *varout=NULL;
    if (ES_GetStoredTMDSize(intTitleId,intTmdSize)>=0) {
        if ((varout=(signed_blob *) memalign(32,(*intTmdSize+31)&(~31)))!=NULL) {
            if (ES_GetStoredTMD(intTitleId,varout,*intTmdSize)<0) {
                free(varout);
                varout=NULL;
            }
            else {
                if (!IS_VALID_SIGNATURE(varout)) {
                    free(varout);
                    varout=NULL;
                }
            }
        }
    }
	return varout;
}
u64 getStoredTitleIos(u64 intTitleId) {
u64 varout=0;
signed_blob *sTmd;
tmd *pTmd;
u32 intTmdSize;
    if ((sTmd=getStoredTmd(intTitleId,&intTmdSize))!=NULL) {
        pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
        varout=pTmd->sys_version;
        free(sTmd);
        sTmd=NULL;
    }
    return varout;
}
u16 getStoredTitleVersion(u64 intTitleId) {
u16 varout=0;
signed_blob *sTmd;
tmd *pTmd;
u32 intTmdSize;
    if ((sTmd=getStoredTmd(intTitleId,&intTmdSize))!=NULL) {
        pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
        varout=pTmd->title_version;
        free(sTmd);
        sTmd=NULL;
    }
    return varout;
}
s32 setStoredTitleVersion(u64 intTitleId,u16 intVersion) {
s32 varout=0;
u8 *sTmd=NULL;
tmd *pTmd;
u32 intTmdSize=0;
	if ((sTmd=(u8 *) getStoredTmd(intTitleId,&intTmdSize))!=NULL) {
        pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
        pTmd->title_version=intVersion;
        writeNandTmdFile(intTitleId,sTmd,intTmdSize);
        free(sTmd);
        sTmd=NULL;
	}
	return varout;
}
u16 downgradeStoredOverwrittenTitleVersion(u64 intTitleId,u16 intFutureTitleVersion) {
u16 varout=0;
u8 *sTmd=NULL;
tmd *pTmd;
u32 intTmdSize=0;
    if (intFutureTitleVersion) {
        if ((sTmd=(u8 *) getStoredTmd(intTitleId,&intTmdSize))!=NULL) {
            pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
            if (pTmd->title_version>intFutureTitleVersion) {
                varout=pTmd->title_version;
                pTmd->title_version=0;
                if (writeNandTmdFile(intTitleId,sTmd,intTmdSize)<0) {
                    varout=0;
                }
            }
            free(sTmd);
            sTmd=NULL;
        }
	}
	return varout;
}
u16 replaceStoredTitleVersion(u64 intTitleId,u16 intTitleVersion,u16 intReplaceTitleVersion) {
u16 varout=0;
u8 *sTmd=NULL;
tmd *pTmd;
u32 intTmdSize=0;
    if (intReplaceTitleVersion) {
        if ((sTmd=(u8 *) getStoredTmd(intTitleId,&intTmdSize))!=NULL) {
            pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
            if ((varout=pTmd->title_version)==intTitleVersion) {
                pTmd->title_version=intReplaceTitleVersion;
                if (writeNandTmdFile(intTitleId,sTmd,intTmdSize)>0) {
                    varout=intReplaceTitleVersion;
                }
            }
            free(sTmd);
            sTmd=NULL;
        }
	}
	return varout;
}
s32 uninstallTitle(u32 intMajorTitleId,u32 intMinorTitleId) {
s32 varout=0;
u64 intTitleId=getFullTitleId(intMajorTitleId,intMinorTitleId);
    if (intMajorTitleId==1) {
        if (getStoredTitleIos(0x100000002ULL)==intMinorTitleId) { //SM IOS
            varout=intMinorTitleId;
        }
        else {
            switch(intMinorTitleId) {
                case 1: //boot2
                case 2: //system menu
                case 242:
                case 0x100: //BC
                case 0x101: //MIOS
                    varout=intMinorTitleId;
                    break;
                case 254: //bootmii IOS
                    if (getStoredTitleVersion(intTitleId)!=intStubsMap[254]) {
                        varout=intMinorTitleId;
                    }
                    break;
            }
		}
	}
	if (!varout) {
        if (haveNandAccess()) {
            varout=-1*deleteTicket(intTitleId)*deleteTitle(intTitleId);
        }
        else {
            varout=-1;
        }
	}
	return varout;
}

