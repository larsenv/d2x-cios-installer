#ifndef _PATCHMII_H_
#define _PATCHMII_H_
#include <gctypes.h>
#include "libios.h"
int patchmii(struct stCiosMap *stSelectedCios,u8 chSelectedCiosSlot,u16 intCiosRevision,u32 intMajorTitleId,const char *strCacheFolder,const char *strNandWorkFolder);
#endif
