#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>

// 加速度计校准最大参数个数: 六面校准/九面椭球拟合校准
#define ACCEL_CAL_MAX_NUM_PARAMS 	9
#define ACCEL_CAL_TOLERANCE 		0.1
#define MAX_ITERATIONS  			50

namespace calibration
{
    
/*
	加速度计校准状态枚举
		未开始
		等待固定方向姿势
		采集数据样本
		校准成功
		校准失败
*/
enum accel_cal_status_t {
    ACCEL_CAL_NOT_STARTED=0,
    ACCEL_CAL_WAITING_FOR_ORIENTATION=1,
    ACCEL_CAL_COLLECTING_SAMPLE=2,
    ACCEL_CAL_SUCCESS=3,
    ACCEL_CAL_FAILED=4
};

/*
	加速度计校准类型枚举
		六面轴对准校准法 (默认)
		椭球拟合校准法
*/
enum accel_cal_fit_type_t {
    ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID=0,
    ACCEL_CAL_ELLIPSOID=1
};

class AccelCalibrator {
public:
    AccelCalibrator();

    /*
		start() 初始化参数
		fit_type: 校准方法, 默认六面轴对准校准
		num_samples: 采集次数, 六面对应六次
		sample_time: 采集时间, 0.5s采集一次
		offset: 零偏误差
		diag: 对角元素
		offdiag: 非对角元素
    */
    void start(enum accel_cal_fit_type_t fit_type = ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID, uint8_t num_samples = 6, float sample_time = 0.5f);
    void start(enum accel_cal_fit_type_t fit_type, uint8_t num_samples, float sample_time, matrix::Vector3f offset, matrix::Vector3f diag, matrix::Vector3f offdiag);
    
    // 清除状态
    void clear();
	// 检查是否正在校准运行
    bool running();
	// 设置并开始采集
    void collect_sample();
	// 计算采集时间是否超时
    void check_for_timeout();

    // 获取校准参数
    void get_calibration(matrix::Vector3f& offset) const;
    void get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag) const;
    void get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag, matrix::Vector3f& offdiag) const;

	// 循环调用, 采集计算数据, 所有的面采集完之后传递到LSQ估计器上
    void new_sample(const matrix::Vector3f& delta_velocity, float dt);

    // LSQ估计器接口, 读取某一面的加速度平均值
    bool get_sample(uint8_t i, matrix::Vector3f& s) const;
    // 读取经过矫正后的 某一面的加速度平均值
    bool get_sample_corrected(uint8_t i, matrix::Vector3f& s) const;

    // set tolerance bar for parameter fitness value to cross so as to be deemed as correct values
    void set_tolerance(float tolerance) { _conf_tolerance = tolerance; }

    // 校准状态接口
    enum accel_cal_status_t get_status() const { return _status; }
	// 返回采样点个数
    uint8_t get_num_samples_collected() const { return _samples_collected; }

    // returns mean squared fitness of sample points to the selected surface
    float get_fitness() const { return _fitness; }

    struct param_t {
        matrix::Vector3f offset;
        matrix::Vector3f diag;
        matrix::Vector3f offdiag;
    };

private:
    struct AccelSample {
        matrix::Vector3f delta_velocity;
        float delta_time;
    };
    typedef matrix::Vector<float, ACCEL_CAL_MAX_NUM_PARAMS> VectorP;

    union param_u {
        struct param_t s;
        matrix::Vector<float, ACCEL_CAL_MAX_NUM_PARAMS> a;

        param_u() : a{} {
            static_assert(sizeof(*this) == sizeof(struct param_t), "Invalid union members: sizes do not match");
        }
    };

    //configuration
    uint8_t _conf_num_samples;					//配置采样个数--默认6次
    float _conf_sample_time;					//配置采样时间--0.5s
    enum accel_cal_fit_type_t _conf_fit_type;	//配置校准类型--默认六面
    float _conf_tolerance;						//配置公差

    // state
    accel_cal_status_t _status;					//校准状态
    struct AccelSample* _sample_buffer;			//采样点数组, 默认长度为6
    uint8_t _samples_collected;					//已采个数, 可做为采样点数组索引
    union param_u _param;							//校准参数
    float _fitness;
    uint32_t _last_samp_frag_collected_ms;		//上次采样时间
    float _min_sample_dist;

    bool accept_sample(const matrix::Vector3f& sample);	//检查采样平均值的健康性
    void reset_state();							//废弃函数

    void set_status(enum accel_cal_status_t);	//主运行函数

    bool accept_result() const;					//检查校准结果是否在可认可范围内

    uint8_t get_num_params() const;				//返回所选拟合类型所需的参数数

    // 与高斯-牛顿最小二乘回归过程有关的函数
    
    //计算从样本（使用提供的参数校正）到重力的残差
    float calc_residual(const matrix::Vector3f& sample, const struct param_t& params) const;
    
    //使用参数计算所有采集样本的总均方适应度
    float calc_mean_squared_residuals(const struct param_t& params) const;
    
    //计算雅可比矩阵，该矩阵定义了适应度随每个参数变化的关系
    //这在LSq估计器中用于调整参数的变化，以便用于LSq的下一次迭代
    void calc_jacob(const matrix::Vector3f& sample, const struct param_t& params, VectorP& ret) const;
    
    //在样本空间上运行高斯-牛顿拟合算法，并得出偏移、对角线/比例因子和串扰/非对角线参数
    void run_fit(uint8_t max_iterations, float& fitness);
};

}
