#ifndef XATOMIC_CXX_H_
#define XATOMIC_CXX_H_

#include <stdbool.h>
#include <stdint.h>
#include "gpm/sched.h"

template <typename T>
class xatomic
{
public:

	xatomic() = default;
	explicit xatomic(T value) : _value(value) {}

	/** Atomically read the current value */
	inline T load() const
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T val = _value;
			leave_critical_section(flags);
			return val;
		} else {
			return __atomic_load_n(&_value, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomically store a value */
	inline void store(T value)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			_value = value;
			leave_critical_section(flags);
		} else {
			__atomic_store(&_value, &value, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomically add a number and return the previous value. */
	inline T fetch_add(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T ret = _value;
			_value += num;
			leave_critical_section(flags);
			return ret;
		} else {
			return __atomic_fetch_add(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomically substract a number and return the previous value. */
	inline T fetch_sub(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T ret = _value;
			_value -= num;
			leave_critical_section(flags);
			return ret;
		} else {
			return __atomic_fetch_sub(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomic AND with a number */
	inline T fetch_and(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T val = _value;
			_value &= num;
			leave_critical_section(flags);
			return val;
		} else {
			return __atomic_fetch_and(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomic XOR with a number */
	inline T fetch_xor(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T val = _value;
			_value ^= num;
			leave_critical_section(flags);
			return val;
		} else {
			return __atomic_fetch_xor(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomic OR with a number */
	inline T fetch_or(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T val = _value;
			_value |= num;
			leave_critical_section(flags);
			return val;
		} else {
			return __atomic_fetch_or(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/** Atomic NAND (~(_value & num)) with a number */
	inline T fetch_nand(T num)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();
			T ret = _value;
			_value = ~(_value & num);
			leave_critical_section(flags);
			return ret;
		} else {
			return __atomic_fetch_nand(&_value, num, __ATOMIC_SEQ_CST);
		}
	}

	/**
	 * Atomic compare and exchange operation.
	 * This compares the contents of _value with the contents of *expected. If
	 * equal, the operation is a read-modify-write operation that writes desired
	 * into _value. If they are not equal, the operation is a read and the current
	 * contents of _value are written into *expected.
	 * @return If desired is written into _value then true is returned
	 */
	inline bool compare_exchange(T *expected, T desired)
	{
		if (!__atomic_always_lock_free(sizeof(T), 0)) {
			uint32_t flags = enter_critical_section();

			if (_value == *expected) {
				_value = desired;
				leave_critical_section(flags);
				return true;

			} else {
				*expected = _value;
				leave_critical_section(flags);
				return false;
			}

		} else {
			return __atomic_compare_exchange(&_value, expected, &desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
		}
	}

private:
	T _value {};
};

using xatomic_int = xatomic<int>;
using xatomic_int32_t = xatomic<int32_t>;
using xatomic_bool = xatomic<bool>;

#endif
