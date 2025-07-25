#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>
#include <geo/geodesicGrid.h>
#include <conversion/rotation.h>
#include "param/parameters.hpp"

#define COMPASS_CAL_NUM_SPHERE_PARAMS       4
#define COMPASS_CAL_NUM_ELLIPSOID_PARAMS    9
#define COMPASS_CAL_NUM_SAMPLES             300

/**
    改动-> 
    vector3f 和 matrix3f 的本地转换
    is_zero() 和 is_equal() 的本地转换
    .x .y .z 转换为(0), (1), (2)
    vector3f_rotate()与vector3f_rotate_inverse引入
    .....
*/
namespace calibration
{

class CompassCalibrator {
public:
    CompassCalibrator();
	
    void update_params();
    
    void commit_params();

    // 开始/停止校准
    void start(bool retry, float delay, uint16_t offset_max, uint8_t compass_idx, float tolerance);
    void stop();

    // 更行输入mag采样值
    void new_sample(const matrix::Vector3f& sample);

    // 设置参数->磁力计相对车头方向, 是否是外部mag, 
    void set_orientation(enum Rotation orientation, bool is_external, bool fix_orientation, bool always_45_deg);

    // running is true if actively calculating offsets, diagonals or offdiagonals
    bool running();

    // failed is true if either of the failure states are hit
    bool failed();


    // update the state machine and calculate offsets, diagonals and offdiagonals
    void update();

    // 磁力计校准状态: Mavlink msg MAG_CAL_PROGRESS -> cal_status
    enum class Status {
        NOT_STARTED = 0,
        WAITING_TO_START = 1,
        RUNNING_STEP_ONE = 2,
        RUNNING_STEP_TWO = 3,
        SUCCESS = 4,
        FAILED = 5,
        BAD_ORIENTATION = 6,
        BAD_RADIUS = 7,
    };

    // get completion mask for mavlink reporting (a bitmask of faces/directions for which we have compass samples)
    // 获取全球磁强网格位表
    typedef uint8_t completion_mask_t[10];

    // Structure accessed for cal status update via mavlink 
    // 状态数据: Mavlink msg MAG_CAL_PROGRESS -> cal_status/attempt/completion_pct/completion_mask
    struct State {
        Status status;
        uint8_t attempt;
        float completion_pct;
        completion_mask_t completion_mask;
    } cal_state;

    // Structure accessed after calibration is finished/failed
    // 校准完成报告数据
    struct Report {
        Status status;
        float fitness;
        matrix::Vector3f ofs;
        matrix::Vector3f diag;
        matrix::Vector3f offdiag;
        float orientation_confidence;
        Rotation original_orientation;
        Rotation orientation;
        float scale_factor;
        bool check_orientation;
    } cal_report;

    // Structure setup to set calibration run settings
    // 磁力计校准配置数据
    struct Settings {
        float tolerance;
        bool check_orientation;
        enum Rotation orientation;
        enum Rotation orig_orientation;
        bool is_external;
        bool fix_orientation;
        uint16_t offset_max;
        uint8_t attempt;
        bool retry;
        float delay_start_sec;
        uint32_t start_time_ms;
        uint8_t compass_idx;
        bool always_45_deg;
    } cal_settings;

    // Get calibration result
    // 获取校准结果
    const Report get_report();
    
    // Get current Calibration state
    // 获取校准状态
    const State get_state();

    matrix::Vector3f correct(const matrix::Vector3f &mag_raw);
    
protected:
    // convert index to rotation, this allows to skip some rotations
    // protected so CompassCalibrator_index_test can see it
    Rotation auto_rotation_index(uint8_t n) const;

    // return true if this is a right angle rotation
    bool right_angle_rotation(Rotation r) const;

public:

    // results
    class param_t {
    public:
        float* get_sphere_params() {
            return &radius;
        }

        float* get_ellipsoid_params() {
            return &offset(0);
        }

        float radius;       // magnetic field strength calculated from samples
        matrix::Vector3f offset;    // offsets
        matrix::Vector3f diag;      // diagonal scaling
        matrix::Vector3f offdiag;   // off diagonal scaling
        float scale_factor; // scaling factor to compensate for radius error
    };

    // compact class for approximate attitude, to save memory
    class AttitudeSample {
    public:
        matrix::Matrix3f get_rotmat() const;
        void set_from_ahrs();
    private:
        int8_t roll;
        int8_t pitch;
        int8_t yaw;
    };

    // compact class to hold compass samples, to save memory
    class CompassSample {
    public:
        matrix::Vector3f get() const;
        void set(const matrix::Vector3f &in);
        AttitudeSample att;
    private:
        int16_t x;
        int16_t y;
        int16_t z;
    };

    // set status including any required initialisation
    bool set_status(Status status);

    // consume point raw sample from intermediate structure
    void pull_sample();

    // returns true if sample should be added to buffer
    bool accept_sample(const matrix::Vector3f &sample, uint16_t skip_index = UINT16_MAX);
    bool accept_sample(const CompassSample &sample, uint16_t skip_index = UINT16_MAX);

    // returns true if fit is acceptable
    bool fit_acceptable() const;

    // clear sample buffer and reset offsets and scaling to their defaults
    void reset_state();

    // initialize fitness before starting a fit
    void initialize_fit();

    // true if enough samples have been collected and fitting has begun (aka runniong())
    bool _fitting() const;

    // thins out samples between step one and step two
    void thin_samples();

    // calc the fitness of a single sample vs a set of parameters (offsets, diagonals, off diagonals)
    float calc_residual(const matrix::Vector3f& sample, const param_t& params) const;

    // calc the fitness of the parameters (offsets, diagonals, off diagonals) vs all the samples collected
    // returns 1.0e30f if the sample buffer is empty
    float calc_mean_squared_residuals(const param_t& params) const;

    // calculate initial offsets by simply taking the average values of the samples
    void calc_initial_offset();

    // run sphere fit to calculate diagonals and offdiagonals
    void calc_sphere_jacob(const matrix::Vector3f& sample, const param_t& params, float* ret) const;
    void run_sphere_fit();

    // run ellipsoid fit to calculate diagonals and offdiagonals
    void calc_ellipsoid_jacob(const matrix::Vector3f& sample, const param_t& params, float* ret) const;
    void run_ellipsoid_fit();

    // update the completion mask based on a single sample
    void update_completion_mask(const matrix::Vector3f& sample);

    // reset and updated the completion mask using all samples in the sample buffer
    void update_completion_mask();

    // calculate compass orientation
    matrix::Vector3f calculate_earth_field(CompassSample &sample, enum Rotation r);
    bool calculate_orientation();

    // fix radius to compensate for sensor scaling errors
    bool fix_radius();

    // update methods to read write intermediate structures, called inside thread
    inline void update_cal_status();
    inline void update_cal_report();
    inline void update_cal_settings();

    // running method for use in thread
    bool _running() const;

    uint8_t _compass_idx;                   // index of the compass providing data
    Status _status;                         // current state of calibrator

    // values provided by caller
    float _delay_start_sec;                 // seconds to delay start of calibration (provided by caller)
    bool _retry;                            // true if calibration should be restarted on failured (provided by caller)
    float _tolerance = 5.0;                 // worst acceptable RMS tolerance (aka fitness).  see set_tolerance()
    uint16_t _offset_max;                   // maximum acceptable offsets (provided by caller)

    // behavioral state
    uint32_t _start_time_ms;                // system time start() function was last called
    uint8_t _attempt;                       // number of attempts have been made to calibrate
    completion_mask_t _completion_mask;     // bitmask of directions in which we have samples
    CompassSample *_sample_buffer;          // buffer of sensor values
    uint16_t _samples_collected;            // number of samples in buffer
    uint16_t _samples_thinned;              // number of samples removed by the thin_samples() call (called before step 2 begins)

    // fit state
    class param_t _params;                  // latest calibration outputs
    uint16_t _fit_step;                     // step during RUNNING_STEP_ONE/TWO which performs sphere fit and ellipsoid fit
    float _fitness;                         // fitness (mean squared residuals) of current parameters
    float _initial_fitness;                 // fitness before latest "fit" was attempted (used to determine if fit was an improvement)
    float _sphere_lambda;                   // sphere fit's lambda
    float _ellipsoid_lambda;                // ellipsoid fit's lambda

    // variables for orientation checking
    enum Rotation _orientation;             // latest detected orientation
    enum Rotation _orig_orientation;        // original orientation provided by caller
    enum Rotation _orientation_solution;    // latest solution
    bool _is_external;                      // true if compass is external (provided by caller)
    bool _check_orientation;                // true if orientation should be automatically checked
    bool _fix_orientation;                  // true if orientation should be fixed if necessary
    bool _always_45_deg;                    // true if orientation should consider 45deg with equal tolerance
    float _orientation_confidence;          // measure of confidence in automatic orientation detection
    CompassSample _last_sample;

    Status _requested_status;
    bool   _status_set_requested;

    bool _new_sample;
    
    ParamFloat<UParams::COMPASS_DIA_X> _param_diag_x;
    ParamFloat<UParams::COMPASS_DIA_Y> _param_diag_y;
    ParamFloat<UParams::COMPASS_DIA_Z> _param_diag_z;
    ParamFloat<UParams::COMPASS_ODI_X> _param_offdiag_x;
    ParamFloat<UParams::COMPASS_ODI_Y> _param_offdiag_y;
    ParamFloat<UParams::COMPASS_ODI_Z> _param_offdiag_z;
    ParamFloat<UParams::COMPASS_OFS_X> _param_offset_x;
    ParamFloat<UParams::COMPASS_OFS_Y> _param_offset_y;
    ParamFloat<UParams::COMPASS_OFS_Z> _param_offset_z;  
};

}
