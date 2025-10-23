#include "p2p_common.h"
#include "sx126x_driver.h"

struct __p2p_obj *_p2p_lnk;

void p2p_platform_obj_bind(struct __p2p_obj *obj)
{
    _p2p_lnk = obj;
}

void sx126x_tx_done_callback()
{
    _p2p_lnk->hw.tx_done = true;
}

void sx126x_cad_done_callback(uint8_t detected)
{
    _p2p_lnk->hw.cad_done = true;
    _p2p_lnk->hw.cad_busy = detected;
    
    if (detected == 1) {
        p2p_warning("cad busy in %d\r\n", _p2p_lnk->_ch_grp.dw_list[_p2p_lnk->_cad_idx].freq);
    }

}

void sx126x_rx_done_callback(uint8_t *payload, uint16_t size, 
    int16_t lastpack_rssi, int16_t aver_rssi, int16_t snr)
{
    if (_p2p_lnk->_role == P2P_SENDER) {
        _p2p_lnk->_up_ch_rssi = lastpack_rssi;
        _p2p_lnk->_up_ch_snr = snr;
    } else if (_p2p_lnk->_role == P2P_RECEIVER) {
        _p2p_lnk->_dw_ch_rssi = lastpack_rssi;
        _p2p_lnk->_dw_ch_snr = snr;
    }

    if (size > 0) {
        prb_write(&_p2p_lnk->_prbuf, payload, size);
    }
}

void p2p_platform_setup(struct __p2p_obj *obj, uint32_t cfg_freq, 
    uint8_t cfg_bw, uint8_t cfg_sf, uint8_t cfg_cr, 
    uint8_t cfg_power, uint16_t cfg_preamblelen, uint8_t max_payloadlen)
{
    int ret = 0;

    G_LoRaConfig.HeaderType = LORA_PACKET_EXPLICIT;
    G_LoRaConfig.CrcMode = LORA_CRC_ON;
    G_LoRaConfig.InvertIQ = LORA_IQ_NORMAL;

    G_LoRaConfig.LoRa_Freq = cfg_freq;
    switch (cfg_bw) {
        case 0: G_LoRaConfig.BandWidth = LORA_BW_125; break;
        case 1: G_LoRaConfig.BandWidth = LORA_BW_250; break;
        case 2: G_LoRaConfig.BandWidth = LORA_BW_500; break;
    }
    G_LoRaConfig.SpreadingFactor = (uint8_t)cfg_sf;
    G_LoRaConfig.CodingRate = (uint8_t)cfg_cr;
    G_LoRaConfig.PowerCfig = cfg_power;
    G_LoRaConfig.PreambleLength = cfg_preamblelen;
    G_LoRaConfig.PayloadLength = max_payloadlen;

    sx126x_hal_init();
    ret = sx126x_lora_init();
    if (ret != NORMAL) {
        while (1) {
            p2p_debug("sx126x lora init failed %d\r\n", ret);
        }
    }
    p2p_debug("sx126x lora init success\r\n");

    obj->_max_payload = max_payloadlen;
}

void p2p_wait_to_idle(struct __p2p_obj *obj)
{
    while (sx126x_get_opmode() == MODE_TX ||
           sx126x_get_opmode() == MODE_RX ||
           sx126x_get_opmode() == MODE_CAD);
}

bool p2p_is_tx_done(struct __p2p_obj *obj)
{
    return obj->hw.tx_done;
}

void p2p_set_standby(struct __p2p_obj *obj)
{
    sx126x_calibrate_error();
    sx126x_set_standby(STDBY_RC);
}

void p2p_set_channel(struct __p2p_obj *obj, uint32_t freq)
{
    sx126x_set_rf_freq(freq);
}

void p2p_set_power(struct __p2p_obj *obj, uint8_t power)
{
    sx126x_set_rf_tx_power(power);
}

void p2p_rx(struct __p2p_obj *obj, uint32_t timeout)
{
    if (timeout == 0) {
        sx126x_start_rx(0xffffff);
    } else {
        sx126x_start_rx(0x2fffff);
    }
}

int p2p_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size)
{
    _p2p_lnk->hw.tx_done = false;
    G_LoRaConfig.PayloadLength = size;
    sx126x_tx_packet(buffer);
}

int p2p_cad_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    return 0;
}

int p2p_lbt_send(struct __p2p_obj *obj, uint8_t *buffer, uint8_t size, 
    int16_t rssiThresh, uint16_t unitms, uint8_t continonousTimes, uint32_t totalTimes)
{
    return 0;
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
    sx126x_cad_init();
    sx126x_cad_sample();
}

bool p2p_cad_detect_channel_free(struct __p2p_obj *obj, uint32_t freq, uint32_t timeout)
{
    int timer;

    sx126x_calibrate_error();
    sx126x_set_standby(STDBY_RC);
    sx126x_set_rf_freq(freq);

    obj->hw.cad_done = false;
    sx126x_cad_init();
    sx126x_cad_sample();

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
            sx126x_cad_init();
            sx126x_cad_sample();
        }
    }

    return true;
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

    return false;
}

/****************************************************************************
 * LBT(FSK) method: Carrier Sense, Read RSSI
 ****************************************************************************/
bool p2p_lbt_is_channel_free(struct __p2p_obj *obj, uint32_t freq, 
    uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime)
{
    bool status = false;

    return status;
}

bool p2p_lbt_scan_first_free_channel(struct __p2p_obj *obj, int16_t rssiThresh, uint32_t unitScanTime,
    uint32_t freeContinuousTime, uint32_t totalScanTime)
{

    return false;
}

