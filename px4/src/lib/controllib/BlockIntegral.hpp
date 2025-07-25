#pragma once

#include "BlockLimitSym.hpp"
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A rectangular integrator.
 * A limiter is built into the class to bound the
 * integral's internal state. This is important
 * for windup protection.
 * @see Limit
 */
class __EXPORT BlockIntegral
{
public:
// methods
	BlockIntegral() :
		_y(0),
		_limit() {}
	~BlockIntegral() = default;
	float update(float input);
// accessors
	float getY() {return _y;}
	float getParamMax() {return _limit.getParamMax();}
	void setY(float y) {_y = y;}
protected:
// attributes
	float _y; /**< previous output */
	BlockLimitSym _limit; /**< limiter */
};

} // namespace control
