#ifndef _WAD_H_
#define _WAD_H_
#include <gctypes.h>
struct stWadHeader {
	u32 intHeaderSize;
	u16 intWadType;
    u16 intPadding;
    u32 intCertsSize;
	u32 intCrlSize;
	u32 intTicketSize;
	u32 intTmdSize;
	u32 intDataSize;
	u32 intFooterSize;
} ATTRIBUTE_PACKED;
struct stWadContent {
    unsigned int intOffsetContentData;
    size_t intContentDataSize;
};
struct stWadMap {
    unsigned int intOffsetCertsData;
    size_t intCertsDataSize;
    unsigned int intOffsetCrlData;
    size_t intCrlDataSize;
    unsigned int intOffsetTicketData;
    size_t intTicketDataSize;
    unsigned int intOffsetTmdData;
    size_t intTmdDataSize;
    u16 intTmdContentsCount;
    struct stWadContent *stTmdContents;
    unsigned int intOffsetFooterData;
    size_t intFooterDataSize;
};
enum WAD_DATAS {
    WAD_CERTS_DATA=-1,
    WAD_CRL_DATA=-2,
    WAD_TICKET_DATA=-3,
    WAD_TMD_DATA=-4,
    WAD_FOOTER_DATA=-5
};
void *getWadDataBlock(FILE *fpwad,unsigned int intOffset,size_t intDataSize);
bool isWadOpened(unsigned char chOpenWadFileId);
void *getWadData(unsigned char chOpenWadFileId,int intWadData,size_t *intDataSize);
char getWadFreeFileId();
void closeWad(unsigned char chOpenWadId);
char openWad(const char *strWadFileName,struct stWadMap **stWadFileMap);
#endif
