#pragma once

#include "BlockLowPass.hpp"

#include <platform_defines.h>
#include <math.h>
#include "matrix/matrix/math.hpp"

namespace control
{

/**
 * A simple derivative approximation.
 * This uses the previous and current input.
 * This has a built in low pass filter.
 * @see LowPass
 */
class __EXPORT BlockDerivative
{
public:
// methods
	BlockDerivative() :
		_u(0),
		_initialized(false),
		_lowPass()
	{}
	~BlockDerivative() = default;

	/**
	 * Update the state and get current derivative
	 *
	 * This call updates the state and gets the current
	 * derivative. As the derivative is only valid
	 * on the second call to update, it will return
	 * no change (0) on the first. To get a closer
	 * estimate of the derivative on the first call,
	 * call setU() one time step before using the
	 * return value of update().
	 *
	 * @param input the variable to calculate the derivative of
	 * @return the current derivative
	 */
	float update(float input);
// accessors
	void setU(float u) {_u = u;}
	void reset() { _initialized = false; };
	float getU() {return _u;}
	float getLP() {return _lowPass.getParamFCut();}
	float getO() { return _lowPass.getState(); }
protected:
// attributes
	float _u; /**< previous input */
	bool _initialized;
	BlockLowPass _lowPass; /**< low pass filter */
};

} // namespace control
