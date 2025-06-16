#include "lora_p2p_proto.h"

/****************************************************************************
 * Request-Connect 
 ****************************************************************************/
int decode_req_connect(rtcm_t *rtcm, req_connect_t *req_connect)
{
    int ret = 1;
    int payload_len = 17;

    int typid = getbitu(rtcm->buff, 24, 8);

    for (int i = 0; i < 12; i++) {
        req_connect->rcv_id[i] = rtcm->buff[4+i];
    }

    req_connect->rcv_key = getbitu(rtcm->buff, 128, 32);

    if (typid == 0x12 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_req_connect(uint8_t *p, req_connect_t *req_connect)
{
    int payload_len = 17;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x12); //req_connect->typid
    for (int i = 0; i < 12; i++) {
        p[4+i] = req_connect->rcv_id[i];
    }
    setbitu(&p[0], 128, 32, req_connect->rcv_key);

    setbitu(&p[0], 160, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Request-Allow 
 ****************************************************************************/
int decode_req_allow(rtcm_t *rtcm, req_allow_t *req_allow)
{
    int ret = 1;
    int payload_len = 33;

    int typid = getbitu(rtcm->buff, 24, 8);
    req_allow->typid = typid;

    for (int i = 0; i < 12; i++) {
        req_allow->rcv_id[i] = rtcm->buff[4+i];
    }
    for (int i = 0; i < 12; i++) {
        req_allow->snd_id[i] = rtcm->buff[16+i];
    }

    req_allow->rcv_key = getbitu(rtcm->buff, 224, 32);
    req_allow->snd_key = getbitu(rtcm->buff, 256, 32);

    if (typid == 0x14 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_req_allow(uint8_t *p, req_allow_t *req_allow)
{
    int payload_len = 33;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x14); //req_allow->typid
    for (int i = 0; i < 12; i++) {
        p[4+i] = req_allow->rcv_id[i];
    }
    for (int i = 0; i < 12; i++) {
        p[16+i] = req_allow->snd_id[i];
    }
    setbitu(&p[0], 224, 32, req_allow->rcv_key);
    setbitu(&p[0], 256, 32, req_allow->snd_key);

    setbitu(&p[0], 288, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Connect-Verify 
 ****************************************************************************/
int decode_connect_verify(rtcm_t *rtcm, uint32_t *freq,
        uint8_t *bw, uint8_t *sf, uint8_t *cr, uint8_t *txpower, 
        uint8_t *rcv_id, uint32_t *tick)
{
    int ret = 1;
    int payload_len = 23;

    int subid = getbitu(rtcm->buff, 24, 12);
    int typid = getbitu(rtcm->buff, 36, 8);
    *freq = getbitu(rtcm->buff, 44, 20);
    *bw = getbitu(rtcm->buff, 64, 4);
    *sf = getbitu(rtcm->buff, 68, 4);
    *cr = getbitu(rtcm->buff, 72, 4);
    *txpower = getbitu(rtcm->buff, 76, 4);

    for (int i = 0; i < 12; i++) {
        rcv_id[i] = rtcm->buff[10+i];
    }

    *tick = getbitu(rtcm->buff, 176, 32);

    if (subid == 666 && typid == 0x24 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_connect_verify(uint8_t *p, uint32_t freq, 
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t txpower, 
        uint8_t *rcv_id, uint32_t tick)
{
    int payload_len = 23;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 12, 666);  // subid
    setbitu(&p[0], 36, 8, 0x24);  // typeid
    setbitu(&p[0], 44, 20, freq);
    setbitu(&p[0], 64, 4, bw);
    setbitu(&p[0], 68, 4, sf);
    setbitu(&p[0], 72, 4, cr);
    setbitu(&p[0], 76, 4, txpower);

    for (int i = 0; i < 12; i++) {
        p[10+i] = rcv_id[i];
    }

    setbitu(&p[0], 176, 32, tick);

    setbitu(&p[0], 208, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Connect-Result 
 ****************************************************************************/
int decode_connect_result(rtcm_t *rtcm, uint32_t *freq,
        int16_t *rssi, int8_t *snr, uint32_t *tickack, uint8_t *snd_id)
{
    int ret = 1;
    int payload_len = 24;

    int subid = getbitu(rtcm->buff, 24, 12);
    int typid = getbitu(rtcm->buff, 36, 8);
    *freq = getbitu(rtcm->buff, 44, 20);
    *rssi = getbits(rtcm->buff, 64, 16);
    *snr = getbits(rtcm->buff, 80, 8);

    for (int i = 0; i < 12; i++) {
        snd_id[i] = rtcm->buff[11+i];
    }

    *tickack = getbitu(rtcm->buff, 184, 32);

    if (subid == 666 && typid == 0x22 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_connect_result(uint8_t *p, uint32_t freq,
        int16_t rssi, int8_t snr, uint32_t tickack, uint8_t *snd_id)
{
    int payload_len = 24;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 12, 666);  // subid
    setbitu(&p[0], 36, 8, 0x22);  // typeid
    setbitu(&p[0], 44, 20, freq);
    setbits(&p[0], 64, 16, rssi);
    setbits(&p[0], 80, 8, snr);

    for (int i = 0; i < 12; i++) {
        p[11+i] = snd_id[i];
    }

    setbitu(&p[0], 184, 32, tickack);

    setbitu(&p[0], 216, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

