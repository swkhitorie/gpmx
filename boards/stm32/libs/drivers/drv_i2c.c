#include "drv_i2c.h"

static struct i2c_master_s *g_i2c_list[CONFIG_STM32_I2C_NUM] = {0};

static uint8_t       _i2c_instance_judge(I2C_HandleTypeDef *hi2c);
static void          _i2c_rcc_init(uint8_t id);
static I2C_TypeDef*  _i2c_obj_get(uint8_t id);
static IRQn_Type     _i2c_event_irq_get(uint8_t id);
static IRQn_Type     _i2c_error_irq_get(uint8_t id);

#if defined(DRV_STM32_H7)
static void     _i2c_set_clocksrc(struct i2c_master_s *dev);
#endif
static void     _i2c_pin_cfg(struct i2c_master_s *dev, uint32_t mode);
static void     _i2c_set_clock(struct i2c_master_s *dev, uint32_t freq);
static void     _i2c_setup(struct i2c_master_s *dev);
static int      _i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count);
static void _i2c_completed_irq(struct i2c_master_s *dev);

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
uint8_t _i2c_instance_judge(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx;
    if      (hi2c->Instance == I2C1) idx = 0;
#if defined(I2C2)
    else if (hi2c->Instance == I2C2) idx = 1;
#endif
#if defined(I2C3)
    else if (hi2c->Instance == I2C3) idx = 2;
#endif
#if defined(I2C4)
    else if (hi2c->Instance == I2C4) idx = 3;
#endif
#if defined(I2C5)
    else if (hi2c->Instance == I2C5) idx = 4;
#endif

    return idx;
}

void _i2c_rcc_init(uint8_t id)
{
	switch (id) {
        case 1:	__HAL_RCC_I2C1_CLK_ENABLE();	break;
#if defined(I2C2)
        case 2:	__HAL_RCC_I2C2_CLK_ENABLE();	break;
#endif
#if defined(I2C3)
        case 3: __HAL_RCC_I2C3_CLK_ENABLE();  break;
#endif
#if defined(I2C4)
        case 4: __HAL_RCC_I2C4_CLK_ENABLE();  break;
#endif
#if defined(I2C5)
        case 5: __HAL_RCC_I2C5_CLK_ENABLE();  break;
#endif
    }
}

I2C_TypeDef* _i2c_obj_get(uint8_t id)
{
    I2C_TypeDef *i2c_array[CONFIG_STM32_I2C_NUM] = {
        I2C1, 
#if defined(I2C2)
        I2C2,
#endif
#if defined(I2C3)
        I2C3,
#endif
#if defined(I2C4)
        I2C4,
#endif
#if defined(I2C5)
        I2C5,
#endif
    };

    return i2c_array[id-1];
}

IRQn_Type _i2c_event_irq_get(uint8_t id)
{
	IRQn_Type i2c_event_irq_array[CONFIG_STM32_I2C_NUM] = {
        I2C1_EV_IRQn,
#if defined(I2C2)
        I2C2_EV_IRQn,
#endif
#if defined(I2C3)
        I2C3_EV_IRQn,
#endif
#if defined(I2C4)
        I2C4_EV_IRQn,
#endif
#if defined(I2C5)
        I2C5_EV_IRQn,
#endif
    };

    return i2c_event_irq_array[id - 1];
}

IRQn_Type _i2c_error_irq_get(uint8_t id)
{
	IRQn_Type i2c_error_irq_array[CONFIG_STM32_I2C_NUM] = {
        I2C1_ER_IRQn,
#if defined(I2C2)
        I2C2_ER_IRQn,
#endif
#if defined(I2C3)
        I2C3_ER_IRQn,
#endif
#if defined(I2C4)
        I2C4_ER_IRQn,
#endif
#if defined(I2C5)
        I2C5_ER_IRQn,
#endif
    };

    return i2c_error_irq_array[id - 1];
}

#if defined(DRV_STM32_H7)
static void _i2c_set_clocksrc(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;

    RCC_PeriphCLKInitTypeDef i2c_clk_init = {0};

    uint32_t i2c_clk[CONFIG_STM32_I2C_NUM] = {
        RCC_PERIPHCLK_I2C1, 
#if defined(I2C2)
        RCC_PERIPHCLK_I2C2,
#endif
#if defined(I2C3)
        RCC_PERIPHCLK_I2C3, 
#endif
#if defined(I2C4)
        RCC_PERIPHCLK_I2C4, 
#endif
#if defined(I2C5)
        RCC_PERIPHCLK_I2C5
#endif
    };
    uint32_t i2c_clk_src[CONFIG_STM32_I2C_NUM] = {
        RCC_I2C1CLKSOURCE_HSI, 
#if defined(I2C2)
        RCC_I2C2CLKSOURCE_HSI,
#endif
#if defined(I2C3)
        RCC_I2C3CLKSOURCE_HSI, 
#endif
#if defined(I2C4)
        RCC_I2C4CLKSOURCE_HSI, 
#endif
#if defined(I2C5)
        RCC_I2C5CLKSOURCE_HSI
#endif
    };

    /* h7 series use HSI/4 --> 16MHZ */
    __HAL_RCC_HSI_CONFIG(RCC_HSI_DIV4);

    i2c_clk_init.PeriphClockSelection = i2c_clk[priv->id-1];
    if (priv->id <= 3 || priv->id == 5) {
        i2c_clk_init.I2c123ClockSelection = i2c_clk_src[priv->id-1];
    } else if (priv->id == 4) {
        i2c_clk_init.I2c4ClockSelection   = i2c_clk_src[priv->id-1];
    }
    HAL_RCCEx_PeriphCLKConfig(&i2c_clk_init);
}
#endif

void _i2c_pin_cfg(struct i2c_master_s *dev, uint32_t mode)
{
    struct up_i2c_master_s *priv = dev->priv;
    struct periph_pin_t *sclpin = &priv->sclpin;
    struct periph_pin_t *sdapin = &priv->sdapin;

    LOW_PERIPH_INITPIN(sclpin->port, sclpin->pin, mode, IO_NOPULL, IO_SPEEDHIGH, sclpin->alternate);
    LOW_PERIPH_INITPIN(sdapin->port, sdapin->pin, mode, IO_NOPULL, IO_SPEEDHIGH, sdapin->alternate);
}

/************************************************************************************
 *   In Platform STM32H7:
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
void _i2c_set_clock(struct i2c_master_s *dev, uint32_t freq)
{
    struct up_i2c_master_s *priv = dev->priv;

#if defined (DRV_STM32_H7)
    uint8_t presc;
    uint8_t scl_delay;
    uint8_t sda_delay;
    uint8_t scl_h_period;
    uint8_t scl_l_period;

    /* I2C peripheral must be disabled to update clocking configuration.
     * This will SW reset the device.
     */

    MODIFY_REG(priv->hi2c.Instance->CR1, I2C_CR1_PE, 0);

    if (freq != priv->frequency) {

        /*  The Speed and timing calculation are based on the following
        *  fI2CCLK = HSI and is 16Mhz
        *  Analog filter is on,
        *  Digital filter off
        *  Rise Time is 120 ns and fall is 10ns
        *  Mode is FastMode
        */

        if (freq == 100000) {
            presc        = 0;
            scl_delay    = 5;
            sda_delay    = 0;
            scl_h_period = 61;
            scl_l_period = 89;

        } else if (freq == 400000) {
            presc        = 0;
            scl_delay    = 3;
            sda_delay    = 0;
            scl_h_period = 6;
            scl_l_period = 24;
        } else if (freq == 1000000) {
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
                (presc << I2C_TIMINGR_PRESC_Pos) |
                (scl_delay << I2C_TIMINGR_SCLDEL_Pos) |
                (sda_delay << I2C_TIMINGR_SDADEL_Pos) |
                (scl_h_period << I2C_TIMINGR_SCLH_Pos) |
                (scl_l_period << I2C_TIMINGR_SCLL_Pos);

        priv->hi2c.Instance->TIMINGR = timingr & (0xF0FFFFFFU);
        priv->frequency = freq;
    }

    /* Enable I2C peripheral */
    MODIFY_REG(priv->hi2c.Instance->CR1, 0, I2C_CR1_PE);
#endif


#if defined(DRV_STM32_F1) || defined(DRV_STM32_F4)
    uint16_t cr1;
    uint16_t ccr;
    uint16_t trise;
    uint16_t freqmhz;
    uint16_t speed;
    uint32_t pclk1 = priv->clock;

    if (freq != priv->frequency) {
        /* Disable the selected I2C periphera */
        cr1 = priv->hi2c.Instance->CR1;
        priv->hi2c.Instance->CR1 = cr1 & ~I2C_CR1_PE;

        /* Update timing and control registers */
        freqmhz = (uint16_t)(pclk1 / 1000000);
        ccr = 0;

        if (freq <= 100000) {
            /* Standard mode speed calculation */
            speed = (uint16_t)(pclk1 / (freq << 1));

            /* The CCR fault must be >= 4 */
            if (speed < 4) {
                /* Set the minimum allowed value */
                speed = 4;
            }
            ccr |= speed;
            /* Set Maximum Rise Time for standard mode */
            trise = freqmhz + 1;
        } else {
            /* (freq <= 400000) */
            if (priv->hi2c.Init.DutyCycle == I2C_DUTYCYCLE_16_9) {
                /* Fast mode speed calculation with Tlow/Thigh = 16/9 */ 
                speed = (uint16_t)(pclk1 / (freq * 25));

                /* Set fast speed bit */
                ccr |= (I2C_CCR_DUTY | I2C_CCR_FS);
            } else if (priv->hi2c.Init.DutyCycle == I2C_DUTYCYCLE_2) {

                /* Fast mode speed calculation with Tlow/Thigh = 2 */
                speed = (uint16_t)(pclk1 / (freq * 3));

                /* Set DUTY and fast speed bits */
                ccr |= I2C_CCR_FS;
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
        priv->frequency = freq;
    }
#endif
}

void _i2c_setup(struct i2c_master_s *dev)
{
    struct up_i2c_master_s *priv = dev->priv;

    priv->clock = HAL_RCC_GetPCLK1Freq();
    priv->hi2c.Instance              = _i2c_obj_get(priv->id);
    priv->hi2c.Init.OwnAddress1      = 0;
    priv->hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    priv->hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    priv->hi2c.Init.OwnAddress2      = 0;
    priv->hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    priv->hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
#if defined (DRV_STM32_F1) || defined (DRV_STM32_F4)
    priv->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    priv->hi2c.Init.ClockSpeed = 100000;
#endif
#if defined (DRV_STM32_H7)
    priv->hi2c.Init.Timing = 0x00503D59;   // HSI/4->16MHz, default 100KHz
#endif
    HAL_I2C_Init(&priv->hi2c);
#if defined (DRV_STM32_H7)
	HAL_I2CEx_AnalogFilter_Config(&priv->hi2c, I2C_ANALOGFILTER_ENABLE); 
#endif
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

            _i2c_set_clock(dev, msg->frequency);
            ret = HAL_I2C_Master_Seq_Receive_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
            if (ret != HAL_OK) {
                goto out;
            }
            if (i2c_dev_transfer_wait(dev, timeout) != DTRUE) {
                goto out;
            }
        } else {

            _i2c_set_clock(dev, msg->frequency);
            ret = HAL_I2C_Master_Seq_Transmit_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
            if (ret != HAL_OK) {
                goto out;
            }
            if (i2c_dev_transfer_wait(dev, timeout) != DTRUE) {
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

        _i2c_set_clock(dev, msg->frequency);
        ret = HAL_I2C_Master_Seq_Receive_IT(&priv->hi2c,(msg->addr<<1), msg->buffer, msg->length, mode);
        if (ret != HAL_OK) {
            goto out;
        }
        if (i2c_dev_transfer_wait(dev, timeout) != DTRUE) {
            goto out;
        }
    } else {

        _i2c_set_clock(dev, msg->frequency);
        ret = HAL_I2C_Master_Seq_Transmit_IT(&priv->hi2c, (msg->addr<<1), msg->buffer, msg->length, mode);
        if (ret != HAL_OK) {
            goto out;
        }
        if (i2c_dev_transfer_wait(dev, timeout) != DTRUE) {
            goto out;
        }
    }
    ret = num;
    return ret;

out:
    if (priv->hi2c.ErrorCode == HAL_I2C_ERROR_AF) {
        printf("I2C NACK Error now stoped \r\n");
        /* Send stop signal to prevent bus lock-up */
#if defined(DRV_STM32_H7)
        priv->hi2c.Instance->CR1 |= I2C_IT_STOPI;
#endif
    }
    if (priv->hi2c.ErrorCode == HAL_I2C_ERROR_BERR) {
        printf("I2C BUS Error now stoped \r\n");
#if defined(DRV_STM32_H7)
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

#if defined (DRV_STM32_H7)
	_i2c_set_clocksrc(dev); 
#endif

    _i2c_rcc_init(priv->id);

    _i2c_pin_cfg(dev, IOMODE_AFOD);

    _i2c_setup(dev);

	HAL_NVIC_SetPriority(_i2c_event_irq_get(priv->id), priv->priority, 0);
	HAL_NVIC_EnableIRQ(_i2c_event_irq_get(priv->id));

	HAL_NVIC_SetPriority(_i2c_error_irq_get(priv->id), priv->priority_error, 0);
	HAL_NVIC_EnableIRQ(_i2c_error_irq_get(priv->id));

    priv->state = 0;

    g_i2c_list[priv->id - 1] = dev;

    return 0;
}

int up_i2c_transfer(struct i2c_master_s *dev, struct i2c_msg_s *msgs, int count)
{
    int ret = 0;

    if (i2c_dev_lock(dev) != DTRUE) {
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
 * STM32 HAL Callback 
 ****************************************************************************/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    idx = _i2c_instance_judge(hi2c);
    _i2c_completed_irq(g_i2c_list[idx]);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t idx = 0;
    idx = _i2c_instance_judge(hi2c);
    _i2c_completed_irq(g_i2c_list[idx]);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    struct up_i2c_master_s *priv;
    uint8_t idx = _i2c_instance_judge(hi2c);
    priv = g_i2c_list[idx]->priv;

    priv->state++;
}

/****************************************************************************
 * STM32 IRQ
 ****************************************************************************/
void I2C1_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[0]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}
void I2C1_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[0]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}

#if defined(I2C2)
void I2C2_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[1]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}
void I2C2_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[1]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}
#endif

#if defined(I2C3)
void I2C3_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[2]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}
void I2C3_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[2]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}
#endif

#if defined(I2C4)
void I2C4_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[3]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}
void I2C4_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[3]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}
#endif

#if defined(I2C5)
void I2C5_EV_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[4]->priv;
    HAL_I2C_EV_IRQHandler(&priv->hi2c);
}
void I2C5_ER_IRQHandler(void)
{
    struct up_i2c_master_s *priv = g_i2c_list[4]->priv;
    HAL_I2C_ER_IRQHandler(&priv->hi2c);
}
#endif

