#include "lora_common.h"

static uint8_t lora_rxmem[LORA_RADIO_RXBUFF_SZ];

static RadioEvents_t _radio_events;
lora_state_t *lora_p2p_callback;

void lora_p2p_objcallback_set(lora_state_t *obj)
{
    lora_p2p_callback = obj;
}

void OnTxDone() {
    lora_p2p_callback->tx_done = true;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo) {
    lora_p2p_callback->rssi = rssi;
    lora_p2p_callback->snr = LoraSnr_FskCfo;
    ringbuffer_t *rxbuff = &lora_p2p_callback->rf_rxbuf;

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

    lora_p2p_callback->tx_timeout_cnt++;
    lora_p2p_callback->tx_timeout_occur = true;
}

void OnRxTimeout() {
    BOARD_DEBUG("Rx Timeout\r\n");

    lora_p2p_callback->rx_timeout_cnt++;
    lora_p2p_callback->rx_timeout_occur = true;
}

void OnRxError() {

    BOARD_DEBUG("Rx CRC Error\r\n");

    lora_p2p_callback->rx_crc_error_cnt++;
    lora_p2p_callback->rx_crc_error_occur = true;
}

void lora_p2p_setup(    
    lora_state_t *obj,
    radio_p2p_role_t role,
    lora_region_t region,
    forwarding_data ph,
    uint32_t *id,
    uint32_t id_key)
{
    obj->role = role;

    _radio_events.TxDone = OnTxDone;
    _radio_events.RxDone = OnRxDone;
    _radio_events.TxTimeout = OnTxTimeout;
    _radio_events.RxTimeout = OnRxTimeout;
    _radio_events.RxError = OnRxError;
    Radio.Init(&_radio_events);

    rb_init(&obj->rf_rxbuf, lora_rxmem, LORA_RADIO_RXBUFF_SZ);

    obj->freq = 915000000;  //US915
    obj->bw = 1;            //bw: 250Khz
    obj->sf = 7;            //sf: 7
    obj->cr = 1;            //cr: 4/5
    obj->power = 18;        //power: 18dbm

    obj->hp = ph;

    obj->tx_done = true;

    obj->id_key_board = id_key;
    for (int i = 0; i < 3; i++) {
        obj->id_board[i] = id[i];
    }

    Radio.SetChannel(obj->freq);

    Radio.SetTxConfig(MODEM_LORA, obj->power, 0, obj->bw, obj->sf, obj->cr, 8, 0, true, 0, 0, 0, 30);

    Radio.SetRxConfig(MODEM_LORA, obj->bw, obj->sf, obj->cr, 0, 8, 5, 0, 0, true, 0, 0, 0, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, 242);

    obj->max_payload = 242;

    LORAP2P_DEBUG("LORA P2P Version:v%d.%d.%d, BOARD ID:%x %x %x\r\n",
        LORA_P2P_VERSION_MAIN, LORA_P2P_VERSION_SUB1, LORA_P2P_VERSION_SUB2,
        obj->id_board[0], obj->id_board[1], obj->id_board[2]);

    LORAP2P_DEBUG("Default Freq:%d, Power:%d, BW:%d, SF:%d, CR:%d, TOA(242):%dms\r\n",
        obj->freq, obj->power, obj->bw, obj->sf, obj->cr, 
        Radio.TimeOnAir(MODEM_LORA, obj->bw, obj->sf, obj->cr, 8, true, 242, true));

    //Radio.Rx(100);
    lora_p2p_state_to_linkfind(obj);
}

void lora_p2p_process(lora_state_t *obj, uint8_t mode)
{
    switch(obj->state) {
    case P2P_LINK_FIND:
        lora_p2p_linkfind(obj);
        break;
    case P2P_LINK_ESTABLISHED:
        lora_p2p_raw_process(obj);
        break;
    }
}

/****************************************************************************
 * UTIL 
 ****************************************************************************/
void util_id_set_to_uid(uint32_t *dst, uint8_t *src)
{
    uint8_t *p = (uint8_t *)dst;
    for (int i = 0; i < 12; i++) {
        p[i] = src[i];
    }
}

void util_id_set_to_array(uint8_t *dst, uint32_t *src)
{
    uint8_t *p = (uint8_t *)src;
    for (int i = 0; i < 12; i++) {
        dst[i] = p[i];
    }
}

bool util_id_compare(uint32_t *a, uint8_t *b)
{
    uint8_t *p = (uint8_t *)a;
    for (int i = 0; i < 12; i++) {
        if (p[i] != b[i]) {
            return false;
        }
    }
    return true;
}

