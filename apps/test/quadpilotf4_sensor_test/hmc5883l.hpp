#pragma once

#include <device/dnode.h>
#include <device/i2c_master.h>
#include <drivers/drv_hrt.h>
#include <cstdint>

/* Max measurement rate is 160Hz, however with 160 it will be set to 166 Hz, therefore workaround using 150 */
#define HMC5883_CONVERSION_INTERVAL   (1e6f / 150)  /* microseconds */

enum HMC5883_register {
	CONF_A = 0x00,
	CONF_B = 0x01,
	MODE = 0x02,
	DATA_OUT_X_MSB = 0x03,
	DATA_OUT_X_LSB = 0x04,
	DATA_OUT_Z_MSB = 0x05,
	DATA_OUT_Z_LSB = 0x06,
	DATA_OUT_Y_MSB = 0x07,
	DATA_OUT_Y_LSB = 0x08,
	STATUS = 0x09,
	ID_A = 0x0a,
	ID_B = 0x0b,
	ID_C = 0x0c,
};

enum HMC5883_id {
	ID_A_WHO_AM_I = 'H',
	ID_B_WHO_AM_I = '4',
	ID_C_WHO_AM_I = '3',
};

/* modes not changeable outside of driver */
#define HMC5883L_MODE_NORMAL		(0 << 0)  /* default */
#define HMC5883L_MODE_POSITIVE_BIAS	(1 << 0)  /* positive bias */
#define HMC5883L_MODE_NEGATIVE_BIAS	(1 << 1)  /* negative bias */

#define HMC5883L_AVERAGING_1		(0 << 5) /* conf a register */
#define HMC5883L_AVERAGING_2		(1 << 5)
#define HMC5883L_AVERAGING_4		(2 << 5)
#define HMC5883L_AVERAGING_8		(3 << 5)

#define MODE_REG_CONTINOUS_MODE		(0 << 0)
#define MODE_REG_SINGLE_MODE		(1 << 0) /* default */

#define STATUS_REG_DATA_OUT_LOCK	(1 << 1) /* page 16: set if data is only partially read, read device to reset */
#define STATUS_REG_DATA_READY		(1 << 0) /* page 16: set if all axes have valid measurements */

#define HMC5983_TEMP_SENSOR_ENABLE	(1 << 7)

class HMC5883 {

public:
    HMC5883(const char* i2c_ops_name);
    ~HMC5883();

    float mag_gaus[3];

    void run();

    int init();

private:

    bool ops_valid;
    char ops_name[16];
    struct i2c_master_s *ops;

    static constexpr uint8_t IIC_ADDR = 0x1E;
    static constexpr uint16_t HMC5883L_GA_LSB0 = 1370;				//0x00    ±0.88Ga
    static constexpr uint16_t HMC5883L_GA_LSB1 = 1090;				//0x20    ±1.30Ga
    static constexpr uint16_t HMC5883L_GA_LSB2 = 820;					//0x40    ±1.90Ga
    static constexpr uint16_t HMC5883L_GA_LSB3 = 660;					//0x60    ±2.50Ga
    static constexpr uint16_t HMC5883L_GA_LSB4 = 440;					//0x80    ±4.00Ga
    static constexpr uint16_t HMC5883L_GA_LSB5 = 390;					//0xA0    ±4.70Ga
    static constexpr uint16_t HMC5883L_GA_LSB6 = 330;					//0xC0    ±5.66Ga
    static constexpr uint16_t HMC5883L_GA_LSB7 = 230;					//0xE0    ±8.10Ga

    struct register_config_t {
        HMC5883_register reg;
        uint8_t set_bits{0};
        uint8_t clear_bits{0};
    };

    bool register_check(const register_config_t &reg_cfg);
    uint8_t register_read(HMC5883_register reg);
    void register_reads(HMC5883_register reg, uint8_t *buf, uint16_t sz);
    void register_write(HMC5883_register reg, uint8_t value);
    void register_modify(HMC5883_register reg, uint8_t setbits, uint8_t clearbits);

    uint8_t rawmag[6];
    uint8_t id_check[3];
    int16_t mag[3];

	bool _updated;
    hrt_abstime _lst_update_time;

    bool reset();

    bool config();

    void process_data();

	static constexpr uint8_t size_register_cfg{3};
    register_config_t _register_cfg[size_register_cfg] {
        // Register                         | Set bits, Clear bits
        { HMC5883_register::CONF_A,         0x78,      0x00 },   /* sample average num: 8, data output rate: 75HZ */
        { HMC5883_register::CONF_B,         0x02<<5,   0x1F },   /* ±1.9Ga, gain:820 */
        { HMC5883_register::MODE,           0x00,      0xFC },   /*  continuous mode*/
    };
};
