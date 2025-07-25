#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A proportional controller.
 * @link http://en.wikipedia.org/wiki/PID_controller
 */
class __EXPORT BlockP
{
public:
// methods
	BlockP()
	{}
	~BlockP() = default;
	float update(float input)
	{
		return getParamKP() * input;
	}
// accessors
	float getParamKP() { return _param_kP; }
	void setParamKP(float kP) { _param_kP = kP; }	
	
protected:
	float _param_kP;
};

} // namespace control
