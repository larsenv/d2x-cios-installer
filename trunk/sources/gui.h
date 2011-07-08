#ifndef _GUI_H_
#define _GUI_H_
#include <gctypes.h>
#include "video.h"
#include "libgeometry.h"
#define AUTOSIZE 255
struct stProgressBar {
    unsigned char chProgressBarTextSize;
    unsigned int intOperationsCount;
    unsigned int intValue;
    unsigned char chProgressBarSize;
    enum CONSOLE_FONT_COLORS PROGRESSBAR_COLOR;
    struct stConsoleCursorLocation stProgressBarLocation;
    struct stConsoleCursorLocation stProgressBarTextLocation;
};
struct stLabel {
    unsigned char chLabelSize;
    struct stConsoleCursorLocation stLabelLocation;
};
enum BORDERS {
    NO_BORDER=0,
    LEFT_BORDER=1,
    TOP_BORDER=2,
    RIGHT_BORDER=4,
    BOTTOM_BORDER=8
};
enum FRAME_JUNCTIONS {
    NO_JUNCTION=0,
    HORIZONTAL_JUNCTION=1,
    VERTICAL_JUNCTION=2,
};
u8 getTextBoxRow(u8 chRow);
u8 getTextBoxColumn(u8 chColumn);
u8 getTextBoxAutoWidth(enum BORDERS BORDER_TYPE,const char *strFormatValue,...);
u8 getTextBoxAutoHeight(enum BORDERS BORDER_TYPE,const char *strFormatValue,...);
void getTextBoxPositions(u8 *chMinRow,u8 *chMinColumn,u8 *chMaxRow,u8 *chMaxColumn,enum BORDERS BORDER_TYPE,const char *strFormatValue,...);
void printLocatedText(u8 chRow,u8 chColumn,const char *strTexteFormat,...);
void printTextInColumnsRange(u8 chRow,u8 chColumn,u8 chMinColumn,u8 chMaxColumn,const char *strTexteFormat,...);
void printAlignedText(double dbColumn,double dbRow,u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,bool blnMultiLine,bool blnHideOverflowText,const char *strTexteFormat,...);
void printStyledText(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *fmt, ...);
void printAlignedStyledText(double dbColumn,double dbRow,u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strTexteFormat,...);
void printFrame(u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum CONSOLE_FONT_COLORS FRAME_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION);
void printTextBox(u8 chMinRow,u8 chMinColumn,u8 chMaxRow,u8 chMaxColumn,double dbHorizontalAlign,double dbVerticalAlign,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION,enum CONSOLE_FONT_COLORS BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strFormatValue,...);
void printAlignedTextBox(u8 chFrameMinRow,u8 chFrameMinColumn,u8 chFrameMaxRow,u8 chFrameMaxColumn,double dbRow,double dbColumn,double dbHeight,double dbWidth,double dbHorizontalAlign,double dbVerticalAlign,enum BORDERS BORDER_TYPE,enum CONSOLE_FONT_COLORS BORDER_COLOR,enum FRAME_JUNCTIONS TOP_LEFT_JUNCTION,enum FRAME_JUNCTIONS RIGHT_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_LEFT_JUNCTION,enum FRAME_JUNCTIONS BOTTOM_RIGHT_JUNCTION,enum CONSOLE_FONT_COLORS BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,bool blnMultiLine,bool blnHideOverflowText,const char *strFormatValue,...);
void drawLabel(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *strLabelCaption,u8 chLabelSize,struct stLabel *stLabelSettings);
void printLabel(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,unsigned char chLabelSize,const char *strFormatLabel,...);
void setProgressBarSettings(struct stProgressBar *stProgressBarSettings,u8 chProgressBarTextSize,u8 chProgressBarTextColumn,u8 chProgressBarTextRow,enum CONSOLE_FONT_COLORS PROGRESSCOLOR,unsigned int intOperationsCount);
void drawProgressBar(u8 chProgressBarRow,u8 chProgressBarColumn,u8 chProgressBarSize,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,enum CONSOLE_FONT_COLORS PROGRESS_BAR_BGCOLOR,struct stProgressBar *stProgressBarSettings,const char *strFormatLabel,...);
void updateProgressBar(struct stProgressBar *stProgressBarSettings,enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT,const char *strFormatProgressBarText,...);
void printLevelsBar(u8 chRow,u8 chColumn,enum CONSOLE_FONT_COLORS LEVELS_BAR_COLOR,enum CONSOLE_FONT_COLORS ACTIVE_LEVEL_COLOR,unsigned char chActiveLevelIndex,unsigned char chLevelsCount,const char *strFirstLevel,...);
#endif
