#include "AdaPersonMultiScale.h"

float adaPersonMultiScale(const float *data, int dimension, int id)
{
	int i;
	int pos = 0;
	float result = .0f;

	const float *thrsN = &person_thrs[id][0];
	const unsigned int *fidsN = &person_fids[id][0];
	const float *hsN = &person_hs[id][0];

	for (i = 0; i < nWeaks; ++i){
		unsigned int k = 0;
		while (person_child[k]){
			if (data[fidsN[pos + k]] < thrsN[pos + k])
				k = person_child[k] - 1;
			else
				k = person_child[k];
		}
		result += hsN[pos + k];
		if (result < -1) return  result;
		pos += weak_dimension;
	}
	return  result;
}
