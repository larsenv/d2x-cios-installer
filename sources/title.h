#ifndef _TITLE_H_
#define _TITLE_H_
#include <gctypes.h>
#include <ogc/es.h>
#include "gui.h"
u32 getMajorTitleId(u64 intTitleId);
u32 getMinorTitleId(u64 intTitleId);
u64 getFullTitleId(u32 intMajorTitleId,u32 intMinorTitleId);
u8 *getContentCryptParameter(u8 *chCryptParameter,u16 intTmdModuleId);
void decryptContent(u8 *chCryptParameter, u8 *chEncryptedBuffer, u8 *chDecryptedBuffer,u32 intContentSize);
void encryptContent(u8 *chCryptParameter,u8 *chDecryptedBuffer,u8 *chEncryptedBuffer,u32 intContentSize);
s32 installTmdContent(tmd *pTmd,u16 intTmdModuleId,const char *strNandContentLocation);
u8 *getTitleKey(signed_blob *sTik,u8 *key);
void changeTicketTitleId(signed_blob *sTik,u32 intMajorTitleId,u32 intMinorTitleId);
void changeTmdTitleId(signed_blob *sTmd,u32 intMajorTitleId,u32 intMinorTitleId);
void setZeroSignature(signed_blob *sSig);
void bruteTmd(tmd *pTmd);
void bruteTicket(tik *pTik);
void forgeTmd(signed_blob *sTmd);
void forgeTicket(signed_blob *sTik);
s32 installTicket(const signed_blob *sTik, const signed_blob *sCerts,u32 intCertsSize,const signed_blob *sCrl,u32 intCrlsize);
s32 installTmdContents(const signed_blob *sTmd,const signed_blob *sCerts,u32 intCertsSize,const signed_blob *sCrl,u32 intCrlsize,const char *strNandContentLocation,struct stProgressBar *stProgressBarSettings);
u64 *getTitles(u32 *intTitlesCount);
bool existTitle(u64 intTitleId);
s32 deleteTicket(u64 intTitleId);
s32 deleteTitle(u64 intTitleId);
signed_blob *getStoredTmd(u64 intTitleId,u32 *intTmdSize);
u64 getStoredTitleIos(u64 intTitleId);
u16 getStoredTitleVersion(u64 intTitleId);
s32 setStoredTitleVersion(u64 intTitleId,u16 intVersion);
u16 downgradeStoredOverwrittenTitleVersion(u64 intTitleId,u16 intFutureTitleVersion);
u16 replaceStoredTitleVersion(u64 intTitleId,u16 intTitleVersion,u16 intReplaceTitleVersion);
s32 uninstallTitle(u32 intMajorTitleId,u32 intMinorTitleId);
#endif
