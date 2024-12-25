/**
 * low level driver for stm32f1 series, base on std periph library
 * module timer
*/

#include "lld_timer.h"

lld_timer_t *mcu_timer_list[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static void lld_timer_io_init(lld_timer_t *obj, GPIOMode_TypeDef mode, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4)
{
    if (obj->timnum <= 4 && remap) {
        //                 				   TIM1  	TIM2     TIM3  	 TIM4
        GPIO_TypeDef *port_ch1[4]       = {GPIOE,	GPIOA,	GPIOA,	GPIOB };
        uint16_t      port_ch1_pin[4]   = {  9,  		0,      6,  	6 }; 
        GPIO_TypeDef* port_ch2[4]       = {GPIOE,	GPIOA,	GPIOA,	GPIOB };
        uint16_t      port_ch2_pin[4]   = {  11,  		1, 		7,  	7 }; 
        GPIO_TypeDef* port_ch3[4]       = {GPIOE,	GPIOA,	GPIOB,	GPIOB };
        uint16_t      port_ch3_pin[4]   = {  13,  	    2,      0,  	8 }; 
        GPIO_TypeDef* port_ch4[4]       = {GPIOE,	GPIOA,	GPIOB,	GPIOB };
        uint16_t      port_ch4_pin[4]   = {  14,  	    3,      1,  	9 };
        
        if(ench1) lld_gpio_init(&obj->ch1, port_ch1[obj->timnum - 1], port_ch1_pin[obj->timnum - 1], mode,
            GPIO_Speed_50MHz, true);
        if(ench2) lld_gpio_init(&obj->ch2, port_ch2[obj->timnum - 1], port_ch2_pin[obj->timnum - 1], mode,
            GPIO_Speed_50MHz, true);
        if(ench3) lld_gpio_init(&obj->ch3, port_ch3[obj->timnum - 1], port_ch3_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);   
        if(ench4) lld_gpio_init(&obj->ch4, port_ch4[obj->timnum - 1], port_ch4_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);
        
		uint32_t remaptim[4] = {GPIO_FullRemap_TIM1, GPIO_FullRemap_TIM2, GPIO_FullRemap_TIM3, GPIO_Remap_TIM4};
		GPIO_PinRemapConfig(remaptim[obj->timnum - 1], ENABLE);
		if (obj->timnum == 2)
			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    }else{
        GPIO_TypeDef *port_ch1[8]       = {GPIOA,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch1_pin[8]   = {  8,  		0,  	6,  	6,		0,	0,	0,	6	 }; 
        GPIO_TypeDef *port_ch2[8]       = {GPIOA,	GPIOA,	GPIOA,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch2_pin[8]   = {  9,  		1, 		7,  	7,		1,	0,	0,	7	 }; 
        GPIO_TypeDef *port_ch3[8]       = {GPIOA,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch3_pin[8]   = {  10,  	    2,  	0,  	8,		2,  0,	0,	8	 }; 
        GPIO_TypeDef *port_ch4[8]       = {GPIOA,	GPIOA,	GPIOB,	GPIOB,	GPIOA,	0,	0,	GPIOC};
        uint16_t      port_ch4_pin[8]   = {  11,  	    3, 	    1,  	9,		3,	0,	0,	9	 };	
        
        if(ench1) lld_gpio_init(&obj->ch1, port_ch1[obj->timnum - 1], port_ch1_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);
        if(ench2) lld_gpio_init(&obj->ch2, port_ch2[obj->timnum - 1], port_ch2_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);
        if(ench3) lld_gpio_init(&obj->ch3, port_ch3[obj->timnum - 1], port_ch3_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);    
        if(ench4) lld_gpio_init(&obj->ch4, port_ch4[obj->timnum - 1], port_ch4_pin[obj->timnum - 1], mode, 
            GPIO_Speed_50MHz, true);	
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
	double count = (double)ts * (double)rcc_clocks.SYSCLK_Frequency;
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
	// TIM_Cmd(obj->_tim,ENABLE);  
}

static void lld_timer_init_ic(lld_timer_t *obj, uint16_t icedge, bool ench1, bool ench2, bool ench3, bool ench4)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_ICPolarity = icedge;   	
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;     
	TIM_ICInitStructure.TIM_ICFilter = 0; 
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInit(obj->_tim, &TIM_ICInitStructure);
	TIM_ITConfig(obj->_tim, TIM_IT_CC1, ench1 ? ENABLE : DISABLE);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(obj->_tim, &TIM_ICInitStructure);
	TIM_ITConfig(obj->_tim, TIM_IT_CC2, ench2 ? ENABLE : DISABLE);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInit(obj->_tim, &TIM_ICInitStructure);
	TIM_ITConfig(obj->_tim, TIM_IT_CC3, ench3 ? ENABLE : DISABLE);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInit(obj->_tim, &TIM_ICInitStructure);
	TIM_ITConfig(obj->_tim, TIM_IT_CC4, ench4 ? ENABLE : DISABLE);
}

void lld_timer_init_tim(lld_timer_t *obj, uint8_t timnum, float ts, void (*updateirq)())
{
    obj->timnum = timnum;
    if (timnum > 8) return;
    mcu_timer_list[timnum - 1] = obj;
    obj->mode = TIM_MODE_TIMER;
    obj->updateirq = updateirq;
    lld_timer_base_init(obj, ts, TIM_CounterMode_Down, false);
    
    IRQn_Type irq[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_IRQn, TIM7_IRQn, (IRQn_Type)46};
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irq[timnum - 1];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	TIM_ClearFlag(obj->_tim, TIM_FLAG_Update);
	TIM_ITConfig(obj->_tim, TIM_IT_Update, ENABLE); 
}

void lld_timer_init_cnter(lld_timer_t *obj, uint8_t timnum, uint16_t exttriggerpsc, bool remap)
{
    obj->timnum = timnum;
    if (timnum == 5 || timnum == 6 || timnum == 7 || timnum > 8) return;
    mcu_timer_list[timnum - 1] = obj;  
    obj->mode = TIM_MODE_COUNTER;
    
	if (!remap) {
		GPIO_TypeDef *_port[8] = {GPIOA, GPIOA,	GPIOD, GPIOE, 0, 0, 0, GPIOA};
		uint16_t      _pin[8]  = { 12,    0,    2,     0,     0, 0, 0,   0  };
        lld_gpio_init(&obj->encoder, _port[obj->timnum - 1], _pin[obj->timnum - 1], GPIO_Mode_IN_FLOATING, 
            GPIO_Speed_10MHz, false);
        
	} else if(timnum == 1 || timnum == 2) {
		GPIO_TypeDef *_port[2] = {GPIOE,GPIOA};
		uint16_t      _pin[2]  = {   7,  15  };
        lld_gpio_init(&obj->encoder, _port[obj->timnum - 1], _pin[obj->timnum - 1], GPIO_Mode_IN_FLOATING, 
            GPIO_Speed_10MHz, false);
		uint32_t      remaptim[2] = {GPIO_FullRemap_TIM1, GPIO_FullRemap_TIM2};
		GPIO_PinRemapConfig(remaptim[obj->timnum - 1], ENABLE);
		if (obj->timnum == 2)
			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	} else {
        return;
    }

	lld_timer_base_init(obj, 1, TIM_CounterMode_Up, true);
	TIM_ETRClockMode2Config(obj->_tim, exttriggerpsc, TIM_ExtTRGPolarity_NonInverted, 5);
	TIM_SetCounter(obj->_tim, 0);
	
    IRQn_Type irq[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_IRQn, TIM7_IRQn, (IRQn_Type)44};
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irq[timnum - 1];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	TIM_ClearITPendingBit(obj->_tim, TIM_IT_Update);
	TIM_ITConfig(obj->_tim, TIM_IT_Update, ENABLE);
}

void lld_timer_init_encoder(lld_timer_t *obj, uint8_t timnum, bool remap)
{
    obj->timnum = timnum;
    if (timnum > 8 || timnum == 6 || timnum == 7) return;
    mcu_timer_list[timnum - 1] = obj;
    obj->mode = TIM_MODE_ENCODER;
    
    lld_timer_io_init(obj, GPIO_Mode_IN_FLOATING, remap, true, true, false, false);
    lld_timer_base_init(obj, 1, TIM_CounterMode_Up, true);
    IRQn_Type irq[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_IRQn, TIM7_IRQn, (IRQn_Type)44};
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irq[timnum - 1];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
	TIM_EncoderInterfaceConfig(obj->_tim, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 6;
	TIM_ICInit(obj->_tim, &TIM_ICInitStructure);
	TIM_ClearFlag(obj->_tim, TIM_FLAG_Update);
	TIM_ITConfig(obj->_tim, TIM_IT_Update,ENABLE);
	TIM_SetCounter(obj->_tim, 0);
	obj->encoderzero = 0;
}

void lld_timer_init_fvs(lld_timer_t *obj, uint8_t timnum, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4)
{
    obj->timnum = timnum;
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
    mcu_timer_list[timnum - 1] = obj;
    obj->mode = TIM_MODE_FRQ;
	lld_timer_io_init(obj, GPIO_Mode_AF_PP, remap, ench1, ench2, ench3, ench4);
    lld_timer_base_init(obj, 50, TIM_CounterMode_Down, false);
    lld_timer_init_oc(obj, false, ench1, ench2, ench3, ench4);
	TIM_ARRPreloadConfig(obj->_tim, DISABLE);
}
    
void lld_timer_init_pwm(lld_timer_t *obj, uint8_t timnum, float frqHz, bool isPWM, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4)
{
    obj->timnum = timnum;
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
    mcu_timer_list[timnum - 1] = obj;
    
	if (isPWM) {
		obj->mode = TIM_MODE_PWM;
		lld_timer_io_init(obj, GPIO_Mode_AF_PP, remap, ench1, ench2, ench3, ench4);
		lld_timer_base_init(obj, 1.0f / frqHz, TIM_CounterMode_Down, false);
		lld_timer_init_oc(obj, true, ench1, ench2, ench3, ench4);
	} else {
		obj->mode = TIM_MODE_IPC;
		lld_timer_io_init(obj, GPIO_Mode_IN_FLOATING, remap, ench1, ench2, ench3, ench4);
		lld_timer_base_init(obj, 1.0f / frqHz, TIM_CounterMode_Up, false);
        
		IRQn_Type irq[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, (IRQn_Type)30, (IRQn_Type)50, (IRQn_Type)54, (IRQn_Type)55, (IRQn_Type)46};
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = irq[timnum - 1];
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        lld_timer_init_ic(obj, TIM_ICPolarity_Rising, ench1, ench2, ench3, ench4);
        //time for one tick(or count)(unit: ms)
        obj->timattr.resolution = ((double)((uint16_t)(72000000.0/(65535.0 * frqHz)) + 1.0) / (double)72000.0);
	}
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
    count = (rcc_clocks.SYSCLK_Frequency / 2) / hz;

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

void lld_timer_enable(lld_timer_t *obj, bool enable)
{
	if(enable) TIM_Cmd(obj->_tim, ENABLE);
	else TIM_Cmd(obj->_tim, DISABLE);
}

double lld_timer_inputcapturevalue(lld_timer_t *obj, uint8_t chnum)
{
    return obj->ipc_cnt[chnum - 1] * obj->timattr.resolution;
}

int64_t lld_timer_cnts(lld_timer_t *obj)
{
    return obj->encoderpulse + (uint32_t)(obj->_tim->CNT);
}

void lld_timer_irq_inputcapture(lld_timer_t *obj)
{
	static bool isRising[4] = {true, true, true,true}; //Rising or falling interrupt flag, inverse each time
	static uint16_t cntRising[4] = {0}; //for saving capture count
	static uint16_t ccr;
    
	if (TIM_GetITStatus(obj->_tim, TIM_IT_CC1) != RESET) {
		ccr = TIM_GetCapture1(obj->_tim);
		if(isRising[0]) {
			cntRising[0] = ccr;
			TIM_OC1PolarityConfig(obj->_tim, TIM_ICPolarity_Falling);
		} else {
			if(ccr >= cntRising[0])	{
                obj->ipc_cnt[0] = ccr - cntRising[0];	
            } else {
                obj->ipc_cnt[0] = obj->timattr.period - cntRising[0] + ccr;
            }
			TIM_OC1PolarityConfig(obj->_tim, TIM_ICPolarity_Rising);
		}
		isRising[0] = !isRising[0];
	}
	
	if (TIM_GetITStatus(obj->_tim, TIM_IT_CC2) != RESET) {
		ccr = TIM_GetCapture2(obj->_tim);
		if (isRising[1]) {
			cntRising[1] = ccr;
			TIM_OC2PolarityConfig(obj->_tim, TIM_ICPolarity_Falling);
		} else {
			if(ccr >= cntRising[1]) {
                obj->ipc_cnt[1] = ccr - cntRising[1];
            } else {
                obj->ipc_cnt[1] = obj->timattr.period - cntRising[1] + ccr;
            }
			TIM_OC2PolarityConfig(obj->_tim, TIM_ICPolarity_Rising);
		}
		isRising[1] = !isRising[1];
	}

	if (TIM_GetITStatus(obj->_tim, TIM_IT_CC3) != RESET) {
		ccr = TIM_GetCapture3(obj->_tim);
		if (isRising[2]) {
			cntRising[2] = ccr;
			TIM_OC3PolarityConfig(obj->_tim, TIM_ICPolarity_Falling);
		} else {
			if(ccr >= cntRising[2]) {
                obj->ipc_cnt[2] = ccr - cntRising[2];
            } else {
                obj->ipc_cnt[2] = obj->timattr.period - cntRising[2] + ccr;
            }
			TIM_OC3PolarityConfig(obj->_tim, TIM_ICPolarity_Rising);
		}
		isRising[2] = !isRising[2];
	}

	if (TIM_GetITStatus(obj->_tim, TIM_IT_CC4) != RESET) {
		ccr = TIM_GetCapture4(obj->_tim);
		if(isRising[3]) {
			cntRising[3] = ccr;
			TIM_OC4PolarityConfig(obj->_tim, TIM_ICPolarity_Falling);
		} else {
			if(ccr >= cntRising[3]) {
                obj->ipc_cnt[3] = ccr - cntRising[3];
            } else {
                obj->ipc_cnt[3] = obj->timattr.period - cntRising[3] + ccr;
            }
			TIM_OC4PolarityConfig(obj->_tim, TIM_ICPolarity_Rising);
		}
		isRising[3] = !isRising[3];
	}
}

void lld_timer_irq_encoder(lld_timer_t *obj)
{
	if (TIM_GetITStatus(obj->_tim, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_Update);
		uint16_t cnt = TIM_GetCounter(obj->_tim);
		if (cnt < 30000) {
            obj->encoderpulse += 60000;
        } else {
            obj->encoderpulse -= 60000;
        }
	} 
}

void lld_timer_irq(lld_timer_t *obj)
{
	switch (obj->mode) {
	case TIM_MODE_TIMER:
		if(TIM_GetITStatus(obj->_tim, TIM_IT_Update) != RESET) {
			TIM_ClearITPendingBit(obj->_tim, TIM_IT_Update);
			if (obj->updateirq == 0) return;
			obj->updateirq();
		}
		break;
	case TIM_MODE_IPC:
		lld_timer_irq_inputcapture(obj);
		break;
	case TIM_MODE_COUNTER:
	case TIM_MODE_ENCODER:
		lld_timer_irq_encoder(obj);
		break;
	case TIM_MODE_PWM:
	default:
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC1);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC2);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC3);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_CC4);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_Trigger);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_Break);
		TIM_ClearITPendingBit(obj->_tim, TIM_IT_Update);
		break;
	}
}


