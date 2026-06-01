#pragma once

#include "gpm/drv_hrt.h"
#include <libs/conversion/rotation.h>
#include <uorb/topics/sensor_mag.h>
#include <PublicationMulti.hpp>

class xMagnetometer
{
public:
	xMagnetometer(uint32_t device_id, enum Rotation rotation = ROTATION_NONE);
	~xMagnetometer();

	void set_device_id(uint32_t device_id) { _device_id = device_id; }
	void set_error_count(uint32_t error_count) { _error_count = error_count; }
	void set_scale(float scale) { _scale = scale; }
	void set_temperature(float temperature) { _temperature = temperature; }

	void update(const hrt_abstime &timestamp_sample, float x, float y, float z);

	int get_instance() { return _sensor_pub.get_instance(); };
	uint32_t get_device_id() const { return _device_id; }

private:
	uORB::PublicationMulti<sensor_mag_s> _sensor_pub{ORB_ID(sensor_mag)};

	uint32_t		_device_id{0};
	const enum Rotation	_rotation;

	float			_scale{1.f};
	float			_temperature{NAN};
	uint32_t		_error_count{0};
};
