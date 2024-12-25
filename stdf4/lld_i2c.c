/**
 * low level driver for stm32f4 series, base on std periph library
 * module i2c
*/

#include "lld_i2c.h"

/*
	=======================================================================
	|  I2C | remap=false |  remap=true |       IRQ   |      RCC           |
	|------|-------------|-------------|-------------|--------------------|
	|      |SCL | PB6    | SCL | PB8   |I2C1_EV_IRQn |                    |
	| I2C1 |-------------|-------------|             |RCC_APB1Periph_I2C1 |
	|      |SDA | PB7    | SDA | PB9   |I2C1_ER_IRQn |                    |
	|------|-------------|-------------|-------------|--------------------|
	|      |SCL | PB10   |     xxx     |I2C2_EV_IRQn |                    |
	| I2C2 |-------------|-------------|             |RCC_APB1Periph_I2C1 |
	|      |SDA | PB11   |     xxx     |I2C2_ER_IRQn |                    |
	|------|-------------|-------------|-------------|--------------------|
	|      |SCL | PA8    |     xxx     |I2C3_EV_IRQn |                    |
	| I2C3 |-------------|-------------|             |RCC_APB1Periph_I2C1 |
	|      |SDA | PC9    |     xxx     |I2C3_ER_IRQn |                    |
	=======================================================================
*/


lld_i2c_t* mcu_i2c_list[3] = {0, 0, 0};

void lld_i2c_cmd_set(lld_i2c_cmd_t *p, uint8_t saddr, uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t rxlen, bool istail)
{
    uint8_t len = (txlen >= 5 ? 5 : txlen);
    p->saddr = saddr;
    p->rxbuf = rxbuf;
    p->rxlen = rxlen;
    p->istail = istail;
    p->txlen = len;
    for (int i = 0; i < len; i++) {
        p->txbuf[i] = txbuf[i];
    }
}

void lld_i2c_cmd_copy(const lld_i2c_cmd_t *src, lld_i2c_cmd_t *dst)
{
    dst->saddr = src->saddr;
    dst->istail = src->istail;
    dst->recently_completed_timestamp = src->recently_completed_timestamp;
    dst->rxlen = src->rxlen;
    dst->txlen = src->txlen;
    dst->rxbuf = src->rxbuf;
    for (int i = 0; i < 5; i++) {
        dst->txbuf[i] = src->txbuf[i];
    }
}

uint16_t lld_i2c_cmdlist_size(lld_i2c_cmdlist_t *obj)
{
    return obj->size;
}

void lld_i2c_cmdlist_reset(lld_i2c_cmdlist_t *obj)
{
    obj->capacity = LLD_I2C_CMD_LIST_SIZE;
    obj->size = 0;
    obj->idx_in = 0;
    obj->idx_out = 0;
}

bool lld_i2c_cmdlist_read(lld_i2c_cmdlist_t *obj, lld_i2c_cmd_t *p, uint16_t len)
{
    if (obj->size < len) return false;
    
    for (int i = 0; i < len; i++) {
        lld_i2c_cmd_copy(&obj->buf[obj->idx_out], &p[i]);
        obj->idx_out++;
        if (obj->idx_out >= obj->capacity)
            obj->idx_out -= obj->capacity;
    }
    
    obj->size -= len;
    return true;
}

bool lld_i2c_cmdlist_write(lld_i2c_cmdlist_t *obj, const lld_i2c_cmd_t *p, uint16_t len)
{
    if ((obj->size + len) > obj->capacity) return false;
    
    for (int i = 0; i < len; i++) {
        lld_i2c_cmd_copy(&p[i], &obj->buf[obj->idx_in]);
        obj->idx_in++;
        if (obj->idx_in >= obj->capacity)
            obj->idx_in -= obj->capacity;
    }
    
    obj->size += len;
    return true;
}

static void lld_i2c_io(lld_i2c_t *obj, GPIOMode_TypeDef mode, GPIOOType_TypeDef otype, bool af, bool remap);
static bool lld_i2c_reset(lld_i2c_t *obj);

void lld_i2c_io(lld_i2c_t *obj, GPIOMode_TypeDef mode, GPIOOType_TypeDef otype, bool af, bool remap)
{
    if (remap && obj->i2cnum == 1) {
        uint8_t       i2caf[3]   = {GPIO_AF_I2C1, GPIO_AF_I2C2, GPIO_AF_I2C3};
        uint8_t       i2c_init_af = (af ? i2caf[obj->i2cnum - 1] : 0);
        lld_gpio_init(&obj->scl, GPIOB, 8, mode, otype, GPIO_PuPd_UP, GPIO_High_Speed, i2c_init_af);
        lld_gpio_init(&obj->sda, GPIOB, 9, mode, otype, GPIO_PuPd_UP, GPIO_High_Speed, i2c_init_af);
        
    } else {
        //			                I2C1	I2C2	I2C3
        GPIO_TypeDef *sclport[3] = {GPIOB,   GPIOB,    GPIOA};
        uint16_t      sclpin[3]  = {6,       10,        8  };
        GPIO_TypeDef *sdaport[3] = {GPIOB,   GPIOB,    GPIOC};
        uint16_t      sdapin[3]  = {7,       11,        9, };
        uint8_t       i2caf[3]   = {GPIO_AF_I2C1, GPIO_AF_I2C2, GPIO_AF_I2C3};
        uint8_t       i2c_init_af = (af ? i2caf[obj->i2cnum - 1] : 0);
        
        lld_gpio_init(&obj->scl, sclport[obj->i2cnum - 1], sclpin[obj->i2cnum - 1], mode, otype, GPIO_PuPd_UP, GPIO_High_Speed, i2c_init_af);
        lld_gpio_init(&obj->sda, sdaport[obj->i2cnum - 1], sdapin[obj->i2cnum - 1], mode, otype, GPIO_PuPd_UP, GPIO_High_Speed, i2c_init_af);
    }
}

bool lld_i2c_reset(lld_i2c_t *obj)
{
    uint16_t cnt = 0;
    
	//Reset I2C device and Rcc
	obj->i2c->CR1 |=  I2C_CR1_SWRST;   //Start reset I2c Controler -- soft reset
	obj->i2c->CR1 &=  ~I2C_CR1_SWRST;  //Stop  reset I2C
	obj->i2c->CR1 &=  ~I2C_CR1_PE;	    //DISABLE I2C
	RCC->APB1RSTR |= obj->rcc;      //Start reset I2C Clock
	RCC->APB1RSTR &= ~obj->rcc;	    //Stop  reset I2C Clock
	RCC->APB1ENR |=  obj->rcc;      //Enable I2C clock

	//Check and solve I2C Bus Busy problem
	while (I2C_GetFlagStatus(obj->i2c, I2C_FLAG_BUSY) && cnt < 20) {
		//Check Busy state and time out FLAG
		RCC_APB1PeriphClockCmd(obj->rcc, DISABLE);   //Close I2C Device;
        lld_i2c_io(obj, GPIO_Mode_OUT, GPIO_OType_PP, false, obj->remap); //Initialize GPIO for Software handle
        lld_gpio_set(&obj->scl, 0);
        lld_gpio_set(&obj->sda, 0);
		for (volatile uint16_t i = 0; i < 5; i++);   //Stop Signal by Pin simulation 
        lld_gpio_set(&obj->scl, 1);
		for (volatile uint16_t i = 0; i < 5; i++);
		lld_gpio_set(&obj->sda, 1);
		for (volatile uint16_t i = 0; i < 5; i++);
        lld_i2c_io(obj, GPIO_Mode_IN, GPIO_OType_OD, false, obj->remap);
		RCC->APB1RSTR |= obj->rcc;      //Start reset I2C Clock
		RCC->APB1RSTR &= ~obj->rcc;     //Stop  reset I2C Clock
		RCC->APB1ENR |=  obj->rcc;      //Enable I2C clock
		++cnt;              //Time out count, maximum 20 trys
	}

	if (cnt >= 20)		return false;
	return true;
}

bool lld_i2c_init(lld_i2c_t *obj, uint8_t i2c_num, uint32_t speed, bool remap)
{
    obj->speed = speed;
    obj->i2cnum = i2c_num;
    obj->state = I2C_STATE_NULL;
    obj->err_cnt = 0;
    obj->remap = remap;
    
    I2C_TypeDef *i2clist[3] = {I2C1, I2C2, I2C3};
    uint32_t i2c_rcclist[3] = {RCC_APB1Periph_I2C1, RCC_APB1Periph_I2C2, RCC_APB1Periph_I2C3};
    uint32_t i2c_eventirq_list[3] = {I2C1_EV_IRQn, I2C2_EV_IRQn, I2C3_EV_IRQn};
    uint32_t i2c_errorirq_list[3] = {I2C1_ER_IRQn, I2C2_ER_IRQn, I2C3_ER_IRQn};
    obj->i2c = i2clist[obj->i2cnum - 1];
    obj->rcc = i2c_rcclist[obj->i2cnum - 1];
    
    lld_i2c_io(obj, GPIO_Mode_AF, GPIO_OType_PP, true, obj->remap);
	if (!lld_i2c_reset(obj)) {
		obj->state = I2C_STATE_ERROR;
		return false;
	}
    lld_i2c_io(obj, GPIO_Mode_AF, GPIO_OType_OD, true, obj->remap);
    
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_ClockSpeed          = obj->speed;                     
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;                 
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;               
	I2C_InitStructure.I2C_OwnAddress1         = 0;                            
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;             
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  
	I2C_Init(obj->i2c, &I2C_InitStructure);
    
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 
	NVIC_InitStructure.NVIC_IRQChannel = i2c_eventirq_list[obj->i2cnum - 1];                  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   
	NVIC_Init(&NVIC_InitStructure);	

	NVIC_InitStructure.NVIC_IRQChannel = i2c_errorirq_list[obj->i2cnum - 1];                   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        
	NVIC_Init(&NVIC_InitStructure);	

	//Enable I2C IRQ AND Global Priority Group
	//Enable Event, Error, Buffer IRQ
	obj->i2c->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_IT_BUF);
	obj->state = I2C_STATE_FREE;
    mcu_i2c_list[obj->i2cnum - 1] = obj;
	return true;
}

bool lld_i2c_ishealthy(lld_i2c_t *obj) { return (obj->state != I2C_STATE_ERROR); }
bool lld_i2c_isfree(lld_i2c_t *obj) { return (obj->state != I2C_STATE_FREE); }
bool lld_i2c_waitfree(lld_i2c_t *obj)
{
	volatile uint32_t cnt = 0;
	//Wait untile I2C bus free or reach maximum time out count
	while (obj->state != I2C_STATE_FREE && ++cnt < 200000);
    return (obj->state == I2C_STATE_FREE);
}

bool lld_i2c_addcmd(lld_i2c_t *obj, uint8_t saddr, uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t rxlen, bool istail)
{
    lld_i2c_cmd_t p;
    lld_i2c_cmd_set(&p, saddr, txbuf, txlen, rxbuf, rxlen, istail);
    return lld_i2c_cmdlist_write(&obj->cmdlist, &p, 1);
}

bool lld_i2c_startnextcmd(lld_i2c_t *obj)
{
	if (obj->state == I2C_STATE_FREE || 
        obj->state == I2C_STATE_STOP) {
		//Check I2C is free or generate a Stop signal
		if (lld_i2c_cmdlist_read(&obj->cmdlist, &obj->currentcmd, 1)) {
			obj->state = I2C_STATE_START;		//generate start signal
			obj->dataidx = 0;
			if (obj->currentcmd.txlen > 0) {
				obj->i2c_direction = I2C_Direction_Transmitter;
			} else{
				obj->i2c_direction = I2C_Direction_Receiver;
			}
            I2C_AcknowledgeConfig(obj->i2c, ENABLE);
			I2C_GenerateSTART(obj->i2c, ENABLE);
			return true;
		}
		obj->state = I2C_STATE_FREE;
	}

	return false;  
}

bool lld_i2c_start(lld_i2c_t *obj)
{
	if (obj->state == I2C_STATE_FREE) {
		return lld_i2c_startnextcmd(obj);
	}														
	return false;
}


void lld_i2c_irq_error(lld_i2c_t *obj)
{
	//Reset I2C to avoid Error
	obj->i2c->CR1 |= I2C_CR1_SWRST;
	obj->i2c->CR1 &= ~I2C_CR1_SWRST;
	obj->state = I2C_STATE_ERROR;
}

void lld_i2c_irq_event(lld_i2c_t *obj)
{
	//Avoid Dead Loop						
	if (++obj->reset_cnt > 80) {
		obj->state = I2C_STATE_ERROR;		//I2C Error
		obj->i2c->CR1 |= I2C_CR1_SWRST;		//I2C start reset 
		obj->i2c->CR1 &= ~I2C_CR1_SWRST;    //I2C end reset
		obj->reset_cnt = 0;
	}
    
	//Event IRQ Handler
	switch (I2C_GetLastEvent(obj->i2c)) {
	case I2C_EVENT_MASTER_MODE_SELECT:
        {
            //EV5:Start signal finished
            obj->state = I2C_STATE_ADDR;    //next to send addr
            obj->dataidx = 0;       //Reset data index 
            //Send slave adress to activate slave
            I2C_Send7bitAddress(obj->i2c, obj->currentcmd.saddr, obj->i2c_direction);
            break;        
        }        
    case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
        {
            //EV6:Slave adress send complete/Tx
            obj->state = I2C_STATE_TXMOD;
            //Send a byte to slave
            I2C_SendData(obj->i2c, obj->currentcmd.txbuf[obj->dataidx++]);
            break;        
        }
    case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
        {
            //EV6:Slave adress send complete/Rx
            obj->state = I2C_STATE_RXMOD;
            if (obj->currentcmd.rxlen <= 1) {
                //Only one byte need to receive No Need to send a Ack
                I2C_AcknowledgeConfig(obj->i2c, DISABLE);
                I2C_GenerateSTOP(obj->i2c, ENABLE);
            }
            break;
        }
    case I2C_EVENT_MASTER_BYTE_RECEIVED:
        {
            //EV7: one byte received / RX
            obj->state = I2C_STATE_RX;
            obj->currentcmd.rxbuf[obj->dataidx++] = I2C_ReceiveData(obj->i2c);
            if ((obj->currentcmd.rxlen - obj->dataidx) == 1) { 
                I2C_AcknowledgeConfig(obj->i2c, DISABLE);
                I2C_GenerateSTOP(obj->i2c, ENABLE);
            }else if (obj->currentcmd.rxlen == obj->dataidx) {
                obj->state = I2C_STATE_STOP;
                obj->reset_cnt = 0;
                if(obj->currentcmd.istail) {
                    obj->currentcmd.recently_completed_timestamp = lld_kernel_get_time(0);
                }
                lld_i2c_startnextcmd(obj);
            }
            break;
        }
    case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
        {
            if (obj->dataidx < obj->currentcmd.txlen) {
                obj->state = I2C_STATE_TX;
                I2C_SendData(obj->i2c, obj->currentcmd.txbuf[obj->dataidx++]);
            } else {
                if (obj->currentcmd.rxlen > 0) {
                    //if need to receive data
                    obj->state = I2C_STATE_START;
                    obj->dataidx = 0;
                    obj->i2c_direction = I2C_Direction_Receiver;
                    I2C_GenerateSTART(obj->i2c, ENABLE);
                } else {
                    obj->state = I2C_STATE_STOP;
                    I2C_GenerateSTOP(obj->i2c, ENABLE);
                    obj->reset_cnt = 0;
                    if(obj->currentcmd.istail) {
                        obj->currentcmd.recently_completed_timestamp = lld_kernel_get_time(0);
                    }
                    lld_i2c_startnextcmd(obj);
                }
            }
            break; 
        }
	} // end switch
}



