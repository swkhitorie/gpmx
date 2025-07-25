#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

template<class Type, size_t M>
class __EXPORT BlockLowPassVector
{
public:
// methods
	BlockLowPassVector() :
		_state()
	{
		for (size_t i = 0; i < M; i++) {
			_state(i) = 0.0f;
		}
	}
	~BlockLowPassVector() = default;
	matrix::Vector<Type, M> update(const matrix::Matrix<Type, M, 1> &input)
	{
		for (size_t i = 0; i < M; i++) {
			if (!PX4_ISFINITE(getState()(i))) {
				setState(input);
			}
		}

		float b = 2 * M_PI_F * getParamFCut() * getDt();
		float a = b / (1 + b);
		setState(input * a + getState() * (1 - a));
		return getState();
	}
// accessors
	matrix::Vector<Type, M> getState() { return _state; }
	float getParamFCut() { return _param_fCut; }
	void setParamFCut(float fCut) { _param_fCut = fCut; }
	void setState(const matrix::Vector<Type, M> &state) { _state = state; }
    void setDt(float dt) { _dt = dt; }
    float getDt() { return _dt; }
private:
// attributes
	matrix::Vector<Type, M> _state;
	float _param_fCut;
    float _dt;
};

} // namespace control
