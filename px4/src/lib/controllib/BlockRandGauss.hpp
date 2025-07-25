#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

class __EXPORT BlockRandGauss
{
public:
// methods
	BlockRandGauss()
	{
		// seed should be initialized somewhere
		// in main program for all calls to rand
		// XXX currently in nuttx if you seed to 0, rand breaks
	}
	~BlockRandGauss() = default;
	float update()
	{
		static float V1, V2, S;
		static int phase = 0;
		float X;

		if (phase == 0) {
			do {
				float U1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float U2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				V1 = 2 * U1 - 1;
				V2 = 2 * U2 - 1;
				S = V1 * V1 + V2 * V2;
			} while (S >= 1 || fabsf(S) < 1e-8f);

			X = V1 * float(sqrtf(-2 * float(logf(S)) / S));

		} else {
			X = V2 * float(sqrtf(-2 * float(logf(S)) / S));
		}

		phase = 1 - phase;
		return X * getParamStdDev() + getParamMean();
	}
// accessors
	float getParamMean() { return _param_mean; }
	float getParamStdDev() { return _param_stdDev; }
	
	void setParamMean(float mean) { _param_mean = mean; }
	void setParamStdDev(float stdDev) { _param_stdDev = stdDev; }
private:
// attributes
	float _param_mean;
	float _param_stdDev;
};

} // namespace control
