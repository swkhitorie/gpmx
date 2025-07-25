#include "accel_calibrator_bak.h"

using namespace calibration;
AccelCalibratorBak::AccelCalibratorBak() {
	clear();
    update_params();
}

void AccelCalibratorBak::clear()
{
    _state = ACCEL_CAL_IDLE;
    
    _sample_period_us = 2 * 1e6f;
    _lst_sample_time = hrt_absolute_time();
    
    _samples_collected = 0;
    _orientaion_collected = ACCEL_POS_NULL;
	for (uint8_t i = 0; i < 6; i++) {
        _sample[i].zero();
        _samples_completed[i] = false;
	}

//	offset.zero();
//	scale.setOne();
//    offset(0) = 0.772f;
//    offset(1) = 0.171f;
//    offset(2) = 0.270f;
//    scale(0) = 0.999f;
//    scale(1) = 1.000f;
//    scale(2) = 0.982f; 
}

void AccelCalibratorBak::update_params()
{
    if ((fabsf(offset(0) - _param_offs_x.get()) > 0.002f) && PX4_ISFINITE(_param_offs_x.get())) offset(0) = _param_offs_x.get();
    if ((fabsf(offset(1) - _param_offs_y.get()) > 0.002f) && PX4_ISFINITE(_param_offs_y.get())) offset(1) = _param_offs_y.get();
    if ((fabsf(offset(2) - _param_offs_z.get()) > 0.002f) && PX4_ISFINITE(_param_offs_z.get())) offset(2) = _param_offs_z.get();
    if ((fabsf(scale(0) - _param_scal_x.get()) > 0.002f) && PX4_ISFINITE(_param_scal_x.get())) scale(0) = _param_scal_x.get();
    if ((fabsf(scale(1) - _param_scal_y.get()) > 0.002f) && PX4_ISFINITE(_param_scal_y.get())) scale(1) = _param_scal_y.get();
    if ((fabsf(scale(2) - _param_scal_z.get()) > 0.002f) && PX4_ISFINITE(_param_scal_z.get())) scale(2) = _param_scal_z.get();
}

void AccelCalibratorBak::commit_params()
{
    _param_offs_x.set(offset(0));
    _param_offs_y.set(offset(1));
    _param_offs_z.set(offset(2));
    _param_scal_x.set(scale(0));
    _param_scal_y.set(scale(1));
    _param_scal_z.set(scale(2));
    
    _param_offs_x.commit();
    _param_offs_y.commit();
    _param_offs_z.commit();
    _param_scal_x.commit();
    _param_scal_y.commit();
    _param_scal_z.commit();
}

void AccelCalibratorBak::reset(float dS[6], float JS[6][6])
{
	for (int16_t j = 0; j < 6; j++) {
        dS[j] = 0.0f;
        for (int16_t k = 0; k < 6; k++) {
            JS[j][k] = 0.0f;
        }
	}
}

void AccelCalibratorBak::find_delta(float dS[6], float JS[6][6], float delta[6])
{
	//Solve 6-d matrix equation JS*x = dS
	//first put in upper triangular form
	int16_t i, j, k;
	float mu;
    
	//make upper triangular
	for (i = 0; i < 6; i++) {
        //eliminate all nonzero entries below JS[i][i]
        for (j = i + 1; j < 6; j++) {
            mu = JS[i][j] / JS[i][i];
            if (mu != 0.0f) {
                dS[j] -= mu * dS[i];
                for (k = j; k < 6; k++) {
                    JS[k][j] -= mu * JS[k][i];
                }
            }
        }
	}
	//back-substitute
	for (i = 5; i >= 0; i--) {
        dS[i] /= JS[i][i];
        JS[i][i] = 1.0f;
			
        for (j = 0; j < i; j++) {
            mu = JS[i][j];
            dS[j] -= mu * dS[i];
            JS[i][j] = 0.0f;
        }
	}
    
	for(i = 0; i < 6; i++) {
        delta[i] = dS[i];
	}	
}

void AccelCalibratorBak::update(float dS[6], float JS[6][6], float beta[6], float data[3])
{
	int16_t j, k;
	float dx, b;
	float residual = 1.0;
	float jacobian[6];
	
	for (j = 0; j < 3; j++) {
        b = beta[3 + j];
        dx = (float)data[j] - beta[j];
        residual -= b * b * dx * dx;
        jacobian[j] = 2.0f * b * b * dx;
        jacobian[3 + j] = -2.0f * b * dx * dx;
	}
	
	for (j = 0; j < 6; j++) {
        dS[j] += jacobian[j] * residual;
        for (k = 0; k < 6; k++) {
            JS[j][k] += jacobian[j] * jacobian[k];
        }
	}	
}

bool AccelCalibratorBak::running_fit(matrix::Vector3f (&_sample)[6], matrix::Vector3f &accel_offsets, matrix::Vector3f &accel_scale)
{
	int16_t i;
	int16_t num_iterations = 0;
	float eps = 0.000000001;
	float change = 100.0;
	float data[3];
	float beta[6];
	float delta[6];
	float ds[6];
	float JS[6][6];
	bool success = true;

	// reset
	beta[0] = beta[1] = beta[2] = 0;
	beta[3] = beta[4] = beta[5] = 1.0f / GRAVITY_MSS;
	
	while (num_iterations < 20 && change > eps) {
        num_iterations++;

        reset(ds, JS);

        for (i = 0; i < 6; i++) {
            data[0] = _sample[i](0);
            data[1] = _sample[i](1);
            data[2] = _sample[i](2);
            update(ds, JS, beta, data);
        }

        find_delta(ds, JS, delta);

        change =delta[0] * delta[0] +
                delta[0] * delta[0] +
                delta[1] * delta[1] +
                delta[2] * delta[2] +
                delta[3] * delta[3] / (beta[3] * beta[3]) +
                delta[4] * delta[4] / (beta[4] * beta[4]) +
                delta[5] * delta[5] / (beta[5] * beta[5]);

        for (i = 0; i < 6; i++) {
            beta[i] -= delta[i];
        }
	}

	// copy results out
	accel_scale.setRow(0, beta[3] * GRAVITY_MSS);
	accel_scale.setRow(1, beta[4] * GRAVITY_MSS);    
 	accel_scale.setRow(2, beta[5] * GRAVITY_MSS);   

 	accel_offsets.setRow(0, beta[0] * accel_scale(0));
	accel_offsets.setRow(1, beta[1] * accel_scale(1));    
 	accel_offsets.setRow(2, beta[2] * accel_scale(2));  

	// sanity check scale
	if (accel_scale.isAllNan() || fabsf(accel_scale(0) - 1.0f) > 0.1f 
        || fabsf(accel_scale(1) - 1.0f) > 0.1f || fabsf(accel_scale(2) - 1.0f) > 0.1f ) {
        success = false;
	}
	// sanity check offsets (3.5 is roughly 3/10th of a G, 5.0 is roughly half a G)
	if (accel_offsets.isAllNan() || fabsf(accel_offsets(0)) > 3.5f || 
        fabsf(accel_offsets(1)) > 3.5f || fabsf(accel_offsets(2)) > 3.5f ) {
        success = false;
	}

	// return success or failure
	return success;
}

void AccelCalibratorBak::run(matrix::Vector3f &accel_sample, float gyrolen)
{
    bool result = false;
    
    switch (_state) {
    case ACCEL_CAL_IDLE: break;
    case ACCEL_CAL_WAITING_FOR_ORIENTATION: break;
    case ACCEL_CAL_SAMPLEING_COLLECTING:
        
        if (!_samples_completed[_orientaion_collected]) {
            if (gyrolen < 5.0f) {
                _sample[_orientaion_collected] += accel_sample;
                _samples_collected++;            
            }
            
            if ((hrt_absolute_time() - _lst_sample_time) >= _sample_period_us) {
                _sample[_orientaion_collected] = _sample[_orientaion_collected] / _samples_collected;
                _samples_completed[_orientaion_collected] = true;
                _samples_collected = 0;
                
                if (_orientaion_collected == 5) {
                    result = running_fit(_sample, offset, scale);
                    if (result) {
                        _state = ACCEL_CAL_SUCCESS;
                    } else {
                        _state = ACCEL_CAL_FAILED;
                    }                        
                }
            }
        }
        break;
    case ACCEL_CAL_SUCCESS: break;
    case ACCEL_CAL_FAILED: break;
    }
}

bool AccelCalibratorBak::next()
{
    if (_state != ACCEL_CAL_WAITING_FOR_ORIENTATION) return false;
    _state = ACCEL_CAL_SAMPLEING_COLLECTING;
    
    switch (_orientaion_collected) {
    case ACCEL_POS_NULL: _orientaion_collected = ACCEL_POS_LEVEL; break;
    case ACCEL_POS_LEVEL: _orientaion_collected = ACCEL_POS_LEFT; break;
    case ACCEL_POS_LEFT: _orientaion_collected = ACCEL_POS_RIGHT; break;
    case ACCEL_POS_RIGHT: _orientaion_collected = ACCEL_POS_NOSEDOWN; break;
    case ACCEL_POS_NOSEDOWN: _orientaion_collected = ACCEL_POS_NOSEUP; break;
    case ACCEL_POS_NOSEUP: _orientaion_collected = ACCEL_POS_BACK; break;
    case ACCEL_POS_BACK: return true;
    } 
    
    _lst_sample_time = hrt_absolute_time();

    return false;
}

matrix::Vector3f AccelCalibratorBak::correct(const matrix::Vector3f &accel_raw)
{
    const float init_matrix_data[3*3] = {
        scale(0),       0,         0,
        0,          scale(1),      0,
        0,              0,      scale(2)
    };
    matrix::Matrix3f scale_correct {init_matrix_data};
    matrix::Vector3f accel_correct;
    
    accel_correct = scale_correct * accel_raw - offset;
    
    return accel_correct;
}

bool AccelCalibratorBak::enabled()
{
    return true;
}

