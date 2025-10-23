#ifndef SX126X_DRIVER_H
#define SX126X_DRIVER_H

/**
 * SubGHz Module from Lierda FL22-C8
 * url: https://alidocs.dingtalk.com/i/nodes/dxXB52LJqnGE6GN4caZ2QLyK8qjMp697?utm_scene=team_space
 */

#include <stdint.h>
// #include <math.h>
#include "sx126x_hal.h"

#define  LSD4RFC_2L714N10      1
#define  LSD4RFC_2L722N10      2
#define  L_LRMRL22_97NN4       3


#define BUFFER_SIZE                                 64          // Define the payload size here
#define RF_FREQUENCY                                470800000   //Hz

#define SX1261                                      1
#define SX1262                                      2
#define SX1268                                      3

#define XTAL_FREQ                                   ( float )32000000.0f
#define FREQ_DIV                                    ( float )33554432.0f //( double )pow( 2.0, 25.0 ) //33554432
#define FREQ_STEP                                   ( float )( XTAL_FREQ / FREQ_DIV )


#define LOWDATARATEOPTIMIZE_ON                       0x01
#define LOWDATARATEOPTIMIZE_OFF                      0x00

/*!
 * Radio complete Wake-up Time with margin for temperature compensation
 */
#define RADIO_WAKEUP_TIME                           3 // [ms]

/*!
 * \brief Compensation delay for SetAutoTx/Rx functions in 15.625 microseconds
 */
#define AUTO_RX_TX_OFFSET                           2

/*!
 * \brief LFSR initial value to compute IBM type CRC
 */
#define CRC_IBM_SEED                                0xFFFF

/*!
 * \brief LFSR initial value to compute CCIT type CRC
 */
#define CRC_CCITT_SEED                              0x1D0F

/*!
 * \brief Polynomial used to compute IBM CRC
 */
#define CRC_POLYNOMIAL_IBM                          0x8005

/*!
 * \brief Polynomial used to compute CCIT CRC
 */
#define CRC_POLYNOMIAL_CCITT                        0x1021

/*!
 * \brief The address of the register holding the first byte defining the CRC seed
 *
 */
#define REG_LR_CRCSEEDBASEADDR                      0x06BC

/*!
 * \brief The address of the register holding the first byte defining the CRC polynomial
 */
#define REG_LR_CRCPOLYBASEADDR                      0x06BE

/*!
 * \brief The address of the register holding the first byte defining the whitening seed
 */
#define REG_LR_WHITSEEDBASEADDR_MSB                 0x06B8
#define REG_LR_WHITSEEDBASEADDR_LSB                 0x06B9

/*!
 * \brief The address of the register holding the packet configuration
 */
#define REG_LR_PACKETPARAMS                         0x0704

/*!
 * \brief The address of the register holding the payload size
 */
#define REG_LR_PAYLOADLENGTH                        0x0702

/*!
 * \brief The addresses of the registers holding SyncWords values
 */
#define REG_LR_SYNCWORDBASEADDRESS                  0x06C0

/*!
 * \brief The addresses of the register holding LoRa Modem SyncWord value
 */
#define REG_LR_SYNCWORD                             0x0740

/*!
 * Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x1424

/*!
 * Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x3444

/*!
 * The address of the register giving a 4 bytes random number
 */
#define RANDOM_NUMBER_GENERATORBASEADDR             0x0819

/*!
 * The address of the register holding RX Gain value (0x94: power saving, 0x96: rx boosted)
 */
#define REG_RX_GAIN                                 0x08AC

/*!
 * The address of the register holding Bit Sync configuration
 */
#define REG_BIT_SYNC                                0x06AC

/*!
 * Change the value on the device internal trimming capacitor
 */
#define REG_XTA_TRIM                                0x0911

/*!
 * Set the current max value in the over current protection
 */
#define REG_OCP                                     0x08E7



/*!
 * Set the PA clamping threshold
 */
#define TxClampConfig                               0x08D8

/*!
 * Set the 
 */
#define TxModulation                                0x0889


/*!
 * \brief Represents all possible opcode understood by the radio
 */
typedef enum RadioCommands_e
{
    RADIO_GET_STATUS                        = 0xC0,
    RADIO_WRITE_REGISTER                    = 0x0D,
    RADIO_READ_REGISTER                     = 0x1D,
    RADIO_WRITE_BUFFER                      = 0x0E,
    RADIO_READ_BUFFER                       = 0x1E,
    RADIO_SET_SLEEP                         = 0x84,
    RADIO_SET_STANDBY                       = 0x80,
    RADIO_SET_FS                            = 0xC1,
    RADIO_SET_TX                            = 0x83,
    RADIO_SET_RX                            = 0x82,
    RADIO_SET_RXDUTYCYCLE                   = 0x94,
    RADIO_SET_CAD                           = 0xC5,
    RADIO_SET_TXCONTINUOUSWAVE              = 0xD1,
    RADIO_SET_TXCONTINUOUSPREAMBLE          = 0xD2,
    RADIO_SET_PACKETTYPE                    = 0x8A,
    RADIO_GET_PACKETTYPE                    = 0x11,
    RADIO_SET_RFFREQUENCY                   = 0x86,
    RADIO_SET_TXPARAMS                      = 0x8E,
    RADIO_SET_PACONFIG                      = 0x95,
    RADIO_SET_CADPARAMS                     = 0x88,
    RADIO_SET_BUFFERBASEADDRESS             = 0x8F,
    RADIO_SET_MODULATIONPARAMS              = 0x8B,
    RADIO_SET_PACKETPARAMS                  = 0x8C,
    RADIO_GET_RXBUFFERSTATUS                = 0x13,
    RADIO_GET_PACKETSTATUS                  = 0x14,
    RADIO_GET_RSSIINST                      = 0x15,
    RADIO_GET_STATS                         = 0x10,
    RADIO_RESET_STATS                       = 0x00,
    RADIO_CFG_DIOIRQ                        = 0x08,
    RADIO_GET_IRQSTATUS                     = 0x12,
    RADIO_CLR_IRQSTATUS                     = 0x02,
    RADIO_CALIBRATE                         = 0x89,
    RADIO_CALIBRATEIMAGE                    = 0x98,
    RADIO_SET_REGULATORMODE                 = 0x96,
    RADIO_CLR_ERROR                         = 0x07,
    RADIO_SET_PRAMSWAPCMD                   = 0x8D,
    RADIO_GET_ERROR                         = 0x17,
    RADIO_SET_TCXOMODE                      = 0x97,
    RADIO_SET_TXFALLBACKMODE                = 0x93,
    RADIO_SET_RFSWITCHMODE                  = 0x9D,
    RADIO_SET_STOPRXTIMERONPREAMBLE         = 0x9F,
    RADIO_SET_LORASYMBTIMEOUT               = 0xA0,
}RadioCommands_t;


typedef enum
{
    MODE_SLEEP                              = 0x00,         //! The radio is in sleep mode
    MODE_STDBY_RC,                                          //! The radio is in standby mode with RC oscillator
    MODE_STDBY_XOSC,                                        //! The radio is in standby mode with XOSC oscillator
    MODE_FS,                                                //! The radio is in frequency synthesis mode
    MODE_TX,                                                //! The radio is in transmit mode
    MODE_RX,                                                //! The radio is in receive mode
    MODE_RX_DC,                                             //! The radio is in receive duty cycle mode
    MODE_CAD                                                //! The radio is in channel activity detection mode
}RadioOperatingModes_t;


/*!
 * \brief Represents the volatge used to control the TCXO on/off from DIO3
 */
typedef enum
{
    TCXO_CTRL_1_6V                          = 0x00,
    TCXO_CTRL_1_7V                          = 0x01,
    TCXO_CTRL_1_8V                          = 0x02,
    TCXO_CTRL_2_2V                          = 0x03,
    TCXO_CTRL_2_4V                          = 0x04,
    TCXO_CTRL_2_7V                          = 0x05,
    TCXO_CTRL_3_0V                          = 0x06,
    TCXO_CTRL_3_3V                          = 0x07,
}RadioTcxoCtrlVoltage_t;
/*!
 * \brief Represents a sleep mode configuration
 */
typedef union
{
    struct
    {
        uint8_t WakeUpRTC               : 1;                    //!< Get out of sleep mode if wakeup signal received from RTC
        uint8_t Reset                   : 1;
        uint8_t WarmStart               : 1;
        uint8_t Reserved                : 5;
    }Fields;
    uint8_t Value;
}SleepParams_t;

/*!
 * \brief Structure describing the radio status
 */
typedef union RadioStatus_u
{
    uint8_t Value;
    struct
    {   //bit order is lsb -> msb
        uint8_t Reserved  : 1;  //!< Reserved                         MSB
        uint8_t CmdStatus : 3;  //!< Command status
        uint8_t ChipMode  : 3;  //!< Chip mode
        uint8_t CpuBusy   : 1;  //!< Flag for CPU radio busy          LSB
    }Fields;
}RadioStatus_t;

/*!
 * \brief Represents a calibration configuration
 */
typedef union
{
    struct
    {
        uint8_t RC64KEnable    : 1;                             //!< Calibrate RC64K clock
        uint8_t RC13MEnable    : 1;                             //!< Calibrate RC13M clock
        uint8_t PLLEnable      : 1;                             //!< Calibrate PLL
        uint8_t ADCPulseEnable : 1;                             //!< Calibrate ADC Pulse
        uint8_t ADCBulkNEnable : 1;                             //!< Calibrate ADC bulkN
        uint8_t ADCBulkPEnable : 1;                             //!< Calibrate ADC bulkP
        uint8_t ImgEnable      : 1;
        uint8_t                : 1;
    }Fields;
    uint8_t Value;
}CalibrationParams_t;


typedef enum
{
    NORMAL,
    PARAMETER_INVALID,
    SPI_READCHECK_WRONG,
}tSX126xError;

typedef enum
{
    STDBY_RC                                = 0x00,
    STDBY_XOSC                              = 0x01,
}RadioStandbyModes_t;


typedef enum
{
    USE_LDO                                 = 0x00, // default
    USE_DCDC                                = 0x01,
}RadioRegulatorMode_t;


typedef enum
{
    RADIO_RAMP_10_US                        = 0x00,
    RADIO_RAMP_20_US                        = 0x01,
    RADIO_RAMP_40_US                        = 0x02,
    RADIO_RAMP_80_US                        = 0x03,
    RADIO_RAMP_200_US                       = 0x04,
    RADIO_RAMP_800_US                       = 0x05,
    RADIO_RAMP_1700_US                      = 0x06,
    RADIO_RAMP_3400_US                      = 0x07,
}RadioRampTimes_t;


typedef enum
{
    IRQ_RADIO_NONE                          = 0x0000,
    IRQ_TX_DONE                             = 0x0001,
    IRQ_RX_DONE                             = 0x0002,
    IRQ_PREAMBLE_DETECTED                   = 0x0004,
    IRQ_SYNCWORD_VALID                      = 0x0008,
    IRQ_HEADER_VALID                        = 0x0010,
    IRQ_HEADER_ERROR                        = 0x0020,
    IRQ_CRC_ERROR                           = 0x0040,
    IRQ_CAD_DONE                            = 0x0080,
    IRQ_CAD_ACTIVITY_DETECTED               = 0x0100,
    IRQ_RX_TX_TIMEOUT                       = 0x0200,
    IRQ_RADIO_ALL                           = 0xFFFF,
}RadioIrqMasks_t;

typedef enum
{
    PACKET_TYPE_GFSK                        = 0x00,
    PACKET_TYPE_LORA                        = 0x01,
    PACKET_TYPE_NONE                        = 0x0F,
}RadioPacketTypes_t;

/*!
 * \brief Holds the lengths mode of a LoRa packet type
 */
typedef enum
{
    LORA_PACKET_VARIABLE_LENGTH             = 0x00,         //!< The packet is on variable size, header included
    LORA_PACKET_FIXED_LENGTH                = 0x01,         //!< The packet is known on both sides, no header included in the packet
    LORA_PACKET_EXPLICIT                    = LORA_PACKET_VARIABLE_LENGTH,
    LORA_PACKET_IMPLICIT                    = LORA_PACKET_FIXED_LENGTH,
}RadioLoRaPacketLengthsMode_t;

/*!
 * \brief Represents the CRC mode for LoRa packet type
 */
typedef enum
{
    LORA_CRC_ON                             = 0x01,         //!< CRC activated
    LORA_CRC_OFF                            = 0x00,         //!< CRC not used
}RadioLoRaCrcModes_t;

/*!
 * \brief Represents the IQ mode for LoRa packet type
 */
typedef enum
{
    LORA_IQ_NORMAL                          = 0x00,
    LORA_IQ_INVERTED                        = 0x01,
}RadioLoRaIQModes_t;

/*!
 * \brief Represents the possible spreading factor values in LoRa packet types
 */
typedef enum
{
    LORA_SF5                                = 0x05,
    LORA_SF6                                = 0x06,
    LORA_SF7                                = 0x07,
    LORA_SF8                                = 0x08,
    LORA_SF9                                = 0x09,
    LORA_SF10                               = 0x0A,
    LORA_SF11                               = 0x0B,
    LORA_SF12                               = 0x0C,
}RadioLoRaSpreadingFactors_t;

/*!
 * \brief Represents the bandwidth values for LoRa packet type
 */
typedef enum
{
    LORA_BW_500                             = 6,
    LORA_BW_250                             = 5,
    LORA_BW_125                             = 4,
    LORA_BW_062                             = 3,
    LORA_BW_041                             = 10,
    LORA_BW_031                             = 2,
    LORA_BW_020                             = 9,
    LORA_BW_015                             = 1,
    LORA_BW_010                             = 8,
    LORA_BW_007                             = 0,
}RadioLoRaBandwidths_t;

/*!
 * \brief Represents the coding rate values for LoRa packet type
 */
typedef enum
{
    LORA_CR_4_5                             = 0x01,
    LORA_CR_4_6                             = 0x02,
    LORA_CR_4_7                             = 0x03,
    LORA_CR_4_8                             = 0x04,
}RadioLoRaCodingRates_t;

typedef struct S_LoRaConfig
{
    uint32_t LoRa_Freq;
    RadioLoRaBandwidths_t BandWidth;                     
    RadioLoRaSpreadingFactors_t SpreadingFactor;            
    RadioLoRaCodingRates_t CodingRate;
    int8_t PowerCfig;
    RadioLoRaPacketLengthsMode_t HeaderType;                         
    RadioLoRaCrcModes_t CrcMode;  
    RadioLoRaIQModes_t  InvertIQ;	
    uint16_t PreambleLength;
    uint8_t PayloadLength;              //1~127
}S_LoRaConfig;



typedef struct S_LoRaPara
{
    uint8_t* BufferPointer;
    uint16_t PreambleLength;
    uint8_t PayloadLength;              //1~127
    int16_t AvgPacket_RSSI;   //RssiPkt
    int16_t Packet_SNR;
    int16_t LastPacket_RSSI; //SignalRssiPkt 
}S_LoRaPara;

/*!
 * \brief Represents the possible radio system error states
 */
typedef union
{
    struct
    {
        uint8_t Rc64kCalib              : 1;                    //!< RC 64kHz oscillator calibration failed
        uint8_t Rc13mCalib              : 1;                    //!< RC 13MHz oscillator calibration failed
        uint8_t PllCalib                : 1;                    //!< PLL calibration failed
        uint8_t AdcCalib                : 1;                    //!< ADC calibration failed
        uint8_t ImgCalib                : 1;                    //!< Image calibration failed
        uint8_t XoscStart               : 1;                    //!< XOSC oscillator failed to start
        uint8_t PllLock                 : 1;                    //!< PLL lock failed
        uint8_t                         : 1;                    //!< Buck converter failed to start
        uint8_t PaRamp                  : 1;                    //!< PA ramp failed
        uint8_t                         : 7;                    //!< Reserved
    }Fields;
    uint16_t Value;
}RadioError_t;
/*!
 * \brief Represents the number of symbols to be used for channel activity detection operation
 */
typedef enum
{
    LORA_CAD_01_SYMBOL                      = 0x00,
    LORA_CAD_02_SYMBOL                      = 0x01,
    LORA_CAD_04_SYMBOL                      = 0x02,
    LORA_CAD_08_SYMBOL                      = 0x03,
    LORA_CAD_16_SYMBOL                      = 0x04,
}RadioLoRaCadSymbols_t;

/*!
 * \brief Represents the Channel Activity Detection actions after the CAD operation is finished
 */
typedef enum
{
    LORA_CAD_ONLY                           = 0x00,
    LORA_CAD_RX                             = 0x01,
    LORA_CAD_LBT                            = 0x10,
}RadioCadExitModes_t;


extern S_LoRaConfig G_LoRaConfig;
extern S_LoRaPara G_LoRaPara;
extern float G_BandWidthKHz;   //use for calculating symbol period
extern float G_TsXms;          //symbol period, units in ms


#ifdef __cplusplus
extern "C" {
#endif

void sx126x_wakeup(void);
RadioOperatingModes_t sx126x_get_opmode(void);
void sx126x_check_ready(void);
void sx126x_write_cmd(RadioCommands_t cmd, uint8_t *p, uint16_t size);
void sx126x_read_cmd(RadioCommands_t cmd, uint8_t *p, uint16_t size);
void sx126x_write_registers(uint16_t addr, uint8_t *p, uint16_t size);
void sx126x_write_register(uint16_t addr, uint8_t val);
void sx126x_read_registers(uint16_t addr, uint8_t *p, uint16_t size);
uint8_t sx126x_read_register(uint16_t addr);
void sx126x_write_buffer(uint8_t offset, uint8_t *p, uint8_t size);
void sx126x_read_buffer(uint8_t offset, uint8_t *p, uint8_t size);
void sx126x_write_fifo(uint8_t *p, uint8_t size);
void sx126x_read_fifo(uint8_t *p, uint8_t size);

void sx126x_reset(void);
void sx126x_set_sleep(SleepParams_t sleepConfig);
void sx126x_sleep(void);
void sx126x_set_standby(RadioStandbyModes_t standbyConfig);
void sx126x_set_packet_type(RadioPacketTypes_t packetType);
void sx126x_set_regulator_mode(RadioRegulatorMode_t mode);
void sx126x_set_buffer_base_address(uint8_t txBaseAddress, uint8_t rxBaseAddress);
void sx126x_calibrate(CalibrationParams_t calibParam);
void sx126x_calibrate_error(void);
void sx126x_calibrate_image(uint32_t freq);
tSX126xError sx126x_set_rf_freq(uint32_t freq);
void sx126x_set_dio_irq_params(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask);
void sx126x_set_paconnfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut);
void sx126x_set_tx_params(int8_t power, RadioRampTimes_t rampTime);
void sx126x_set_rf_tx_power(int8_t power);
void sx126x_set_lora_modulation_params(void);
void sx126x_set_lora_packet_param(void);
void sx126x_set_stop_rx_timer_on_preamble_detect(bool enable);
void sx126x_set_lora_symbol_num_timeout(uint8_t SymbNum);

bool lora_config_check(void);
tSX126xError sx126x_lora_init(void);

void sx126x_clear_irq_status(uint16_t irq);
uint16_t sx126x_get_irq_status(void);
RadioError_t sx126x_get_device_errors(void);
void sx126x_clear_device_errors(void);
void sx126x_clear_timeout_event(void);
void sx126x_set_dio2_as_rfswitch_ctrl(uint8_t enable);
void sx126x_set_dio3_as_tcxo_ctrl(RadioTcxoCtrlVoltage_t tcxoVoltage, uint32_t timeout);
RadioStatus_t sx126x_get_status(void);
int8_t sx126x_get_rssi(void);
void sx126x_get_rxbuffer_status(uint8_t *payloadLength);
void sx126x_get_packet_status(S_LoRaPara *pktStatus);
void sx126x_set_fs(void);
void sx126x_set_tx(uint32_t timeout);
void sx126x_set_rx(uint32_t timeout);
void sx126x_send_payload(uint8_t *payload, uint8_t size, uint32_t timeout);
void sx126x_tx_packet(uint8_t *data);

void lsd_rf_rx_mode(void);
void lsd_rf_send_packet(uint8_t *cbuf);

void sx126x_start_rx(uint32_t timeout);
void sx126x_get_payload(uint8_t *buffer, uint8_t size);
void sx126x_rx_packet(uint8_t *cbuf);

/****************************************************************************
 * CAD Function 
 ****************************************************************************/
void sx126x_set_cad_params(RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, 
    uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout);
void sx126x_set_cad(void);
void sx126x_cad_sample(void);
void sx126x_cad_init(void);
void sx126x_wor_init(void);
void sx126x_wor_execute(uint8_t cclen);
void sx126x_awake(uint8_t*cbuf, uint16_t Preamble_Length);
void sx126x_wor_exit(void);



/****************************************************************************
 * sx126x user irq callback 
 ****************************************************************************/
void sx126x_tx_done_callback(void);

void sx126x_rx_done_callback(uint8_t *payload, uint16_t size, 
    int16_t lastpack_rssi, int16_t aver_rssi, int16_t snr);

void sx126x_cad_done_callback(uint8_t detected);

#ifdef __cplusplus
}
#endif

#endif
