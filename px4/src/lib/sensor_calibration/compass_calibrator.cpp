#include "compass_calibrator.h"
#include "mavlink_vehicle.h"

using namespace calibration;
/*
  simple 16 bit random number generator
 */
static uint16_t get_random16(void)
{
    static uint32_t m_z = 1234;
    static uint32_t m_w = 76542;
    m_z = 36969 * (m_z & 0xFFFFu) + (m_z >> 16);
    m_w = 18000 * (m_w & 0xFFFFu) + (m_w >> 16);
    return ((m_z << 16) + m_w) & 0xFFFF;
}

static void vector3f_rotate(matrix::Vector3f &vtr, enum Rotation rotation)
{
    float _x = vtr(0);
    float _y = vtr(1);
    float _z = vtr(2);
    rotate_3f(rotation, _x, _y, _z);
    vtr.setRow(0, _x); 
    vtr.setRow(1, _y); 
    vtr.setRow(2, _z);  
}

static void vector3f_rotate_inverse(matrix::Vector3f &vtr, enum Rotation rotation)
{
    matrix::Vector3f x_vec(1.0f,0.0f,0.0f);
    matrix::Vector3f y_vec(0.0f,1.0f,0.0f);
    matrix::Vector3f z_vec(0.0f,0.0f,1.0f);

    vector3f_rotate(x_vec, rotation);
    vector3f_rotate(y_vec, rotation);
    vector3f_rotate(z_vec, rotation);

    const float init_matrix_data[3*3] = {
        x_vec(0), y_vec(0), z_vec(0),
        x_vec(1), y_vec(1), z_vec(1),
        x_vec(2), y_vec(2), z_vec(2)
    };
    
    // M.mul_transpose(vtr), M -> after transpose -> M(T) multi vtr
    matrix::Matrix3f M(init_matrix_data);

    float vtr_x, vtr_y, vtr_z;
    
    vtr_x = M(0, 0) * vtr(0) + M(1, 0) * vtr(1) + M(2, 0) * vtr(2);
    vtr_y = M(0, 1) * vtr(0) + M(1, 1) * vtr(1) + M(2, 1) * vtr(2);
    vtr_z = M(0, 2) * vtr(0) + M(1, 2) * vtr(1) + M(2, 2) * vtr(2);
    
    vtr.setRow(0, vtr_x);
    vtr.setRow(1, vtr_y);
    vtr.setRow(2, vtr_z);
}


#define FIELD_RADIUS_MIN 150
#define FIELD_RADIUS_MAX 950
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
////////////////////////////////////////////////////////////
///////////////////// PUBLIC INTERFACE /////////////////////
////////////////////////////////////////////////////////////

CompassCalibrator::CompassCalibrator()
{
    set_status(Status::NOT_STARTED);
    
    update_params();
        //in room 1
//    _params.diag(0) = 0.984f;
//    _params.diag(1) = 1.020f;
//    _params.diag(2) = 0.992f;
//    
//    _params.offdiag(0) = 0.012f;
//    _params.offdiag(1) = 0.010f;
//    _params.offdiag(2) = 0.0001f;
//    
//    _params.offset(0) = 82.124f;
//    _params.offset(1) = 156.318f;
//    _params.offset(2) = -4.468f;  

//        //in room 2
//    _params.diag(0) = 1.000f;
//    _params.diag(1) = 1.000f;
//    _params.diag(2) = 1.000f;
//    
//    _params.offdiag(0) = 0.000f;
//    _params.offdiag(1) = 0.000f;
//    _params.offdiag(2) = 0.000f;
//    
//    _params.offset(0) = 88.897f;
//    _params.offset(1) = 146.986f;
//    _params.offset(2) = -32.076f;  
    // shi che chang
//    _params.diag(0) = 0.989f;
//    _params.diag(1) = 0.998f;
//    _params.diag(2) = 1.009f;
//    
//    _params.offdiag(0) = 0.028f;
//    _params.offdiag(1) = 0.010f;
//    _params.offdiag(2) = -0.003f;
//    _params.offset(0) = 92.303f;
//    _params.offset(1) = 155.430f;
//    _params.offset(2) = -67.841f;  
//    _params.offset.zero();
//    _params.offdiag.zero();
//    _params.diag.setOne();
//    _params.offset.zero();
//    _params.offdiag.zero();
//    _params.diag.setOne();
}

void CompassCalibrator::update_params()
{
    if ((fabsf(_params.diag(0) - _param_diag_x.get()) > 0.002f) && PX4_ISFINITE(_param_diag_x.get())) _params.diag(0) = _param_diag_x.get();
    if ((fabsf(_params.diag(1) - _param_diag_y.get()) > 0.002f) && PX4_ISFINITE(_param_diag_y.get())) _params.diag(1) = _param_diag_y.get();
    if ((fabsf(_params.diag(2) - _param_diag_z.get()) > 0.002f) && PX4_ISFINITE(_param_diag_z.get())) _params.diag(2) = _param_diag_z.get();
    if ((fabsf(_params.offdiag(0) - _param_offdiag_x.get()) > 0.002f) && PX4_ISFINITE(_param_offdiag_x.get())) _params.offdiag(0) = _param_offdiag_x.get();
    if ((fabsf(_params.offdiag(1) - _param_offdiag_y.get()) > 0.002f) && PX4_ISFINITE(_param_offdiag_y.get())) _params.offdiag(1) = _param_offdiag_y.get();
    if ((fabsf(_params.offdiag(2) - _param_offdiag_z.get()) > 0.002f) && PX4_ISFINITE(_param_offdiag_z.get())) _params.offdiag(2) = _param_offdiag_z.get();
    if ((fabsf(_params.offset(0) - _param_offset_x.get()) > 0.002f) && PX4_ISFINITE(_param_offset_x.get())) _params.offset(0) = _param_offset_x.get();
    if ((fabsf(_params.offset(1) - _param_offset_y.get()) > 0.002f) && PX4_ISFINITE(_param_offset_y.get())) _params.offset(1) = _param_offset_y.get();
    if ((fabsf(_params.offset(2) - _param_offset_z.get()) > 0.002f) && PX4_ISFINITE(_param_offset_z.get())) _params.offset(2) = _param_offset_z.get();
}

void CompassCalibrator::commit_params()
{
    _param_diag_x.set(_params.diag(0));
    _param_diag_y.set(_params.diag(1));
    _param_diag_z.set(_params.diag(2));
    _param_offdiag_x.set(_params.offdiag(0));
    _param_offdiag_y.set(_params.offdiag(1));
    _param_offdiag_z.set(_params.offdiag(2));
    _param_offset_x.set(_params.offset(0));
    _param_offset_y.set(_params.offset(1));
    _param_offset_z.set(_params.offset(2));
    
    _param_diag_x.commit();
    _param_diag_y.commit();
    _param_diag_z.commit();
    _param_offdiag_x.commit();
    _param_offdiag_y.commit();
    _param_offdiag_z.commit();
    _param_offset_x.commit();
    _param_offset_y.commit();
    _param_offset_z.commit();
}

// Request to cancel calibration 
void CompassCalibrator::stop()
{
    _requested_status = Status::NOT_STARTED;
    _status_set_requested = true;
}

void CompassCalibrator::set_orientation(enum Rotation orientation, bool is_external, bool fix_orientation, bool always_45_deg)
{
    cal_settings.check_orientation = false; // true
    cal_settings.orientation = orientation;
    cal_settings.orig_orientation = orientation;
    cal_settings.is_external = is_external;
    cal_settings.fix_orientation = fix_orientation;
    cal_settings.always_45_deg = always_45_deg;
}

void CompassCalibrator::start(bool retry, float delay, uint16_t offset_max, uint8_t compass_idx, float tolerance)
{
    if (compass_idx > 3) {
        return;
    }

    // Don't do this while we are already started
    if (_running()) {
        return;
    }
    cal_settings.offset_max = offset_max;
    cal_settings.attempt = 1;
    cal_settings.retry = retry;
    cal_settings.delay_start_sec = delay;
    cal_settings.start_time_ms = hrt_absolute_time() / 1000;
    cal_settings.compass_idx = compass_idx;
    cal_settings.tolerance = tolerance;

    // Request status change to Waiting to start
    _requested_status = Status::WAITING_TO_START;
    _status_set_requested = true;
}

// Record point mag sample and associated attitude sample to intermediate struct
void CompassCalibrator::new_sample(const matrix::Vector3f& sample)
{
    _last_sample.set(sample);
    // _last_sample.att.set_from_ahrs();
    _new_sample = true;
}

bool CompassCalibrator::failed() {
    return (cal_state.status == Status::FAILED ||
            cal_state.status == Status::BAD_ORIENTATION || 
            cal_state.status == Status::BAD_RADIUS);
}


bool CompassCalibrator::running() {
    return (cal_state.status == Status::RUNNING_STEP_ONE || cal_state.status == Status::RUNNING_STEP_TWO);
}

const CompassCalibrator::Report CompassCalibrator::get_report() {
    return cal_report;
}

const CompassCalibrator::State CompassCalibrator::get_state() {
    return cal_state;
}

matrix::Vector3f CompassCalibrator::correct(const matrix::Vector3f &mag_raw)
{
    const float init_matrix_data[3*3] = {
        _params.diag(0),    _params.offdiag(0), _params.offdiag(1),
        _params.offdiag(0), _params.diag(1),    _params.offdiag(2),
        _params.offdiag(1), _params.offdiag(2), _params.diag(2)
    };
    
    matrix::Matrix3f softiron {init_matrix_data};
    matrix::Vector3f corrected = softiron * (mag_raw + _params.offset); 
    return corrected;
}

/////////////////////////////////////////////////////////////
////////////////////// PRIVATE METHODS //////////////////////
/////////////////////////////////////////////////////////////

void CompassCalibrator::update()
{

    //pickup samples from intermediate struct
    pull_sample();

    {
        //update_settings
        if (!running()) {
            update_cal_settings();
        }

        //update requested state
        if (_status_set_requested) {
            _status_set_requested = false;
            set_status(_requested_status);
        }
        //update report and status
        update_cal_status();
        update_cal_report();
    }

    // collect the minimum number of samples
    if (!_fitting()) {
        return;
    }

    if (_status == Status::RUNNING_STEP_ONE) {
        if (_fit_step >= 10) {
            if (matrix::isEqualF(_fitness, _initial_fitness) || isnan(_fitness)) {  // if true, means that fitness is diverging instead of converging
                set_status(Status::FAILED);
            } else {
                set_status(Status::RUNNING_STEP_TWO);
            }
        } else {
            if (_fit_step == 0) {
                calc_initial_offset();
            }
            run_sphere_fit();
            _fit_step++;
        }
    } else if (_status == Status::RUNNING_STEP_TWO) {
        if (_fit_step >= 35) {
            if (fit_acceptable() && fix_radius() && calculate_orientation()) {
                set_status(Status::SUCCESS);
            } else { 
                set_status(Status::FAILED);
            }
        } else if (_fit_step < 15) {
            run_sphere_fit();  
            _fit_step++;
        } else {
            run_ellipsoid_fit();
            _fit_step++;
        }
    }
}

void CompassCalibrator::pull_sample()
{
    CompassSample mag_sample;
    {
        if (!_new_sample) {
            return;
        }
        if (_status == Status::WAITING_TO_START) {
            set_status(Status::RUNNING_STEP_ONE);
        }
        _new_sample = false;
        mag_sample = _last_sample;
    }
    if (_running() && _samples_collected < COMPASS_CAL_NUM_SAMPLES && accept_sample(mag_sample.get())) {
        update_completion_mask(mag_sample.get());
        _sample_buffer[_samples_collected] = mag_sample;
        _samples_collected++;
    }
}


void CompassCalibrator::update_cal_settings()
{
    _tolerance = cal_settings.tolerance;
    _check_orientation = cal_settings.check_orientation;
    _orientation = cal_settings.orientation;
    _orig_orientation = cal_settings.orig_orientation;
    _is_external = cal_settings.is_external;
    _fix_orientation = cal_settings.fix_orientation;
    _offset_max = cal_settings.offset_max;
    _attempt = cal_settings.attempt;
    _retry = cal_settings.retry;
    _delay_start_sec = cal_settings.delay_start_sec;
    _start_time_ms = cal_settings.start_time_ms;
    _compass_idx = cal_settings.compass_idx;
    _always_45_deg = cal_settings.always_45_deg;
}

// update completion mask based on latest sample
// used to ensure we have collected samples in all directions
void CompassCalibrator::update_completion_mask(const matrix::Vector3f& v)
{
    const float init_matrix_data[3*3] = {
        _params.diag(0),    _params.offdiag(0), _params.offdiag(1),
        _params.offdiag(0), _params.diag(1),    _params.offdiag(2),
        _params.offdiag(1), _params.offdiag(2), _params.diag(2)
    };
    
    matrix::Matrix3f softiron {init_matrix_data};
    matrix::Vector3f corrected = softiron * (v + _params.offset);
    int section = GeodesicGrid::section(corrected, true);
    if (section < 0) {
        return;
    }
    _completion_mask[section / 8] |= 1 << (section % 8);
}

// reset and update the completion mask using all samples in the sample buffer
void CompassCalibrator::update_completion_mask()
{
    memset(_completion_mask, 0, sizeof(_completion_mask));
    for (int i = 0; i < _samples_collected; i++) {
        update_completion_mask(_sample_buffer[i].get());
    }
}

void CompassCalibrator::update_cal_status()
{
    cal_state.status = _status;
    cal_state.attempt = _attempt;
    memcpy(cal_state.completion_mask, _completion_mask, sizeof(completion_mask_t));
    cal_state.completion_pct = 0.0f;
    // first sampling step is 1/3rd of the progress bar
    // never return more than 99% unless _status is Status::SUCCESS
    switch (_status) {
        case Status::NOT_STARTED:
        case Status::WAITING_TO_START:
            cal_state.completion_pct = 0.0f;
            break;
        case Status::RUNNING_STEP_ONE:
            cal_state.completion_pct = 33.3f * _samples_collected/COMPASS_CAL_NUM_SAMPLES;
            break;
        case Status::RUNNING_STEP_TWO:
            cal_state.completion_pct = 33.3f + 65.7f*((float)(_samples_collected-_samples_thinned)/(COMPASS_CAL_NUM_SAMPLES-_samples_thinned));
            break;
        case Status::SUCCESS:
            cal_state.completion_pct = 100.0f;
            break;
        case Status::FAILED:
        case Status::BAD_ORIENTATION:
        case Status::BAD_RADIUS:
            cal_state.completion_pct = 0.0f;
            break;
    };
}

void CompassCalibrator::update_cal_report()
{
    cal_report.status = _status;
    cal_report.fitness = sqrtf(_fitness);
    cal_report.ofs = _params.offset;
    cal_report.diag = _params.diag;
    cal_report.offdiag = _params.offdiag;
    cal_report.scale_factor = _params.scale_factor;
    cal_report.orientation_confidence = _orientation_confidence;
    cal_report.original_orientation = _orig_orientation;
    cal_report.orientation = _orientation_solution;
    cal_report.check_orientation = _check_orientation;
}

// running method for use in thread
bool CompassCalibrator::_running() const
{
    return _status == Status::RUNNING_STEP_ONE || _status == Status::RUNNING_STEP_TWO;
}

// fitting method for use in thread
bool CompassCalibrator::_fitting() const
{
    return _running() && (_samples_collected == COMPASS_CAL_NUM_SAMPLES);
}

// initialize fitness before starting a fit
void CompassCalibrator::initialize_fit()
{
    if (_samples_collected != 0) {
        _fitness = calc_mean_squared_residuals(_params);
    } else {
        _fitness = 1.0e30f;
    }
    _initial_fitness = _fitness;
    _sphere_lambda = 1.0f;
    _ellipsoid_lambda = 1.0f;
    _fit_step = 0;
}

void CompassCalibrator::reset_state()
{
    _samples_collected = 0;
    _samples_thinned = 0;
    _params.radius = 200;
    _params.offset.zero();
    _params.diag = matrix::Vector3f(1.0f,1.0f,1.0f);
    _params.offdiag.zero();
    _params.scale_factor = 0;

    memset(_completion_mask, 0, sizeof(_completion_mask));
    initialize_fit();
}

bool CompassCalibrator::set_status(CompassCalibrator::Status status)
{
    if (status != Status::NOT_STARTED && _status == status) {
        return true;
    }

    switch (status) {
        case Status::NOT_STARTED:
            reset_state();
            _status = Status::NOT_STARTED;
            if (_sample_buffer != nullptr) {
                free(_sample_buffer);
                _sample_buffer = nullptr;
            }
            return true;

        case Status::WAITING_TO_START:
            reset_state();
            _status = Status::WAITING_TO_START;
            set_status(Status::RUNNING_STEP_ONE);
            return true;

        case Status::RUNNING_STEP_ONE:
            if (_status != Status::WAITING_TO_START) {
                return false;
            }

            // on first attempt delay start if requested by caller
            if (_attempt == 1 && ((hrt_absolute_time() / 1000) - _start_time_ms)*1.0e-3f < _delay_start_sec) {
                return false;
            }

            if (_sample_buffer == nullptr) {
                _sample_buffer = (CompassSample*)calloc(COMPASS_CAL_NUM_SAMPLES, sizeof(CompassSample));
            }
            if (_sample_buffer != nullptr) {
                initialize_fit();
                _status = Status::RUNNING_STEP_ONE;
                return true;
            }
            return false;

        case Status::RUNNING_STEP_TWO:
            if (_status != Status::RUNNING_STEP_ONE) {
                return false;
            }
            thin_samples();
            initialize_fit();
            _status = Status::RUNNING_STEP_TWO;
            return true;

        case Status::SUCCESS:
            if (_status != Status::RUNNING_STEP_TWO) {
                return false;
            }

            if (_sample_buffer != nullptr) {
                free(_sample_buffer);
                _sample_buffer = nullptr;
            }

            _status = Status::SUCCESS;
            return true;

        case Status::FAILED:
            if (_status == Status::BAD_ORIENTATION ||
                _status == Status::BAD_RADIUS) {
                // don't overwrite bad orientation status
                return false;
            }
            // ===== FALLTHROUGH;

        case Status::BAD_ORIENTATION:
        case Status::BAD_RADIUS:
            if (_status == Status::NOT_STARTED) {
                return false;
            }

            if (_retry && set_status(Status::WAITING_TO_START)) {
                _attempt++;
                return true;
            }

            if (_sample_buffer != nullptr) {
                free(_sample_buffer);
                _sample_buffer = nullptr;
            }

            _status = status;
            return true;

        default:
            return false;
    };
}

bool CompassCalibrator::fit_acceptable() const
{
    if (!isnan(_fitness) &&
        _params.radius > FIELD_RADIUS_MIN && _params.radius < FIELD_RADIUS_MAX &&
        fabsf(_params.offset(0)) < _offset_max &&
        fabsf(_params.offset(1)) < _offset_max &&
        fabsf(_params.offset(2)) < _offset_max &&
        _params.diag(0) > 0.2f && _params.diag(0) < 5.0f &&
        _params.diag(1) > 0.2f && _params.diag(1) < 5.0f &&
        _params.diag(2) > 0.2f && _params.diag(2) < 5.0f &&
        fabsf(_params.offdiag(0)) < 1.0f &&      //absolute of sine/cosine output cannot be greater than 1
        fabsf(_params.offdiag(1)) < 1.0f &&
        fabsf(_params.offdiag(2)) < 1.0f ) {
            return _fitness <= math::sq(_tolerance);
        }
    return false;
}

void CompassCalibrator::thin_samples()
{
    if (_sample_buffer == nullptr) {
        return;
    }

    _samples_thinned = 0;
    // shuffle the samples http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    // this is so that adjacent samples don't get sequentially eliminated
    for (uint16_t i=_samples_collected-1; i>=1; i--) {
        uint16_t j = get_random16() % (i+1);
        CompassSample temp = _sample_buffer[i];
        _sample_buffer[i] = _sample_buffer[j];
        _sample_buffer[j] = temp;
    }

    // remove any samples that are close together
    for (uint16_t i=0; i < _samples_collected; i++) {
        if (!accept_sample(_sample_buffer[i], i)) {
            _sample_buffer[i] = _sample_buffer[_samples_collected-1];
            _samples_collected--;
            _samples_thinned++;
        }
    }

    update_completion_mask();
}

/*
 * The sample acceptance distance is determined as follows:
 * For any regular polyhedron with triangular faces, the angle theta subtended
 * by two closest points is defined as
 *
 *      theta = arccos(cos(A)/(1-cos(A)))
 *
 * Where:
 *      A = (4pi/F + pi)/3
 * and
 *      F = 2V - 4 is the number of faces for the polyhedron in consideration,
 *      which depends on the number of vertices V
 *
 * The above equation was proved after solving for spherical triangular excess
 * and related equations.
 */
bool CompassCalibrator::accept_sample(const matrix::Vector3f& sample, uint16_t skip_index)
{
    static const uint16_t faces = (2 * COMPASS_CAL_NUM_SAMPLES - 4);
    static const float a = (4.0f * M_PI_F / (3.0f * faces)) + M_PI_F / 3.0f;
    static const float theta = 0.5f * acosf(cosf(a) / (1.0f - cosf(a)));

    if (_sample_buffer == nullptr) {
        return false;
    }

    float min_distance = _params.radius * 2*sinf(theta/2);

    for (uint16_t i = 0; i<_samples_collected; i++) {
        if (i != skip_index) {
            float distance = (sample - _sample_buffer[i].get()).length();
            if (distance < min_distance) {
                return false;
            }
        }
    }
    return true;
}

bool CompassCalibrator::accept_sample(const CompassSample& sample, uint16_t skip_index)
{
    return accept_sample(sample.get(), skip_index);
}

float CompassCalibrator::calc_residual(const matrix::Vector3f& sample, const param_t& params) const
{
    const float init_matrix_data[3*3] = {
        params.diag(0),    params.offdiag(0), params.offdiag(1),
        params.offdiag(0), params.diag(1),    params.offdiag(2),
        params.offdiag(1), params.offdiag(2), params.diag(2)
    };
    matrix::Matrix3f softiron(init_matrix_data);
    const matrix::Vector3f tmp = (softiron * (sample + params.offset));
    
    return params.radius - tmp.length();
}

// calc the fitness given a set of parameters (offsets, diagonals, off diagonals)
float CompassCalibrator::calc_mean_squared_residuals(const param_t& params) const
{
    if (_sample_buffer == nullptr || _samples_collected == 0) {
        return 1.0e30f;
    }
    float sum = 0.0f;
    for (uint16_t i=0; i < _samples_collected; i++) {
        matrix::Vector3f sample = _sample_buffer[i].get();
        float resid = calc_residual(sample, params);
        sum += math::sq(resid);
    }
    sum /= _samples_collected;
    return sum;
}

// calculate initial offsets by simply taking the average values of the samples
void CompassCalibrator::calc_initial_offset()
{
    // Set initial offset to the average value of the samples
    _params.offset.zero();
    for (uint16_t k = 0; k < _samples_collected; k++) {
        _params.offset -= _sample_buffer[k].get();
    }
    _params.offset /= _samples_collected;
}

void CompassCalibrator::calc_sphere_jacob(const matrix::Vector3f& sample, const param_t& params, float* ret) const
{
    const matrix::Vector3f &offset = params.offset;
    const matrix::Vector3f &diag = params.diag;
    const matrix::Vector3f &offdiag = params.offdiag;
    
    const float init_matrix_data[3*3] = {
        diag(0)    , offdiag(0) , offdiag(1),
        offdiag(0) , diag(1)    , offdiag(2),
        offdiag(1) , offdiag(2) , diag(2)
    };
    
    matrix::Matrix3f softiron(init_matrix_data);

    float A =  (diag(0)    * (sample(0) + offset(0))) + (offdiag(0) * (sample(1) + offset(1))) + (offdiag(1) * (sample(2) + offset(2)));
    float B =  (offdiag(0) * (sample(0) + offset(0))) + (diag(1)    * (sample(1) + offset(1))) + (offdiag(2) * (sample(2) + offset(2)));
    float C =  (offdiag(1) * (sample(0) + offset(0))) + (offdiag(2) * (sample(1) + offset(1))) + (diag(2)    * (sample(2) + offset(2)));
    const matrix::Vector3f tmp = (softiron * (sample + params.offset));
    float length = tmp.length();

    // 0: partial derivative (radius wrt fitness fn) fn operated on sample
    ret[0] = 1.0f;
    // 1-3: partial derivative (offsets wrt fitness fn) fn operated on sample
    ret[1] = -1.0f * (((diag(0)    * A) + (offdiag(0) * B) + (offdiag(1) * C))/length);
    ret[2] = -1.0f * (((offdiag(0) * A) + (diag(1)    * B) + (offdiag(2) * C))/length);
    ret[3] = -1.0f * (((offdiag(1) * A) + (offdiag(2) * B) + (diag(2)    * C))/length);
}

// run sphere fit to calculate diagonals and offdiagonals
void CompassCalibrator::run_sphere_fit()
{
    if (_sample_buffer == nullptr) {
        return;
    }

    const float lma_damping = 10.0f;

    // take backup of fitness and parameters so we can determine later if this fit has improved the calibration
    float fitness = _fitness;
    float fit1, fit2;
    param_t fit1_params, fit2_params;
    fit1_params = fit2_params = _params;

    float JTJ[COMPASS_CAL_NUM_SPHERE_PARAMS*COMPASS_CAL_NUM_SPHERE_PARAMS] = { };
    float JTJ2[COMPASS_CAL_NUM_SPHERE_PARAMS*COMPASS_CAL_NUM_SPHERE_PARAMS] = { };
    float JTFI[COMPASS_CAL_NUM_SPHERE_PARAMS] = { };

    // Gauss Newton Part common for all kind of extensions including LM
    for (uint16_t k = 0; k<_samples_collected; k++) {
        matrix::Vector3f sample = _sample_buffer[k].get();

        float sphere_jacob[COMPASS_CAL_NUM_SPHERE_PARAMS];

        calc_sphere_jacob(sample, fit1_params, sphere_jacob);

        for (uint8_t i = 0;i < COMPASS_CAL_NUM_SPHERE_PARAMS; i++) {
            // compute JTJ
            for (uint8_t j = 0; j < COMPASS_CAL_NUM_SPHERE_PARAMS; j++) {
                JTJ[i*COMPASS_CAL_NUM_SPHERE_PARAMS+j] += sphere_jacob[i] * sphere_jacob[j];
                JTJ2[i*COMPASS_CAL_NUM_SPHERE_PARAMS+j] += sphere_jacob[i] * sphere_jacob[j];   //a backup JTJ for LM
            }
            // compute JTFI
            JTFI[i] += sphere_jacob[i] * calc_residual(sample, fit1_params);
        }
    }

    //------------------------Levenberg-Marquardt-part-starts-here---------------------------------//
    // refer: http://en.wikipedia.org/wiki/Levenberg%E2%80%93Marquardt_algorithm#Choice_of_damping_parameter
    for (uint8_t i = 0; i < COMPASS_CAL_NUM_SPHERE_PARAMS; i++) {
        JTJ[i*COMPASS_CAL_NUM_SPHERE_PARAMS+i] += _sphere_lambda;
        JTJ2[i*COMPASS_CAL_NUM_SPHERE_PARAMS+i] += _sphere_lambda/lma_damping;
    }
    matrix::Matrix<float, COMPASS_CAL_NUM_SPHERE_PARAMS, COMPASS_CAL_NUM_SPHERE_PARAMS> JTJ_m{JTJ};
    matrix::Matrix<float, COMPASS_CAL_NUM_SPHERE_PARAMS, COMPASS_CAL_NUM_SPHERE_PARAMS> JTJ2_m{JTJ2}; 
    matrix::Matrix<float, COMPASS_CAL_NUM_SPHERE_PARAMS, COMPASS_CAL_NUM_SPHERE_PARAMS> JTJ_Inv;
    matrix::Matrix<float, COMPASS_CAL_NUM_SPHERE_PARAMS, COMPASS_CAL_NUM_SPHERE_PARAMS> JTJ2_Inv;
    
    if (!matrix::geninv(JTJ_m, JTJ_Inv)) {
        return;
    }

    if (!matrix::geninv(JTJ2_m, JTJ2_Inv)) {
        return;
    }

    // extract radius, offset, diagonals and offdiagonal parameters
    for (uint8_t row=0; row < COMPASS_CAL_NUM_SPHERE_PARAMS; row++) {
        for (uint8_t col=0; col < COMPASS_CAL_NUM_SPHERE_PARAMS; col++) {
            fit1_params.get_sphere_params()[row] -= JTFI[col] * JTJ_Inv(row, col);
            fit2_params.get_sphere_params()[row] -= JTFI[col] * JTJ2_Inv(row, col);
        }
    }

    // calculate fitness of two possible sets of parameters
    fit1 = calc_mean_squared_residuals(fit1_params);
    fit2 = calc_mean_squared_residuals(fit2_params);

    // decide which of the two sets of parameters is best and store in fit1_params
    if (fit1 > _fitness && fit2 > _fitness) {
        // if neither set of parameters provided better results, increase lambda
        _sphere_lambda *= lma_damping;
    } else if (fit2 < _fitness && fit2 < fit1) {
        // if fit2 was better we will use it. decrease lambda
        _sphere_lambda /= lma_damping;
        fit1_params = fit2_params;
        fitness = fit2;
    } else if (fit1 < _fitness) {
        fitness = fit1;
    }
    //--------------------Levenberg-Marquardt-part-ends-here--------------------------------//

    // store new parameters and update fitness
    if (!isnan(fitness) && fitness < _fitness) {
        _fitness = fitness;
        _params = fit1_params;
        update_completion_mask();
    }
}

void CompassCalibrator::calc_ellipsoid_jacob(const matrix::Vector3f& sample, const param_t& params, float* ret) const
{
    const matrix::Vector3f &offset = params.offset;
    const matrix::Vector3f &diag = params.diag;
    const matrix::Vector3f &offdiag = params.offdiag;
    
    const float init_matrix_data[3*3] = {
        diag(0)    , offdiag(0) , offdiag(1),
        offdiag(0) , diag(1)    , offdiag(2),
        offdiag(1) , offdiag(2) , diag(2)
    };
    
    matrix::Matrix3f softiron(init_matrix_data);

    float A =  (diag(0)    * (sample(0) + offset(0))) + (offdiag(0) * (sample(1) + offset(1))) + (offdiag(1) * (sample(2) + offset(2)));
    float B =  (offdiag(0) * (sample(0) + offset(0))) + (diag(1)    * (sample(1) + offset(1))) + (offdiag(2) * (sample(2) + offset(2)));
    float C =  (offdiag(1) * (sample(0) + offset(0))) + (offdiag(2) * (sample(1) + offset(1))) + (diag(2)    * (sample(2) + offset(2)));
    const matrix::Vector3f tmp = (softiron * (sample + offset));
    float length = tmp.length();

    // 0-2: partial derivative (offset wrt fitness fn) fn operated on sample
    ret[0] = -1.0f * (((diag(0)    * A) + (offdiag(0) * B) + (offdiag(1) * C))/length);
    ret[1] = -1.0f * (((offdiag(0) * A) + (diag(1)    * B) + (offdiag(2) * C))/length);
    ret[2] = -1.0f * (((offdiag(1) * A) + (offdiag(2) * B) + (diag(2)    * C))/length);
    // 3-5: partial derivative (diag offset wrt fitness fn) fn operated on sample
    ret[3] = -1.0f * ((sample(0) + offset(0)) * A)/length;
    ret[4] = -1.0f * ((sample(1) + offset(1)) * B)/length;
    ret[5] = -1.0f * ((sample(2) + offset(2)) * C)/length;
    // 6-8: partial derivative (off-diag offset wrt fitness fn) fn operated on sample
    ret[6] = -1.0f * (((sample(1) + offset(1)) * A) + ((sample(0) + offset(0)) * B))/length;
    ret[7] = -1.0f * (((sample(2) + offset(2)) * A) + ((sample(0) + offset(0)) * C))/length;
    ret[8] = -1.0f * (((sample(2) + offset(2)) * B) + ((sample(1) + offset(1)) * C))/length;
}

void CompassCalibrator::run_ellipsoid_fit()
{
    if (_sample_buffer == nullptr) {
        return;
    }

    const float lma_damping = 10.0f;

    // take backup of fitness and parameters so we can determine later if this fit has improved the calibration
    float fitness = _fitness;
    float fit1, fit2;
    param_t fit1_params, fit2_params;
    fit1_params = fit2_params = _params;

    float JTJ[COMPASS_CAL_NUM_ELLIPSOID_PARAMS*COMPASS_CAL_NUM_ELLIPSOID_PARAMS] = { };
    float JTJ2[COMPASS_CAL_NUM_ELLIPSOID_PARAMS*COMPASS_CAL_NUM_ELLIPSOID_PARAMS] = { };
    float JTFI[COMPASS_CAL_NUM_ELLIPSOID_PARAMS] = { };

    // Gauss Newton Part common for all kind of extensions including LM
    for (uint16_t k = 0; k<_samples_collected; k++) {
        matrix::Vector3f sample = _sample_buffer[k].get();

        float ellipsoid_jacob[COMPASS_CAL_NUM_ELLIPSOID_PARAMS];

        calc_ellipsoid_jacob(sample, fit1_params, ellipsoid_jacob);

        for (uint8_t i = 0;i < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; i++) {
            // compute JTJ
            for (uint8_t j = 0; j < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; j++) {
                JTJ [i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+j] += ellipsoid_jacob[i] * ellipsoid_jacob[j];
                JTJ2[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+j] += ellipsoid_jacob[i] * ellipsoid_jacob[j];
            }
            // compute JTFI
            JTFI[i] += ellipsoid_jacob[i] * calc_residual(sample, fit1_params);
        }
    }

    //------------------------Levenberg-Marquardt-part-starts-here---------------------------------//
    //refer: http://en.wikipedia.org/wiki/Levenberg%E2%80%93Marquardt_algorithm#Choice_of_damping_parameter
    for (uint8_t i = 0; i < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; i++) {
        JTJ[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+i] += _ellipsoid_lambda;
        JTJ2[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+i] += _ellipsoid_lambda/lma_damping;
    }
    matrix::Matrix<float, COMPASS_CAL_NUM_ELLIPSOID_PARAMS, COMPASS_CAL_NUM_ELLIPSOID_PARAMS> JTJ_m{JTJ};
    matrix::Matrix<float, COMPASS_CAL_NUM_ELLIPSOID_PARAMS, COMPASS_CAL_NUM_ELLIPSOID_PARAMS> JTJ2_m{JTJ2}; 
    matrix::Matrix<float, COMPASS_CAL_NUM_ELLIPSOID_PARAMS, COMPASS_CAL_NUM_ELLIPSOID_PARAMS> JTJ_Inv;
    matrix::Matrix<float, COMPASS_CAL_NUM_ELLIPSOID_PARAMS, COMPASS_CAL_NUM_ELLIPSOID_PARAMS> JTJ2_Inv;
    
    if (!matrix::geninv(JTJ_m, JTJ_Inv)) {
        return;
    }

    if (!matrix::geninv(JTJ2_m, JTJ2_Inv)) {
        return;
    }

    // extract radius, offset, diagonals and offdiagonal parameters
    for (uint8_t row=0; row < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; row++) {
        for (uint8_t col=0; col < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; col++) {
            fit1_params.get_ellipsoid_params()[row] -= JTFI[col] * JTJ_Inv(row, col);
            fit2_params.get_ellipsoid_params()[row] -= JTFI[col] * JTJ2_Inv(row, col);
        }
    }

    // calculate fitness of two possible sets of parameters
    fit1 = calc_mean_squared_residuals(fit1_params);
    fit2 = calc_mean_squared_residuals(fit2_params);

    // decide which of the two sets of parameters is best and store in fit1_params
    if (fit1 > _fitness && fit2 > _fitness) {
        // if neither set of parameters provided better results, increase lambda
        _ellipsoid_lambda *= lma_damping;
    } else if (fit2 < _fitness && fit2 < fit1) {
        // if fit2 was better we will use it. decrease lambda
        _ellipsoid_lambda /= lma_damping;
        fit1_params = fit2_params;
        fitness = fit2;
    } else if (fit1 < _fitness) {
        fitness = fit1;
    }
    //--------------------Levenberg-part-ends-here--------------------------------//

    // store new parameters and update fitness
    if (fitness < _fitness) {
        _fitness = fitness;
        _params = fit1_params;
        update_completion_mask();
    }
}


//////////////////////////////////////////////////////////
//////////// CompassSample public interface //////////////
//////////////////////////////////////////////////////////

#define COMPASS_CAL_SAMPLE_SCALE_TO_FIXED(__X) ((int16_t)math::constrainFloatToInt16(roundf(__X*8.0f)))
#define COMPASS_CAL_SAMPLE_SCALE_TO_FLOAT(__X) (__X/8.0f)

matrix::Vector3f CompassCalibrator::CompassSample::get() const
{
    return matrix::Vector3f(COMPASS_CAL_SAMPLE_SCALE_TO_FLOAT(x),
                    COMPASS_CAL_SAMPLE_SCALE_TO_FLOAT(y),
                    COMPASS_CAL_SAMPLE_SCALE_TO_FLOAT(z));
}

void CompassCalibrator::CompassSample::set(const matrix::Vector3f &in)
{
    x = COMPASS_CAL_SAMPLE_SCALE_TO_FIXED(in(0));
    y = COMPASS_CAL_SAMPLE_SCALE_TO_FIXED(in(1));
    z = COMPASS_CAL_SAMPLE_SCALE_TO_FIXED(in(2));
}

void CompassCalibrator::AttitudeSample::set_from_ahrs(void)
{
//    const matrix::Matrix3f &dcm = AP::ahrs().get_DCM_rotation_body_to_ned();
//    float roll_rad, pitch_rad, yaw_rad;
//    dcm.to_euler(&roll_rad, &pitch_rad, &yaw_rad);
//    roll = constrain_int16(127 * (roll_rad / M_PI), -127, 127);
//    pitch = constrain_int16(127 * (pitch_rad / M_PI_2), -127, 127);
//    yaw = constrain_int16(127 * (yaw_rad / M_PI), -127, 127);
}

matrix::Matrix3f CompassCalibrator::AttitudeSample::get_rotmat(void) const
{
    float roll_rad, pitch_rad, yaw_rad;
    roll_rad = math::radians(roll);
    pitch_rad = math::radians(pitch);
    yaw_rad = math::radians(yaw);
    matrix::Dcmf dcm(matrix::Eulerf(roll, pitch, yaw));
    return dcm;
}

/*
  calculate the implied earth field for a compass sample and compass
  rotation. This is used to check for consistency between
  samples. 

  If the orientation is correct then when rotated the same (or
  similar) earth field should be given for all samples. 

  Note that this earth field uses an arbitrary north reference, so it
  may not match the true earth field.
 */
matrix::Vector3f CompassCalibrator::calculate_earth_field(CompassSample &sample, enum Rotation r)
{
    matrix::Vector3f v = sample.get();

    // convert the sample back to sensor frame
    // v.rotate_inverse(_orientation);
    vector3f_rotate_inverse(v, _orientation);
    
    // rotate to body frame for this rotation
    // v.rotate(r);
    vector3f_rotate(v, r);
    
    // apply offsets, rotating them for the orientation we are testing
    matrix::Vector3f rot_offsets = _params.offset;
    // rot_offsets.rotate_inverse(_orientation);
    vector3f_rotate_inverse(rot_offsets, _orientation);
    
    // rot_offsets.rotate(r);
    vector3f_rotate(rot_offsets, r);
    
    v += rot_offsets;

    // rotate the sample from body frame back to earth frame
    matrix::Matrix3f rot = sample.att.get_rotmat();

    matrix::Vector3f efield = rot * v;

    // earth field is the mag sample in earth frame
    return efield;
}

/*
  calculate compass orientation using the attitude estimate associated
  with each sample, and fix orientation on external compasses if
  the feature is enabled
 */
bool CompassCalibrator::calculate_orientation(void)
{
    if (!_check_orientation) {
        // we are not checking orientation
        return true;
    }

    // this function is very slow
    // EXPECT_DELAY_MS(1000);

    // Skip 4 rotations, see: auto_rotation_index()
    float variance[ROTATION_MAX-4] {};

    _orientation_solution = _orientation;
    
    for (uint8_t n=0; n < ARRAY_SIZE(variance); n++) {
        Rotation r = auto_rotation_index(n);

        // calculate the average implied earth field across all samples
        matrix::Vector3f total_ef {};
        for (uint32_t i=0; i<_samples_collected; i++) {
            matrix::Vector3f efield = calculate_earth_field(_sample_buffer[i], r);
            total_ef += efield;
        }
        matrix::Vector3f avg_efield = total_ef / _samples_collected;

        // now calculate the square error for this rotation against the average earth field
        for (uint32_t i=0; i<_samples_collected; i++) {
            matrix::Vector3f efield = calculate_earth_field(_sample_buffer[i], r);
            float err = (efield - avg_efield).norm_squared(); // length_squared
            // divide by number of samples collected to get the variance
            variance[n] += err / _samples_collected;
        }
    }

    // find the rotation with the lowest variance
    enum Rotation besti = ROTATION_NONE;
    float bestv = variance[0];
    enum Rotation besti_90 = ROTATION_NONE;
    float bestv_90 = variance[0];
    for (uint8_t n=0; n < ARRAY_SIZE(variance); n++) {
        Rotation r = auto_rotation_index(n);
        if (variance[n] < bestv) {
            bestv = variance[n];
            besti = r;
        }
        if (right_angle_rotation(r) && variance[n] < bestv_90) {
            bestv_90 = variance[n];
            besti_90 = r;
        }
    }


    float second_best = besti==ROTATION_NONE?variance[1]:variance[0];
    enum Rotation besti2 = besti==ROTATION_NONE?ROTATION_YAW_45:ROTATION_NONE;
    float second_best_90 = besti_90==ROTATION_NONE?variance[2]:variance[0];
    enum Rotation besti2_90 = besti_90==ROTATION_NONE?ROTATION_YAW_90:ROTATION_NONE;
    for (uint8_t n=0; n < ARRAY_SIZE(variance); n++) {
        Rotation r = auto_rotation_index(n);
        if (besti != r) {
            if (variance[n] < second_best) {
                second_best = variance[n];
                besti2 = r;
            }
        }
        if (right_angle_rotation(r) && (besti_90 != r) && (variance[n] < second_best_90)) {
            second_best_90 = variance[n];
            besti2_90 = r;
        }
    }

    _orientation_confidence = second_best/bestv;

    bool pass;
    if (besti == _orientation) {
        // if the orientation matched then allow for a low threshold
        pass = true;
    } else {
        if (_orientation_confidence > 4.0) {
            // very confident, always pass
            pass = true;

        } else if (_always_45_deg && (_orientation_confidence > 2.0)) {
            // use same tolerance for all rotations
            pass = true;

        } else {
            // just consider 90's
            _orientation_confidence = second_best_90 / bestv_90;
            pass = _orientation_confidence > 2.0;
            besti = besti_90;
            besti2 = besti2_90;
        }
    }
    if (!pass) {
//        GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "Mag(%u) bad orientation: %u/%u %.1f", _compass_idx,
//                        besti, besti2, (double)_orientation_confidence);
        (void)besti2;
    } else if (besti == _orientation) {
        // no orientation change
        // GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Mag(%u) good orientation: %u %.1f", _compass_idx, besti, (double)_orientation_confidence);
    } else if (!_is_external || !_fix_orientation) {
        // GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "Mag(%u) internal bad orientation: %u %.1f", _compass_idx, besti, (double)_orientation_confidence);
    } else {
        // GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Mag(%u) new orientation: %u was %u %.1f", _compass_idx, besti, _orientation, (double)_orientation_confidence);
    }

    if (!pass) {
        set_status(Status::BAD_ORIENTATION);
        return false;
    }

    if (_orientation == besti) {
        // no orientation change
        return true;
    }

    if (!_is_external || !_fix_orientation) {
        // we won't change the orientation, but we set _orientation
        // for reporting purposes
        _orientation = besti;
        _orientation_solution = besti;
        set_status(Status::BAD_ORIENTATION);
        return false;
    }

    // correct the offsets for the new orientation
    matrix::Vector3f rot_offsets = _params.offset;
    // rot_offsets.rotate_inverse(_orientation);
    vector3f_rotate_inverse(rot_offsets, _orientation);
    // rot_offsets.rotate(besti);
    vector3f_rotate(rot_offsets, besti);
    _params.offset = rot_offsets;

    // rotate the samples for the new orientation
    for (uint32_t i=0; i<_samples_collected; i++) {
        matrix::Vector3f s = _sample_buffer[i].get();
        //s.rotate_inverse(_orientation);
        vector3f_rotate_inverse(s, _orientation);
        // s.rotate(besti);
        vector3f_rotate(s, besti);
        _sample_buffer[i].set(s);
    }

    _orientation = besti;
    _orientation_solution = besti;

    // re-run the fit to get the diagonals and off-diagonals for the
    // new orientation
    initialize_fit();
    run_sphere_fit();
    run_ellipsoid_fit();

    return fit_acceptable();
}

/*
  fix radius of the fit to compensate for sensor scale factor errors
  return false if radius is outside acceptable range
 */
bool CompassCalibrator::fix_radius(void)
{
//    if (AP::gps().status() < AP_GPS::GPS_OK_FIX_2D) {
//        // we don't have a position, leave scale factor as 0. This
//        // will disable use of WMM in the EKF. Users can manually set
//        // scale factor after calibration if it is known
//        _params.scale_factor = 0;
//        return true;
//    }
//    const Location &loc = AP::gps().location();
//    float intensity;
//    float declination;
//    float inclination;
//    AP_Declination::get_mag_field_ef(loc.lat * 1e-7f, loc.lng * 1e-7f, intensity, declination, inclination);

//    float expected_radius = intensity * 1000; // mGauss
//    float correction = expected_radius / _params.radius;

//    if (correction > COMPASS_MAX_SCALE_FACTOR || correction < COMPASS_MIN_SCALE_FACTOR) {
//        // don't allow more than 30% scale factor correction
//        GCS_SEND_TEXT(MAV_SEVERITY_ERROR, "Mag(%u) bad radius %.0f expected %.0f",
//                        _compass_idx,
//                        _params.radius,
//                        expected_radius);
//        set_status(Status::BAD_RADIUS);
//        return false;
//    }

//    _params.scale_factor = correction;

    return true;
}

// convert index to rotation, this allows to skip some rotations
Rotation CompassCalibrator::auto_rotation_index(uint8_t n) const
{
    if (n < ROTATION_PITCH_180_YAW_90) {
        return (Rotation)n;
    }
    // ROTATION_PITCH_180_YAW_90 (26) is the same as ROTATION_ROLL_180_YAW_90 (10)
    // ROTATION_PITCH_180_YAW_270 (27) is the same as ROTATION_ROLL_180_YAW_270 (14)
    n += 2;
    if (n < ROTATION_ROLL_90_PITCH_68_YAW_293) {
        return (Rotation)n;
    }
    // ROTATION_ROLL_90_PITCH_68_YAW_293 is for solo leg compass, not expecting to see this in other vehicles
    n += 1;
    if (n < ROTATION_PITCH_315) {
        return (Rotation)n;
    }
    // ROTATION_PITCH_7 is too close to ROTATION_NONE to tell the difference in compass cal
    n += 1;
    if (n < ROTATION_MAX) {
        return (Rotation)n;
    }
    return ROTATION_NONE;
};

bool CompassCalibrator::right_angle_rotation(Rotation r) const
{
    switch (r) {
        case ROTATION_NONE:
        case ROTATION_YAW_90:
        case ROTATION_YAW_180:
        case ROTATION_YAW_270:
        case ROTATION_ROLL_180:
        case ROTATION_ROLL_180_YAW_90:
        case ROTATION_PITCH_180:
        case ROTATION_ROLL_180_YAW_270:
        case ROTATION_ROLL_90:
        case ROTATION_ROLL_90_YAW_90:
        case ROTATION_ROLL_270:
        case ROTATION_ROLL_270_YAW_90:
        case ROTATION_PITCH_90:
        case ROTATION_PITCH_270:
        case ROTATION_PITCH_180_YAW_90:
        case ROTATION_PITCH_180_YAW_270:
        case ROTATION_ROLL_90_PITCH_90:
        case ROTATION_ROLL_180_PITCH_90:
        case ROTATION_ROLL_270_PITCH_90:
        case ROTATION_ROLL_90_PITCH_180:
        case ROTATION_ROLL_270_PITCH_180:
        case ROTATION_ROLL_90_PITCH_270:
        case ROTATION_ROLL_180_PITCH_270:
        case ROTATION_ROLL_270_PITCH_270:
        case ROTATION_ROLL_90_PITCH_180_YAW_90:
        case ROTATION_ROLL_90_YAW_270:
            return true;

        default:
            return false;
    }
}

