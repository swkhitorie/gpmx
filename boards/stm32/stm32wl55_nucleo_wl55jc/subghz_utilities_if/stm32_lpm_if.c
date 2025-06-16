#include "stm32_lpm.h"
#include "stm32_lpm_if.h"

const struct UTIL_LPM_Driver_s UTIL_PowerDriver =
{
  PWR_EnterSleepMode,
  PWR_ExitSleepMode,

  PWR_EnterStopMode,
  PWR_ExitStopMode,

  PWR_EnterOffMode,
  PWR_ExitOffMode,
};

void PWR_EnterOffMode(void)
{

}

void PWR_ExitOffMode(void)
{

}

void PWR_EnterStopMode(void)
{

}

void PWR_ExitStopMode(void)
{

}

void PWR_EnterSleepMode(void)
{

}

void PWR_ExitSleepMode(void)
{

}
