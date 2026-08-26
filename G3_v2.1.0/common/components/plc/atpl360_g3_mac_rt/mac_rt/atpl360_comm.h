/**
 * \file
 *
 * \brief atpl360_host G3 Physical layer
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef ATPL360_COMM_H_INCLUDED
#define ATPL360_COMM_H_INCLUDED

#include "conf_atpl360.h"
#include "MacRt.h"
#include "MacRtDefs.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* ! Maximum length of PHY message (G3). Worts case: G3-FCC */
#define ATPL360_MAX_PHY_DATA_LENGTH              494
/* ! Maximum number of tone map */
#define TONE_MAP_SIZE_MAX                        3
/* ! Maximum length of MAC message (G3) */
#define ATPL360_MAX_MAC_DATA_LENGTH              (MAC_RT_MAX_PAYLOAD_SIZE + MAC_RT_FULL_HEADER_SIZE)

/* ! PL360 Host controller internal buffers size */
#define ATPL360_MAC_TX_PKT_SIZE                  (ATPL360_MAX_MAC_DATA_LENGTH + 2)
#define ATPL360_MAC_COMM_STATUS_SIZE             (MAC_RT_FULL_HEADER_SIZE)
#define ATPL360_MAC_TX_CMF_SIZE                  (2)
#define ATPL360_MAC_RX_PAR_SIZE                  (sizeof(struct TMacRtRxParameters))
#define ATPL360_REG_PKT_SIZE                     (sizeof(struct TMacRtPibValue) + 7)

/* ! Internal Memory Map */
typedef enum atpl360_mem_id {
	ATPL360_STATUS_INFO_ID = 0,
	ATPL360_SET_COORD_ID,
	ATPL360_TX_REQ_ID,
	ATPL360_TX_CFM_ID,
	ATPL360_DATA_IND_ID,
	ATPL360_MAC_SNIF_ID,
	ATPL360_COMM_STATUS_ID,
	ATPL360_RX_PAR_IND_ID,
	ATPL360_REG_INFO_ID,
	ATPL360_PHY_SNF_ID
} atpl360_mem_id_t;

#pragma pack(push,1)

/* ! \name G3 Structure defining PHY sniffer frame */
typedef struct phy_snf_frm {
	uint8_t uc_snf_cmd_vs;                         /* SNIFFER_IF_PHY_COMMAND_G3_VERSION */
	uint8_t uc_snf_vs;                             /* SNIFFER_VERSION */
	uint8_t uc_snf_dev_vs;                         /* SNIFFER_PL360_G3 */
	uint8_t uc_mod_type_scheme;                    /* ModType (high) + ModScheme (low) */
	uint8_t puc_tone_map[TONE_MAP_SIZE_MAX];       /* Tone Map */
	uint16_t us_num_symbols;                       /* Number of symbols */
	uint8_t uc_lqi;                                /* LQI */
	uint8_t uc_dt;                                 /* Delimiter Type */
	uint32_t ul_time_ini;                          /* Init time */
	uint32_t ul_time_end;                          /* End time */
	uint16_t us_rssi;                              /* RSSI */
	uint16_t us_agc_factor;                        /* AGC factor */
	uint16_t us_data_len;                          /* Data length */
	uint8_t puc_data[ATPL360_MAX_PHY_DATA_LENGTH]; /* Data message */
} phy_snf_frm_t;

/* ! \name G3 Structure defining events */
typedef struct atpl360_events {
	/* HW Timer reference */
	uint32_t ul_timer_ref;
	/* Length of DATA INDICATION event */
	uint16_t us_data_ind_len;
	/* Length of MAC SNIFFER event */
	uint16_t us_mac_snf_ind_len;
	/* Length of PHY Sniffer Frame event */
	uint16_t us_phy_snf_ind_len;
	/* Length of REGISTER DATA RESPONSE event */
	uint16_t us_reg_rsp_len;
	/* Flag to indicate if TX CONFIRMATION MSG event is enable */
	bool b_tx_cfm_ev;
	/* Flag to indicate if DATA INDICATION event is enable */
	bool b_data_ind_ev;
	/* Flag to indicate if MAC SNIFFER event is enable */
	bool b_mac_snf_ev;
	/* Flag to indicate if COMMUNICATION STATUS event is enable */
	bool b_comm_status_ev;
	/* Flag to indicate if RX PARAMETERS INDICATION event is enable */
	bool b_rx_par_ind_ev;
	/* Flag to indicate if REGISTER DATA RESPONSE event is enable */
	bool b_reg_data_ev;
	/* Flag to indicate if PHY Sniffer Frame event is enable */
	bool b_phy_snf_ev;
} atpl360_events_t;

#pragma pack(pop)

#define ATPL360_EVENT_DATA_LENGTH                12

/* ! FLAG MASKs for set events */
#define ATPL360_TX_CFM_FLAG_MASK                 (1 << 0)
#define ATPL360_DATA_IND_FLAG_MASK               (1 << 1)
#define ATPL360_MAC_SNF_FLAG_MASK                (1 << 2)
#define ATPL360_COMM_STATUS_FLAG_MASK            (1 << 3)
#define ATPL360_RX_PAR_IND_FLAG_MASK             (1 << 4)
#define ATPL360_REG_RSP_MASK                     (1 << 5)
#define ATPL360_PHY_SNF_FLAG_MASK                (1 << 6)

uint16_t atpl360_comm_tx_stringify(uint8_t *puc_dst, uint8_t *pTxSdu, uint16_t u16TxSduLen);
void atpl360_comm_set_event_info(atpl360_events_t *px_events_info, uint16_t us_int_flags);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* ATPL360_COMM_H_INCLUDED */
