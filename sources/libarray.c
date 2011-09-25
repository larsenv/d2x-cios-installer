#include <string.h>
#include <stdio.h>
#include "libarray.h"
#include "libmath.h"
bool inArray(void *varArray,unsigned int intItemsCount,size_t intItemSize,void *varValue) {
bool varout=false;
    while (intItemsCount) {
        intItemsCount--;
        if (!memcmp(varArray+intItemsCount*intItemSize,varValue,intItemSize)) {
            varout=true;
            break;
        }
    }
    return varout;
}
unsigned int getArrayValueIndex(void *varArray,unsigned int intItemsCount,size_t intItemSize,void *varValue) {
unsigned int varout=intItemsCount;
    while (intItemsCount) {
        intItemsCount--;
        if (!memcmp(varArray+intItemsCount*intItemSize,varValue,intItemSize)) {
            varout=intItemsCount;
            break;
        }
    }
    return varout;
}
int getArrayMinValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize) {
int varout=-1;
void *pCurrentArrayItem,*pCurrentItemMinValue;
    if (intItemsCount) {
        varout=0;
        pCurrentItemMinValue=(void *) varArrayValues;
        while (intItemsCount>1) {
            intItemsCount--;
            pCurrentArrayItem=(void *) varArrayValues+intItemsCount*intItemSize;
            if (memcmp(pCurrentArrayItem,pCurrentItemMinValue,intItemSize)<0) {
                pCurrentItemMinValue=pCurrentArrayItem;
                varout=intItemsCount;
            }
        }
    }
    return varout;
}
int getArrayMaxValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize) {
int varout=-1;
void *pCurrentArrayItem,*pCurrentItemMaxValue;
    if (intItemsCount) {
        varout=0;
        pCurrentItemMaxValue=(void *) varArrayValues;
        while (intItemsCount>1) {
            intItemsCount--;
            pCurrentArrayItem=(void *) varArrayValues+intItemsCount*intItemSize;
            if (memcmp(pCurrentArrayItem,pCurrentItemMaxValue,intItemSize)>0) {
                pCurrentItemMaxValue=pCurrentArrayItem;
                varout=intItemsCount;
            }
        }
    }
    return varout;
}
unsigned int printStringArrayValues(const char *strFormatIndex,const char *strFormatValue,const char **strArray,unsigned int intArraySize) {
unsigned int varout=0,i;
static char strFormattedText[1024];
    for (i=0;i<intArraySize;i++) {
        snprintf(strFormattedText,sizeof(strFormattedText),strFormatIndex,i);
        printf("%s",strFormattedText);
        varout=varout+strlen(strFormattedText);
        snprintf(strFormattedText,sizeof(strFormattedText),strFormatValue,strArray[i]);
        printf("%s",strFormattedText);
        varout=varout+strlen(strFormattedText);
	}
    return varout;
}
unsigned int printJoinedStringArrayValues(const char *strFormatValue,const char *strJoinString,const char **strArray,unsigned int intArraySize) {
unsigned int varout=0,i;
static char strFormattedText[1024];
    for (i=0;i<intArraySize;i++) {
        if (i) {
            printf("%s",strJoinString);
            varout=varout+strlen(strJoinString);
        }
        snprintf(strFormattedText,sizeof(strFormattedText),strFormatValue,strArray[i]);
        printf("%s",strFormattedText);
        varout=varout+strlen(strFormattedText);
	}
    return varout;
}
unsigned int insertInArray(void *varArray,unsigned int varArrayItemsCount,size_t intArrayItemSize,void *varInsertItems,unsigned int intInsertItemsCount,unsigned int intInsertPos) {
unsigned int varout=0,intItemsCount;
void *pInsertOffset;
    if (intInsertPos<varArrayItemsCount) {
        intItemsCount=varArrayItemsCount-intInsertPos;
        varout=getMinValue(intInsertItemsCount,intItemsCount);
        pInsertOffset=varArray+intInsertPos*intArrayItemSize;
        memmove(pInsertOffset+varout*intArrayItemSize,pInsertOffset,(intItemsCount-varout)*intArrayItemSize);
        memcpy(pInsertOffset,varInsertItems,varout*intArrayItemSize);
    }
    return varout;
}
