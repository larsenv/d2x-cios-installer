#ifndef _DEBUG_H_
#define _DEBUG_H_
enum DEBUG_MESSAGES {
    NORMAL_DEBUG_MESSAGE=0,
    ERROR_DEBUG_MESSAGE=1
};
void printDebugMsg(enum DEBUG_MESSAGES MESSAGE_TYPE,const char *fmt, ...);
#endif
