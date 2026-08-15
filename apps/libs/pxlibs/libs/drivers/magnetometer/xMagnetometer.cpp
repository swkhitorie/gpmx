#include "xMagnetometer.hpp"

xMagnetometer::xMagnetometer(uint32_t device_id, enum Rotation rotation) :
	_device_id{device_id},
	_rotation{rotation}
{
}

xMagnetometer::~xMagnetometer()
{
	_sensor_pub.unadvertise();
}

void xMagnetometer::update(const hrt_abstime &timestamp_sample, float x, float y, float z)
{
	sensor_mag_s report;
	report.timestamp_sample = timestamp_sample;
	report.device_id = _device_id;
	report.temperature = _temperature;
	report.error_count = _error_count;

	// Apply rotation (before scaling)
	rotate_3f(_rotation, x, y, z);

	report.x = x * _scale;
	report.y = y * _scale;
	report.z = z * _scale;

	report.timestamp = hrt_absolute_time();
	_sensor_pub.publish(report);
}
