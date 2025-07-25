#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A high pass filter as described here:
 * http://en.wikipedia.org/wiki/High-pass_filter.
 */
class __EXPORT BlockHighPass
{
public:
// methods
	BlockHighPass() :
		_u(0),
		_y(0)
	{}
	~BlockHighPass() = default;
	float update(float input);
// accessors
	float getU() {return _u;}
	float getY() {return _y;}
	float getParamFCut() {return _param_fCut;}
	void setParamFCut(float fCut) { _param_fCut = fCut;}
	void setU(float u) {_u = u;}
	void setY(float y) {_y = y;}
    void setDt(float dt) { _dt = dt; }
	float getDt() { return _dt; }
protected:
// attributes
	float _u; /**< previous input */
	float _y; /**< previous output */
	float _param_fCut; /**< cut-off frequency, Hz */
    float _dt;
};

} // namespace control
