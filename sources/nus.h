#ifndef _NUS_H_
#define _NUS_H_
#include <gctypes.h>
struct stNusTitle {
    u32 intMajorTitleId;
    u32 intMinorTitleId;
    u64 intTitleId;
    u16 intTitleVersion;
};
void *getNusObject(const char *strCacheFolder,const char *strWadFileName,const char *strNusObjectFileName,const char *strNusObjectUrl,size_t *intNusObjectSize);
unsigned int writeNusScript(const char *strNusScriptFile,struct stNusTitle *stNusTitles,unsigned int intNusTitlesCount);
#endif
