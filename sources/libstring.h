#ifndef _LIBSTRING_H_
#define _LIBSTRING_H_
char **getSplitStrings(const char *strValue,const char *strSeparator,unsigned int *intItemsCount);
char *getBreakString(const char *strValue,const char chBreakChar,size_t intMaxStringSize);
char **getBreakStrings(const char *strValue,const char chBreakChar,size_t intMaxLineSize,unsigned int *intBreakStringsCount);
unsigned int getLinesCount(const char *strValue,unsigned int *intMaxLineSize);
#endif
