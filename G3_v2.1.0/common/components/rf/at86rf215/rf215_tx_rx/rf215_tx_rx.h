/**
 *
 * \file
 *
 * \brief RF215 TX/RX controller.
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

#ifndef RF215_TX_RX_H_INCLUDE
#define RF215_TX_RX_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"
#include "rf215_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** BBCn_IRQS receiver flags: AGCH, AGCR, RXFS, RXFE, FBLI */
#define BBC_IRQS_RX_FLAGS        (RF215_BBCn_IRQ_AGCH | RF215_BBCn_IRQ_AGCR | \
	RF215_BBCn_IRQ_RXFS | RF215_BBCn_IRQ_RXFE | RF215_BBCn_IRQ_FBLI)

/** RF215 TX controller function declaration */
void rf215_tx_init(uint8_t uc_trx_id);
void rf215_tx_frame_end_event(uint8_t uc_trx_id);
void rf215_tx_trxrdy_event(uint8_t uc_trx_id);
void rf215_tx_edc_event(uint8_t uc_trx_id);
void rf215_tx_event_handler(uint8_t uc_trx_id);
void rf215_tx_auto_stop(uint8_t uc_trx_id, at86rf_tx_cfm_res_t uc_tx_res);
void rf215_tx_abort_by_rx(uint8_t uc_trx_id);
void rf215_tx_set_bb_delay(uint8_t uc_trx_id, uint16_t us_bb_delay_us_q5, uint8_t uc_pe_delay_us_q5);
void rf215_tx_set_proc_delay(uint8_t uc_trx_id, uint16_t us_proc_delay_us_q5);
void rf215_tx_rx_check_aborts(uint8_t uc_trx_id, bool b_reset);

/** RF215 RX controller function declaration */
void rf215_rx_init(uint8_t uc_trx_id);
void rf215_rx_event(uint8_t uc_trx_id, uint8_t uc_rx_flags);
void rf215_rx_abort(uint8_t uc_trx_id);
void rf215_rx_event_handler(uint8_t uc_trx_id);

#ifdef __cplusplus
}
#endif

#endif  /* RF215_TX_RX_H_INCLUDE */
