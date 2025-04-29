#include "drv_i2c.h"

static struct i2c_master_s *i2c_mlist[4] = {0, 0, 0, 0};

static uint32_t low_h7_calculate_timing(uint32_t frequency);
static bool low_pinconfig(struct i2c_master_s *dev);
static bool low_pinsetup(struct i2c_master_s *dev, uint32_t mode);
static void low_config(struct i2c_master_s *dev);
static void low_setup(struct i2c_master_s *dev);
static int  low_transfer(struct i2c_master_s *dev);
static int  low_transfer_it(struct i2c_master_s *dev);
static void low_completed_irq(struct i2c_master_s *dev);
static int low_reset(struct i2c_master_s *dev);

static int up_setup(struct i2c_master_s *dev);
static int up_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);
static int up_transferit(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);
static int up_reset(struct i2c_master_s *dev);
const struct i2c_ops_s g_i2c_master_ops = 
{
    .setup = up_setup,
    .reset = up_reset,
    .transfer = up_transfer,
    .transferit = up_transferit,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/

/************************************************************************************
 *   This function supports bus clock frequencies of:
 *
 *      1000Khz (Fast Mode+)
 *      400Khz  (Fast Mode)
 *      100Khz  (Standard Mode)
 *      10Khz   (Standard Mode)
 *
 *    PCLK1 >------|\   I2CCLK
 *   SYSCLK >------| |--------->
 *      HSI >------|/
 *
 *   HSI is the default and is always 16Mhz.
 ************************************************************************************/
uint32_t low_h7_calculate_timing(uint32_t frequency) {
    uint8_t presc;
    uint8_t scl_delay;
    uint8_t sda_delay;
    uint8_t scl_h_period;
    uint8_t scl_l_period;

    /*  The Speed and timing calculation are based on the following
       *  fI2CCLK = HSI and is 16Mhz
       *  Analog filter is on,
       *  Digital filter off
       *  Rise Time is 120 ns and fall is 10ns
       *  Mode is FastMode
    */

    if (frequency == 100000) {
        presc        = 0;
        scl_delay    = 5;
        sda_delay    = 0;
        scl_h_period = 61;
        scl_l_period = 89;
    } else if (frequency == 400000) {
        presc        = 0;
        scl_delay    = 3;
        sda_delay    = 0;
        scl_h_period = 6;
        scl_l_period = 24;
    } else if (frequency == 1000000) {
        presc        = 0;
        scl_delay    = 2;
        sda_delay    = 0;
        scl_h_period = 1;
        scl_l_period = 5;
    } else {
        presc        = 7;
        scl_delay    = 0;
        sda_delay    = 0;
        scl_h_period = 35;
        scl_l_period = 162;
    }
    //503D59
    uint32_t timingr =
            (presc << 28) |
            (scl_delay << 20) |
            (sda_delay << 16) |
            (scl_h_period << 8) |
            (scl_l_period << 0);
    return timingr;
}

bool low_pinconfig(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;
    uint8_t pin_scl = priv->pin_scl;
    uint8_t pin_sda = priv->pin_sda;

#if defined (DRV_BSP_H7)
	const struct pin_node *scl_node;
	const struct pin_node *sda_node;
	uint32_t illegal;

	switch (num) {
	case 1:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, pin_scl)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, pin_sda)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, pin_scl)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, pin_sda)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, pin_scl)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, pin_sda)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, pin_scl)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, pin_sda)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}

	if (illegal != 0) {
        priv->scl_port = scl_node->port;
        priv->sda_port = sda_node->port;
        priv->scl_pin = scl_node->pin;
        priv->sda_pin = sda_node->pin;
	}else {
		return false;
	}
	return true;
#endif
#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
    GPIO_TypeDef *scl_port[3]  = {GPIOB,   GPIOB,    GPIOA};
    uint16_t      scl_pin[3]   = {6,       10,        8  };

    GPIO_TypeDef *sda_port[3]  = {GPIOB,   GPIOB,    GPIOC};
    uint16_t      sda_pin[3]   = {7,       11,        9, };

    priv->scl_port = scl_port[num-1];
    priv->sda_port = sda_port[num-1];
    priv->scl_pin = scl_pin[num-1];
    priv->sda_pin = sda_pin[num-1];
    return true;
#endif
}

bool low_pinsetup(struct i2c_master_s *dev, uint32_t mode)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;
    uint8_t scl_pin = priv->scl_pin;
    uint8_t sda_pin = priv->sda_pin;
    GPIO_TypeDef *scl_port = priv->scl_port;
    GPIO_TypeDef *sda_port = priv->sda_port;
    uint32_t i2c_af[4]    = {
        GPIO_AF4_I2C1, GPIO_AF4_I2C2, GPIO_AF4_I2C3, 
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        GPIO_AF4_I2C4,
#endif
    };

    low_gpio_setup(scl_port, scl_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1], NULL, 0);
    low_gpio_setup(sda_port, sda_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1], NULL, 0);

    return true;
}

void low_config(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;
    uint32_t timing = 0;
	I2C_TypeDef *i2cx[4] = {
        I2C1, I2C2, I2C3,
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        I2C4
#endif
    };
#if defined (DRV_BSP_H7)
    /* h7 series use HSI/4 --> 16MHZ */
    __HAL_RCC_HSI_CONFIG(RCC_HSI_DIV4);
    uint32_t i2c_clk[4] = {
        RCC_PERIPHCLK_I2C1, RCC_PERIPHCLK_I2C2, 
        RCC_PERIPHCLK_I2C3, RCC_PERIPHCLK_I2C4,
    };
    uint32_t i2c_clk_src[4] = {
        RCC_I2C1CLKSOURCE_HSI, RCC_I2C2CLKSOURCE_HSI,
        RCC_I2C3CLKSOURCE_HSI, RCC_I2C4CLKSOURCE_HSI
    };
    RCC_PeriphCLKInitTypeDef i2c_clk_init = {0};
    i2c_clk_init.PeriphClockSelection = i2c_clk[num-1];
    if (num <= 3)      i2c_clk_init.I2c123ClockSelection = i2c_clk_src[num-1];
    else if (num == 4) i2c_clk_init.I2c4ClockSelection   = i2c_clk_src[num-1];
    HAL_RCCEx_PeriphCLKConfig(&i2c_clk_init);
#endif

    low_pinconfig(dev);

    priv->hi2c.Instance 			    = i2cx[num-1];
#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
    priv->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    priv->hi2c.Init.ClockSpeed = dev->clk_speed;    //100k, 400k
#endif
#if defined (DRV_BSP_H7)
    timing = low_h7_calculate_timing(dev->clk_speed);
    priv->hi2c.Init.Timing  = timing;  //constant: 0x20B0CCFF(h7b0) 0x00901954(h743)
#endif
    priv->hi2c.Init.OwnAddress1      = 0;
    priv->hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    priv->hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    priv->hi2c.Init.OwnAddress2      = 0;
    priv->hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    priv->hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
}

void low_setup(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;

	IRQn_Type i2c_err_irq[4] = {
        I2C1_ER_IRQn, I2C2_ER_IRQn, I2C3_ER_IRQn,
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        I2C4_ER_IRQn,
#endif
    };
	IRQn_Type i2c_event_irq[4] = {
        I2C1_EV_IRQn, I2C2_EV_IRQn, I2C3_EV_IRQn,
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        I2C4_EV_IRQn,
#endif
    };

	switch (num) {
	case 1: __HAL_RCC_I2C1_CLK_ENABLE(); break;
	case 2: __HAL_RCC_I2C2_CLK_ENABLE(); break;
	case 3: __HAL_RCC_I2C3_CLK_ENABLE(); break;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
	case 4: __HAL_RCC_I2C4_CLK_ENABLE(); break; 
#endif
	default: break;
	}

    low_config(dev);
    low_pinsetup(dev, IOMODE_AFOD);

    HAL_I2C_Init(&priv->hi2c);
#if defined (DRV_BSP_H7)
	HAL_I2CEx_AnalogFilter_Config(&priv->hi2c, I2C_ANALOGFILTER_ENABLE); 
#endif

	HAL_NVIC_SetPriority(i2c_err_irq[num-1], priv->priority_error, 0);
	HAL_NVIC_EnableIRQ(i2c_err_irq[num-1]);
	HAL_NVIC_SetPriority(i2c_event_irq[num-1], priv->priority_event, 0);
	HAL_NVIC_EnableIRQ(i2c_event_irq[num-1]);
}

int low_transfer(struct i2c_master_s *dev)
{
    int ret = 0xff;
    struct up_i2c_master_s *priv = dev->priv;
    dev->msgi = 0;

    for (; dev->msgi < dev->msgc; dev->msgi++) {
        struct i2c_msg_s *msg = &dev->msgv[dev->msgi];
        switch (msg->flags) {
        case I2C_M_WRITE:
            ret = HAL_I2C_Master_Transmit(&priv->hi2c, msg->addr, &msg->xbuffer[0], msg->xlength, 1000);
            break;
        case I2C_M_READ:
            ret = HAL_I2C_Master_Receive(&priv->hi2c, msg->addr, &msg->rbuffer[0], msg->rlength, 1000);
            break;
        case I2C_REG_WRITE:
            ret = HAL_I2C_Mem_Write(&priv->hi2c, msg->addr, msg->xbuffer[0], msg->reg_sz, 
                                &msg->xbuffer[msg->reg_sz], msg->xlength - msg->reg_sz, 1000);
            break;
        case I2C_REG_READ:
            ret = HAL_I2C_Mem_Read(&priv->hi2c, msg->addr, msg->xbuffer[0], msg->reg_sz, 
                                &msg->rbuffer[0], msg->rlength, 1000);
            break;
        }
        if (ret != HAL_OK) {
            priv->ts_ecnt++;
        }
    }
    return ret;
}

int low_transfer_it(struct i2c_master_s *dev)
{
    int ret = 0xff;
    struct up_i2c_master_s *priv = dev->priv;
    if (dev->msgi < dev->msgc && dev->msgc != 0) {
        struct i2c_msg_s *msg = &dev->msgv[dev->msgi];
        dev->msgi++;

        switch (msg->flags) {
        case I2C_M_WRITE:
            ret = HAL_I2C_Master_Transmit_IT(&priv->hi2c, msg->addr, &msg->xbuffer[0], msg->xlength);
            break;
        case I2C_M_READ:
            ret = HAL_I2C_Master_Receive_IT(&priv->hi2c, msg->addr, &msg->rbuffer[0], msg->rlength);
            break;
        case I2C_REG_WRITE:
            ret = HAL_I2C_Mem_Write_IT(&priv->hi2c, msg->addr, msg->xbuffer[0], msg->reg_sz, 
                                &msg->xbuffer[msg->reg_sz], msg->xlength - msg->reg_sz);
            break;
        case I2C_REG_READ:
            ret = HAL_I2C_Mem_Read_IT(&priv->hi2c, msg->addr, msg->xbuffer[0], msg->reg_sz, 
                                &msg->rbuffer[0], msg->rlength);
            break;
        }
        if (ret != HAL_OK) {
            priv->ts_ecnt++;
        }
    }
    return ret;
}

void low_completed_irq(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;

    if (dev->msgi == dev->msgc) {
#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
        xSemaphoreGive(dev->sem_isr);
#endif
    } else if (HAL_I2C_GetState(&priv->hi2c) == HAL_I2C_STATE_READY) {
        low_transfer_it(dev);
    }
}

int low_reset(struct i2c_master_s *dev)
{
    return 0;
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_setup(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;

    low_setup(dev);

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
    i2c_sem_init(dev);
#endif

    dev->msgc = 0;
    dev->msgi = 0;
    dev->msgv = NULL;
    priv->state = 0;
    priv->ecnt = 0;
    priv->ts_ecnt = 0;
    i2c_mlist[num-1] = dev;
    return 0;
}

int up_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count)
{
    int ret = 0;
    int ret2 = 0, timeout = 0;
    if (count == 0) return 0;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
    i2c_sem_wait(dev);
#endif
    dev->msgv = msgs;
    dev->msgc = count;
    dev->msgi = 0;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
    ret = low_transfer_it(dev);

    do {
        if (xSemaphoreTake(dev->sem_isr, 1000) == pdTRUE) {
            break;
        } else {
            timeout++;
        }
    } while (dev->msgi < dev->msgc && timeout < 10);

    if (dev->msgi < dev->msgc) {
        ret = 0xff;
    }
#else
    ret = low_transfer(dev);
#endif

#if defined(CONFIG_BOARD_FREERTOS_ENABLE) && defined(CONFIG_I2C_TASKSYNC)
    i2c_sem_post(dev);
#endif

    return ret;
}

int up_transferit(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count)
{
    int i = 0;
    int timeout = 1000;
    int timei;

    for (timei = 0; timei < timeout; i++) {
        if (dev->msgc == 0) {
            break;
        }

        if (dev->msgc != 0 && dev->msgc == dev->msgi) {
            break;
        }
    }

    if (timei == timeout) {
        return -1;
    }

    dev->msgv = msgs;
    dev->msgc = count;
    dev->msgi = 0;
    return low_transfer_it(dev);
}

int up_reset(struct i2c_master_s *dev)
{
    return low_reset(dev);
}

/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
    else if (hi2c->Instance == I2C4)	idx = 3;
#endif
    low_completed_irq(i2c_mlist[idx]);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
    else if (hi2c->Instance == I2C4)	idx = 3;
#endif
    low_completed_irq(i2c_mlist[idx]);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
    else if (hi2c->Instance == I2C4)	idx = 3;
#endif

    low_completed_irq(i2c_mlist[idx]);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    struct up_i2c_master_s *priv;
    uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
    else if (hi2c->Instance == I2C4)	idx = 3;
#endif

    priv = i2c_mlist[idx]->priv;
    priv->ecnt++;
}

void I2C1_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[0]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}

void I2C1_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[0]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}

void I2C2_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[1]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}

void I2C2_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[1]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}

void I2C3_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[2]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}

void I2C3_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[2]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}

void I2C4_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[3]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}

void I2C4_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = i2c_mlist[3]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}
