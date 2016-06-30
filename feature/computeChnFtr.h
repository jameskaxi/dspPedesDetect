#ifndef COMPUTE_CHNS_FTR_H_
#define COMPUTE_CHNS_FTR_H_

int getFtrDim(int rows, int cols);
void computeChnFtr(const UInt8 *pSrc, int srcR, int srcC, rect r, float *pChnFtr, int ftrDim);

#endif
