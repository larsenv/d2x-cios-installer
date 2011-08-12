#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ogc/gu.h>
#include "video.h"
#include "gui.h"
int getListItemId(unsigned int intCurrentItemId,int intMoveStep,unsigned int intListItemsCount) {
    return MIN(MAX((long) intCurrentItemId+(long) intMoveStep,0),(long) intListItemsCount-1);
}
void printListItem(u8 chRow,u8 chColumn,const char *strListLabel,unsigned int intCurrentItemId,unsigned int intListItemsCount,enum CONSOLE_FONT_COLORS HIGHLIGHT_ITEM_COLOR,char *strPrependString,char *strAppendString,unsigned char chPaddingMaxColumn,const char *strFormatItemValue,...) {
va_list pArguments;
static char strItemValue[51];
struct stConsoleCursorLocation stListItemLocation;
    printLocatedText(chRow,chColumn,&stListItemLocation,"%s%s ",strPrependString,strListLabel);
    if (!intCurrentItemId) {
        setFontFgColor(CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL);
        printf("<");
        resetPreviousFgColor();
    }
    else {
        printf("<");
    }
    setFontBgColor(HIGHLIGHT_ITEM_COLOR);
    va_start(pArguments,strFormatItemValue);
    vsnprintf(strItemValue,sizeof(strItemValue),strFormatItemValue,pArguments);
    va_end(pArguments);
    printf("%s",strItemValue);
    resetPreviousBgColor();
    if (intCurrentItemId>=intListItemsCount-1) {
        setFontFgColor(CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL);
        printf(">");
        resetPreviousFgColor();
    }
    else {
        printf(">");
    }
    printf("%s",strAppendString);
    if ((chColumn=getConsoleColumn())<=chPaddingMaxColumn) {
        printf("%*s",chPaddingMaxColumn-chColumn+1,"");
    }
    resetSavedPreviousCursorPosition();
}
