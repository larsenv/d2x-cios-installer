#include <stdlib.h>
#include <math.h>
#include "libarray.h"
#include "libgeometry.h"
#include "libmath.h"
double getAlignedPolyContainerPosition(const double *dbPolyPointsPositions,unsigned int intPointsCount,double dbContainerMinPosition,double dbContainerMaxPosition,enum ALIGNEMENTS ALIGNEMENT) {
double varout;
	switch(ALIGNEMENT) {
		case ALIGN_LEFT:
			varout=dbContainerMinPosition;
			break;
		case ALIGN_CENTER:
            varout=dbContainerMinPosition+(dbContainerMaxPosition-dbContainerMinPosition-dbPolyPointsPositions[getArrayMaxValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))]+dbPolyPointsPositions[getArrayMinValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))])/2;
			break;
		case ALIGN_RIGHT:
			varout=dbContainerMaxPosition-dbPolyPointsPositions[getArrayMaxValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))]+dbPolyPointsPositions[getArrayMinValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))];
			break;
        default:
            varout=dbContainerMinPosition+dbPolyPointsPositions[getArrayMinValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))];
            break;
	};
	return varout;
}
double getPolyContainerPosition(const double *dbPolyPointsPositions,unsigned int intPointsCount,double dbContainerMinPosition,double dbContainerMaxPosition,double dbPositionType) {
double varout=dbContainerMinPosition+dbPolyPointsPositions[getArrayMinValueIndex((const void *) dbPolyPointsPositions,intPointsCount,sizeof(double))];
    if (fabs(dbPositionType)<1) {
        varout=dbContainerMinPosition+dbPositionType*(dbContainerMaxPosition-dbContainerMinPosition);
    }
    else if (isInRange(dbPositionType,ALIGN_NONE,-1,true,true)) {
        varout=getAlignedPolyContainerPosition(dbPolyPointsPositions,intPointsCount,dbContainerMinPosition,dbContainerMaxPosition,(enum ALIGNEMENTS) (int) getRoundNumber(dbPositionType));
    }
    return varout;
}
