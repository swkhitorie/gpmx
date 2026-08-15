/**
  * @file    Commissioning.h
  * @brief   End-device commissioning parameters
 */

#ifndef __COMMISSIONING_H__
#define __COMMISSIONING_H__

/*!
 ******************************************************************************
 ********************************** WARNING ***********************************
 ******************************************************************************

 The LoRaWAN AES128 keys are stored and provisioned on secure-elements.

 This project provides a software emulated secure-element.
 The LoRaWAN AES128 keys SHALL be updated under
 src/peripherals/<secure-element name>-se\se-identity.h file.

 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 */
#include "se-identity.h"
#include "LoRaMacVersion.h"


/*!
 * When using ABP activation the MAC layer must know in advance to which server
 * version it will be connected.
 */
#define ABP_ACTIVATION_LRWAN_VERSION                       LORAMAC_VERSION

/*!
 * Indicates if the end-device support the operation with repeaters
 */
#define LORAWAN_REPEATER_SUPPORT                           false

/*!
 * Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK                             true

/*!
 * Current network ID
 */
#define LORAWAN_NETWORK_ID                                 ( uint32_t )0

#endif
