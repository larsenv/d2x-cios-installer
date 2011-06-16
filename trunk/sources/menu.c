#include <stdio.h>
#include <stdlib.h>
#include <ogc/gu.h>
#include "video.h"
int getListItemId(unsigned int intCurrentItemId,int intMoveStep,unsigned int intListItemsCount) {
    return MIN(MAX((int) intCurrentItemId+intMoveStep,0),(int) intListItemsCount-1);
}
void printListItem(const char *strItemValue,const char *strListLabel,unsigned int intCurrentItemId,unsigned int intListItemsCount,bool blnHighLightItem,char *strPrependString,char *strAppendString) {
    printf("%s%s ",strPrependString,strListLabel);
    if (!intCurrentItemId) {
        setFontFgColor(CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL);
        printf("<");
        resetPreviousFgColor();
    }
    else {
        printf("<");
    }
    if (blnHighLightItem) {
        setFontBgColor(CONSOLE_FONT_RED);
        printf("%s",strItemValue);
        resetPreviousBgColor();
    }
    else {
        printf("%s",strItemValue);
    }
    if (intCurrentItemId>=intListItemsCount-1) {
        setFontFgColor(CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL);
        printf(">");
        resetPreviousFgColor();
    }
    else {
        printf(">");
    }
    printf("%s",strAppendString);
}
