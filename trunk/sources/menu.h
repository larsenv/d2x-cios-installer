#ifndef _MENU_H_
#define _MENU_H_
int getListItemId(unsigned int intCurrentItemId,int intMoveStep,unsigned int intListItemsCount);
void printListItem(u8 chRow,u8 chColumn,const char *strItemValue,const char *strListLabel,unsigned int intCurrentItemId,unsigned int intListItemsCount,enum CONSOLE_FONT_COLORS HIGHLIGHT_ITEM_COLOR,char *strPrependString,char *strAppendString,unsigned char chPaddingMaxColumn);
#endif
