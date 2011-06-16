#ifndef _LIBMATH_H_
#define _LIBMATH_H_
#include <gctypes.h>
bool isInRange(double dbValue,double dbMinRangeValue,double dbMaxRangeValue,bool blnIncludeMinRange,bool blnIncludeMaxRange);
double getRoundNumber(double dbValue);
double getMinValue(double dbValue1,double dbValue2);
double getMaxValue(double dbValue1,double dbValue2);
#endif
