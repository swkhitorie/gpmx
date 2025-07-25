#pragma once

#include "platform_defines.h"
#include <mathlib/mathlib.h>
#include <matrix/matrix/math.hpp>

// ���ٶȼ�У׼����������: ����У׼/�����������У׼
#define ACCEL_CAL_MAX_NUM_PARAMS 	9
#define ACCEL_CAL_TOLERANCE 		0.1
#define MAX_ITERATIONS  			50

namespace calibration
{
    
/*
	���ٶȼ�У׼״̬ö��
		δ��ʼ
		�ȴ��̶���������
		�ɼ���������
		У׼�ɹ�
		У׼ʧ��
*/
enum accel_cal_status_t {
    ACCEL_CAL_NOT_STARTED=0,
    ACCEL_CAL_WAITING_FOR_ORIENTATION=1,
    ACCEL_CAL_COLLECTING_SAMPLE=2,
    ACCEL_CAL_SUCCESS=3,
    ACCEL_CAL_FAILED=4
};

/*
	���ٶȼ�У׼����ö��
		�������׼У׼�� (Ĭ��)
		�������У׼��
*/
enum accel_cal_fit_type_t {
    ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID=0,
    ACCEL_CAL_ELLIPSOID=1
};

class AccelCalibrator {
public:
    AccelCalibrator();

    /*
		start() ��ʼ������
		fit_type: У׼����, Ĭ���������׼У׼
		num_samples: �ɼ�����, �����Ӧ����
		sample_time: �ɼ�ʱ��, 0.5s�ɼ�һ��
		offset: ��ƫ���
		diag: �Խ�Ԫ��
		offdiag: �ǶԽ�Ԫ��
    */
    void start(enum accel_cal_fit_type_t fit_type = ACCEL_CAL_AXIS_ALIGNED_ELLIPSOID, uint8_t num_samples = 6, float sample_time = 0.5f);
    void start(enum accel_cal_fit_type_t fit_type, uint8_t num_samples, float sample_time, matrix::Vector3f offset, matrix::Vector3f diag, matrix::Vector3f offdiag);
    
    // ���״̬
    void clear();
	// ����Ƿ�����У׼����
    bool running();
	// ���ò���ʼ�ɼ�
    void collect_sample();
	// ����ɼ�ʱ���Ƿ�ʱ
    void check_for_timeout();

    // ��ȡУ׼����
    void get_calibration(matrix::Vector3f& offset) const;
    void get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag) const;
    void get_calibration(matrix::Vector3f& offset, matrix::Vector3f& diag, matrix::Vector3f& offdiag) const;

	// ѭ������, �ɼ���������, ���е���ɼ���֮�󴫵ݵ�LSQ��������
    void new_sample(const matrix::Vector3f& delta_velocity, float dt);

    // LSQ�������ӿ�, ��ȡĳһ��ļ��ٶ�ƽ��ֵ
    bool get_sample(uint8_t i, matrix::Vector3f& s) const;
    // ��ȡ����������� ĳһ��ļ��ٶ�ƽ��ֵ
    bool get_sample_corrected(uint8_t i, matrix::Vector3f& s) const;

    // set tolerance bar for parameter fitness value to cross so as to be deemed as correct values
    void set_tolerance(float tolerance) { _conf_tolerance = tolerance; }

    // У׼״̬�ӿ�
    enum accel_cal_status_t get_status() const { return _status; }
	// ���ز��������
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
    uint8_t _conf_num_samples;					//���ò�������--Ĭ��6��
    float _conf_sample_time;					//���ò���ʱ��--0.5s
    enum accel_cal_fit_type_t _conf_fit_type;	//����У׼����--Ĭ������
    float _conf_tolerance;						//���ù���

    // state
    accel_cal_status_t _status;					//У׼״̬
    struct AccelSample* _sample_buffer;			//����������, Ĭ�ϳ���Ϊ6
    uint8_t _samples_collected;					//�Ѳɸ���, ����Ϊ��������������
    union param_u _param;							//У׼����
    float _fitness;
    uint32_t _last_samp_frag_collected_ms;		//�ϴβ���ʱ��
    float _min_sample_dist;

    bool accept_sample(const matrix::Vector3f& sample);	//������ƽ��ֵ�Ľ�����
    void reset_state();							//��������

    void set_status(enum accel_cal_status_t);	//�����к���

    bool accept_result() const;					//���У׼����Ƿ��ڿ��Ͽɷ�Χ��

    uint8_t get_num_params() const;				//������ѡ�����������Ĳ�����

    // ���˹-ţ����С���˻ع�����йصĺ���
    
    //�����������ʹ���ṩ�Ĳ���У�����������Ĳв�
    float calc_residual(const matrix::Vector3f& sample, const struct param_t& params) const;
    
    //ʹ�ò����������вɼ��������ܾ�����Ӧ��
    float calc_mean_squared_residuals(const struct param_t& params) const;
    
    //�����ſɱȾ��󣬸þ���������Ӧ����ÿ�������仯�Ĺ�ϵ
    //����LSq�����������ڵ��������ı仯���Ա�����LSq����һ�ε���
    void calc_jacob(const matrix::Vector3f& sample, const struct param_t& params, VectorP& ret) const;
    
    //�������ռ������и�˹-ţ������㷨�����ó�ƫ�ơ��Խ���/�������Ӻʹ���/�ǶԽ��߲���
    void run_fit(uint8_t max_iterations, float& fitness);
};

}
