#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A proportional-integral-derivative controller.
 * @link http://en.wikipedia.org/wiki/PID_controller
 */
class __EXPORT BlockPID
{
public:
// methods
	BlockPID() :
		_integral(),
		_derivative()
	{}
	~BlockPID() = default;
	float update(float input)
	{
		return getParamKP() * input +
		       getParamKI() * getIntegral().update(input) +
		       getParamKD() * getDerivative().update(input);
	}
// accessors
	float getParamKP() { return _param_kP; }
	float getParamKI() { return _param_kI; }
	float getParamKD() { return _param_kD; }
	
	void setParamKP(float kP) { _param_kP = kP; }
	void setParamKI(float kI) { _param_kI = kI; }
	void setParamKD(float kD) { _param_kD = kD; }
	
	BlockIntegral &getIntegral() { return _integral; }
	BlockDerivative &getDerivative() { return _derivative; }
private:
// attributes
	BlockIntegral _integral;
	BlockDerivative _derivative;
	float _param_kP;
	float _param_kI;
	float _param_kD;
};

} // namespace control
