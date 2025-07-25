#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockHighPass::update(float input)
{
	float b = 2 * M_PI_F * getParamFCut() * getDt();
	float a = 1 / (1 + b);
	setY(a * (getY() + input - getU()));
	setU(input);
	return getY();
}

} // namespace control
