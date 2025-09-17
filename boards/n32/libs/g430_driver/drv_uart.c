#include "drv_uart.h"

struct n32_uart_dev *g_uart_list[4] = {0, 0, 0, 0};

static void _usart_pin_config(GPIO_Module* txport, uint8_t txpin, uint32_t txalternate,
    GPIO_Module* rxport, uint8_t rxpin, uint32_t rxalternate);
static void _usart_config(uint8_t id, uint32_t baudrate, uint32_t wordlen, uint32_t stopbits, uint32_t parity);
static void _usart_irq_config(uint8_t id, uint8_t priority);
static void _usart_rxdma_config(uint8_t id, DMA_ChannelType *channel, uint8_t *buf, uint32_t size);
static void _usart_rxdma_idle_irq(struct n32_uart_dev *dev);

static void _usart_pin_config(GPIO_Module* txport, uint8_t txpin, uint32_t txalternate,
    GPIO_Module* rxport, uint8_t rxpin, uint32_t rxalternate)
{
    LOW_PERIPH_INITPIN(txport, txpin, GPIO_MODE_AF_PP, GPIO_NO_PULL, GPIO_SLEW_RATE_FAST, txalternate);
    LOW_PERIPH_INITPIN(rxport, rxpin, GPIO_MODE_AF_PP, GPIO_NO_PULL, GPIO_SLEW_RATE_FAST, rxalternate);
}

void _usart_config(uint8_t id, uint32_t baudrate, uint32_t wordlen, uint32_t stopbits, uint32_t parity)
{
    USART_InitType h_tty;
    USART_Module *uart_list[4] = {USART1, USART2, UART3, UART4};

    USART_Structure_Initializes(&h_tty);
    switch(id) {
    case 1: RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_USART1); break;
    case 2: RCC_APB1_Peripheral_Clock_Enable(RCC_APB1_PERIPH_USART2); break;
    case 3: RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_UART3); break;
    case 4: RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_UART4); break;
    }

    h_tty.BaudRate               = baudrate;
    h_tty.Parity                 = parity;     //USART_PE_NO
    h_tty.StopBits               = stopbits;   //USART_STPB_1
    h_tty.WordLength             = wordlen;    //USART_WL_8B
    h_tty.HardwareFlowControl    = USART_HFCTRL_NONE;
    h_tty.Mode                   = USART_MODE_RX | USART_MODE_TX;

    USART_Initializes(uart_list[id-1], &h_tty);
    USART_Enable(uart_list[id-1]);
}

void _usart_irq_config(uint8_t id, uint8_t priority)
{
    NVIC_InitType nvic_init;
    uint8_t uart_irq_list[4] = {USART1_IRQn, USART2_IRQn, UART3_IRQn, UART4_IRQn};
    USART_Module *uart_list[4] = {USART1, USART2, UART3, UART4};

    nvic_init.NVIC_IRQChannel                   = uart_irq_list[id-1];
    nvic_init.NVIC_IRQChannelCmd                = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = priority;
    NVIC_Initializes(&nvic_init);

    USART_DMA_Transfer_Enable(uart_list[id-1], USART_DMAREQ_RX|USART_DMAREQ_TX);
    USART_Interrput_Enable(uart_list[id-1], USART_INT_IDLEF);
}

void _usart_rxdma_config(uint8_t id, DMA_ChannelType *channel, uint8_t *buf, uint32_t size)
{
    DMA_InitType h_rdma;
    volatile uint16_t *periph_addr[4] = { &USART1->DAT, &USART2->DAT, &UART3->DAT, &UART4->DAT};
    uint32_t dma_req[4] = {DMA_REMAP_USART1_RX, DMA_REMAP_USART2_RX, DMA_REMAP_UART3_RX, DMA_REMAP_UART4_RX};

    RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_DMA);
    DMA_Reset(channel);  //DMA_CH1

    h_rdma.PeriphAddr      = (uint32_t)periph_addr[id-1];
    h_rdma.MemAddr         = (uint32_t)buf;
    h_rdma.BufSize         = size;

    h_rdma.Direction       = DMA_DIR_PERIPH_SRC;
    h_rdma.PeriphInc       = DMA_PERIPH_INC_MODE_DISABLE;
    h_rdma.MemoryInc       = DMA_MEM_INC_MODE_ENABLE;
    h_rdma.PeriphDataSize  = DMA_PERIPH_DATA_WIDTH_BYTE;
    h_rdma.MemDataSize     = DMA_MEM_DATA_WIDTH_BYTE;
    h_rdma.CircularMode    = DMA_CIRCULAR_MODE_DISABLE;
    h_rdma.Priority        = DMA_CH_PRIORITY_HIGHEST;
    h_rdma.Mem2Mem         = DMA_MEM2MEM_DISABLE;
    
    DMA_Initializes(channel, &h_rdma);
    DMA_Channel_Request_Remap(channel, dma_req[id-1]);
    DMA_Channel_Enable(channel);
}

void _usart_rxdma_idle_irq(struct n32_uart_dev *dev)
{
    USART_Module *uart_list[4] = {USART1, USART2, UART3, UART4};
    if (RESET != USART_Flag_Status_Get(uart_list[dev->id-1], USART_FLAG_IDLEF)) {
        uint16_t count = 0;
        uint32_t reg_tmp;

        USART_DMA_Transfer_Disable(uart_list[dev->id-1], USART_DMAREQ_RX);
        DMA_Channel_Disable(dev->rxdma_channel);

        // clear IDLE flag
        reg_tmp = uart_list[dev->id-1]->STS;
        reg_tmp = uart_list[dev->id-1]->DAT;

        count = dev->rxdma_size - DMA_Current_Data_Transfer_Number_Get(dev->rxdma_channel);

        rbdrv_write(&dev->rxbuf, dev->rxdma_buffer, count);
        // printf("%s %d %d %d \n", (char *)dev->rxdma_buffer, count, dev->rxdma_size, 
        //         DMA_Current_Data_Transfer_Number_Get(dev->rxdma_channel));
        // memset(dev->rxdma_buffer, 0, dev->rxdma_size);

        // reconfig dma addr
        DMA_Memory_Address_Config(dev->rxdma_channel, (uint32_t)dev->rxdma_buffer);  

        DMA_Current_Data_Transfer_Number_Set(dev->rxdma_channel, dev->rxdma_size); 
        
        USART_DMA_Transfer_Enable(uart_list[dev->id-1], USART_DMAREQ_RX);
        DMA_Channel_Enable(dev->rxdma_channel);
    }
    
}

void n32_uart_init(struct n32_uart_dev *dev)
{
    _usart_pin_config(dev->txport, dev->txpin, dev->txalternate,
        dev->rxport, dev->rxpin, dev->rxalternate);

    _usart_config(dev->id, dev->baud, dev->wordlen, dev->stopbits, dev->parity);
    _usart_rxdma_config(dev->id, dev->rxdma_channel, dev->rxdma_buffer, dev->rxdma_size);
    _usart_irq_config(dev->id, dev->priority);

    g_uart_list[dev->id-1] = dev;
}

uint16_t n32_uart_sendblk(struct n32_uart_dev *dev, uint8_t *p, uint16_t size)
{
    USART_Module *uart_list[4] = {USART1, USART2, UART3, UART4};
    int i = 0;

    for (; i < size; i++) {
        USART_Data_Send(uart_list[dev->id-1], p[i]);
        while (USART_Flag_Status_Get(uart_list[dev->id-1], USART_FLAG_TXDE) == RESET);
    }

    return size;
}

uint16_t n32_uart_dmasend(struct n32_uart_dev *dev, uint8_t *p, uint16_t size)
{
    int i = 0;
    USART_Module *uart_list[4] = {USART1, USART2, UART3, UART4};
    
    return size;
}

/****************************************************************************
 * N32 Interrupt 
 ****************************************************************************/
void USART1_IRQHandler(void)
{
    _usart_rxdma_idle_irq(g_uart_list[0]);
}

void USART2_IRQHandler(void)
{
    _usart_rxdma_idle_irq(g_uart_list[1]);
}

void UART3_IRQHandler(void)
{
    _usart_rxdma_idle_irq(g_uart_list[2]);
}

void UART4_IRQHandler(void)
{
    _usart_rxdma_idle_irq(g_uart_list[3]);
}

