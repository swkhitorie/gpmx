#include "drv_i2c.h"

static struct i2c_master_s *i2c_mlist[4] = {0, 0, 0, 0};

#if defined (DRV_BSP_H7)
static uint32_t _i2c_h7_set_clock(struct i2c_master_s *dev, uint32_t frequency, bool set);
#endif

static bool _i2c_pinconfig(struct i2c_master_s *dev);
static bool _i2c_pinsetup(struct i2c_master_s *dev, uint32_t mode);
static void _i2c_config(struct i2c_master_s *dev);
static void _i2c_setup(struct i2c_master_s *dev);
static int  _i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);
static void _i2c_completed_irq(struct i2c_master_s *dev);
static int  _i2c_eset(struct i2c_master_s *dev);

static int up_i2c_setup(struct i2c_master_s *dev);
static int up_i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);
static int up_i2c_reset(struct i2c_master_s *dev);
const struct i2c_ops_s g_i2c_master_ops = 
{
    .setup = up_i2c_setup,
    .transfer = up_i2c_transfer,
    .reset = up_i2c_reset,
};

/****************************************************************************
 * Private Function
 ****************************************************************************/

bool _i2c_pinconfig(struct i2c_master_s *dev)
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
		if ((I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, pin_scl)) != NULLPIN &&
			(I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, pin_sda)) != NULLPIN) {
			scl_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if ((I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, pin_scl)) != NULLPIN &&
			(I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, pin_sda)) != NULLPIN) {
			scl_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 3:
		if ((I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, pin_scl)) != NULLPIN &&
			(I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, pin_sda)) != NULLPIN) {
			scl_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, pin_scl);
			sda_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, pin_sda);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 4:
		if ((I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, pin_scl)) != NULLPIN &&
			(I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, pin_sda)) != NULLPIN) {
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
    GPIO_TypeDef *scl_port[2]  = {GPIOB,   GPIOB  };
    uint16_t      scl_pin[2]   = {6,       10,    };

    GPIO_TypeDef *sda_port[2]  = {GPIOB,   GPIOB  };
    uint16_t      sda_pin[2]   = {7,       11,    };

    if (num == 1 && pin_scl != 0) {
        scl_pin[0] = 8;  // PB8
        sda_pin[0] = 9;  // PB9
    }

    priv->scl_port = scl_port[num-1];
    priv->sda_port = sda_port[num-1];
    priv->scl_pin = scl_pin[num-1];
    priv->sda_pin = sda_pin[num-1];
    return true;
#endif
}

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
#if defined (DRV_BSP_H7)
uint32_t _i2c_h7_set_clock(struct i2c_master_s *dev, uint32_t frequency, bool set)
{
    uint8_t presc;
    uint8_t scl_delay;
    uint8_t sda_delay;
    uint8_t scl_h_period;
    uint8_t scl_l_period;
    struct up_i2c_master_s *priv = dev->priv;

    /*  The Speed and timing calculation are based on the following
       *  fI2CCLK = HSI and is 16Mhz
       *  Analog filter is on,
       *  Digital filter off
       *  Rise Time is 120 ns and fall is 10ns
       *  Mode is FastMode
    */

    if (set) {
        /* I2C peripheral must be disabled to update clocking configuration.
        * This will SW reset the device.
        */

        MODIFY_REG(priv->hi2c.Instance->CR1, I2C_CR1_PE, 0);
    }

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
    uint32_t timingr =
            (presc << 28) |
            (scl_delay << 20) |
            (sda_delay << 16) |
            (scl_h_period << 8) |
            (scl_l_period << 0);

    if (frequency != dev->cfg.frequency && set) {

        priv->hi2c.Instance->TIMINGR = timingr & (0xF0FFFFFFU);
        dev->cfg.frequency = frequency;
    }

    if (set) {
        /* Enable I2C peripheral */

        MODIFY_REG(priv->hi2c.Instance->CR1, 0, I2C_CR1_PE);
    }

    return timingr;
}
#endif

#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
void _i2c_set_clock(struct i2c_master_s *dev, uint32_t frequency)
{
    uint16_t cr1;
    uint16_t ccr;
    uint16_t trise;
    uint16_t freqmhz;
    uint16_t speed;
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    struct up_i2c_master_s *priv = dev->priv;

    if (frequency != dev->cfg.frequency) {

        /* Disable the selected I2C periphera */
        cr1 = priv->hi2c.Instance->CR1;
        priv->hi2c.Instance->CR1 = cr1 & ~I2C_CR1_PE;

        /* Update timing and control registers */
        freqmhz = (uint16_t)(pclk1 / 1000000);
        ccr = 0;

        if (frequency <= 100000) {
            /* Standard mode speed calculation */
            speed = (uint16_t)(pclk1 / (frequency << 1));

            /* The CCR fault must be >= 4 */
            if (speed < 4) {
                /* Set the minimum allowed value */
                speed = 4;
            }
            ccr |= speed;
            /* Set Maximum Rise Time for standard mode */
            trise = freqmhz + 1;
        } else {
            /* (frequency <= 400000) */
            /* Fast mode speed calculation with Tlow/Thigh = 16/9 */ 
            if (priv->hi2c.Init.DutyCycle == I2C_DUTYCYCLE_16_9) {
                /* Fast mode speed calculation with Tlow/Thigh = 2 */
                speed = (uint16_t)(pclk1 / (frequency * 3));
                /* Set fast speed bit */
                ccr |= I2C_CCR_FS;
            } else if (priv->hi2c.Init.DutyCycle == I2C_DUTYCYCLE_2) {
                speed = (uint16_t)(pclk1 / (frequency * 25));
                /* Set DUTY and fast speed bits */
                ccr |= (I2C_CCR_DUTY | I2C_CCR_FS);
            }
            /* Verify that the CCR speed value is nonzero */
            if (speed < 1) {
                /* Set the minimum allowed value */
                speed = 1;
            }
            ccr |= speed;
            /* Set Maximum Rise Time for fast mode */
            trise = (uint16_t)(((freqmhz * 300) / 1000) + 1);
        }

        /* Write the new values of the CCR and TRISE registers */
        priv->hi2c.Instance->CCR = ccr;
        priv->hi2c.Instance->TRISE = trise;

        /* Bit 14 of OAR1 must be configured and kept at 1 */
        priv->hi2c.Instance->OAR1 = (1<<14);

        /* Re-enable the peripheral (or not) */
        priv->hi2c.Instance->CR1 = cr1;

        /* Save the new I2C frequency */
        dev->cfg.frequency = frequency;
    }
}
#endif

bool _i2c_pinsetup(struct i2c_master_s *dev, uint32_t mode)
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

    LOW_PERIPH_INITPIN(scl_port, scl_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1]);
    LOW_PERIPH_INITPIN(sda_port, sda_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1]);

    return true;
}

void _i2c_config(struct i2c_master_s *dev)
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

    _i2c_pinconfig(dev);

    priv->hi2c.Instance 			    = i2cx[num-1];
#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
    priv->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    priv->hi2c.Init.ClockSpeed = dev->cfg.frequency;    //100k, 400k
#endif
#if defined (DRV_BSP_H7)
    timing = _i2c_h7_set_clock(dev, dev->cfg.frequency, false);
    priv->hi2c.Init.Timing  = timing;  //constant: 0x20B0CCFF(h7b0) 0x00901954(h743)
#endif
    priv->hi2c.Init.OwnAddress1      = 0;
    priv->hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    priv->hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    priv->hi2c.Init.OwnAddress2      = 0;
    priv->hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    priv->hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
}

void _i2c_setup(struct i2c_master_s *dev)
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

    _i2c_config(dev);
    _i2c_pinsetup(dev, IOMODE_AFOD);

    HAL_I2C_Init(&priv->hi2c);
#if defined (DRV_BSP_H7)
	HAL_I2CEx_AnalogFilter_Config(&priv->hi2c, I2C_ANALOGFILTER_ENABLE); 
#endif

	HAL_NVIC_SetPriority(i2c_err_irq[num-1], priv->priority_error, 0);
	HAL_NVIC_EnableIRQ(i2c_err_irq[num-1]);
	HAL_NVIC_SetPriority(i2c_event_irq[num-1], priv->priority_event, 0);
	HAL_NVIC_EnableIRQ(i2c_event_irq[num-1]);
}

int _i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count)
{
    int ret = 0x00;
    struct up_i2c_master_s *priv = dev->priv;
    struct i2c_msg_s *msg = msgs;
    struct i2c_msg_s *next_msg = 0;
    int num = count;
    uint32_t mode = 0;
    uint8_t next_flag = 0;
    uint32_t timeout;
    int i = 0;

    for (i = 0; i < (num - 1); i++) {
        mode = 0;
        msg = &msgs[i];
        next_msg = &msgs[i + 1];
        next_flag = next_msg->flags;
        timeout = msg->length/8 + 5;

        if (next_flag & I2C_M_NOSTART) {
            if ((next_flag & I2C_M_READ) == (msg->flags & I2C_M_READ)) { 
                /* The same mode, can use no start */
                mode = I2C_FIRST_AND_NEXT_FRAME;
            } else {
                mode = I2C_LAST_FRAME_NO_STOP;
            }
        } else {
            mode = I2C_LAST_FRAME_NO_STOP;
        }

        if (msg->flags & I2C_M_READ) {

            ret = HAL_I2C_Master_Seq_Receive_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
            if (ret != HAL_OK) {
                goto out;
            }
            if (i2c_dev_transfer_wait(dev, timeout) != GOK) {
                goto out;
            }
        } else {

            ret = HAL_I2C_Master_Seq_Transmit_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
            if (ret != HAL_OK) {
                goto out;
            }
            if (i2c_dev_transfer_wait(dev, timeout) != GOK) {
                goto out;
            }
        }
    }

    msg = &msgs[i];
    timeout = msg->length/8 + 5;

    if (msg->flags & I2C_M_NOSTOP) {
        mode = I2C_LAST_FRAME_NO_STOP;
    } else {
        mode = I2C_LAST_FRAME;
    }

    if (msg->flags & I2C_M_READ) {

        ret = HAL_I2C_Master_Seq_Receive_IT(&priv->hi2c,(msg->addr<<1), msg->buffer, msg->length, mode);
        if (ret != HAL_OK) {
            goto out;
        }
        if (i2c_dev_transfer_wait(dev, timeout) != GOK) {
            goto out;
        }
    } else {

        ret = HAL_I2C_Master_Seq_Transmit_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
        if (ret != HAL_OK) {
            goto out;
        }
        if (i2c_dev_transfer_wait(dev, timeout) != GOK) {
            goto out;
        }
    }
    ret = num;
    return ret;

out:
    if (priv->hi2c.ErrorCode == HAL_I2C_ERROR_AF) {
        printf("I2C NACK Error now stoped \r\n");
        /* Send stop signal to prevent bus lock-up */
#if defined(DRV_BSP_H7)
        priv->hi2c.Instance->CR1 |= I2C_IT_STOPI;
#endif
    }
    if (priv->hi2c.ErrorCode == HAL_I2C_ERROR_BERR) {
        printf("I2C BUS Error now stoped \r\n");
#if defined(DRV_BSP_H7)
        priv->hi2c.Instance->CR1 |= I2C_IT_STOPI;
#else
        priv->hi2c.Instance->CR1 |= I2C_CR1_STOP;
#endif
        ret=i-1;
    }
    return ret;
}

void _i2c_completed_irq(struct i2c_master_s *dev)
{
    i2c_dev_transfer_completed(dev);
}

int _i2c_reset(struct i2c_master_s *dev)
{
    return 0;
}

/****************************************************************************
 * Public Function Interface 
 ****************************************************************************/
int up_i2c_setup(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;
    uint8_t num = priv->id;

    _i2c_setup(dev);

    priv->ecnt = 0;
    i2c_mlist[num-1] = dev;
    return 0;
}

int up_i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count)
{
    int ret = 0;

    if (i2c_dev_lock(dev) != GOK) {
        return 1;
    }

    _i2c_transfer(dev, msgs, count);
    i2c_dev_unlock(dev);

    return ret;
}

int up_i2c_reset(struct i2c_master_s *dev)
{
    return _i2c_reset(dev);
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
    _i2c_completed_irq(i2c_mlist[idx]);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
    else if (hi2c->Instance == I2C4)	idx = 3;
#endif
    _i2c_completed_irq(i2c_mlist[idx]);
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
