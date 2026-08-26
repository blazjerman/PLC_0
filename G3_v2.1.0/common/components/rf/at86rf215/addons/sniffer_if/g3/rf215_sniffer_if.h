/**
 *
 * \file
 *
 * \brief RF215 G3 sniffer addon.
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

#ifndef RF215_SNIFFER_IF_H_INCLUDED
#define RF215_SNIFFER_IF_H_INCLUDED

/* RF215 includes */
#include "at86rf_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* G3 sniffer message max length, including data and parameters */
#define RF215_SNIFFER_MSG_HEADER_LEN    25
#define RF215_SNIFFER_MSG_MAX_LEN       (AT86RF215_MAX_PSDU_LEN + RF215_SNIFFER_MSG_HEADER_LEN)

/** RF215 G3 sniffer addon function declaration */
uint16_t rf215_sniffer_if_stringify_ind(uint8_t uc_trx_id, uint8_t *puc_dst_buf, at86rf_rx_ind_t *px_rx_ind);
void rf215_sniffer_if_stringify_cfm(uint8_t uc_trx_id, uint8_t *puc_dst_buf, at86rf_tx_cfm_t *px_tx_cfm);
uint16_t rf215_sniffer_if_stringify_tx(uint8_t *puc_dst_buf, at86rf_tx_params_t *px_tx_params);

#ifdef __cplusplus
}
#endif

#endif   /* RF215_SNIFFER_IF_H_INCLUDED */
