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

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

/* System includes */
#include <string.h>

#include "compiler.h"
#include "atpl360_comm.h"
#include "atpl360.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t atpl360_comm_tx_stringify(uint8_t *puc_dst, uint8_t *pTxSdu, uint16_t u16TxSduLen)
{
	if ((u16TxSduLen == 0) || (u16TxSduLen > ATPL360_MAX_MAC_DATA_LENGTH)) {
		return 0;
	}

	*puc_dst++ = (uint8_t)u16TxSduLen;
	*puc_dst++ = (uint8_t)(u16TxSduLen >> 8);
	memcpy(puc_dst, pTxSdu, u16TxSduLen);

	return u16TxSduLen + 2;
}

void atpl360_comm_set_event_info(atpl360_events_t *px_events_info, uint16_t us_int_flags)
{
	if (us_int_flags & ATPL360_TX_CFM_FLAG_MASK) {
		px_events_info->b_tx_cfm_ev = true;
	} else {
		px_events_info->b_tx_cfm_ev = false;
	}

	if (us_int_flags & ATPL360_DATA_IND_FLAG_MASK) {
		px_events_info->b_data_ind_ev = true;
	} else {
		px_events_info->b_data_ind_ev = false;
	}

	if (us_int_flags & ATPL360_MAC_SNF_FLAG_MASK) {
		px_events_info->b_mac_snf_ev = true;
	} else {
		px_events_info->b_mac_snf_ev = false;
	}

	if (us_int_flags & ATPL360_COMM_STATUS_FLAG_MASK) {
		px_events_info->b_comm_status_ev = true;
	} else {
		px_events_info->b_comm_status_ev = false;
	}

	if (us_int_flags & ATPL360_RX_PAR_IND_FLAG_MASK) {
		px_events_info->b_rx_par_ind_ev = true;
	} else {
		px_events_info->b_rx_par_ind_ev = false;
	}

	if (us_int_flags & ATPL360_REG_RSP_MASK) {
		px_events_info->b_reg_data_ev = true;
	} else {
		px_events_info->b_reg_data_ev = false;
	}

	if (us_int_flags & ATPL360_PHY_SNF_FLAG_MASK) {
		px_events_info->b_phy_snf_ev = true;
	} else {
		px_events_info->b_phy_snf_ev = false;
	}
}

#ifdef __cplusplus
}
#endif
