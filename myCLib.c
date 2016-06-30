#include "myCLib.h"
#include "numDisp.h"

void drawRect(UInt8 *img, int imgR, int imgC, rect r){
	int j, index;
	int colorVal = 255;
	int thickness = 1;

	for (j = r.y; j < r.y + r.height; j++){
		index = j*imgC + r.x;
		img[index] = colorVal;
		img[index + r.width - thickness] = colorVal;

	}
	//set two rows to coloVal
	memset(&img[r.y*imgC + r.x], colorVal, r.width);
	memset(&img[(r.y + r.height - thickness)*imgC + r.x], colorVal, r.width);
}

//unsigned char shuzi_disp[8 * 16 * 12]
void drawClassLabel(UInt8 *img, int imgR, int imgC, rect r, int label){
	const int R = 16;
	const int C = 8;
	int i = 0, j = 0;
	for (j = 0; j < R; j++){
		for (i = 0; i < C; i++){
			img[(j + r.y)*imgC + r.x + i] = shuzi_disp[i + j*C + label*R*C];
		}
	}
}


void drawNum(UInt8 *disparity, UInt8 *img, int imgR, int imgC, rect r){
	int startX=100,startY=100;
	startX=r.y;startY=r.x;
	int arry[3];
	float  num=0,sum=0 ;
	int i,j,index;
	int dig_num=0;//the num of the value of distance
	int count=0;

	for(i =r.y ;i<r.y + r.height;i++ )
	{
		for(j=r.x;j<r.x+r.width;j++)
		{
			index = i*imgC+j;
			if(disparity[index]>=16)
			{
				count++;
				sum += disparity[index];
			}
		}
	}
	int fir=0;
	int sec=0;
//	num = 160*count/sum;//z = bf/d; disparity_value = d *16; z = bf*16/disparity_value;disparity_value = sum /count;
//	num = 520*count/sum; //b=7cm.f = 470.
    num = 360*count/sum; //b=7cm.f = 320.
	fir = floor(num);
	sec = floor(num*10 - fir*10);
//	int fir =5;
//	int sec =9;
	arry[0] = fir;
	arry[1] = 10;//the point
	arry[2] = sec;
	for ( dig_num = 0; dig_num < 3; dig_num++){
		for (i = 2; i < 16; i++){
			for (j = 0; j < 8; j++){
				img[(startX + i) * 320 + startY + dig_num * 8 + j] = shuzi_disp[arry[dig_num] * 8 * 16 + i * 8 + j];
			}
		}
	}

}
