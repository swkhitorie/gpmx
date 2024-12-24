/**
 * low level driver for stm32h7 series, base on cubehal library
 * module timer
*/

#include "include/lld_timer.h"

lld_timer_t *mcu_timer_list[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void lld_timer_baseinit(lld_timer_t *obj, uint8_t timNum, double ts)
{
	TIM_TypeDef *TIMx[8] = {TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8};
    obj->htim.Instance = TIMx[timNum - 1];
	switch (timNum) {
	case 1: __HAL_RCC_TIM1_CLK_ENABLE();	break;
	case 2: __HAL_RCC_TIM2_CLK_ENABLE();	break;
	case 3: __HAL_RCC_TIM3_CLK_ENABLE();	break;
	case 4: __HAL_RCC_TIM4_CLK_ENABLE();	break;
	case 5: __HAL_RCC_TIM5_CLK_ENABLE();	break;
	case 6: __HAL_RCC_TIM6_CLK_ENABLE();	break;
	case 7: __HAL_RCC_TIM7_CLK_ENABLE();	break;
	case 8: __HAL_RCC_TIM8_CLK_ENABLE();	break;
	}
	
	uint16_t prescaler;
	if (obj->htim.Instance == TIM6 || obj->htim.Instance == TIM7) {
		prescaler = 2000 - 1;
		obj->timattr.period = (uint16_t)(65535 - 1);
		obj->timattr.resolution = 0.0100000000000;
	} else {
		/* Maxium Ticks on 200MHz clock */
		double cnt = ts * 200000000.0;
		/* Maxium Ticks / 2^16 = prescaler, make full use of CNT's 16 bits register */
		prescaler = (uint16_t)(cnt / 65536.0);
		/* period = Maxium Ticks / prescaler, from 0, so - 1 */
		obj->timattr.period = (uint16_t)(cnt / (prescaler + 1)) - 1;
		/* the ms when the timer counter increse once */
		obj->timattr.resolution = 1000.0 * (prescaler + 1) / 200000000.0;
	}

	obj->htim.Instance = TIMx[timNum - 1];
	obj->htim.Init.Prescaler = prescaler;
	obj->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	obj->htim.Init.Period = obj->timattr.period;
	obj->htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	IRQn_Type timUpdateIRQ[8] = {TIM1_UP_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_DAC_IRQn, TIM7_IRQn, TIM8_UP_TIM13_IRQn};
	IRQn_Type timipcIRQ[8] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_DAC_IRQn, TIM7_IRQn, TIM8_CC_IRQn};
	
	switch (obj->mode) {
	case TIM_MODE_PWM:
		{
			HAL_TIM_PWM_Init(&obj->htim); 
			break;		
		}
	case TIM_MODE_TIMER:
		{
			HAL_TIM_Base_Init(&obj->htim);
			__HAL_TIM_CLEAR_IT(&obj->htim, TIM_IT_UPDATE);
			__HAL_TIM_ENABLE_IT(&obj->htim, TIM_IT_UPDATE);
			__HAL_TIM_SET_COUNTER(&obj->htim, 0);
			HAL_TIM_Base_Start_IT(&obj->htim);
			HAL_NVIC_SetPriority(timUpdateIRQ[timNum - 1], 1, 0);
			HAL_NVIC_EnableIRQ(timUpdateIRQ[timNum - 1]);
			break;		
		}
	case TIM_MODE_COUNTER:
		{
			obj->htim.Init.Prescaler = 0;
			obj->htim.Init.Period = 60000;
			HAL_TIM_Base_Init(&obj->htim);

			TIM_ClockConfigTypeDef sClockSoureConfig = {0};
			TIM_MasterConfigTypeDef sMasterConfig = {0};
			sClockSoureConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
			sClockSoureConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
			sClockSoureConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
			sClockSoureConfig.ClockFilter = 0;
			HAL_TIM_ConfigClockSource(&obj->htim,&sClockSoureConfig);
            
			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_TIMEx_MasterConfigSynchronization(&obj->htim, &sMasterConfig);
            
			__HAL_TIM_CLEAR_IT(&obj->htim, TIM_IT_UPDATE);
			__HAL_TIM_ENABLE_IT(&obj->htim, TIM_IT_UPDATE);
			__HAL_TIM_SET_COUNTER(&obj->htim, 0);
			HAL_TIM_Base_Start_IT(&obj->htim);
			HAL_NVIC_SetPriority(timUpdateIRQ[timNum - 1], 1, 0);
			HAL_NVIC_EnableIRQ(timUpdateIRQ[timNum - 1]);
			break;		
		}
	case TIM_MODE_IPC:
		{
			HAL_TIM_IC_Init(&obj->htim);
			HAL_NVIC_SetPriority(timipcIRQ[timNum - 1],1,3);
			HAL_NVIC_EnableIRQ(timipcIRQ[timNum - 1]);
			break;
		}
	case TIM_MODE_ENCODER:
		{
			TIM_Encoder_InitTypeDef sConfig = {0};
			TIM_MasterConfigTypeDef sMasterConfig = {0};
			obj->htim.Init.Prescaler = 0;
			obj->htim.Init.Period = 60000;
			obj->htim.Init.RepetitionCounter = 0;
			obj->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
			
			sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
			sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Filter = 0;
			sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC2Filter = 0;
			HAL_TIM_Encoder_Init(&obj->htim, &sConfig);
			
			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_TIMEx_MasterConfigSynchronization(&obj->htim, &sMasterConfig);
			
			HAL_TIM_Encoder_Start(&obj->htim,TIM_CHANNEL_ALL);
			__HAL_TIM_CLEAR_IT(&obj->htim, TIM_IT_UPDATE);
			HAL_NVIC_SetPriority(timUpdateIRQ[timNum-  1], 1, 0);
			HAL_NVIC_EnableIRQ(timUpdateIRQ[timNum - 1]); 
			__HAL_TIM_ENABLE_IT(&obj->htim, TIM_IT_UPDATE);
			__HAL_TIM_SET_COUNTER(&obj->htim, 0);
			break;
		}
	case TIM_MODE_FRQ:
		{
			obj->htim.Init.CounterMode = TIM_COUNTERMODE_DOWN;
			obj->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
			HAL_TIM_Base_Init(&obj->htim);
			HAL_TIM_Base_Start(&obj->htim);
			break;
		}
	default: break;
	}
}

void lld_timer_opc_init(lld_timer_t *obj, bool isPWM, uint8_t timNum, 
    uint8_t ch1Sx, uint8_t ch2Sx, uint8_t ch3Sx, uint8_t ch4Sx)
{
	TIM_OC_InitTypeDef chx_OC_handler = {0}; 
	
	if (isPWM)  chx_OC_handler.OCMode = TIM_OCMODE_PWM1;
	else        chx_OC_handler.OCMode = TIM_OCMODE_TOGGLE;
	/* set the compare value, to confirm duty, default 50% */
	chx_OC_handler.Pulse = obj->timattr.period / 2;
	chx_OC_handler.OCPolarity = TIM_OCPOLARITY_LOW;
	
	if (ch1Sx > 0 && ch1Sx < 4 && TIMER_GLOBAL_PORT(timNum, 0, ch1Sx) != 0) {
		GPIO_TypeDef *init_port_ch1 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,0,ch1Sx);
        lld_gpio_init(&obj->ch1, init_port_ch1, TIMER_GLOBAL_PINNUM(timNum, 0, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_PWM_ConfigChannel(&obj->htim, &chx_OC_handler, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&obj->htim, TIM_CHANNEL_1);
	}
	if (ch2Sx > 0 && ch2Sx < 4 && TIMER_GLOBAL_PORT(timNum, 1, ch2Sx) != 0) {
		GPIO_TypeDef *init_port_ch2 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,1,ch2Sx);
        lld_gpio_init(&obj->ch2, init_port_ch2, TIMER_GLOBAL_PINNUM(timNum, 1, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_PWM_ConfigChannel(&obj->htim, &chx_OC_handler, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&obj->htim, TIM_CHANNEL_2);
	}	
	if (ch3Sx > 0 && ch3Sx < 4 && TIMER_GLOBAL_PORT(timNum, 2, ch3Sx) != 0) {
		GPIO_TypeDef *init_port_ch3 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,2,ch3Sx);
        lld_gpio_init(&obj->ch3, init_port_ch3, TIMER_GLOBAL_PINNUM(timNum, 2, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_PWM_ConfigChannel(&obj->htim, &chx_OC_handler, TIM_CHANNEL_3);
		HAL_TIM_PWM_Start(&obj->htim, TIM_CHANNEL_3);
	}	
	if (ch4Sx > 0 && ch4Sx < 4 && TIMER_GLOBAL_PORT(timNum, 3, ch4Sx) != 0) {
		GPIO_TypeDef *init_port_ch4 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,3,ch4Sx);
        lld_gpio_init(&obj->ch4, init_port_ch4, TIMER_GLOBAL_PINNUM(timNum, 3, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_PWM_ConfigChannel(&obj->htim, &chx_OC_handler, TIM_CHANNEL_4);
		HAL_TIM_PWM_Start(&obj->htim, TIM_CHANNEL_4);
	}
}

void lld_timer_ipc_init(lld_timer_t *obj, uint8_t timNum, 
    uint8_t ch1Sx, uint8_t ch2Sx, uint8_t ch3Sx, uint8_t ch4Sx)
{
	TIM_IC_InitTypeDef chx_IC_handler = {0};
	/* edge capture mode --- rising    falling    both */
	chx_IC_handler.ICPolarity = TIM_ICPOLARITY_RISING;
	chx_IC_handler.ICSelection = TIM_ICSELECTION_DIRECTTI;
	chx_IC_handler.ICPrescaler = TIM_ICPSC_DIV1;
	chx_IC_handler.ICFilter = 0;

	if (ch1Sx > 0 && ch1Sx < 4 && TIMER_GLOBAL_PORT(timNum, 0, ch1Sx) != 0) {
		GPIO_TypeDef *init_port_ch1 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,0,ch1Sx);
        lld_gpio_init(&obj->ch1, init_port_ch1, TIMER_GLOBAL_PINNUM(timNum, 0, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_IC_ConfigChannel(&obj->htim, &chx_IC_handler, TIM_CHANNEL_1);
		HAL_TIM_IC_Start_IT(&obj->htim, TIM_CHANNEL_1);
	}
	if (ch2Sx > 0 && ch2Sx < 4 && TIMER_GLOBAL_PORT(timNum, 1, ch2Sx) != 0) {
		GPIO_TypeDef *init_port_ch2 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,1,ch2Sx);
        lld_gpio_init(&obj->ch2, init_port_ch2, TIMER_GLOBAL_PINNUM(timNum, 1, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_IC_ConfigChannel(&obj->htim, &chx_IC_handler, TIM_CHANNEL_2);
		HAL_TIM_IC_Start_IT(&obj->htim, TIM_CHANNEL_2);
	}	
	if (ch3Sx > 0 && ch3Sx < 4 && TIMER_GLOBAL_PORT(timNum, 2, ch3Sx) != 0) {
		GPIO_TypeDef *init_port_ch3 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,2,ch3Sx);
        lld_gpio_init(&obj->ch3, init_port_ch3, TIMER_GLOBAL_PINNUM(timNum, 2, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_IC_ConfigChannel(&obj->htim, &chx_IC_handler, TIM_CHANNEL_3);
		HAL_TIM_IC_Start_IT(&obj->htim, TIM_CHANNEL_3);
	}	
	if (ch4Sx > 0 && ch4Sx < 4 && TIMER_GLOBAL_PORT(timNum, 3, ch4Sx) != 0) {
		GPIO_TypeDef *init_port_ch4 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timNum,3,ch4Sx);
        lld_gpio_init(&obj->ch4, init_port_ch4, TIMER_GLOBAL_PINNUM(timNum, 3, ch1Sx), 
			GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timNum));
		HAL_TIM_IC_ConfigChannel(&obj->htim, &chx_IC_handler, TIM_CHANNEL_4);
		HAL_TIM_IC_Start_IT(&obj->htim, TIM_CHANNEL_4);
	}
}

void lld_timer_init_tim(lld_timer_t *obj, uint8_t timnum, float ts, void (*updateirq)())
{
	if (timnum > 8 || timnum < 1) return;
	mcu_timer_list[timnum - 1] = obj;
    obj->timnum = timnum;
    obj->mode = TIM_MODE_TIMER;
    obj->updateirq = updateirq;
    lld_timer_baseinit(obj, timnum, ts);
}

void lld_timer_init_cnter(lld_timer_t *obj, uint8_t timnum, uint8_t select)
{
	if (timnum < 1 || timnum == 6 || timnum == 7 || timnum > 8 || select < 1 || select > 3)  return;
	mcu_timer_list[timnum - 1] = obj;
    obj->timnum = timnum;
    obj->mode = TIM_MODE_COUNTER;
    
	if (TIMER_GLOBAL_PORT(timnum, 4, select) == 0) return;
	lld_timer_baseinit(obj, timnum, 0.00001f);
    
	GPIO_TypeDef *init_port_tmp = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timnum, 4, select);
    lld_gpio_init(&obj->cnter, init_port_tmp, TIMER_GLOBAL_PINNUM(timnum, 4, select),
			GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timnum));

	obj->pulsecnt = 0;
}

void lld_timer_init_pwm(lld_timer_t *obj, uint8_t timnum, float frqHz, bool isPWM, 
    uint8_t ch1Sx, uint8_t ch2Sx, uint8_t ch3Sx, uint8_t ch4Sx)
{
	if (timnum > 8 || timnum == 6 || timnum == 7 || timnum < 1 || frqHz < 0.1f) return;
	mcu_timer_list[timnum - 1] = obj;
    obj->timnum = timnum;
	if (isPWM) {
		obj->mode = TIM_MODE_PWM;
		lld_timer_baseinit(obj, timnum, 1.0f / frqHz);
        lld_timer_opc_init(obj, true, timnum, ch1Sx, ch2Sx, ch3Sx, ch4Sx);
	}else {
		obj->mode = TIM_MODE_IPC;
		lld_timer_baseinit(obj, timnum, 1.0f / frqHz);
        lld_timer_ipc_init(obj, timnum, ch1Sx, ch2Sx, ch3Sx, ch4Sx);
	}
}

void lld_timer_init_encoder(lld_timer_t *obj, uint8_t timnum, uint8_t selectA, uint8_t selectB)
{
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
	mcu_timer_list[timnum - 1] = obj;
    obj->timnum = timnum;
    obj->mode = TIM_MODE_ENCODER;

	lld_timer_baseinit(obj, timnum, 1.0f);
	
	GPIO_TypeDef *init_port_ch1 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timnum,0,selectA);
	GPIO_TypeDef *init_port_ch2 = (GPIO_TypeDef *)TIMER_GLOBAL_PORT(timnum,1,selectB);
    
    lld_gpio_init(&obj->encoder_a, init_port_ch1, TIMER_GLOBAL_PINNUM(timnum,0,selectA),
				GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timnum));
    lld_gpio_init(&obj->encoder_b, init_port_ch2, TIMER_GLOBAL_PINNUM(timnum,1,selectB),
				GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, TIMER_GLOBAL_ALTERNATE(timnum));
	obj->encoderzero = 0;
}

void lld_timer_init_fvs(lld_timer_t *obj, uint8_t timnum,
    uint8_t ch1Sx, uint8_t ch2Sx, uint8_t ch3Sx, uint8_t ch4Sx)
{
	if (timnum > 8 || timnum == 6 || timnum == 7) return;
	mcu_timer_list[timnum - 1] = obj;
    obj->timnum = timnum;
    obj->mode = TIM_MODE_FRQ;
    
    lld_timer_baseinit(obj, timnum, 0.01f);
    lld_timer_opc_init(obj, false, timnum, ch1Sx, ch2Sx, ch3Sx, ch4Sx);
}

void lld_timer_irq_inputcapture(lld_timer_t *obj)
{
	/* Rising or falling interrupt flag, inverse each time */
	static bool isRising[4]  = {true, true, true,true};
	static uint32_t lstCount_Rising[4] = {0};
	static uint32_t lstCount_Falling[4] = {0};
	static uint32_t ccr;
	
	static HAL_TIM_ActiveChannel icChannel[4] = {HAL_TIM_ACTIVE_CHANNEL_1, HAL_TIM_ACTIVE_CHANNEL_2, 
											HAL_TIM_ACTIVE_CHANNEL_3, HAL_TIM_ACTIVE_CHANNEL_4};
	static uint32_t timChannel[4] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4};
	static uint8_t chIndex = 0;
	
	for (chIndex = 0; chIndex < 4; chIndex++) {
		if(obj->htim.Channel == icChannel[chIndex])
			break;
	}
	
	if (chIndex >= 4) return;
	
	ccr = HAL_TIM_ReadCapturedValue(&obj->htim, timChannel[chIndex]); 
	
	if (isRising[chIndex]) {
		if (ccr > lstCount_Falling[chIndex]) {
			obj->ipcdata.level_low[chIndex] = ccr - lstCount_Falling[chIndex];
		}else {                               
			obj->ipcdata.level_low[chIndex] = obj->timattr.period + ccr - lstCount_Falling[chIndex];
		}
		
		if (ccr > lstCount_Rising[chIndex]) {
			obj->ipcdata.period_rising[chIndex] = ccr - lstCount_Rising[chIndex];
		}else {                            
			obj->ipcdata.period_rising[chIndex] = obj->timattr.period + ccr - lstCount_Rising[chIndex];
		}
		
		lstCount_Rising[chIndex] = ccr;
		
		TIM_RESET_CAPTUREPOLARITY(&obj->htim, timChannel[chIndex]);
		TIM_SET_CAPTUREPOLARITY(&obj->htim, timChannel[chIndex], TIM_ICPOLARITY_FALLING);
	}else {
		if (ccr > lstCount_Rising[chIndex])
			obj->ipcdata.level_high[chIndex] = ccr - lstCount_Rising[chIndex];
		else
			obj->ipcdata.level_high[chIndex] = obj->timattr.period + ccr - lstCount_Rising[chIndex];
		
		if (ccr > lstCount_Falling[chIndex])
			obj->ipcdata.period_falling[chIndex] = ccr - lstCount_Falling[chIndex];
		else
			obj->ipcdata.period_falling[chIndex] = obj->timattr.period + ccr - lstCount_Falling[chIndex];
		
		lstCount_Falling[chIndex] = ccr;
		
		TIM_RESET_CAPTUREPOLARITY(&obj->htim, timChannel[chIndex]);
		TIM_SET_CAPTUREPOLARITY(&obj->htim, timChannel[chIndex], TIM_ICPOLARITY_RISING);
	}
	
	isRising[chIndex] = !isRising[chIndex];	
}

void lld_timer_irq_encoder(lld_timer_t *obj)
{
	uint16_t cnt = __HAL_TIM_GetCounter(&obj->htim);
	if (cnt < 30000) {
		/* Count Up Overflow, Positive diraction, add one period */
		obj->pulsecnt += 60000;
	} else {
		/* Count Down Overflow, Negtive diraction, minus one period */
		obj->pulsecnt -= 60000;
	}
}

void lld_timer_irq_cnter(lld_timer_t *obj)
{
    obj->pulsecnt += 60000;
}

void lld_timer_irq(lld_timer_t *obj)
{
	switch (obj->mode) {
	case TIM_MODE_TIMER:
		if (obj->updateirq == 0) return;
		obj->updateirq();
		break;
	case TIM_MODE_IPC:
		lld_timer_irq_inputcapture(obj);
		break;
	case TIM_MODE_COUNTER:
		lld_timer_irq_cnter(obj);
		break;
	case TIM_MODE_ENCODER:
		lld_timer_irq_encoder(obj);
		break;
	case TIM_MODE_PWM:
	default: break;
	}
}

void lld_timer_sethz(lld_timer_t *obj, float hz)
{
	if (hz < 1) hz = 1;
	else if (hz > 300000) hz = 300000;
	float cnt = 100000000.0f / hz;
	/* Maxium Ticks / 2^16 = prescaler, make full use of CNT's 16 bits register */
	uint16_t prescaler = (uint16_t)(cnt / 65536.0f);
	/* period = Maxium Ticks / prescaler, from 0, so - 1 */
	uint16_t period = (uint16_t)(cnt / (prescaler + 1)) - 1;
	obj->htim.Instance->PSC = prescaler;
	obj->htim.Instance->ARR = period;
}

void lld_timer_setduty(lld_timer_t *obj, float ch1,float ch2, float ch3, float ch4)
{
	obj->htim.Instance->CCR1 = obj->timattr.period * (100.0f - ch1) / 100.0f;
	obj->htim.Instance->CCR2 = obj->timattr.period * (100.0f - ch2) / 100.0f;
	obj->htim.Instance->CCR3 = obj->timattr.period * (100.0f - ch3) / 100.0f;
	obj->htim.Instance->CCR4 = obj->timattr.period * (100.0f - ch4) / 100.0f;
}

int64_t lld_timer_cnts(lld_timer_t *obj)
{
	int64_t cnt = obj->pulsecnt + obj->htim.Instance->CNT;
	return cnt;
}

double lld_timer_ic_level(lld_timer_t *obj, uint8_t chnum, bool high)
{
	if (chnum < 1 || chnum > 4) return -1; 
	if (high) {
		return obj->ipcdata.level_high[chnum - 1] * obj->timattr.resolution;
	} else {
		return obj->ipcdata.level_low[chnum - 1] * obj->timattr.resolution;
	}
}

double lld_timer_ic_period(lld_timer_t *obj, uint8_t chnum, bool rising)
{
	if (chnum < 1 || chnum > 4) return -1; 
	if (rising) {
		return obj->ipcdata.period_rising[chnum - 1] * obj->timattr.resolution;
	} else {
		return obj->ipcdata.period_falling[chnum - 1] * obj->timattr.resolution;
	}
}


/*****************************************************************
 *****  CubeH7 Timer interrupt and callback
 ****************************************************************
*/
/**
 * @brief 	Timer Update IRQ callback Function (when coutner overflow)
 *			it will be called by HAL_TIM_IRQHandler
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)			lld_timer_irq(mcu_timer_list[0]);
	else if (htim->Instance == TIM2)	lld_timer_irq(mcu_timer_list[1]);
	else if (htim->Instance == TIM3)	lld_timer_irq(mcu_timer_list[2]);
	else if (htim->Instance == TIM4)	lld_timer_irq(mcu_timer_list[3]);
	else if (htim->Instance == TIM5)	lld_timer_irq(mcu_timer_list[4]);
	else if (htim->Instance == TIM6)	lld_timer_irq(mcu_timer_list[5]);
	else if (htim->Instance == TIM7)	lld_timer_irq(mcu_timer_list[6]);
	else if (htim->Instance == TIM8)	lld_timer_irq(mcu_timer_list[7]);
}

/**
 * @brief 	Timer InputCapture IRQ callback Function
 *			it will be called by HAL_TIM_IRQHandler
*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)			lld_timer_irq(mcu_timer_list[0]);
	else if (htim->Instance == TIM2)	lld_timer_irq(mcu_timer_list[1]);
	else if (htim->Instance == TIM3)	lld_timer_irq(mcu_timer_list[2]);
	else if (htim->Instance == TIM4)	lld_timer_irq(mcu_timer_list[3]);
	else if (htim->Instance == TIM5)	lld_timer_irq(mcu_timer_list[4]);
	else if (htim->Instance == TIM6)	lld_timer_irq(mcu_timer_list[5]);
	else if (htim->Instance == TIM7)	lld_timer_irq(mcu_timer_list[6]);
	else if (htim->Instance == TIM8)	lld_timer_irq(mcu_timer_list[7]);
}

/** TIM1 Up  IRQ */
void TIM1_UP_IRQHandler(void)
{ 
	if (mcu_timer_list[0]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[0]->htim));
}

/** TIM1 CC  IRQ */
void TIM1_CC_IRQHandler(void)
{ 
	if (mcu_timer_list[0]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[0]->htim));
}

/** TIM2 IRQ */
void TIM2_IRQHandler(void)
{ 
	if (mcu_timer_list[1]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[1]->htim));
}

/** TIM3 IRQ */
void TIM3_IRQHandler(void)
{ 
	if (mcu_timer_list[2]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[2]->htim));
}

/** TIM4 IRQ */
void TIM4_IRQHandler(void)
{ 
	if (mcu_timer_list[3]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[3]->htim));
}

/** TIM5 IRQ */
void TIM5_IRQHandler(void) 
{ 
	if (mcu_timer_list[4]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[4]->htim));
}

/** TIM6 DAC IRQ */
void TIM6_DAC_IRQHandler(void)
{ 
	if (mcu_timer_list[5]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[5]->htim));
}

/** TIM7 IRQ */
void TIM7_IRQHandler(void)
{ 
	if (mcu_timer_list[6]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[6]->htim));
}

/** TIM8 UP IRQ */
void TIM8_UP_TIM13_IRQHandler(void)
{ 
	if (mcu_timer_list[7]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[7]->htim));
}

/** TIM8 CC IRQ */
void TIM8_CC_IRQHandler(void)
{ 
	if (mcu_timer_list[7]) 
		HAL_TIM_IRQHandler(&(mcu_timer_list[7]->htim));
}






