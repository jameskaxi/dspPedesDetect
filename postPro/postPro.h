#ifndef _CIRCLEPERSON_
#define _CIRCLEPERSON_

#include"./myCLib.h"

typedef enum ObjectType{
	ROAD, PERSON, CAR, BM
}ObjectType;

typedef struct Bbox{
	rect pos;
	float score;
	ObjectType obj;
}Bbox;

void circleMultiClass(UInt8 *restrict pImg, int imgR, int imgC,
		Bbox *restrict bb, int bbNum, BOOL isPostPro);

//max supress
void circleMultiClass2(UInt8 *restrict pImg, int imgR, int imgC,
		Bbox *restrict bb, int bbNum, BOOL isPostPro);


void circlePerson(UInt8 *restrict imgDp, UInt8 *restrict pImg, int imgR, int imgC,
		rect *restrict found, int rectNum, BOOL isPostPro);

void circlePerson2(UInt8 *restrict pImg, int imgR, int imgC,
		rect *restrict found, float *restrict pScore, int rectNum, BOOL isPostPro);

#endif
