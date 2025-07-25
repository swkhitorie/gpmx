#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>

namespace calibration
{
    
class CompassCalibratorBak
{
public:
    enum CompassCAL_STATE {
        COMPASS_CAL_IDLE = 0,
        COMPASS_CAL_SAMPLEING_COLLECTING = 2,
        COMPASS_CAL_SUCCESS = 3,
        COMPASS_CAL_FAILED = 4
    };
    
    CompassCalibratorBak() = default;
    ~CompassCalibratorBak() = default;

    static constexpr uint64_t COMPASS_CAL_SAMPLE_US = 50 * 1e6f;
    
    void clear();
    
    void start();
    
    uint8_t state() { return _state; }
    
    matrix::Vector3f direction() { return sample; }
    
    bool enabled();
    
    uint8_t percent() { return _percent; }
    
    void run(matrix::Vector3f &mag_sample);
    
public:
    
    void sample_point(double x, double y, double z);            //根据采样数据生成最小二乘法的矩阵
    bool running_fit();

	void row1_swap_row2(uint8_t row1,uint8_t row2);
	void k_muilt_row(double k,uint8_t row);
    void row2_add_krow1(double k,uint8_t row1,uint8_t row2);
    void row_simplify();
    void move_biggestelement_to_top(uint8_t k);
    void calculate_inputaverage();

    bool gauss_elimination();
    void matrix_solve(double *solve);

    double _paramlist[6];
    double _linear_equation_solve[6];
    double _equation[6][6 + 1];

    matrix::Vector3f sample;

    uint32_t _sample_collected;
    uint8_t _state;

    uint64_t _sample_begin_time;
    uint8_t _percent;
};

}
