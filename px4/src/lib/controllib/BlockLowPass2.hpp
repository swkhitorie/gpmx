#pragma once


#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A 2nd order low pass filter block which uses the default px4 2nd order low pass filter
 */
class __EXPORT BlockLowPass2
{
public:
// methods
	BlockLowPass2(float sample_freq) :
        _param_fCut(10.0f),
		_state(0.0 / 0.0 /* initialize to invalid val, force into is_finite() check on first call */),
		_fs(sample_freq),
		_lp(_fs, _param_fCut)
	{}
	~BlockLowPass2() = default;
	float update(float input);
// accessors
	float getState() { return _state; }
	float getFCutParam() { return _param_fCut; }
	void setFCutParam(float fCut) 
	{ 
		_param_fCut = fCut; 
		_lp.set_cutoff_frequency(_fs, _param_fCut);
	}
	void setState(float state) { _state = _lp.reset(state); }
protected:
// attributes
	float _state;
	float _param_fCut;
	float _fs;
	math::LowPassFilter2p<float> _lp;
};

} // namespace control
