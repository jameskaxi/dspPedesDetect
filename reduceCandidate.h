#ifndef REDUCE_CANDIDATE_H_
#define REDUCE_CANDIDATE_H_

#include"myCLib.h"

void processDisparity(const UInt8 *pSrc, int imgR, int imgC);
BOOL judgeCandidate(int startY, int startX, int winR, int winC);

#endif
