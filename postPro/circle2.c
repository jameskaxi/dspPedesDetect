/*
 * circlePerson2.c
 *
 *  Created on: May 9, 2016
 *      Author: root
 */

#include"./myCLib.h"
#include "postPro.h"

static inline BOOL isMerge(rect r1, rect r2);
static void mergeRect2(rect *r, const float *score, int rectNum, rect *pRet);

#define END -1

void circleMultiClass2(UInt8 *restrict pImg, int imgR, int imgC, Bbox *restrict bb, int bbNum, BOOL isPostPro){
	if (isPostPro){
		int i = 0, j = 0;
		rect *pr = (rect*)malloc(bbNum*sizeof(rect));
		float *ps = (float*)malloc(bbNum*sizeof(float));
		int prCount = 0;
		ObjectType obj[4] = { ROAD, PERSON, CAR, BM };
		for (j = 0; j < 4/* NUM_CLASS*/; j++){
			prCount = 0;
			for (i = 0; i < bbNum; ++i){
				if (bb[i].obj == obj[j]){
					pr[prCount] = bb[i].pos;
					ps[prCount++] = bb[i].score;
				}
			}
			if (obj[j] != ROAD){
				rect foundFiltered[20];
				mergeRect2(pr, ps, prCount, foundFiltered);
				for (i = 0; foundFiltered[i].x != END; i++){
					UTILS_assert(i < 20);
					drawRect(pImg, imgR, imgC, foundFiltered[i]);
					drawClassLabel(pImg, imgR, imgC, foundFiltered[i], obj[j]);
				}
			}
			else{//ROAD no postprocessing
				for (i = 0; i < prCount; ++i){
					drawRect(pImg, imgR, imgC, pr[i]);
				}
			}
		}
		free(pr);
	}
	else{
		int i = 0;
		for (i = 0; i < bbNum; ++i){
			drawRect(pImg, imgR, imgC, bb[i].pos);
			drawClassLabel(pImg, imgR, imgC, bb[i].pos, bb[i].obj);
		}
	}
}

void circlePerson2(UInt8 *restrict pImg, int imgR, int imgC,
				   rect *restrict found, float *restrict pScore, int rectNum,
				  BOOL isPostPro){

	rect foundFiltered[20]; //
	int i = 0;

	if (isPostPro){
		mergeRect2(found, pScore, rectNum, foundFiltered);
		for (i = 0; foundFiltered[i].x != END; i++){
			UTILS_assert(i < 20);
			drawRect(pImg, imgR, imgC, foundFiltered[i]);
		}
	}
	else{
		for (i = 0; i < rectNum; ++i){
			drawRect(pImg, imgR, imgC, found[i]);
		}
	}

}

static void mergeRect2(rect *r, const float *score, int rectNum, rect *pRet){
	int i = 0, j = 0;
	int BE_MERGED = -1;
	int *pNum = (int*)calloc(rectNum, sizeof(int));
	for (i = 0; i < rectNum; ++i){
		if (r[i].x == BE_MERGED) continue;//maxg
		for (j = i + 1; j < rectNum; ++j){
			if (r[j].x == BE_MERGED) continue;
			if (isMerge(r[i], r[j])){
				int pos = score[i]>=score[j] ? j : i;
				r[pos].x = BE_MERGED;
				pos = score[i]>=score[j] ? i : j;
				pNum[pos]++;
			}
		}
	}
	int k = 0;
	for (i = 0; i < rectNum; ++i){
		if (r[i].x != BE_MERGED)/*unmerged and at least merge one window*/
			pRet[k++] = r[i];
	}
	pRet[k].x = END;
	free(pNum);
}

static inline BOOL isMerge(rect r1, rect r2){

	int w = 0, h = 0;
	int area1 = r1.width*r1.height;
	int area2 = r2.width*r2.height;

	if (r1.x > r2.x) w = r2.x + r2.width - r1.x;
	else w = r1.x + r1.width - r2.x;
	if (w <= 0) return FALSE;

	if (r1.y > r2.y) h = r2.y + r2.height - r1.y;
	else h = r1.y + r1.height - r2.y;
	if (h <= 0) return FALSE;

	return w*h >= (area1 + area2 - w*h)*0.5;
}

