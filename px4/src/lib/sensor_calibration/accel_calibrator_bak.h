#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>
#include "param/parameters.hpp"
namespace calibration
{
    
class AccelCalibratorBak
{
public:
    enum ACCELCAL_STATE {
        ACCEL_CAL_IDLE = 0,
        ACCEL_CAL_WAITING_FOR_ORIENTATION = 1,
        ACCEL_CAL_SAMPLEING_COLLECTING = 3,
        ACCEL_CAL_SUCCESS = 4,
        ACCEL_CAL_FAILED = 5
    };
    
    /*
        accel x axis正方向为头部, 右手系
        采集顺序: POS_LEVEL -> POS_NOSEUP -> POS_NOSEDOWN -> POS_LEFT -> POS_RIGHT -> POS_BACK
    */
    enum ACCELCAL_ORINTATION {
        ACCEL_POS_NULL = 0xFF,
        ACCEL_POS_LEVEL = 0x00,
        ACCEL_POS_LEFT = 0x03,
        ACCEL_POS_RIGHT = 0x04,
        ACCEL_POS_NOSEDOWN = 0x02,
        ACCEL_POS_NOSEUP = 0x01,
        ACCEL_POS_BACK = 0x05
    };
    
	AccelCalibratorBak();
	~AccelCalibratorBak() = default;
	
    void update_params();
    
    void commit_params();
    
    uint8_t state() { return _state; }
    
    bool enabled();
    
	void clear();
    
    void wait_orientation() { _state = ACCEL_CAL_WAITING_FOR_ORIENTATION; }
    
    void stop() { _state = ACCEL_CAL_IDLE; }
    
    bool next();
    
    bool complete(enum ACCELCAL_ORINTATION now_pos) { return _samples_completed[_orientaion_collected]; }
    
    enum ACCELCAL_ORINTATION now_pos() { return _orientaion_collected; }
    
	void run(matrix::Vector3f &accel_sample, float gyrolen);	
	
    matrix::Vector3f correct(const matrix::Vector3f &accel_raw);
    
    void sensor_corrections_update(){}
    
public:
    
	void reset(float dS[6], float JS[6][6]);

	void find_delta(float dS[6],float JS[6][6], float delta[6]);

	void update(float dS[6], float JS[6][6], float beta[6], float data[6]);

	bool running_fit(matrix::Vector3f (&_sample)[6], matrix::Vector3f &accel_offsets, matrix::Vector3f &accel_scale);

    uint64_t _lst_sample_time;
    uint64_t _sample_period_us;

    matrix::Vector3f _sample[6];
    bool _samples_completed[6];
    enum ACCELCAL_ORINTATION _orientaion_collected;
    uint32_t _samples_collected;

    uint8_t _state;

	matrix::Vector3f offset { 0.0f, 0.0f, 0.0f };
	matrix::Vector3f scale { 1.0f, 1.0f, 1.0f };
    
    ParamFloat<UParams::INS_ACCOFFS_X> _param_offs_x;
    ParamFloat<UParams::INS_ACCOFFS_Y> _param_offs_y;
    ParamFloat<UParams::INS_ACCOFFS_Z> _param_offs_z;
    ParamFloat<UParams::INS_ACCSCAL_X> _param_scal_x;
    ParamFloat<UParams::INS_ACCSCAL_Y> _param_scal_y;
    ParamFloat<UParams::INS_ACCSCAL_Z> _param_scal_z;
};
}

/* 
    AccelCalibratorBak _accel_calibration {};

    Connect to GCS Calibration:
        process 1 (freq running): _accel_calibration.running(accel_sample, dt);
        process 2 (receive interact): 
            if (start_condition) {
                _accel_calibration.wait_orientation();
            }
            if (next_condition) {
                if (cnt_pos <= pos_all_6) {
                    start_cal_accel_sample = true;
                }
            }

        process 3 (freq running):
            if (start_cal_accel_sample) {
                if (_accel_calibration->state() == calibration::AccelCalibratorBak::ACCEL_CAL_WAITING_FOR_ORIENTATION) {
                    _accel_calibration->next();

                } else if (_accel_calibration->complete(_accel_calibration->now_pos())) {
                    start_cal_accel_sample = false;
                    _accel_calibration->wait_orientation();
                    
                    if (cnt_pos == pos_all_6) {
                        send_msg_to_gcs_all_end(...);
                    } else {
                        cnt_pos++;
                        send_msg_to_gcs_begin_next_pos(...);                    
                    }
                }
            }
*/
