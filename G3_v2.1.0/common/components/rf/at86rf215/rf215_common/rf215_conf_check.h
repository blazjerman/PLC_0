/**
 *
 * \file
 *
 * \brief RF215 configuration check (conf_at86rf).
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

#ifndef RF215_CONF_CHECK_H_INCLUDE
#define RF215_CONF_CHECK_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Check AT86RF_PART configuration (conf_at86rf.h) */
#ifndef AT86RF_PART
# error "AT86RF_PART not defined. See conf_at86rf.h"
#endif
#if (AT86RF_PART != AT86RF_PART_AT86RF215)
# if (AT86RF_PART != AT86RF_PART_AT86RF215M)
#  error "Unsupported AT86RF_PART. See Device Part Definition in at86rf.h"
# endif
#endif
#if (defined(AT86RF215_DISABLE_RF09_TRX) && defined(AT86RF215_DISABLE_RF24_TRX))
# error "At least one AT86RF215 Transceiver must be enabled. See conf_at86rf.h"
#endif

/** Check AT86RF215_MAX_PSDU_LEN configuration (conf_at86rf.h) */
#if ((AT86RF215_MAX_PSDU_LEN > 2047) || (AT86RF215_MAX_PSDU_LEN <= AT86RF_FCS_LEN))
# error "AT86RF215_MAX_PSDU_LEN: invalid configuration value (conf_at86rf.h)"
#endif

/** Check AT86RF215_NUM_RX_BUFFERS configuration (conf_at86rf.h) */
#if ((AT86RF215_NUM_RX_BUFFERS == 0) || (AT86RF215_NUM_RX_BUFFERS > 254))
# error "AT86RF215_NUM_RX_BUFFERS: invalid configuration value (conf_at86rf.h)"
#endif

/** Check AT86RF215_NUM_TX_PROG_BUFFERS configuration (conf_at86rf.h) */
#if ((AT86RF215_NUM_TX_PROG_BUFFERS == 0) || (AT86RF215_NUM_TX_PROG_BUFFERS > 254))
# error "AT86RF215_NUM_TX_PROG_BUFFERS: invalid configuration value (conf_at86rf.h)"
#endif

#ifdef __cplusplus
}
#endif

#endif  /* RF215_CONF_CHECK_H_INCLUDE */
