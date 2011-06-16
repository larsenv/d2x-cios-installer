#include <stdio.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <unistd.h>
#include "controllers.h"
#include "libarray.h"
#include "system.h"
s32 getMappedPadsKey(s32 intPadsKey) {
s32 varout=0;
    switch(intPadsKey) {
        case WPAD_BUTTON_LEFT:
        case WPAD_CLASSIC_BUTTON_LEFT:
            varout=PAD_BUTTON_LEFT;
            break;
        case WPAD_BUTTON_RIGHT:
        case WPAD_CLASSIC_BUTTON_RIGHT:
            varout=PAD_BUTTON_RIGHT;
            break;
        case WPAD_BUTTON_DOWN:
        case WPAD_CLASSIC_BUTTON_DOWN:
            varout=PAD_BUTTON_DOWN;
            break;
        case WPAD_BUTTON_UP:
        case WPAD_CLASSIC_BUTTON_UP:
            varout=PAD_BUTTON_UP;
            break;
        case WPAD_CLASSIC_BUTTON_ZR:
            varout=PAD_TRIGGER_Z;
            break;
        case WPAD_BUTTON_PLUS:
        case WPAD_CLASSIC_BUTTON_PLUS:
            varout=PAD_TRIGGER_R;
            break;
        case WPAD_BUTTON_MINUS:
        case WPAD_CLASSIC_BUTTON_MINUS:
            varout=PAD_TRIGGER_L;
            break;
        case WPAD_BUTTON_A:
        case WPAD_CLASSIC_BUTTON_A:
            varout=PAD_BUTTON_A;
            break;
        case WPAD_BUTTON_B:
        case WPAD_CLASSIC_BUTTON_B:
            varout=PAD_BUTTON_B;
            break;
        case WPAD_BUTTON_1:
        case WPAD_CLASSIC_BUTTON_Y:
            varout=PAD_BUTTON_Y;
            break;
        case WPAD_BUTTON_2:
        case WPAD_CLASSIC_BUTTON_X:
            varout=PAD_BUTTON_X;
            break;
        case WPAD_CLASSIC_BUTTON_ZL:
            varout=PAD_BUTTON_MENU;
            break;
        case WPAD_BUTTON_HOME:
        case WPAD_CLASSIC_BUTTON_HOME:
            varout=PAD_BUTTON_START;
            break;
    }
    return varout;
}
s32 getPadsKeyPressed(s32 *intExpectedPadsKeysPressed,unsigned int intExpectedPadsKeysCount,bool blnMappedPadsKey) {
s32 varout=0;
bool blnWiiPadKeyPressed;
    while (!varout) {
        blnWiiPadKeyPressed=false;
        while (!varout) {
            WPAD_ScanPads();
            if ((varout=(WPAD_ButtonsDown(WPAD_CHAN_0)|WPAD_ButtonsDown(WPAD_CHAN_1)|WPAD_ButtonsDown(WPAD_CHAN_2)|WPAD_ButtonsDown(WPAD_CHAN_3)))) {
                varout=(WPAD_ButtonsHeld(WPAD_CHAN_0)|WPAD_ButtonsHeld(WPAD_CHAN_1)|WPAD_ButtonsHeld(WPAD_CHAN_2)|WPAD_ButtonsHeld(WPAD_CHAN_3));
                blnWiiPadKeyPressed=true;
            }
            else {
                PAD_ScanPads();
                if ((varout=(PAD_ButtonsDown(PAD_CHAN0)|PAD_ButtonsDown(PAD_CHAN1)|PAD_ButtonsDown(PAD_CHAN2)|PAD_ButtonsDown(PAD_CHAN3)))) {
                    varout=(PAD_ButtonsHeld(PAD_CHAN0)|PAD_ButtonsHeld(PAD_CHAN1)|PAD_ButtonsHeld(PAD_CHAN2)|PAD_ButtonsHeld(PAD_CHAN3));
                }
                usleep(20000);
            }
            if (getUserResetRequestStatus()) {
                return 0;
            }
        }
        if (blnMappedPadsKey && blnWiiPadKeyPressed) {
            varout=getMappedPadsKey(varout);
        }
        if (intExpectedPadsKeysPressed!=NULL) {
            if (!inArray((void *) intExpectedPadsKeysPressed,intExpectedPadsKeysCount,sizeof(s32),(void *) &varout)) {
                varout=0;
            }
        }
    }
    return varout;
}
s32 waitPadsKeyPressed(const char *strMessage) {
s32 varout;
    printf("%s",strMessage);
    varout=getPadsKeyPressed(NULL,0,false);
    return varout;
}
