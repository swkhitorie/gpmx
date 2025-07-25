#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockLimit::update(float input)
{
	if (input > getParamMax()) {
		input = getParamMax();

	} else if (input < getParamMin()) {
		input = getParamMin();
	}

	return input;
}

} // namespace control
