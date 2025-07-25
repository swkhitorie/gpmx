#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockIntegralTrap::update(float input)
{
	// trapezoidal integration
	setY(_limit.update(getY() +
			   (getU() + input) / 2.0f * getDt()));
	setU(input);
	return getY();
}

} // namespace control
