/**
 *
 * \file
 *
 * \brief RF215 PHY layer definitions.
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

#ifndef RF215_PHY_DEFS_H_INCLUDE
#define RF215_PHY_DEFS_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Number of FSK symbol rates */
#define RF215_NUM_FSK_SYMRATES       (AT86RF_FSK_SYMRATE_400kHz + 1)

/** Number of FSK modulation indexes */
#define RF215_NUM_FSK_MODIDX         (AT86RF_FSK_MODIDX_0_5 + 1)

/** Number of OFDM Options */
#define RF215_NUM_OFDM_OPTIONS       (AT86RF_OFDM_OPT_4 + 1)

/** Number of OFDM Modulation and Code Schemes (MCS) */
#define RF215_NUM_OFDM_MCS           (AT86RF_OFDM_MCS_6 + 1)

/** TX and RX LED identifiers */
#define RF215_LED_TX                  1
#define RF215_LED_RX                  2

/* Enumeration of RF215 component state machine */
typedef enum rf215_component_state {
	RF215_COMPONENT_OFF = 0,
	RF215_COMPONENT_INIT = 1,
	RF215_COMPONENT_ENABLING = 2,
	RF215_COMPONENT_ENABLED = 3,
	RF215_COMPONENT_DISABLED = 4,
	RF215_COMPONENT_ERROR = 5
} rf215_component_state_t;

/* Enumeration of RF215 PHY state machine */
typedef enum rf_phy_state {
	RF_PHY_STATE_RESET = 0,
	RF_PHY_STATE_SLEPT = 1,
	RF_PHY_STATE_RX_LISTEN = 2,
	RF_PHY_STATE_RX_HEADER = 3,
	RF_PHY_STATE_RX_PAYLOAD = 4,
	RF_PHY_STATE_TX_ABORT = 5,
	RF_PHY_STATE_TX_TXPREP = 6,
	RF_PHY_STATE_TX_CCA_ED = 7,
	RF_PHY_STATE_TX = 8,
	RF_PHY_STATE_TX_CONTINUOUS = 9,
	RF_PHY_STATE_IDLE = 10,
} rf_phy_state_t;

/* RF215 TRX and PHY control struct */
typedef struct rf215_phy_ctl {
	uint32_t ul_sync_time_trx;
	uint32_t ul_sync_time_phy;
	uint32_t ul_chn_switch_time_id;
	at86rf_phy_cfg_t x_phy_cfg_pending;
	at86rf_phy_band_opm_t us_band_opm;
	uint16_t us_tx_pay_symbols;
	uint16_t us_rx_pay_symbols;
	uint16_t us_turnaround_time_us;
	uint16_t us_chn_num;
	uint16_t us_chn_num_listen;
	uint16_t us_chn_num_switch;
	uint16_t us_chn_num_pending;
	uint8_t uc_trx_state;
	rf_phy_state_t uc_phy_state;
	at86rf_tx_cfm_res_t uc_set_chn_result;
	bool b_trxrdy;
	bool b_set_chn_requested;
	bool b_set_chn_notify;
	bool b_phy_cfg_pending;
} rf215_phy_ctl_t;

/* Struct of PHY statistics */
typedef struct rf215_phy_stats {
	uint32_t ul_tx_total;
	uint32_t ul_tx_total_bytes;
	uint32_t ul_tx_err_total;
	uint32_t ul_tx_err_busy_tx;
	uint32_t ul_tx_err_busy_chn;
	uint32_t ul_tx_err_busy_rx;
	uint32_t ul_tx_err_bad_len;
	uint32_t ul_tx_err_bad_format;
	uint32_t ul_tx_err_timeout;
	uint32_t ul_tx_err_aborted;
	uint32_t ul_tx_cfm_not_handled;
	uint32_t ul_rx_total;
	uint32_t ul_rx_total_bytes;
	uint32_t ul_rx_err_total;
	uint32_t ul_rx_err_false_positive;
	uint32_t ul_rx_err_bad_len;
	uint32_t ul_rx_err_bad_format;
	uint32_t ul_rx_err_bad_fcs_pay;
	uint32_t ul_rx_err_aborted;
	uint32_t ul_rx_override;
	uint32_t ul_rx_ind_not_handled;
} rf215_phy_stats_t;

/** RF215 internal global variables declared as extern */
extern at86rf_phy_cfg_t gpx_phy_cfg[AT86RF_NUM_TRX];
extern volatile rf215_phy_ctl_t gpx_phy_ctl[AT86RF_NUM_TRX];
extern rf215_phy_stats_t gpx_phy_stats[AT86RF_NUM_TRX];
extern rf215_component_state_t guc_rf215_comp_state;

#ifdef __cplusplus
}
#endif

#endif  /* RF215_PHY_DEFS_H_INCLUDE */
