#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_
#define CONTROLLERS_COUNT 3
#include <gctypes.h>
enum CONTROLLERS {
    GAMECUBE_CONTROLLER=0,
    WIIMOTE_CONTROLLER=1,
    CLASSIC_CONTROLLER=2
};
s32 getMappedPadsKey(s32 intPadsKey);
s32 getUnmappedPadKey(s32 intMappedPadKey,enum CONTROLLERS CONTROLLER);
s32 getPadsKeyPressed(s32 *intExpectedPadsKeysPressed,unsigned int intExpectedPadsKeysCount,bool blnMappedPadsKey);
s32 waitPadsKeyPressed(const char *strMessage);
char *getPadKeyLabel(s32 intPadKey,enum CONTROLLERS CONTROLLER);
unsigned char getPadsKeyLabels(s32 intMappedPadKey,char **strPadsKeyLabels);
#endif
