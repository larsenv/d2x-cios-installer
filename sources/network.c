#include <network.h>
#include <sys/errno.h>
#include <unistd.h>
#include "debug.h"
#include "http.h"
#include "network.h"
static int intNotInitNetwork=1;
int initNetwork(time_t lngInitNetworkTimeout) {
//return 0 if success
int varout=1;
time_t lngCurrentTime=time(NULL);
    while (varout) {
        varout=net_init();
        if (varout<0) {
            if (varout!=-EAGAIN) {
                printDebugMsg(ERROR_DEBUG_MESSAGE,"\nnet_init failed");
                break;
            }
        }
        usleep(100000);
        if((time(NULL)-lngCurrentTime)>lngInitNetworkTimeout) {
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nTimeout reached");
            break;
        }
    }
    sleep(1);
    return varout;
}
void *getHttpFileContent(const char *strFileUrl,size_t *intDownloadBytesCount,time_t lngInitNetworkTimeout,unsigned char chRetry) {
void *varout=NULL;
u32 http_status;
    if (intNotInitNetwork) {
        if ((intNotInitNetwork=initNetwork(lngInitNetworkTimeout))) {
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nInit Network failed: %d",intNotInitNetwork);
        }
    }
    if (!intNotInitNetwork) {
        while ((chRetry) && (!http_request(strFileUrl, (u32) (1 << 31)))) {
            chRetry--;
            printDebugMsg(ERROR_DEBUG_MESSAGE,"\nError making http request");
            sleep(1);
        }
        if (chRetry) {
            http_get_result(&http_status,(u8 **) &varout,intDownloadBytesCount);
        }
    }
    return varout;
}
bool isInitNetwork() {
    return (intNotInitNetwork==0);
}
void deInitNetwork() {
    if (isInitNetwork()) {
        net_deinit();
    }
}
