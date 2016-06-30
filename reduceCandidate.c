#include"reduceCandidate.h"
#include"myCLib.h"

static int intHist[MAX_IN_IMG_R + 1][MAX_IN_IMG_C + 1] = { 0 };
static void constructIntHist(const UInt8 *pImg, int row, int col, int threshold);

void processDisparity(const UInt8 *pSrc, int imgR, int imgC){

	const int THRESHOLD = 16;	
	const UInt8 *pImg = pSrc;
	constructIntHist(pImg, imgR, imgC, THRESHOLD);
}

BOOL judgeCandidate(int startY , int startX, int winR, int winC){

	const int THRESHOLD = winR*winC / 6;

	int x1 = startX + winC / 3;
	int x2 = startX + winC / 3 *2;
	int y1 = startY;
	int y2 = startY + winR - 1;

	int sum = intHist[y2][x2] + intHist[y1][x1] - intHist[y2][x1] - intHist[y1][x2];
	return sum > THRESHOLD;
}

static void constructIntHist(const UInt8 *pImg,  int row, int col, int threshold){

	int i = 0, j = 0;
	memset(intHist,0,sizeof(intHist));

	//each row accumulation
	int pos = 0;
	for (j = 0 + 1; j < row + 1; j++){
		for (i = 1; i < col + 1; i++){

			intHist[j][i] = (pImg[pos + i - 1]> threshold )*1 + intHist[j][i - 1];
		}
		pos += col;
	}

	//each cols accumulation
	for (i = 0 + 1; i < col + 1; i++){
		for (j = 1 + 1; j < row + 1; j++){

			intHist[j][i] += intHist[j - 1][i];

		}
	}
}
