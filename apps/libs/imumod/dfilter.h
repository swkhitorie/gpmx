#ifndef DFILTER_H_
#define DFILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdint.h>

struct lpf {
	float cutoff_Freq;
	float sample_Freq;
	float old_data;
	float dt;
};

struct lpf2ndbutterworth {
	float sample_Freq;
	float cutoff_Freq;
	float _a11;
	float _a21;
	float _b01;
	float _b11;
	float _b21;
	float _delay_element_11;
	float _delay_element_21;
};

struct movmeanfilter {
    float *buf;
    uint16_t size;
    uint16_t idx;
};

void lpf_setcutoff_freq(struct lpf *obj, float _sample_freq, float _cutoff_freq);
float lpf_apply(struct lpf *obj, float sample);

void lpf2_setcutoff_freq(struct lpf2ndbutterworth *obj, 
                float _sample_freq, float _cutoff_freq);
float lpf2_apply(struct lpf2ndbutterworth *obj, float sample);

void movmeanfilter_init(struct movmeanfilter *obj, float *space, uint16_t size);
float movmeanfilter_apply(struct movmeanfilter *obj, float sample);
float movmeanfilter_apply_antipulse(struct movmeanfilter *obj, float sample);

#ifdef __cplusplus
}
#endif

#endif
