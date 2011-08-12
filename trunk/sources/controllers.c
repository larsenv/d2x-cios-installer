#include <stdio.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <unistd.h>
#include "controllers.h"
#include "libarray.h"
#include "libstring.h"
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
s32 getUnmappedPadKey(s32 intMappedPadKey,enum CONTROLLERS CONTROLLER) {
s32 varout=0;
    switch(CONTROLLER) {
        case GAMECUBE_CONTROLLER:
            varout=intMappedPadKey;
            break;
        case WIIMOTE_CONTROLLER:
            switch(intMappedPadKey) {
                case PAD_BUTTON_LEFT:
                    varout=WPAD_BUTTON_LEFT;
                    break;
                case PAD_BUTTON_RIGHT:
                    varout=WPAD_BUTTON_RIGHT;
                    break;
                case PAD_BUTTON_DOWN:
                    varout=WPAD_BUTTON_DOWN;
                    break;
                case PAD_BUTTON_UP:
                    varout=WPAD_BUTTON_UP;
                    break;
                case PAD_TRIGGER_R:
                    varout=WPAD_BUTTON_PLUS;
                    break;
                case PAD_TRIGGER_L:
                    varout=WPAD_BUTTON_MINUS;
                    break;
                case PAD_BUTTON_A:
                    varout=WPAD_BUTTON_A;
                    break;
                case PAD_BUTTON_B:
                    varout=WPAD_BUTTON_B;
                    break;
                case PAD_BUTTON_Y:
                    varout=WPAD_BUTTON_1;
                    break;
                case PAD_BUTTON_X:
                    varout=WPAD_BUTTON_2;
                    break;
                case PAD_BUTTON_START:
                    varout=WPAD_BUTTON_HOME;
                    break;
            }
            break;
        case CLASSIC_CONTROLLER:
            switch(intMappedPadKey) {
                case PAD_BUTTON_LEFT:
                    varout=WPAD_CLASSIC_BUTTON_LEFT;
                    break;
                case PAD_BUTTON_RIGHT:
                    varout=WPAD_CLASSIC_BUTTON_RIGHT;
                    break;
                case PAD_BUTTON_DOWN:
                    varout=WPAD_CLASSIC_BUTTON_DOWN;
                    break;
                case PAD_BUTTON_UP:
                    varout=WPAD_CLASSIC_BUTTON_UP;
                    break;
                case PAD_TRIGGER_Z:
                    varout=WPAD_CLASSIC_BUTTON_ZR;
                    break;
                case PAD_TRIGGER_R:
                    varout=WPAD_CLASSIC_BUTTON_PLUS;
                    break;
                case PAD_TRIGGER_L:
                    varout=WPAD_CLASSIC_BUTTON_MINUS;
                    break;
                case PAD_BUTTON_A:
                    varout=WPAD_CLASSIC_BUTTON_A;
                    break;
                case PAD_BUTTON_B:
                    varout=WPAD_CLASSIC_BUTTON_B;
                    break;
                case PAD_BUTTON_Y:
                    varout=WPAD_CLASSIC_BUTTON_Y;
                    break;
                case PAD_BUTTON_X:
                    varout=WPAD_CLASSIC_BUTTON_X;
                    break;
                case PAD_BUTTON_START:
                    varout=WPAD_CLASSIC_BUTTON_HOME;
                    break;
            }
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
char *getPadKeyLabel(s32 intPadKey,enum CONTROLLERS CONTROLLER) {
static char strPadButtonNoneSymbol[]="";
static char strPadLeftButtonSymbol[]={17,0};
static char strPadRightButtonSymbol[]={16,0};
static char strPadDownButtonSymbol[]={31,0};
static char strPadUpButtonSymbol[]={30,0};
static char strPadPlusButtonSymbol[]="+";
static char strPadMinusButtonSymbol[]="-";
static char strPadAButtonSymbol[]="A";
static char strPadBButtonSymbol[]="B";
static char strPad1ButtonSymbol[]="1";
static char strPad2ButtonSymbol[]="2";
static char strPadHomeButtonSymbol[]="Home";
static char strPadRButtonSymbol[]="R";
static char strPadZButtonSymbol[]="Z";
static char strPadXButtonSymbol[]="X";
static char strPadYButtonSymbol[]="Y";
static char strPadLButtonSymbol[]="L";
static char strPadStartButtonSymbol[]="Start";
char *varout=strPadButtonNoneSymbol;
    switch (CONTROLLER) {
        case GAMECUBE_CONTROLLER:
            switch(intPadKey) {
                case PAD_BUTTON_LEFT:
                    varout=strPadLeftButtonSymbol;
                    break;
                case PAD_BUTTON_RIGHT:
                    varout=strPadRightButtonSymbol;
                    break;
                case PAD_BUTTON_DOWN:
                    varout=strPadDownButtonSymbol;
                    break;
                case PAD_BUTTON_UP:
                    varout=strPadUpButtonSymbol;
                    break;
                case PAD_TRIGGER_Z:
                    varout=strPadZButtonSymbol;
                    break;
                case PAD_TRIGGER_R:
                    varout=strPadRButtonSymbol;
                    break;
                case PAD_TRIGGER_L:
                    varout=strPadLButtonSymbol;
                    break;
                case PAD_BUTTON_A:
                    varout=strPadAButtonSymbol;
                    break;
                case PAD_BUTTON_B:
                    varout=strPadBButtonSymbol;
                    break;
                case PAD_BUTTON_Y:
                    varout=strPadYButtonSymbol;
                    break;
                case PAD_BUTTON_X:
                    varout=strPadXButtonSymbol;
                    break;
                case PAD_BUTTON_START:
                    varout=strPadStartButtonSymbol;
                    break;
            }
            break;
        case WIIMOTE_CONTROLLER:
            switch(intPadKey) {
                case WPAD_BUTTON_LEFT:
                    varout=strPadLeftButtonSymbol;
                    break;
                case WPAD_BUTTON_RIGHT:
                    varout=strPadRightButtonSymbol;
                    break;
                case WPAD_BUTTON_DOWN:
                    varout=strPadDownButtonSymbol;
                    break;
                case WPAD_BUTTON_UP:
                    varout=strPadUpButtonSymbol;
                    break;
                case WPAD_BUTTON_PLUS:
                    varout=strPadPlusButtonSymbol;
                    break;
                case WPAD_BUTTON_MINUS:
                    varout=strPadMinusButtonSymbol;
                    break;
                case WPAD_BUTTON_A:
                    varout=strPadAButtonSymbol;
                    break;
                case WPAD_BUTTON_B:
                    varout=strPadBButtonSymbol;
                    break;
                case WPAD_BUTTON_1:
                    varout=strPad1ButtonSymbol;
                    break;
                case WPAD_BUTTON_2:
                    varout=strPad2ButtonSymbol;
                    break;
                case WPAD_BUTTON_HOME:
                    varout=strPadHomeButtonSymbol;
                    break;
            }
            break;
        case CLASSIC_CONTROLLER:
            switch(intPadKey) {
                case WPAD_CLASSIC_BUTTON_LEFT:
                    varout=strPadLeftButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_RIGHT:
                    varout=strPadRightButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_DOWN:
                    varout=strPadDownButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_UP:
                    varout=strPadUpButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_ZR:
                    varout=strPadRButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_PLUS:
                    varout=strPadPlusButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_MINUS:
                    varout=strPadMinusButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_A:
                    varout=strPadAButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_B:
                    varout=strPadBButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_Y:
                    varout=strPadYButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_X:
                    varout=strPadXButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_ZL:
                    varout=strPadLButtonSymbol;
                    break;
                case WPAD_CLASSIC_BUTTON_HOME:
                    varout=strPadHomeButtonSymbol;
                    break;
            }
            break;
    }
    return varout;
}
unsigned char getPadsKeyLabels(s32 intMappedPadKey,char **strPadsKeyLabels) {
    strPadsKeyLabels[GAMECUBE_CONTROLLER]=getPadKeyLabel(intMappedPadKey,GAMECUBE_CONTROLLER);
    strPadsKeyLabels[WIIMOTE_CONTROLLER]=getPadKeyLabel(getUnmappedPadKey(intMappedPadKey,WIIMOTE_CONTROLLER),WIIMOTE_CONTROLLER);
    strPadsKeyLabels[CLASSIC_CONTROLLER]=getPadKeyLabel(getUnmappedPadKey(intMappedPadKey,CLASSIC_CONTROLLER),CLASSIC_CONTROLLER);
    return uniqueStringArray(strPadsKeyLabels,CONTROLLERS_COUNT);
}
