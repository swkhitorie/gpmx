#pragma once

#include <dev/i2c_master.h>

/* helper macro for handling report buffer indices */
#define INCREMENT(_x, _lim)	do { __typeof__(_x) _tmp = _x+1; if (_tmp >= _lim) _tmp = 0; _x = _tmp; } while(0)

/* helper macro for arithmetic - returns the square of the argument */
#define POW2(_x)		((_x) * (_x))

/*
 * MS5611/MS5607 internal constants and data structures.
 */
#define MS5611_ADDR_CMD_CONVERT_D1_OSR256		0x40	/* write to this address to start pressure conversion */
#define MS5611_ADDR_CMD_CONVERT_D1_OSR512		0x42	/* write to this address to start pressure conversion */
#define MS5611_ADDR_CMD_CONVERT_D1_OSR1024		0x44	/* write to this address to start pressure conversion */
#define MS5611_ADDR_CMD_CONVERT_D1_OSR2048		0x46	/* write to this address to start pressure conversion */
#define MS5611_ADDR_CMD_CONVERT_D1_OSR4096		0x48	/* write to this address to start pressure conversion */
#define MS5611_ADDR_CMD_CONVERT_D2_OSR256		0x50	/* write to this address to start temperature conversion */
#define MS5611_ADDR_CMD_CONVERT_D2_OSR512		0x52	/* write to this address to start temperature conversion */
#define MS5611_ADDR_CMD_CONVERT_D2_OSR1024		0x54	/* write to this address to start temperature conversion */
#define MS5611_ADDR_CMD_CONVERT_D2_OSR2048		0x56	/* write to this address to start temperature conversion */
#define MS5611_ADDR_CMD_CONVERT_D2_OSR4096		0x58	/* write to this address to start temperature conversion */

/*
  use an OSR of 1024 to reduce the self-heating effect of the
  sensor. Information from MS tells us that some individual sensors
  are quite sensitive to this effect and that reducing the OSR can
  make a big difference
 */
#define MS5611_ADDR_CMD_CONVERT_D1			ADDR_CMD_CONVERT_D1_OSR1024
#define MS5611_ADDR_CMD_CONVERT_D2			ADDR_CMD_CONVERT_D2_OSR1024

/*
 * Maximum internal conversion time for OSR 1024 is 2.28 ms. We set an update
 * rate of 100Hz which is be very safe not to read the ADC before the
 * conversion finished
 */
#define MS5611_CONVERSION_INTERVAL	10000	/* microseconds */
#define MS5611_MEASUREMENT_RATIO	3	/* pressure measurements per temperature measurement */


/*
	Reset
	Read PROM(128 bit of calibration words)
	D1 conversion
	D2 conversion
	Read ADC result(24 bit pressure/temperature)
	1 mbar = 0.1 Kpa
	Maximum Values for calculation results:
	Pmin = 10mbar   Pmax = 1200mbar
	Tmin = -40°C  Tmax = 85°C  Tref = 20°C 
*/
#define MS5611_D1D2_SIZE                0x03	

#define MS5611_PROM_BASE_ADDR           0xA2
#define MS5611_PROM_REG_COUNT           0x06 
#define MS5611_PROM_REG_SIZE            0x02 


class MS5611
{
public:
    MS5611();
    ~MS5611();

    int init();

    void run();

protected:

uORB::PublicationMulti<sensor_baro_s> _sensor_baro_pub{ORB_ID(sensor_baro)};

device::Device		*_interface;

ms5611::prom_s		_prom;

enum MS56XX_DEVICE_TYPES _device_type;
bool			_collect_phase{false};
unsigned		_measure_phase{false};

/* intermediate temperature values per MS5611/MS5607 datasheet */
int64_t			_OFF{0};
int64_t			_SENS{0};

bool _initialized{false};

float _last_pressure{NAN};
float _last_temperature{NAN};

perf_counter_t		_sample_perf;
perf_counter_t		_measure_perf;
perf_counter_t		_comms_errors;

/**
 * Initialize the automatic measurement state machine and start it.
 *
 * @note This function is called at open and error time.  It might make sense
 *       to make it more aggressive about resetting the bus in case of errors.
 */
void			start();

/**
 * Issue a measurement command for the current state.
 *
 * @return		OK if the measurement command was successful.
 */
int			measure();

/**
 * Collect the result of the most recent measurement.
 */
int			collect();
};


