#include "app_subghz.h"
#include "radio.h"
#include <string.h>
#include "ringbuffer.h"

SUBGHZ_HandleTypeDef hsubghz;
static RadioEvents_t RadioEvents;
uint8_t MaxUserPayloadSize = 255;

bool tx_done_flag = true;
bool tx_timeout_flag = false;
bool rx_timeout_flag = false;
bool rx_error_flag = false;
int16_t rx_rssi = 99;
int8_t rx_lora_snr = 0;
uint8_t radio_rxmem_sender[RADIO_RXSIZE_SENDER];
uint8_t radio_rxmem_receiver[RADIO_RXSIZE_RECEIVER];
ringbuffer_t radio_rxbuf;

static void OnTxDone(void);
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
static void OnTxTimeout(void);
static void OnRxTimeout(void);
static void OnRxError(void);

void MX_SUBGHZ_Init(void)
{
    int ret;
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
    UTIL_TIMER_Init();

    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
    ret = HAL_SUBGHZ_Init(&hsubghz);
    if (ret != HAL_OK) {
        while(1){}
    }
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

void app_subghz_init(enum RADIO_ROLE role)
{
    MX_SUBGHZ_Init();

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_FREQUENCY);

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_INVERSION_ON, 30);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                        LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                        0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] [SF7..SF12] */

    if((LORA_BANDWIDTH == 2) && (LORA_SPREADING_FACTOR == 5))         MaxUserPayloadSize = 255;//242
    else if ((LORA_BANDWIDTH == 1) && (LORA_SPREADING_FACTOR == 5))   MaxUserPayloadSize = 242;
    else if ( (LORA_BANDWIDTH == 0) && (LORA_SPREADING_FACTOR == 5) ) MaxUserPayloadSize = 242;

    if( (LORA_BANDWIDTH == 2) && (LORA_SPREADING_FACTOR == 6) )       MaxUserPayloadSize = 255;//242
    else if ( (LORA_BANDWIDTH == 1) && (LORA_SPREADING_FACTOR == 6) ) MaxUserPayloadSize = 242;
    else if ( (LORA_BANDWIDTH == 0) && (LORA_SPREADING_FACTOR == 6) ) MaxUserPayloadSize = 242;
    
    if( (LORA_BANDWIDTH == 2) && (LORA_SPREADING_FACTOR == 7) )       MaxUserPayloadSize = 53;//242
    else if ( (LORA_BANDWIDTH == 1) && (LORA_SPREADING_FACTOR == 7) ) MaxUserPayloadSize = 242;
    else if ( (LORA_BANDWIDTH == 0) && (LORA_SPREADING_FACTOR == 7) ) MaxUserPayloadSize = 242;
    else if ( (LORA_BANDWIDTH == 2) && (LORA_SPREADING_FACTOR == 8) ) MaxUserPayloadSize = 255;//???
    else if ( (LORA_BANDWIDTH == 1) && (LORA_SPREADING_FACTOR == 8) ) MaxUserPayloadSize = 242;//???
    else if ( (LORA_BANDWIDTH == 0) && (LORA_SPREADING_FACTOR == 8) ) MaxUserPayloadSize = 125;
    else if ( (LORA_BANDWIDTH == 2) && (LORA_SPREADING_FACTOR == 9) ) MaxUserPayloadSize = 242;//???
    else if ( (LORA_BANDWIDTH == 1) && (LORA_SPREADING_FACTOR == 9) ) MaxUserPayloadSize = 242;//???
    else if ( (LORA_BANDWIDTH == 0) && (LORA_SPREADING_FACTOR == 9) ) MaxUserPayloadSize = 53;

    Radio.SetMaxPayloadLength(MODEM_LORA, MaxUserPayloadSize);

    switch (role) {
    case RADIO_SENDER:
        radio_rxbuf.buf = &radio_rxmem_sender[0];
        radio_rxbuf.capacity = RADIO_RXSIZE_SENDER;
        radio_rxbuf.in = radio_rxbuf.out = radio_rxbuf.size = 0;
        break;
    case RADIO_RECEIVER:
        radio_rxbuf.buf = &radio_rxmem_receiver[0];
        radio_rxbuf.capacity = RADIO_RXSIZE_RECEIVER;
        radio_rxbuf.in = radio_rxbuf.out = radio_rxbuf.size = 0;
        break;
    default:break;
    }

    /*starts reception*/
    Radio.Rx(100);
}

static void OnTxDone(void)
{
    tx_done_flag = true;
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    rx_rssi = rssi;
    rx_lora_snr = LoraSnr_FskCfo;

    if (size > 0) {
        if (radio_rxbuf.capacity == radio_rxbuf.size) {
            rb_reset(&radio_rxbuf);
        } else {
            rb_write(&radio_rxbuf, payload, size);
        }
    }
}

static void OnTxTimeout(void)
{
    tx_timeout_flag = true;
}

static void OnRxTimeout(void)
{
    rx_timeout_flag = true;
}

static void OnRxError(void)
{
    rx_error_flag = true;
}


/****************************************************************************
 * Board Radio Function
 ****************************************************************************/
size_t board_radio_rxbuf_write(uint8_t *p, uint16_t sz) { return rb_write(&radio_rxbuf, p, sz); }
size_t board_radio_rxbuf_size() { return rb_size(&radio_rxbuf); }
size_t board_radio_rxbuf_read(uint8_t *p, uint16_t sz) { return rb_read(&radio_rxbuf, p, sz); }

void board_radio_set_send_done_flag(bool flag) { tx_done_flag = flag; }
bool board_radio_get_send_done_flag() { return tx_done_flag; }
bool board_radio_is_ready_to_send() { return READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE) == (SPI_SR_TXE);}

void board_radio_set_send_timeout_flag(bool flag) { tx_timeout_flag = flag; }
bool board_radio_get_send_timeout_flag() { return tx_timeout_flag; }

void board_radio_set_recv_timeout_flag(bool flag) { rx_timeout_flag = flag; }
bool board_radio_get_recv_timeout_flag() { return rx_timeout_flag; }

void board_radio_set_recv_error_flag(bool flag) { rx_error_flag = flag; }
bool board_radio_get_recv_error_flag() { return rx_error_flag; }

int16_t board_radio_get_rssi() { return rx_rssi; }

int8_t board_radio_get_lora_snr() { return rx_lora_snr; }
uint8_t board_radio_max_payload_sz() { return MaxUserPayloadSize; }

void board_radio_delay_between_rx_tx()
{
    HAL_Delay(Radio.GetWakeupTime()+50);
}
/****************************************************************************
 * Low Layer
 ****************************************************************************/
void SUBGHZ_Radio_IRQHandler(void)
{
    HAL_SUBGHZ_IRQHandler(&hsubghz);
}
