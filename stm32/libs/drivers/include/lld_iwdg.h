#ifndef __LLD_IWDG_H_
#define __LLD_IWDG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * low level driver for stm32h7 series, base on cubehal library
 * module independent watch dog
*/

#include "lld_h7_global.h"

typedef enum __lld_iwdt_presc {
    PRE_4 = IWDG_PRESCALER_4,
    PRE_8 = IWDG_PRESCALER_8,
    PRE_16 = IWDG_PRESCALER_16,
    PRE_32 = IWDG_PRESCALER_32,
    PRE_64 = IWDG_PRESCALER_64,
    PRE_128 = IWDG_PRESCALER_128,
    PRE_256 = IWDG_PRESCALER_256 
} lld_iwdt_presc_t;

/**
 * @brief 	the Constructor of IWDG
 * 			 Initialize IWDG
 *			 the can bus baudrate computational formula is --->
 *												pre * reload	       64*1000				
 *			the times of reset signal time  = ---------------    = --------------- = 2s
 *												32000(LSI_HZ)           32000
 * @param _pre value of enum prescaler
 *		@arg PRE_4
 *		@arg PRE_8
 *		@arg PRE_16
 *		@arg PRE_32
 *		@arg PRE_64
 *		@arg PRE_128
 *		@arg PRE_256
 * @param reload register of reload, the max value is 4095
*/
void lld_iwdg_init(lld_iwdt_presc_t _pre, uint16_t reload);
void lld_iwdg_feed(void);

#ifdef __cplusplus
}
#endif

#endif
