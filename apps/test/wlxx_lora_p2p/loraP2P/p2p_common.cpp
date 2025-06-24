#include "p2p_common.h"

static uint8_t radio_rxmem[RADIO_RXBUFF_SZ];

static RadioEvents_t _radio_events;
p2p_obj_t *_p2p_obj_callback;

void CadDone(bool channelActivityDetected) {
    //true: channel busy, false: channel free
    BOARD_DEBUG("CAD: %d\r\n", channelActivityDetected);

    _p2p_obj_callback->cad_done = true;
    _p2p_obj_callback->cad_busy = channelActivityDetected;
}

void OnTxDone() {
    _p2p_obj_callback->tx_done = true;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo) {

    if (_p2p_obj_callback->role == P2P_SENDER) {
        _p2p_obj_callback->channelgrp.up_rssi = rssi;
        _p2p_obj_callback->channelgrp.up_snr = LoraSnr_FskCfo;
    } else if (_p2p_obj_callback->role == P2P_RECEIVER) {
        _p2p_obj_callback->channelgrp.down_rssi = rssi;
        _p2p_obj_callback->channelgrp.down_snr = LoraSnr_FskCfo;
    }

    ringbuffer_t *rxbuff = &_p2p_obj_callback->rf_rxbuf;

    if (size > 0) {
        if (rxbuff->capacity == rxbuff->size) {
            rb_reset(rxbuff);
        } else {
            rb_write(rxbuff, payload, size);
        }
    }
}

void OnTxTimeout() {
    BOARD_DEBUG("Tx Timeout\r\n");
    _p2p_obj_callback->tx_timeout_cnt++;
    _p2p_obj_callback->tx_timeout_occur = true;
}

void OnRxTimeout() {
    BOARD_DEBUG("Rx Timeout/Head Error\r\n");
    _p2p_obj_callback->rx_timeout_cnt++;
    _p2p_obj_callback->rx_timeout_occur = true;
}

void OnRxError() {
    BOARD_DEBUG("Rx CRC Error\r\n");
    _p2p_obj_callback->rx_crc_error_cnt++;
    _p2p_obj_callback->rx_crc_error_occur = true;
}

void p2p_objcallback_set(p2p_obj_t *obj)
{
    _p2p_obj_callback = obj;
}

void p2p_setup(p2p_obj_t *obj, p2p_role_t role, p2p_mode_t mode,
    region_t region, forwarding_data ph, authkey_generate auth,
    uint32_t *id, uint32_t id_key)
{
    obj->role = role;
    obj->p2p_mode = mode;

    obj->tx_done = true;
    _radio_events.TxDone = OnTxDone;
    _radio_events.RxDone = OnRxDone;
    _radio_events.TxTimeout = OnTxTimeout;
    _radio_events.RxTimeout = OnRxTimeout;
    _radio_events.RxError = OnRxError;
    _radio_events.CadDone = CadDone;
    Radio.Init(&_radio_events);

    rb_init(&obj->rf_rxbuf, radio_rxmem, RADIO_RXBUFF_SZ);

    obj->hp = ph;
    obj->hauth = auth;

    memcpy(&obj->id.uid_board[0], &id[0], 12);
    obj->id.rand_key_board = id_key;
    obj->id.auth_key_board = (*obj->hauth)(obj->id.uid_board, obj->id.rand_key_board);
    rand_lcg_seed_set(obj->id.rand_key_board);

    /* init channel group */
    switch (region) {
    case LORA_REGION_EU868:
        region_eu868_init_default(&obj->channelgrp);
        break;
    case LORA_REGION_US915:
        region_us915_init_default(&obj->channelgrp);
        break;
    }

    obj->channelgrp.current.bw = 1;            //bw: 500Khz
    obj->channelgrp.current.sf = 6;            //sf: 7
    obj->channelgrp.current.cr = 1;            //cr: 4/5
    obj->channelgrp.current.power = 18;        //power: 18dbm
    obj->channelgrp.current.freq = 902300000; //obj->channelgrp.ping.freq;    //init with ping channel

    p2p_radio_cfg(obj, &obj->channelgrp.current);
    obj->channelgrp.time_on_air = Radio.TimeOnAir(MODEM_LORA, 
        obj->channelgrp.current.bw, 
        obj->channelgrp.current.sf, 
        obj->channelgrp.current.cr, 8, true, obj->channelgrp.max_payload, true);

    P2P_DEBUG("\r\n\tLORA P2P Version:v%d.%d.%d, BOARD ID:%x %x %x\r\n\
\tDefault Freq:%d, Power:%d, BW:%d, SF:%d, CR:%d, TOA(%d):%dms\r\n\
\tKey: %x, Auth Key: %x\r\n",
        P2P_VERSION_MAIN, P2P_VERSION_SUB1, P2P_VERSION_SUB2,
        obj->id.uid_board[0], obj->id.uid_board[1], obj->id.uid_board[2],
        obj->channelgrp.current.freq, 
        obj->channelgrp.current.power, 
        obj->channelgrp.current.bw, 
        obj->channelgrp.current.sf, 
        obj->channelgrp.current.cr, 
        obj->channelgrp.max_payload,
        obj->channelgrp.time_on_air,
        obj->id.rand_key_board,
        obj->id.auth_key_board);

    obj->status.first_enter = true;
    if (obj->role == P2P_SENDER) {
        bool isfind = p2p_detect_first_freechannel(obj, 800, 800, 10000);
        if (!isfind) {
            P2P_DEBUG("Can't Find Any FreeChannel\r\n");
            while(1){}
        }
    }

    if (obj->p2p_mode == P2P_LISTEN) {
        P2P_DEBUG("start to scan channels rssi\r\n");
    } else {
        p2p_state_to_linkfind(obj);
    }
}

void p2p_process(p2p_obj_t *obj)
{
    switch (obj->p2p_mode) {
    case P2P_LISTEN:
        static int idx_scan = 0;
        if (idx_scan >= obj->channelgrp.list_num - 1) {
            idx_scan = 0;
        } else {
            idx_scan++;
        }

        int16_t rssiThrold = -80;
        int freq = obj->channelgrp.ch_list[idx_scan].freq;
        bool isfree = p2p_ischannelfree(obj, freq, 250000, rssiThrold, 5);
        if (!isfree) {
            P2P_DEBUG("channel %d busy, rssi over %d\r\n", freq, rssiThrold);
        }
        return;
    }

    switch(obj->state) {
    case P2P_LINK_FIND:
        p2p_linkfind(obj);
        break;
    case P2P_LINK_ESTABLISHED:
        if (obj->p2p_mode == P2P_RAW) {
            p2p_raw_process(obj);
        } else if (obj->p2p_mode == P2P_RAWACK) {
            p2p_raw_ack_process(obj);
        } else if (obj->p2p_mode == P2P_RAWACK_FHSS) {
            //p2p_raw_ackfhss_process(obj);
        }
        break;
    }
}

