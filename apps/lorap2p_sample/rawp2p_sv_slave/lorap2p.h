#ifndef LORA_P2P_H_
#define LORA_P2P_H_

#include "p2p_common.h"

enum __LORA_DEV_TYPE {
    DEV_LORAP2P_SLAVE = 0x00,
};

enum __LORA_CMD_TYPE {
    CMD_LORAP2P_OTA_START = 0x01,
    CMD_LORAP2P_OTA_PAYLOAD = 0x02,
    CMD_LORAP2P_OTA_END = 0x03,
};

enum __LORA_CMD_RET_TYPE {
    CMD_LORAP2P_OTA_START_ACK = 0xD1,
    CMD_LORAP2P_OTA_PAYLOAD_ACK = 0xD2,
    CMD_LORAP2P_OTA_END_ACK = 0xD3,
};

#ifdef __cplusplus
extern "C" {
#endif

void lorap2p_init();

void lorap2p_process();

#ifdef __cplusplus
}
#endif


#endif
