#include "drv_i2c_v1.h"

/** 
  * SR2 register flags  
  */
#define I2C_V1_FLAG_DUALF                  ((uint32_t)0x00800000)
#define I2C_V1_FLAG_SMBHOST                ((uint32_t)0x00400000)
#define I2C_V1_FLAG_SMBDEFAULT             ((uint32_t)0x00200000)
#define I2C_V1_FLAG_GENCALL                ((uint32_t)0x00100000)
#define I2C_V1_FLAG_TRA                    ((uint32_t)0x00040000)
#define I2C_V1_FLAG_BUSY                   ((uint32_t)0x00020000)
#define I2C_V1_FLAG_MSL                    ((uint32_t)0x00010000)

/** 
  * SR1 register flags  
  */
#define I2C_V1_FLAG_SMBALERT               ((uint32_t)0x10008000)
#define I2C_V1_FLAG_TIMEOUT                ((uint32_t)0x10004000)
#define I2C_V1_FLAG_PECERR                 ((uint32_t)0x10001000)
#define I2C_V1_FLAG_OVR                    ((uint32_t)0x10000800)
#define I2C_V1_FLAG_AF                     ((uint32_t)0x10000400)
#define I2C_V1_FLAG_ARLO                   ((uint32_t)0x10000200)
#define I2C_V1_FLAG_BERR                   ((uint32_t)0x10000100)
#define I2C_V1_FLAG_TXE                    ((uint32_t)0x10000080)
#define I2C_V1_FLAG_RXNE                   ((uint32_t)0x10000040)
#define I2C_V1_FLAG_STOPF                  ((uint32_t)0x10000010)
#define I2C_V1_FLAG_ADD10                  ((uint32_t)0x10000008)
#define I2C_V1_FLAG_BTF                    ((uint32_t)0x10000004)
#define I2C_V1_FLAG_ADDR                   ((uint32_t)0x10000002)
#define I2C_V1_FLAG_SB                     ((uint32_t)0x10000001)

static void drv_i2c_v1_reset_clk(struct drv_i2c_v1_t *obj)
{
	RCC->APB1RSTR |= obj->rcc;       //Start reset I2C Clock
	RCC->APB1RSTR &= ~obj->rcc;      //Stop  reset I2C Clock
	RCC->APB1ENR  |=  obj->rcc;      //Enable I2C clock
}

static void drv_i2c_v1_set_clk(struct drv_i2c_v1_t *obj, bool state)
{
    if (state) {
        RCC->APB1ENR |= obj->rcc;
    } else {
        RCC->APB1ENR &= ~obj->rcc;
    }
}

static void drv_i2c_v1_reset_dev(struct drv_i2c_v1_t *obj)
{
	//Reset I2C device and RCC
	obj->com->CR1 |=  I2C_CR1_SWRST;    //Start reset I2c Controler -- soft reset
	obj->com->CR1 &=  ~I2C_CR1_SWRST;   //Stop  reset I2C
	obj->com->CR1 &=  ~I2C_CR1_PE;      //DISABLE I2C
}

static uint32_t drv_i2c_v1_get_flagstatus(struct drv_i2c_v1_t *obj, uint32_t flag)
{
    uint32_t status = 0x00;
    volatile uint32_t i2c_reg = 0, i2c_base = 0;
    i2c_base = (uint32_t)obj->com;
    i2c_reg = flag >> 28;

    /* Get bit[23:0] of the flag */
    flag &= 0x00FFFFFF;
    if (i2c_reg != 0) {
        /* Get the I2Cx SR1 register address */
        i2c_base += 0x14;
    } else {
        /* Flag in I2Cx SR2 Register */
        flag = (uint32_t)(flag >> 16);
        /* Get the I2Cx SR2 register address */
        i2c_base += 0x18;
    }
    if(((*(volatile uint32_t *)i2c_base) & flag) != (uint32_t)RESET) {
        status = SET;
    } else {
        status = RESET;
    }
    return status;
}

static void drv_i2c_reg_perphal_init(struct drv_i2c_v1_t *obj)
{
    uint16_t tmpreg = 0, freqrange = 0;
    uint16_t result = 0x04;
    uint32_t pclk1 = 8000000;


    tmpreg = obj->com->CR2;
    tmpreg &= (uint16_t)~((uint16_t)I2C_CR2_FREQ);
    RCC_GetClocksFreq(&rcc_clocks);
    pclk1 = HAL_RCC_GetPCLK1Freq();
    freqrange = (uint16_t)(pclk1 / 1000000);
    tmpreg |= freqrange;
    obj->com->CR2 = tmpreg;

    obj->com->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_PE);
    tmpreg = 0;

    /* Configure speed in standard mode */
    if (I2C_InitStruct->I2C_ClockSpeed <= 100000) {
        /* Standard mode speed calculate */
        result = (uint16_t)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed << 1));
        /* Test if CCR value is under 0x4*/
        if (result < 0x04) {
            /* Set minimum allowed value */
            result = 0x04;  
        }
        /* Set speed value for standard mode */
        tmpreg |= result;	  
        /* Set Maximum Rise Time for standard mode */
        I2Cx->TRISE = freqrange + 1; 
    } else { /*(I2C_InitStruct->I2C_ClockSpeed <= 400000)*/
        /* Configure speed in fast mode */
        /* To use the I2C at 400 KHz (in fast mode), the PCLK1 frequency (I2C peripheral
        input clock) must be a multiple of 10 MHz */
        if (I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_2) {
        /* Fast mode speed calculate: Tlow/Thigh = 2 */
        result = (uint16_t)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed * 3));
        } else { /*I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_16_9*/
            /* Fast mode speed calculate: Tlow/Thigh = 16/9 */
            result = (uint16_t)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed * 25));
            /* Set DUTY bit */
            result |= I2C_DutyCycle_16_9;
        }
        /* Test if CCR value is under 0x1*/
        if ((result & I2C_CCR_CCR) == 0) {
            /* Set minimum allowed value */
            result |= (uint16_t)0x0001;  
        }
        /* Set speed value and set F/S bit for fast mode */
        tmpreg |= (uint16_t)(result | I2C_CCR_FS);
        /* Set Maximum Rise Time for fast mode */
        I2Cx->TRISE = (uint16_t)(((freqrange * (uint16_t)300) / (uint16_t)1000) + (uint16_t)1);  
    }

    /* Write to I2Cx CCR */
    I2Cx->CCR = tmpreg;
    /* Enable the selected I2C peripheral */
    I2Cx->CR1 |= I2C_CR1_PE;

    /*---------------------------- I2Cx CR1 Configuration ------------------------*/
    /* Get the I2Cx CR1 value */
    tmpreg = I2Cx->CR1;
    /* Clear ACK, SMBTYPE and  SMBUS bits */
    tmpreg &= CR1_CLEAR_MASK;
    /* Configure I2Cx: mode and acknowledgement */
    /* Set SMBTYPE and SMBUS bits according to I2C_Mode value */
    /* Set ACK bit according to I2C_Ack value */
    tmpreg |= (uint16_t)((uint32_t)I2C_InitStruct->I2C_Mode | I2C_InitStruct->I2C_Ack);
    /* Write to I2Cx CR1 */
    I2Cx->CR1 = tmpreg;

    /*---------------------------- I2Cx OAR1 Configuration -----------------------*/
    /* Set I2Cx Own Address1 and acknowledged address */
    I2Cx->OAR1 = (I2C_InitStruct->I2C_AcknowledgedAddress | I2C_InitStruct->I2C_OwnAddress1);
}

static void drv_i2c_reg_gpio_init(struct drv_i2c_v1_t *obj, uint32_t mode)
{
    uint32_t i2c_af[3]    = {GPIO_AF_I2C1, GPIO_AF_I2C2, GPIO_AF_I2C3};

    drv_gpio_init(obj->scl_port, obj->scl_pin, mode, IO_PULLUP, IO_SPEEDHIGH, i2c_af[obj->num-1], NULL);
    drv_gpio_init(obj->scl_port, obj->scl_pin, mode, IO_PULLUP, IO_SPEEDHIGH, i2c_af[obj->num-1], NULL);
}

void drv_i2c_reg_attr_config(struct drv_i2c_reg_attr_t *obj, uint32_t speed)
{
    obj->speed = speed;
}

void drv_i2c_reg_cmdlist_config(struct drv_i2c_reg_cmdlist *obj, struct drv_i2c_reg_cmd *buf, uint8_t capacity)
{
    obj->capacity = capacity;
    obj->buf = buf;
    obj->size = 0;
    obj->in = obj->out = 0;
}

void drv_i2c_v1_config(uint8_t num, struct drv_i2c_v1_t *obj, 
            struct drv_i2c_reg_attr_t *attr, struct drv_i2c_reg_cmdlist *list)
{
    I2C_TypeDef *i2c_list[3] = { I2C1, I2C2, I2C3 };
    uint32_t i2c_rcc[3] = { RCC_APB1ENR_I2C1EN, RCC_APB1ENR_I2C2EN, RCC_APB1ENR_I2C3EN};

    GPIO_TypeDef *scl_port[3]  = {GPIOB,   GPIOB,    GPIOA};
    uint16_t      scl_pin[3]   = {6,       10,        8  };

    GPIO_TypeDef *sda_port[3]  = {GPIOB,   GPIOB,    GPIOC};
    uint16_t      sda_pin[3]   = {7,       11,        9, };

    obj->num = num;
    obj->com = i2c_list[num-1];
    obj->rcc = i2c_rcc[num-1];
    obj->scl_port = scl_port[num-1];
    obj->sda_port = sda_port[num-1];
    obj->scl_pin = scl_pin[num-1];
    obj->sda_pin = sda_pin[num-1];

    obj->attr = *attr;
    obj->cmdlist = *list;
}



bool drv_i2c_v1_reset(struct drv_i2c_v1_t *obj)
{
    drv_i2c_v1_reset_dev(obj);
    drv_i2c_v1_reset_clk(obj);

	//Check and solve I2C Bus Busy problem
	uint16_t timeout_cnt = 0;
	while (drv_i2c_v1_get_flagstatus(obj, I2C_V1_FLAG_BUSY) && timeout_cnt < 20) {
		//Check Busy state and time out FLAG
		drv_i2c_v1_set_clk(obj, false)		                //Close I2C Device;
        drv_i2c_reg_gpio_init(obj, IOMODE_OUTPP);

        HAL_GPIO_WritePin(obj->scl_port, obj->scl_pin, 0);	
        HAL_GPIO_WritePin(obj->sda_port, obj->sda_pin, 0);
		for(volatile uint32_t i=0;i<5;i++);                // Stop Signal by Pin simulation 
        HAL_GPIO_WritePin(obj->scl_port, obj->scl_pin, 1);
		for(volatile uint32_t i=0;i<5;i++);
        HAL_GPIO_WritePin(obj->sda_port, obj->sda_pin, 1);
		for(volatile uint32_t i=0;i<5;i++);

        drv_i2c_reg_gpio_init(obj, IOMODE_INPUT);
        drv_i2c_v1_reset_clk(obj);
		++timeout_cnt;
	}

	return (timeout_cnt < 20);
}

bool drv_i2c_v1_init(struct drv_i2c_v1_t *obj)
{
    obj->i2c_error_cnt = 0;

    drv_i2c_reg_gpio_init(obj, IOMODE_AFPP);
	if (!drv_i2c_v1_reset()) {
		obj->state = I2CSTATE_ERROR;
		return false;
	}
    drv_i2c_reg_gpio_init(obj, IOMODE_AFOD);

	I2C_InitTypeDef I2C_InitStructure;
	
	I2C_InitStructure.I2C_ClockSpeed          = _I2CSpeed;                     
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;                 
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;               
	I2C_InitStructure.I2C_OwnAddress1         = 0;                            
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;             
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  
	I2C_Init(_I2Cx,&I2C_InitStructure);	

I2C_GetLastEvent

    obj->state = I2CSTATE_FREE;
    return true;
}


