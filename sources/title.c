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
#include "gui.h"
#define ALIGN(a,b) ((((a)+(b)-1)/(b))*(b))
#define round_up(x,n) (-(-(x) & -(n)))
const u8 aesCommonKey[16]={0xeb,0xe4,0x2a,0x22,0x5e,0x85,0x93,0xe4,0x48,0xd9,0xc5,0x45,0x73,0x81,0xaa,0xf7};
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
	aes_decrypt(chTitleId, chCryptedKey, chDecryptedkey, sizeof(chCryptedKey));
	pTik->titleid=(u64)intMajorTitleId << 32 | (u64)intMinorTitleId;
	memset(chTitleId,0,sizeof(chTitleId));
	memcpy(chTitleId,&pTik->titleid,sizeof(pTik->titleid));
	aes_encrypt(chTitleId,chDecryptedkey,chCryptedKey,sizeof(chDecryptedkey));
	memcpy(pEnckey,chCryptedKey,sizeof(chCryptedKey));
}
void changeTmdTitleId(signed_blob *sTmd,u32 intMajorTitleId,u32 intMinorTitleId) {
	((tmd*)SIGNATURE_PAYLOAD(sTmd))->title_id=(u64)intMajorTitleId << 32 | (u64)intMinorTitleId;
}
void setZeroSignature(signed_blob *sSig) {
  u8 *pSig=(u8 *) sSig;
  memset(pSig+4,0,SIGNATURE_SIZE(sSig)-4);
}
void bruteTmd(tmd *pTmd) {
u16 fill;
sha1 hash;
    for(fill=0;fill<65535;fill++) {
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
    for(fill=0;fill<65535;fill++) {
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
s32 installTicket(const signed_blob *sTik, const signed_blob *sCerts,u32 intCertsSize) {
s32 varout;
    if ((varout=ES_AddTicket(sTik,SIGNED_TIK_SIZE(sTik),sCerts,intCertsSize,NULL,0))<0) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddTicket failed: %d",varout);
    }
    return varout;
}
s32 installTmdContents(const signed_blob *sTmd,const signed_blob *sCerts,u32 intCertsSize,const char *strNandContentLocation,bool blnProgressBar) {
s32 varout;
u16 i;
tmd *pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
    if ((varout=ES_AddTitleStart(sTmd,SIGNED_TMD_SIZE(sTmd),sCerts,intCertsSize,NULL,0))<0) {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nES_AddTitleStart failed: %d",varout);
    }
    else {
        for (i=0;i<pTmd->num_contents;i++) {
            if (blnProgressBar) {
                updateProgressBar("Installing content %08x...",i);
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
u8 getSlotsMap(u8 *intSlotsMap) {
u8 varout=0;
u64 *intTitles;
u32 intTitlesCount,intMajorTitleId,intMinorTitleId;
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
                intSlotsMap[intMinorTitleId]=1;
                varout++;
            }
        }
        free(intTitles);
	}
    return varout;
}
