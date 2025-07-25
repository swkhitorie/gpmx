#include "compass_calibrator_bak.h"
#include <math.h>
using namespace calibration;
static double Abs(double a) {return a < 0 ? -a : a; }

/*
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
*/

void CompassCalibratorBak::row1_swap_row2(uint8_t row1, uint8_t row2)
{
	for (uint8_t col = 0; col < 6 + 1; col++) {
        float temp = 0.0f;
		temp = _equation[row1][col];
		_equation[row1][col] = _equation[row2][col];
		_equation[row2][col] = temp;
	}  
}

void CompassCalibratorBak::k_muilt_row(double k,uint8_t row)
{
	for (uint8_t col = 0; col < 6 + 1; col++)
		_equation[row][col] *= k;
}

void CompassCalibratorBak::row2_add_krow1(double k,uint8_t row1,uint8_t row2)
{
	for (uint8_t col = 0; col < 6 + 1; col++)
		_equation[row2][col] += k * _equation[row1][col];
}

void CompassCalibratorBak::row_simplify()
{
	float k = 0;
	for (uint8_t row = 0; row < 6; row++) {
		k = 1 / _equation[row][row];
		k_muilt_row(k, row);
	}
}

void CompassCalibratorBak::move_biggestelement_to_top(uint8_t k)
{
	uint8_t row = 0;
	for (row = k + 1; row < 6; row++)
		if (Abs(_equation[k][k]) < Abs(_equation[row][k]))
			row1_swap_row2(k,row);
}

bool CompassCalibratorBak::gauss_elimination()
{
	double k = 0;
	for (uint8_t cnt = 0; cnt < 6;cnt++) {
		move_biggestelement_to_top(cnt);
		if (_equation[cnt][cnt] == 0) return false;
		for (uint8_t row = cnt + 1; row < 6; row++) {
			k = -_equation[row][cnt] / _equation[cnt][cnt];
			row2_add_krow1(k,cnt,row);
		}	
	}
	return true;	
}

void CompassCalibratorBak::calculate_inputaverage()
{
	for(uint8_t row = 0; row < 6; row++)
		for(uint8_t col = 0; col < 6 + 1; col++)
				_equation[row][col] /= _sample_collected;
}

void CompassCalibratorBak::matrix_solve(double *solve)
{
	for (short row = 6 - 1; row >= 0; row--) {
		_linear_equation_solve[row] = _equation[row][6];
		for (uint8_t col = 6 - 1; col >= row + 1; col--)
			_linear_equation_solve[row] -= _equation[row][col] * _linear_equation_solve[col];
	}
}

bool CompassCalibratorBak::enabled()
{
    return true;
}

void CompassCalibratorBak::clear()
{
	_sample_collected = 0;
    
    for(int8_t i = 0; i < 6; i++) {
        _paramlist[i] = _linear_equation_solve[i] = 0;
    }
    
	for (uint8_t row = 0; row < 6; row++) {
		for (uint8_t col = 0; col < 6; col++)
			_equation[row][col] = 0.0f;
	}
    
    _state = COMPASS_CAL_IDLE;
}

void CompassCalibratorBak::start()
{
    _state = COMPASS_CAL_SAMPLEING_COLLECTING;
    _sample_begin_time = hrt_absolute_time();
}

void CompassCalibratorBak::sample_point(double x, double y, double z)
{
    uint8_t row,col;
    
	double V[6 + 1];

	V[0] = y * y;
	V[1] = z * z;
	V[2] = x;
	V[3] = y;
	V[4] = z;
	V[5] = 1.0;
	V[6] = -1 * x * x;
    
    for (row = 0; row < 6; row++) {
        for (col = 0; col < 6 + 1; col++) {
            _equation[row][col] += V[row] * V[col];
        }
    }
    _sample_collected++;
}

bool CompassCalibratorBak::running_fit()
{
	calculate_inputaverage();
    
	if (!gauss_elimination()) return false;
    
    row_simplify();
    
    matrix_solve(_linear_equation_solve);
		
    double a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    a = _linear_equation_solve[0];
    b = _linear_equation_solve[1];
    c = _linear_equation_solve[2];
    d = _linear_equation_solve[3];
    e = _linear_equation_solve[4];
    f = _linear_equation_solve[5];

    double X0 = 0, Y0 = 0, Z0 = 0, A = 0, B = 0, C = 0;
    X0 = -c / 2;
    Y0 = -d / (2 * a);
    Z0 = -e / (2 * b);
    A = sqrt(X0 * X0 + a * Y0 * Y0 + b * Z0 * Z0 - f);
    B = A / sqrt(a);
    C = A / sqrt(b);
    
    _paramlist[0] = X0;
    _paramlist[1] = Y0;
    _paramlist[2] = Z0;
    _paramlist[3] = A;
    _paramlist[4] = B;
    _paramlist[5] = C;
    
	return true;
}

void CompassCalibratorBak::run(matrix::Vector3f &mag_sample)
{
    float x, y, z;
    x = mag_sample(0);
    y = mag_sample(1);
    z = mag_sample(2);
    sample = mag_sample;
    
    switch (_state) {
    case COMPASS_CAL_IDLE: break;
    case COMPASS_CAL_SAMPLEING_COLLECTING: 
        if ((hrt_absolute_time() - _sample_begin_time) >= COMPASS_CAL_SAMPLE_US) {
            if (running_fit()) {
                _state = COMPASS_CAL_SUCCESS;
            } else {
                _state = COMPASS_CAL_FAILED;
            }
        } else {
            sample_point(x, y, z);
            _percent = (hrt_absolute_time() - _sample_begin_time) * 100 / COMPASS_CAL_SAMPLE_US;
        }
        break;
    case COMPASS_CAL_SUCCESS: break;
    case COMPASS_CAL_FAILED: break;
    }
}


