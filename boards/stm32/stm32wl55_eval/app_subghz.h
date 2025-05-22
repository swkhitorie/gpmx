#ifndef __APP_SUBGHZ_PHY_H__
#define __APP_SUBGHZ_PHY_H__

#include <board_config.h>

/* MODEM type: one shall be 1 the other shall be 0 */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#define REGION_US915

#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 /* Hz */
#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 /* Hz */

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 /* Hz */

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 /* Hz */

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 /* Hz */

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 /* Hz */

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 /* Hz */

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 /* Hz */

#else
#error "Please define a frequency band in the compiler options."
#endif /* REGION_XXxxx */

#define TX_OUTPUT_POWER                             22        /* dBm */

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
#define LORA_BANDWIDTH                              1         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       5         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define RADIO_RXSIZE_SENDER    (128)
#define RADIO_RXSIZE_RECEIVER  (1024*2)

enum RADIO_ROLE {
    RADIO_SENDER = 0x00,
    RADIO_RECEIVER,
};

#ifdef __cplusplus
extern "C"{
#endif

void MX_SUBGHZ_Init();

void app_subghz_init(enum RADIO_ROLE role);

/****************************************************************************
 * Board Radio Function
 ****************************************************************************/
size_t board_radio_rxbuf_write(uint8_t *p, uint16_t sz);
size_t board_radio_rxbuf_size();
size_t board_radio_rxbuf_read(uint8_t *p, uint16_t sz);

void board_radio_set_send_done_flag(bool flag);
bool board_radio_get_send_done_flag();
bool board_radio_is_ready_to_send();

void board_radio_set_send_timeout_flag(bool flag);
bool board_radio_get_send_timeout_flag();

void board_radio_set_recv_timeout_flag(bool flag);
bool board_radio_get_recv_timeout_flag();

void board_radio_set_recv_error_flag(bool flag);
bool board_radio_get_recv_error_flag();

int16_t board_radio_get_rssi();
int8_t board_radio_get_lora_snr();

uint8_t board_radio_max_payload_sz();

/**
 * when radio tx trans to rx
 *      no waiting, wait txDone() and call Rado.Rx
 * when radio rx trans to tx
 *      waiting call board_radio_delay_between_rx_tx()
 */
void board_radio_delay_between_rx_tx();

#ifdef __cplusplus
}
#endif

#endif

