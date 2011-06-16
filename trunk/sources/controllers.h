#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_
#include <gctypes.h>
s32 getMappedPadsKey(s32 intPadsKey);
s32 getPadsKeyPressed(s32 *intExpectedPadsKeysPressed,unsigned int intExpectedPadsKeysCount,bool blnMappedPadsKey);
s32 waitPadsKeyPressed(const char *strMessage);
#endif
