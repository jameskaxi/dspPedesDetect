#include"./myCLib.h"
#include "postPro.h"

static void mergeRect(const rect *_vec, int rectNum, int groupThreshold, float weight, rect *rectFinalRes);
static inline int similarRect(rect r1, rect r2, float weight);
static void  merge(int x, int y, int *father);
static int getRoot(int x, int *father);

#define END -1

void circleMultiClass(UInt8 *restrict pImg, int imgR, int imgC, Bbox *restrict bb, int bbNum, BOOL isPostPro){
	if (isPostPro){
		int i = 0, j = 0;
		rect *pr = (rect*)malloc(bbNum*sizeof(rect));
		int prCount = 0;
		//const int NUM_CLASS = 4;
		ObjectType obj[4] = { CAR, BM,PERSON, ROAD };
		for (j = 0; j < 4/* NUM_CLASS*/; j++){
			prCount = 0;
			for (i = 0; i < bbNum; ++i){
				if (bb[i].obj == obj[j])
					pr[prCount++] = bb[i].pos;
			}
			if (obj[j] != ROAD){
				rect foundFiltered[20];
				mergeRect(pr, prCount, 1, 0.2, foundFiltered);
				for (i = 0; foundFiltered[i].x != END; i++){
					UTILS_assert(i < 20);
					drawRect(pImg, imgR, imgC, foundFiltered[i]);
					drawClassLabel(pImg, imgR, imgC, foundFiltered[i], obj[j]);
				}
			}
			else{//ROAD postprocessing
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

void circlePerson(Uint8 *imgDisparity,UInt8 *pImg, int imgR, int imgC, rect *found, int rectNum, BOOL isPostPro){

	rect foundFiltered[20]; //
	int i = 0;

	if (isPostPro){
		mergeRect(found, rectNum, 2, 0.2, foundFiltered);
		for (i = 0; foundFiltered[i].x != END; i++){
			UTILS_assert(i < 20);
			drawRect(pImg, imgR, imgC, foundFiltered[i]);
//			drawNum(imgDisparity,pImg, imgR, imgC, foundFiltered[i]);

		}
	}
	else{
		for (i = 0; i < rectNum; ++i){
			drawRect(pImg, imgR, imgC, found[i]);
//			drawNum(imgDisparity,pImg, imgR, imgC, foundFiltered[i]);
		}
	}
}


//#define NO_MALLOC

#ifdef NO_MALLOC
#define RECTNUM 100
static int father[RECTNUM];
static int className[RECTNUM];
static rect vecRst[RECTNUM];
static int rectCount[RECTNUM];
#endif

static void mergeRect(const rect *_vec, int rectNum, int groupThreshold, float weight, rect *rectFinalRes){

	int i, j;
	int N = rectNum;
	const rect* vec = &_vec[0];

#ifndef NO_MALLOC
	int *father = (int*)calloc(N, sizeof(int));
	int *className;
	rect *vecRst;
	int *rectCount;
#endif

	int nclass = 0, k = 0, ri = 0;

	// init
	for (i = 0; i < N; ++i)
		father[i] = i;

	//if similar, merge.
	for (i = 0; i < N; ++i){
		for (j = i + 1; j < N; j++){
			if (similarRect(vec[i], vec[j], weight))
				merge(i, j, father);
		}
	}

#ifndef NO_MALLOC
	className = (int *)calloc(N, sizeof(int));
#endif

	for (i = 0; i < N; i++){
		if (father[i] == i){
			className[nclass++] = i;
		}
	}

#ifndef NO_MALLOC
	vecRst = (rect*)calloc(nclass, sizeof(rect));
	rectCount = (int*)calloc(nclass, sizeof(int));
#endif

	/*��������ͬ�����ֵ�����������ֵ����������groupThreshold����*/
	for (i = 0; i < nclass; i++){
		rectCount[k] = 0;
		memset(&vecRst[k], 0, sizeof(rect));
		for (j = 0; j < N; j++){
			if (getRoot(j, father) == className[i]){
				rectCount[k]++;
				vecRst[k].x += vec[j].x;
				vecRst[k].y += vec[j].y;
				vecRst[k].width += vec[j].width;
				vecRst[k].height += vec[j].height;
			}
		}
		/*����groupThresholdʱ��k++����Ȼԭ�ȵĻᱻ���ǵ�*/
		if (rectCount[k] >= groupThreshold){
			vecRst[k].x /= rectCount[k];
			vecRst[k].y /= rectCount[k];
			vecRst[k].width /= rectCount[k];
			vecRst[k].height /= rectCount[k];
			k++;
		}
	}
	/*��Ƕ�ڴ���ο��ڲ���С���ο���˵�,
	���ʣ�µľ��ο�Ϊ����Ľ��*/
	for (i = 0; i < k; i++){
		for (j = 0; j < k; j++){
			if (i != j && vecRst[i].x >= vecRst[j].x && vecRst[i].y >= vecRst[j].y &&
				vecRst[i].x + vecRst[i].width <= vecRst[j].x + vecRst[j].width  &&
				vecRst[i].y + vecRst[i].height <= vecRst[j].y + vecRst[j].height)
				break;
		}
		if (j == k){
			rectFinalRes[ri++] = vecRst[i];
		}
	}
	rectFinalRes[ri].x = END; /*��x=-1����ΪrectFinalRes�����θ������ֹ��*/

#ifndef NO_MALLOC
	free(father);
	free(className);
	free(vecRst);
	free(rectCount);
#endif
}


static inline int similarRect(rect r1, rect r2, float weight){

	float eps = weight;
	int minWidth = (r1.width > r2.width)*r2.width + (r1.width <= r2.width)*r1.width;
	int minHeight = (r1.height > r2.height)*r2.height + (r1.height <= r2.height)*r1.height;
	float delta = eps*(minWidth + minHeight)*0.5;

	return (abs(r1.x - r2.x) <= delta &&
		abs(r1.y - r2.y) <= delta &&
		abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
		abs(r1.y + r1.height - r2.y - r2.height) <= delta);
}

static int getRoot(int x, int *father)
{
	if (father[x] != x) father[x] = getRoot(father[x], father);
	return father[x];
}

static void  merge(int x, int y, int *father)
{
	int rx = getRoot(x, father);
	int ry = getRoot(y, father);
	father[ry] = x*(rx != ry) + father[ry] * (rx == ry);
}


