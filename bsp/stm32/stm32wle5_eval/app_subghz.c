#include "app_subghz.h"
#include "radio.h"
#include <string.h>

SUBGHZ_HandleTypeDef hsubghz;
static RadioEvents_t RadioEvents;

uint8_t MaxUserPayloadSize = 255;

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

    /*starts reception*/
    Radio.Rx(100);
}

static void OnTxDone(void)
{
    // printf("onTxDone\r\n");
    if (!board_radio_sync_already()) {
        Radio.Rx(100);
    } else {
        board_radio_txflag_clr();
    }
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    // printf("onRXDone : %s %d\r\n", payload, size);
    if (size > 0) {
        board_radio_rxbuf_write(payload, size);
    }
}

static void OnTxTimeout(void)
{
    // printf("onTxTimeout \r\n");
}

static void OnRxTimeout(void)
{
    // HAL_Delay(50);
    // printf("OnRxTimeout \r\n");
}

static void OnRxError(void)
{
    // HAL_Delay(50);
    // printf("OnRxError \r\n");
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

// HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
// {
//     return HAL_OK;
// }

// uint32_t HAL_GetTick(void)
// {
//     uint32_t ret = 0;
//     if (SYS_TimerInitialisedFlag == 0) {
//     } else {
//         ret = TIMER_IF_GetTimerValue();
//     }
//     return ret;
// }

// void HAL_Delay(__IO uint32_t Delay)
// {
//     TIMER_IF_DelayMs(Delay);
// }

