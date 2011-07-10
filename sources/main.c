#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <ogc/consol.h>
#include <ogc/ios.h>
#include "CIOSMAPS_xml.h"
#include "patchmii.h"
#include "libios.h"
#include "libutils.h"
#include "debug.h"
#include "nand.h"
#include "controllers.h"
#include "video.h"
#include "network.h"
#include "d2x-cios-installer.h"
#include "title.h"
#include "menu.h"
#include "system.h"
#include "filesystem.h"
#include "gui.h"
#include "libfile.h"
#define IOS_MAJOR_TITLEID 1
#define MODWII_CACHE_FOLDER "/nus"
#define NAND_TEMP_DIRECTORY "/tmp/patchmii"
#if TESTING_CODE
#include "tests.h"
#define MODWII_TEST_FOLDER "/modwii"
char strModWiiTestFolder[256];
char strPrebuildCiosFolder[256];
#else
#include "d2xbg_png.h"
#endif
void onExit() {
	WPAD_Shutdown();
	sleep(5);
}
int main(int intArgumentsCount, char **strArguments) {
char strCacheFolder[256]={0},*strMountedDevice,strIosNumber[4],*strCiosMapsXmlFileContent,strCiosMapXmlFileName[256],*strHomebrewAppFolder;
bool blnRemoveCacheFolder=false,blnIosOptionEnabled=false;
unsigned char chSlots[]={200,201,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,243,244,245,246,247,248,249,250,251,252,253,255};
unsigned int intCiosCount=0,intSlotsCount=sizeof(chSlots),intSelectedCiosSlot=44,intSelectedCiosBase=0,intSelectedCios=0,intPreviousSelectedCios=0,intActiveMenu=0,intTempValue,intFatDevicesPriority=SD_STORAGE+10*USB_STORAGE;
u8 intSlotsMap[256],intWorkingCios=36;
s32 pressed=0,intExpectedPadsKeys[]={PAD_BUTTON_LEFT,PAD_BUTTON_RIGHT,PAD_BUTTON_A,PAD_BUTTON_B,PAD_BUTTON_UP,PAD_BUTTON_DOWN};
int intConsoleColumnsCount,intReturnValue,varout=1;
struct stCiosGroup *stCiosMaps,*stBetaCiosMaps=NULL;
struct stLabel stLabelSettings[6];
enum CONSOLE_FONT_COLORS HIGHLIGHT_ITEM_COLOR;
	atexit(onExit);
#if TESTING_CODE
    initConsole(NULL,GUI_BACKGROUND_COLOR,"Loading, please wait...",0,0,1,1);
#else
	initConsole(d2xbg_png,GUI_BACKGROUND_COLOR,"Loading, please wait...",-2,73,0.95,333);
#endif
    if (getSlotsMap(&intSlotsMap[0])) {
        intSlotsMap[242]=0;
        intSlotsMap[254]=0;
        if (intArgumentsCount>1) {
            while (intArgumentsCount) {
                intArgumentsCount--;
                if (!strncmp("--ios=",strArguments[intArgumentsCount],6)) {
                    blnIosOptionEnabled=true;
                    intWorkingCios=atoi(strchr(strArguments[intArgumentsCount],'=')+1);
                }
                if (!strncmp("--remove-cache",strArguments[intArgumentsCount],15)) {
                    blnRemoveCacheFolder=true;
                }
            }
        }
        intSlotsMap[0]=HAVE_AHBPROT*ACTIVE_IOS;
        if ((!blnIosOptionEnabled) || (intSlotsMap[intWorkingCios]<=STUB_IOS)) {
            if (intSlotsMap[236]) {
                intWorkingCios=236;
            }
            else if(intSlotsMap[249]) {
                intWorkingCios=249;
            }
            else if (intSlotsMap[250]) {
                intWorkingCios=250;
            }
            else if (intSlotsMap[222]) {
                intWorkingCios=222;
            }
            else if (intSlotsMap[223]) {
                intWorkingCios=223;
            }
            else {
                while (intSlotsMap[intWorkingCios]<=STUB_IOS) {
                    intWorkingCios++;
                    if(intWorkingCios>255) {
                        intWorkingCios=0;
                    }
                }
            }
        }
        if (blnIosOptionEnabled) {
            if (intWorkingCios) {
                IOS_ReloadIOS(intWorkingCios);
                sleep(1);
            }
            else {
                intSlotsMap[0]=getRuntimeIos();
                blnIosOptionEnabled=(applyAhbProtPatchs(false)>0);
            }
        }
        PAD_Init();
        WPAD_Init();
        clearConsole();
        intConsoleColumnsCount=getConsoleColumnsCount()-1;
        printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,0,4,"WELCOME","WORKING IOS","CIOS SETTINGS","INSTALLATION PROCESS");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] ABOUT D2X CIOS INSTALLER\n");
        printf("d2x cIOS installer is the official homebrew to install the d2x cIOS\n");
        printf("Website: http://www.wii-addict.fr\n\n");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"[*] DISCLAIMER\n");
        printBreakLines(' ',intConsoleColumnsCount,"IF YOU PAID FOR THIS SOFTWARE OR RECEIVED IT AS PART OF A \"BUNDLE\", YOU HAVE BEEN SCAMMED AND YOU SHOULD DEMAND YOUR MONEY BACK IMMEDIATELY. THIS PACKAGE COMES WITH ABSOLUTELY NO WARRANTY, NEITHER STATED NOR IMPLIED. NO ONE BUT YOURSELF IS TO BE HELD RESPONSIBLE FOR ANY DAMAGE THIS MAY CAUSE TO YOUR NINTENDO WII CONSOLE! USE ON YOUR OWN RISK!");
        printf("\n\n");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"[*] CREDITS\n");
        printf("Coders: Dragbe, Xflak, Davebaol, Xabby666, Waninkoko.\n");
        printf("Committers: Nutnut.\n");
        printf("Contributors: Sinedmax, Whynot, RobGee789, Damysteryman, WiiD, Rodries.\n");
        printf("Credits: Team Twiizers, Hermes, Arikado, Shagkur, WinterMute, Wiipower.\n\n");
        SYS_SetResetCallback(onResetButtonDown);
        if (waitPadsKeyPressed("Press any button to continue...\n")) {
            strMountedDevice=mountFatFs(intFatDevicesPriority);
            if (*strMountedDevice) {
#if TESTING_CODE
                snprintf(strModWiiTestFolder,sizeof(strModWiiTestFolder),"%s:%s",strMountedDevice,MODWII_TEST_FOLDER);
                mkdir(strModWiiTestFolder,S_IREAD | S_IWRITE);
#endif
                snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s",strMountedDevice,MODWII_CACHE_FOLDER);
                mkdir(strCacheFolder,S_IREAD | S_IWRITE);
            }
            if ((stCiosMaps=getCiosMaps((char *) CIOSMAPS_xml,strCacheFolder,&intCiosCount))!=NULL) {
                intSelectedCios=intCiosCount-1;
                intPreviousSelectedCios=intSelectedCios;
                if (*strCacheFolder) {
                    strHomebrewAppFolder=getDirName(strArguments[0]);
                    if (*strHomebrewAppFolder) {
                        snprintf(strCiosMapXmlFileName,sizeof(strCiosMapXmlFileName),"%s/CIOSMAPS.xml",strHomebrewAppFolder);
                        if ((strCiosMapsXmlFileContent=(char *) getFileContent(strCiosMapXmlFileName,&intTempValue))!=NULL) {
                            if ((stBetaCiosMaps=getCiosMaps(strCiosMapsXmlFileContent,strHomebrewAppFolder,&intTempValue))!=NULL) {
                                if ((stCiosMaps=(struct stCiosGroup *) realloc(stCiosMaps,(intCiosCount+intTempValue)*sizeof(struct stCiosGroup)))!=NULL) {
                                    while (intTempValue) {
                                        intTempValue--;
                                        stCiosMaps[intCiosCount]=stBetaCiosMaps[intTempValue];
                                        intCiosCount++;
                                    }
                                }
                                free(stBetaCiosMaps);
                            }
                            free(strCiosMapsXmlFileContent);
                        }
                    }
                    free(strHomebrewAppFolder);
                }
#if TESTING_CODE
                //showCiosMaps(stCiosMaps,intCiosCount);
#endif
                if (mountNandFs()) {
                    if ((intReturnValue=createNandDirectory(NAND_TEMP_DIRECTORY))) {
                        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to create temp dir %s: %d",NAND_TEMP_DIRECTORY,intReturnValue);
                        varout=intReturnValue;
                    }
                    else {
                        do {
                            if (blnIosOptionEnabled) {
                                pressed=PAD_BUTTON_A;
                            }
                            else {
                                if (varout) {
                                    pressed=0;
                                    clearConsole();
                                    printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,1,4,"WELCOME","WORKING IOS","CIOS SETTINGS","INSTALLATION PROCESS");
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] SETTINGS");
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nSelect IOS to use during installation ",intConsoleColumnsCount-38,&stLabelSettings[0]);
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] NOTES\n");
                                    printBreakLines(' ',intConsoleColumnsCount,"It is recommended to use the patched IOS236 installed with simple IOS patcher or IOS236 installer.");
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] NAVIGATION\n");
                                    printf("Press LEFT/RIGHT to select another IOS.\n");
                                    printf("Press A to continue.\n");
                                    printf("Press B to exit.\n\n");
                                    printStyledText(-1,-1,CONSOLE_FONT_BLACK,CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL,"Current IOS: %d v%d\n",getRuntimeIos(),getRuntimeIosVersion());
                                }
                            }
                            while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B))  {
                                if (intWorkingCios) {
                                    printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,"<IOS %i>",intWorkingCios);
                                }
                                else {
                                    printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,"<IOS %i+AHBPROT>",getRuntimeIos());
                                }
                                VIDEO_WaitVSync();
                                if ((pressed=getPadsKeyPressed(&intExpectedPadsKeys[0],4,true))) {
                                    if (pressed==PAD_BUTTON_RIGHT) {
                                        do {
                                            intWorkingCios++;
                                        } while (intSlotsMap[intWorkingCios]<=STUB_IOS);
                                    }
                                    if (pressed==PAD_BUTTON_LEFT) {
                                        do {
                                            intWorkingCios--;
                                        } while(intSlotsMap[intWorkingCios]<=STUB_IOS);
                                    }
                                }
                                else {
                                    pressed=PAD_BUTTON_B;
                                }
                            }
                            if (pressed==PAD_BUTTON_A) {
                                if (varout) {
                                    if (intWorkingCios) {
                                        if (getRuntimeIos()==intWorkingCios) {
                                            varout=0;
                                        }
                                        else {
                                            printf("Reloading IOS %i...\n",intWorkingCios);
                                            unmountNandFs();
                                            deInitNetwork();
                                            unmountFatFs(strMountedDevice);
                                            WPAD_Shutdown();
                                            sleep(1);
                                            IOS_ReloadIOS(intWorkingCios);
                                            sleep(1);
                                            WPAD_Init();
                                            PAD_Init();
                                            strMountedDevice=mountFatFs(intFatDevicesPriority);
                                            if (*strMountedDevice) {
                                                snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s",strMountedDevice,MODWII_CACHE_FOLDER);
                                                mkdir(strCacheFolder,S_IREAD | S_IWRITE);
                                            }
                                            else {
                                                strCacheFolder[0]=0;
                                            }
                                            if (mountNandFs()) {
                                                if ((varout=createNandDirectory(NAND_TEMP_DIRECTORY))) {
                                                    printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to create temp dir %s: %d",NAND_TEMP_DIRECTORY,varout);
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        if (intSlotsMap[0]==ACTIVE_IOS) {
                                            intSlotsMap[0]=getRuntimeIos();
                                            printf("Applying AHBPROT Patchs...\n");
                                            varout=(!(applyAhbProtPatchs(false)>0));
                                        }
                                        else {
                                            varout=0;
                                        }
                                    }
                                }
                                if (varout) {
                                    pressed=PAD_BUTTON_B;
                                }
                                else {
                                    pressed=0;
                                    clearConsole();
                                    printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,2,4,"WELCOME","WORKING IOS","CIOS SETTINGS","INSTALLATION PROCESS");
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] SETTINGS\n");
                                    CON_GetPosition(&stLabelSettings[0].stLabelLocation.intColumn,&stLabelSettings[0].stLabelLocation.intRow);
                                    stLabelSettings[0].chLabelSize=1;
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT," Select cIOS",intConsoleColumnsCount-13,&stLabelSettings[1]);
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n Select cIOS base",intConsoleColumnsCount-18,&stLabelSettings[2]);
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n Select cIOS slot",intConsoleColumnsCount-18,&stLabelSettings[3]);
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] NOTES");
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nOffline installation: ",intConsoleColumnsCount-22,&stLabelSettings[4]);
                                    drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nWarning: ",intConsoleColumnsCount-9,&stLabelSettings[5]);
                                    printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,"\n\n[*] NAVIGATION");
                                    printf("\nPress UP/DOWN to navigate.\n");
                                    printf("Press LEFT/RIGHT to change the values.\n");
                                    printf("Press A to install.\n");
                                    printf("Press B to exit.\n\n");
                                    printStyledText(-1,-1,CONSOLE_FONT_BLACK,CONSOLE_FONT_YELLOW,CONSOLE_FONT_NORMAL,"Current IOS: %d v%d\n",getRuntimeIos(),getRuntimeIosVersion());
                                }
                                while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B)) {
                                    for (intTempValue=0;intTempValue<3;intTempValue++) {
                                        printLabel(stLabelSettings[0].stLabelLocation.intRow+intTempValue,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,(intTempValue==intActiveMenu)?">":" ");
                                    }
                                    printListItem(stLabelSettings[1].stLabelLocation.intRow,stLabelSettings[1].stLabelLocation.intColumn,stCiosMaps[intSelectedCios].strGroupName,"",intSelectedCios,intCiosCount,CONSOLE_FONT_BLACK,"","",intConsoleColumnsCount);
                                    snprintf(&strIosNumber[0],sizeof(strIosNumber),"%d",stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase);
                                    printListItem(stLabelSettings[2].stLabelLocation.intRow,stLabelSettings[2].stLabelLocation.intColumn,strIosNumber,"",intSelectedCiosBase,stCiosMaps[intSelectedCios].chCiosCount,CONSOLE_FONT_BLACK,"","",intConsoleColumnsCount);
                                    switch (intSlotsMap[chSlots[intSelectedCiosSlot]]) {
                                        case 0:
                                            HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_BLACK;
                                            break;
                                        case STUB_IOS:
                                            HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_MAGENTA;
                                            break;
                                        default:
                                            HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_RED;
                                    }
                                    snprintf(&strIosNumber[0],sizeof(strIosNumber),"%d",chSlots[intSelectedCiosSlot]);
                                    printListItem(stLabelSettings[3].stLabelLocation.intRow,stLabelSettings[3].stLabelLocation.intColumn,strIosNumber,"",intSelectedCiosSlot,intSlotsCount,HIGHLIGHT_ITEM_COLOR,"","",intConsoleColumnsCount);
                                    printLabel(stLabelSettings[4].stLabelLocation.intRow,stLabelSettings[4].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[4].chLabelSize,"IOS%d-64-v%d.wad required (downloadable with NUSD)",stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.intBaseRevision);
                                    switch (intSlotsMap[chSlots[intSelectedCiosSlot]]) {
                                        case 0:
                                            printLabel(stLabelSettings[5].stLabelLocation.intRow,stLabelSettings[5].stLabelLocation.intColumn,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[5].chLabelSize,"Slot %d free",chSlots[intSelectedCiosSlot]);
                                            break;
                                        case STUB_IOS:
                                            printLabel(stLabelSettings[5].stLabelLocation.intRow,stLabelSettings[5].stLabelLocation.intColumn,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[5].chLabelSize,"The IOS STUB detected in the slot %d will be overwritten",chSlots[intSelectedCiosSlot]);
                                            break;
                                        default:
                                            printLabel(stLabelSettings[5].stLabelLocation.intRow,stLabelSettings[5].stLabelLocation.intColumn,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[5].chLabelSize,"The (c)IOS detected in the slot %d will be overwritten",chSlots[intSelectedCiosSlot]);
                                    }
                                    VIDEO_WaitVSync();
                                    pressed=getPadsKeyPressed(&intExpectedPadsKeys[0],6,true);
                                    if (pressed) {
                                        switch (pressed) {
                                            case PAD_BUTTON_DOWN:
                                                intActiveMenu=getListItemId(intActiveMenu,1,3);
                                                break;
                                            case PAD_BUTTON_UP:
                                                intActiveMenu=getListItemId(intActiveMenu,-1,3);
                                                break;
                                            case PAD_BUTTON_LEFT:
                                            case PAD_BUTTON_RIGHT:
                                                switch(intActiveMenu) {
                                                    case 0:
                                                        intPreviousSelectedCios=intSelectedCios;
                                                        if (pressed==PAD_BUTTON_LEFT) {
                                                            intSelectedCios=getListItemId(intSelectedCios,-1,intCiosCount);
                                                        }
                                                        else {
                                                            intSelectedCios=getListItemId(intSelectedCios,1,intCiosCount);
                                                        }
                                                        if (intPreviousSelectedCios!=intSelectedCios) {
                                                            intSelectedCiosBase=0;
                                                        }
                                                        break;
                                                    case 1:
                                                        if (pressed==PAD_BUTTON_LEFT) {
                                                            intSelectedCiosBase=getListItemId(intSelectedCiosBase,-1,stCiosMaps[intSelectedCios].chCiosCount);
                                                        }
                                                        else {
                                                            intSelectedCiosBase=getListItemId(intSelectedCiosBase,1,stCiosMaps[intSelectedCios].chCiosCount);
                                                        }
                                                        break;
                                                    case 2:
                                                        if (pressed==PAD_BUTTON_LEFT) {
                                                            intSelectedCiosSlot=getListItemId(intSelectedCiosSlot,-1,intSlotsCount);
                                                        }
                                                        else {
                                                            intSelectedCiosSlot=getListItemId(intSelectedCiosSlot,1,intSlotsCount);
                                                        }
                                                        break;
                                                }
                                                break;
                                        }
                                    }
                                    else {
                                        pressed=PAD_BUTTON_B;
                                    }
                                }
                                if (pressed==PAD_BUTTON_A) {
                                    if (strCacheFolder[0]) {
#if TESTING_CODE
                                        snprintf(strModWiiTestFolder,sizeof(strModWiiTestFolder),"%s:%s/%d-%d",strMountedDevice,MODWII_TEST_FOLDER,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].intCiosRevision);
                                        mkdir(strModWiiTestFolder,S_IREAD|S_IWRITE);
#endif
                                        snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s/%08x%08xv%d",strMountedDevice,MODWII_CACHE_FOLDER,IOS_MAJOR_TITLEID,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.intBaseRevision);
                                        mkdir(strCacheFolder,S_IREAD | S_IWRITE);
                                    }
                                    clearConsole();
                                    printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,3,4,"WELCOME","WORKING IOS","CIOS SETTINGS","INSTALLATION PROCESS");
                                    printf("\n\n");
                                    if ((varout=patchmii(&stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase],chSlots[intSelectedCiosSlot],stCiosMaps[intSelectedCios].intCiosRevision,IOS_MAJOR_TITLEID,strCacheFolder,NAND_TEMP_DIRECTORY))) {
#if TESTING_CODE == 0
                                        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_RED,CONSOLE_FONT_BOLD,"\nERROR(s) occured during cIOS%d %s base %d rev%d installation\n",chSlots[intSelectedCiosSlot],stCiosMaps[intSelectedCios].strGroupName,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].intCiosRevision);
#endif
                                        deInitNetwork();
                                    }
                                    else {
                                        intSlotsMap[chSlots[intSelectedCiosSlot]]=ACTIVE_IOS;
#if TESTING_CODE == 0
                                        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_GREEN,CONSOLE_FONT_BOLD,"cIOS%d %s base %d rev%d installed with SUCCESS\n",chSlots[intSelectedCiosSlot],stCiosMaps[intSelectedCios].strGroupName,stCiosMaps[intSelectedCios].stCios[intSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].intCiosRevision);
#endif
                                    }
                                    printf("Press A to continue or B to exit\n");
                                    pressed=getPadsKeyPressed(&intExpectedPadsKeys[2],2,true);
                                    if (!pressed) {
                                        pressed=PAD_BUTTON_B;
                                        printf("Aborted, ");
                                    }
                                }
                                else {
                                    printf("Aborted, ");
                                }
                            }
                            else {
                                printf("Aborted, ");
                            }
                        } while (pressed!=PAD_BUTTON_B);
                    }
                    unmountNandFs();
                }
                else {
                    varout=1;
                }
                deInitNetwork();
                free(stCiosMaps);
                freeExternalCiosModules();
            }
            else {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError getting cIOS maps\n");
            }
            if (*strMountedDevice) {
                if (blnRemoveCacheFolder) {
                    snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s",strMountedDevice,MODWII_CACHE_FOLDER);
                    removeDirectory(strCacheFolder);
                }
                unmountFatFs(strMountedDevice);
            }
            free(strMountedDevice);
        }
        else {
            printf("Aborted, ");
        }
        printf("Exiting...");
        WPAD_Shutdown();
    }
    else {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError getting slots map\n");
    }
	return varout;
}
