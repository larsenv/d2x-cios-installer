#ifndef _LIBARRAY_H_
#define _LIBARRAY_H_
#include <gctypes.h>
#include <STDDEF.H>
bool inArray(void *varArray,unsigned int intItemsCount,size_t intItemSize,void *varValue);
unsigned int getArrayValueIndex(void *varArray,unsigned int intItemsCount,size_t intItemSize,void *varValue);
int getArrayMinValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize);
int getArrayMaxValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize);
unsigned int printStringArrayValues(const char *strFormatIndex,const char *strFormatValue,const char **strArray,unsigned int intArraySize);
unsigned int printJoinedStringArrayValues(const char *strFormatValue,const char *strJoinString,const char **strArray,unsigned int intArraySize);
unsigned int insertInArray(void *varArray,unsigned int varArrayItemsCount,size_t intArrayItemSize,void *varInsertItems,unsigned int intInsertItemsCount,unsigned int intInsertPos);
#endif
