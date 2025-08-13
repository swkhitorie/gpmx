#include "drv_rtc.h"

#define BKUP_REG_DATA 0xA5A5

static RTC_HandleTypeDef RTC_Handler;

uint32_t stm32_rtc_get_subsecond(void)
{
    uint32_t reg = 0;

    /* Get sub seconds values from the correspondent registers*/
    reg = (uint32_t)(RTC_Handler.Instance->SSR);

    /* Read DR register to unfroze calendar registers */
    (void)(RTC_Handler.Instance->DR);

    return (reg);
}

#ifdef DRV_BSP_F1
/* update RTC_BKP_DRx*/
static void low_f1_bkp_update()
{
    RTC_DateTypeDef RTC_DateStruct = {0};

    HAL_PWR_EnableBkUpAccess();

    RTC_DateStruct.Year    = HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR2);
    RTC_DateStruct.Month   = HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR3);
    RTC_DateStruct.Date    = HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR4);
    RTC_DateStruct.WeekDay = HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR5);
    if (HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN) != HAL_OK) {
        // while(1) {} 
    }

    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
    if (HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR4) != RTC_DateStruct.Date) {
        HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR1, BKUP_REG_DATA);
        HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR2, RTC_DateStruct.Year);
        HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR3, RTC_DateStruct.Month);
        HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR4, RTC_DateStruct.Date);
        HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR5, RTC_DateStruct.WeekDay);
    }
}
#endif

bool hw_stm32_rtc_config()
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    HAL_PWR_EnableBkUpAccess();
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
#if defined(BSP_RTC_USING_LSI)
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#elif defined(BSP_RTC_USING_LSE)
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#else
#if defined(DRV_BSP_H7)
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;
#else
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#endif
#endif

    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

#if defined(DRV_BSP_WL) || defined(DRV_BSP_G0)
    __HAL_RCC_RTCAPB_CLK_ENABLE();
#endif

    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    RTC_Handler.Instance = RTC;
    if (HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR1) != BKUP_REG_DATA) {
#if defined(DRV_BSP_F1)
        RTC_Handler.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
        RTC_Handler.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
#elif defined(DRV_BSP_F4) || defined(DRV_BSP_WL) || \
    defined(DRV_BSP_H7) || defined (DRV_BSP_G0)
        /* set the frequency division */
#ifdef BSP_RTC_USING_LSI
        RTC_Handler.Init.AsynchPrediv = 0X7D;
#else
        RTC_Handler.Init.AsynchPrediv = 0X7F;
#endif
        RTC_Handler.Init.SynchPrediv = 0XFF;
#endif
        RTC_Handler.Init.HourFormat = RTC_HOURFORMAT_24;
        RTC_Handler.Init.OutPut = RTC_OUTPUT_DISABLE;
        RTC_Handler.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        RTC_Handler.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

        if (HAL_RTC_Init(&RTC_Handler) != HAL_OK) {
            return false;
        }
    } else {
#ifdef SOC_SERIES_STM32F1
        /* F1 series need update by bkp reg datas */
        low_f1_bkp_update();
#endif
    }

    return true;
}

bool stm32_rtc_setup()
{
#if !defined(DRV_BSP_H7) && !defined(DRV_BSP_WL)
    __HAL_RCC_PWR_CLK_ENABLE();
#if defined(DRV_BSP_F1) 
    __HAL_RCC_BKP_CLK_ENABLE();
#endif
#endif

// #if defined(BSP_RTC_USING_LSI) || defined(BSP_RTC_USING_LSE)
//     RCC_OscInitTypeDef RCC_OscInitStruct = {0};
// #ifdef BSP_RTC_USING_LSI
//     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
//     RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
//     RCC_OscInitStruct.LSIState = RCC_LSI_ON;
// #else
//     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
//     RCC_OscInitStruct.LSEState = RCC_LSE_ON;
//     RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
// #endif
//     RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//     HAL_RCC_OscConfig(&RCC_OscInitStruct);
// #endif

    if (!hw_stm32_rtc_config())
    {
        return false;
    }

    return true;
}

time_t stm32_rtc_get_timeval(struct timeval *tv)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};
    struct tm tm_new = {0};

    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);

    tm_new.tm_sec  = RTC_TimeStruct.Seconds;
    tm_new.tm_min  = RTC_TimeStruct.Minutes;
    tm_new.tm_hour = RTC_TimeStruct.Hours;
    tm_new.tm_mday = RTC_DateStruct.Date;
    tm_new.tm_mon  = RTC_DateStruct.Month - 1;
    tm_new.tm_year = RTC_DateStruct.Year + 100;

#ifdef RT_ALARM_USING_LOCAL_TIME
    tv->tv_sec = mktime(&tm_new);
#else
    tv->tv_sec = mktime(&tm_new); //timegm(&tm_new);
#endif

#if defined(DRV_BSP_H7)
    tv->tv_usec = (255.0 - stm32_rtc_get_subsecond() * 1.0) / 256.0 * 1000.0 * 1000.0;
#endif

    return tv->tv_sec;
}

void stm32_rtc_get_tm(struct tm *now)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};

    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);

    now->tm_sec  = RTC_TimeStruct.Seconds;
    now->tm_min  = RTC_TimeStruct.Minutes;
    now->tm_hour = RTC_TimeStruct.Hours;
    now->tm_mday = RTC_DateStruct.Date;
    now->tm_mon  = RTC_DateStruct.Month - 1;
    now->tm_year = RTC_DateStruct.Year + 100;
}


bool stm32_rtc_set_time_stamp(time_t time_stamp)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};
    struct tm tm = {0};

#ifdef RT_ALARM_USING_LOCAL_TIME
    localtime_r(&time_stamp,&tm);
#else
    gmtime_r(&time_stamp, &tm);
#endif

    if (tm.tm_year < 100) {
        return false;
    }

    RTC_TimeStruct.Seconds = tm.tm_sec ;
    RTC_TimeStruct.Minutes = tm.tm_min ;
    RTC_TimeStruct.Hours   = tm.tm_hour;
    RTC_DateStruct.Date    = tm.tm_mday;
    RTC_DateStruct.Month   = tm.tm_mon + 1 ;
    RTC_DateStruct.Year    = tm.tm_year - 100;
    RTC_DateStruct.WeekDay = tm.tm_wday + 1;

    if (HAL_RTC_SetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    if (HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }

    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR1, BKUP_REG_DATA);

#ifdef DRV_BSP_F1
    /* F1 series does't save year/month/date datas. so keep those datas to bkp reg */
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR2, RTC_DateStruct.Year);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR3, RTC_DateStruct.Month);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR4, RTC_DateStruct.Date);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR5, RTC_DateStruct.WeekDay);
#endif

    return true;
}

