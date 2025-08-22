#ifndef P2P_PLATFORM_H_
#define P2P_PLATFORM_H_

#include <stdint.h>

/* user include */
#include "radio.h"
#include "board_config.h"

#define P2P_ENTER_CRITICAL()
#define P2P_EXIT_CRITICAL()

/* p2p timestamp Interface, units in ms */
#define P2P_TIMESTAMP_GET()        HAL_GetTick()

/* p2p delay Interface, units in ms */
#define P2P_TIME_DELAY(x)          HAL_Delay(x)

/* p2p elapsed Interface, units in ms */
#define P2P_ELAPSED_TIME(x)        board_elapsed_tick(x)

#define p2p_info(...) do {\
    printf("[%d.%03d] ", P2P_TIMESTAMP_GET()/1000, P2P_TIMESTAMP_GET()%1000); \
    if(obj->_role == P2P_SENDER) printf("SENDER "); \
    else if(obj->_role == P2P_RECEIVER) printf("RECEIVER "); \
    printf(__VA_ARGS__); \
    } while(0)

#define p2p_warning(...) printf(__VA_ARGS__)

#define p2p_debug(...)

/** Forward declaration */
struct __p2p_obj;

struct p2p_hw {
    volatile bool cad_done;
    volatile bool cad_busy;
    volatile bool tx_done;
    volatile bool rx_crc_error_occur;
    volatile bool rx_timeout_occur;
    volatile bool tx_timeout_occur;
    volatile uint8_t rx_crc_error_cnt;
    volatile uint8_t rx_timeout_cnt;
    volatile uint8_t tx_timeout_cnt;
};

#ifdef __cplusplus
extern "C" {
#endif

void p2p_platform_obj_bind(struct __p2p_obj *obj);

void p2p_platform_setup(struct __p2p_obj *obj, uint32_t cfg_freq, 
    uint8_t cfg_bw, uint8_t cfg_sf, uint8_t cfg_cr, 
    uint8_t cfg_power, uint16_t cfg_preamblelen, uint8_t max_payloadlen);

void p2p_wait_to_idle(struct __p2p_obj *obj);

bool p2p_is_tx_done(struct __p2p_obj *obj);

void p2p_set_standby(struct __p2p_obj *obj);

void p2p_set_channel(struct __p2p_obj *obj, uint32_t freq);

void p2p_rx(struct __p2p_obj *obj, uint32_t timeout);

int  p2p_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size);

int  p2p_lbt_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    int16_t rssiThresh, uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes);

int  p2p_cad_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes);

bool p2p_cad_is_channel_busy(struct __p2p_obj *obj);

/****************************************************************************
 * LBT(CAD) method: Channel Activity Detect
 ****************************************************************************/
void p2p_cad_start(struct __p2p_obj *obj);

bool p2p_cad_detect_channel_free(struct __p2p_obj *obj, uint32_t freq, uint32_t timeout);

bool p2p_cad_scan_first_free_channel(struct __p2p_obj *obj, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime);

/****************************************************************************
 * LBT(FSK) method: Carrier Sense, Read RSSI
 ****************************************************************************/
bool p2p_lbt_is_channel_free(struct __p2p_obj *obj, uint32_t freq, 
    uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime);

bool p2p_lbt_scan_first_free_channel(struct __p2p_obj *obj, int16_t rssiThresh, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime);


#ifdef __cplusplus
}
#endif


#endif // P2P_PLATFORM_H_

