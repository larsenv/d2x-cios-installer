#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "debug.h"
void *getVector(size_t intItemsCount,size_t intItemSize,void *varDefautValue) {
void *pVector;
    pVector=malloc(intItemsCount*intItemSize);
    if (pVector!=NULL) {
        while (intItemsCount) {
            intItemsCount--;
            memcpy(pVector+intItemsCount*intItemSize,varDefautValue,intItemSize);
		}
	}
    return pVector;
}
unsigned char *getUnsignedCharsVector(size_t intSizeVect,unsigned char chInitValue) {
unsigned char *pChar;
	pChar=(unsigned char *) malloc(intSizeVect);
	if (pChar==NULL) {
	    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to create unsigned chars vector");
		exit(3);
	}
	else {
	    pChar=memset(pChar,chInitValue,intSizeVect);
	}
	return pChar;
}
char *getCharsVector(size_t intSizeVect,char chInitValue) {
char *pChar;
	pChar=(char *) malloc(intSizeVect);
	if (pChar==NULL) {
	    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFail to create chars vector");
		exit(6);
	}
	else {
	    pChar=memset(pChar,chInitValue,intSizeVect);
	}
	return pChar;
}
