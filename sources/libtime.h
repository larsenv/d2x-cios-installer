#ifndef _LIBTIME_H_
#define _LIBTIME_H_
#include <time.h>
#include <stdbool.h>
struct stTimer {
    clock_t clkStartTime;
    unsigned short int intInterval;
    bool blnRunning;
};
void initTimer(struct stTimer *stTimerSettings,unsigned short int intTimerInterval);
bool isExpiredTimer(struct stTimer *stTimerSettings);
#endif
