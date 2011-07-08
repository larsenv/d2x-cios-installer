#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ogc/consol.h>
#include <ogc/gu.h>
#include "gui.h"
#include "libutils.h"
#include "libmath.h"
#include "libstring.h"
u8 getTextBoxRow(u8 chRow) {
    return (chRow==AUTOSIZE)?getConsoleRow():chRow;
}
u8 getTextBoxColumn(u8 chColumn) {
    return (chColumn==AUTOSIZE)?getConsoleColumn():chColumn;
}
u8 getTextBoxAutoWidth(enum BORDERS BORDER_TYPE,const char *strFormatValue,...) {
static char strTextBuffer[1024];
va_list pArguments;
unsigned intLinesCount,intMaxColumn;
    va_start(pArguments,strFormatValue);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strFormatValue,pArguments);
    va_end(pArguments);
    intLinesCount=getLinesCount(strTextBuffer,&intMaxColumn);
    return intMaxColumn+((BORDER_TYPE & RIGHT_BORDER)/RIGHT_BORDER)+((BORDER_TYPE & LEFT_BORDER)/LEFT_BORDER);
}
u8 getTextBoxAutoHeight(enum BORDERS BORDER_TYPE,const char *strFormatValue,...) {
static char strTextBuffer[1024];
va_list pArguments;
unsigned intMaxColumn;
    va_start(pArguments,strFormatValue);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strFormatValue,pArguments);
    va_end(pArguments);
    return getLinesCount(strTextBuffer,&intMaxColumn)+((BORDER_TYPE & TOP_BORDER)/TOP_BORDER)+((BORDER_TYPE & BOTTOM_BORDER)/BOTTOM_BORDER);
}
void getTextBoxPositions(u8 *chMinRow,u8 *chMinColumn,u8 *chMaxRow,u8 *chMaxColumn,enum BORDERS BORDER_TYPE,const char *strFormatValue,...) {
static char strTextBuffer[1024];
va_list pArguments;
    va_start(pArguments,strFormatValue);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strFormatValue,pArguments);
    va_end(pArguments);
    *chMinRow=getTextBoxRow(*chMinRow);
    *chMinColumn=getTextBoxColumn(*chMinColumn);
    if (*chMaxRow==AUTOSIZE) {
        *chMaxRow=*chMinRow+getTextBoxAutoHeight(BORDER_TYPE,strTextBuffer)-1;
    }
    if (*chMaxColumn==AUTOSIZE) {
        *chMaxColumn=*chMinColumn+getTextBoxAutoWidth(BORDER_TYPE,strTextBuffer)-1;
    }
    if (*chMinColumn>*chMaxColumn) {
        permutePointers((void *) chMinColumn,(void *) chMaxColumn);
    }
    if (*chMinRow>*chMaxRow) {
        permutePointers((void *) chMinRow,(void *) chMaxRow);
    }
}
void printLocatedText(u8 chRow,u8 chColumn,const char *strTexteFormat,...) {
static char strTextBuffer[1024];
va_list pArguments;
    setCursorPosition(getTextBoxRow(chRow),getTextBoxColumn(chColumn));
    va_start(pArguments,strTexteFormat);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strTexteFormat,pArguments);
    va_end(pArguments);
    printf("%s",strTextBuffer);
    saveCursorPosition();
}
void printTextInColumnsRange(u8 chRow,u8 chColumn,u8 chMinColumn,u8 chMaxColumn,const char *strTexteFormat,...) {
static char strTextBuffer[1024];
va_list pArguments;
     if (chMinColumn>chMaxColumn) {
        permutePointers((void *) &chMinColumn,(void *) &chMaxColumn);
    }
    va_start(pArguments,strTexteFormat);
    if (vsnprintf(strTextBuffer,sizeof(strTextBuffer),strTexteFormat,pArguments)) {
        if (isInRange(chColumn,(double) chMinColumn-strlen(strTextBuffer)+1,chMaxColumn,true,true)) {
            if (chColumn>chMinColumn) {
                strTextBuffer[1+chMaxColumn-(chColumn-chMinColumn)]=0;
                printLocatedText(chRow,chColumn,strTextBuffer);
            }
            else {
                printLocatedText(chRow,chColumn,&strTextBuffer[chMinColumn-chColumn]);
            }
            resetSavedPreviousCursorPosition();
        }
    }
    va_end(pArguments);
}
void printAlignedText(double dbHorizontalAlign,double dbVerticalAlign,u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,bool blnMultiLine,bool blnHideOverflowText,const char *strTexteFormat,...) {
static char strTextBuffer[1024];
va_list pArguments;
u8 intFreeColumsCount,intFreeRowsCount;
unsigned int intBreakLinesCount;
char **strBreakLines;
double dbTextContainerX[2]={0,0},dbTextContainerY[2]={0,0};
    va_start(pArguments,strTexteFormat);
    if (vsnprintf(strTextBuffer,sizeof(strTextBuffer),strTexteFormat,pArguments)) {
        if (chMinColumn>chMaxColumn) {
            permutePointers((void *) &chMinColumn,(void *) &chMaxColumn);
        }
        if (chMinRow>chMaxRow) {
            permutePointers((void *) &chMinRow,(void *) &chMaxRow);
        }
        intFreeColumsCount=chMaxColumn-chMinColumn+1;
        intFreeRowsCount=chMaxRow-chMinRow+1;
        dbTextContainerX[1]=strlen(strTextBuffer)-1;
        if (blnMultiLine) {
            intBreakLinesCount=intFreeRowsCount;
            strBreakLines=getBreakStrings(strTextBuffer,' ',intFreeColumsCount-1,&intBreakLinesCount);
            if (strBreakLines) {
                dbTextContainerY[1]=intBreakLinesCount-1;
                dbHorizontalAlign=getRoundNumber(getPolyContainerPosition(&dbTextContainerX[0],2,chMinColumn,chMaxColumn,dbHorizontalAlign));
                dbVerticalAlign=getRoundNumber(getPolyContainerPosition(&dbTextContainerY[0],2,chMinRow,chMaxRow,dbVerticalAlign));
                if (blnHideOverflowText) {
                    while ((intBreakLinesCount) && (isInRange(dbVerticalAlign,chMinRow,chMaxRow,true,true))) {
                        printTextInColumnsRange(dbVerticalAlign,dbHorizontalAlign,chMinColumn,chMaxColumn,strTextBuffer);
                        dbVerticalAlign++;
                        intBreakLinesCount--;
                    }
                }
                else {
                    while (intBreakLinesCount) {
                        printLocatedText(dbVerticalAlign,dbHorizontalAlign,strTextBuffer);
                        resetSavedPreviousCursorPosition();
                        dbVerticalAlign++;
                        intBreakLinesCount--;
                    }
                }
                free(strBreakLines);
            }
        }
        else {
            dbHorizontalAlign=getRoundNumber(getPolyContainerPosition(&dbTextContainerX[0],2,chMinColumn,chMaxColumn,dbHorizontalAlign));
            dbVerticalAlign=getRoundNumber(getPolyContainerPosition(&dbTextContainerY[0],2,chMinRow,chMaxRow,dbVerticalAlign));
            if (blnHideOverflowText) {
                if (isInRange(dbVerticalAlign,chMinRow,chMaxRow,true,true)) {
                    printTextInColumnsRange(dbVerticalAlign,dbHorizontalAlign,chMinColumn,chMaxColumn,strTextBuffer);
                }
            }
            else {
                printLocatedText(dbVerticalAlign,dbHorizontalAlign,strTextBuffer);
                resetSavedPreviousCursorPosition();
            }
        }
    }
    va_end(pArguments);
}
void printStyledText(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *fmt, ...) {
static char strTextBuffer[1024];
va_list pArguments;
    setFontStyle(FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT);
    va_start(pArguments,fmt);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),fmt,pArguments);
    va_end(pArguments);
    printLocatedText(chRow,chColumn,strTextBuffer);
    resetPreviousFontStyle();
}
void printAlignedStyledText(double dbColumn,double dbRow,u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strTexteFormat,...) {
static char strTextBuffer[1024];
va_list pArguments;
    va_start(pArguments,strTexteFormat);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strTexteFormat,pArguments);
    va_end(pArguments);
    setFontStyle(FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT);
    printAlignedText(dbColumn,dbRow,chMinRow,chMinColumn,chMaxRow,chMaxColumn,blnMultiLine,blnHideOverflowText,strTextBuffer);
    resetPreviousFontStyle();
}
void printFrame(u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum CONSOLE_FONT_COLORS FRAME_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION) {
static char strLeftBorder[2],strRightBorder[2];
unsigned char chLeftBorder=179*(BORDER_TYPE & LEFT_BORDER)/LEFT_BORDER,chRightBorder=179*(BORDER_TYPE & RIGHT_BORDER)/RIGHT_BORDER;
s16 i,intFrameColumnsCount,intMinRow,intMaxRow;
    if (chMinColumn>chMaxColumn) {
        permutePointers((void *) &chMinColumn,(void *) &chMaxColumn);
    }
    if (chMinRow>chMaxRow) {
        permutePointers((void *) &chMinRow,(void *) &chMaxRow);
    }
    intMinRow=chMinRow;
    intMaxRow=chMaxRow;
    setFontStyle(FRAME_COLOR,BORDER_COLOR,CONSOLE_FONT_NORMAL);
    intFrameColumnsCount=chMaxColumn-chMinColumn+1;
    saveCursorPosition();
    if (BORDER_TYPE & TOP_BORDER) {
        setCursorPosition(chMinRow,chMinColumn);
        printRepeatString(intFrameColumnsCount,"%c",196);
        if (chLeftBorder) {
            switch ((unsigned char) (TOP_LEFT_JUNCTION)) {
                case HORIZONTAL_JUNCTION:
                    printLocatedText(chMinRow,chMinColumn,"%c",194);
                    break;
                case VERTICAL_JUNCTION:
                    printLocatedText(chMinRow,chMinColumn,"%c",195);
                    break;
                case (HORIZONTAL_JUNCTION|VERTICAL_JUNCTION):
                    printLocatedText(chMinRow,chMinColumn,"%c",197);
                    break;
                default:
                    printLocatedText(chMinRow,chMinColumn,"%c",218);
            }
            resetSavedPreviousCursorPosition();
        }
        if (chRightBorder) {
            switch ((unsigned char) (TOP_LEFT_JUNCTION)) {
                case HORIZONTAL_JUNCTION:
                    printLocatedText(chMinRow,chMaxColumn,"%c",194);
                    break;
                case VERTICAL_JUNCTION:
                    printLocatedText(chMinRow,chMaxColumn,"%c",180);
                    break;
                case (HORIZONTAL_JUNCTION|VERTICAL_JUNCTION):
                    printLocatedText(chMinRow,chMaxColumn,"%c",197);
                    break;
                default:
                    printLocatedText(chMinRow,chMaxColumn,"%c",191);
            }
            resetSavedPreviousCursorPosition();
        }
        intMinRow++;
    }
    if (BORDER_TYPE & BOTTOM_BORDER) {
        setCursorPosition(chMaxRow,chMinColumn);
        printRepeatString(intFrameColumnsCount,"%c",196);
        if (chLeftBorder) {
            switch ((unsigned char) (TOP_LEFT_JUNCTION)) {
                case HORIZONTAL_JUNCTION:
                    printLocatedText(chMaxRow,chMinColumn,"%c",193);
                    break;
                case VERTICAL_JUNCTION:
                    printLocatedText(chMaxRow,chMinColumn,"%c",195);
                    break;
                case (HORIZONTAL_JUNCTION|VERTICAL_JUNCTION):
                    printLocatedText(chMaxRow,chMinColumn,"%c",197);
                    break;
                default:
                    printLocatedText(chMaxRow,chMinColumn,"%c",192);
            }
            resetSavedPreviousCursorPosition();
        }
        if (chRightBorder) {
            switch ((unsigned char) (TOP_LEFT_JUNCTION)) {
                case HORIZONTAL_JUNCTION:
                    printLocatedText(chMaxRow,chMaxColumn,"%c",193);
                    break;
                case VERTICAL_JUNCTION:
                    printLocatedText(chMaxRow,chMaxColumn,"%c",180);
                    break;
                case (HORIZONTAL_JUNCTION|VERTICAL_JUNCTION):
                    printLocatedText(chMaxRow,chMaxColumn,"%c",197);
                    break;
                default:
                    printLocatedText(chMaxRow,chMaxColumn,"%c",217);
            }
            resetSavedPreviousCursorPosition();
        }
        intMaxRow--;
    }
    if ((intMaxRow-intMinRow+1)>0) {
        snprintf(strLeftBorder,sizeof(strLeftBorder),"%c",chLeftBorder);
        snprintf(strRightBorder,sizeof(strRightBorder),"%c",chRightBorder);
        intFrameColumnsCount=MAX(intFrameColumnsCount-strlen(strLeftBorder)-strlen(strRightBorder),0);
        for (i=intMinRow;i<=intMaxRow;i++) {
            setCursorPosition(i,chMinColumn);
            printf("%s%*s%s",strLeftBorder,intFrameColumnsCount,"",strRightBorder);
        }
    }
    resetSavedCursorPosition();
    resetPreviousFontStyle();
}
void printTextBox(u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,double dbHorizontalAlign,double dbVerticalAlign,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION,enum CONSOLE_FONT_COLORS BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strFormatValue,...) {
static char strTextBuffer[1024];
int intMinRow,intMaxRow,intMinColumn,intMaxColumn;
va_list pArguments;
    va_start(pArguments,strFormatValue);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strFormatValue,pArguments);
    va_end(pArguments);
    getTextBoxPositions(&chMinRow,&chMinColumn,&chMaxRow,&chMaxColumn,BORDER_TYPE,strTextBuffer);
    intMinRow=chMinRow+(BORDER_TYPE & TOP_BORDER)/TOP_BORDER;
    intMaxRow=chMaxRow-(BORDER_TYPE & BOTTOM_BORDER)/BOTTOM_BORDER;
    intMinColumn=chMinColumn+(BORDER_TYPE & LEFT_BORDER)/LEFT_BORDER;
    intMaxColumn=chMaxColumn-(BORDER_TYPE & RIGHT_BORDER)/RIGHT_BORDER;
    printFrame(chMinRow,chMinColumn,chMaxRow,chMaxColumn,BORDER_TYPE,BORDER_COLOR,BGCOLOR,TOP_LEFT_JUNCTION,RIGHT_LEFT_JUNCTION,BOTTOM_LEFT_JUNCTION,BOTTOM_RIGHT_JUNCTION);
    if ((intMaxColumn-intMinColumn+1>0) && (intMaxRow-intMinRow+1>0)) {
        printAlignedStyledText(dbHorizontalAlign,dbVerticalAlign,intMinRow,intMinColumn,intMaxRow,intMaxColumn,BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT,blnMultiLine,blnHideOverflowText,strTextBuffer);
    }
}
void printAlignedTextBox(u8 chFrameMinRow,u8 chFrameMinColumn,u8 chFrameMaxRow,u8 chFrameMaxColumn,double dbRow,double dbColumn,double dbHeight,double dbWidth,double dbHorizontalAlign,double dbVerticalAlign,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION,enum CONSOLE_FONT_COLORS BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strFormatValue,...) {
static char strTextBuffer[1024];
va_list pArguments;
double dbTextContainerX[2]={dbColumn,dbColumn},dbTextContainerY[2]={dbRow,dbRow};
    va_start(pArguments,strFormatValue);
    vsnprintf(strTextBuffer,sizeof(strTextBuffer),strFormatValue,pArguments);
    va_end(pArguments);
    if (chFrameMinRow>chFrameMaxRow) {
        permutePointers((void *) &chFrameMinRow,(void *) &chFrameMaxRow);
    }
    if (chFrameMinColumn>chFrameMaxColumn) {
        permutePointers((void *) &chFrameMinColumn,(void *) &chFrameMaxColumn);
    }
    dbWidth=fabs(dbWidth);
    dbHeight=fabs(dbHeight);
    if (dbWidth<=1) {
        dbWidth=(double) (chFrameMaxColumn-chFrameMinColumn+1)*dbWidth;
        dbTextContainerX[0]=0;
        dbTextContainerX[1]=dbWidth;
    }
    if (dbHeight<=1) {
        dbHeight=(double) (chFrameMaxRow-chFrameMinRow+1)*dbHeight;
        dbTextContainerY[0]=0;
        dbTextContainerY[1]=dbHeight;
    }
    if (dbWidth==AUTOSIZE) {
        dbWidth=getTextBoxAutoWidth(BORDER_TYPE,strTextBuffer);
        dbTextContainerX[0]=0;
        dbTextContainerX[1]=dbWidth;
    }
    if (dbHeight==AUTOSIZE) {
        dbHeight=getTextBoxAutoHeight(BORDER_TYPE,strTextBuffer);
        dbTextContainerY[0]=0;
        dbTextContainerY[1]=dbHeight;
    }
    dbColumn=getRoundNumber(getPolyContainerPosition(&dbTextContainerX[0],2,chFrameMinColumn,chFrameMaxColumn,dbColumn));
    dbRow=getRoundNumber(getPolyContainerPosition(&dbTextContainerY[0],2,chFrameMinRow,chFrameMaxRow,dbRow));
    printTextBox(dbRow,dbColumn,getRoundNumber(dbRow+dbHeight-1),getRoundNumber(dbColumn+dbWidth-1),dbHorizontalAlign,dbVerticalAlign,BORDER_TYPE,BORDER_COLOR,TOP_LEFT_JUNCTION,RIGHT_LEFT_JUNCTION,BOTTOM_LEFT_JUNCTION,BOTTOM_RIGHT_JUNCTION,BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT,blnMultiLine,blnHideOverflowText,strTextBuffer);
}
void drawLabel(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *strLabelCaption,u8 chLabelSize,struct stLabel *stLabelSettings) {
    printStyledText(chRow,chColumn,FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT,strLabelCaption);
    CON_GetPosition(&stLabelSettings->stLabelLocation.intColumn,&stLabelSettings->stLabelLocation.intRow);
    stLabelSettings->chLabelSize=chLabelSize;
    printf("%*s",(unsigned int) stLabelSettings->chLabelSize,"");
}
void printLabel(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,unsigned char chLabelSize,const char *strFormatLabel,...) {
static char strLabelText[256];
va_list pArguments;
unsigned char chLabelLength;
    saveCursorPosition();
    va_start(pArguments,strFormatLabel);
    vsnprintf(strLabelText,chLabelSize+1,strFormatLabel,pArguments);
    va_end(pArguments);
    setCursorPosition(getTextBoxRow(chRow),getTextBoxColumn(chColumn));
    setFontStyle(FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT);
    printf("%.*s",(unsigned int) chLabelSize,strLabelText);
    resetPreviousFontStyle();
    chLabelLength=strlen(strLabelText);
    printf("%*s",(unsigned int) ((chLabelLength>chLabelSize)?0:chLabelSize-chLabelLength),"");
    resetSavedCursorPosition();
}
void setProgressBarSettings(struct stProgressBar *stProgressBarSettings,u8 chProgressBarTextSize,u8 chProgressBarTextColumn,u8 chProgressBarTextRow,enum CONSOLE_FONT_COLORS PROGRESSCOLOR,unsigned int intOperationsCount) {
    stProgressBarSettings->chProgressBarTextSize=chProgressBarTextSize;
    stProgressBarSettings->stProgressBarTextLocation.intColumn=chProgressBarTextColumn;
    stProgressBarSettings->stProgressBarTextLocation.intRow=chProgressBarTextRow;
    stProgressBarSettings->PROGRESSBAR_COLOR=PROGRESSCOLOR;
    stProgressBarSettings->intOperationsCount=intOperationsCount;
}
void drawProgressBar(u8 chProgressBarRow,u8 chProgressBarColumn,u8 chProgressBarSize,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,enum CONSOLE_FONT_COLORS PROGRESS_BAR_BGCOLOR,struct stProgressBar *stProgressBarSettings,const char *strFormatLabel,...) {
static char strLabel[51];
va_list pArguments;
    va_start(pArguments,strFormatLabel);
    vsnprintf(strLabel,sizeof(strLabel),strFormatLabel,pArguments);
    va_end(pArguments);
    printStyledText(chProgressBarRow,chProgressBarColumn,FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT,"%s",strLabel);
    printf(" ");
    CON_GetPosition(&stProgressBarSettings->stProgressBarLocation.intColumn,&stProgressBarSettings->stProgressBarLocation.intRow);
    setFontBgColor(PROGRESS_BAR_BGCOLOR);
    stProgressBarSettings->chProgressBarSize=chProgressBarSize;
    printf("%*s",(unsigned int) chProgressBarSize,"");
    resetPreviousBgColor();
    stProgressBarSettings->intValue=0;
}
void updateProgressBar(struct stProgressBar *stProgressBarSettings,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *strFormatProgressBarText,...) {
static char strProgressBarText[256];
va_list pArguments;
    if (stProgressBarSettings->intValue<stProgressBarSettings->intOperationsCount) {
        saveCursorPosition();
        stProgressBarSettings->intValue=stProgressBarSettings->intValue+1;
        setCursorPosition(stProgressBarSettings->stProgressBarLocation.intRow,stProgressBarSettings->stProgressBarLocation.intColumn);
        setFontBgColor(stProgressBarSettings->PROGRESSBAR_COLOR);
        printf("%*s",(unsigned int) getRoundNumber((double)stProgressBarSettings->intValue*(double) stProgressBarSettings->chProgressBarSize/(double) stProgressBarSettings->intOperationsCount),"");
        resetPreviousBgColor();
        resetSavedCursorPosition();
        va_start(pArguments,strFormatProgressBarText);
        vsnprintf(strProgressBarText,stProgressBarSettings->chProgressBarTextSize+1,strFormatProgressBarText,pArguments);
        va_end(pArguments);
        printLabel(stProgressBarSettings->stProgressBarTextLocation.intRow,stProgressBarSettings->stProgressBarTextLocation.intColumn,FONT_BGCOLOR,FONT_FGCOLOR,FONT_WEIGHT,stProgressBarSettings->chProgressBarTextSize,strProgressBarText);
    }
}
void printLevelsBar(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS LEVELS_BAR_COLOR,enum CONSOLE_FONT_COLORS ACTIVE_LEVEL_COLOR,unsigned char chActiveLevelIndex,unsigned char chLevelsCount,const char *strFirstLevel,...) {
va_list pArguments;
unsigned short int intLevelIndex=0;
const char *strCurrentLevel=strFirstLevel;
    va_start(pArguments,strFirstLevel);
    setFontFgColor((intLevelIndex==chActiveLevelIndex)?ACTIVE_LEVEL_COLOR:LEVELS_BAR_COLOR,CONSOLE_FONT_BOLD);
    printLocatedText(chRow,chColumn,"%s",strCurrentLevel);
    resetPreviousFgColor();
    intLevelIndex++;
    while (intLevelIndex<chLevelsCount) {
        strCurrentLevel=va_arg(pArguments,const char *);
        setFontFgColor(LEVELS_BAR_COLOR,CONSOLE_FONT_BOLD);
        printf(" > ");
        resetPreviousFgColor();
        setFontFgColor((intLevelIndex==chActiveLevelIndex)?ACTIVE_LEVEL_COLOR:LEVELS_BAR_COLOR,CONSOLE_FONT_BOLD);
        printf("%s",strCurrentLevel);
        resetPreviousFgColor();
        intLevelIndex++;
    }
    va_end(pArguments);
}
