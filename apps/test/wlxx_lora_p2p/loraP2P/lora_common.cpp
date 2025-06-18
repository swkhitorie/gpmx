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
    BOARD_DEBUG("Rx Timeout/Head Error\r\n");

    lora_p2p_callback->rx_timeout_cnt++;
    lora_p2p_callback->rx_timeout_occur = true;
}

void OnRxError() {
    BOARD_DEBUG("Rx CRC Error\r\n");

    lora_p2p_callback->rx_crc_error_cnt++;
    lora_p2p_callback->rx_crc_error_occur = true;
}

void lora_channel_set(channel_param_t *channel, uint32_t freq,
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power)
{
    channel->freq = freq;
    channel->bw = bw;
    channel->sf = sf;
    channel->cr = cr;
    channel->power = power;
}

uint32_t lora_p2p_radio_setup(channel_param_t *channel)
{
    uint8_t max_payload_sz = 242;

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);

    Radio.SetChannel(channel->freq);

    Radio.SetTxConfig(MODEM_LORA, channel->power, 0, channel->bw, channel->sf, channel->cr, 8, 0, true, 0, 0, 0, 30);

    Radio.SetRxConfig(MODEM_LORA, channel->bw, channel->sf, channel->cr, 0, 8, 5, 0, 0, true, 0, 0, 0, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, max_payload_sz);

    return max_payload_sz;
}

void lora_p2p_setup(    
    lora_state_t *obj,
    radio_p2p_role_t role,
    lora_region_t region,
    forwarding_data ph,
    authkey_generate auth,
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

    switch (region) {
    case LORA_REGION_EU868:
        region_eu868_init_default(&obj->region_grp);
        break;
    case LORA_REGION_US915:
        region_us915_init_default(&obj->region_grp);
        break;
    }

    obj->region_grp.current.freq = 923300000;  //US915
    obj->region_grp.current.bw = 2;            //bw: 500Khz
    obj->region_grp.current.sf = 5;            //sf: 7
    obj->region_grp.current.cr = 1;            //cr: 4/5
    obj->region_grp.current.power = 18;        //power: 18dbm

    obj->hp = ph;
    obj->hauth = auth;

    obj->tx_done = true;

    obj->id_key_board = id_key;
    for (int i = 0; i < 3; i++) {
        obj->id_board[i] = id[i];
    }
    obj->auth_key_board = (*obj->hauth)(obj->id_board, obj->id_key_board);
    rand_lcg_seed_set(obj->id_key_board);

    obj->max_payload = lora_p2p_radio_setup(&obj->region_grp.current);
    obj->time_on_air = Radio.TimeOnAir(MODEM_LORA, 
        obj->region_grp.current.bw, 
        obj->region_grp.current.sf, 
        obj->region_grp.current.cr, 8, true, obj->max_payload, true);

    LORAP2P_DEBUG("\r\n\tLORA P2P Version:v%d.%d.%d, BOARD ID:%x %x %x\r\n\
\tDefault Freq:%d, Power:%d, BW:%d, SF:%d, CR:%d, TOA(%d):%dms\r\n\
\tKey: %x, Auth Key: %x\r\n",
        LORA_P2P_VERSION_MAIN, LORA_P2P_VERSION_SUB1, LORA_P2P_VERSION_SUB2,
        obj->id_board[0], obj->id_board[1], obj->id_board[2],
        obj->region_grp.current.freq, 
        obj->region_grp.current.power, 
        obj->region_grp.current.bw, 
        obj->region_grp.current.sf, 
        obj->region_grp.current.cr, 
        obj->max_payload, obj->time_on_air,
        obj->id_key_board,
        obj->auth_key_board);

    //Radio.Rx(100);
    lora_p2p_state_to_linkfind(obj);
}

uint8_t lora_p2p_downchannelnext(lora_state_t *obj)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_downchannelnext(obj);
        break;
    case LORA_REGION_US915:
        region_us915_downchannelnext(obj);
        break;
    }

    return 0;
}

uint8_t lora_p2p_upchannelnext(lora_state_t *obj)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_upchannelnext(obj);
        break;
    case LORA_REGION_US915:
        region_us915_upchannelnext(obj);
        break;
    }

    return 0;
}

void lora_p2p_process(lora_state_t *obj, uint8_t mode)
{
    switch(obj->state) {
    case P2P_LINK_FIND:
        lora_p2p_linkfind(obj);
        break;
    case P2P_LINK_ESTABLISHED:
        lora_p2p_raw_ackfhss_process(obj);
        // lora_p2p_raw_ack_process(obj);
        // lora_p2p_raw_process(obj);
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

bool util_id_empty(uint32_t *uid)
{
    return (uid[0] == 0) && (uid[1] == 0) && (uid[2] == 0);
}
