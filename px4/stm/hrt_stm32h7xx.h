#ifndef HRT_STM32H7XX_H_
#define HRT_STM32H7XX_H_

#include <stdint.h>
#include <board_config.h>

#define HRT_TIMER_CHANNEL     (1)
#define HRT_TIMER             (1)

#define modifyreg32           MODIFY_REG
#define up_enable_irq         __NVIC_EnableIRQ

/* event generation register */
#define GTIM_EGR_UG         (TIM_EGR_UG)

/* control register 1 */
#define GTIM_CR1_CEN        (TIM_CR1_CEN)

/* interrupt enable for HRT */
#define GTIM_DIER_CC1IE     (TIM_DIER_CC1IE)
#define GTIM_DIER_CC2IE     (TIM_DIER_CC2IE)
#define GTIM_DIER_CC3IE     (TIM_DIER_CC3IE)
#define GTIM_DIER_CC4IE     (TIM_DIER_CC4IE)

/* interrupt status for HRT */
#define GTIM_SR_CC1IF       (TIM_SR_CC1IF)
#define GTIM_SR_CC2IF       (TIM_SR_CC2IF)
#define GTIM_SR_CC3IF       (TIM_SR_CC3IF)
#define GTIM_SR_CC4IF       (TIM_SR_CC4IF)

#define STM32_TIM1_BASE     (TIM1_BASE)
#define STM32_TIM2_BASE     (TIM2_BASE)
#define STM32_TIM3_BASE     (TIM3_BASE)
#define STM32_TIM4_BASE     (TIM4_BASE)
#define STM32_TIM5_BASE     (TIM5_BASE)
#define STM32_TIM8_BASE     (TIM8_BASE)

#define STM32_GTIM_CR1_OFFSET      0x0000  /* Control register 1 (16-bit) */
#define STM32_GTIM_CR2_OFFSET      0x0004  /* Control register 2 (16-bit, TIM2, 5 only) */
#define STM32_GTIM_SMCR_OFFSET     0x0008  /* Slave mode control register (32-bit, TIM2, 5, 9, 12 only) */
#define STM32_GTIM_DIER_OFFSET     0x000c  /* DMA/Interrupt enable register (16-bit) */
#define STM32_GTIM_SR_OFFSET       0x0010  /* Status register (16-bit) */
#define STM32_GTIM_EGR_OFFSET      0x0014  /* Event generation register (16-bit) */
#define STM32_GTIM_CCMR1_OFFSET    0x0018  /* Capture/compare mode register 1 (32-bit) */
#define STM32_GTIM_CCMR2_OFFSET    0x001c  /* Capture/compare mode register 2 (32-bit on TIM2,5 only) */
#define STM32_GTIM_CCER_OFFSET     0x0020  /* Capture/compare enable register (16-bit) */
#define STM32_GTIM_CNT_OFFSET      0x0024  /* Counter (16 bit and 32-bit on TIM2, 5 only) */
#define STM32_GTIM_PSC_OFFSET      0x0028  /* Prescaler (16-bit) */
#define STM32_GTIM_ARR_OFFSET      0x002c  /* Auto-reload register (16 bit and 32-bit on TIM2, 5 only) */
#define STM32_GTIM_CCR1_OFFSET     0x0034  /* Capture/compare register 1 (16-bit on all TIMx and 32-bit on TIM2,5 only) */
#define STM32_GTIM_CCR2_OFFSET     0x0038  /* Capture/compare register 2 (16-bit TIM 3-4, 9, 12 and 32-bit on TIM2,5 only) */
#define STM32_GTIM_CCR3_OFFSET     0x003c  /* Capture/compare register 3 (16-bit TIM 3-4 and 32-bit on TIM2,5 only) */
#define STM32_GTIM_CCR4_OFFSET     0x0040  /* Capture/compare register 4 (16-bit TIM 3-4 and 32-bit on TIM2,5 only) */
#define STM32_GTIM_DCR_OFFSET      0x0048  /* DMA control register (16-bit, TIM2-5 only) */
#define STM32_GTIM_DMAR_OFFSET     0x004c  /* DMA address for burst mode (16-bit, TIM2-5 only) */

#define STM32_RCC_APB2ENR          (RCC->APB2ENR)
#define STM32_RCC_APB1ENR          (RCC_C1->APB1LENR)

// #define RCC_APB2ENR_TIM1EN         (RCC_APB2ENR_TIM1EN)
#define RCC_APB1ENR_TIM2EN         (RCC_APB1LENR_TIM2EN)
#define RCC_APB1ENR_TIM3EN         (RCC_APB1LENR_TIM3EN)
#define RCC_APB1ENR_TIM4EN         (RCC_APB1LENR_TIM4EN)
#define RCC_APB1ENR_TIM5EN         (RCC_APB1LENR_TIM5EN)
// #define RCC_APB2ENR_TIM8EN         (RCC_APB2ENR_TIM8EN)

#define STM32_IRQ_TIM1CC           (TIM1_CC_IRQn)
#define STM32_IRQ_TIM2             (TIM2_IRQn)
#define STM32_IRQ_TIM3             (TIM3_IRQn)
#define STM32_IRQ_TIM4             (TIM4_IRQn)
#define STM32_IRQ_TIM5             (TIM5_IRQn)
#define STM32_IRQ_TIM8CC           (TIM8_CC_IRQn)

#endif
