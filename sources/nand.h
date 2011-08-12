#ifndef _NAND_H_
#define _NAND_H_
#include <gctypes.h>
bool existNandFile(const char *strFileName);
s32 deleteNandFile(const char *strFileName);
s32 createNandFile(const char *strFileName);
s32 writeNandFile(const char *strNandFileName,u8 *chFileContent,u32 intContentSize);
s32 createNandDirectory(const char *strFolderName);
s32 writeNandTmdFile(u64 intTitleId,u8 *sTmd,u32 intTmdSize);
#endif
