#ifndef __LLD_TIMER_H_
#define __LLD_TIMER_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module timer
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "lld_kernel.h"
#include "lld_gpio.h"

typedef enum __lld_timer_mode {
    TIM_MODE_TIMER      = 0x01,	/**< timer mode */
    TIM_MODE_PWM		= 0x02,	/**< PWM output mode */
    TIM_MODE_IPC		= 0x03,	/**< Input capture mode */
    TIM_MODE_ENCODER	= 0x04,	/**< Encoder mode */
    TIM_MODE_FRQ		= 0x05,	/**< Square Pulse with varible frequence mode */
    TIM_MODE_COUNTER	= 0x06 	/**< Counter mode */
} lld_timer_mode_t;

typedef struct __lld_timer_attr {
    uint16_t period;
    double resolution;
} lld_timer_attr_t;

typedef struct __lld_timer {
    TIM_TypeDef *_tim;
    uint8_t timnum;
    uint8_t mode;
    lld_timer_attr_t timattr;
    
    lld_gpio_t ch1;
    lld_gpio_t ch2;
    lld_gpio_t ch3;
    lld_gpio_t ch4;
    lld_gpio_t encoder;
    
    int64_t encoderpulse;
    int64_t encoderzero;
    uint32_t ipc_cnt[4];
    void (*updateirq)();
} lld_timer_t;

extern lld_timer_t *mcu_timer_list[8];

/* init mode: timer mode */
void lld_timer_init_tim(lld_timer_t *obj, uint8_t timnum, float ts, void (*updateirq)());

/* init mode: counter mode */
void lld_timer_init_cnter(lld_timer_t *obj, uint8_t timnum, uint16_t exttriggerpsc, bool remap);

/* init mode: encoder mode */
void lld_timer_init_encoder(lld_timer_t *obj, uint8_t timnum, bool remap);

/* init mode: Frequence variable Signal output */
void lld_timer_init_fvs(lld_timer_t *obj, uint8_t timnum, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4);

/* init mode: pwm or input capture */
void lld_timer_init_pwm(lld_timer_t *obj, uint8_t timnum, float frqHz, bool isPWM, bool remap, 
    bool ench1, bool ench2, bool ench3, bool ench4);

void lld_timer_enable(lld_timer_t *obj, bool enable);
void lld_timer_sethz(lld_timer_t *obj, float hz);
void lld_timer_setduty(lld_timer_t *obj, float ch1,float ch2, float ch3, float ch4);
double lld_timer_inputcapturevalue(lld_timer_t *obj, uint8_t chnum);
int64_t lld_timer_cnts(lld_timer_t *obj);

void lld_timer_irq_inputcapture(lld_timer_t *obj);
void lld_timer_irq_encoder(lld_timer_t *obj);
void lld_timer_irq(lld_timer_t *obj);


#ifdef __cplusplus
}
#endif

#endif
