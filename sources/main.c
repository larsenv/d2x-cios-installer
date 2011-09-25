#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <ogc/gu.h>
#include <ogc/consol.h>
#include <ogc/ios.h>
#include "CIOSMAPS_xml.h"
#include "patchmii.h"
#include "libios.h"
#include "libutils.h"
#include "libmath.h"
#include "debug.h"
#include "nand.h"
#include "controllers.h"
#include "video.h"
#include "network.h"
#include "d2x-cios-installer.h"
#include "title.h"
#include "menu.h"
#include "nus.h"
#include "libarray.h"
#include "system.h"
#include "filesystem.h"
#include "libgeometry.h"
#include "gui.h"
#include "libfile.h"
#define CIOS_BASE_SLOT 200
#define CIOS_SLOTS_COUNT 56
//multiple of SLOTS_TABLE_ROWS <65 with (CIOS_BASE_SLOT+CIOS_SLOTS_COUNT-1)<256
#define IOS_MAJOR_TITLEID 1
#define MODWII_CACHE_FOLDER "/nus"
#define NAND_TEMP_DIRECTORY "/tmp/patchmii"
#define CIOS_CONFIG_FILE "ciosconf.cfg"
#define NUS_SCRIPT_FILE "ios.nus"
#define SLOTS_TABLE_ROWS 4
#define SLOTS_TABLE_COLUMNS (CIOS_SLOTS_COUNT/SLOTS_TABLE_ROWS)
#define SLOTS_TABLE_CELL_WIDTH 4
#define SLOTS_TABLE_CELL_HEIGHT 2
#if TESTING_CODE
#include "tests.h"
#define MODWII_TEST_FOLDER "/modwii"
char strModWiiTestFolder[256];
char strPrebuildCiosFolder[256];
#else
#include "d2xbg_png.h"
#endif
struct stCiosSettings {
    struct stCiosMap *stCios;
    const char *strCiosName;
    u16 intRevision;
};
struct stInstallSlotsMap {
    unsigned char chCiosCount;
    unsigned long long lngScheduledCios;
    struct stCiosSettings stCiosInstallSettings[CIOS_SLOTS_COUNT];
};
bool isFreeSlotSetting(unsigned char chSlot,struct stInstallSlotsMap *stCiosInstallMap) {
    return ((stCiosInstallMap->lngScheduledCios & (unsigned long long) pow(2,chSlot-CIOS_BASE_SLOT))==0);
}
signed char getSelectedCiosCmp(const char *strCiosName,u8 chBase,u16 intRevision,struct stInstallSlotsMap *stCiosInstallMap) {
signed char varout=DISTINCT_CIOS;
unsigned long long lngScheduledCios=stCiosInstallMap->lngScheduledCios;
unsigned char chReducedSlot;
    while (lngScheduledCios) {
        chReducedSlot=(unsigned char) (log(lngScheduledCios)/log(2));
        if (!strcmp(strCiosName,stCiosInstallMap->stCiosInstallSettings[chReducedSlot].strCiosName)) {
            if (stCiosInstallMap->stCiosInstallSettings[chReducedSlot].stCios->stBase.chBase==chBase) {
                varout=(signed char) chReducedSlot+1;
                if (intRevision==stCiosInstallMap->stCiosInstallSettings[chReducedSlot].intRevision) {
                    varout=varout*(signed char) SAME_CIOS;
                }
                else {
                    varout=varout*(signed char) SAME_BASE_CIOS;
                }
            }
        }
        lngScheduledCios=lngScheduledCios-(unsigned long long) pow(2,chReducedSlot);
    }
    return varout;
}
unsigned char removeScheduledCios(unsigned char chSelectedCiosSlot,struct stInstallSlotsMap *stCiosInstallMap) {
    if (!isFreeSlotSetting(chSelectedCiosSlot,stCiosInstallMap)) {
        stCiosInstallMap->lngScheduledCios=stCiosInstallMap->lngScheduledCios-(unsigned long long) pow(2,chSelectedCiosSlot-CIOS_BASE_SLOT);
        stCiosInstallMap->chCiosCount=stCiosInstallMap->chCiosCount-1;
    }
    return stCiosInstallMap->chCiosCount;
}
unsigned char addScheduledCios(struct stCiosMap *stSelectedCios,const char *strCiosGroupName,unsigned char *chSelectedCiosSlot,u16 intCiosRevision,struct stInstallSlotsMap *stCiosInstallMap) {
unsigned char chReducedSlot;
    if (isFreeSlotSetting(*chSelectedCiosSlot,stCiosInstallMap)) {
        if ((chReducedSlot=*chSelectedCiosSlot-CIOS_BASE_SLOT)<CIOS_SLOTS_COUNT) {
            stCiosInstallMap->stCiosInstallSettings[chReducedSlot].stCios=stSelectedCios;
            stCiosInstallMap->stCiosInstallSettings[chReducedSlot].strCiosName=strCiosGroupName;
            stCiosInstallMap->stCiosInstallSettings[chReducedSlot].intRevision=intCiosRevision;
            stCiosInstallMap->lngScheduledCios=stCiosInstallMap->lngScheduledCios+(unsigned long long) pow(2,chReducedSlot);
            stCiosInstallMap->chCiosCount=stCiosInstallMap->chCiosCount+1;
        }
    }
    return stCiosInstallMap->chCiosCount;
}
unsigned char writeCiosConfigFile(struct stInstallSlotsMap *stCiosInstallMap,const char *strMountedDeviceName) {
static char strConfigFile[256];
unsigned long long lngScheduledCios=stCiosInstallMap->lngScheduledCios;
unsigned char chReducedSlot,varout=0;
FILE *fh;
    snprintf(strConfigFile,sizeof(strConfigFile),"%s:/%s",strMountedDeviceName,CIOS_CONFIG_FILE);
    if ((fh=fopen(strConfigFile,"w"))!=NULL) {
        while (lngScheduledCios) {
            chReducedSlot=(unsigned char) (log(lngScheduledCios)/log(2));
            if (fprintf(fh,"|%s| %d %d %d\n",stCiosInstallMap->stCiosInstallSettings[chReducedSlot].strCiosName,stCiosInstallMap->stCiosInstallSettings[chReducedSlot].stCios->stBase.chBase,CIOS_BASE_SLOT+chReducedSlot,stCiosInstallMap->stCiosInstallSettings[chReducedSlot].intRevision)>0) {
                varout++;
            }
            lngScheduledCios=lngScheduledCios-(unsigned long long) pow(2,chReducedSlot);
        }
        fclose(fh);
    }
    return varout;
}
unsigned char writeNusScriptFile(struct stInstallSlotsMap *stCiosInstallMap,const char *strMountedDeviceName) {
static char strNusScriptFile[256];
struct stNusTitle *stNusTitles=NULL;
unsigned long long lngScheduledCios=stCiosInstallMap->lngScheduledCios;
unsigned char chReducedSlot,varout=0;
    if ((stNusTitles=(struct stNusTitle *) malloc(stCiosInstallMap->chCiosCount*sizeof(struct stNusTitle)))!=NULL) {
        snprintf(strNusScriptFile,sizeof(strNusScriptFile),"%s:/%s",strMountedDeviceName,NUS_SCRIPT_FILE);
        while (lngScheduledCios) {
            chReducedSlot=(unsigned char) (log(lngScheduledCios)/log(2));
            stNusTitles[varout].intTitleId=0;
            stNusTitles[varout].intMajorTitleId=IOS_MAJOR_TITLEID;
            stNusTitles[varout].intMinorTitleId=stCiosInstallMap->stCiosInstallSettings[chReducedSlot].stCios->stBase.chBase;
            stNusTitles[varout].intTitleVersion=stCiosInstallMap->stCiosInstallSettings[chReducedSlot].stCios->stBase.intBaseRevision;
            lngScheduledCios=lngScheduledCios-(unsigned long long) pow(2,chReducedSlot);
            varout++;
        }
        varout=writeNusScript(strNusScriptFile,&stNusTitles[0],varout);
        free(stNusTitles);
        stNusTitles=NULL;
    }
    return varout;
}
unsigned char loadCiosConfigFile(const char *strHomebrewAppFolder,struct stInstallSlotsMap *stCiosInstallMap,unsigned char *chAvailabledSlots,unsigned char chAvailabledSlotsCount,struct stCiosGroup *stCiosMaps,unsigned int intCiosGroupCount) {
unsigned char chCiosSlot,varout=0;
unsigned int intCiosBase,intCiosSlot,intCiosRevision;
char strCiosName[256];
struct stCiosMap *stSelectedCios;
char *strCiosGroupName=NULL;
char strCiosConfigFile[256];
FILE *fh;
    stCiosInstallMap->lngScheduledCios=0;
    snprintf(strCiosConfigFile,sizeof(strCiosConfigFile),"%s/%s",strHomebrewAppFolder,CIOS_CONFIG_FILE);
    if ((fh=fopen(strCiosConfigFile,"r"))!=NULL) {
        while (fscanf(fh,"|%256[^|]|%u%u%u\n",strCiosName,&intCiosBase,&intCiosSlot,&intCiosRevision)==4) {
            chCiosSlot=intCiosSlot;
            if ((chCiosSlot=getArrayValueIndex(chAvailabledSlots,chAvailabledSlotsCount,sizeof(unsigned char),&chCiosSlot))<chAvailabledSlotsCount) {
                if ((stSelectedCios=getCiosMap(strCiosName,intCiosBase,intCiosRevision,stCiosMaps,intCiosGroupCount,&strCiosGroupName))!=NULL) {
                    varout=addScheduledCios(stSelectedCios,strCiosGroupName,&chAvailabledSlots[chCiosSlot],intCiosRevision,stCiosInstallMap);
                }
            }
        }
        fclose(fh);
    }
    return varout;
}
unsigned char getFreeSlotSetting(unsigned char chSelectedCiosSlot,unsigned char *chSlots,unsigned char chSlotsCount,struct stInstallSlotsMap *stCiosInstallMap) {
unsigned char varout=chSelectedCiosSlot;
    do {
        varout=getReducedRangedNumber(varout+1,0,chSlotsCount-1);
    } while ((varout!=chSelectedCiosSlot) && (!isFreeSlotSetting(chSlots[varout],stCiosInstallMap)));
    return varout;
}
unsigned char setFreeCiosSettings(unsigned int *intSelectedCios,unsigned char *chSelectedCiosBase,unsigned char *chSelectedCiosSlot,unsigned char *chSlots,unsigned char chSlotsCount,u16 *intCiosAlternativeRevisions,unsigned char *chSelectedCiosRevision,struct stCiosGroup *stCiosMaps,unsigned int intCiosCount,struct stInstallSlotsMap *stCiosInstallMap) {
unsigned char chInitialSelection=*chSelectedCiosBase;
unsigned int intInitialSelection=*intSelectedCios;
    do {
        if ((*chSelectedCiosBase=getReducedRangedNumber(*chSelectedCiosBase+1,0,stCiosMaps[*intSelectedCios].chCiosCount-1))==0) {
            *intSelectedCios=getReducedRangedNumber(*intSelectedCios+1,0,intCiosCount-1);
        }
    } while (((chInitialSelection!=*chSelectedCiosBase) || (intInitialSelection!=*intSelectedCios)) && (getSelectedCiosCmp(stCiosMaps[*intSelectedCios].strGroupName,stCiosMaps[*intSelectedCios].stCios[*chSelectedCiosBase].stBase.chBase,0,stCiosInstallMap)!=DISTINCT_CIOS));
    *chSelectedCiosSlot=getFreeSlotSetting(*chSelectedCiosSlot,chSlots,chSlotsCount,stCiosInstallMap);
    *chSelectedCiosRevision=0;
    return getCiosAlternativeRevisions(intCiosAlternativeRevisions,stCiosMaps[*intSelectedCios].intCiosRevision);
}
unsigned char getHomebrewFatDevicesPriority(const char *strHomebrewFileName) {
unsigned char varout=SD_STORAGE_DEVICE+10*USB_STORAGE_DEVICE;
char *strHomebrewDeviceFileName=NULL;
    if (*strHomebrewFileName) {
        strHomebrewDeviceFileName=getDeviceFileName(strHomebrewFileName);
        if (!strcmp(strHomebrewDeviceFileName,"usb")) {
            varout=USB_STORAGE_DEVICE+10*SD_STORAGE_DEVICE;
        }
        free(strHomebrewDeviceFileName);
        strHomebrewDeviceFileName=NULL;
    }
    return varout;
}
void onExit() {
	WPAD_Shutdown();
	sleep(1);
}
int main(int intArgumentsCount, char **strArguments) {
char strCacheFolder[256]={0},*strMountedDevice,*strCiosMapsXmlFileContent,strCiosMapXmlFileName[256],*strHomebrewAppFolder;
bool blnRemoveCacheFolder=false,blnIosOptionEnabled=false;
unsigned char chFatDevicesPriority=getHomebrewFatDevicesPriority(strArguments[0]),chSlots[]={CIOS_BASE_SLOT,CIOS_BASE_SLOT+1,CIOS_BASE_SLOT+3,CIOS_BASE_SLOT+4,CIOS_BASE_SLOT+5,CIOS_BASE_SLOT+6,CIOS_BASE_SLOT+7,CIOS_BASE_SLOT+8,CIOS_BASE_SLOT+9,CIOS_BASE_SLOT+10,CIOS_BASE_SLOT+11,CIOS_BASE_SLOT+12,CIOS_BASE_SLOT+13,CIOS_BASE_SLOT+14,CIOS_BASE_SLOT+15,CIOS_BASE_SLOT+16,CIOS_BASE_SLOT+17,CIOS_BASE_SLOT+18,CIOS_BASE_SLOT+19,CIOS_BASE_SLOT+20,CIOS_BASE_SLOT+21,CIOS_BASE_SLOT+25,CIOS_BASE_SLOT+26,CIOS_BASE_SLOT+27,CIOS_BASE_SLOT+28,CIOS_BASE_SLOT+29,CIOS_BASE_SLOT+30,CIOS_BASE_SLOT+31,CIOS_BASE_SLOT+32,CIOS_BASE_SLOT+33,CIOS_BASE_SLOT+34,CIOS_BASE_SLOT+35,CIOS_BASE_SLOT+36,CIOS_BASE_SLOT+37,CIOS_BASE_SLOT+38,CIOS_BASE_SLOT+39,CIOS_BASE_SLOT+40,CIOS_BASE_SLOT+41,CIOS_BASE_SLOT+43,CIOS_BASE_SLOT+44,CIOS_BASE_SLOT+45,CIOS_BASE_SLOT+46,CIOS_BASE_SLOT+47,CIOS_BASE_SLOT+48,CIOS_BASE_SLOT+49,CIOS_BASE_SLOT+50,CIOS_BASE_SLOT+51,CIOS_BASE_SLOT+52,CIOS_BASE_SLOT+53,CIOS_BASE_SLOT+55},chSlotsCount=sizeof(chSlots),chSelectedCiosSlot=44,chSelectedCiosBase=0,chSelectedCiosRevision=0,chCiosAlternativeRevisionsCount,chWorkingCios=36,chLevelIndex=0;
unsigned int intCiosCount=0,intSelectedCios=0,intPreviousSelectedCios=0,intActiveMenu=0,intTempValue;
s32 pressed=0,intExpectedPadsKeys[]={PAD_BUTTON_UP,PAD_BUTTON_DOWN,PAD_BUTTON_LEFT,PAD_BUTTON_RIGHT,PAD_BUTTON_A,PAD_BUTTON_B,PAD_BUTTON_START,PAD_BUTTON_Y,PAD_BUTTON_X,PAD_TRIGGER_L};
int intConsoleColumnsCount,intReturnValue,varout=1;
struct stCiosGroup *stCiosMaps,*stBetaCiosMaps=NULL;
struct stLabel stLabelSettings[9];
struct stTable stTableSettings;
enum CONSOLE_FONT_COLORS HIGHLIGHT_ITEM_COLOR;
u16 intCiosAlternativeRevisions[3];
struct stSlotInfos *stSlotsMap;
struct stInstallSlotsMap stCiosInstallMap;
struct stConsoleCursorLocation stTexteLocation;
struct stBlinkTextsGroup stBlinkTexts;
struct stCommandsBar stCommandsBarSettings;
unsigned long long lngScheduledCios;
	//atexit(onExit);
#if TESTING_CODE
    initConsole(NULL,GUI_BACKGROUND_COLOR,"Loading, please wait...",0,0,1,1);
#else
	initConsole(d2xbg_png,GUI_BACKGROUND_COLOR,"Loading, please wait...",-2,73,0.95,333);
#endif
    if (setAhbProtAccessRights(false)>0) {
        chWorkingCios=IOS_GetVersion();
        IOS_ReloadIOS(chWorkingCios);
        sleep(1);
        blnIosOptionEnabled=(applyAhbProtPatchs(false)>0);
    }
    if ((stSlotsMap=getSlotsMap())!=NULL) {
        if (intArgumentsCount>1) {
            while (intArgumentsCount) {
                intArgumentsCount--;
                if (!blnIosOptionEnabled) {
                    if (!strncmp("--ios=",strArguments[intArgumentsCount],6)) {
                        blnIosOptionEnabled=true;
                        chWorkingCios=atoi(strchr(strArguments[intArgumentsCount],'=')+1);
                    }
                }
                if (!strncmp("--remove-cache",strArguments[intArgumentsCount],15)) {
                    blnRemoveCacheFolder=true;
                }
            }
        }
        if ((!blnIosOptionEnabled) || (stSlotsMap[chWorkingCios].chIosType<=RESERVED_IOS)) {
            if (stSlotsMap[236].chIosType>RESERVED_IOS) {
                chWorkingCios=236;
            }
            else if(stSlotsMap[249].chIosType>RESERVED_IOS) {
                chWorkingCios=249;
            }
            else if (stSlotsMap[250].chIosType>RESERVED_IOS) {
                chWorkingCios=250;
            }
            else if (stSlotsMap[222].chIosType>RESERVED_IOS) {
                chWorkingCios=222;
            }
            else if (stSlotsMap[223].chIosType>RESERVED_IOS) {
                chWorkingCios=223;
            }
            else {
                while (stSlotsMap[chWorkingCios].chIosType<=RESERVED_IOS) {
                    chWorkingCios++;
                }
            }
        }
        if (blnIosOptionEnabled) {
            if (chWorkingCios!=IOS_GetVersion()) {
                IOS_ReloadIOS(chWorkingCios);
                sleep(1);
            }
        }
        PAD_Init();
        WPAD_Init();
        clearConsole();
        intConsoleColumnsCount=getConsoleColumnsCount()-1;
        printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,chLevelIndex,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"\n\n[*] ABOUT D2X CIOS INSTALLER\n");
        printf("Official homebrew (http://www.wii-addict.fr) to install the d2x cIOS\n\n");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"[*] DISCLAIMER\n");
        printBreakLines(' ',intConsoleColumnsCount,"THIS PACKAGE COMES WITH ABSOLUTELY NO WARRANTY, NEITHER STATED NOR IMPLIED. NO ONE BUT YOURSELF IS TO BE HELD RESPONSIBLE FOR ANY DAMAGE THIS MAY CAUSE TO YOUR NINTENDO WII CONSOLE! USE ON YOUR OWN RISK!");
        printf("\n\n");
        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"[*] CREDITS\n");
        printf("Project members: Dragbe, Nutnut, Lola, Abdeljutsu, WiiD, RobGee789.\n");
        printf("Initiators: Xflak, Davebaol, Xabby666, Waninkoko.\n");
        printf("Contributors: Sinedmax, Whynot, Damysteryman, Rodries.\n");
        printf("Credits: Team Twiizers, Hermes, Arikado, Shagkur, WinterMute, Wiipower.\n\n");
        drawCommandsBar(0,true,&stCommandsBarSettings);
        setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
        SYS_SetResetCallback(onResetButtonDown);
        if (waitPadsKeyPressed("Press any button to continue...\n")) {
            strMountedDevice=mountFatDevice(chFatDevicesPriority);
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
                stCiosInstallMap.lngScheduledCios=0;
                stCiosInstallMap.chCiosCount=0;
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
                                stBetaCiosMaps=NULL;
                            }
                            free(strCiosMapsXmlFileContent);
                            strCiosMapsXmlFileContent=NULL;
                        }
                        stCiosInstallMap.chCiosCount=loadCiosConfigFile(strHomebrewAppFolder,&stCiosInstallMap,&chSlots[0],chSlotsCount,&stCiosMaps[0],intCiosCount);
                    }
                    free(strHomebrewAppFolder);
                    strHomebrewAppFolder=NULL;
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
                        if ((stBlinkTexts.stBlinkTexts=(struct stBlinkText *) malloc((chSlotsCount+1)*sizeof(struct stBlinkText)))==NULL) {
                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nFailed to create the blink texts settings memory block");
                        }
                        else {
                            chCiosAlternativeRevisionsCount=getCiosAlternativeRevisions(&intCiosAlternativeRevisions[0],stCiosMaps[intSelectedCios].intCiosRevision);
                            do {
                                if (blnIosOptionEnabled) {
                                    pressed=PAD_BUTTON_A;
                                }
                                else {
                                    if (varout) {
                                        pressed=0;
                                        clearConsole();
                                        chLevelIndex=1;
                                        printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,chLevelIndex,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
                                        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"\n\n[*] SETTINGS");
                                        drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nSelect IOS to use during installation ",intConsoleColumnsCount-39,&stLabelSettings[0],&stTexteLocation);
                                        printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"\n\n[*] NOTES\n");
                                        printBreakLines(' ',intConsoleColumnsCount,"It is recommended to use the patched IOS236 installed with simple IOS patcher or IOS236 installer.");
                                        printf("\n");
                                        drawCommandsBar(3,true,&stCommandsBarSettings);
                                        addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[4],1,": Continue");
                                        addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[5],1,": Exit");
                                        addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[2],2,": IOS selection");
                                        setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                    }
                                }
                                while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B))  {
                                    if (chWorkingCios) {
                                        printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"<IOS %i>",chWorkingCios);
                                    }
                                    else {
                                        printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"<IOS %i+AHBPROT>",IOS_GetVersion());
                                    }
                                    VIDEO_WaitVSync();
                                    if ((pressed=getPadsKeyPressed(&intExpectedPadsKeys[2],4,true))) {
                                        if (pressed==PAD_BUTTON_RIGHT) {
                                            do {
                                                chWorkingCios++;
                                            } while (stSlotsMap[chWorkingCios].chIosType<=RESERVED_IOS);
                                        }
                                        if (pressed==PAD_BUTTON_LEFT) {
                                            do {
                                                chWorkingCios--;
                                            } while(stSlotsMap[chWorkingCios].chIosType<=RESERVED_IOS);
                                        }
                                    }
                                    else {
                                        pressed=PAD_BUTTON_B;
                                    }
                                }
                                if (pressed==PAD_BUTTON_A) {
                                    if (varout) {
                                        if (IOS_GetVersion()==chWorkingCios) {
                                            varout=0;
                                        }
                                        else {
                                            setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Reloading IOS %i...",chWorkingCios);
                                            unmountNandFs();
                                            deInitNetwork();
                                            unmountFatDevice(strMountedDevice);
                                            free(strMountedDevice);
                                            strMountedDevice=NULL;
                                            WPAD_Shutdown();
                                            sleep(1);
                                            IOS_ReloadIOS(chWorkingCios);
                                            sleep(1);
                                            WPAD_Init();
                                            PAD_Init();
                                            strMountedDevice=mountFatDevice(chFatDevicesPriority);
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
                                    if (varout) {
                                        pressed=PAD_BUTTON_B;
                                    }
                                    else {
                                        if ((!stCiosInstallMap.chCiosCount) || (chLevelIndex==3)) {
                                            pressed=0;
                                            clearConsole();
                                            chLevelIndex=2;
                                            printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,chLevelIndex,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
                                            printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"\n\n[*] SETTINGS\n");
                                            stLabelSettings[0].chLabelSize=1;
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT," Select cIOS",intConsoleColumnsCount-13,&stLabelSettings[1],&stLabelSettings[0].stLabelLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n Select cIOS base",intConsoleColumnsCount-18,&stLabelSettings[2],&stTexteLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n Select cIOS slot",intConsoleColumnsCount-18,&stLabelSettings[3],&stTexteLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n Select cIOS revision",intConsoleColumnsCount-22,&stLabelSettings[4],&stTexteLocation);
                                            printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"\n\n[*] NOTES");
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nOffline mode: ",intConsoleColumnsCount-15,&stLabelSettings[5],&stTexteLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\nWarning(s): ",intConsoleColumnsCount-13,&stLabelSettings[6],&stTexteLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n            ",intConsoleColumnsCount-13,&stLabelSettings[7],&stTexteLocation);
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"\n            ",intConsoleColumnsCount-13,&stLabelSettings[8],&stTexteLocation);
                                            printf("\n");
                                            drawCommandsBar(8,true,&stCommandsBarSettings);
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[4],1,": Continue");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[5],1,": Exit");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[2],2,": Change values");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[0],2,": Navigation");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[6],1,": Add cIOS to batch");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[9],1,": Disable batch mode");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[7],1,": Save the NUS script");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[8],1,": Save the batch settings");
                                            setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                        }
                                    }
                                    if (chLevelIndex==2) {
                                        while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B)) {
                                            for (intTempValue=0;intTempValue<4;intTempValue++) {
                                                printLabel(stLabelSettings[0].stLabelLocation.intRow+intTempValue,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,(intTempValue==intActiveMenu)?">":" ");
                                            }
                                            intTempValue=6;
                                            if ((intReturnValue=getSelectedCiosCmp(stCiosMaps[intSelectedCios].strGroupName,stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase].stBase.chBase,intCiosAlternativeRevisions[chSelectedCiosRevision],&stCiosInstallMap))==DISTINCT_CIOS) {
                                                HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_BLACK;
                                            }
                                            else {
                                                intReturnValue=abs(intReturnValue)-1;
                                                printLabel(stLabelSettings[intTempValue].stLabelLocation.intRow,stLabelSettings[intTempValue].stLabelLocation.intColumn,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[intTempValue].chLabelSize,&stTexteLocation,"cIOS already added in batch with revision %d and slot %d",stCiosInstallMap.stCiosInstallSettings[intReturnValue].intRevision,intReturnValue+CIOS_BASE_SLOT);
                                                HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_MAGENTA;
                                                intTempValue++;
                                            }
                                            printListItem(stLabelSettings[1].stLabelLocation.intRow,stLabelSettings[1].stLabelLocation.intColumn,"",intSelectedCios,intCiosCount,HIGHLIGHT_ITEM_COLOR,"","",intConsoleColumnsCount,"%s",stCiosMaps[intSelectedCios].strGroupName);
                                            printListItem(stLabelSettings[2].stLabelLocation.intRow,stLabelSettings[2].stLabelLocation.intColumn,"",chSelectedCiosBase,stCiosMaps[intSelectedCios].chCiosCount,HIGHLIGHT_ITEM_COLOR,"","",intConsoleColumnsCount,"%d",stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase].stBase.chBase);
                                            if (isFreeSlotSetting(chSlots[chSelectedCiosSlot],&stCiosInstallMap)) {
                                                HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_BLACK;
                                            }
                                            else {
                                                printLabel(stLabelSettings[intTempValue].stLabelLocation.intRow,stLabelSettings[intTempValue].stLabelLocation.intColumn,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[intTempValue].chLabelSize,&stTexteLocation,"Slot %d already used in batch settings",chSlots[chSelectedCiosSlot]);
                                                HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_MAGENTA;
                                                intTempValue++;
                                            }
                                            if ((intReturnValue=getIosTypeColor(stSlotsMap[chSlots[chSelectedCiosSlot]].chIosType))!=CONSOLE_FONT_BLACK) {
                                                HIGHLIGHT_ITEM_COLOR=intReturnValue;
                                            }
                                            printListItem(stLabelSettings[3].stLabelLocation.intRow,stLabelSettings[3].stLabelLocation.intColumn,"",chSelectedCiosSlot,chSlotsCount,HIGHLIGHT_ITEM_COLOR,"","",intConsoleColumnsCount,"%d",chSlots[chSelectedCiosSlot]);
                                            printListItem(stLabelSettings[4].stLabelLocation.intRow,stLabelSettings[4].stLabelLocation.intColumn,"",chSelectedCiosRevision,chCiosAlternativeRevisionsCount,CONSOLE_FONT_BLACK,"","",intConsoleColumnsCount,"%d",intCiosAlternativeRevisions[chSelectedCiosRevision]);
                                            printLabel(stLabelSettings[5].stLabelLocation.intRow,stLabelSettings[5].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[5].chLabelSize,&stTexteLocation,"IOS%d-64-v%d.wad file required (downloadable with NUSD)",stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase].stBase.chBase,stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase].stBase.intBaseRevision);
                                            if (stSlotsMap[chSlots[chSelectedCiosSlot]].chIosType>NONE_IOS) {
                                                printLabel(stLabelSettings[intTempValue].stLabelLocation.intRow,stLabelSettings[intTempValue].stLabelLocation.intColumn,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[intTempValue].chLabelSize,&stTexteLocation,"%s detected in the slot %d will be overwritten",(stSlotsMap[chSlots[chSelectedCiosSlot]].chIosType==STUB_IOS)?"The IOS STUB":"The (c)IOS",chSlots[chSelectedCiosSlot]);
                                                intTempValue++;
                                            }
                                            if (stCiosInstallMap.chCiosCount) {
                                                setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d, Batch mode: %d cIOS",IOS_GetVersion(),IOS_GetRevision(),stCiosInstallMap.chCiosCount);
                                            }
                                            else {
                                                setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                            }
                                            while (intTempValue<9) {
                                                printLabel(stLabelSettings[intTempValue].stLabelLocation.intRow,stLabelSettings[intTempValue].stLabelLocation.intColumn,CONSOLE_FONT_BLACK,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[intTempValue].chLabelSize,&stTexteLocation,(intTempValue==6)?"none":"");
                                                intTempValue++;
                                            }
                                            VIDEO_WaitVSync();
                                            pressed=getPadsKeyPressed(&intExpectedPadsKeys[0],10,true);
                                            if (pressed) {
                                                switch (pressed) {
                                                    case PAD_TRIGGER_L:
                                                        stCiosInstallMap.lngScheduledCios=0;
                                                        stCiosInstallMap.chCiosCount=0;
                                                        break;
                                                    case PAD_BUTTON_Y:
                                                        if (stCiosInstallMap.chCiosCount) {
                                                            if (*strMountedDevice) {
                                                                setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_GREEN,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"%d item(s) saved in the %s:/%s file",writeNusScriptFile(&stCiosInstallMap,strMountedDevice),strMountedDevice,NUS_SCRIPT_FILE);
                                                            }
                                                            else {
                                                                setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"No FAT device detected");
                                                            }
                                                        }
                                                        else {
                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Nothing to save");
                                                        }
                                                        sleep(3);
                                                        break;
                                                    case PAD_BUTTON_X:
                                                        if (stCiosInstallMap.chCiosCount) {
                                                            if (*strMountedDevice) {
                                                                setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_GREEN,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"%d cIOS saved in the %s:/%s file",writeCiosConfigFile(&stCiosInstallMap,strMountedDevice),strMountedDevice,CIOS_CONFIG_FILE);
                                                            }
                                                            else {
                                                                setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"No FAT device detected");
                                                            }

                                                        }
                                                        else {
                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Nothing to save");
                                                        }
                                                        sleep(3);
                                                        break;
                                                    case PAD_BUTTON_START:
                                                        if (stCiosInstallMap.chCiosCount==addScheduledCios(&stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase],stCiosMaps[intSelectedCios].strGroupName,&chSlots[chSelectedCiosSlot],intCiosAlternativeRevisions[chSelectedCiosRevision],&stCiosInstallMap)) {
                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Slot %d already used in batch settings",chSlots[chSelectedCiosSlot]);
                                                        }
                                                        else if (stCiosInstallMap.chCiosCount) {
                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_GREEN,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"%s base %d rev%d added in batch",stCiosMaps[intSelectedCios].strGroupName,stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase].stBase.chBase,intCiosAlternativeRevisions[chSelectedCiosRevision]);
                                                            chCiosAlternativeRevisionsCount=setFreeCiosSettings(&intSelectedCios,&chSelectedCiosBase,&chSelectedCiosSlot,chSlots,chSlotsCount,&intCiosAlternativeRevisions[0],&chSelectedCiosRevision,stCiosMaps,intCiosCount,&stCiosInstallMap);
                                                        }
                                                        else {
                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Unable to add the selected cIOS in batch");
                                                        }
                                                        sleep(3);
                                                        break;
                                                    case PAD_BUTTON_DOWN:
                                                        intActiveMenu=getListItemId(intActiveMenu,1,4);
                                                        break;
                                                    case PAD_BUTTON_UP:
                                                        intActiveMenu=getListItemId(intActiveMenu,-1,4);
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
                                                                    chCiosAlternativeRevisionsCount=getCiosAlternativeRevisions(&intCiosAlternativeRevisions[0],stCiosMaps[intSelectedCios].intCiosRevision);
                                                                    chSelectedCiosBase=0;
                                                                    chSelectedCiosRevision=0;
                                                                }
                                                                break;
                                                            case 1:
                                                                if (pressed==PAD_BUTTON_LEFT) {
                                                                    chSelectedCiosBase=getListItemId(chSelectedCiosBase,-1,stCiosMaps[intSelectedCios].chCiosCount);
                                                                }
                                                                else {
                                                                    chSelectedCiosBase=getListItemId(chSelectedCiosBase,1,stCiosMaps[intSelectedCios].chCiosCount);
                                                                }
                                                                break;
                                                            case 2:
                                                                if (pressed==PAD_BUTTON_LEFT) {
                                                                    chSelectedCiosSlot=getListItemId(chSelectedCiosSlot,-1,chSlotsCount);
                                                                }
                                                                else {
                                                                    chSelectedCiosSlot=getListItemId(chSelectedCiosSlot,1,chSlotsCount);
                                                                }
                                                                break;
                                                            case 3:
                                                                if (pressed==PAD_BUTTON_LEFT) {
                                                                    chSelectedCiosRevision=getListItemId(chSelectedCiosRevision,-1,chCiosAlternativeRevisionsCount);
                                                                }
                                                                else {
                                                                    chSelectedCiosRevision=getListItemId(chSelectedCiosRevision,1,chCiosAlternativeRevisionsCount);
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
                                    }
                                    if (pressed==PAD_BUTTON_A) {
                                        if (stCiosInstallMap.chCiosCount==0) {
                                            addScheduledCios(&stCiosMaps[intSelectedCios].stCios[chSelectedCiosBase],stCiosMaps[intSelectedCios].strGroupName,&chSlots[chSelectedCiosSlot],intCiosAlternativeRevisions[chSelectedCiosRevision],&stCiosInstallMap);
                                        }
                                        if (stCiosInstallMap.chCiosCount) {
                                            pressed=0;
                                            clearConsole();
                                            chLevelIndex=3;
                                            printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,chLevelIndex,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
                                            printf("\n\n");
                                            printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"[*] SLOTS MAP");
                                            drawAlignedTable(ALIGN_CENTER,ALIGN_TOP,3,0,getConsoleRowsCount()-1,intConsoleColumnsCount,SLOTS_TABLE_COLUMNS,SLOTS_TABLE_ROWS,SLOTS_TABLE_CELL_WIDTH,SLOTS_TABLE_CELL_HEIGHT,SINGLE_BORDER,CONSOLE_FONT_WHITE,DEFAULT_FONT_BGCOLOR,&stTableSettings);
                                            stBlinkTexts.chBlinkTextsCount=0;
                                            for (intTempValue=0;intTempValue<CIOS_SLOTS_COUNT;intTempValue++) {
                                                intReturnValue=CIOS_BASE_SLOT+intTempValue;
                                                HIGHLIGHT_ITEM_COLOR=getIosTypeColor(stSlotsMap[intReturnValue].chIosType);
                                                updateTableCell(intTempValue/SLOTS_TABLE_COLUMNS,intTempValue % SLOTS_TABLE_COLUMNS,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,ALIGN_CENTER,ALIGN_MIDDLE,&stTableSettings,&stTexteLocation,"%03d",intReturnValue);
                                                if (!isFreeSlotSetting(intReturnValue,&stCiosInstallMap)) {
                                                    printBlinkText(stTexteLocation.intRow,stTexteLocation.intColumn,HIGHLIGHT_ITEM_COLOR,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,500,&stBlinkTexts.stBlinkTexts[stBlinkTexts.chBlinkTextsCount],"%03d",intReturnValue);
                                                    resetSavedPreviousCursorPosition();
                                                    stBlinkTexts.chBlinkTextsCount=stBlinkTexts.chBlinkTextsCount+1;
                                                }
                                            }
                                            selectTableCell(0,0,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                            printStyledText(4+SLOTS_TABLE_CELL_HEIGHT*SLOTS_TABLE_ROWS,stTableSettings.stTableLocation.intColumn,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,&stTexteLocation,"   ");
                                            printf(" STUB IOS   ");
                                            printStyledText(-1,-1,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,&stTexteLocation,"   ");
                                            printf(" ACTIVE (c)IOS   ");
                                            printBlinkText(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,500,&stBlinkTexts.stBlinkTexts[stBlinkTexts.chBlinkTextsCount],"2XX");
                                            stBlinkTexts.chBlinkTextsCount=stBlinkTexts.chBlinkTextsCount+1;
                                            printf(" (c)IOS to install\n\n");
                                            drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Slot infos: ",intConsoleColumnsCount-13,&stLabelSettings[0],&stTexteLocation);
                                            printf("\n");
                                            drawCommandsBar(4,true,&stCommandsBarSettings);
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[4],1,": Install");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[5],1,": Back");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[0],4,": Slot selection");
                                            addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[6],1,": Remove cIOS installation");
                                            setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                            runBlinkTexts(&stBlinkTexts);
                                            while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B)) {
                                                while (LWP_MutexLock(stBlinkTexts.mtxThread)) {}
                                                intTempValue=CIOS_BASE_SLOT+stTableSettings.chSelectedCellRow*SLOTS_TABLE_COLUMNS+stTableSettings.chSelectedCellColumn;
                                                if (stSlotsMap[intTempValue].chIosType) {
                                                    printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"(c)IOS%d rev%d ",intTempValue,stSlotsMap[intTempValue].intIosRevision);
                                                }
                                                else {
                                                    printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"free slot ");
                                                }
                                                if (!isFreeSlotSetting(intTempValue,&stCiosInstallMap)) {
                                                    intReturnValue=intTempValue-CIOS_BASE_SLOT;
                                                    printStyledText(stTexteLocation.intRow,stTexteLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,&stTexteLocation,"> %s base %d rev%d",stCiosInstallMap.stCiosInstallSettings[intReturnValue].strCiosName,stCiosInstallMap.stCiosInstallSettings[intReturnValue].stCios->stBase.chBase,stCiosInstallMap.stCiosInstallSettings[intReturnValue].intRevision);
                                                    resetSavedPreviousCursorPosition();
                                                }
                                                VIDEO_WaitVSync();
                                                LWP_MutexUnlock(stBlinkTexts.mtxThread);
                                                pressed=getPadsKeyPressed(&intExpectedPadsKeys[0],7,true);
                                                if (pressed) {
                                                    while (LWP_MutexLock(stBlinkTexts.mtxThread)) {}
                                                    switch (pressed) {
                                                        case PAD_BUTTON_DOWN:
                                                            selectTableCell(stTableSettings.chSelectedCellRow+1,stTableSettings.chSelectedCellColumn,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                            break;
                                                        case PAD_BUTTON_UP:
                                                            selectTableCell(stTableSettings.chSelectedCellRow-1,stTableSettings.chSelectedCellColumn,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                            break;
                                                        case PAD_BUTTON_LEFT:
                                                            selectTableCell(stTableSettings.chSelectedCellRow,stTableSettings.chSelectedCellColumn-1,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                            break;
                                                        case PAD_BUTTON_RIGHT:
                                                            selectTableCell(stTableSettings.chSelectedCellRow,stTableSettings.chSelectedCellColumn+1,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                            break;
                                                        case PAD_BUTTON_START:
                                                            if (stCiosInstallMap.chCiosCount>removeScheduledCios(intTempValue,&stCiosInstallMap)) {
                                                                for (intReturnValue=0;intReturnValue<stBlinkTexts.chBlinkTextsCount;intReturnValue++) {
                                                                    if (atoi(stBlinkTexts.stBlinkTexts[intReturnValue].strBlinkText)==intTempValue) {
                                                                        stBlinkTexts.stBlinkTexts[intReturnValue].stBlinkTimer.blnRunning=false;
                                                                        printStyledText(stBlinkTexts.stBlinkTexts[intReturnValue].stTextLocation.intRow,stBlinkTexts.stBlinkTexts[intReturnValue].stTextLocation.intColumn,stBlinkTexts.stBlinkTexts[intReturnValue].FONT_BGCOLOR,stBlinkTexts.stBlinkTexts[intReturnValue].FONT_FGCOLOR,stBlinkTexts.stBlinkTexts[intReturnValue].FONT_WEIGHT,&stTexteLocation,"%s",stBlinkTexts.stBlinkTexts[intReturnValue].strBlinkText);
                                                                        intReturnValue=stBlinkTexts.chBlinkTextsCount;
                                                                        resetSavedPreviousCursorPosition();
                                                                    }
                                                                }
                                                            }
                                                            break;
                                                    }
                                                    LWP_MutexUnlock(stBlinkTexts.mtxThread);
                                                }
                                                else {
                                                    pressed=PAD_BUTTON_B;
                                                }
                                            }
                                            destroyBlinkTexts(&stBlinkTexts);
                                            if (pressed==PAD_BUTTON_A) {
                                                lngScheduledCios=stCiosInstallMap.lngScheduledCios;
                                                chLevelIndex=0;
                                                while (lngScheduledCios) {
                                                    intTempValue=(unsigned char) (log(lngScheduledCios)/log(2));
                                                    if (strCacheFolder[0]) {
#if TESTING_CODE
                                                        snprintf(strModWiiTestFolder,sizeof(strModWiiTestFolder),"%s:%s/%d-%d",strMountedDevice,MODWII_TEST_FOLDER,stCiosInstallMap.stCiosInstallSettings[intTempValue].stCios->stBase.chBase,stCiosInstallMap.stCiosInstallSettings[intTempValue].intRevision);
                                                        mkdir(strModWiiTestFolder,S_IREAD|S_IWRITE);
#endif
                                                        snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s/%08x%08xv%d",strMountedDevice,MODWII_CACHE_FOLDER,IOS_MAJOR_TITLEID,stCiosInstallMap.stCiosInstallSettings[intTempValue].stCios->stBase.chBase,stCiosInstallMap.stCiosInstallSettings[intTempValue].stCios->stBase.intBaseRevision);
                                                        mkdir(strCacheFolder,S_IREAD | S_IWRITE);
                                                    }
                                                    clearConsole();
                                                    drawCommandsBar(0,true,&stCommandsBarSettings);
                                                    intReturnValue=intTempValue+CIOS_BASE_SLOT;
                                                    chLevelIndex++;
                                                    setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Installing %s base %d rev%d into slot %d (%d/%d)...",stCiosInstallMap.stCiosInstallSettings[intTempValue].strCiosName,stCiosInstallMap.stCiosInstallSettings[intTempValue].stCios->stBase.chBase,stCiosInstallMap.stCiosInstallSettings[intTempValue].intRevision,intReturnValue,chLevelIndex,stCiosInstallMap.chCiosCount);
                                                    printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,4,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
                                                    printf("\n\n");
                                                    if ((varout=patchmii(stCiosInstallMap.stCiosInstallSettings[intTempValue].stCios,intReturnValue,stCiosInstallMap.stCiosInstallSettings[intTempValue].intRevision,IOS_MAJOR_TITLEID,strCacheFolder,NAND_TEMP_DIRECTORY))) {
                                                        deInitNetwork();
                                                        break;
                                                    }
                                                    else {
                                                        stSlotsMap[intReturnValue].chIosType=255;
                                                    }
                                                    lngScheduledCios=lngScheduledCios-(unsigned long long) pow(2,intTempValue);
                                                }
                                                clearConsole();
                                                chLevelIndex=5;
                                                printLevelsBar(-1,-1,CONSOLE_FONT_CYAN,CONSOLE_FONT_GREEN,chLevelIndex,6,&stTexteLocation,"WELCOME","WORKING IOS","CIOS SETTINGS","SUMMARY","INSTALLATION","LOG");
                                                printf("\n\n");
                                                printStyledText(-1,-1,DEFAULT_FONT_BGCOLOR,CONSOLE_FONT_YELLOW,CONSOLE_FONT_BOLD,&stTexteLocation,"[*] INSTALLATION MAP");
                                                drawAlignedTable(ALIGN_CENTER,ALIGN_TOP,3,0,getConsoleRowsCount()-1,intConsoleColumnsCount,SLOTS_TABLE_COLUMNS,SLOTS_TABLE_ROWS,SLOTS_TABLE_CELL_WIDTH,SLOTS_TABLE_CELL_HEIGHT,SINGLE_BORDER,CONSOLE_FONT_WHITE,DEFAULT_FONT_BGCOLOR,&stTableSettings);
                                                for (intTempValue=0;intTempValue<CIOS_SLOTS_COUNT;intTempValue++) {
                                                    intReturnValue=CIOS_BASE_SLOT+intTempValue;
                                                    updateTableCell(intTempValue/SLOTS_TABLE_COLUMNS,intTempValue % SLOTS_TABLE_COLUMNS,DEFAULT_FONT_BGCOLOR,(stSlotsMap[intReturnValue].chIosType)?CONSOLE_FONT_YELLOW:DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,ALIGN_CENTER,ALIGN_MIDDLE,&stTableSettings,&stTexteLocation,"%03d",intReturnValue);
                                                }
                                                HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_GREEN;
                                                lngScheduledCios=stCiosInstallMap.lngScheduledCios;
                                                while (lngScheduledCios) {
                                                    intTempValue=(unsigned char) (log(lngScheduledCios)/log(2));
                                                    intReturnValue=intTempValue+CIOS_BASE_SLOT;
                                                    if (HIGHLIGHT_ITEM_COLOR==CONSOLE_FONT_GREEN) {
                                                        if (stSlotsMap[intReturnValue].chIosType==255) {
                                                            stSlotsMap[intReturnValue].chIosType=ACTIVE_IOS;
                                                            stSlotsMap[intReturnValue].intIosRevision=stCiosInstallMap.stCiosInstallSettings[intTempValue].intRevision;
                                                            HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_GREEN;
                                                        }
                                                        else {
                                                            HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_RED;
                                                        }
                                                    }
                                                    else {
                                                        HIGHLIGHT_ITEM_COLOR=CONSOLE_FONT_MAGENTA;
                                                    }
                                                    updateTableCell(intTempValue/SLOTS_TABLE_COLUMNS,intTempValue % SLOTS_TABLE_COLUMNS,HIGHLIGHT_ITEM_COLOR,(stSlotsMap[intReturnValue].chIosType)?CONSOLE_FONT_YELLOW:DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,ALIGN_CENTER,ALIGN_MIDDLE,&stTableSettings,&stTexteLocation,"%03d",intReturnValue);
                                                    lngScheduledCios=lngScheduledCios-(unsigned long long) pow(2,intTempValue);
                                                }
                                                selectTableCell(0,0,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                printStyledText(4+SLOTS_TABLE_CELL_HEIGHT*SLOTS_TABLE_ROWS,stTableSettings.stTableLocation.intColumn,CONSOLE_FONT_GREEN,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,&stTexteLocation,"   ");
                                                printf(" SUCCESS   ");
                                                printStyledText(-1,-1,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,&stTexteLocation,"   ");
                                                printf(" CANCELLED   ");
                                                printStyledText(-1,-1,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,&stTexteLocation,"   ");
                                                printf(" FAILED\n\n");
                                                drawLabel(-1,-1,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Installed cIOS: ",intConsoleColumnsCount-17,&stLabelSettings[0],&stTexteLocation);
                                                printf("\n");
                                                if (haveNandAccess()) {
                                                    drawCommandsBar(4,true,&stCommandsBarSettings);
                                                    addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[6],1,": Uninstall (c)IOS");
                                                }
                                                else {
                                                    drawCommandsBar(3,true,&stCommandsBarSettings);
                                                }
                                                addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[4],1,": Continue");
                                                addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[5],1,": Exit");
                                                addCommandsBarItem(&stCommandsBarSettings,&intExpectedPadsKeys[0],4,": Slot selection");
                                                setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                                pressed=0;
                                                while ((pressed!=PAD_BUTTON_A) && (pressed!=PAD_BUTTON_B)) {
                                                    intReturnValue=stTableSettings.chSelectedCellRow*SLOTS_TABLE_COLUMNS+stTableSettings.chSelectedCellColumn;
                                                    intTempValue=CIOS_BASE_SLOT+intReturnValue;
                                                    if (isFreeSlotSetting(intTempValue,&stCiosInstallMap)) {
                                                        if (stSlotsMap[intTempValue].chIosType) {
                                                            printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"(c)IOS%d rev%d ",intTempValue,stSlotsMap[intTempValue].intIosRevision);
                                                            if (stSlotsMap[intTempValue].chIosType==STUB_IOS) {
                                                                printLocatedText(stTexteLocation.intRow,stTexteLocation.intColumn,&stTexteLocation,"(STUB)");
                                                                resetSavedPreviousCursorPosition();
                                                            }
                                                        }
                                                        else {
                                                            printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"None");
                                                        }
                                                    }
                                                    else {
                                                        printLabel(stLabelSettings[0].stLabelLocation.intRow,stLabelSettings[0].stLabelLocation.intColumn,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,stLabelSettings[0].chLabelSize,&stTexteLocation,"%s base %d rev%d",stCiosInstallMap.stCiosInstallSettings[intReturnValue].strCiosName,stCiosInstallMap.stCiosInstallSettings[intReturnValue].stCios->stBase.chBase,stCiosInstallMap.stCiosInstallSettings[intReturnValue].intRevision);
                                                    }
                                                    VIDEO_WaitVSync();
                                                    pressed=getPadsKeyPressed(&intExpectedPadsKeys[0],stCommandsBarSettings.chCommandsCount+3,true);
                                                    if (pressed) {
                                                        switch (pressed) {
                                                            case PAD_BUTTON_START:
                                                                if (stSlotsMap[intTempValue].chIosType) {
                                                                    if ((intReturnValue=uninstallTitle(IOS_MAJOR_TITLEID,intTempValue))) {
                                                                        if (intReturnValue>0) {
                                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Critical (c)IOS uninstallation cancelled");
                                                                        }
                                                                        else {
                                                                            setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_RED,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"(c)IOS uninstallation failed");
                                                                        }
                                                                    }
                                                                    else {
                                                                        setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_GREEN,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"(c)IOS uninstallation successful");
                                                                        stSlotsMap[intTempValue].chIosType=NONE_IOS;
                                                                        stSlotsMap[intTempValue].intIosRevision=0;
                                                                        intReturnValue=intTempValue-CIOS_BASE_SLOT;
                                                                        updateTableCell(intReturnValue/SLOTS_TABLE_COLUMNS,intReturnValue % SLOTS_TABLE_COLUMNS,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,CONSOLE_FONT_BOLD,ALIGN_CENTER,ALIGN_MIDDLE,&stTableSettings,&stTexteLocation,"%03d",intTempValue);
                                                                    }
                                                                }
                                                                else {
                                                                    setStatusBar(&stCommandsBarSettings,CONSOLE_FONT_MAGENTA,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"No (c)IOS to uninstall");
                                                                }
                                                                sleep(3);
                                                                setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Current IOS: %d v%d",IOS_GetVersion(),IOS_GetRevision());
                                                                break;
                                                            case PAD_BUTTON_DOWN:
                                                                selectTableCell(stTableSettings.chSelectedCellRow+1,stTableSettings.chSelectedCellColumn,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                                break;
                                                            case PAD_BUTTON_UP:
                                                                selectTableCell(stTableSettings.chSelectedCellRow-1,stTableSettings.chSelectedCellColumn,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                                break;
                                                            case PAD_BUTTON_LEFT:
                                                                selectTableCell(stTableSettings.chSelectedCellRow,stTableSettings.chSelectedCellColumn-1,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                                break;
                                                            case PAD_BUTTON_RIGHT:
                                                                selectTableCell(stTableSettings.chSelectedCellRow,stTableSettings.chSelectedCellColumn+1,CONSOLE_FONT_YELLOW,CONSOLE_FONT_YELLOW,&stTableSettings);
                                                                break;
                                                        }
                                                    }
                                                    else {
                                                        pressed=PAD_BUTTON_B;
                                                    }
                                                }
                                                stCiosInstallMap.lngScheduledCios=0;
                                                stCiosInstallMap.chCiosCount=0;
                                            }
                                            else {
                                                pressed=PAD_BUTTON_A;
                                            }
                                        }
                                        else {
                                            pressed=PAD_BUTTON_B;
                                            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError getting selected cIOS\n");
                                        }
                                    }
                                }
                            } while (pressed!=PAD_BUTTON_B);
                            free(stBlinkTexts.stBlinkTexts);
                            stBlinkTexts.stBlinkTexts=NULL;
                        }
                    }
                    unmountNandFs();
                    deInitNetwork();
                    free(stCiosMaps);
                    stCiosMaps=NULL;
                    freeExternalCiosModules();
                }
                else {
                    varout=1;
                }
            }
            else {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError getting cIOS maps\n");
            }
            if (*strMountedDevice) {
                if (blnRemoveCacheFolder) {
                    snprintf(strCacheFolder,sizeof(strCacheFolder),"%s:%s",strMountedDevice,MODWII_CACHE_FOLDER);
                    removeDirectory(strCacheFolder);
                }
                unmountFatDevice(strMountedDevice);
                free(strMountedDevice);
                strMountedDevice=NULL;
            }
        }
        setStatusBar(&stCommandsBarSettings,DEFAULT_FONT_BGCOLOR,DEFAULT_FONT_FGCOLOR,DEFAULT_FONT_WEIGHT,"Exiting...");
        WPAD_Shutdown();
    }
    else {
        printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError getting slots map\n");
    }
    fflush(stdout);
    VIDEO_WaitVSync();
    return varout;
}
