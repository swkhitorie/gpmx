#include "drv_uart.h"
#include <stddef.h>

struct drv_uart_t *drv_uart_list[5] = {0, 0, 0, 0, 0};

/**
 * init uart tx and rx gpio
*/
static void drv_uart_io_init(uint8_t num)
{
    uint32_t tx_port_array[5] = {GPIOA, GPIOA, GPIOB, GPIOC, GPIOC};
    uint32_t tx_pin_array[5] =  { 9,     2,     10,    10,    12};
    
    uint32_t rx_port_array[5] = {GPIOA, GPIOA, GPIOB, GPIOC, GPIOD};
    uint32_t rx_pin_array[5] =  {10,     3,     11,    11,    2};  
    
    drv_gpio_init(tx_port_array[num], tx_pin_array[num], IOMODE_AF_PP, IOSPEED_50M);
    drv_gpio_init(rx_port_array[num], rx_pin_array[num], IOMODE_IN_FLOATING, IOSPEED_50M);
}

/**
 * init uart tx and rx dma parameters
 * dir=0 -> memory to peripheral, dir=1 -> peripheral to memory
*/
static void drv_uart_dma_init(struct drv_uart_dma_attr_t *attr,
                         uint32_t dma_periph,
                         dma_channel_enum channel,
                         uint32_t periph_addr, uint8_t dir)
{
    dma_parameter_struct dma_init_obj;

    dma_deinit(dma_periph, channel);
    dma_struct_para_init(&dma_init_obj);
    dma_init_obj.direction    = (dir == 0) ? DMA_MEMORY_TO_PERIPHERAL : 
                                             DMA_PERIPHERAL_TO_MEMORY;
    dma_init_obj.memory_addr  = (uint32_t)attr->mem_buff;
    dma_init_obj.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_obj.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_obj.number       = attr->mem_capacity;
    dma_init_obj.periph_addr  = periph_addr;
    dma_init_obj.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_obj.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_obj.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(dma_periph, channel, &dma_init_obj);
    dma_circulation_disable(dma_periph, channel);
    dma_memory_to_memory_disable(dma_periph, channel);

    dma_interrupt_enable(dma_periph, channel, DMA_INT_FTF);
    dma_interrupt_flag_clear(dma_periph, channel, DMA_INT_FLAG_G);
    dma_interrupt_flag_clear(dma_periph, channel, DMA_INT_FLAG_FTF);
}

/**
 * init uart dma attribute parameters
*/
void drv_uart_dma_attr_init(struct drv_uart_dma_attr_t *obj, 
                            uint8_t *p, uint32_t len, 
                            uint8_t priority)
{
    obj->mem_buff = p;
    obj->mem_capacity = len;
    obj->priority = priority;
}

/**
 * init uart attribute parameters
*/
void drv_uart_attr_init(struct drv_uart_attr_t *obj, 
                        uint32_t baudrate, uint32_t wordlen,
                        uint32_t stopbitlen, uint32_t parity,
                        uint8_t priority)
{
    obj->baudrate = baudrate;
    obj->wordlen = wordlen;
    obj->stopbitlen = stopbitlen;
    obj->parity = parity;
    obj->priority = priority;
}

/**
 * init uart with configure obj of uart, uart_tx_dma, uart_rx_dma
 * if dma is not used, give param with NULL
 * step:
 * init uart io 
 * init uart
 * init uart tx dma and rx dma (if dma is not NULL)
 * config uart irq IDLE
*/
void drv_uart_init(uint8_t num, struct drv_uart_t *obj,
                   struct drv_uart_attr_t *com_attr,
                   struct drv_uart_dma_attr_t *txdma_attr,
                   struct drv_uart_dma_attr_t *rxdma_attr)
{
    uint32_t com_array[5] = {USART0, USART1, USART2, UART3, UART4};
    rcu_periph_enum clk_array[5] = {RCU_USART0, RCU_USART1, RCU_USART2, RCU_UART3, RCU_UART4};
    uint32_t com_irq[5] = {USART0_IRQn, USART1_IRQn, USART2_IRQn, UART3_IRQn, UART4_IRQn};
    
    uint32_t dma_clk[5] = {RCU_DMA0, RCU_DMA0, RCU_DMA0, RCU_DMA1, 0};
    uint32_t dma_periph_array[5] = {DMA0, DMA0, DMA0, DMA1, 0};
    uint32_t dma_tx_channel_array[5] = {DMA_CH3, DMA_CH6, DMA_CH1, DMA_CH4, 0};
    uint32_t dma_rx_channel_array[5] = {DMA_CH4, DMA_CH5, DMA_CH2, DMA_CH2, 0};
    uint32_t dma_tx_irq[5] = {DMA0_Channel3_IRQn, DMA0_Channel6_IRQn, DMA0_Channel1_IRQn, DMA1_Channel4_IRQn, 0};
    uint32_t dma_rx_irq[5] = {DMA0_Channel4_IRQn, DMA0_Channel5_IRQn, DMA0_Channel2_IRQn, DMA1_Channel2_IRQn, 0};

    uint32_t com = com_array[num];
    uint32_t clk = clk_array[num];

    obj->com = com;
    obj->txdma_periph = dma_periph_array[num];
    obj->rxdma_periph = dma_periph_array[num];
    obj->tx_channel = dma_tx_channel_array[num];
    obj->rx_channel = dma_rx_channel_array[num];
    obj->attr = *com_attr;

    rcu_periph_clock_enable(clk);

    drv_uart_io_init(num);

    usart_deinit(com);
    usart_baudrate_set(com, com_attr->baudrate);
    usart_word_length_set(com, com_attr->wordlen);
    usart_stop_bit_set(com, com_attr->stopbitlen);
    usart_parity_config(com, com_attr->parity);

    usart_hardware_flow_rts_config(com, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(com, USART_CTS_DISABLE);

    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
    
    if (txdma_attr != NULL) {
        obj->attr_txdma = *txdma_attr;
        rcu_periph_clock_enable(dma_clk[num]);
        drv_uart_dma_init(txdma_attr, obj->txdma_periph,
                          obj->tx_channel, (uint32_t)&USART_DATA(com), 0);
        nvic_irq_enable(dma_tx_irq[num], txdma_attr->priority, 0);
        // usart_dma_transmit_config(com, USART_TRANSMIT_DMA_ENABLE);
    } else {
        obj->attr_txdma.mem_capacity = 0;
    }

    if (rxdma_attr != NULL) {
        obj->attr_rxdma = *rxdma_attr;
        rcu_periph_clock_enable(dma_clk[num]);
        usart_dma_receive_config(com, USART_RECEIVE_DMA_ENABLE);
        drv_uart_dma_init(rxdma_attr, obj->rxdma_periph,
                          obj->rx_channel, (uint32_t)&USART_DATA(com), 1);
        dma_channel_enable(obj->rxdma_periph, obj->rx_channel);
        nvic_irq_enable(dma_rx_irq[num], rxdma_attr->priority, 0);
    } else {
        obj->attr_rxdma.mem_capacity = 0;
    }

    nvic_irq_enable(com_irq[num], com_attr->priority, 0);
    usart_flag_clear(com, USART_FLAG_TC);
    usart_interrupt_enable(com, USART_INT_IDLE);
    
    obj->tx_busy = false;
    drv_uart_list[num] = obj;
}

void drv_uart_buff_init(struct drv_uart_t *obj, uint8_t *txbuf, uint16_t tlen,
                        uint8_t *rxbuf, uint16_t rlen)
{
    devbuf_init(&obj->tx_buf, txbuf, tlen);
    devbuf_init(&obj->rx_buf, rxbuf, rlen);
}

/**
 * send data with uart
 * way == 0: send with BLOCK way
 * way == 1: send with DMA way
*/
int drv_uart_send(struct drv_uart_t *obj, const uint8_t *p, uint16_t len, uint8_t way)
{
    if (way == 0) {
        for (int i = 0; i < len; i++) {
            usart_data_transmit(obj->com, p[i]);
            while(RESET == usart_flag_get(obj->com, USART_FLAG_TBE)); 
        }
    } else if (way == 1) {
        devbuf_write(&obj->tx_buf, &p[0], len);
        if (obj->tx_busy) return false;
        obj->tx_busy = true;
        if (obj->attr_txdma.mem_buff != NULL) {
            uint16_t bufsize = (uint32_t)devbuf_size(&obj->tx_buf);
            if (bufsize > obj->attr_txdma.mem_capacity) bufsize = obj->attr_txdma.mem_capacity;
            devbuf_read(&obj->tx_buf, &obj->attr_txdma.mem_buff[0], bufsize);
            
            dma_channel_disable(obj->txdma_periph, obj->tx_channel);
//            dma_interrupt_flag_clear(obj->txdma_periph, obj->tx_channel, DMA_INT_FLAG_FTF);
//            dma_interrupt_flag_clear(obj->txdma_periph, obj->tx_channel, DMA_INT_FLAG_G);
//            dma_memory_address_config(obj->txdma_periph, obj->tx_channel, &obj->attr_txdma.mem_buff[0]);
            dma_transfer_number_config(obj->txdma_periph, obj->tx_channel, bufsize);
            usart_dma_transmit_config(obj->com, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(obj->txdma_periph, obj->tx_channel);
        }
    }
    return 0;
}

void drv_uart_irq(struct drv_uart_t *obj)
{
    if (obj == NULL) return;

    if (RESET != usart_interrupt_flag_get(obj->com, USART_INT_FLAG_IDLE)) {
        usart_interrupt_flag_clear(obj->com, USART_INT_FLAG_IDLE);
        usart_data_receive(obj->com);

        dma_channel_disable(obj->rxdma_periph, obj->rx_channel);

        uint16_t len = obj->attr_rxdma.mem_capacity - 
                       dma_transfer_number_get(obj->rxdma_periph, obj->rx_channel);

        devbuf_write(&obj->rx_buf, &obj->attr_rxdma.mem_buff[0], len);

        dma_memory_address_config(obj->rxdma_periph, obj->rx_channel, 
                                 (uint32_t)&obj->attr_rxdma.mem_buff[0]);
        //dma_transfer_number_config(obj->rxdma_periph, obj->rx_channel, obj->attr_rxdma.mem_capacity);
        dma_channel_enable(obj->rxdma_periph, obj->rx_channel);
    }
}

void drv_uart_txdma_irq(struct drv_uart_t *obj)
{
    if (obj == NULL || obj->attr_txdma.mem_capacity == 0) return;
    if (dma_interrupt_flag_get(obj->txdma_periph, obj->tx_channel, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(obj->txdma_periph, obj->tx_channel, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(obj->txdma_periph, obj->tx_channel, DMA_INT_FLAG_FTF);

	    uint32_t len = devbuf_size(&obj->tx_buf);
	    if (len > 0) {
		    if (len > obj->attr_txdma.mem_capacity) len = obj->attr_txdma.mem_capacity;
        
            devbuf_read(&obj->tx_buf, &obj->attr_txdma.mem_buff[0], len);
		    obj->tx_busy = true;
            
            dma_transfer_number_config(obj->txdma_periph, obj->tx_channel, len);
            usart_dma_transmit_config(obj->com, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(obj->txdma_periph, obj->tx_channel);
	    } else {
		    obj->tx_busy = false; 
	    }
    }
}

void drv_uart_rxdma_irq(struct drv_uart_t *obj)
{
    if (obj == NULL || obj->attr_rxdma.mem_capacity == 0) return;
    if (dma_interrupt_flag_get(obj->rxdma_periph, obj->rx_channel, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(obj->rxdma_periph, obj->rx_channel, DMA_INT_FLAG_G);
        dma_channel_disable(obj->rxdma_periph, obj->rx_channel);

        uint16_t len = obj->attr_rxdma.mem_capacity;

        devbuf_write(&obj->rx_buf, &obj->attr_rxdma.mem_buff[0], len);

        dma_memory_address_config(obj->rxdma_periph, obj->rx_channel, 
                                 (uint32_t)&obj->attr_rxdma.mem_buff[0]);
        dma_transfer_number_config(obj->rxdma_periph, obj->rx_channel, obj->attr_rxdma.mem_capacity);
        dma_channel_enable(obj->rxdma_periph, obj->rx_channel);
    }
}

void USART0_IRQHandler()
{
    drv_uart_irq(drv_uart_list[0]);
}

void USART1_IRQHandler()
{
    drv_uart_irq(drv_uart_list[1]);
}

void USART2_IRQHandler()
{
    drv_uart_irq(drv_uart_list[2]);
}

void UART3_IRQHandler()
{
    drv_uart_irq(drv_uart_list[3]);
}

void UART4_IRQHandler()
{
    drv_uart_irq(drv_uart_list[4]);
}

void DMA0_Channel3_IRQHandler()
{
    drv_uart_txdma_irq(drv_uart_list[0]);
}

void DMA0_Channel4_IRQHandler()
{
    drv_uart_rxdma_irq(drv_uart_list[0]);
}

void DMA0_Channel6_IRQHandler()
{
    drv_uart_txdma_irq(drv_uart_list[1]);
}

void DMA0_Channel5_IRQHandler()
{
    drv_uart_rxdma_irq(drv_uart_list[1]);
}

void DMA0_Channel1_IRQHandler()
{
    drv_uart_txdma_irq(drv_uart_list[2]);
}

void DMA0_Channel2_IRQHandler()
{
    drv_uart_rxdma_irq(drv_uart_list[2]);
}

void DMA1_Channel4_IRQHandler()
{
    drv_uart_txdma_irq(drv_uart_list[3]);
}

void DMA1_Channel2_IRQHandler()
{
    drv_uart_rxdma_irq(drv_uart_list[3]);
}

