/**
 * low level driver for stm32f1 series, base on std periph library
 * module dac
*/

#include "lld_dac.h"

void lld_dac_init(lld_dac_t *obj, uint8_t num)
{
    if (num != 1 && num != 2) return;
    obj->num = num;
    
	if (num == 1) {
        lld_gpio_init(&obj->pin, GPIOA, 4, GPIO_Mode_AIN, GPIO_Speed_50MHz, true);
        lld_gpio_set(&obj->pin, 1);
	}else if (num == 2) {
        lld_gpio_init(&obj->pin, GPIOA, 6, GPIO_Mode_AIN, GPIO_Speed_50MHz, true);
        lld_gpio_set(&obj->pin, 1);
	} 
    
    DAC_InitTypeDef DAC_InitType;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	DAC_InitType.DAC_Trigger = DAC_Trigger_None;												
	DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None;				
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    
    uint32_t dac_channel[2] = {DAC_Channel_1, DAC_Channel_2};
    obj->channel = dac_channel[num - 1];
    
    DAC_Init(obj->channel, &DAC_InitType);
    DAC_Cmd(obj->channel, ENABLE);
    DAC_SoftwareTriggerCmd(obj->channel, ENABLE);
    
	if (num == 1) {
        DAC_SetChannel1Data(DAC_Align_12b_R, 0);
	}else if (num == 2) {
        DAC_SetChannel2Data(DAC_Align_12b_R, 0);
	}
}

void lld_dac_set_voltage(lld_dac_t *obj, float value)
{
	if (value < 0.0f || value > 3.3f) return;
	
	float vol = value * 4096 / 3.3f;	
	if (obj->num == 1) {
		DAC_SetChannel1Data(DAC_Align_12b_R, vol);			
		DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE); 
	} else if (obj->num == 2) {
		DAC_SetChannel2Data(DAC_Align_12b_R, vol);			
		DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE); 
	}
}
