#include"myCLib.h"
#include"setting.h"
#include"feature/computeChnFtr.h"
#include"feature/constructFeatureIntHist.h"
#include"reduceCandidate.h"
#include"detectMultiScale.h"

#include "algLink_priv.h"
#include "classify/AdaClassify.h"
#include "postPro/postPro.h"
#include "PostRoad.h"


#define FOUND_MAX_NUM   (500)

static float chnFtrForDetect[MAX_CHN_FTR_DIM] = {.0f};

void detectRoad(UInt8 *restrict pSrc, int imgR, int imgC,DetectOpt detectOpt,
		Bbox *restrict bb, int *restrict bbNum, float scores[], int *restrict scoreNum)
{
	int winR, winC, winStrideR, winStrideC;
	winR = detectOpt.winR;
	winC = detectOpt.winC;
	winStrideR = detectOpt.winStrideR;
	winStrideC = detectOpt.winStrideC;
	int trueWinCount = 0, totalWinCount = 0;

	int chnFtrDim = getFtrDim(winR,winC);

	int pixelXOffset = imgC % winC;
	int pixelYOffset = imgR % winR;
	int numXOffset = 0;
	int numYOffset = 2;

	int winStepNumR = (imgR - winR) / winStrideR + 1; //tip：int div int is int
	int winStepNumC = (imgC - winC) / winStrideC + 1;

	int wr = 0, wc = 0;
	for (wr = numYOffset; wr < winStepNumR; wr++)
	{
		int startR = wr*winStrideR + pixelYOffset;
		for (wc = numXOffset; wc < winStepNumC; wc++)
		{
			int startC = wc*winStrideC + pixelXOffset;
			rect r;
			r.x = startC;
			r.y = startR;
			r.width = winC;
			r.height = winR;
			computeChnFtr(pSrc, imgR, imgC, r, chnFtrForDetect, chnFtrDim);
			float score = adaboostPredictRoad(chnFtrForDetect);
			scores[totalWinCount] = score;
			if (score > 0)
			{
				bb[*bbNum].pos = r;
				bb[*bbNum].obj = ROAD;
				bb[*bbNum].score = score;
				(*bbNum)++;
				trueWinCount++;
			}
			totalWinCount++;
		}
	}
	*scoreNum = totalWinCount;
}

void detectPedes(UInt8 *restrict pSrc, int imgR, int imgC,
		DetectOpt detectOpt, Bbox *restrict bb, int *restrict bbNum)
{
	int trueWinCount = 0, totalWinCount = 0,computedWinCount = 0;
	int zi = 0;

	const int OPT[3][4] = { //here
		{ 128, 64, 8, 8},
		{ 144, 72, 8, 8},
		{ 152, 76, 8, 8}
	};

	float ratio = detectOpt.scale;
	for (zi = 0; zi < 3; zi++){
		int newWinR =     OPT[zi][0];
		int newWinC =     OPT[zi][1];
		int newWinStepR = OPT[zi][2];
		int newWinStepC = OPT[zi][3];

		int chnFtrDim = getFtrDim(newWinR,newWinC);
		int startR = 0;
		while (startR + newWinR <= imgR){
			int startC = 0;
			while (startC + newWinC <= imgC){
				totalWinCount++;
				BOOL likely = TRUE;
				if(detectOpt.isUseDp){
				likely = judgeCandidate((startR + 0.5)*ratio - 0.5, (startC + 0.5)*ratio - 0.5,
										newWinR*ratio, newWinC*ratio);//here
				}

				if (likely){
					computedWinCount++;
					rect r;
					r.x = startC;
					r.y = startR;
					r.width = newWinC;
					r.height = newWinR;
					computeChnFtr(pSrc, imgR, imgC, r, chnFtrForDetect,chnFtrDim);

					float score = adaPersonMultiScale(chnFtrForDetect, chnFtrDim,zi); //here

					if (score > 0) {
						rect r;
						r.x = (startC + 0.5)*ratio - 0.5;
						r.y = (startR + 0.5)*ratio - 0.5;//here
						r.width = newWinC*ratio;
						r.height = newWinR*ratio;
						bb[*bbNum].pos = r;
						bb[*bbNum].obj = PERSON;
						bb[*bbNum].score = score;
						(*bbNum)++;
						trueWinCount++;
					}
				}
				startC += newWinStepC;
			}
			startR += newWinStepR;
		}
	}
	Vps_printf("Pedes totalWinCount: %d\n", totalWinCount);
	Vps_printf("Pedes computedWinCount: %d\n", computedWinCount);
	Vps_printf("Pedes trueWinCount: %d\n", trueWinCount);
}

//TODO
void detectMultiClasses(AlgLink_OsdObj *restrict pObj,
		UInt8 *restrict pSrc, Uint8 *restrict pU, Uint8 *restrict pV, int imgR, int imgC,
		Uint8 *restrict disp, DetectOpt detectOpt[], BOOL isPostPro)
{

	processDisparity(disp, imgR, imgC);

	int startTime = Utils_getCurTimeInMsec();
	constructFeatureIntHist(pObj,pSrc, pU, pV, imgR, imgC);
	int endTime = Utils_getCurTimeInMsec();
	Vps_printf("++++++[constructFeatureIntHist] elapsed time: %d ++++++", endTime-startTime);

	Bbox bb[FOUND_MAX_NUM]; int bbNum = 0;
//	startTime = Utils_getCurTimeInMsec();
//	float scores[50] = { .0f };
//	int scoreNum = 0;
//	detectRoad(pSrc, imgR, imgC, detectOpt[0], bb, &bbNum, scores, &scoreNum);
//	endTime = Utils_getCurTimeInMsec();
//	Vps_printf("++++++[for road] elapsed time: %d ++++++", endTime-startTime);

    startTime = Utils_getCurTimeInMsec();
	detectOpt[1].scale = 1.0;
	detectPedes(pSrc, imgR, imgC, detectOpt[1], bb, &bbNum);

	int nfactor = 4096*1.331;
	int imgC_rescale = imgC*4096/nfactor;
	int imgR_rescale = imgR*4096/nfactor;
	Uint8 *pSrc_rescale = memalign(4,imgC_rescale*imgR_rescale);
	UTILS_assert(pSrc_rescale != NULL);
	VLIB_image_rescale(pSrc,pSrc_rescale,nfactor,imgC,imgR,3);
	constructFeatureIntHist(pObj,pSrc_rescale, pU, pV, imgR_rescale, imgC_rescale);
	detectOpt[1].scale = 1.331;
	detectPedes(pSrc_rescale, imgR_rescale, imgC_rescale, detectOpt[1], bb, &bbNum);
	free(pSrc_rescale);

	endTime = Utils_getCurTimeInMsec();
	Vps_printf("++++++[for pedes] elapsed time: %d ++++++", endTime-startTime);

	if (bbNum > 0){
		UTILS_assert(bbNum < FOUND_MAX_NUM);
		circleMultiClass(pSrc, imgR, imgC, bb, bbNum, isPostPro);
	}
}
