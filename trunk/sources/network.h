#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <time.h>
#include <gctypes.h>
int initNetwork(time_t lngInitNetworkTimeout);
void *getHttpFileContent(const char *strFileUrl,size_t *intDownloadBytesCount,time_t lngInitNetworkTimeout,unsigned char chRetry);
bool isInitNetwork();
void deInitNetwork();
#endif
