#ifndef _LIBUTILS_H_
#define _LIBUTILS_H_
#include <gctypes.h>
unsigned int printBreakLines(const char chBreakChar,size_t intMaxLineSize,const char *strFormatTextLine,...);
int patchBytesData(u8 *chBytesData,u32 lngPatchOffset,u8 *chOriginalBytes,u8 *chNewBytes,u32 lngPatchBytesCount);
void permutePointers(void *varValue1,void *varValue2);
void printRepeatString(unsigned int intRepeatsCount,const char *strFormatValue,...);
#endif
