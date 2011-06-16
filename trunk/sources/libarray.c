#include <string.h>
#include "libarray.h"
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
int getArrayMinValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize) {
int varout=-1;
void *pCurrentArrayItem,*pCurrentItemMinValue;
unsigned int i;
    if (intItemsCount) {
        varout=0;
        pCurrentItemMinValue=(void *) varArrayValues;
        for (i=1;i<intItemsCount;i++) {
            pCurrentArrayItem=(void *) varArrayValues+i*intItemSize;
            if (memcmp(pCurrentArrayItem,pCurrentItemMinValue,intItemSize)<0) {
                pCurrentItemMinValue=pCurrentArrayItem;
                varout=i;
            }
        }
    }
    return varout;
}
int getArrayMaxValueIndex(const void *varArrayValues,unsigned int intItemsCount,size_t intItemSize) {
int varout=-1;
void *pCurrentArrayItem,*pCurrentItemMaxValue;
unsigned int i;
    if (intItemsCount) {
        varout=0;
        pCurrentItemMaxValue=(void *) varArrayValues;
        for (i=1;i<intItemsCount;i++) {
            pCurrentArrayItem=(void *) varArrayValues+i*intItemSize;
            if (memcmp(pCurrentArrayItem,pCurrentItemMaxValue,intItemSize)>0) {
                pCurrentItemMaxValue=pCurrentArrayItem;
                varout=i;
            }
        }
    }
    return varout;
}
