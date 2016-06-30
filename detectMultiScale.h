#ifndef DETECT_MULTI_SCALE_H_
#define DETECT_MULTI_SCALE_H_

#include"myCLib.h"

typedef struct DetectOpt{

	int winR;
	int winC;
	int winStrideR;
	int winStrideC;
	float scale;
	BOOL isUseDp;  // 是否使用视差
	BOOL isPostPro;// 是否后处理

}DetectOpt;

void detectMultiClasses(AlgLink_OsdObj *restrict pObj,
		UInt8 *restrict pSrc, Uint8 *restrict pU, Uint8 *restrict pV, int imgR, int imgC,
		Uint8 *restrict disp, DetectOpt detectOpt[],BOOL isPostPro);

#endif
