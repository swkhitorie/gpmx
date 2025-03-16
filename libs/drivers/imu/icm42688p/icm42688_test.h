#ifndef ICM42688P_TEST_H_
#define ICM42688P_TEST_H_

#include <board_config.h>

/* macros for register */
#define ICM_426XX_REG_BANK_SEL      0x76
#define ICM_426XX_REG_CHIP_CONFIG   0x11
#define ICM_426XX_REG_WHO_AM_I      0x75
#define ICM_426XX_REG_DRIVE_CONFIG  0x13
#define ICM_42688_REG_INTF_CONFIG0  0x4C
#define ICM_426XX_REG_INT_STATUS    0x2D
#define ICM_42688_REG_PWR_MGMT0     0x4E
#define ICM_42688_REG_GYRO_CONFIG0  0x4F
#define ICM_42688_REG_ACCEL_CONFIG0 0x50

/* macros for data */
#define ICM_426XX_ID 0x47

/* temperature */
#define ICM_426XX_GET_TEMPERATURE_0 0x1D
#define ICM_426XX_GET_TEMPERATURE_1 0x1E

/* accel */
#define ICM_426XX_GET_ACCEL_DATA_X0 0x20
#define ICM_426XX_GET_ACCEL_DATA_Y0 0x22
#define ICM_426XX_GET_ACCEL_DATA_Z0 0x24
#define ICM_426XX_GET_ACCEL_DATA_X1 0x1F
#define ICM_426XX_GET_ACCEL_DATA_Y1 0x21
#define ICM_426XX_GET_ACCEL_DATA_Z1 0x23

/* gyro */
#define ICM_426XX_GET_GYRO_DATA_X0 0x26
#define ICM_426XX_GET_GYRO_DATA_Y0 0x28
#define ICM_426XX_GET_GYRO_DATA_Z0 0x2A
#define ICM_426XX_GET_GYRO_DATA_X1 0x25
#define ICM_426XX_GET_GYRO_DATA_Y1 0x27
#define ICM_426XX_GET_GYRO_DATA_Z1 0x29

/* macors for others */
#define BIT_GYRO_MODE_LN ((0x03) << 2)
#define BIT_ACCEL_MODE_LN 0x03
#define READ_BIT_MASK 0x80

/* macros for frequency of acceleration */
typedef enum __freq_accel__ {
    ICM_426XX_ACCEL_32KHZ = 0x01,
    ICM_426XX_ACCEL_16KHZ,
    ICM_426XX_ACCEL_8KHZ,
    ICM_426XX_ACCEL_4KHZ,
    ICM_426XX_ACCEL_1KHZ,
    ICM_426XX_ACCEL_200HZ,
    ICM_426XX_ACCEL_100HZ,
} E_ICM_ACCEL_FREQ;

/* macros for range of acceleration */
/* take a big refer to 'g = 9.8065(m/(s*s))' */
typedef enum __range_accel__ {
    ICM_426XX_ACCEL_16G = 0x00,
    ICM_426XX_ACCEL_8G,
    ICM_426XX_ACCEL_4G,
    ICM_426XX_ACCEL_2G,
} E_ICM_ACCEL_RANGE;

/* macros for frequency of gyroscope */
typedef enum __freq_gyro__ {
    ICM_426XX_GYRO_32KHZ = 0x01,
    ICM_426XX_GYRO_16KHZ,
    ICM_426XX_GYRO_8KHZ,
    ICM_426XX_GYRO_4KHZ,
    ICM_426XX_GYRO_2KHZ,
    ICM_426XX_GYRO_1KHZ,
} E_ICM_GYRO_FREQ;

/* macros for range of gyroscope like °/s */
typedef enum __range_gyro__ {
    ICM_426XX_GYRO_2000S = 0x00,
    ICM_426XX_GYRO_1000S,
    ICM_426XX_GYRO_500S,
    ICM_426XX_GYRO_250S,
    ICM_426XX_GYRO_125S,
} E_ICM_GYRO_RANGE;

#ifdef __cplusplus
extern "C" {
#endif
void icm42688_init();

int icm42688_read(int16_t *data);

__attribute__((weak)) void icm42688_write_register(uint8_t addr, uint8_t data);

__attribute__((weak)) void icm42688_read_register(uint8_t addr, uint8_t *buf, uint8_t len, int rwway);

#ifdef __cplusplus
}
#endif

#endif
