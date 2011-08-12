#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_
#include <gctypes.h>
enum STORAGE_DEVICES {
    SD_STORAGE_DEVICE=1,
    USB_STORAGE_DEVICE=2,
    ALL_DEVICES=SD_STORAGE_DEVICE | USB_STORAGE_DEVICE
};
bool isMountedSD();
void unmountSD();
bool mountSD();
bool isMountedUSB();
void unmountUSB();
bool mountUSB();
bool isMountedNANDFS();
void unmountNandFs();
bool mountNandFs();
void unmountFatDevice(char *strDevice);
char *mountFatDevice(unsigned char chFatDevicesPriority);
#endif
