#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{

template<class Type, size_t M, size_t N, size_t LEN>
class __EXPORT BlockDelay
{
public:
// methods
	BlockDelay() :
		_h(),
		_index(0),
		_delay(-1)
	{}
	~BlockDelay() = default;
	matrix::Matrix<Type, M, N> update(const matrix::Matrix<Type, M, N> &u)
	{
		// store current value
		_h[_index] = u;

		// delay starts at zero, then increases to LEN
		_delay += 1;

		if (_delay > (int)(LEN - 1)) {
			_delay = LEN - 1;
		}

		// compute position of delayed value
		int j = _index - _delay;

		if (j < 0) {
			j += LEN;
		}

		// increment storage position
		_index += 1;

		if (_index > (LEN - 1)) {
			_index  = 0;
		}

		// get delayed value
		return _h[j];
	}
	matrix::Matrix<Type, M, N> get(size_t delay)
	{
		int j = _index - delay;

		if (j < 0) { j += LEN; }

		return _h[j];
	}
private:
// attributes
	matrix::Matrix<Type, M, N> _h[LEN];
	size_t _index;
	int _delay;
};

} // namespace control
