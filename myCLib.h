#ifndef MYLIB_H_
#define MYLIB_H_
#pragma once

#include"setting.h"

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<time.h>
#include "mcfw/src_bios6/utils/utils_trace.h"
#include "algLink_priv.h"
#include <sw_osd_ti_priv.h>
#include <sw_osd_ti_dmaOpt.h>

typedef unsigned char UInt8;
//typedef unsigned int uint;

typedef struct rect{
	int x, y, width, height;
}rect;

#define BOOL unsigned short
#define FALSE 0
#define TRUE 1

void drawRect(UInt8 *img, int imgR, int imgC, rect r);

void drawClassLabel(UInt8 *img, int imgR, int imgC, rect r, int label);

#endif
