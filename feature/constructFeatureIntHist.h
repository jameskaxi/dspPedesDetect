#ifndef CONSTRUCT_GRAD_HIST_INT_HIST_H_
#define CONSTRUCT_GRAD_HIST_INT_HIST_H_
#pragma once

#include "algLink_priv.h"
#include <sw_osd_ti_priv.h>
#include <sw_osd_ti_dmaOpt.h>

//void constructFeatureIntHist(AlgLink_OsdObj * pObj, const UInt8 *pSrc, int srcR, int srcC);

void constructFeatureIntHist(AlgLink_OsdObj *restrict pObj,
		const UInt8 *restrict pY, const Uint8 *restrict pU, const Uint8 *restrict pV,
		int srcR, int srcC);
#endif

