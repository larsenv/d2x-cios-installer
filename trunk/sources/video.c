#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <ogc/color.h>
#include <ogc/consol.h>
#include <ogc/video.h>
#include <ogc/system.h>
#include "image.h"
#include "pngu.h"
#include "video.h"
#include "d2x-cios-installer.h"
#include "libmath.h"
enum CONSOLE_FONT_COLORS CURRENT_FONT_BGCOLOR=DEFAULT_FONT_BGCOLOR,PREVIOUS_FONT_BGCOLOR=DEFAULT_FONT_BGCOLOR,CURRENT_FONT_FGCOLOR=DEFAULT_FONT_FGCOLOR,PREVIOUS_FONT_FGCOLOR=DEFAULT_FONT_FGCOLOR;
enum CONSOLE_FONT_WEIGHTS PREVIOUS_FONT_WEIGHT=DEFAULT_FONT_WEIGHT,CURRENT_FONT_WEIGHT=DEFAULT_FONT_WEIGHT;
void clearConsole() {
	printf("\x1b[2J");
	fflush(stdout);
}
void clearConsoleLine() {
	printf("\r\x1b[2K\r");
	fflush(stdout);
}
void setFontFgColor(enum CONSOLE_FONT_COLORS FONT_COLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT) {
    PREVIOUS_FONT_FGCOLOR=CURRENT_FONT_FGCOLOR;
    PREVIOUS_FONT_WEIGHT=CURRENT_FONT_WEIGHT;
	printf("\x1b[%u;%um",(u8) FONT_COLOR+30,FONT_WEIGHT);
	CURRENT_FONT_FGCOLOR=FONT_COLOR;
    CURRENT_FONT_WEIGHT=FONT_WEIGHT;
}
void setFontBgColor(enum CONSOLE_FONT_COLORS FONT_COLOR) {
	PREVIOUS_FONT_BGCOLOR=CURRENT_FONT_BGCOLOR;
	printf("\x1b[%um",(u8) FONT_COLOR+40);
	CURRENT_FONT_BGCOLOR=FONT_COLOR;
}
void setFontStyle(enum CONSOLE_FONT_COLORS FONT_BGCOLOR,enum CONSOLE_FONT_COLORS FONT_FGCOLOR,enum CONSOLE_FONT_WEIGHTS FONT_WEIGHT) {
    setFontBgColor(FONT_BGCOLOR);
    setFontFgColor(FONT_FGCOLOR,FONT_WEIGHT);
}
void resetDefaultFontSyle() {
    setFontStyle(DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT);
}
void resetPreviousFgColor() {
    setFontFgColor(PREVIOUS_FONT_FGCOLOR,PREVIOUS_FONT_WEIGHT);
}
void resetPreviousBgColor() {
    setFontBgColor(PREVIOUS_FONT_BGCOLOR);
}
void resetPreviousFontStyle() {
    setFontStyle(PREVIOUS_FONT_BGCOLOR,PREVIOUS_FONT_FGCOLOR,PREVIOUS_FONT_WEIGHT);
}
void initConsole(const void *imgBgData,enum CONSOLE_COLORS CONSOLE_COLOR,double dbLeft,double dbTop,double dbWidth,double dbHeight) {
PNGUPROP imgProperties;
IMGCTX imgContext;
void *pFramebuffer=NULL;
GXRModeObj *pRmode=NULL;
double dbConsoleFrameX[2]={dbLeft,dbLeft},dbConsoleFrameY[2]={dbTop,dbTop},dbBgImgXScaleFactor=1,dbBgImgYScaleFactor=1,dbReferenceWidth,dbReferenceHeight;
    VIDEO_Init();
    pRmode=VIDEO_GetPreferredMode(NULL);
    pFramebuffer=MEM_K0_TO_K1(SYS_AllocateFramebuffer(pRmode));
    VIDEO_ClearFrameBuffer(pRmode,pFramebuffer,CONSOLE_COLOR);
    VIDEO_Configure(pRmode);
    VIDEO_SetNextFramebuffer(pFramebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(pRmode->viTVMode&VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }
    dbWidth=fabs(dbWidth);
    dbHeight=fabs(dbHeight);
    if ((imgContext=getPngImageRessources(imgBgData,&imgProperties))) {
        dbReferenceWidth=imgProperties.imgWidth;
        dbReferenceHeight=imgProperties.imgHeight;
        dbBgImgXScaleFactor=pRmode->fbWidth/imgProperties.imgWidth;
        dbBgImgYScaleFactor=pRmode->xfbHeight/imgProperties.imgHeight;
    }
    else {
        dbReferenceWidth=pRmode->fbWidth;
        dbReferenceHeight=pRmode->xfbHeight;
    }
    if (dbWidth<=1) {
        dbWidth=dbReferenceWidth*dbWidth;
        dbConsoleFrameX[0]=0;
        dbConsoleFrameX[1]=dbWidth;
    }
    if (dbHeight<=1) {
        dbHeight=dbReferenceHeight*dbHeight;
        dbConsoleFrameY[0]=0;
        dbConsoleFrameY[1]=dbHeight;
    }
    CON_InitEx(pRmode,(s32) getRoundNumber(getPolyContainerPosition(&dbConsoleFrameX[0],2,0,dbReferenceWidth,dbLeft)*dbBgImgXScaleFactor),(s32) getRoundNumber(getPolyContainerPosition(&dbConsoleFrameY[0],2,0,dbReferenceHeight,dbTop)*dbBgImgYScaleFactor),(s32) getRoundNumber(dbWidth*dbBgImgXScaleFactor),(s32) getRoundNumber(dbHeight*dbBgImgYScaleFactor));
    if (imgContext) {
        PNGU_DECODE_TO_COORDS_YCbYCr(imgContext,0,0,imgProperties.imgWidth,imgProperties.imgHeight,pRmode->fbWidth,pRmode->xfbHeight,pFramebuffer);
        PNGU_ReleaseImageContext(imgContext);
    }
    resetDefaultFontSyle();
}
void setCursorPosition(u8 intRow,u8 intColumn) {
    printf("\x1b[%u;%uH",intRow,intColumn);
}
int getConsoleColumnsCount() {
int intConsoleColumnsCount,intConsoleRowsCount;
    CON_GetMetrics(&intConsoleColumnsCount,&intConsoleRowsCount);
    return intConsoleColumnsCount;
}
int getConsoleRowsCount() {
int intConsoleColumnsCount,intConsoleRowsCount;
    CON_GetMetrics(&intConsoleColumnsCount,&intConsoleRowsCount);
    return intConsoleRowsCount;
}
int getConsoleColumn() {
int intConsoleColumn,intConsoleRow;
    CON_GetPosition(&intConsoleColumn,&intConsoleRow);
    return intConsoleColumn;
}
int getConsoleRow() {
int intConsoleColumn,intConsoleRow;
    CON_GetPosition(&intConsoleColumn,&intConsoleRow);
    return intConsoleRow;
}
