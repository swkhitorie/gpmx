#include "p2p_common.h"

uint8_t p2p_channel_idx(p2p_obj_t *obj, uint32_t freq)
{
    uint8_t i = 0;
    for (i = 0; i < obj->channelgrp.list_num; i++) {
        if (freq == obj->channelgrp.ch_list[i].freq) {
            return i;
        }
    }
    return 0;
}

void p2p_radio_cfg(p2p_obj_t *obj, channel_cfg_t *channel)
{
    uint8_t max_payload = 242;
    uint8_t preamble_len = obj->channelgrp.current.preamblelen;

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);

    Radio.SetModem(MODEM_LORA);

    Radio.SetChannel(channel->freq);

    Radio.SetTxConfig(MODEM_LORA, channel->power, 0, channel->bw, channel->sf, channel->cr, preamble_len, 0, true, 0, 0, 0, 30);

    Radio.SetRxConfig(MODEM_LORA, channel->bw, channel->sf, channel->cr, 0, preamble_len, 5, 0, 0, true, 0, 0, 0, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, max_payload);

    obj->channelgrp.max_payload = max_payload;
}

void p2p_channel_cfg(channel_cfg_t *channel, uint32_t freq,
        uint8_t bw, uint8_t sf, uint8_t cr, uint8_t power)
{
    channel->freq = freq;
    channel->bw = bw;
    channel->sf = sf;
    channel->cr = cr;
    channel->power = power;
}

void p2p_start_cad(p2p_obj_t *obj)
{
    obj->cad_done = false;
    Radio.StartCad();
}

bool p2p_ischannelbusy(p2p_obj_t *obj)
{
    return obj->cad_busy;
}

bool p2p_detectchannelfree(p2p_obj_t *obj, uint32_t freq, uint32_t timeout)
{
    int timer;

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);
    Radio.SetChannel(freq);

    obj->cad_done = false;
    Radio.StartCad();
    timer = P2P_TIMESTAMP_GET();
    while (1) {
        if (obj->cad_done) {
            if (obj->cad_busy) {
                return false; // channel busy
            }
            if (P2P_ELAPSED_TIME(timer) >= timeout) {
                break; //timeout
            }
            obj->cad_done = false;
            Radio.StartCad();
        }
    }

    return true; // channel free
}

bool p2p_detect_first_freechannel(p2p_obj_t *obj, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime)
{
    int unit_timer = 0;
    int unit_time = unitScanTime; //5ms
    int total_time = totalScanTime; //3000ms
    int free_timer = freeContinuousTime; //500ms
    int total_timestamp = P2P_TIMESTAMP_GET();
    int freq = 0;
    int i_scan = 0;
    int free_cnt = 0;

    int channel_len = obj->channelgrp.downlen;
    channel_cfg_t *pchannel = obj->channelgrp.downlist;

    while (P2P_ELAPSED_TIME(total_timestamp) <= total_time) {

        freq = pchannel[i_scan].freq;
        obj->channelgrp.cad_idx = i_scan;
        bool isfree = p2p_detectchannelfree(obj, freq, unit_time);
        if (isfree) {
            free_cnt++;
            if (free_cnt >= (free_timer/unit_time)) {
                obj->channelgrp.fdown_idx = i_scan;
                return true;
            }

            continue;
        } else {
            free_cnt = 0;
            P2P_DEBUG("CAD: Busy in %d\r\n", freq);
        }

        if (i_scan >= channel_len - 1) {
            i_scan = 0;
        } else {
            i_scan++;
        }
    }

    P2P_DEBUG("WTF Error: NO Free Channel\r\n");
    return false;
}

bool p2p_ischannelfree(p2p_obj_t *obj, uint32_t freq, 
    uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime)
{
    bool status = true;
    int16_t rssi = 0;
    uint32_t carrierSenseTime = 0;

    Radio.Standby();

    Radio.SetModem(MODEM_FSK);

    Radio.SetChannel(freq);

    // Set Rx bandwidth. Other parameters are not used.
    Radio.SetRxConfig( MODEM_FSK, rxBandwidth, 600, 0, rxBandwidth, 3, 0, false,
                    0, false, 0, 0, false, true );
    Radio.Rx(0);

    P2P_TIME_DELAY(4);

    carrierSenseTime = P2P_TIMESTAMP_GET();

    // Perform carrier sense for maxCarrierSenseTime
    while (P2P_ELAPSED_TIME(carrierSenseTime) < maxCarrierSenseTime) {
        rssi = Radio.Rssi( MODEM_LORA );

        if (rssi > rssiThresh) {
            status = false;
            break;
        }
    }

    Radio.Standby();

    return status;
}

bool p2p_scan_first_freechannel(p2p_obj_t *obj, int16_t rssiThresh, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime)
{
    int rxbandwidth = 150000;
    int16_t rssi_thresh = rssiThresh;
    int unit_carrier_sense_time = unitScanTime; //5ms
    int total_scan_time = totalScanTime; //3000ms
    int free_continuous_time = freeContinuousTime; //500ms
    int total_timestamp = P2P_TIMESTAMP_GET();
    int freq = 0;
    int i_scan = 0;
    int free_cnt = 0;

    int channel_len = obj->channelgrp.downlen;
    channel_cfg_t *pchannel = obj->channelgrp.downlist;

    while (P2P_ELAPSED_TIME(total_timestamp) <= total_scan_time) {

        if (i_scan >= channel_len - 1) {
            i_scan = 0;
        } else {
            i_scan++;
        }

        freq = pchannel[i_scan].freq;
        bool isfree = p2p_ischannelfree(obj, freq, rxbandwidth, rssi_thresh, unit_carrier_sense_time);
        if (isfree) {
            free_cnt++;
            if (free_cnt >= (free_continuous_time/unit_carrier_sense_time)) {
                obj->channelgrp.fdown_idx = i_scan;
                return true;
            }

            i_scan--;
        } else {
            free_cnt = 0;
            P2P_DEBUG("SCAN: Busy in %d, rssi over %d\r\n", freq, rssiThresh);
        }

    }

    P2P_DEBUG("WTF Error: NO Free Channel\r\n");
    return false;
}

bool p2p_is_tx_done(p2p_obj_t *obj)
{
    return obj->tx_done && board_subghz_tx_ready();
}

int p2p_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size)
{
    obj->tx_done = false;
    return Radio.Send(buffer, size);
}

int p2p_lbt_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size, 
    int16_t rssiThresh, uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    int freq = obj->channelgrp.current.freq;
    uint16_t preamble_len = obj->channelgrp.current.preamblelen;
    int rxbandwidth = 150000;
    int16_t rssi_thresh = rssiThresh;
    int unit_carrier_sense_time = unitms; //5ms
    int total_scan_time = totalTimes; //50ms
    int free_continuous_time = continonousTimes; //20ms
    int total_timestamp = P2P_TIMESTAMP_GET();
    int freecnt = 0;
    int backcnt = 0;
    bool air_free = false;

    while (P2P_ELAPSED_TIME(total_timestamp) <= total_scan_time) {

        bool isfree = p2p_ischannelfree(obj, freq, rxbandwidth, rssi_thresh, unit_carrier_sense_time);
        if (isfree) {
            freecnt++;
            if (freecnt >= (free_continuous_time/unit_carrier_sense_time)) {
                air_free = true;
                break;
            }

        } else {
            obj->status.lbt_back_time += unit_carrier_sense_time;
            backcnt++;
            freecnt = 0;
        }
    }

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);

    Radio.SetModem(MODEM_LORA);

    Radio.SetChannel(freq);

    Radio.SetTxConfig(MODEM_LORA, obj->channelgrp.current.power, 0,
        obj->channelgrp.current.bw, obj->channelgrp.current.sf, obj->channelgrp.current.cr, preamble_len, 0, true, 0, 0, 0, 30);

    Radio.SetRxConfig(MODEM_LORA, obj->channelgrp.current.bw, 
        obj->channelgrp.current.sf, obj->channelgrp.current.cr, 0, preamble_len, 5, 0, 0, true, 0, 0, 0, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, obj->channelgrp.max_payload);

    // P2P_DEBUG("LBT back cnt: %d \r\n", backcnt);
    if (!air_free) {
        return RADIO_STATUS_ERROR;
    }

    obj->tx_done = false;
    return Radio.Send(buffer, size);
}

int p2p_cad_send(p2p_obj_t *obj, uint8_t *buffer, uint8_t size, 
    uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    int unit_timer = 0;
    int unit_time = unitms;
    int total_time = totalTimes;
    int free_timer = continonousTimes;
    int total_timestamp = P2P_TIMESTAMP_GET();
    int freq = obj->channelgrp.current.freq;
    int free_cnt = 0;
    int backcnt = 0;
    int freq_idx = p2p_channel_idx(obj, freq);
    bool air_free = false;

    
    while (P2P_ELAPSED_TIME(total_timestamp) <= total_time) {
        obj->channelgrp.cad_idx = freq_idx;
        bool isfree = p2p_detectchannelfree(obj, freq, unit_time);
        if (isfree) {
            free_cnt++;
            if (free_cnt >= (free_timer/unit_time)) {
                air_free = true;
                break;
            }
        } else {
            obj->status.lbt_back_time += unit_time;
            backcnt++;
            free_cnt = 0;
        }
    }

    if (!air_free) {
        return RADIO_STATUS_ERROR;
    }

    obj->tx_done = false;
    return Radio.Send(buffer, size);
}

void p2p_channelstate_reset(p2p_obj_t *obj)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_channelstate_reset(obj);
        break;
    case LORA_REGION_US915:
        region_us915_channelstate_reset(obj);
        break;
    }

    return;
}

uint8_t p2p_downchannelnext(p2p_obj_t *obj, int16_t rssi, int8_t snr)
{
    switch (obj->region) {
    case LORA_REGION_EU868:
        region_eu868_downchannelnext(obj, rssi, snr);
        break;
    case LORA_REGION_US915:
        region_us915_downchannelnext(obj, rssi, snr);
        break;
    }

    return 0;
}

uint8_t p2p_upchannelnext(p2p_obj_t *obj)
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

int fcl_cal(p2p_obj_t *obj, int verify_sz)
{
    int wsz = 0;
    int rsz = 0;

    if (obj->flowctrl_bytes_max_ones <= 0) {
        return obj->channelgrp.max_payload;
    }

    if (obj->flowctrl_bytes_snd == 0) {
        obj->flowctrl_bytes_snd = obj->flowctrl_bytes_max_ones;
        obj->status.flowctrl_timestamp = P2P_TIMESTAMP_GET();
        P2P_DEBUG("first resume\r\n");
    }

    if (P2P_ELAPSED_TIME(obj->status.flowctrl_timestamp) > 1000) {
        obj->flowctrl_bytes_snd = obj->flowctrl_bytes_max_ones;
        obj->status.flowctrl_timestamp = P2P_TIMESTAMP_GET();
        P2P_DEBUG("resume bytes, and start cnt\r\n");
    }

    wsz = (obj->flowctrl_bytes_snd > obj->channelgrp.max_payload) ? 
        obj->channelgrp.max_payload : 
        obj->flowctrl_bytes_snd;

    rsz = wsz - verify_sz;
    if (rsz < verify_sz) {
        // P2P_DEBUG("Remain flow can't send rtcm data except heading \r\n");
        return 0;
    } 

    // P2P_DEBUG("debug1 %d %d %d \r\n", rsz, wsz, verify_sz);
    return rsz;
}

void fcl_sndbytes(p2p_obj_t *obj)
{
    if (obj->flowctrl_bytes_max_ones <= 0) {
        return;
    }
    obj->flowctrl_bytes_snd -= obj->status.nbytes_send;
    return;
}
