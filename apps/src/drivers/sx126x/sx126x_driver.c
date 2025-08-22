#include "sx126x_driver.h"

/**
    LSD4RFC_2L722N10
    LSD4RFC_2L714N10
    L_LRMRL22_97NN4
 */

#define VERSION        L_LRMRL22_97NN4     //product num

float G_BandWidthKHz = 500.0;
float G_TsXms = 1.024;
S_LoRaPara G_LoRaPara;
S_LoRaConfig G_LoRaConfig = {
    470000000,
    LORA_BW_125,
    LORA_SF7,
    LORA_CR_4_5,
    22,
    LORA_PACKET_EXPLICIT,
    LORA_CRC_ON,
    LORA_IQ_NORMAL,
    8,
    64,
};

static RadioOperatingModes_t g_opmode;
static uint8_t radio_rx_buffer[SX126X_RX_BUFFER_LEN];
static uint8_t usr_rx_continous_mode = 0x00;

/****************************************************************************
 * Basic Function Interface 
 ****************************************************************************/
void sx126x_wakeup()
{
    uint8_t datasnd[2] = {RADIO_GET_STATUS, 0x00};
    sx126x_hal_nss_set(0);
    sx126x_hal_exchange_byte(RADIO_GET_STATUS);
    sx126x_hal_exchange_byte(0x00);
    sx126x_hal_nss_set(1);

    sx126x_hal_wait_onbusy();
}

RadioOperatingModes_t sx126x_get_opmode()
{
    return g_opmode;
}

void sx126x_check_ready()
{
    if (sx126x_get_opmode() == MODE_SLEEP ||
        sx126x_get_opmode() == MODE_RX_DC) {
        sx126x_wakeup();
    }
    sx126x_wakeup();
}

void sx126x_write_cmd(RadioCommands_t cmd, uint8_t *p, uint16_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte((uint8_t)cmd);
    for(i = 0; i < size; i++) {
        sx126x_hal_exchange_byte(p[i]);
    }

    sx126x_hal_nss_set(1);
    if (cmd != RADIO_SET_SLEEP) {
        sx126x_hal_wait_onbusy();
    }
}

void sx126x_read_cmd(RadioCommands_t cmd, uint8_t *p, uint16_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte((uint8_t)cmd);
    sx126x_hal_exchange_byte(0);
    for (i = 0; i < size; i++) {
        p[i] = sx126x_hal_exchange_byte(0);
    }

    sx126x_hal_nss_set(1);
    sx126x_hal_wait_onbusy();
}


void sx126x_write_registers(uint16_t addr, uint8_t *p, uint16_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte(RADIO_WRITE_REGISTER);
    sx126x_hal_exchange_byte((addr & 0xFF00) >> 8);
    sx126x_hal_exchange_byte(addr & 0x00FF);
    for (i = 0; i < size; i++) {
        sx126x_hal_exchange_byte(p[i]);
    }

    sx126x_hal_nss_set(1);
    sx126x_hal_wait_onbusy();
}

void sx126x_write_register(uint16_t addr, uint8_t val)
{
    sx126x_write_registers(addr, &val, 1);
}

void sx126x_read_registers(uint16_t addr, uint8_t *p, uint16_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte(RADIO_READ_REGISTER);
    sx126x_hal_exchange_byte((addr & 0xFF00) >> 8);
    sx126x_hal_exchange_byte(addr & 0x00FF);
    sx126x_hal_exchange_byte(0);
    for (i = 0; i < size; i++) {
        p[i] = sx126x_hal_exchange_byte(0);
    }

    sx126x_hal_nss_set(1);
    sx126x_hal_wait_onbusy();
}

uint8_t sx126x_read_register(uint16_t addr)
{
    uint8_t val;
    sx126x_read_registers(addr, &val, 1);
    return val;
}

void sx126x_write_buffer(uint8_t offset, uint8_t *p, uint8_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte(RADIO_WRITE_BUFFER);
    sx126x_hal_exchange_byte(offset);
    for (i = 0; i < size; i++) {
        sx126x_hal_exchange_byte(p[i]);
    }

    sx126x_hal_nss_set(1);
    sx126x_hal_wait_onbusy();
}


void sx126x_read_buffer(uint8_t offset, uint8_t *p, uint8_t size)
{
    uint16_t i = 0;
    sx126x_check_ready();
    sx126x_hal_nss_set(0);

    sx126x_hal_exchange_byte(RADIO_READ_BUFFER);
    sx126x_hal_exchange_byte(offset);
    sx126x_hal_exchange_byte(0);
    for (i = 0; i < size; i++) {
        p[i] = sx126x_hal_exchange_byte(0);
    }

    sx126x_hal_nss_set(1);
    sx126x_hal_wait_onbusy();
}

void sx126x_write_fifo(uint8_t *p, uint8_t size)
{
    sx126x_write_buffer(0, p, size);
}

void sx126x_read_fifo(uint8_t *p, uint8_t size)
{
    sx126x_read_buffer(0, p, size);
}

/****************************************************************************
 * Setting Function Interface 
 ****************************************************************************/

void sx126x_reset()
{
    SX126X_MS_DELAY(10);
    sx126x_hal_reset_set(0);
    SX126X_MS_DELAY(20);
    sx126x_hal_reset_set(1);
    SX126X_MS_DELAY(10);
}

void sx126x_write_rx_tx(bool tx_enable)
{
    if (tx_enable) {
        // switch to TX mode
        sx126x_hal_swctl_1_set(0);
        sx126x_hal_swctl_2_set(1);
    } else {
        // switch to Rx mode
        sx126x_hal_swctl_1_set(1);
        sx126x_hal_swctl_2_set(0);
    }
}

void sx126x_switch_off(void)
{
    sx126x_hal_swctl_1_set(0);
    sx126x_hal_swctl_1_set(0);
}

void sx126x_set_standby(RadioStandbyModes_t standbyConfig)
{
		RadioStatus_t status;
    unsigned int n_times = 65535;
    sx126x_write_cmd(RADIO_SET_STANDBY, (uint8_t *)&standbyConfig, 1);
		status = sx126x_get_status();
	
    if (standbyConfig == STDBY_RC) {
        do {
            status = sx126x_get_status();
            n_times--;
        } while((status.Fields.ChipMode != 0X02) && n_times);
        g_opmode = MODE_STDBY_RC;
    } else {
        do {
            status = sx126x_get_status();
            n_times--;
        } while((status.Fields.ChipMode != 0X03) && n_times);
        g_opmode = MODE_STDBY_XOSC;
    }
}

void sx126x_sleep()
{	 
    SleepParams_t params = {0};
    params.Fields.WarmStart = 1;
    sx126x_set_sleep(params);
    sx126x_switch_off();
}

void sx126x_set_sleep(SleepParams_t sleepConfig)
{
    sx126x_write_cmd(RADIO_SET_SLEEP, &sleepConfig.Value, 1);
    g_opmode = MODE_SLEEP;
}

void sx126x_set_packet_type(RadioPacketTypes_t packetType)
{
    // Save packet type internally to avoid questioning the radio
    if (packetType == PACKET_TYPE_GFSK) {
        sx126x_write_register(REG_BIT_SYNC, 0x00);
    }

    sx126x_write_cmd(RADIO_SET_PACKETTYPE, (uint8_t *)&packetType, 1);
}

void sx126x_set_regulator_mode(RadioRegulatorMode_t mode)
{
    sx126x_write_cmd(RADIO_SET_REGULATORMODE, (uint8_t *)&mode, 1);
}

void sx126x_set_buffer_base_address(uint8_t txBaseAddress, uint8_t rxBaseAddress)
{
    uint8_t buf[2];

    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;
    sx126x_write_cmd(RADIO_SET_BUFFERBASEADDRESS, buf, 2);
}

void sx126x_calibrate(CalibrationParams_t calibParam)
{
    sx126x_write_cmd(RADIO_CALIBRATE, (uint8_t *)&calibParam, 1);
}

void sx126x_calibrate_error()
{
		RadioError_t error;
    CalibrationParams_t calibParam;

		error = sx126x_get_device_errors();
		if (error.Value != 0) {
        sx126x_clear_device_errors();
        calibParam.Value = 0x7F;
        sx126x_calibrate(calibParam);
		}
}	

void sx126x_calibrate_image(uint32_t freq)
{
    uint8_t cal_freq[2];

    if (freq > 900000000) {

        cal_freq[0] = 0xE1;
        cal_freq[1] = 0xE9;
    } else if(freq > 850000000) {

        cal_freq[0] = 0xD7;
        cal_freq[1] = 0xD8;
    } else if(freq > 770000000) {

        cal_freq[0] = 0xC1;
        cal_freq[1] = 0xC5;
    } else if(freq > 460000000) {

        cal_freq[0] = 0x75;
        cal_freq[1] = 0x81;
    } else if(freq > 425000000) {

        cal_freq[0] = 0x6B;
        cal_freq[1] = 0x6F;
    }

    sx126x_write_cmd(RADIO_CALIBRATEIMAGE, cal_freq, 2);
}

tSX126xError sx126x_set_rf_freq(uint32_t freq)
{
    uint8_t buf[4];
    uint32_t freq_cal = 0;

    if((freq > 960000000) || (freq < 410000000)) {
        return PARAMETER_INVALID;
    }

    sx126x_calibrate_image(freq);

    freq_cal = (uint32_t)((double)freq / (double)FREQ_STEP);
    buf[0] = (uint8_t)((freq_cal >> 24) & 0xFF);
    buf[1] = (uint8_t)((freq_cal >> 16) & 0xFF);
    buf[2] = (uint8_t)((freq_cal >> 8) & 0xFF);
    buf[3] = (uint8_t)(freq_cal & 0xFF);
    sx126x_write_cmd(RADIO_SET_RFFREQUENCY, buf, 4);

    return NORMAL;
}

void sx126x_set_dio_irq_params(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask)
{
    uint8_t buf[8];

    buf[0] = (uint8_t)((irqMask >> 8) & 0x00FF);
    buf[1] = (uint8_t)(irqMask & 0x00FF);
    buf[2] = (uint8_t)((dio1Mask >> 8) & 0x00FF);
    buf[3] = (uint8_t)(dio1Mask & 0x00FF);
    buf[4] = (uint8_t)((dio2Mask >> 8) & 0x00FF);
    buf[5] = (uint8_t)(dio2Mask & 0x00FF);
    buf[6] = (uint8_t)((dio3Mask >> 8) & 0x00FF);
    buf[7] = (uint8_t)(dio3Mask & 0x00FF);

    sx126x_write_cmd(RADIO_CFG_DIOIRQ, buf, 8);
}

void sx126x_set_paconnfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut)
{
    uint8_t buf[4];

    buf[0] = paDutyCycle;
    buf[1] = hpMax;
    buf[2] = deviceSel;
    buf[3] = paLut;

    sx126x_write_cmd(RADIO_SET_PACONFIG, buf, 4);
}


#if (VERSION==LSD4RFC_2L714N10)
/**
  * Transmitting power config 
  * this function is configured in conjunction with hardware SD4RFC-2L714N10 to 
  *      consider power consumption and optimal power settings;
  * power: -3 ~ 15
  */

void sx126x_set_tx_params(int8_t power, RadioRampTimes_t rampTime)
{
    uint8_t buf[2];

    if (power >= 14) {
        power = 15;
    } else if (power < -3) {
        power = -3;
    }	 

    sx126x_set_paconnfig(0x04, 0x06, 0x00, 0x01);
		
    sx126x_write_register(REG_OCP, 0x38); // current max 140mA for the whole device

    buf[0] = power;
    buf[1] = (uint8_t)rampTime;

    sx126x_write_cmd(RADIO_SET_TXPARAMS, buf, 2);
}

#else
/**
  * Transmitting power config 
  * this function is configured in conjunction with hardware SD4RFC-2L714N10 to 
  *      consider power consumption and optimal power settings;
  * power: -3 ~ 22
  */
void sx126x_set_tx_params(int8_t power, RadioRampTimes_t rampTime)
{
    uint8_t buf[2];

    if (power > 22) {
        power = 22;
    } else if (power < -3) {
        power = -3;
    }	 

    sx126x_set_paconnfig(0x04, 0x07, 0x00, 0x01);
		
    sx126x_write_register(REG_OCP, 0x38); // current max 140mA for the whole device

    buf[0] = power;
    buf[1] = (uint8_t)rampTime;

    sx126x_write_cmd(RADIO_SET_TXPARAMS, buf, 2);
}
#endif


void sx126x_set_rf_tx_power(int8_t power)
{
    sx126x_set_tx_params(power, RADIO_RAMP_800_US);
}

/**
  * Modulation parameter setting, SF/BW/CR
*/
void sx126x_set_lora_modulation_params()
{
    uint8_t n;

    uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00};

    n = 4;
    buf[0] = G_LoRaConfig.SpreadingFactor;
    buf[1] = G_LoRaConfig.BandWidth;
    buf[2] = G_LoRaConfig.CodingRate;

		if (G_TsXms > 16.0) {
        buf[3] = LOWDATARATEOPTIMIZE_ON;
    } else {
        buf[3] = LOWDATARATEOPTIMIZE_OFF;
    }

		
    // when in LoRa Mode
    // BW == 500KHz, register in 0x0889:bits[1] is 0
    // other BW or other mode, register in 0x0889:bits[1] is 1

    if (G_LoRaConfig.BandWidth == LORA_BW_500) {
        sx126x_write_register(TxModulation, sx126x_read_register(TxModulation) & 0xfB);
    } else {
        sx126x_write_register(TxModulation, sx126x_read_register(TxModulation) | 0x04);
		}

    sx126x_write_cmd(RADIO_SET_MODULATIONPARAMS, buf, n);
}

/**
  * preamblelen, headformat, PL, CRC config
*/
void sx126x_set_lora_packet_param()
{
    uint8_t n;
    uint8_t buf[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // when in SF6, SF7, PreambleLen should >= 12
		if((G_LoRaConfig.SpreadingFactor == LORA_SF5) || (G_LoRaConfig.SpreadingFactor == LORA_SF6)) {
				if (G_LoRaConfig.PreambleLength < 12) {
						G_LoRaConfig.PreambleLength = 12;
				}
      }   

    n = 6;
    buf[0] = (G_LoRaConfig.PreambleLength >> 8) & 0xFF;
    buf[1] = G_LoRaConfig.PreambleLength;
    buf[2] = G_LoRaConfig.HeaderType;
    buf[3] = G_LoRaConfig.PayloadLength;
    buf[4] = G_LoRaConfig.CrcMode;
    buf[5] = G_LoRaConfig.InvertIQ;

    sx126x_write_cmd(RADIO_SET_PACKETPARAMS, buf, n);

		 // WORKAROUND - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
		if (G_LoRaConfig.InvertIQ == LORA_IQ_INVERTED) {
				// RegIqPolaritySetup = @address 0x0736
				sx126x_write_register(0x0736, sx126x_read_register(0x0736) & ~(1 << 2));
		} else {
				// RegIqPolaritySetup @address 0x0736
				sx126x_write_register(0x0736, sx126x_read_register(0x0736) | (1 << 2));
		}
}

void sx126x_set_stop_rx_timer_on_preamble_detect(bool enable)
{
    sx126x_write_cmd(RADIO_SET_STOPRXTIMERONPREAMBLE, (uint8_t *)&enable, 1);
}

void sx126x_set_lora_symbol_num_timeout(uint8_t SymbNum)
{
    sx126x_write_cmd(RADIO_SET_LORASYMBTIMEOUT, &SymbNum, 1);
}

/**
  * sx126x freq check
  */
bool lora_config_check()
{
    if ((G_LoRaConfig.LoRa_Freq < 410000000) || (G_LoRaConfig.LoRa_Freq > 960000000)) {
        return false;
    }

    G_LoRaConfig.BandWidth = (RadioLoRaBandwidths_t)(G_LoRaConfig.BandWidth & 0x0F);

    //LLCC68 BW Limit
    if ((0x3>=G_LoRaConfig.BandWidth) & (0x7<=G_LoRaConfig.BandWidth)) {
        return false;
    }

    switch (G_LoRaConfig.BandWidth) {
    case  LORA_BW_500:
        G_BandWidthKHz = 500.0;
        break;
    case  LORA_BW_250:
        G_BandWidthKHz = 250.0;
        break;
    case  LORA_BW_125:
        G_BandWidthKHz = 125.0;
        break;
    case  LORA_BW_062:
        G_BandWidthKHz = 62.5;
        break;
    case  LORA_BW_041:
        G_BandWidthKHz = 41.67;
        break;
    case  LORA_BW_031:
        G_BandWidthKHz = 31.25;
        break;
    case  LORA_BW_020:
        G_BandWidthKHz = 20.83;
        break;
    case  LORA_BW_015:
        G_BandWidthKHz = 15.63;
        break;
    case  LORA_BW_010:
        G_BandWidthKHz = 10.42;
        break;
    case  LORA_BW_007:
        G_BandWidthKHz = 7.81;
        break;
    }
		
    //LLCC68 Speed Limit

		G_LoRaConfig.SpreadingFactor = (RadioLoRaSpreadingFactors_t)(G_LoRaConfig.SpreadingFactor & 0x0F);
		if (G_LoRaConfig.BandWidth == LORA_BW_125) {
        if ((G_LoRaConfig.SpreadingFactor >= LORA_SF10) || (G_LoRaConfig.SpreadingFactor < LORA_SF5)) {
            return false;
        }
		}

		if (G_LoRaConfig.BandWidth == LORA_BW_250) {
				if ((G_LoRaConfig.SpreadingFactor >= LORA_SF11) || (G_LoRaConfig.SpreadingFactor < LORA_SF5)) {
            return false;
        }
		}

		if (G_LoRaConfig.BandWidth == LORA_BW_500) {
				if ((G_LoRaConfig.SpreadingFactor >= LORA_SF12) || (G_LoRaConfig.SpreadingFactor < LORA_SF5)) {
            return false;
        }
		}

    // calculate symbol, unit in ms
    G_TsXms = (2 << ((G_LoRaConfig.SpreadingFactor) - 1)) / G_BandWidthKHz;
    G_LoRaConfig.CodingRate = (RadioLoRaCodingRates_t)(G_LoRaConfig.CodingRate & 0x07);

    if ((G_LoRaConfig.CodingRate > LORA_CR_4_8) || (G_LoRaConfig.CodingRate < LORA_CR_4_5)) {
        return false;
    }

    if (G_LoRaConfig.PowerCfig > 22) {
        return false;
    }

		return true;
}

tSX126xError sx126x_lora_init()
{
    uint8_t test = 0;

    if (false == lora_config_check()) {
        return PARAMETER_INVALID;
    }

    sx126x_reset();
		sx126x_wakeup();
		sx126x_set_standby(STDBY_RC);

    //SX126X 芯片PA内部有过压保护，如果失配会导致功率降低问题；
		//修改寄存器TxClampConfig来优化PA的阈值，位4-1必须
		//设置为"1111“（默认为0100）---2019.07.31
		sx126x_write_register(TxClampConfig, sx126x_read_register(TxClampConfig) | 0x1E);

		sx126x_set_regulator_mode(USE_DCDC);

		sx126x_set_dio2_as_rfswitch_ctrl(1);

		sx126x_set_packet_type(PACKET_TYPE_LORA);

    // SPI Check

    sx126x_write_register(REG_LR_PAYLOADLENGTH, 0x09);
    test = sx126x_read_register(REG_LR_PAYLOADLENGTH);
    if (test != 0x09) {
        return SPI_READCHECK_WRONG;
    }

		sx126x_write_register(REG_RX_GAIN, 0x96);
    sx126x_set_rf_freq(G_LoRaConfig.LoRa_Freq);
    sx126x_set_rf_tx_power(G_LoRaConfig.PowerCfig);

		sx126x_set_buffer_base_address(0x00, 0x00);

    ////SF, BW, CR, LOWDATARATEOPTIMIZE
    sx126x_set_lora_modulation_params();

    //PreambleLength, HeaderType, PayloadLength, CrcMode, InvertIQ
    sx126x_set_lora_packet_param();

    return NORMAL;
}


void sx126x_clear_irq_status(uint16_t irq)
{
    uint8_t buf[2];

    buf[0] = (uint8_t)(((uint16_t)irq >> 8) & 0x00FF);
    buf[1] = (uint8_t)((uint16_t)irq & 0x00FF);

    sx126x_write_cmd(RADIO_CLR_IRQSTATUS, buf, 2);
}

uint16_t sx126x_get_irq_status()
{
    uint8_t irq_status[2];

    sx126x_read_cmd(RADIO_GET_IRQSTATUS, irq_status, 2);
    return (irq_status[0] << 8) | irq_status[1];
}

RadioError_t sx126x_get_device_errors()
{
    uint8_t err[] = { 0, 0 };
    RadioError_t error = { .Value = 0 };

    sx126x_read_cmd(RADIO_GET_ERROR, (uint8_t *)err, 2);

    error.Fields.PaRamp     = ( err[0] & ( 1 << 0 ) ) >> 0;
    error.Fields.PllLock    = ( err[1] & ( 1 << 6 ) ) >> 6;
    error.Fields.XoscStart  = ( err[1] & ( 1 << 5 ) ) >> 5;
    error.Fields.ImgCalib   = ( err[1] & ( 1 << 4 ) ) >> 4;
    error.Fields.AdcCalib   = ( err[1] & ( 1 << 3 ) ) >> 3;
    error.Fields.PllCalib   = ( err[1] & ( 1 << 2 ) ) >> 2;
    error.Fields.Rc13mCalib = ( err[1] & ( 1 << 1 ) ) >> 1;
    error.Fields.Rc64kCalib = ( err[1] & ( 1 << 0 ) ) >> 0;
    return error;
}

void sx126x_clear_device_errors()
{
    uint8_t buf[2] = { 0x00, 0x00 };
    sx126x_write_cmd(RADIO_CLR_ERROR, buf, 2);
}

/**
  * when in Lora Rx and Implicit Mode, open RxTimeOut Timer
  * may be triggerd Rx Completed, but Timeout Timer not closed
  * should stop RTC, and clear Timeout Event
*/

void sx126x_clear_timeout_event()
{
		// WORKAROUND - Implicit Header Mode Timeout Behavior, see DS_SX1261-2_V1.2 datasheet chapter 15.3
		// RegRtcControl = @address 0x0902
		sx126x_write_register(0x0902, 0x00);
		// RegEventMask = @address 0x0944
		sx126x_write_register(0x0944, sx126x_read_register(0x0944) | (1 << 1));
		// WORKAROUND END
}

/**
  * DIO2 Switch Control
*/
void sx126x_set_dio2_as_rfswitch_ctrl(uint8_t enable)
{
    sx126x_write_cmd(RADIO_SET_RFSWITCHMODE, &enable, 1);
}

/**
  * DIO3 TCXO power control function open
*/
void sx126x_set_dio3_as_tcxo_ctrl(RadioTcxoCtrlVoltage_t tcxoVoltage, uint32_t timeout)
{
    uint8_t buf[4];

    buf[0] = tcxoVoltage & 0x07;
    buf[1] = (uint8_t)((timeout >> 16) & 0xFF);
    buf[2] = (uint8_t)((timeout >> 8) & 0xFF);
    buf[3] = (uint8_t)(timeout & 0xFF);

    sx126x_write_cmd(RADIO_SET_TCXOMODE, buf, 4);
}

RadioStatus_t sx126x_get_status()
{
    uint8_t stat = 0;
    RadioStatus_t status;

    sx126x_read_cmd(RADIO_GET_STATUS, (uint8_t *)&stat, 1);

    status.Value = stat;
    return status;
}

int8_t sx126x_get_rssi()
{
    uint8_t buf[1];
    int8_t rssi = 0;

    sx126x_read_cmd(RADIO_GET_RSSIINST, buf, 1);
    rssi = -buf[0] >> 1;
    return rssi;
}

void sx126x_get_rxbuffer_status(uint8_t *payloadLength)
{
    uint8_t status[2];

    sx126x_read_cmd(RADIO_GET_RXBUFFERSTATUS, status, 2);

    // In case of LORA fixed header, the payloadLength is obtained by reading
    // the register REG_LR_PAYLOADLENGTH
    if ((sx126x_read_register(REG_LR_PACKETPARAMS) >> 7 == 1)) {
        *payloadLength = sx126x_read_register(REG_LR_PAYLOADLENGTH);  
    } else {
        *payloadLength = status[0];
    }
}

/**
  * get Rssi and SNR
*/
void sx126x_get_packet_status(S_LoRaPara *pktStatus)
{
    uint8_t status[3];

    sx126x_read_cmd(RADIO_GET_PACKETSTATUS, status, 3);

    // last data pack rssi average value
    pktStatus->AvgPacket_RSSI = -status[0] >> 1;
    (status[1] < 128) ? (pktStatus->Packet_SNR = status[1] >> 2) : (pktStatus->Packet_SNR = ((status[1] - 256) >> 2));
    pktStatus->LastPacket_RSSI = -status[2] >> 1;
}

void sx126x_set_fs()
{
    sx126x_write_cmd(RADIO_SET_FS, 0, 0);
}

void sx126x_set_tx(uint32_t timeout)
{
    uint8_t buf[3];

    sx126x_clear_irq_status(IRQ_RADIO_ALL);
    buf[0] = (uint8_t)((timeout >> 16) & 0xFF);
    buf[1] = (uint8_t)((timeout >> 8) & 0xFF);
    buf[2] = (uint8_t)(timeout & 0xFF);

    sx126x_write_cmd(RADIO_SET_TX, buf, 3);
}

void sx126x_set_rx(uint32_t timeout)
{
    uint8_t buf[3];

    sx126x_clear_irq_status(IRQ_RADIO_ALL);
    buf[0] = (uint8_t)((timeout >> 16) & 0xFF);
    buf[1] = (uint8_t)((timeout >> 8) & 0xFF);
    buf[2] = (uint8_t)(timeout & 0xFF);

    sx126x_write_cmd(RADIO_SET_RX, buf, 3);
}

void sx126x_send_payload(uint8_t *payload, uint8_t size, uint32_t timeout)
{
    sx126x_write_buffer(0x00, payload, size);
    sx126x_set_tx(timeout);
}

void sx126x_tx_packet(uint8_t *data)
{
    //这里如果不是使用TCXO版本，STDBY_XOSC和STDBY_RC都无影响，如果使用的是TCXO版本，需要使用STDBY_XOSC，否则超时时间会增加到切换到TX模式周期中；
    sx126x_set_standby(STDBY_XOSC);

    //防止上电之后或者是以Cold-Start的条件从Sleep唤醒，或者低温运行时sleep状态切换过来时出现PLL Lock，XoscStart等错误现象，导致RF无法工作；
    sx126x_calibrate_error();

    sx126x_set_regulator_mode(USE_DCDC);

    // TX Mode
    sx126x_write_rx_tx(true);

    sx126x_clear_irq_status(IRQ_RADIO_ALL);

    sx126x_set_dio_irq_params(IRQ_TX_DONE|IRQ_RX_TX_TIMEOUT,
          IRQ_TX_DONE|IRQ_RX_TX_TIMEOUT, IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    sx126x_set_lora_packet_param();

    sx126x_set_buffer_base_address(0x00, 0x00);

    //开启了发射超时，时间为：0x2fffff*15.625≈50s,
    //这是一个安全措施，可能由于某种突发导执行了该命令但是不产生TxDOne,
    //这时设置Tx超时就可以保证状态不被打乱，用户根据实际应用配置情况进行设置超时时间

    sx126x_send_payload(data, G_LoRaConfig.PayloadLength, 0x2fffff);
} 

void lsd_rf_rx_mode()
{

}


void lsd_rf_send_packet(uint8_t *cbuf)
{

}

void sx126x_start_rx(uint32_t timeout)
{
    //这里如果不是使用TCXO版本，STDBY_XOSC和STDBY_RC都无影响，如果使用的是TCXO版本，需要使用STDBY_XOSC，否则超时时间会增加到切换到RX模式周期中；
    sx126x_set_standby(STDBY_XOSC);

    //防止上电之后或者是以Cold-Start的条件从Sleep唤醒，或者低温运行时sleep状态切换过来时出现PLL Lock，XoscStart等错误现象，导致RF无法工作；
    sx126x_calibrate_error();	

    sx126x_set_regulator_mode(USE_DCDC);

    // RX Mode
    sx126x_write_rx_tx(false);

    sx126x_clear_irq_status(IRQ_RADIO_ALL);

    //打开RX、CRC、RX超时中断，映射RX中断到DIO1；
    sx126x_set_dio_irq_params(IRQ_RX_DONE|IRQ_CRC_ERROR|IRQ_RX_TX_TIMEOUT, 
                            IRQ_RX_DONE|IRQ_RX_TX_TIMEOUT,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE);

    //用来选择定时器停止是检测前导码还是报头；
    sx126x_set_stop_rx_timer_on_preamble_detect(false);

    //设置用来验证正确LoRa信号的字符个数，多数情况下设置为0即可；
	  //一般设置为0时，即表示接收机验证一个LoRa信号字符，但是由于各种突发情况存在极低的可能误判，所以加大验证数值可以提高一定的正确性；
    sx126x_set_lora_symbol_num_timeout(0);

    sx126x_set_buffer_base_address(0x00, 0x00);

    if (timeout == 0xffffff) {
        usr_rx_continous_mode = 1;
    } else {
        usr_rx_continous_mode = 0;
    }
    // 开启了接收超时，时间为：0x2fffff*15.625us≈50s,用户根据实际应用配置情况进行设置超时时间；（默认情况是检测到报头（LoRa）或同步字（FSK）才会终止该定时器）
    sx126x_set_rx(timeout);
}

void sx126x_get_payload(uint8_t *buffer, uint8_t size)
{
    sx126x_read_buffer(0x00, buffer, size);
}

void sx126x_rx_packet(uint8_t *cbuf)
{
    // get SNR, RSSI
    sx126x_get_packet_status(&G_LoRaPara);

    sx126x_clear_irq_status(IRQ_RADIO_ALL);

    if (G_LoRaConfig.HeaderType == LORA_PACKET_EXPLICIT) {
        sx126x_get_rxbuffer_status(&G_LoRaConfig.PayloadLength);
    }

    sx126x_get_payload(cbuf, G_LoRaConfig.PayloadLength);
}

/****************************************************************************
 * CAD Function 
 ****************************************************************************/
void sx126x_set_cad_params(RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, 
    uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout)
{
    uint8_t buf[7];

    buf[0] = (uint8_t)cadSymbolNum;
    buf[1] = cadDetPeak;
    buf[2] = cadDetMin;
    buf[3] = (uint8_t)cadExitMode;
    buf[4] = (uint8_t)((cadTimeout >> 16) & 0xFF);
    buf[5] = (uint8_t)((cadTimeout >> 8) & 0xFF);
    buf[6] = (uint8_t)(cadTimeout & 0xFF);

    sx126x_write_cmd(RADIO_SET_CADPARAMS, buf, 5);
}

void sx126x_set_cad()
{
    sx126x_write_cmd(RADIO_SET_CAD, 0, 0);
}

void sx126x_cad_sample()
{
    // 防止上电之后或者是以Cold-Start的条件从Sleep唤醒
    // 或者低温运行时sleep状态切换过来时出现PLL Lock，XoscStart等错误现象，导致RF无法工作；
	sx126x_calibrate_error();	
	sx126x_set_standby(STDBY_RC);
    sx126x_set_cad();
}

void sx126x_cad_init()
{
    sx126x_set_standby(STDBY_RC);
	sx126x_set_packet_type(PACKET_TYPE_LORA);
	sx126x_set_regulator_mode(USE_DCDC);

	//这里按照SF=7时推荐配置；参数配置与SF、BW配置及功耗考虑有关，可以参照本函书说明配置
	sx126x_set_cad_params(LORA_CAD_02_SYMBOL, 22, 10, LORA_CAD_ONLY, 0xff);

	sx126x_write_rx_tx(false);

	sx126x_clear_irq_status(IRQ_RADIO_ALL);

	sx126x_set_dio_irq_params(IRQ_CAD_DONE|IRQ_CAD_ACTIVITY_DETECTED,
                            IRQ_CAD_DONE,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE);
}

void sx126x_wor_init()
{
    sx126x_cad_init();
}

void sx126x_wor_execute(uint8_t cclen)
{
    switch(cclen) {
    case 0:
        sx126x_clear_irq_status(IRQ_RADIO_ALL);
        sx126x_set_standby(STDBY_RC);
        sx126x_sleep();
        break;
    case 1:
        sx126x_cad_sample();
        break;
    default: break;
    }
}

void sx126x_awake(uint8_t*cbuf, uint16_t Preamble_Length)
{
    //防止上电之后或者是以Cold-Start的条件从Sleep唤醒
    //或者低温运行时sleep状态切换过来时出现PLL Lock，XoscStart等错误现象，导致RF无法工作；
    sx126x_calibrate_error();

    sx126x_set_standby(STDBY_RC);

	sx126x_set_regulator_mode(USE_DCDC);

    sx126x_write_rx_tx(true);

	sx126x_clear_irq_status(IRQ_RADIO_ALL);

	sx126x_set_dio_irq_params(IRQ_TX_DONE,
                            IRQ_TX_DONE,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE);

	G_LoRaConfig.PreambleLength = Preamble_Length;

	sx126x_set_lora_packet_param();

    sx126x_set_buffer_base_address(0x00, 0x00);

    sx126x_send_payload(cbuf, G_LoRaConfig.PayloadLength, 0xffffff);
}

void sx126x_wor_exit()
{
    //防止上电之后或者是以Cold-Start的条件从Sleep唤醒
    //或者低温运行时sleep状态切换过来时出现PLL Lock，XoscStart等错误现象，导致RF无法工作；
    sx126x_calibrate_error();

    sx126x_set_standby(STDBY_RC);

    sx126x_set_packet_type(PACKET_TYPE_LORA);

    sx126x_set_regulator_mode(USE_DCDC);

    sx126x_write_rx_tx(false);

    sx126x_clear_irq_status(IRQ_RADIO_ALL);

    sx126x_set_dio_irq_params(IRQ_RX_DONE|IRQ_CRC_ERROR, 
                            IRQ_RX_DONE,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE);

    sx126x_set_stop_rx_timer_on_preamble_detect(false);

    sx126x_set_lora_symbol_num_timeout(0);

	G_LoRaConfig.PreambleLength=0xffff;

    sx126x_set_lora_packet_param();
    sx126x_set_buffer_base_address(0x00, 0x00);

    sx126x_set_rx(0xffffff); // Rx Continuout
}

/****************************************************************************
 * sx126x user irq callback 
 ****************************************************************************/
__attribute__((weak)) void sx126x_tx_done_callback()
{

}

__attribute__((weak)) void sx126x_rx_done_callback(uint8_t *payload, uint16_t size, 
    int16_t lastpack_rssi, int16_t aver_rssi, int16_t snr)
{
    (void)payload;
    (void)size;
    (void)lastpack_rssi;
    (void)aver_rssi;
    (void)snr;
}

/****************************************************************************
 * sx126x irq 
 ****************************************************************************/
void sx126x_irq()
{
    uint16_t flag;
    flag = sx126x_get_irq_status();
    sx126x_clear_irq_status(IRQ_RADIO_ALL);

    if ((flag & IRQ_TX_DONE) == IRQ_TX_DONE) {
        sx126x_tx_done_callback();
    } else if ((flag & (IRQ_RX_DONE | IRQ_CRC_ERROR)) == IRQ_RX_DONE) {
        if (G_LoRaConfig.HeaderType == LORA_PACKET_IMPLICIT) {
            sx126x_clear_timeout_event();
        }
        sx126x_rx_packet(radio_rx_buffer);
        sx126x_rx_done_callback(&radio_rx_buffer[0], G_LoRaConfig.PayloadLength, 
            G_LoRaPara.LastPacket_RSSI, G_LoRaPara.AvgPacket_RSSI,
            G_LoRaPara.Packet_SNR);
        if (usr_rx_continous_mode == 1) {
            sx126x_set_buffer_base_address(0x00, 0x00);
        }
    }
}

