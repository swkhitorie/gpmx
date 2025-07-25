#pragma once

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A low pass filter as described here:
 * http://en.wikipedia.org/wiki/Low-pass_filter.
 */
class __EXPORT BlockLowPass
{
public:
// methods
	BlockLowPass()
	{}
	~BlockLowPass() = default;
	float update(float input);
// accessors
	float getState() { return _state; }
	float getParamFCut() { return _param_fCut; }
	void setParamFCut(float fCut) { _param_fCut = fCut; }
	void setState(float state) { _state = state; }
    void setDt(float dt) { _dt = dt; }
    float getDt() { return _dt; }
protected:
// attributes
	float _state{NAN}; // initialize to invalid val, force into is_finite() check on first call
	float _param_fCut;
    float _dt;
};

} // namespace control
