#include <stdio.h>
#include <stdlib.h>
#include <ogc/gu.h>
#include "video.h"
#include "gui.h"
int getListItemId(unsigned int intCurrentItemId,int intMoveStep,unsigned int intListItemsCount) {
    return MIN(MAX((int) intCurrentItemId+intMoveStep,0),(int) intListItemsCount-1);
}
void printListItem(u8 chRow,u8 chColumn,const char *strItemValue,const char *strListLabel,unsigned int intCurrentItemId,unsigned int intListItemsCount,enum CONSOLE_FONT_COLORS HIGHLIGHT_ITEM_COLOR,char *strPrependString,char *strAppendString,unsigned char chPaddingMaxColumn) {
    printLocatedText(chRow,chColumn,"%s%s ",strPrependString,strListLabel);
    if (!intCurrentItemId) {
        setFontFgColor(CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL);
        printf("<");
        resetPreviousFgColor();
    }
    else {
        printf("<");
    }
    setFontBgColor(HIGHLIGHT_ITEM_COLOR);
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
