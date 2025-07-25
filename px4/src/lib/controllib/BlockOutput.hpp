#pragma once


#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{
/**
 * An output trim/ saturation block
 */
class __EXPORT BlockOutput
{
public:
// methods
	BlockOutput() :
		_limit(),
		_val(0)
	{
		update(0);
	}

	virtual ~BlockOutput() = default;
	void update(float input)
	{
		_val = _limit.update(input + getParamTrim());
	}
// accessors
	float getMin() { return _limit.getParamMin(); }
	float getMax() { return _limit.getParamMax(); }
	float getParamTrim() { return _param_trim; }
	void setParamTrim(float trim) { _param_trim = trim; }
	float get() { return _val; }
private:
// attributes
	float _param_trim;
	BlockLimit _limit;
	float _val;
};

} // namespace control
