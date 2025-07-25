#include <math.h>
#include <float.h>

#include "blocks.hpp"

namespace control
{

float BlockLowPass2::update(float input)
{
	if (!PX4_ISFINITE(getState())) {
		setState(input);
	}

	if (fabsf(_lp.get_cutoff_freq() - getFCutParam()) > FLT_EPSILON) {
		_lp.set_cutoff_frequency(_fs, getFCutParam());
	}

	_state = _lp.apply(input);
	return _state;
}

} // namespace control
