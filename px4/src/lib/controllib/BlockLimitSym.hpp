#pragma once


#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A symmetric limiter/ saturation.
 * Same as limiter but with only a max, is used for
 * upper limit of +max, and lower limit of -max
 */
class __EXPORT BlockLimitSym
{
public:
// methods
	BlockLimitSym()
	{}
	virtual ~BlockLimitSym() = default;
	float update(float input);
// accessors
	float getParamMax() { return _param_max; }
	void setParamMax(float Max) { _param_max = Max; }
protected:
// attributes
	float _param_max;
};

} // namespace control
