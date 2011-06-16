#ifndef _LIBARRAY_H_
#define _LIBARRAY_H_
#include <gctypes.h>
#include <STDDEF.H>
bool inArray(void *varArray,unsigned int intItemsCount,size_t intItemSize,void *varValue);
int getArrayMinValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize);
int getArrayMaxValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize);
#endif
