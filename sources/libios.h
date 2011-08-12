#ifndef _LIBIOS_H_
#define _LIBIOS_H_
#include <gctypes.h>
#include <ogc/es.h>
#include "video.h"
#define HAVE_AHBPROT ((*(volatile u32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
enum IOS_TYPES {
    NONE_IOS=0,
    STUB_IOS=1,
    BOOTMII_IOS=2,
    RESERVED_IOS=3,
    ACTIVE_IOS=4
};
enum CIOS_CMPS {
    SAME_BASE_CIOS=-1,
    DISTINCT_CIOS=0,
    SAME_CIOS=1
};
struct stModule {
   u32 *intModuleSize;
   u8 *chModuleContent;
   u16 intTmdModuleId;
};
struct stPatch {
    u32 intOffset;
    u32 intPatchSize;
    u8 *chOriginalBytesValues;
    u8 *chNewBytesValues;
};
struct stContent {
    u32 intPatchsCount;
    u32 intContentId;
    struct stPatch *stPatchs;
};
struct stCiosBase {
    u16 intBaseRevision;
    u8 chBase;
};
struct stCiosMap {
    u16 intModulesCount;
    u16 intContentsCount;
    struct stCiosBase stBase;
    struct stContent *stContents;
    struct stModule *stCiosModules;
};
struct stCiosGroup {
    u8 chCiosCount;
    struct stCiosMap *stCios;
    char *strGroupName;
    u16 intCiosRevision;
};
struct stExternalCiosModule {
    char *strModuleName;
    u32 *intModuleSize;
    u8 *chModuleContent;
};
struct stSlotInfos {
    u16 intIosRevision;
    u8 chIosType;
};
void freeExternalCiosModules();
u8 *getCiosModule(const char *strModuleName,char *strCiosContentsFolder,u32 **intModuleSize);
struct stCiosGroup *getCiosMaps(const char *strXmlCiosMap,const char *strHomebrewAppFolder,unsigned int *intCiosCount);
s32 addModule(tmd *pTmd,struct stModule *stCiosModule,u32 intContentId,const char *strNandContentFileName);
u32 applyAhbProtPatchs(bool blnVerbose);
u32 setAhbProtAccessRights(bool blnVerbose);
struct stSlotInfos *getSlotsMap();
unsigned char getCiosAlternativeRevisions(u16 *intCiosAlternativeRevisions,u16 intOfficialRevision);
enum CONSOLE_FONT_COLORS getIosTypeColor(enum IOS_TYPES IOS_TYPE);
struct stCiosMap *getCiosMap(const char *strCiosName,u8 chCiosBase,u16 intCiosRevision,struct stCiosGroup *stCiosMaps,unsigned int intCiosCount,char **strCiosGroupName);
#endif
