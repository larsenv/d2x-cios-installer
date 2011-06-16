#include "image.h"
IMGCTX getPngImageRessources(const void *imgData,PNGUPROP *imgProperties) {
IMGCTX varout;
    if ((varout=PNGU_SelectImageFromBuffer(imgData))) {
        if (PNGU_GetImageProperties(varout,imgProperties)!=PNGU_OK) {
            PNGU_ReleaseImageContext(varout);
            varout=NULL;
        }
	}
    return varout;
}
bool drawPng(const void *imgData,u32 intLeft,u32 intTop,u32 intWidth,u32 intHeight,void *pFramebuffer) {
IMGCTX imgContext;
PNGUPROP imgProperties;
bool varout=true;
    if ((imgContext=getPngImageRessources(imgData,&imgProperties))) {
        PNGU_DECODE_TO_COORDS_YCbYCr(imgContext,intLeft,intTop,imgProperties.imgWidth,imgProperties.imgHeight,intWidth,intHeight,pFramebuffer);
        PNGU_ReleaseImageContext(imgContext);
    }
    else {
        varout=false;
    }
	return varout;
}
