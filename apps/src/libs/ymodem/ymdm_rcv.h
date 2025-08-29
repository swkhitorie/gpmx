#ifndef _YMODEM_H_
#define _YMODEM_H_

#include "ymdm_cmn.h"

#define YM_LOG(...)   printf(__VA_ARGS__)

// block sending
typedef void (*itf_ymodem_receiver_sndbyte)(uint8_t c);

enum __ymodem_rcv_state {
	YMODEM_IDLE = 0x00,
    YMODEM_SYNC_WAIT,
	YMODEM_RCV_PAYLOAD,
	YMODEM_RCV_EOTA,
    YMODEM_RCV_EOTB,
	YMODEM_RCV_ENDTRANSMISSION,


};

struct __ymodem_rcv_parse {
    uint8_t buff[YMODEM_NONPAYLOADLEN+YMODEM_PAYLOADLEN_STX+1];
    uint16_t nbyte;
    uint16_t nlen;

	uint8_t seq;
	uint8_t seq_xor;
};

struct __ymodem_receiver {

    volatile enum __ymodem_rcv_state state;

    struct __ymodem_status status;

    struct __ymodem_rcv_parse proto;

    itf_ymodem_receiver_sndbyte _fsnd;

	uint8_t *rcv_mem;
	uint32_t rcv_mem_capacity;
    uint32_t rcv_mem_sz;

	char fname[128];
	uint32_t fsize;
	uint32_t fsize_cal;

	uint8_t seq_lst;
	uint8_t lst_packtype;
	uint8_t packtype;
	uint8_t eot_count;
};

#ifdef __cplusplus
extern "C" {
#endif

void ymodem_cfg_snd_interface(struct __ymodem_receiver *yrcv, itf_ymodem_receiver_sndbyte intf);

void ymodem_rx_process(struct __ymodem_receiver *yrcv, const uint8_t *p, uint16_t len);


#ifdef __cplusplus
}
#endif


#endif
