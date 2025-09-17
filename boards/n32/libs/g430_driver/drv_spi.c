#include "drv_spi.h"

static void _spi_pin_config(
    uint8_t mode,
    GPIO_Module* clkport, uint8_t clkpin, uint32_t clk_alternate,
    GPIO_Module* mosiport, uint8_t mosipin, uint32_t mosi_alternate,
    GPIO_Module* misoport, uint8_t misopin, uint32_t miso_alternate,
    GPIO_Module* ncsport, uint8_t ncspin);
static void _spi_config(uint8_t id, uint8_t mode, uint8_t words);

void _spi_pin_config(
    uint8_t mode,
    GPIO_Module* clkport, uint8_t clkpin, uint32_t clk_alternate,
    GPIO_Module* mosiport, uint8_t mosipin, uint32_t mosi_alternate,
    GPIO_Module* misoport, uint8_t misopin, uint32_t miso_alternate,
    GPIO_Module* ncsport, uint8_t ncspin)
{
    if (mode == 0 || mode == 1) {
        // CLKPOL Low
        LOW_PERIPH_INITPIN(clkport, clkpin, GPIO_MODE_AF_PP, GPIO_PULL_DOWN, GPIO_SLEW_RATE_FAST, clk_alternate);
    } else if (mode == 2 || mode == 3) {
        // CLKPOL High
        LOW_PERIPH_INITPIN(clkport, clkpin, GPIO_MODE_AF_PP, GPIO_PULL_UP, GPIO_SLEW_RATE_FAST, clk_alternate);
    }

    LOW_PERIPH_INITPIN(mosiport, mosipin, GPIO_MODE_AF_PP, GPIO_NO_PULL, GPIO_SLEW_RATE_FAST, mosi_alternate);
    LOW_PERIPH_INITPIN(misoport, misopin, GPIO_MODE_AF_PP, GPIO_NO_PULL, GPIO_SLEW_RATE_FAST, miso_alternate);
    
    LOW_PERIPH_INITPIN(ncsport, ncspin, GPIO_MODE_OUT_PP, GPIO_NO_PULL, GPIO_SLEW_RATE_FAST, 0);
}

void _spi_config(uint8_t id, uint8_t mode, uint8_t words)
{
    SPI_InitType spi_init;
    SPI_Module *spi_list[2] = {SPI1, SPI2};

    SPI_Initializes_Structure(&spi_init);
    SPI_I2S_Reset(spi_list[id-1]);

    switch(id) {
    case 1: RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_SPI1); break;
    case 2: RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_SPI2); break;
    }

    /* It is recommended that the SPI master mode of the C version chips should not exceed 18MHz */
    spi_init.BaudRatePres  = SPI_BR_PRESCALER_4;   // 16MHz
    spi_init.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;
    spi_init.SpiMode       = SPI_MODE_MASTER;
    spi_init.NSS           = SPI_NSS_SOFT;
    spi_init.FirstBit      = SPI_FB_MSB;
    spi_init.CRCPoly       = 7;

    if (words == 16) {
        spi_init.DataLen = SPI_DATA_SIZE_16BITS; 
    } else {
        spi_init.DataLen = SPI_DATA_SIZE_8BITS; 
    }

    switch (mode) {
    case 0:
        spi_init.CLKPOL  = SPI_CLKPOL_LOW;
        spi_init.CLKPHA  = SPI_CLKPHA_FIRST_EDGE;
        break;
    case 1:
        spi_init.CLKPOL  = SPI_CLKPOL_LOW;
        spi_init.CLKPHA  = SPI_CLKPHA_SECOND_EDGE;
        break;
    case 2:
        spi_init.CLKPOL  = SPI_CLKPOL_HIGH;
        spi_init.CLKPHA  = SPI_CLKPHA_FIRST_EDGE;
        break;
    case 3:
        spi_init.CLKPOL  = SPI_CLKPOL_HIGH;
        spi_init.CLKPHA  = SPI_CLKPHA_SECOND_EDGE;
        break;
    }

    SPI_Initializes(spi_list[id-1], &spi_init);
    SPI_Set_Nss_Level(spi_list[id-1], SPI_NSS_HIGH);
    // SPI_CRC_Enable(spi_list[id-1]);
    SPI_ON(spi_list[id-1]); 
}

void n32_spimaster_init(struct n32_spimaster_dev *dev)
{
    _spi_pin_config(dev->mode, dev->clk_port, dev->clk_pin, dev->clk_alternate,
        dev->mosi_port, dev->mosi_pin, dev->mosi_alternate,
        dev->miso_port, dev->miso_pin, dev->miso_alternate,
        dev->cs_port, dev->cs_pin);

    _spi_config(dev->id, dev->mode, dev->words);
}

uint8_t n32_spimaster_exchange_byte(struct n32_spimaster_dev *dev, uint8_t val)
{
    SPI_Module *spi_list[2] = {SPI1, SPI2};

    uint8_t retry_cnt = 0;
    while (SPI_I2S_Flag_Status_Get(spi_list[dev->id-1], SPI_I2S_FLAG_TE) == RESET) {
        retry_cnt++;
        if (retry_cnt > 200) {
            return 0;
        }
    }

    SPI_I2S_Data_Transmit(spi_list[dev->id-1], val);

    retry_cnt = 0;
    while (SPI_I2S_Flag_Status_Get(spi_list[dev->id-1], SPI_I2S_FLAG_RNE) == RESET) {
        retry_cnt++;
        if (retry_cnt > 200) {
            return 0;
        }
    }

    return SPI_I2S_Data_Get(spi_list[dev->id-1]);
}

