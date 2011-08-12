#include <math.h>
#include <stdlib.h>
#include "libmath.h"
bool isInRange(double dbValue,double dbMinRangeValue,double dbMaxRangeValue,bool blnIncludeMinRange,bool blnIncludeMaxRange) {
bool varout;
	if (blnIncludeMinRange) {
		varout=(dbValue>=dbMinRangeValue);
	}
	else {
		varout=(dbValue>dbMinRangeValue);
	}
	if (varout) {
		if (blnIncludeMaxRange) {
			varout=(dbValue<=dbMaxRangeValue);
		}
		else {
			varout=(dbValue<dbMaxRangeValue);
		}
	}
	return varout;
}
double getRoundNumber(double dbValue) {
    return floor(dbValue+0.5);
}
double getMinValue(double dbValue1,double dbValue2) {
    return (dbValue1<dbValue2)?dbValue1:dbValue2;
}
double getMaxValue(double dbValue1,double dbValue2) {
    return (dbValue1>dbValue2)?dbValue1:dbValue2;
}
double getReducedRangedNumber(double dbValue,double dbMinRange,double dbMaxRange) {
double dbRangeInterval=dbMaxRange-dbMinRange+1,dbIncrementFactor=dbMinRange-dbValue;
    if (dbIncrementFactor) {
        dbIncrementFactor=dbIncrementFactor/abs(dbIncrementFactor);
    }
    while (!isInRange(dbValue,dbMinRange,dbMaxRange,true,true)) {
        dbValue=dbValue+dbIncrementFactor*dbRangeInterval;
    }
    return dbValue;
}
