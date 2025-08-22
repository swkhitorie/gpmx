#ifndef P2P_CONFIG_H_
#define P2P_CONFIG_H_

/**
 * LoRaP2P Freqency Configuration
 */
#define P_REGION_US915
// #define P_REGION_EU868
// #define P_REGION_CN470

/**
 * LoRaP2P Role Configuration
 */
#define P_ROLE_MASTER
// #define P_ROLE_SLAVE

#define P_RBUFFER_LENGTH   (255)

/* include wait-connecting message TOA */
#define P2P_WAIT_CONNECTING_TIMEOUT      (200)
#define P2P_ACK_TIMEOUT                  (150)
#define P2P_ACK_TIMEOUT_MAX_TRY_TIMES    (5)
#define P2P_LINK_FAIL_TIMEOUT            (1000)

#define P2P_SEND_LBT_RSSI_THRESH         (-40)
#define P2P_SEND_LBT_SENSE_TIME          (5)
#define P2P_SEND_LBT_FREECNT_TIME        (5)
#define P2P_SEND_LBT_TOTAL_TIME          (50)


#endif
