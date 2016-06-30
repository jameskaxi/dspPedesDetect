#include "AdaPredictUpper.h"

float adaboostPredictUpper(const float *data){
	int i;
	int pos = 0;
	float result = .0f;
	for (i = 0; i < nWeaks; ++i){
		unsigned int k = 0;
		while (upper_child[k]){
			k = upper_child[k] - (data[upper_fids[pos + k]] < upper_thrs[pos + k]);
		}
		result += upper_hs[pos + k];
		if (result < -1) return  result;
		if (result > 10 - (i >> 7)) return  result;
		pos += weak_dimension;
	}
	return  result;
}
