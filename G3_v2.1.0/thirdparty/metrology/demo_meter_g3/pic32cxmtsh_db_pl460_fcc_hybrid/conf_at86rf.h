/**
 *
 * \file
 *
 * \brief AT86RF Driver Configuration.
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef CONF_AT86RF_H_INCLUDE
#define CONF_AT86RF_H_INCLUDE

#include "at86rf_defs.h"

/* Select the AT86RF device part */
#define AT86RF_PART                      AT86RF_PART_AT86RF215
/* #define AT86RF_PART                 AT86RF_PART_AT86RF215M */

/* Uncomment to disable the AT86RF215 2.4GHz Transceiver (RF24) */
#define AT86RF215_DISABLE_RF24_TRX

/* Uncomment to disable the AT86RF215 Sub-1GHz Transceiver (RF09) */
/* #define AT86RF215_DISABLE_RF09_TRX */

/* Enable automatic calculation of FCS in both TX and RX (32-bit FCS) */
#define AT86RF215_ENABLE_AUTO_FCS

/* Maximum PSDU data length (maximum for G3-RF), including FCS */
#define AT86RF215_MAX_PSDU_LEN           571

/* Number of RX buffers. Maximum number of RX indications that can be stored
 * without calling at86rf_event_handler(). It can be set to 1 if
 * at86rf_event_handler() is called at least once in the shortest frame duration
 * or once after an interrupt occurs (WFE) */
#define AT86RF215_NUM_RX_BUFFERS         1

/* Number of programmed TX buffers (instantaneous TX doesn't need buffer) */
#define AT86RF215_NUM_TX_PROG_BUFFERS    1

/* Enable AT86RF215 addon for G3 sniffer */
//#define AT86RF_ADDONS_ENABLE

#endif  /* CONF_AT86RF_H_INCLUDE */
