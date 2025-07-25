#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A proportional-derivative controller.
 * @link http://en.wikipedia.org/wiki/PID_controller
 */
class __EXPORT BlockPD
{
public:
// methods
	BlockPD() :
		_derivative()
	{}
	~BlockPD() = default;
	float update(float input)
	{
		return getParamKP() * input +
		       getParamKD() * getDerivative().update(input);
	}
// accessors
	float getParamKP() { return _param_kP; }
	float getParamKD() { return _param_kD; }
	
	void setParamKP(float kP) { _param_kP = kP; }
	void setParamKD(float kD) { _param_kD = kD; }
	
	BlockDerivative &getDerivative() { return _derivative; }
private:
	BlockDerivative _derivative;
	float _param_kP;
	float _param_kD;
};

} // namespace control
