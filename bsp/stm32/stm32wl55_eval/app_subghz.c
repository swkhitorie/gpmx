#include "app_subghz.h"
#include "radio.h"

SUBGHZ_HandleTypeDef hsubghz;
static RadioEvents_t RadioEvents;

static void OnTxDone(void);
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
static void OnTxTimeout(void);
static void OnRxTimeout(void);
static void OnRxError(void);

void MX_SUBGHZ_Init(void)
{
    int ret;
    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
    ret = HAL_SUBGHZ_Init(&hsubghz);
    if (ret != HAL_OK) {
        while(1){}
    }
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

void app_subghz_init()
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

    Radio.SetMaxPayloadLength(MODEM_LORA, 255);

    /*starts reception*/
    Radio.Rx(30);
}

static void OnTxDone(void)
{
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    board_led_toggle(2);
}

static void OnTxTimeout(void)
{
}

static void OnRxTimeout(void)
{
}

static void OnRxError(void)
{
}

void app_subghz_process()
{

}

/****************************************************************************
 * Low Layer
 ****************************************************************************/
void SUBGHZ_Radio_IRQHandler(void)
{
    HAL_SUBGHZ_IRQHandler(&hsubghz);
}
