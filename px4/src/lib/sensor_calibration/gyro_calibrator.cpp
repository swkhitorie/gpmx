#include "gyro_calibrator.h"
using namespace calibration;

GyroscopeCalibrator::GyroscopeCalibrator()
{
    clear();
    update_params();
}

uint8_t GyroscopeCalibrator::state()
{
    return _state;
}

void GyroscopeCalibrator::clear()
{
    _sample_all.zero();
    _sample_collected = 0;
    _state = GYRO_CAL_IDLE;
}

void GyroscopeCalibrator::update_params()
{
    if ((fabsf(offset(0) - _param_offs_x.get()) > 0.00001f) && PX4_ISFINITE(_param_offs_x.get())) offset(0) = _param_offs_x.get();
    if ((fabsf(offset(1) - _param_offs_y.get()) > 0.00001f) && PX4_ISFINITE(_param_offs_y.get())) offset(1) = _param_offs_y.get();
    if ((fabsf(offset(2) - _param_offs_z.get()) > 0.00001f) && PX4_ISFINITE(_param_offs_z.get())) offset(2) = _param_offs_z.get();
}

void GyroscopeCalibrator::commit_params()
{
    _param_offs_x.set(offset(0));
    _param_offs_y.set(offset(1));
    _param_offs_z.set(offset(2));
    
    _param_offs_x.commit();
    _param_offs_y.commit();
    _param_offs_z.commit();
}

void GyroscopeCalibrator::start()
{
    _state = GYRO_CAL_SAMPLEING_COLLECTING;
}

void GyroscopeCalibrator::run(const matrix::Vector3f &gyro_raw)
{
    switch (_state) {
    case GYRO_CAL_IDLE: break;
    case GYRO_CAL_WAITING_START:
    case GYRO_CAL_SAMPLEING_COLLECTING:
        if (_sample_collected < GYRO_CAL_SAMPLE_NUM) {
            _sample_all += gyro_raw;
            _sample_collected++;            
        } else if (_sample_collected == GYRO_CAL_SAMPLE_NUM) {
            offset = _sample_all / _sample_collected;
            _state = GYRO_CAL_SUCCESS;
        }
        break;
    case GYRO_CAL_SUCCESS: break;
    case GYRO_CAL_FAILED: break;
    }
}

matrix::Vector3f GyroscopeCalibrator::correct(const matrix::Vector3f &gyro_raw)
{
    matrix::Vector3f gyro_correct;
    gyro_correct = gyro_raw - offset;
    
    return gyro_correct;
}


