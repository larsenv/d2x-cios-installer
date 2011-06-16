#ifndef _LIBIOS_H_
#define _LIBIOS_H_
#include <ogc/es.h>
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
void freeExternalCiosModules();
u8 *getCiosModule(const char *strModuleName,char *strCiosContentsFolder,u32 **intModuleSize);
struct stCiosGroup *getCiosMaps(const char *strXmlCiosMap,const char *strHomebrewAppFolder,unsigned int *intCiosCount);
s32 addModule(tmd *pTmd,struct stModule *stCiosModule,u32 intContentId,const char *strNandContentFileName);
u8 getLoadedIos();
u16 getLoadedIosVersion();
#endif
