/**
 * low level driver for stm32h7 series, base on cubehal library
 * module i2c
*/

#include "include/lld_i2c.h"

lld_i2c_t *mcu_i2c_list[4] = {0, 0, 0, 0};

bool lowlevel_i2c_pin_source_init(lld_i2c_t *obj, uint8_t i2c_num, uint8_t scl_selec, uint8_t sda_selec)
{
	const struct pin_node *i2c_scl_node;
	const struct pin_node *i2c_sda_node;
	uint32_t is_pin_node_initable;
	
	switch (i2c_num) {
	case 1:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, scl_selec)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, sda_selec)) != NULL) {
			i2c_scl_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SCL, scl_selec);
			i2c_sda_node = I2C_PINCTRL_SOURCE(1, I2C_PIN_SDA, sda_selec);
			is_pin_node_initable = i2c_scl_node->gpio_port && i2c_sda_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, scl_selec)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, sda_selec)) != NULL) {
			i2c_scl_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SCL, scl_selec);
			i2c_sda_node = I2C_PINCTRL_SOURCE(2, I2C_PIN_SDA, sda_selec);
			is_pin_node_initable = i2c_scl_node->gpio_port && i2c_sda_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 3:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, scl_selec)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, sda_selec)) != NULL) {
			i2c_scl_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SCL, scl_selec);
			i2c_sda_node = I2C_PINCTRL_SOURCE(3, I2C_PIN_SDA, sda_selec);
			is_pin_node_initable = i2c_scl_node->gpio_port && i2c_sda_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 4:
		if (PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, scl_selec)) != NULL &&
			PINNODE(uint32_t)(I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, sda_selec)) != NULL) {
			i2c_scl_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SCL, scl_selec);
			i2c_sda_node = I2C_PINCTRL_SOURCE(4, I2C_PIN_SDA, sda_selec);
			is_pin_node_initable = i2c_scl_node->gpio_port && i2c_sda_node->gpio_port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (is_pin_node_initable != 0) {
        lld_gpio_init(&obj->scl, i2c_scl_node->gpio_port, i2c_scl_node->pin_num, GPIO_MODE_AF_OD,
							GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, i2c_scl_node->alternate);
        lld_gpio_init(&obj->sda, i2c_sda_node->gpio_port, i2c_sda_node->pin_num, GPIO_MODE_AF_OD,
							GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, i2c_sda_node->alternate);
	}else {
		return false;
	}
	return true;
}

bool lld_i2c_init(lld_i2c_t *obj, uint8_t i2c_num, uint32_t speed, uint8_t scl_selec, uint8_t sda_selec)
{
    obj->i2cnum = i2c_num;
	switch (i2c_num) {
	case 1: __HAL_RCC_I2C1_CLK_ENABLE(); break;
	case 2: __HAL_RCC_I2C2_CLK_ENABLE(); break;
	case 3: __HAL_RCC_I2C3_CLK_ENABLE(); break;
	case 4: __HAL_RCC_I2C4_CLK_ENABLE(); break;
	default: break;
	}
    
	if(!lowlevel_i2c_pin_source_init(obj, i2c_num, scl_selec, sda_selec)) return false;
    
	I2C_TypeDef *I2Cx[4] = {I2C1, I2C2, I2C3, I2C4};
	obj->hi2c.Instance 			    = I2Cx[i2c_num - 1];
	obj->hi2c.Init.Timing           = LLD_I2C_STANDARDMODE;       // 100M pclk1
	obj->hi2c.Init.OwnAddress1      = 0;
	obj->hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
	obj->hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
	obj->hi2c.Init.OwnAddress2      = 0;
	obj->hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	obj->hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
	obj->hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&obj->hi2c);
	HAL_I2CEx_AnalogFilter_Config(&obj->hi2c, I2C_ANALOGFILTER_ENABLE); 
	
	IRQn_Type i2cERIRQ[4] = {I2C1_ER_IRQn, I2C2_ER_IRQn, I2C3_ER_IRQn, I2C4_ER_IRQn};
	IRQn_Type i2cEVIRQ[4] = {I2C1_EV_IRQn, I2C2_EV_IRQn, I2C3_EV_IRQn, I2C4_EV_IRQn};
	HAL_NVIC_SetPriority(i2cERIRQ[i2c_num - 1], 0, 0);
	HAL_NVIC_EnableIRQ(i2cERIRQ[i2c_num - 1]);
	HAL_NVIC_SetPriority(i2cEVIRQ[i2c_num - 1], 0, 0);
	HAL_NVIC_EnableIRQ(i2cEVIRQ[i2c_num - 1]);
    
	mcu_i2c_list[i2c_num - 1] = obj;
    
    return true;
}

void lld_i2c_blockwait(lld_i2c_t *obj, uint16_t slave_addr)
{
	while (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY);
	while (HAL_I2C_IsDeviceReady(&obj->hi2c, slave_addr, 1000, 1000) == HAL_TIMEOUT);
	while (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY);
}

void lld_i2c_irq_cmdcomplete(lld_i2c_t *obj)
{
    if (HAL_I2C_GetState(&obj->hi2c) == HAL_I2C_STATE_READY) {
        obj->is_busytransmit = false;
    }
}

uint8_t lld_i2c_write_reg(lld_i2c_t *obj, uint8_t slave, uint8_t reg, uint8_t *writeAddr, uint8_t regtype, uint16_t len, lld_rwway way)
{
	HAL_StatusTypeDef res = HAL_OK;
    uint16_t type = regtype;
	switch (way) {
	case RWPOLL:
		res = HAL_I2C_Mem_Write(&obj->hi2c, slave, reg, type, writeAddr, len, 1000);
        lld_i2c_blockwait(obj, slave);
		break;
	case RWIT:
    case RWDMA:
        if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY || obj->is_busytransmit) return HAL_ERROR;
        obj->is_busytransmit = true;
		res = HAL_I2C_Mem_Write_IT(&obj->hi2c, slave, reg, type, writeAddr, len);
		break;
	default: break;
	}
	return res;
}

uint8_t lld_i2c_read_reg(lld_i2c_t *obj, uint8_t slave, uint8_t reg, uint8_t *readAddr, uint8_t regtype, uint16_t len, lld_rwway way)
{
	HAL_StatusTypeDef res = HAL_OK;
    uint16_t type = regtype;
	switch (way) {
	case RWPOLL:
		res = HAL_I2C_Mem_Read(&obj->hi2c, slave, reg, type, readAddr, len, 1000);
        lld_i2c_blockwait(obj, slave);
		break;
	case RWIT:
    case RWDMA:
        if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY || obj->is_busytransmit) return HAL_ERROR;
        obj->is_busytransmit = true;
		res = HAL_I2C_Mem_Read_IT(&obj->hi2c, slave, reg, type, readAddr, len);
		break;
	default: break;
	}
	return res;
}

uint8_t lld_i2c_write_single(lld_i2c_t *obj, uint16_t slave, uint8_t *pData, uint16_t Size, lld_rwway way)
{
	HAL_StatusTypeDef res = HAL_OK;
	switch (way) {
	case RWPOLL:
		res = HAL_I2C_Master_Transmit(&obj->hi2c, slave, pData, Size, 1000);
        lld_i2c_blockwait(obj, slave);
		break;
	case RWIT:
    case RWDMA:
        if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY || obj->is_busytransmit) return HAL_ERROR;
        obj->is_busytransmit = true;
		res = HAL_I2C_Master_Transmit_IT(&obj->hi2c, slave, pData, Size);
		break;
	default: break;
	}
	return res;	
}

uint8_t lld_i2c_read_single(lld_i2c_t *obj, uint16_t slave, uint8_t *pData, uint16_t Size, lld_rwway way)
{
	HAL_StatusTypeDef res = HAL_OK;
	switch (way) {
	case RWPOLL:
		res = HAL_I2C_Master_Receive(&obj->hi2c, slave, pData, Size, 1000);
        lld_i2c_blockwait(obj, slave);
		break;
	case RWIT:
    case RWDMA:
        if (HAL_I2C_GetState(&obj->hi2c) != HAL_I2C_STATE_READY || obj->is_busytransmit) return HAL_ERROR;
        obj->is_busytransmit = true;
		res = HAL_I2C_Master_Receive_IT(&obj->hi2c, slave, pData, Size);
		break;
	default: break;
	}
	return res;	 
}

/*****************************************************************
 *****  CubeH7 I2C interrupt and callback
 ****************************************************************
*/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t idx = 0;
	if (hi2c->Instance == I2C1)		idx = 0;
	else if (hi2c->Instance == I2C2)    idx = 1;
	else if (hi2c->Instance == I2C3)	idx = 2;
	else if (hi2c->Instance == I2C4)	idx = 3;
	if (mcu_i2c_list[idx]) {
		lld_i2c_irq_cmdcomplete(mcu_i2c_list[idx]);
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t idx = 0;
	if (hi2c->Instance == I2C1)		idx = 0;
	else if (hi2c->Instance == I2C2)    idx = 1;
	else if (hi2c->Instance == I2C3)	idx = 2;
	else if (hi2c->Instance == I2C4)	idx = 3;
	if (mcu_i2c_list[idx]) {
		lld_i2c_irq_cmdcomplete(mcu_i2c_list[idx]);
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t idx = 0;
	if (hi2c->Instance == I2C1)		idx = 0;
	else if (hi2c->Instance == I2C2)    idx = 1;
	else if (hi2c->Instance == I2C3)	idx = 2;
	else if (hi2c->Instance == I2C4)	idx = 3;
	if (mcu_i2c_list[idx]) {
		lld_i2c_irq_cmdcomplete(mcu_i2c_list[idx]);
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
	if (HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF) {
	}
}

void I2C1_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&mcu_i2c_list[0]->hi2c);
}

void I2C1_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&mcu_i2c_list[0]->hi2c);
}

void I2C2_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&mcu_i2c_list[1]->hi2c);
}

void I2C2_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&mcu_i2c_list[1]->hi2c);
}

void I2C3_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&mcu_i2c_list[2]->hi2c);
}

void I2C3_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&mcu_i2c_list[2]->hi2c);
}

void I2C4_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&mcu_i2c_list[3]->hi2c);
}

void I2C4_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&mcu_i2c_list[3]->hi2c);
}
