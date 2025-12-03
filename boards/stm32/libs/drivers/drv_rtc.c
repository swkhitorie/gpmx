#include "drv_rtc.h"

#define BKUP_REG_DATA 0xA5A5

static RTC_HandleTypeDef RTC_Handler;

#if defined(DRV_STM32_F1)

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

bool stm32_rtc_config()
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    HAL_PWR_EnableBkUpAccess();
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
#if defined(CONFIG_RTC_USING_LSI)
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#elif defined(CONFIG_RTC_USING_LSE)
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#else
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#endif
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

#if defined(DRV_STM32_WL)
    __HAL_RCC_RTCAPB_CLK_ENABLE();
#endif

    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    RTC_Handler.Instance = RTC;
    if (HAL_RTCEx_BKUPRead(&RTC_Handler, RTC_BKP_DR1) != BKUP_REG_DATA) {
#if defined(DRV_STM32_F1)
        RTC_Handler.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
        RTC_Handler.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
#elif defined(DRV_STM32_F4) || defined(DRV_STM32_WL) || defined(DRV_STM32_H7)
        /* set the frequency division */
#ifdef CONFIG_RTC_USING_LSI
        RTC_Handler.Init.AsynchPrediv = 0X7D;
#else
        RTC_Handler.Init.AsynchPrediv = 0X7F;
#endif
        RTC_Handler.Init.SynchPrediv = 0XFF;

        RTC_Handler.Init.HourFormat = RTC_HOURFORMAT_24;
        RTC_Handler.Init.OutPut = RTC_OUTPUT_DISABLE;
        RTC_Handler.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        RTC_Handler.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
#else
#warning "current chip doesn't support yet!"
#endif
        if (HAL_RTC_Init(&RTC_Handler) != HAL_OK) {
            return false;
        }
    } else {
#if defined(DRV_STM32_F1)
        /* F1 series need update by bkp reg datas */
        low_f1_bkp_update();
#endif
    }

    return true;
}

bool hw_stm32_rtc_setup()
{
#if !defined(DRV_STM32_H7) && !defined(DRV_STM32_WL)
    __HAL_RCC_PWR_CLK_ENABLE();
#if defined(DRV_STM32_F1) 
    __HAL_RCC_BKP_CLK_ENABLE();
#endif
#endif

#if defined(CONFIG_RTC_USING_LSI) || defined(CONFIG_RTC_USING_LSE)
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
#ifdef CONFIG_RTC_USING_LSI
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
#else
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
#endif
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
#endif

    if (!stm32_rtc_config())
    {
        return false;
    }

    return true;
}

void hw_stm32_rtc_deinit()
{
    HAL_RTC_DeInit(&RTC_Handler);
}

rclk_time_t hw_stm32_rtc_get_timeval(struct rclk_timeval *tv)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};
    struct rclk_tm tm_new = {0};

    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);

    tm_new._sec  = RTC_TimeStruct.Seconds;
    tm_new._min  = RTC_TimeStruct.Minutes;
    tm_new._hour = RTC_TimeStruct.Hours;
    tm_new._mday = RTC_DateStruct.Date;
    tm_new._mon  = RTC_DateStruct.Month - 1;
    tm_new._year = RTC_DateStruct.Year + 100;

    if (!rclk_tm_to_timstamp(&tm_new, &tv->_sec)) {
        return 0xff;
    }

#if defined(DRV_STM32_H7) || defined(DRV_STM32_F4) || defined(DRV_STM32_WL)
        tv->_usec = (255.0 - RTC_TimeStruct.SubSeconds * 1.0) / 256.0 * 1000.0 * 1000.0;
#endif

    return tv->_sec;
}

void hw_stm32_rtc_get_tm(struct rclk_tm *now)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};

    HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);

    now->_sec  = RTC_TimeStruct.Seconds;
    now->_min  = RTC_TimeStruct.Minutes;
    now->_hour = RTC_TimeStruct.Hours;
    now->_mday = RTC_DateStruct.Date;
    now->_mon  = RTC_DateStruct.Month - 1;
    now->_year = RTC_DateStruct.Year + 100;
}

bool hw_stm32_rtc_set_time_stamp(rclk_time_t time_stamp)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};

    struct rclk_tm tma = {0};
    if (!rclk_timstamp_to_tm(time_stamp, &tma)) {
        false;
    }

    RTC_TimeStruct.Seconds = tma._sec ;
    RTC_TimeStruct.Minutes = tma._min ;
    RTC_TimeStruct.Hours   = tma._hour;
    RTC_DateStruct.Date    = tma._mday;
    RTC_DateStruct.Month   = tma._mon + 1 ;
    RTC_DateStruct.Year    = tma._year - 100;
    RTC_DateStruct.WeekDay = tma._wday + 1;

    if (HAL_RTC_SetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    if (HAL_RTC_SetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }

    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR1, BKUP_REG_DATA);

#if defined(DRV_STM32_F1)
    /* F1 series does't save year/month/date datas. so keep those datas to bkp reg */
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR2, RTC_DateStruct.Year);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR3, RTC_DateStruct.Month);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR4, RTC_DateStruct.Date);
    HAL_RTCEx_BKUPWrite(&RTC_Handler, RTC_BKP_DR5, RTC_DateStruct.WeekDay);
#endif

    return true;
}

