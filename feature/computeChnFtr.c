#include"./myCLib.h"
#include"constructFeatureIntHist.h"
#include"computeChnFtr.h"


void getColorFeature(const float *src, int srcC, rect r, float *pColorFtr){

	int j = 0;
	int kd = 0;
	int ks = r.y*srcC + r.x;

	for (j = 0; j <= r.height - 1; ++j){

		memcpy(pColorFtr + kd, src + ks, sizeof(src[0])*r.width);
		kd += r.width;
		ks += srcC;
	}
}

void getGradMagFeature(const float *pMag, int srcC, rect r, float *pMagFtr){

	int j = 0;
	int kd = 0;
	int ks = r.y*srcC + r.x;

	for (j = 0; j <= r.height - 1; ++j){

		memcpy(pMagFtr + kd, pMag + ks, sizeof(pMag[0])*r.width);
		kd += r.width;
		ks += srcC;
	}
}

static void getGradHistFeature(float pSrc[][NUM_ORIENT], int srcC, rect r, float *pFtr){


	int j = 0;
	int kd = 0;
	int ks = r.y*srcC + r.x;
	const int KD_PLUS = r.width*NUM_ORIENT;

	for (j = 0; j <= r.height - 1; ++j){

		memcpy(pFtr + kd, &pSrc[ks][0], sizeof(pFtr[0])*KD_PLUS);
		kd += KD_PLUS;
		ks += srcC;
	}
}

extern float yShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)];
//extern float uShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)];
//extern float vShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)];//cahnged
extern float gradMagShrink[MAX_IN_IMG_SIZE / SHRINK / SHRINK];
extern float gradHistShrink[(MAX_IN_IMG_R / BIN_SIZE)*(MAX_IN_IMG_C / BIN_SIZE)][NUM_ORIENT];

int getFtrDim(int rows, int cols){
	return  (rows / SHRINK)*(cols / SHRINK) +
			(rows / SHRINK)*(cols / SHRINK) +
			(rows / BIN_SIZE)* (cols / BIN_SIZE)*NUM_ORIENT;
}

void computeChnFtr(const UInt8 *pSrc, int srcR, int srcC, rect r, float *pChnFtr, int ftrDim){

	rect rShrk ;
	rShrk.x = r.x/SHRINK;
	rShrk.y = r.y/SHRINK;
	rShrk.width = r.width/SHRINK;
	rShrk.height = r.height/SHRINK;
	int shrkC = srcC / SHRINK;

	int gradHistFtrLen = (r.width / BIN_SIZE) * (r.height / BIN_SIZE) * NUM_ORIENT;
	int gradMagFtrLen = rShrk.width *rShrk.height;
	int colorFtrLen = rShrk.width *rShrk.height;

	UTILS_assert(ftrDim <= MAX_CHN_FTR_DIM);
	UTILS_assert(colorFtrLen + gradMagFtrLen + gradHistFtrLen == ftrDim);//changed
	UTILS_assert(rShrk.x + rShrk.width <= MAX_IN_IMG_C / SHRINK);
	UTILS_assert(rShrk.y + rShrk.height <= MAX_IN_IMG_R / SHRINK);

	getGradHistFeature(gradHistShrink, shrkC, rShrk, pChnFtr);
	getGradMagFeature(gradMagShrink, shrkC, rShrk, pChnFtr + gradHistFtrLen);
	getColorFeature(yShrink, shrkC, rShrk, pChnFtr + gradHistFtrLen + gradMagFtrLen);//changed
//	getColorFeature(uShrink, shrkC, rShrk, pChnFtr + gradHistFtrLen + gradMagFtrLen + colorFtrLen);//changed
//	getColorFeature(vShrink, shrkC, rShrk, pChnFtr + gradHistFtrLen + gradMagFtrLen + colorFtrLen*2);//changed
}
