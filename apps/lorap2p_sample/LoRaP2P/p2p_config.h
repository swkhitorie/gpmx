#ifndef P2P_CONFIG_H_
#define P2P_CONFIG_H_

/**

1: define it when linkfind state need antenna idx selection 
#define LORAP2P_ANTENNA_LINKTEST

2: define it when resource is not tight
#define LORAP2P_SAVE

3: LoRaP2P Freqency Configuration, Must Define
#define P2P_REGION_US915 / P2P_REGION_EU868 / P2P_REGION_CN470

4. (when define LORAP2P_SAVE) LoRaP2P Role Configuration 
#define P2P_ROLE_MASTER / P2P_ROLE_SLAVE

5. (when define LORAP2P_SAVE) LoRaP2P Default mode Configuration
#define P2P_MODE_RAW / P2P_MODE_RAWACK / P2P_MODE_RAWACK_FHSS

*/

#define P_RBUFFER_LENGTH                 (255)

/* include wait-connecting message TOA */
#define P2P_WAIT_CONNECTING_TIMEOUT      (200)
#define P2P_ACK_TIMEOUT                  (150)
#define P2P_ACK_TIMEOUT_MAX_TRY_TIMES    (5)
#define P2P_LINK_FAIL_TIMEOUT            (1000)
#define P2P_LINK_CRUSH_TIMEOUT           (1000*15)

#define P2P_SEND_LBT_RSSI_THRESH         (-40)
#define P2P_SEND_LBT_SENSE_TIME          (5)
#define P2P_SEND_LBT_FREECNT_TIME        (5)
#define P2P_SEND_LBT_TOTAL_TIME          (50)

#define P2P_ANTENNA_ARRAY                (2)
#define P2P_ANTENNA_ARRAY_RSSI_LEN       (10)
#define P2P_ANTENNA_TEST_TIMES           (50)
#define P2P_ANTENNA_RSSI_THRESOLD        (-95)

#endif
