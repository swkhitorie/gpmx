#ifndef DRIVE_DMA_H_
#define DRIVE_DMA_H_

#include <stdbool.h>
#include "drv_cmn.h"

#if defined(DRV_STM32_F1) || defined(DRV_STM32_WL)

#define DMA_INSTANCE_TYPE              DMA_Channel_TypeDef
#elif defined(DRV_STM32_F4) || defined(DRV_STM32_H7)

#define DMA_INSTANCE_TYPE              DMA_Stream_TypeDef
#endif

struct dma_config {
    DMA_INSTANCE_TYPE *instance;
    uint32_t dma_rcc;
    IRQn_Type dma_irq;

#if defined(DRV_STM32_F4)
    uint32_t channel;
#endif

#if defined(DRV_STM32_H7) || defined(DRV_STM32_WL)
    uint32_t request;
#endif

    uint32_t priority;

    bool enable;
};

#endif
