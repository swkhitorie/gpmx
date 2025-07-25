#include "accel_calibrator.h"
using namespace calibration;
/*
 * TODO
 * - time out when not receiving samples
 */

////////////////////////////////////////////////////////////
///////////////////// PUBLIC INTERFACE /////////////////////
////////////////////////////////////////////////////////////

/*
	构造函数
*/
AccelCalibrator::AccelCalibrator() : _conf_tolerance(ACCEL_CAL_TOLERANCE), _sample_buffer(nullptr) {
    clear();
}
/*
    Select options, initialise variables and initiate accel calibration
    Options:
    Fit Type:       Will assume that if accelerometer static samples around all possible orientation
                    are spread in space will cover a surface of AXIS_ALIGNED_ELLIPSOID or any general 
                    ELLIPSOID as chosen

    Num Samples:    Number of samples user should will be gathering, please note that with type of surface
                    chosen the minimum number of samples required will vary, for instance in the case of AXIS
                    ALIGNED ELLIPSOID atleast 6 distinct samples are required while for generic ELLIPSOIDAL fit
                    which will include calculation of offdiagonal parameters too requires atleast 8 parameters
                    to form a distinct ELLIPSOID

    Sample Time:    Time over which the samples will be gathered and averaged to add to a single sample for least
                    square regression

    offset,diag,offdiag: initial parameter values for LSQ calculation
*/
/*
	初始化函数, 默认六面校准, 采样时间0.5s
*/
void AccelCalibrator::start(enum accel_cal_fit_type_t fit_type, uint8_t num_samples, float sample_time) {
    start(fit_type, num_samples, sample_time, matrix::Vector3f(0,0,0), matrix::Vector3f(1,1,1), matrix::Vector3f(0,0,0));
}

/*
	具体初始化函数, 默认六面校准, 采样时间0.5s
*/
void AccelCalibrator::start(enum accel_cal_fit_type_t fit_type, uint8_t num_samples, float sample_time, 
    matrix::Vector3f offset, matrix::Vector3f diag, matrix::Vector3f offdiag) {
    if (_status == ACCEL_CAL_FAILED || _status == ACCEL_CAL_SUCCESS) {
        clear();
    }
    if (_status != ACCEL_CAL_NOT_STARTED) {
        return;
    }

    _conf_num_samples = num_samples;
    _conf_sample_time = sample_time;
    _conf_fit_type = fit_type;

	//计算各面采样模的最小误差???
    const uint16_t faces = 2*_conf_num_samples-4;
    const float a = (4.0f * M_PI_F / (3.0f * faces)) + M_PI_F / 3.0f;
    const float theta = 0.5f * acosf(cosf(a) / (1.0f - cosf(a)));
    _min_sample_dist = GRAVITY_MSS * 2*sinf(theta/2);

    _param.s.offset = offset;
    _param.s.diag = diag;
    _param.s.offdiag = offdiag;

    switch (_conf_fit_type) {
        case ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID:
            if (_conf_num_samples < 6) {
                set_status(ACCEL_CAL_FAILED);
                return;
            }
            break;
        case ACCEL_CAL_ELLIPSOID:
            if (_conf_num_samples < 8) {
                set_status(ACCEL_CAL_FAILED);
                return;
            }
            break;
    }

    set_status(ACCEL_CAL_WAITING_FOR_ORIENTATION);
}


// 清除状态
void AccelCalibrator::clear() {
    set_status(ACCEL_CAL_NOT_STARTED);
}

// 检查是否正在校准运行
bool AccelCalibrator::running() {
    return _status == ACCEL_CAL_WAITING_FOR_ORIENTATION || _status == ACCEL_CAL_COLLECTING_SAMPLE;
}

// 设置并开始采集
void AccelCalibrator::collect_sample() {
    set_status(ACCEL_CAL_COLLECTING_SAMPLE);
}

// 循环调用, 采集计算数据, 所有的面采集完之后传递到LSQ估计器上
void AccelCalibrator::new_sample(const matrix::Vector3f& delta_velocity, float dt) {
    if (_status != ACCEL_CAL_COLLECTING_SAMPLE) {
        return;
    }

    if (_samples_collected >= _conf_num_samples) {
        set_status(ACCEL_CAL_FAILED);
        return;
    }

	//累计数据,并计算时间
    _sample_buffer[_samples_collected].delta_velocity += delta_velocity;
    _sample_buffer[_samples_collected].delta_time += dt;

	//计算上一次的时间
    _last_samp_frag_collected_ms = hrt_absolute_time();

	//如果某一面的累计时间大于配置的0.5s
    if (_sample_buffer[_samples_collected].delta_time > _conf_sample_time) {
		//计算平均值
        matrix::Vector3f sample = _sample_buffer[_samples_collected].delta_velocity/_sample_buffer[_samples_collected].delta_time;
        
        //计算采集的平均值是否满足要求
        if (!accept_sample(sample)) {
            set_status(ACCEL_CAL_FAILED);
            return;
        }

		//转向下一面
        _samples_collected++;

		//如果六面全部采集完毕
        if (_samples_collected >= _conf_num_samples) {
			//计算采集参数, 迭代次数50次, 最小残差0.1
            run_fit(MAX_ITERATIONS, _fitness);

            if (_fitness < _conf_tolerance && accept_result()) {
                set_status(ACCEL_CAL_SUCCESS);
            } else {
                set_status(ACCEL_CAL_FAILED);
            }
        } else {
            set_status(ACCEL_CAL_WAITING_FOR_ORIENTATION);
        }
    }
}

//检查校准结果是否在可认可范围内
bool AccelCalibrator::accept_result() const {
    if (fabsf(_param.s.offset(0)) > GRAVITY_MSS ||
        fabsf(_param.s.offset(1)) > GRAVITY_MSS ||
        fabsf(_param.s.offset(2)) > GRAVITY_MSS ||
        _param.s.diag(0) < 0.8f || _param.s.diag(0) > 1.2f ||
        _param.s.diag(1) < 0.8f || _param.s.diag(1) > 1.2f ||
        _param.s.diag(2) < 0.8f || _param.s.diag(2) > 1.2f) {
        return false;
    } else {
        return true;
    }
}

// LSQ估计器接口, 读取某一面的加速度平均值
bool AccelCalibrator::get_sample(uint8_t i, matrix::Vector3f& s) const {
    if (i >= _samples_collected) {
        return false;
    }
    s = _sample_buffer[i].delta_velocity / _sample_buffer[i].delta_time;
    return true;
}

// 读取经过矫正后的 某一面的加速度平均值
bool AccelCalibrator::get_sample_corrected(uint8_t i, matrix::Vector3f& s) const {
    if (_status != ACCEL_CAL_SUCCESS || !get_sample(i,s)) {
        return false;
    }

    const float init_matrix_data[3*3] = {
        _param.s.diag(0)    , _param.s.offdiag(0) , _param.s.offdiag(1),
        _param.s.offdiag(0) , _param.s.diag(1)    , _param.s.offdiag(2),
        _param.s.offdiag(1) , _param.s.offdiag(2) , _param.s.diag(2)
    };
    
    matrix::Matrix3f M(init_matrix_data);

    s = M * (s + _param.s.offset);

    return true;
}

// 计算采集时间是否超时
void AccelCalibrator::check_for_timeout() {
    const uint32_t timeout = _conf_sample_time*2*1000 + 500;
    if (_status == ACCEL_CAL_COLLECTING_SAMPLE && hrt_absolute_time() - _last_samp_frag_collected_ms > timeout) {
        set_status(ACCEL_CAL_FAILED);
    }
}

 // 获取校准参数
void AccelCalibrator::get_calibration(matrix::Vector3f& offset) const {
    offset = -_param.s.offset;
}

 // 获取校准参数
void AccelCalibrator::get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag) const {
    offset = -_param.s.offset;
    diag = _param.s.diag;
}

 // 获取校准参数
void AccelCalibrator::get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag, matrix::Vector3f& offdiag) const {
    offset = -_param.s.offset;
    diag = _param.s.diag;
    offdiag = _param.s.offdiag;
}

/////////////////////////////////////////////////////////////
////////////////////// PRIVATE METHODS //////////////////////
/////////////////////////////////////////////////////////////

/*
  The sample acceptance distance is determined as follows:
  For any regular polyhedron with triangular faces, the angle theta subtended
  by two closest points is defined as
 
       theta = arccos(cos(A)/(1-cos(A)))
 
  Where:
       A = (4pi/F + pi)/3
  and
       F = 2V - 4 is the number of faces for the polyhedron in consideration,
       which depends on the number of vertices V
 
  The above equation was proved after solving for spherical triangular excess
  and related equations.
 */
 //检查采样平均值的健康性
bool AccelCalibrator::accept_sample(const matrix::Vector3f& sample)
{
    if (_sample_buffer == nullptr) {
        return false;
    }

    for(uint8_t i=0; i < _samples_collected; i++) {
        matrix::Vector3f other_sample;
        get_sample(i, other_sample);

        if ((other_sample - sample).length() < _min_sample_dist){
            return false;
        }
    }
    return true;
}

//主运行函数
void AccelCalibrator::set_status(enum accel_cal_status_t status) {
    switch (status) {
        case ACCEL_CAL_NOT_STARTED:                 
            //Calibrator not started
            _status = ACCEL_CAL_NOT_STARTED;

            _samples_collected = 0;
            if (_sample_buffer != nullptr) {
                free(_sample_buffer);
                _sample_buffer = nullptr;
            }

            break;

        case ACCEL_CAL_WAITING_FOR_ORIENTATION:     
            //Callibrator has been started and is waiting for user to ack after orientation setting
            if (!running()) {
                _samples_collected = 0;
                if (_sample_buffer == nullptr) {
                    _sample_buffer = (struct AccelSample*)calloc(_conf_num_samples,sizeof(struct AccelSample));
                    if (_sample_buffer == nullptr) {
                        set_status(ACCEL_CAL_FAILED);
                        break;
                    }
                }
            }
            if (_samples_collected >= _conf_num_samples) {
                break;
            }
            _status = ACCEL_CAL_WAITING_FOR_ORIENTATION;
            break;

        case ACCEL_CAL_COLLECTING_SAMPLE:
            // Calibrator is waiting on collecting samples from accelerometer for amount of 
            // time as requested by user/GCS
            if (_status != ACCEL_CAL_WAITING_FOR_ORIENTATION) {
                break;
            }
            _last_samp_frag_collected_ms = hrt_absolute_time();
            _status = ACCEL_CAL_COLLECTING_SAMPLE;
            break;

        case ACCEL_CAL_SUCCESS:
            // Calibrator has successfully fitted the samples to user requested surface model 
            // and has passed tolerance test
            if (_status != ACCEL_CAL_COLLECTING_SAMPLE) {
                break;
            }

            _status = ACCEL_CAL_SUCCESS;
            break;

        case ACCEL_CAL_FAILED:
            // Calibration has failed with reasons that can range from 
            // bad sample data leading to failure in tolerance test to lack of distinct samples
            if (_status == ACCEL_CAL_NOT_STARTED) {
                break;
            }

            _status = ACCEL_CAL_FAILED;
            break;
    };
}

/*
    Run Gauss Newton fitting algorithm over the sample space and come up with offsets, diagonal/scale factors
    and crosstalk/offdiagonal parameters
*/
//在样本空间上运行高斯-牛顿拟合算法，并得出偏移、对角线/比例因子和串扰/非对角线参数
void AccelCalibrator::run_fit(uint8_t max_iterations, float& fitness)
{
    if (_sample_buffer == nullptr) {
        return;
    }
    fitness = calc_mean_squared_residuals(_param.s);
    float min_fitness = fitness;
    union param_u fit_param;
    fit_param.a = _param.a;
    fit_param.s = _param.s;
    
    uint8_t num_iterations = 0;

    while(num_iterations < max_iterations) {
        float JTJ[ACCEL_CAL_MAX_NUM_PARAMS * ACCEL_CAL_MAX_NUM_PARAMS] {};
        VectorP JTFI;

        for(uint16_t k = 0; k<_samples_collected; k++) {
            matrix::Vector3f sample;
            get_sample(k, sample);

            matrix::Vector<float, ACCEL_CAL_MAX_NUM_PARAMS> jacob;

            calc_jacob(sample, fit_param.s, jacob);

            for(uint8_t i = 0; i < get_num_params(); i++) {
                // compute JTJ
                for(uint8_t j = 0; j < get_num_params(); j++) {
                    JTJ[i*get_num_params()+j] += jacob(i) * jacob(j);
                }
                // compute JTFI
                JTFI(i) += jacob(i) * calc_residual(sample, fit_param.s);
            }
        }
        
        if (get_num_params() == 6) {
            matrix::Matrix<float, 6, 6> JTJ_t(JTJ);
            matrix::Matrix<float, 6, 6> JTJ_inv_t;
            if (!geninv(JTJ_t, JTJ_inv_t)) return;
            for(uint8_t row=0; row < get_num_params(); row++) {
                for(uint8_t col=0; col < get_num_params(); col++) {
                    fit_param.a(row) -= JTFI(col) * JTJ_inv_t(row, col);
                }
            }
        } else if (get_num_params() == 9) {
            matrix::Matrix<float, 9, 9> JTJ_t(JTJ);
            matrix::Matrix<float, 9, 9> JTJ_inv_t;
            if (!geninv(JTJ_t, JTJ_inv_t)) return;
            for(uint8_t row=0; row < get_num_params(); row++) {
                for(uint8_t col=0; col < get_num_params(); col++) {
                    fit_param.a(row) -= JTFI(col) * JTJ_inv_t(row, col);
                }
            }   
        }



        fitness = calc_mean_squared_residuals(fit_param.s);

        if (isnan(fitness) || isinf(fitness)) {
            return;
        }

        if (fitness < min_fitness) {
            min_fitness = fitness;
            //_param = fit_param;
            _param.a = fit_param.a;
            _param.s = fit_param.s;
        }

        num_iterations++;
    }
}

//计算从样本（使用提供的参数校正）到重力的残差
float AccelCalibrator::calc_residual(const matrix::Vector3f& sample, const struct param_t& params) const {
    
    const float init_matrix_data[3*3] = {
        params.diag(0)    , params.offdiag(0) , params.offdiag(1),
        params.offdiag(0) , params.diag(1)    , params.offdiag(2),
        params.offdiag(1) , params.offdiag(2) , params.diag(2)
    };
    matrix::Matrix3f M(init_matrix_data);
    const matrix::Vector3f tmp = (M * (sample + params.offset));
    return GRAVITY_MSS - tmp.length();
}

//使用参数计算所有采集样本的总均方适应度
float AccelCalibrator::calc_mean_squared_residuals(const struct param_t& params) const
{
    if (_sample_buffer == nullptr || _samples_collected == 0) {
        return 1.0e30f;
    }
    float sum = 0.0f;
    
    for(uint16_t i=0; i < _samples_collected; i++){
        matrix::Vector3f sample;
        get_sample(i, sample);
        float resid = calc_residual(sample, params);
        sum += math::sq(resid);
    }
    sum /= _samples_collected;
    return sum;
}

//计算雅可比矩阵，该矩阵定义了适应度随每个参数变化的关系
//这在LSq估计器中用于调整参数的变化，以便用于LSq的下一次迭代
void AccelCalibrator::calc_jacob(const matrix::Vector3f& sample, const struct param_t& params, VectorP &ret) const {
    switch (_conf_fit_type) {
        case ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID:
        case ACCEL_CAL_ELLIPSOID: {
            const matrix::Vector3f &offset = params.offset;
            const matrix::Vector3f &diag = params.diag;
            const matrix::Vector3f &offdiag = params.offdiag;
            
            const float init_matrix_data[3*3] = {
                diag(0)    , offdiag(0) , offdiag(1),
                offdiag(0) , diag(1)    , offdiag(2),
                offdiag(1) , offdiag(2) , diag(2)
            };
            matrix::Matrix3f M(init_matrix_data);

            float A =  (diag(0)    * (sample(0) + offset(0))) + (offdiag(0) * (sample(1) + offset(1))) + (offdiag(1) * (sample(2) + offset(2)));
            float B =  (offdiag(0) * (sample(0) + offset(0))) + (diag(1)    * (sample(1) + offset(1))) + (offdiag(2) * (sample(2) + offset(2)));
            float C =  (offdiag(1) * (sample(0) + offset(0))) + (offdiag(2) * (sample(1) + offset(1))) + (diag(2)    * (sample(2) + offset(2)));
            
            const matrix::Vector3f tmp = M * (sample + offset);
            float length = tmp.length();

            // 0-2: offsets
            ret.setRow(0, -1.0f * (((diag(0)    * A) + (offdiag(0) * B) + (offdiag(1) * C))/length));
            ret.setRow(1, -1.0f * (((offdiag(0) * A) + (diag(1)    * B) + (offdiag(2) * C))/length));
            ret.setRow(2, -1.0f * (((offdiag(1) * A) + (offdiag(2) * B) + (diag(2)    * C))/length));
            
            // 3-5: diagonals
            ret.setRow(3, -1.0f * ((sample(0) + offset(0)) * A)/length);
            ret.setRow(4, -1.0f * ((sample(1) + offset(1)) * B)/length);
            ret.setRow(5, -1.0f * ((sample(2) + offset(2)) * C)/length);

            // 6-8: off-diagonals
            ret.setRow(6, -1.0f * (((sample(1) + offset(1)) * A) + ((sample(0) + offset(0)) * B))/length);
            ret.setRow(7, -1.0f * (((sample(2) + offset(2)) * A) + ((sample(0) + offset(0)) * C))/length);
            ret.setRow(8, -1.0f * (((sample(2) + offset(2)) * B) + ((sample(1) + offset(1)) * C))/length);
            return;
        }
    }
}

//返回所选拟合类型所需的参数数
uint8_t AccelCalibrator::get_num_params() const {
    switch (_conf_fit_type) {
        case ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID:
            return 6;
        case ACCEL_CAL_ELLIPSOID:
            return 9;
        default:
            return 6;
    }
}
