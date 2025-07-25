#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>
#include "param/parameters.hpp"

#define GYRO_CAL_SAMPLE_NUM (10000)

namespace calibration
{

class GyroscopeCalibrator
{
public:
    enum Status {
        GYRO_CAL_IDLE = 0,
        GYRO_CAL_WAITING_START,
        GYRO_CAL_SAMPLEING_COLLECTING,
        GYRO_CAL_SUCCESS,
        GYRO_CAL_FAILED
    };
    
	GyroscopeCalibrator();
	~GyroscopeCalibrator() = default;
	
    void update_params();
    
    void commit_params();
    
    bool enabled() { return true; }

    matrix::Vector3f correct(const matrix::Vector3f &gyro_raw);
    
    void run(const matrix::Vector3f &gyro_raw);
    
    void start();
    
    uint8_t state();
    
    void clear();
    
public:
    
    matrix::Vector3f _sample_all;
    uint32_t _sample_collected;

    uint8_t _state;

    matrix::Vector3f offset {0.0f, 0.0f, 0.0f};
    matrix::Vector3f diag;
    matrix::Vector3f offdiag;

    ParamFloat<UParams::INS_GYROFFS_X> _param_offs_x;
    ParamFloat<UParams::INS_GYROFFS_Y> _param_offs_y;
    ParamFloat<UParams::INS_GYROFFS_Z> _param_offs_z;
};


}

