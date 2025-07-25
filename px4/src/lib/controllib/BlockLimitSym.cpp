#include <math.h>
#include <float.h>


#include "blocks.hpp"

namespace control
{

float BlockLimitSym::update(float input)
{
	if (input > getParamMax()) {
		input = _max.get();

	} else if (input < -getParamMax()) {
		input = -getParamMax();
	}

	return input;
}

} // namespace control
