#pragma once


#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A limiter/ saturation.
 * The output of update is the input, bounded
 * by min/max.
 */
class __EXPORT BlockLimit
{
public:
// methods
	BlockLimit()
	{}
	virtual ~BlockLimit() = default;
	float update(float input);
// accessors
	float getParamMin() { return _param_min; }
	float getParamMax() { return _param_max; }
	
	void setParamMin(float min) { _param_min = min; }
	void setParamMax(float max) { _param_max = max; }	
	
protected:
// attributes
	float _param_min;
	float _param_max;
};

} // namespace control
