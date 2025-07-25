#pragma once


#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A trapezoidal integrator.
 * http://en.wikipedia.org/wiki/Trapezoidal_rule
 * A limiter is built into the class to bound the
 * integral's internal state. This is important
 * for windup protection.
 * @see Limit
 */
class __EXPORT BlockIntegralTrap
{
public:
// methods
	BlockIntegralTrap() :
		_u(0),
		_y(0),
		_limit() {}
	virtual ~BlockIntegralTrap() = default;
	float update(float input);
// accessors
	float getU() {return _u;}
	float getY() {return _y;}
	float getParamMax() {return _limit.getParamMax();}
	void setU(float u) {_u = u;}
	void setY(float y) {_y = y;}
protected:
// attributes
	float _u; /**< previous input */
	float _y; /**< previous output */
	BlockLimitSym _limit; /**< limiter */
};

} // namespace control
