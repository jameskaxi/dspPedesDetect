#include"./myCLib.h"
#include"constructFeatureIntHist.h"
#include"magAndOriLut.h"

float yShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)] = { .0f };
//float uShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)] = { .0f };
//float vShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)] = { .0f };
float gradMagShrink[(MAX_IN_IMG_R / SHRINK)* (MAX_IN_IMG_C / SHRINK)] = { .0f };
float gradHistShrink[(MAX_IN_IMG_R / BIN_SIZE)*(MAX_IN_IMG_C / BIN_SIZE)][NUM_ORIENT] = { .0f };

static float gradHistIntArr[NUM_ORIENT][MAX_IN_IMG_R*MAX_IN_IMG_C] = { .0f };
static float chnAccu[(MAX_IN_IMG_R)*(MAX_IN_IMG_C / SHRINK)] = { .0f };

static float gradMag[MAX_IN_IMG_SIZE] = { .0f };
static float gradOri[MAX_IN_IMG_SIZE] = { .0f };

static void computeGradient(const UInt8 *restrict src, int imgR, int imgC,
		                   float *restrict pGradMag, float *restrict pOri){

	static float ysub[MAX_IN_IMG_SIZE] = { 0 };
	static float xsub[MAX_IN_IMG_SIZE] = { 0 };

	int y = 0, x = 0;
	int pos = 0;

	/*gradient of x orientation,replication padding*/
	for (y = 0; y < imgR; y++){
		for (x = 1; x < imgC - 1; x++)
		{
			pos = y*imgC + x;
			xsub[pos] = src[pos + 1] - src[pos - 1];
			xsub[pos] /= 2.0;
		}
	}
	for (y = 0; y < imgR; y++)
	{
		pos = y*imgC;
		xsub[pos] = src[pos + 1] - src[pos];
		xsub[pos + imgC - 1] = src[pos + imgC - 1] - src[pos + imgC - 2];
	}

	/*gradient of y orientation，replication padding*/
	for (y = 1; y < imgR - 1; y++){
		for (x = 0; x < imgC; x++)
		{
			pos = y*imgC + x;
			ysub[pos] = src[pos + imgC] - src[pos - imgC];
			ysub[pos] /= 2.0;
		}
	}
	for (x = 0; x < imgC; x++)
	{
		pos = imgC*(imgR - 1);
		ysub[x] = src[imgC + x] - src[x];
		ysub[pos + x] = src[pos + x] - src[pos - imgC + x];
	}

	int i = 0;
	int imgRMpyC = imgR*imgC;

	for (i = 0; i < imgRMpyC; ++i)
	{
		pGradMag[i] = sqrt(ysub[i] * ysub[i] + xsub[i] * xsub[i]); //magnitude of gradient
		pOri[i] = atan2(ysub[i], xsub[i]) + 3.14159*(ysub[i] < 0); //angle of gradient 3.14159

	}
}

void computeGradient2(const UInt8* restrict src, int imgR, int imgC, float *restrict pGradMag, float *restrict pOri){

	static short ysub[MAX_IN_IMG_SIZE] = { 0 };
	static short xsub[MAX_IN_IMG_SIZE] = { 0 };
	//unsigned short width = imgC;
	//unsigned short height = imgR;
	int y = 0, x = 0;
	int pos = 0;
	/*gradient of x orientation,replication padding*/
	for (y = 0; y < imgR; y++){
		for (x = 1; x < imgC - 1; x++)
		{
			pos = y*imgC + x;
			xsub[pos] = src[pos + 1] - src[pos - 1];
		}
	}
	for (y = 0; y < imgR; y++)
	{
		pos = y*imgC;
		xsub[pos] = src[pos + 1] - src[pos];
		xsub[pos + imgC - 1] = src[pos + imgC - 1] - src[pos + imgC - 2];
	}

	/*gradient of y orientation，replication padding*/
	for (y = 1; y < imgR - 1; y++){
		for (x = 0; x < imgC; x++)
		{
			pos = y*imgC + x;
			ysub[pos] = src[pos + imgC] - src[pos - imgC];
		}
	}
	for (x = 0; x < imgC; x++)
	{
		pos = imgC*(imgR - 1);
		ysub[x] = src[imgC + x] - src[x];
		ysub[pos + x] = src[pos + x] - src[pos - imgC + x];
	}

	int i = 0;
	int imgRMpyC = imgR*imgC;

	for (i = 0; i < imgRMpyC; ++i){
		pGradMag[i] = magLUT[ysub[i] + 255][xsub[i] + 255];
		pOri[i]     = oriLUT[ysub[i] + 255][xsub[i] + 255];
	}
}

static void gradQuantize(const float *restrict pO, const float *restrict pM, int row, int col){
	int i = 0, j = 0;
	float alpha = .0f;
	int idx = 0;
	static const float O_MULT = NUM_ORIENT / 3.1416f;
	static const double S = 1.0 / (BIN_SIZE*BIN_SIZE);
	float mag = .0f;
	int rpos = 0, pos = 0;
	for (j = 0; j < row; ++j, rpos += col){
		for (i = 0; i < col; ++i){

			pos = rpos + i;
			alpha = pO[pos] * O_MULT;
			idx = (int)alpha;
			alpha = alpha - idx;
			//if (idx < 0) idx += NUM_ORIENT;
			idx += NUM_ORIENT*(idx < 0);
			//else if (idx >= NUM_ORIENT) idx -= NUM_ORIENT;
			idx -= NUM_ORIENT*(idx >= NUM_ORIENT);
			mag = pM[pos] * S;
			//gradHistIntArr[j + 1][i + 1][idx] = mag * (1 - alpha);
			gradHistIntArr[idx][pos] = mag * (1 - alpha);
			idx++; //if (idx >= NUM_ORIENT) idx = 0;
			idx *= (idx < NUM_ORIENT);
			gradHistIntArr[idx][pos] = mag * alpha;
			//gradHistIntArr[j + 1][i + 1][idx] = mag * alpha;
		}
	}
}

static void fltColAccum(const float *restrict pIn, int srcR, int srcC, float *restrict pOut){
	int i = 0, k = 0;
	int len = srcR*srcC;
	for (i = 0; i < len; i += 4){
		pOut[k++] = pIn[i] + pIn[i + 1] + pIn[i + 2] + pIn[i + 3];
	}
}

static void ghRowAccum(const float *restrict pIn, int srcR, int srcC, float (*pOut)[NUM_ORIENT], int ofs){

	int i = 0, j = 0;
	int pos = 0;
	int cp = 0;
	//const double S = 1.0 / 16.f;
	const int OFS = srcC << 2;

	for (j = 0; j < srcR; j += 4){
		for (i = 0; i < srcC; ++i){
			cp = pos + i;
			(*pOut)[ofs] = pIn[cp]; cp += srcC;
			(*pOut)[ofs] += pIn[cp]; cp += srcC;
			(*pOut)[ofs] += pIn[cp]; cp += srcC;
			(*pOut++)[ofs] += pIn[cp];
			//(*pOut++)[ofs] *= S;
		}
		pos += OFS;
	}
}
static void computeGradHistShrink(const float *restrict pOri, const float *restrict pMag, int srcR, int srcC){
	int i = 0;
	memset(gradHistIntArr, 0, sizeof(gradHistIntArr));

	gradQuantize(pOri, pMag, srcR, srcC);

	float *p = NULL;
	float(*pa)[NUM_ORIENT];
	for (i = 0; i < NUM_ORIENT; ++i){
		p = &gradHistIntArr[i][0];
		fltColAccum(p, srcR, srcC, chnAccu);
		pa = &gradHistShrink[0];
		ghRowAccum(chnAccu, srcR, srcC / 4, pa,i);
	}
}

static void colorAccum(const UInt8 *restrict pIn, int srcR, int srcC, float *restrict pOut){
	int i = 0, k = 0;
	int len = srcR*srcC;
	for (i = 0; i < len; i += 4){
		pOut[k++] = pIn[i] + pIn[i + 1] +pIn[i + 2] + pIn[i + 3];
	}
}

static void rowAccum(const float *restrict pIn, int srcR, int srcC, float *restrict pOut){
	int i = 0, j = 0, k = 0;
	int pos = 0;
	int cp = 0;
	const double S = 1.0 / 16.f;
	const int OFS = srcC << 2;

	for (j = 0; j < srcR; j += 4){
		for (i = 0; i < srcC; ++i){
			cp = pos + i;
			pOut[k] = pIn[cp]; cp += srcC;
			pOut[k] += pIn[cp]; cp += srcC;
			pOut[k] += pIn[cp]; cp += srcC;
			pOut[k] += pIn[cp];
			pOut[k++] *= S;
		}
		pos += OFS;
	}
}

static void computeGradMagShrink(const float *pMag, int srcR, int srcC){
	fltColAccum(pMag, srcR, srcC, chnAccu);
	rowAccum(chnAccu, srcR, srcC / 4, gradMagShrink);
}

static void computeColorShrink(
		const Uint8 *restrict pY, const Uint8 *restrict pU ,const Uint8 *restrict pV, int srcR, int srcC)
{
	colorAccum(pY, srcR, srcC, chnAccu);
	rowAccum(chnAccu, srcR, srcC / 4, yShrink);

//	colorAccum(pU, srcR, srcC, chnAccu);
//	rowAccum(chnAccu, srcR, srcC / 4, uShrink);
//
//	colorAccum(pV, srcR, srcC, chnAccu);
//	rowAccum(chnAccu, srcR, srcC / 4, vShrink);
}

void constructFeatureIntHist(AlgLink_OsdObj *restrict pObj,
		const UInt8 *restrict pY, const Uint8 *restrict pU, const Uint8 *restrict pV,
		int srcR, int srcC){

//	UTILS_assert(srcR == 240 && srcC == 320);
	UTILS_assert(SHRINK == 4 && BIN_SIZE == 4);
	//int startTime,endTime;

	//startTime = Utils_getCurTimeInMsec();
	computeGradient2(pY, srcR, srcC, gradMag, gradOri);
	//endTime = Utils_getCurTimeInMsec();
	//Vps_printf("++++++[computeGradient2] elapsed time: %d ++++++", endTime-startTime);


	//startTime = Utils_getCurTimeInMsec();
	computeGradHistShrink(gradOri, gradMag, srcR, srcC);//grad hist
	//endTime = Utils_getCurTimeInMsec();
	//Vps_printf("++++++[computeGradHistShrink] elapsed time: %d ++++++", endTime-startTime);


	//startTime = Utils_getCurTimeInMsec();
	computeGradMagShrink(gradMag, srcR, srcC); //gradient magnitude
	//endTime = Utils_getCurTimeInMsec();
	//Vps_printf("++++++[computeGradMagShrink] elapsed time: %d ++++++", endTime-startTime);


	//startTime = Utils_getCurTimeInMsec();
	computeColorShrink(pY, pU ,pV, srcR, srcC); //y u v channel
	//endTime = Utils_getCurTimeInMsec();
	//Vps_printf("++++++[computeColorShrink] elapsed time: %d ++++++", endTime-startTime);
}
