/**
 * low level driver for stm32f4 series, base on std periph library
 * module spi
*/

#include "lld_spi.h"

void lld_spi_init(lld_spi_t *obj, uint8_t num, uint32_t prescaler, lld_spi_mode mode, bool remap)
{
	if (num < 1 || num > 6) return;
	
	SPI_TypeDef *tspi[6] = {SPI1,SPI2,SPI3,SPI4,SPI5,SPI6};
    obj->spi = tspi[num - 1];

	if ((num == 1 || num == 2) && remap) {
		if (num == 2) {
            lld_gpio_init(&obj->sck_pin, GPIOB, 10, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI2);
            lld_gpio_init(&obj->miso_pin, GPIOC, 2, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI2);
            lld_gpio_init(&obj->mosi_pin, GPIOC, 3, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI2);
			lld_gpio_set(&obj->sck_pin, 1);
            lld_gpio_set(&obj->miso_pin, 1);
            lld_gpio_set(&obj->mosi_pin, 1);	
		} else if(num == 1) {
            lld_gpio_init(&obj->sck_pin, GPIOB, 3, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI1);
            lld_gpio_init(&obj->miso_pin, GPIOB, 4, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI1);
            lld_gpio_init(&obj->mosi_pin, GPIOB, 5, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, GPIO_AF_SPI1);
			lld_gpio_set(&obj->sck_pin, 1);
            lld_gpio_set(&obj->miso_pin, 1);
            lld_gpio_set(&obj->mosi_pin, 1);		
		}
	}else if (num == 1 || num == 2) {
		//			                 SPI1     	 SPI2	
		GPIO_TypeDef *port[3]  =  {GPIOA,       GPIOB};
		uint16_t      sck_num[3] = {5,           13};   
		uint16_t      miso_num[3] = {6,           14};  
		uint16_t      mosi_num[3] = {7,           15}; 
		uint8_t       spi_af[3]  = {GPIO_AF_SPI1, GPIO_AF_SPI2};
        
        lld_gpio_init(&obj->sck_pin, port[num - 1], sck_num[num - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, spi_af[num - 1]);
        lld_gpio_init(&obj->miso_pin, port[num - 1], miso_num[num - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, spi_af[num - 1]);
        lld_gpio_init(&obj->mosi_pin, port[num - 1], mosi_num[num - 1], GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_High_Speed, spi_af[num - 1]);		
        lld_gpio_set(&obj->sck_pin, 1);
        lld_gpio_set(&obj->miso_pin, 1);
        lld_gpio_set(&obj->mosi_pin, 1);	
	}
    
	if(obj->spi == SPI1)			RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	else if(obj->spi == SPI2)		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);	
	else if(obj->spi == SPI3)		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);	
	else if(obj->spi == SPI4)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI4,ENABLE);	
	else if(obj->spi == SPI5)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5,ENABLE);	
	else if(obj->spi == SPI6)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6,ENABLE);	
	

	SPI_InitTypeDef SPI_InitStructure;
	SPI_Cmd(obj->spi, DISABLE);
    
    switch (mode) {
    case SPI_MODE_0:
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        break;
    case SPI_MODE_1:
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
        break;
    case SPI_MODE_2:
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        break;
    case SPI_MODE_3:
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
        break;
    }
    
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = prescaler;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(obj->spi, &SPI_InitStructure);
	SPI_Cmd(obj->spi, ENABLE); 
	SPI_ClearFlag(obj->spi, SPI_I2S_FLAG_RXNE);
	SPI_ClearFlag(obj->spi, SPI_I2S_FLAG_TXE);
}

void lld_spi_write(lld_spi_t *obj, const uint8_t *p, uint16_t len, lld_rwway way)
{
    switch (way) {
        case RWPOLL:
        case RWIT:
        case RWDMA:
            for (int i = 0; i < len; i++) {
                SPI_I2S_SendData(obj->spi, p[i]);
                while(SPI_I2S_GetFlagStatus(obj->spi, SPI_I2S_FLAG_TXE) == RESET);
            }
            break;
    }
}

void lld_spi_read(lld_spi_t *obj, uint8_t *p, uint16_t len)
{
    for (int i = 0; i < len; i++) {
        while(SPI_I2S_GetFlagStatus(obj->spi, SPI_I2S_FLAG_RXNE) == RESET);
        p[i] = SPI_I2S_ReceiveData(obj->spi);
    }
}

