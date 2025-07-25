#pragma once

#include <stdlib.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter2p.hpp>
#include "matrix/matrix/math.hpp"

namespace control
{
template<class Type, size_t M>
class __EXPORT BlockStats
{

public:
// methods
    BlockStats():
		_sum(),
		_sumSq(),
		_count(0)
	{}
	~BlockStats() = default;
	void update(const matrix::Vector<Type, M> &u)
	{
		_sum += u;
		_sumSq += u.emult(u);
		_count += 1;
	}
	void reset()
	{
		_sum.setZero();
		_sumSq.setZero();
		_count = 0;
	}
// accessors
	size_t getCount() { return _count; }
	matrix::Vector<Type, M> getMean() { return _sum / _count; }
	matrix::Vector<Type, M> getVar()
	{
		return (_sumSq - _sum.emult(_sum) / _count) / _count;
	}
	matrix::Vector<Type, M> getStdDev()
	{
		return getVar().sqrt();
	}
private:
// attributes
	matrix::Vector<Type, M> _sum;
	matrix::Vector<Type, M> _sumSq;
	size_t _count;
};

} // namespace control
