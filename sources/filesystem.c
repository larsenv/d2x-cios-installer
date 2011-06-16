#include <fat.h>
#include <string.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <ogc/system.h>
#include <ogc/isfs.h>
#include "filesystem.h"
#include "debug.h"
#include "libstring.h"
#include <ogc/usbstorage.h>
bool isMountedSd=false;
bool isMountedUsb=false;
bool isMountedNandFs=false;
bool isMountedSD() {
    return isMountedSd;
}
void unmountSD() {
    if (isMountedSd) {
        fatUnmount("sd:/");
        __io_wiisd.shutdown();
        isMountedSd=false;
    }
}
bool mountSD() {
    if (!isMountedSd) {
        isMountedSd=fatMountSimple("sd",&__io_wiisd);
    }
    return isMountedSd;
}
bool isMountedUSB() {
    return isMountedUsb;
}
void unmountUSB() {
    if (isMountedUsb) {
        fatUnmount("usb:/");
        __io_usbstorage.shutdown();
        isMountedUsb=false;
    }
}
bool mountUSB() {
unsigned char chRetry=10;
    if (!isMountedUsb) {
        fatMountSimple("usb",&__io_usbstorage);
        if ((isMountedUsb=__io_usbstorage.isInserted())) {
            while ((chRetry) && ((isMountedUsb=fatMountSimple("usb",&__io_usbstorage))==false)) {
                sleep(1);
                chRetry--;
            }
        }
    }
    return isMountedUsb;
}
bool isMountedNANDFS() {
    return isMountedNandFs;
}
void unmountNandFs() {
    if (isMountedNandFs) {
        ISFS_Deinitialize();
        isMountedNandFs=false;
    }
}
bool mountNandFs() {
int intReturnValue;
    if (!isMountedNandFs) {
        if ((intReturnValue=ISFS_Initialize())) {
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nISFS_Initialize failed: %d",intReturnValue);
        }
        else {
            isMountedNandFs=true;
        }
    }
    return isMountedNandFs;
}
void unmountFatFs(char *strDevice) {
    if (!strcmp(strDevice,"sd")) {
        unmountSD();
    }
    else if (!strcmp(strDevice,"usb")) {
        unmountUSB();
    }
}
char *mountFatFs(unsigned int intFatDevicePriority) {
char *varout=NULL;
unsigned char chDeviceId;
    while (((chDeviceId=intFatDevicePriority%10)) && (!varout)) {
        switch (chDeviceId) {
            case SD_STORAGE:
                if (mountSD()) {
                    varout=strdup("sd");
                }
                break;
            case USB_STORAGE:
                if (mountUSB()) {
                    varout=strdup("usb");
                }
                break;
        }
        intFatDevicePriority=intFatDevicePriority/10;
    }
    return (varout)?varout:strdup("");
}
