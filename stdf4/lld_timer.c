/**
 * low level driver for stm32f4 series, base on std periph library
 * module timer
*/

#include "lld_timer.h"


lld_timer_t *mcu_timer_list[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static void lld_timer_io_init(lld_timer_t *obj, GPIOMode_TypeDef mode, GPIOOType_TypeDef otype, 
    bool remap, bool ench1, bool ench2, bool ench3, bool ench4)
{
    uint8_t tim_af[8] = {GPIO_AF_TIM1, GPIO_AF_TIM2, GPIO_AF_TIM3, GPIO_AF_TIM4, GPIO_AF_TIM5, 0, 0, GPIO_AF_TIM8};
    
    if (obj->timnum == 1 && remap) {
        //                 				   TIM1  	TIM2     TIM3  	 TIM4   TIM5             TIM8
        GPIO_TypeDef *port_ch1[8]       = {GPIOE,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch1_pin[8]   = {  9,  		0,      6,  	6,	0,		0,	0,	6    }; 
        GPIO_TypeDef* port_ch2[8]       = {GPIOE,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch2_pin[8]   = {  11,  		1, 		7,  	7,	1,		0,	0,	7	 }; 
        GPIO_TypeDef* port_ch3[8]       = {GPIOE,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch3_pin[8]   = {  13,  	    2,      0,  	8,  2,		0,	0,	8	 }; 
        GPIO_TypeDef* port_ch4[8]       = {GPIOE,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch4_pin[8]   = {  14,  	    3,      1,  	9,	3,		0,	0,	9	 };
        
        if(ench1) lld_gpio_init(&obj->ch1, port_ch1[obj->timnum - 1], port_ch1_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);
        if(ench2) lld_gpio_init(&obj->ch2, port_ch2[obj->timnum - 1], port_ch2_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]); 
        if(ench3) lld_gpio_init(&obj->ch3, port_ch3[obj->timnum - 1], port_ch3_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);       
        if(ench4) lld_gpio_init(&obj->ch4, port_ch4[obj->timnum - 1], port_ch4_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);
    }else{
        GPIO_TypeDef *port_ch1[8]       = {GPIOA,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch1_pin[8]   = {  8,  		0,  	6,  	6,		0,	0,	0,	6	 }; 
        GPIO_TypeDef *port_ch2[8]       = {GPIOA,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch2_pin[8]   = {  9,  		1, 		7,  	7,		1,	0,	0,	7	 }; 
        GPIO_TypeDef *port_ch3[8]       = {GPIOA,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch3_pin[8]   = {  10,  	    2,  	0,  	8,		2,  0,	0,	8	 }; 
        GPIO_TypeDef *port_ch4[8]       = {GPIOA,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch4_pin[8]   = {  11,  	    3, 	    1,  	9,		3,	0,	0,	9	 };	
        
        if(ench1) lld_gpio_init(&obj->ch1, port_ch1[obj->timnum - 1], port_ch1_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);
        if(ench2) lld_gpio_init(&obj->ch2, port_ch2[obj->timnum - 1], port_ch2_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]); 
        if(ench3) lld_gpio_init(&obj->ch3, port_ch3[obj->timnum - 1], port_ch3_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);       
        if(ench4) lld_gpio_init(&obj->ch4, port_ch4[obj->timnum - 1], port_ch4_pin[obj->timnum - 1], mode, otype, 
            GPIO_PuPd_NOPULL, GPIO_High_Speed, tim_af[obj->timnum - 1]);			
    }	  
}

static void lld_timer_base_init(lld_timer_t *obj, float ts, uint16_t cntway, bool encoder)
{
	uint32_t rcc[8] = { RCC_APB2Periph_TIM1, RCC_APB1Periph_TIM2, RCC_APB1Periph_TIM3,
                        RCC_APB1Periph_TIM4, RCC_APB1Periph_TIM5, RCC_APB1Periph_TIM6,
                        RCC_APB1Periph_TIM7, RCC_APB2Periph_TIM8};
    TIM_TypeDef *timx[8] = {TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8};
    RCC_ClocksTypeDef rcc_clocks;
    TIM_TimeBaseInitTypeDef TIM_BashInitStructure;
    RCC_GetClocksFreq(&rcc_clocks);
                        
	if (obj->timnum == 1 || obj->timnum == 8) {
        RCC_APB2PeriphClockCmd(rcc[obj->timnum - 1], ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(rcc[obj->timnum - 1], ENABLE);
    }
	
	//Calculate Period
	uint16_t prescaler;
	double count = ((double)ts * (double)rcc_clocks.SYSCLK_Frequency) / 2.0f;
	prescaler = (uint16_t)(count / 65536.0);
	obj->timattr.period = (uint16_t)(count / (prescaler + 1)) - 1;

	if (encoder) {
		obj->timattr.period = 60000;
		prescaler = 0;
	}

    TIM_BashInitStructure.TIM_Period = obj->timattr.period;
    TIM_BashInitStructure.TIM_Prescaler = prescaler;
    TIM_BashInitStructure.TIM_ClockDivision = 0;
    TIM_BashInitStructure.TIM_RepetitionCounter = 0;
    TIM_BashInitStructure.TIM_CounterMode = cntway;
    obj->_tim = timx[obj->timnum - 1];
    TIM_TimeBaseInit(obj->_tim, &TIM_BashInitStructure);
    TIM_Cmd(obj->_tim, ENABLE);
}

static void lld_timer_init_oc(lld_timer_t *obj, bool ispwm, bool ench1, bool ench2, bool ench3, bool ench4)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
    
	TIM_Cmd(obj->_tim, DISABLE);
    TIM_OCInitStructure.TIM_OCMode = (ispwm ? TIM_OCMode_PWM1 : TIM_OCMode_Toggle);
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	if(ench1){
		TIM_OC1Init(obj->_tim, &TIM_OCInitStructure);
		TIM_OC1PreloadConfig(obj->_tim, TIM_OCPreload_Enable);
	}
	if(ench2){
		TIM_OC2Init(obj->_tim, &TIM_OCInitStructure);
		TIM_OC2PreloadConfig(obj->_tim, TIM_OCPreload_Enable);
	}
	if(ench3){
		TIM_OC3Init(obj->_tim, &TIM_OCInitStructure);
		TIM_OC3PreloadConfig(obj->_tim, TIM_OCPreload_Enable);
	}
	if(ench4){
		TIM_OC4Init(obj->_tim, &TIM_OCInitStructure);
		TIM_OC4PreloadConfig(obj->_tim, TIM_OCPreload_Enable);
	}
	if (obj->_tim == TIM1 || obj->_tim == TIM8) {
		TIM_CtrlPWMOutputs(obj->_tim, ENABLE);
    }
	TIM_Cmd(obj->_tim,ENABLE);  
}

void lld_timer_init_pwm(lld_timer_t *obj, uint8_t timnum, float frqHz, bool isPWM, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4)
{
    obj->timnum = timnum;
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
    mcu_timer_list[timnum - 1] = obj;
    
	if (isPWM) {
		obj->mode = TIM_MODE_PWM;
		lld_timer_io_init(obj, GPIO_Mode_AF, GPIO_OType_PP, remap, ench1, ench2, ench3, ench4);
		lld_timer_base_init(obj, 1.0f / frqHz, TIM_CounterMode_Down, false);
		lld_timer_init_oc(obj, true, ench1, ench2, ench3, ench4);
	} else {
		obj->mode = TIM_MODE_IPC;
		lld_timer_io_init(obj, GPIO_Mode_IN, GPIO_OType_PP, remap, ench1, ench2, ench3, ench4);
		lld_timer_base_init(obj, 1.0f / frqHz, TIM_CounterMode_Up, false);
        
		IRQn_Type irq[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, (IRQn_Type)30, (IRQn_Type)50, (IRQn_Type)54, (IRQn_Type)55, (IRQn_Type)46};
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = irq[timnum - 1];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
		//InitIC(TIM_ICPolarity_Rising, enCh1, enCh2, enCh3, enCh4);
        ////time for one tick(or count)(unit: ms)
		//_Resolution = ((double)(u16(168000000.0/(65535.0*frqHz)) + 1.0) / (double)168000.0);
	}  
}

void lld_timer_init_fvs(lld_timer_t *obj, uint8_t timnum, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4)
{
    obj->timnum = timnum;
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
    mcu_timer_list[timnum - 1] = obj;
    obj->mode = TIM_MODE_FRQ;
	lld_timer_io_init(obj, GPIO_Mode_AF, GPIO_OType_PP, remap, ench1, ench2, ench3, ench4);
    lld_timer_base_init(obj, 50, TIM_CounterMode_Down, false);
    lld_timer_init_oc(obj, false, ench1, ench2, ench3, ench4);
	TIM_ARRPreloadConfig(obj->_tim, DISABLE);
}

void lld_timer_sethz(lld_timer_t *obj, float hz)
{
    RCC_ClocksTypeDef rcc_clocks;
    float count;
    
    RCC_GetClocksFreq(&rcc_clocks);
    if (hz < 1.0f) {
        hz = 1.0f;
    } else if (hz > 300000) {
        hz = 300000;
    }
    count = rcc_clocks.SYSCLK_Frequency / hz;

	uint16_t prescaler = (uint16_t)(count / 65536.0f);
	uint16_t period = (uint16_t)(count / (prescaler + 1)) - 1;
	
    obj->_tim->PSC = prescaler;
    obj->_tim->ARR = period;
    obj->timattr.period = period;
}

void lld_timer_setduty(lld_timer_t *obj, float ch1,float ch2, float ch3, float ch4)
{
	obj->_tim->CCR1 = obj->timattr.period*ch1 / 100.0f;
	obj->_tim->CCR2 = obj->timattr.period*ch2 / 100.0f;
	obj->_tim->CCR3 = obj->timattr.period*ch3 / 100.0f;
	obj->_tim->CCR4 = obj->timattr.period*ch4 / 100.0f;
}

void lld_timer_irq(lld_timer_t *obj)
{
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC1);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC2);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC3);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC4);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_Trigger);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_Break);
    TIM_ClearITPendingBit(obj->_tim, TIM_IT_Update);
}

