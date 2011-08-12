#include <stdio.h>
#include <malloc.h>
#include <ogc/gu.h>
#include <ogc/es.h>
#include <string.h>
#include "wad.h"
#include "libfile.h"
#include "macro.h"
#include "libmath.h"
#define MAX_OPENED_WADS 10
struct stWadMap stWadFileMaps[MAX_OPENED_WADS];
FILE *fpwad[MAX_OPENED_WADS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
void *getWadDataBlock(FILE *fpwad,unsigned int intOffset,size_t intDataSize) {
void *varout=NULL;
size_t intExpectedDataSize=intDataSize;
    if ((varout=getFileDataBlock(fpwad,intOffset,&intExpectedDataSize))!=NULL) {
        if (intExpectedDataSize!=intDataSize) {
            free(varout);
            varout=NULL;
        }
    }
	return varout;
}
bool setWadFileMap(FILE *fpwad,struct stWadMap *stWadFileMap) {
bool varout=false;
unsigned int intOffsetContentData;
struct stWadHeader *stWadTitleHeader;
signed_blob *sTmd;
tmd *pTmd;
tmd_content *pTmdContent;
u8 *pTmdBuffer;
    if ((stWadTitleHeader=(struct stWadHeader *) getWadDataBlock(fpwad,0,sizeof(struct stWadHeader)))!=NULL) {
        stWadFileMap->intOffsetCertsData=round_up(stWadTitleHeader->intHeaderSize,64);
        stWadFileMap->intCertsDataSize=stWadTitleHeader->intCertsSize;
        stWadFileMap->intOffsetCrlData=stWadFileMap->intOffsetCertsData+round_up(stWadTitleHeader->intCertsSize,64);
        stWadFileMap->intCrlDataSize=stWadTitleHeader->intCrlSize;
        stWadFileMap->intOffsetTicketData=stWadFileMap->intOffsetCrlData+round_up(stWadTitleHeader->intCrlSize,64);
        stWadFileMap->intTicketDataSize=stWadTitleHeader->intTicketSize;
        stWadFileMap->intOffsetTmdData=stWadFileMap->intOffsetTicketData+round_up(stWadTitleHeader->intTicketSize,64);
        stWadFileMap->intTmdDataSize=stWadTitleHeader->intTmdSize;
        if ((pTmdBuffer=(u8 *) getWadDataBlock(fpwad,stWadFileMap->intOffsetTmdData,stWadFileMap->intTmdDataSize))!=NULL) {
            sTmd=(signed_blob *) pTmdBuffer;
            pTmd=(tmd *) SIGNATURE_PAYLOAD(sTmd);
            if ((stWadFileMap->stTmdContents=(struct stWadContent *) malloc(pTmd->num_contents*sizeof(struct stWadContent)))!=NULL) {
                intOffsetContentData=stWadFileMap->intOffsetTmdData+round_up(stWadTitleHeader->intTmdSize,64);
                stWadFileMap->intTmdContentsCount=0;
                pTmdContent=TMD_CONTENTS(pTmd);
                while (stWadFileMap->intTmdContentsCount<pTmd->num_contents) {
                    stWadFileMap->stTmdContents[stWadFileMap->intTmdContentsCount].intOffsetContentData=intOffsetContentData;
                    stWadFileMap->stTmdContents[stWadFileMap->intTmdContentsCount].intContentDataSize=round_up(pTmdContent[stWadFileMap->intTmdContentsCount].size,16);
                    intOffsetContentData=intOffsetContentData+round_up(pTmdContent[stWadFileMap->intTmdContentsCount].size,64);
                    stWadFileMap->intTmdContentsCount=stWadFileMap->intTmdContentsCount+1;
                }
                stWadFileMap->intOffsetFooterData=intOffsetContentData;
                stWadFileMap->intFooterDataSize=stWadTitleHeader->intFooterSize;
                varout=true;
            }
            free(pTmdBuffer);
            pTmdBuffer=NULL;
        }
        free(stWadTitleHeader);
        stWadTitleHeader=NULL;
    }
    return varout;
}
bool isWadOpened(unsigned char chOpenWadFileId) {
bool varout=false;
    if (isInRange(chOpenWadFileId,0,MAX_OPENED_WADS,true,false)) {
        if (fpwad[chOpenWadFileId]!=NULL) {
            varout=true;
        }
    }
    return varout;
}
void *getWadData(unsigned char chOpenWadFileId,int intWadData,size_t *intDataSize) {
void *varout=NULL;
    if (isWadOpened(chOpenWadFileId)) {
        if (isInRange(intWadData,WAD_FOOTER_DATA,stWadFileMaps[chOpenWadFileId].intTmdContentsCount,true,false)) {
            switch(intWadData) {
                case WAD_CERTS_DATA:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].intCertsDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].intOffsetCertsData,*intDataSize);
                    break;
                case WAD_CRL_DATA:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].intCrlDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].intOffsetCrlData,*intDataSize);
                    break;
                case WAD_TICKET_DATA:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].intTicketDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].intOffsetTicketData,*intDataSize);
                    break;
                case WAD_TMD_DATA:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].intTmdDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].intOffsetTmdData,*intDataSize);
                    break;
                case WAD_FOOTER_DATA:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].intFooterDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].intOffsetFooterData,*intDataSize);
                    break;
                default:
                    *intDataSize=stWadFileMaps[chOpenWadFileId].stTmdContents[intWadData].intContentDataSize;
                    varout=getWadDataBlock(fpwad[chOpenWadFileId],stWadFileMaps[chOpenWadFileId].stTmdContents[intWadData].intOffsetContentData,*intDataSize);
                }
        }
    }
    return varout;
}
char getWadFreeFileId() {
char varout=-1;
unsigned char i;
    for (i=0;i<MAX_OPENED_WADS;i++) {
        if (fpwad[i]==NULL) {
            varout=i;
            break;
        }
    }
    return varout;
}
void closeWad(unsigned char chOpenWadFileId) {
    if (isWadOpened(chOpenWadFileId)) {
        free(stWadFileMaps[chOpenWadFileId].stTmdContents);
        stWadFileMaps[chOpenWadFileId].stTmdContents=NULL;
        fclose(fpwad[chOpenWadFileId]);
        fpwad[chOpenWadFileId]=NULL;
    }
}
char openWad(const char *strWadFileName,struct stWadMap **stWadFileMap) {
char varout=getWadFreeFileId();
unsigned char chOpenWadFileId;
    if (varout>-1) {
        chOpenWadFileId=varout;
        if ((fpwad[chOpenWadFileId]=fopen(strWadFileName,"rb"))==NULL) {
            varout=-1;
        }
        else {
            if (setWadFileMap(fpwad[chOpenWadFileId],&stWadFileMaps[chOpenWadFileId])) {
                *stWadFileMap=&stWadFileMaps[chOpenWadFileId];
            }
            else {
                fclose(fpwad[chOpenWadFileId]);
                fpwad[chOpenWadFileId]=NULL;
                varout=-1;
            }
        }
    }
    return varout;
}

