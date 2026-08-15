/**
  * @file    lora_info.c
  * @brief   To give info to the application about LoRaWAN configuration
*/
#include "LoRaMac.h"
#include "lora_info.h"
#include "lorawan_conf.h"

// #include "sys_app.h" /* APP_PRINTF */

static LoraInfo_t loraInfo = {0, 0};

void LoraInfo_Init(void)
{
  loraInfo.ContextManagement = 0;
  loraInfo.Region = 0;
  loraInfo.ClassB = 0;
  loraInfo.Kms = 0;

#ifdef  REGION_AS923
  loraInfo.Region |= (1 << LORAMAC_REGION_AS923);
#endif
#ifdef  REGION_AU915
  loraInfo.Region |= (1 << LORAMAC_REGION_AU915);
#endif
#ifdef  REGION_CN470
  loraInfo.Region |= (1 << LORAMAC_REGION_CN470);
#endif
#ifdef  REGION_CN779
  loraInfo.Region |= (1 << LORAMAC_REGION_CN779);
#endif
#ifdef  REGION_EU433
  loraInfo.Region |= (1 << LORAMAC_REGION_EU433);
#endif
#ifdef  REGION_EU868
  loraInfo.Region |= (1 << LORAMAC_REGION_EU868);
#endif
#ifdef  REGION_KR920
  loraInfo.Region |= (1 << LORAMAC_REGION_KR920);
#endif
#ifdef  REGION_IN865
  loraInfo.Region |= (1 << LORAMAC_REGION_IN865);
#endif
#ifdef  REGION_US915
  loraInfo.Region |= (1 << LORAMAC_REGION_US915);
#endif
#ifdef  REGION_RU864
  loraInfo.Region |= (1 << LORAMAC_REGION_RU864);
#endif

#if ( LORAMAC_CLASSB_ENABLED == 1 )
  loraInfo.ClassB = 1;
#elif !defined (LORAMAC_CLASSB_ENABLED)
#error LORAMAC_CLASSB_ENABLED not defined ( shall be <0 or 1> )
#endif

#if (!defined (LORAWAN_KMS) || (LORAWAN_KMS == 0))
  loraInfo.Kms = 0;
#else
  loraInfo.Kms = 1;
#endif

#if (!defined (CONTEXT_MANAGEMENT_ENABLED) || (CONTEXT_MANAGEMENT_ENABLED == 0))
  loraInfo.ContextManagement = 0;
#else
  loraInfo.ContextManagement = 1;
#endif
}

LoraInfo_t *LoraInfo_GetPtr(void)
{
  return &loraInfo;
}

