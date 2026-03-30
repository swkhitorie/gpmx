#include "dfilter.h"

#ifndef PI_F
    #define PI_F 3.1415926f
#endif

void lpf_setcutoff_freq(struct lpf *obj, float _sample_freq, float _cutoff_freq)
{
    obj->sample_Freq = _sample_freq;
    obj->cutoff_Freq = _cutoff_freq;
    obj->dt = 1 / obj->sample_Freq;
}

float lpf_apply(struct lpf *obj, float sample)
{
    float result;
    float factor = ((1.0f / obj->sample_Freq) * 2 * PI_F * obj->cutoff_Freq);
    result = factor * sample + (1 - factor) * obj->old_data;
    obj->old_data = result;
    return result;
}

void lpf2_setcutoff_freq(struct lpf2ndbutterworth *obj, 
                float _sample_freq, float _cutoff_freq)
{
		float fr = 0;
		float ohm = 0;
		float c = 0;

		obj->sample_Freq = _sample_freq;
		obj->cutoff_Freq = _cutoff_freq;

		fr = obj->sample_Freq / obj->cutoff_Freq;
		ohm = tanf(PI_F / fr);
		c = 1.0f + 2.0f * cosf(PI_F / 4.0f) * ohm + ohm * ohm;

		if (obj->cutoff_Freq > 0.0f) {
			obj->_b01 = ohm * ohm / c;
			obj->_b11 = 2.0f * obj->_b01;
			obj->_b21 = obj->_b01;
			obj->_a11 = 2.0f * (ohm * ohm - 1.0f) / c;
			obj->_a21 = (1.0f - 2.0f * cosf(PI_F / 4.0f) * ohm + ohm * ohm) / c;
		}
}

void movmeanfilter_init(struct movmeanfilter *obj, float *space, uint16_t size)
{
    if (!space || size < 5) return;
    obj->buf = space;
    obj->size = size;
    obj->idx = 0;
}

float movmeanfilter_apply(struct movmeanfilter *obj, float sample)
{
    float sum = 0.0f;
    obj->buf[obj->idx++] = sample;
    if (obj->idx == obj->size)
        obj->idx = 0;

    for (int i = 0; i < obj->size; i++) {
        sum += obj->buf[i]
    }
    return (sum / obj->size);
}

float movmeanfilter_apply_antipulse(struct movmeanfilter *obj, float sample)
{
    float sum = 0.0f;
    float max = 0.0f;
    float min = 0.0f;
    min = max = sample;

    obj->buf[obj->idx++] = sample;
    if (obj->idx == obj->size)
        obj->idx = 0;

    for (int i = 0; i < obj->size; i++) {
        sum += obj->buf[i];
        if (obj->buf[i] > max)
            max = obj->buf[i];
        else if (obj->buf[i] < min)
            min = obj->buf[i];
    }
    return (sum - max - min) / (obj->size - 2);
}
