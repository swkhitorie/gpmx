#ifndef SCH1633_HW_H_
#define SCH1633_HW_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define AVG_FACTOR  1      // SCH1 sample averaging

#define FILTER_RATE         30.0f       // Hz, LPF1 Nominal Cut-off Frequency (-3dB).
#define FILTER_ACC12        30.0f
#define FILTER_ACC3         30.0f
#define SENSITIVITY_RATE1   1600.0f     // LSB / dps, DYN1 Nominal Sensitivity for 20 bit data.
#define SENSITIVITY_RATE2   1600.0f
#define SENSITIVITY_ACC1    3200.0f     // LSB / m/s2, DYN1 Nominal Sensitivity for 20 bit data.
#define SENSITIVITY_ACC2    3200.0f
#define SENSITIVITY_ACC3    3200.0f     // LSB / m/s2, DYN1 Nominal Sensitivity for 20 bit data.
#define DECIMATION_RATE     32          // DEC5, Output sample rate decimation.
#define DECIMATION_ACC      32

void        hw_init(void);
void        hw_EXTRESN_High(void);
void        hw_EXTRESN_Low(void);
void        hw_CS_High(void);
void        hw_CS_Low(void);
void        hw_delay(uint32_t ms);
uint64_t    hw_SPI48_Send_Request(uint64_t Request);


#endif
