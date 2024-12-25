/**
 * low level driver for stm32f4 series, base on std periph library
 * module adc
*/

#include "lld_adc.h"

static uint16_t adc_value[19] = {0};
static uint8_t  adc_channel_list[19] = {0};
static uint8_t  adc_channel_cnt = 0;

void lld_adc_init(lld_adc_t *obj, uint8_t ch_num, float k)
{
    if (ch_num >= 19) return;
    obj->factor = k;
    obj->channel_num = ch_num;
    obj->order_num = adc_channel_cnt;
    adc_channel_list[obj->order_num] = ch_num;
    adc_channel_cnt++;

	if (ch_num <= 15) {
		//                         AIN0   AIN1  AIN2  AIN3  AIN4  AIN5  AIN6  AIN7  AIN8  AIN9 AIN10 AIN11 AIN12 AIN13 AIN14 AIN15 
		GPIO_TypeDef *port[16] = {GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOB,GPIOB,GPIOC,GPIOC,GPIOC,GPIOC,GPIOC,GPIOC};
		uint8_t		  pin[16]  = {0,    1,     2,    3,    4,    5,    6,    7,    0,    1,    0,    1,    2,    3,    4,    5};
        lld_gpio_init(&obj->pin, port[ch_num], pin[ch_num], GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, 0);
	}
    
	RCC_AHB2PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA2_Stream0);
	DMA_InitStructure.DMA_Channel =	DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR));				
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(&adc_value[0]);	                   
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;												
	DMA_InitStructure.DMA_BufferSize = adc_channel_cnt;                             
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	         
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  			    				
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;       
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		                        						
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                       
	DMA_InitStructure.DMA_FIFOMode	 = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold 		 = DMA_FIFOThreshold_3QuartersFull;
	DMA_InitStructure.DMA_MemoryBurst    		 = DMA_MemoryBurst_Single;								
	DMA_InitStructure.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;                                
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);            									
	DMA_Cmd(DMA2_Stream0, ENABLE); 

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult_InjecSimult;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;				
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = adc_channel_cnt;
	ADC_Init(ADC1,&ADC_InitStructure);		
	
	for (int i = 0; i < adc_channel_cnt; i++) {
		ADC_RegularChannelConfig(ADC1, adc_channel_list[i], i + 1, ADC_SampleTime_480Cycles);
		if (adc_channel_list[i] == 16 || adc_channel_list[i] == 18)
			ADC_TempSensorVrefintCmd(ENABLE);
	}
                   
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1);
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
}

float lld_adc_value(lld_adc_t *obj)
{
	//voltage = Raw data * resolusion * coefficient
	float value = adc_value[obj->order_num] * 0.0008056640625 * obj->factor;
	//resolusion = 3.3v / 2^12 = 3.3 / 4096 = 0.0008056640625
	if (obj->channel_num == 16 || obj->channel_num == 18) {
        //Internal Temperature channel
		float temperature = (value - 0.76f) / 0.0025f + 25.0f;
		return temperature;
	} else {
		return value;
    }
}
