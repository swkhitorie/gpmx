#include "drv_i2c.h"

struct drv_i2c_t *drv_i2c_list[4] = {0, 0, 0, 0};

static void drv_i2c_gpio_init(struct drv_i2c_t *obj, uint32_t mode)
{
    uint8_t num = obj->num;
    uint8_t scl_pin = obj->scl_pin;
    uint8_t sda_pin = obj->sda_pin;
    GPIO_TypeDef *scl_port = obj->scl_port;
    GPIO_TypeDef *sda_port = obj->sda_port;
    uint32_t i2c_af[4]    = {
        GPIO_AF4_I2C1, GPIO_AF4_I2C2, GPIO_AF4_I2C3, 
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        GPIO_AF4_I2C4,
#endif
    };

    // printf("%d %d %d\r\n", obj->scl_pin, obj->sda_pin, obj->num);
    // why here must delay???
    HAL_Delay(1);
    drv_gpio_init(scl_port, scl_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1], NULL);
    drv_gpio_init(sda_port, sda_pin, mode, IO_NOPULL, IO_SPEEDHIGH, i2c_af[num-1], NULL);
}

static void drv_i2c_gpio_config(struct drv_i2c_t *obj, uint8_t scls, uint8_t sdas)
{
#if defined (DRV_BSP_H7)
	const struct pin_node *scl_node;
	const struct pin_node *sda_node;
	uint32_t illegal;

	switch (obj->num) {
	case 1:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, scls)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, sdas)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, scls);
			sda_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, sdas);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, scls)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, sdas)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, scls);
			sda_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, sdas);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, scls)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, sdas)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, scls);
			sda_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, sdas);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, scls)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, sdas)) != NULL) {
			scl_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, scls);
			sda_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, sdas);
			illegal = scl_node->port && sda_node->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}

	if (illegal != 0) {
        obj->scl_port = scl_node->port;
        obj->sda_port = sda_node->port;
        obj->scl_pin = scl_node->pin;
        obj->sda_pin = sda_node->pin;
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

    obj->scl_port = scl_port[obj->num-1];
    obj->sda_port = sda_port[obj->num-1];
    obj->scl_pin = scl_pin[obj->num-1];
    obj->sda_pin = sda_pin[obj->num-1];
    return true;
#endif
}

static void drv_i2c_blockwait(struct drv_i2c_t *obj, uint16_t addr)
{
	while (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY);
	while (HAL_I2C_IsDeviceReady(&obj->hi2c, addr, 1000, 1000) == HAL_TIMEOUT);
	while (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY);
}

static bool drv_i2c_cmdlist_put(struct drv_i2c_t *obj, struct drv_i2c_reg_cmd *cmd)
{
    if (obj->cmdlist.size == obj->cmdlist.capacity)
        return false;
    obj->cmdlist.buf[obj->cmdlist.in] = *cmd;
    obj->cmdlist.in++;
    if (obj->cmdlist.in == obj->cmdlist.capacity)
        obj->cmdlist.in -= obj->cmdlist.capacity;
    obj->cmdlist.size++;
}

static bool drv_i2c_cmdlist_get(struct drv_i2c_t *obj, struct drv_i2c_reg_cmd *cmd)
{
    if (obj->cmdlist.size <= 0)
        return false;
    *cmd = obj->cmdlist.buf[obj->cmdlist.out];
    obj->cmdlist.out++;
    if (obj->cmdlist.out == obj->cmdlist.capacity)
        obj->cmdlist.out -= obj->cmdlist.capacity;
    obj->cmdlist.size--;
}

void drv_i2c_attr_config(struct drv_i2c_attr_t *obj, uint32_t speed, uint8_t pevent, uint32_t perror)
{
    obj->speed = speed;
    obj->priority_event = pevent;
    obj->priority_error = perror;
}

void drv_i2c_cmdlist_config(struct drv_i2c_reg_cmdlist *obj, struct drv_i2c_reg_cmd *buf, uint8_t capacity)
{
    obj->capacity = capacity;
    obj->buf = buf;
    obj->size = 0;
    obj->in = obj->out = 0;
}

void drv_i2c_config(uint8_t num, uint8_t scls, uint8_t sdas, struct drv_i2c_t *obj, 
            struct drv_i2c_attr_t *attr, struct drv_i2c_reg_cmdlist *list)
{
	I2C_TypeDef *i2cx[4] = {
        I2C1, I2C2, I2C3,
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
        I2C4
#endif
    };

    obj->num = num;
    obj->attr = *attr;
    obj->cmdlist = *list;
    drv_i2c_gpio_config(obj, scls, sdas);

    obj->hi2c.Instance 			    = i2cx[num-1];
#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
    obj->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    obj->hi2c.Init.ClockSpeed = attr->speed;    //100k, 400k
#endif
#if defined (DRV_BSP_H7)
    obj->hi2c.Init.Timing  = 0x00901954;
#endif
    obj->hi2c.Init.OwnAddress1      = 0;
    obj->hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    obj->hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    obj->hi2c.Init.OwnAddress2      = 0;
    obj->hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    obj->hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
}

void drv_i2c_init(struct drv_i2c_t *obj)
{
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

    obj->i2c_error_cnt = 0;
    obj->state = 0;

	switch (obj->num) {
	case 1: __HAL_RCC_I2C1_CLK_ENABLE(); break;
	case 2: __HAL_RCC_I2C2_CLK_ENABLE(); break;
	case 3: __HAL_RCC_I2C3_CLK_ENABLE(); break;
#if (BSP_CHIP_RESOURCE_LEVEL > 4)
	case 4: __HAL_RCC_I2C4_CLK_ENABLE(); break; 
#endif
	default: break;
	}

    drv_i2c_gpio_init(obj, IOMODE_AFOD);

    HAL_I2C_Init(&obj->hi2c);
#if defined (DRV_BSP_H7)
	HAL_I2CEx_AnalogFilter_Config(&obj->hi2c, I2C_ANALOGFILTER_ENABLE); 
#endif

	HAL_NVIC_SetPriority(i2c_err_irq[obj->num-1], obj->attr.priority_error, 0);
	HAL_NVIC_EnableIRQ(i2c_err_irq[obj->num-1]);
	HAL_NVIC_SetPriority(i2c_event_irq[obj->num-1], obj->attr.priority_event, 0);
	HAL_NVIC_EnableIRQ(i2c_event_irq[obj->num-1]);
    drv_i2c_list[obj->num - 1] = obj;
    return true;
}

int drv_i2c_reg_write(struct drv_i2c_t *obj, uint16_t slave, uint16_t reg, 
                        uint16_t reg_addr_type, uint8_t *p, uint16_t len, enum __drv_rwway way)
{
	int ret = HAL_OK;
    struct drv_i2c_reg_cmd cmd;
    uint8_t in_len;
    uint8_t idx;

	switch (way) {
	case RWPOLL:
		ret = HAL_I2C_Mem_Write(&obj->hi2c, slave, reg, reg_addr_type, p, len, 1000);
        //drv_i2c_blockwait(&obj->hi2c, slave);
		break;
	case RWIT:
    case RWDMA:
        in_len = (len >= DRV_I2C_REG_CMD_DATA_OUT_LEN)?DRV_I2C_REG_CMD_DATA_OUT_LEN:len;
        cmd.slv_addr = slave;
        cmd.reg_addr = reg;
        cmd.reg_addr_type = reg_addr_type;
        cmd.data_outlen = in_len;
        for (idx = 0; idx < in_len; idx++) {
            cmd.data_out[idx] = p[idx];
        }
        if (obj->isbusysend) {
            drv_i2c_cmdlist_put(obj, &cmd);
            return 0x04;
        }
        obj->isbusysend = true;
        //if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY) return HAL_ERROR;
		ret = HAL_I2C_Mem_Write_IT(&obj->hi2c, slave, reg, reg_addr_type, p, len);
		break;
	default: break;
	}
	return ret;
}

int drv_i2c_reg_read(struct drv_i2c_t *obj, uint16_t slave, uint16_t reg, 
                        uint16_t reg_addr_type, uint8_t *p, uint16_t len, enum __drv_rwway way)
{
	int ret = HAL_OK;
    struct drv_i2c_reg_cmd cmd;
    uint8_t idx;

	switch (way) {
	case RWPOLL:
		ret = HAL_I2C_Mem_Read(&obj->hi2c, slave, reg, reg_addr_type, p, len, 1000);
        //drv_i2c_blockwait(&obj->hi2c, slave);
		break;
	case RWIT:
    case RWDMA:
        cmd.slv_addr = slave;
        cmd.reg_addr = reg;
        cmd.reg_addr_type = reg_addr_type;
        cmd.data_inlen = len;
        cmd.pdatain = p;
        if (obj->isbusysend) {
            drv_i2c_cmdlist_put(obj, &cmd);
            return 0x04;
        }
        obj->isbusysend = true;
        //if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY) return HAL_ERROR;
		ret = HAL_I2C_Mem_Read_IT(&obj->hi2c, slave, reg, reg_addr_type, p, len);
		break;
	default: break;
	}
	return ret;
}

int drv_i2c_transfer(struct drv_i2c_t *obj, uint16_t slave, uint8_t *p, uint16_t len,
                        enum __drv_rwway way)
{
	int ret = HAL_OK;
    struct drv_i2c_reg_cmd cmd;
    uint8_t in_len;
    uint8_t idx;

	switch (way) {
	case RWPOLL:
        ret = HAL_I2C_Master_Transmit(&obj->hi2c, slave, p, len, 1000);
        drv_i2c_blockwait(&obj->hi2c, slave);
		break;
	case RWIT:
    case RWDMA:
        in_len = (len >= DRV_I2C_REG_CMD_DATA_OUT_LEN)?DRV_I2C_REG_CMD_DATA_OUT_LEN:len;
        cmd.slv_addr = slave;
        cmd.data_outlen = in_len;
        for (idx = 0; idx < in_len; idx++) {
            cmd.data_out[idx] = p[idx];
        }
        if (obj->isbusysend) {
            drv_i2c_cmdlist_put(obj, &cmd);
            return 0x04;
        }
        obj->isbusysend = true;
        // if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY) return HAL_ERROR;
		ret = HAL_I2C_Master_Transmit_IT(&obj->hi2c, slave, p, len);
		break;
	default: break;
	}
	return ret;	
}

int drv_i2c_receive(struct drv_i2c_t *obj, uint16_t slave, uint8_t *p, uint16_t len,
                        enum __drv_rwway way)
{
	int ret = HAL_OK;
    struct drv_i2c_reg_cmd cmd;
    uint8_t idx;

	switch (way) {
	case RWPOLL:
        ret = HAL_I2C_Master_Receive(&obj->hi2c, slave, p, len, 1000);
        drv_i2c_blockwait(&obj->hi2c, slave);
		break;
	case RWIT:
    case RWDMA:
        cmd.slv_addr = slave;
        cmd.data_inlen = len;
        cmd.pdatain = p;
        if (obj->isbusysend) {
            drv_i2c_cmdlist_put(obj, &cmd);
            return 0x04;
        }
        obj->isbusysend = true;
        // if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY) return HAL_ERROR;
		ret = HAL_I2C_Master_Receive_IT(&obj->hi2c, slave, p, len);
		break;
	default: break;
	}
	return ret;	
}

void drv_i2c_cmd_complete(struct drv_i2c_t *obj)
{
    struct drv_i2c_reg_cmd cmd;

    if (HAL_I2C_GetState(&obj->hi2c) == HAL_I2C_STATE_READY) {
        if (!drv_i2c_cmdlist_get(obj, &cmd)) {
            obj->isbusysend = false;
        } else {
            if (cmd.reg_addr == 0 && cmd.reg_addr_type == 0) {
                if (cmd.data_outlen == 0) {
                    HAL_I2C_Master_Receive_IT(&obj->hi2c, cmd.slv_addr, cmd.pdatain, cmd.data_inlen);
                } else if (cmd.data_inlen == 0) {
                    HAL_I2C_Master_Transmit_IT(&obj->hi2c, cmd.slv_addr, &cmd.data_out[0], cmd.data_outlen);
                }
            } else {
                if (cmd.data_outlen == 0) {
                    HAL_I2C_Mem_Read_IT(&obj->hi2c, cmd.slv_addr, 
                        cmd.reg_addr, cmd.reg_addr_type, cmd.pdatain, cmd.data_inlen);
                } else if (cmd.data_inlen == 0) {
                    HAL_I2C_Mem_Write_IT(&obj->hi2c, cmd.slv_addr,
                        cmd.reg_addr, cmd.reg_addr_type, &cmd.data_out[0], cmd.data_outlen);
                }
            }
        }
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    static uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
    else if (hi2c->Instance == I2C4)	idx = 3;

    drv_i2c_cmd_complete(drv_i2c_list[idx]);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    static uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
    else if (hi2c->Instance == I2C4)	idx = 3;
    
    drv_i2c_cmd_complete(drv_i2c_list[idx]);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    static uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
    else if (hi2c->Instance == I2C4)	idx = 3;
    
    drv_i2c_cmd_complete(drv_i2c_list[idx]);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    static uint8_t idx = 0;
    if (hi2c->Instance == I2C1)		    idx = 0;
    else if (hi2c->Instance == I2C2)    idx = 1;
    else if (hi2c->Instance == I2C3)	idx = 2;
    else if (hi2c->Instance == I2C4)	idx = 3;

    drv_i2c_list[idx]->i2c_error_cnt++;
}

void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&drv_i2c_list[0]->hi2c);
}

void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&drv_i2c_list[0]->hi2c);
    drv_i2c_list[0]->i2c_error_cnt++;
}

void I2C2_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&drv_i2c_list[1]->hi2c);
}

void I2C2_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&drv_i2c_list[1]->hi2c);
    drv_i2c_list[1]->i2c_error_cnt++;
}

void I2C3_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&drv_i2c_list[2]->hi2c);
}

void I2C3_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&drv_i2c_list[2]->hi2c);
    drv_i2c_list[2]->i2c_error_cnt++;
}

void I2C4_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&drv_i2c_list[3]->hi2c);
}

void I2C4_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&drv_i2c_list[3]->hi2c);
    drv_i2c_list[3]->i2c_error_cnt++;
}
