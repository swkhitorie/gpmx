#ifndef SCRIPT_MACROS_H_
#define SCRIPT_MACROS_H_

#if (__ARMCC_VERSION >= 300586) || defined( __GNUC__)
#define GENERIC_CODE_ISR      __attribute__((section ("GENERIC_CODE_ISR_REGION")))
#define GENERIC_DATA_ISR      __attribute__((section ("GENERIC_DATA_ISR_REGION")))
#define GENERIC_NORELOC       __attribute__((section ("GENERIC_NORELOC_REGION")))
#define GNSS_FAST             __attribute__((section ("GNSS_FAST_REGION")))
#define STAGPS_FAST           __attribute__((section ("STAGPS_FAST_REGION")))
#define BIN_IMAGE_FAST        __attribute__((section ("BIN_IMAGE_FAST_REGION")))
#define SRAM2_DATA            __attribute__((section("SRAM2_AREA")))
#define SRAM_DATA             __attribute__((section("SRAM_AREA")))
#define SRAM_ERROR_DATA       __attribute__((section("SRAM_ERROR_AREA")))
#if defined(__GNUC__)
#define SRAM_STDBY_DATA       __attribute__ ((section("SRAM_STDBY_AREA")))
#define SW_CONFIG_NVM_DATA   __attribute__ ((section("SW_CONFIG_NVM_AREA")))
#else
#define SRAM_STDBY_DATA       __attribute__ ((zero_init))
#define SW_CONFIG_NVM_DATA   __attribute__ ((zero_init))
#endif
#else
#define GENERIC_CODE_ISR
#define GENERIC_DATA_ISR
#define GENERIC_NORELOC
#define GNSS_FAST
#define STAGPS_FAST
#define BIN_IMAGE_FAST
#define SRAM2_DATA
#define SRAM_DATA
#define SRAM_ERROR_DATA
#define SRAM_STDBY_DATA
#define SW_CONFIG_NVM_DATA
#endif

#endif
