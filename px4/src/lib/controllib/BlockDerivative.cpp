#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockDerivative::update(float input)
{
	float output;

	if (_initialized) {
		output = _lowPass.update((input - getU()) / getDt());

	} else {
		// if this is the first call to update
		// we have no valid derivative
		// and so we use the assumption the
		// input value is not changing much,
		// which is the best we can do here.
		_lowPass.update(0.0f);
		output = 0.0f;
		_initialized = true;
	}

	setU(input);
	return output;
}

} // namespace control
