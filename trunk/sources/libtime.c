#include "libtime.h"
void initTimer(struct stTimer *stTimerSettings,unsigned short int intTimerInterval) {
    stTimerSettings->clkStartTime=clock();
    stTimerSettings->intInterval=intTimerInterval;
    stTimerSettings->blnRunning=true;
}
bool isExpiredTimer(struct stTimer *stTimerSettings) {
clock_t clkTime=clock();
bool varout=false;
    if ((double)(clkTime-stTimerSettings->clkStartTime)*1000/CLOCKS_PER_SEC<stTimerSettings->intInterval) {
        varout=true;
        stTimerSettings->clkStartTime=clkTime;
    }
    return varout;
}
