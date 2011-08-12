#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "video.h"
#include "d2x-cios-installer.h"
void printDebugMsg(enum DEBUG_MESSAGES MESSAGE_TYPE,const char *fmt, ...) {
static char strDebugMsg[1024];
va_list pArguments;
    va_start(pArguments,fmt);
    vsnprintf(strDebugMsg,sizeof(strDebugMsg),fmt,pArguments);
    va_end(pArguments);
    switch(MESSAGE_TYPE) {
        case ERROR_DEBUG_MESSAGE:
            setFontStyle(CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_NORMAL);
            printf("%s",strDebugMsg);
            break;
        default:
            setFontStyle(DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_NORMAL);
            printf("%s",strDebugMsg);
    }
    resetPreviousFontStyle();

}
