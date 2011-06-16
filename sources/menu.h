#ifndef _MENU_H_
#define _MENU_H_
int getListItemId(unsigned int intCurrentItemId,int intMoveStep,unsigned int intListItemsCount);
void printListItem(const char *strItemValue,const char *strListLabel,unsigned int intCurrentItemId,unsigned int intListItemsCount,bool blnHighLightItem,char *strPrependString,char *strAppendString);
#endif
