#include "p2p_common.h"

struct __p2p_obj *_p2p_lnk;

void p2p_platform_obj_bind(struct __p2p_obj *obj)
{
    _p2p_lnk = obj;
}

/****************************************************************************
 * stm32wlxx radio driver
 ****************************************************************************/
static RadioEvents_t _radio_events;

void CadDone(bool channelActivityDetected) {

    //true: channel busy, false: channel free
    if (channelActivityDetected) {
        p2p_warning("CAD Busy\r\n");
    }

    _p2p_lnk->hw.cad_done = true;
    _p2p_lnk->hw.cad_busy = channelActivityDetected;
}

void OnTxDone() {
    _p2p_lnk->hw.tx_done = true;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo) {

    if (_p2p_lnk->_role == P2P_SENDER) {
        _p2p_lnk->_up_ch_rssi = rssi;
        _p2p_lnk->_up_ch_snr = LoraSnr_FskCfo;
    } else if (_p2p_lnk->_role == P2P_RECEIVER) {
        _p2p_lnk->_dw_ch_rssi = rssi;
        _p2p_lnk->_dw_ch_snr = LoraSnr_FskCfo;
    }

    if (size > 0) {
        prb_write(&_p2p_lnk->_prbuf, payload, size);
    }
}

void OnTxTimeout() {
    p2p_warning("Tx Timeout\r\n");
    _p2p_lnk->hw.tx_timeout_cnt++;
    _p2p_lnk->hw.tx_timeout_occur = true;
}

void OnRxTimeout() {
    p2p_warning("Rx Timeout/Head Error\r\n");
    _p2p_lnk->hw.rx_timeout_cnt++;
    _p2p_lnk->hw.rx_timeout_occur = true;
}

void OnRxError() {
    p2p_warning("Rx CRC Error\r\n");
    _p2p_lnk->hw.rx_crc_error_cnt++;
    _p2p_lnk->hw.rx_crc_error_occur = true;
}

void p2p_platform_setup(struct __p2p_obj *obj, uint32_t cfg_freq, 
    uint8_t cfg_bw, uint8_t cfg_sf, uint8_t cfg_cr, 
    uint8_t cfg_power, uint16_t cfg_preamblelen, uint8_t max_payloadlen)
{
    _radio_events.TxDone = OnTxDone;
    _radio_events.RxDone = OnRxDone;
    _radio_events.TxTimeout = OnTxTimeout;
    _radio_events.RxTimeout = OnRxTimeout;
    _radio_events.RxError = OnRxError;
    _radio_events.CadDone = CadDone;
    Radio.Init(&_radio_events);

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);

    Radio.SetModem(MODEM_LORA);

    Radio.SetChannel(cfg_freq);

    Radio.SetTxConfig(MODEM_LORA, cfg_power, 0, cfg_bw, cfg_sf, cfg_cr, cfg_preamblelen, 0, true, 0, 0, 0, 30);

    Radio.SetRxConfig(MODEM_LORA, cfg_bw, cfg_sf, cfg_cr, 0, cfg_preamblelen, 5, 0, 0, true, 0, 0, 0, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, max_payloadlen);

    obj->_max_payload = max_payloadlen;
}

void p2p_wait_to_idle(struct __p2p_obj *obj)
{
    while (Radio.GetStatus() != RF_IDLE);
}

bool p2p_is_tx_done(struct __p2p_obj *obj)
{
    return obj->hw.tx_done && board_subghz_tx_ready();
}

void p2p_set_standby(struct __p2p_obj *obj)
{
    Radio.Standby();
    p2p_wait_to_idle(obj);
}

void p2p_set_channel(struct __p2p_obj *obj, uint32_t freq)
{
    Radio.SetChannel(freq);
}

void p2p_rx(struct __p2p_obj *obj, uint32_t timeout)
{
    Radio.Rx(timeout);
}

int p2p_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size)
{
    obj->hw.tx_done = false;
    return Radio.Send(buffer, size);
}

int p2p_cad_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    int unit_timer = 0;
    int unit_time = unitms;
    int total_time = totalTimes;
    int free_timer = continonousTimes;
    int total_timestamp = P2P_TIMESTAMP_GET();
    int freq = obj->_ch_grp.current.freq;
    int free_cnt = 0;
    int backcnt = 0;
    int freq_idx = p2p_channel_fq_get_idx(&obj->_ch_grp, freq);
    bool air_free = false;
    
    while (P2P_ELAPSED_TIME(total_timestamp) <= total_time) {
        obj->_cad_idx = freq_idx;
        bool isfree = p2p_cad_detect_channel_free(obj, freq, unit_time);
        if (isfree) {
            free_cnt++;
            if (free_cnt >= (free_timer/unit_time)) {
                air_free = true;
                break;
            }
        } else {
            obj->_status.lbt_back_time += unit_time;
            backcnt++;
            free_cnt = 0;
        }
    }

    if (!air_free) {
        return RADIO_STATUS_ERROR;
    }

    return p2p_send(obj, buffer, size);
}

int p2p_lbt_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    int16_t rssiThresh, uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    // int freq = obj->channelgrp.current.freq;
    // uint16_t preamble_len = obj->channelgrp.current.preamblelen;
    // int rxbandwidth = 150000;
    // int16_t rssi_thresh = rssiThresh;
    // int unit_carrier_sense_time = unitms; //5ms
    // int total_scan_time = totalTimes; //50ms
    // int free_continuous_time = continonousTimes; //20ms
    // int total_timestamp = P2P_TIMESTAMP_GET();
    // int freecnt = 0;
    // int backcnt = 0;
    // bool air_free = false;

    // while (P2P_ELAPSED_TIME(total_timestamp) <= total_scan_time) {

    //     bool isfree = p2p_lbt_is_channel_free(obj, freq, rxbandwidth, rssi_thresh, unit_carrier_sense_time);
    //     if (isfree) {
    //         freecnt++;
    //         if (freecnt >= (free_continuous_time/unit_carrier_sense_time)) {
    //             air_free = true;
    //             break;
    //         }

    //     } else {
    //         obj->status.lbt_back_time += unit_carrier_sense_time;
    //         backcnt++;
    //         freecnt = 0;
    //     }
    // }

    // Radio.Standby();
    // while (Radio.GetStatus() != RF_IDLE);

    // Radio.SetModem(MODEM_LORA);

    // Radio.SetChannel(freq);

    // Radio.SetTxConfig(MODEM_LORA, obj->channelgrp.current.power, 0,
    //     obj->channelgrp.current.bw, obj->channelgrp.current.sf, obj->channelgrp.current.cr, preamble_len, 0, true, 0, 0, 0, 30);

    // Radio.SetRxConfig(MODEM_LORA, obj->channelgrp.current.bw, 
    //     obj->channelgrp.current.sf, obj->channelgrp.current.cr, 0, preamble_len, 5, 0, 0, true, 0, 0, 0, true);

    // Radio.SetMaxPayloadLength(MODEM_LORA, obj->channelgrp.max_payload);

    // // p2p_debug("LBT back cnt: %d \r\n", backcnt);

    // if (!air_free) {
    //     return RADIO_STATUS_ERROR;
    // }

    // obj->hw.tx_done = false;
    // return Radio.Send(buffer, size);
}

bool p2p_cad_is_channel_busy(struct __p2p_obj *obj)
{
    return obj->hw.cad_busy;
}

/****************************************************************************
 * LBT(CAD) method: Channel Activity Detect
 ****************************************************************************/
void p2p_cad_start(struct __p2p_obj *obj)
{
    obj->hw.cad_done = false;
    Radio.StartCad();
}

bool p2p_cad_detect_channel_free(struct __p2p_obj *obj, uint32_t freq, uint32_t timeout)
{
    int timer;

    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);
    Radio.SetChannel(freq);

    obj->hw.cad_done = false;
    Radio.StartCad();
    timer = P2P_TIMESTAMP_GET();
    while (1) {
        if (obj->hw.cad_done) {
            if (obj->hw.cad_busy) {
                return false; // channel busy
            }
            if (P2P_ELAPSED_TIME(timer) >= timeout) {
                break; //timeout
            }
            obj->hw.cad_done = false;
            Radio.StartCad();
        }
    }

    return true; // channel free
}

bool p2p_cad_scan_first_free_channel(struct __p2p_obj *obj, uint32_t unitScanTime,
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

    int channel_len = obj->_ch_grp.dw_ch_len;
    struct __channel_cfg *pchannel = obj->_ch_grp.dw_list;

    while (P2P_ELAPSED_TIME(total_timestamp) <= total_time) {

        freq = pchannel[i_scan].freq;
        obj->_cad_idx = i_scan;
        bool isfree = p2p_cad_detect_channel_free(obj, freq, unit_time);
        if (isfree) {
            free_cnt++;
            if (free_cnt >= (free_timer/unit_time)) {
                obj->_ch_grp.dw_fq_idx_fix = i_scan;
                return true;
            }

            continue;
        } else {
            free_cnt = 0;

            p2p_debug("CAD: Busy in %d\r\n", freq);

        }

        if (i_scan >= channel_len - 1) {
            i_scan = 0;
        } else {
            i_scan++;
        }
    }

    p2p_debug("WTF Error: NO Free Channel\r\n");

    return false;
}

/****************************************************************************
 * LBT(FSK) method: Carrier Sense, Read RSSI
 ****************************************************************************/
bool p2p_lbt_is_channel_free(struct __p2p_obj *obj, uint32_t freq, 
    uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime)
{
    bool status = false;
    // int16_t rssi = 0;
    // uint32_t carrierSenseTime = 0;

    // Radio.Standby();

    // Radio.SetModem(MODEM_FSK);

    // Radio.SetChannel(freq);

    // // Set Rx bandwidth. Other parameters are not used.
    // Radio.SetRxConfig( MODEM_FSK, rxBandwidth, 600, 0, rxBandwidth, 3, 0, false,
    //                 0, false, 0, 0, false, true );
    // Radio.Rx(0);

    // P2P_TIME_DELAY(4);

    // carrierSenseTime = P2P_TIMESTAMP_GET();

    // // Perform carrier sense for maxCarrierSenseTime
    // while (P2P_ELAPSED_TIME(carrierSenseTime) < maxCarrierSenseTime) {
    //     rssi = Radio.Rssi( MODEM_LORA );

    //     if (rssi > rssiThresh) {
    //         status = false;
    //         break;
    //     }
    // }

    // Radio.Standby();

    return status;
}

bool p2p_lbt_scan_first_free_channel(struct __p2p_obj *obj, int16_t rssiThresh, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime)
{
    // int rxbandwidth = 150000;
    // int16_t rssi_thresh = rssiThresh;
    // int unit_carrier_sense_time = unitScanTime; //5ms
    // int total_scan_time = totalScanTime; //3000ms
    // int free_continuous_time = freeContinuousTime; //500ms
    // int total_timestamp = P2P_TIMESTAMP_GET();
    // int freq = 0;
    // int i_scan = 0;
    // int free_cnt = 0;

    // int channel_len = obj->channelgrp.downlen;
    // channel_cfg_t *pchannel = obj->channelgrp.downlist;

    // while (P2P_ELAPSED_TIME(total_timestamp) <= total_scan_time) {

    //     if (i_scan >= channel_len - 1) {
    //         i_scan = 0;
    //     } else {
    //         i_scan++;
    //     }

    //     freq = pchannel[i_scan].freq;
    //     bool isfree = p2p_lbt_is_channel_free(obj, freq, rxbandwidth, rssi_thresh, unit_carrier_sense_time);
    //     if (isfree) {
    //         free_cnt++;
    //         if (free_cnt >= (free_continuous_time/unit_carrier_sense_time)) {
    //             obj->channelgrp.fdown_idx = i_scan;
    //             return true;
    //         }

    //         i_scan--;
    //     } else {
    //         free_cnt = 0;

    //         p2p_debug("SCAN: Busy in %d, rssi over %d\r\n", freq, rssiThresh);

    //     }

    // }

    // p2p_debug("WTF Error: NO Free Channel\r\n");

    return false;
}

