#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockLowPass::update(float input)
{
	if (!PX4_ISFINITE(getState())) {
		setState(input);
	}

	float b = 2 * M_PI_F * getParamFCut() * getDt();
	float a = b / (1 + b);
	setState(a * input + (1 - a)*getState());
	return getState();
}

} // namespace control
