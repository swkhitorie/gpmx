#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A proportional-integral controller.
 * @link http://en.wikipedia.org/wiki/PID_controller
 */
class __EXPORT BlockPI
{
public:
// methods
	BlockPI() :
		_integral()
	{}
	~BlockPI() = default;
	float update(float input)
	{
		return getParamKP() * input +
		       getParamKI() * getIntegral().update(input);
	}
// accessors
	float getParamKP() { return _param_kP; }
	float getParamKI() { return _param_kI; }
	
	void setParamKP(float kP) { _param_kP = kP; }
	void setParamKI(float kI) { _param_kI = kI; }
	
	BlockIntegral &getIntegral() { return _integral; }
private:
	BlockIntegral _integral;
	float _param_kP;
	float _param_kI;
};

} // namespace control
