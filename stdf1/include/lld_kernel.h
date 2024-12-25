#ifndef __LLD_KERNEL_H_
#define __LLD_KERNEL_H_

/**
 * low level driver for stm32f1 series, base on std periph library
 * module for core kernel
*/

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x.h"

typedef enum __lld_rwway {
    RWPOLL,
    RWIT,
    RWDMA
} lld_rwway;
    
void lld_kernel_init(uint32_t init);
void lld_kernel_irq(void);
uint64_t lld_kernel_get_time(uint32_t way);


#ifdef __cplusplus
}
#endif

#endif
