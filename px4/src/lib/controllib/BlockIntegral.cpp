#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockIntegral::update(float input)
{
	// trapezoidal integration
	setY(_limit.update(getY() + input * getDt()));
	return getY();
}

} // namespace control
