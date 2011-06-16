#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_
#include <gctypes.h>
enum STORAGE_DEVICES {
    SD_STORAGE=1,
    USB_STORAGE=2
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
void unmountFatFs(char *strDevice);
char *mountFatFs(unsigned int intFatDevicePriority);
#endif
