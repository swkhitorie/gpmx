#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A uniform random number generator
 */
class __EXPORT BlockRandUniform
{
public:
// methods
	BlockRandUniform()
	{
		// seed should be initialized somewhere
		// in main program for all calls to rand
		// XXX currently in nuttx if you seed to 0, rand breaks
	}
	~BlockRandUniform() = default;
	float update()
	{
		static float rand_max = static_cast<float>(RAND_MAX);
		float rand_val = rand();
		float bounds = getParamMax() - getParamMin();
		return getParamMin() + (rand_val * bounds) / rand_max;
	}
// accessors
	float getParamMin() { return _param_min; }
	float getParamMax() { return _param_max; }
	
	void setParamMin(float min) { _param_min = min; }
	void setParamMax(float max) { _param_max = max; }
private:
// attributes
	float _param_min;
	float _param_max;
};

} // namespace control
