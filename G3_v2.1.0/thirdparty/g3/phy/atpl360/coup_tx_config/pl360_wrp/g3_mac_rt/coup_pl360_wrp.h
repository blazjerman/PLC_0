/**
 * \file
 *
 * \brief Coupling configuration PHY app wrapper.
 *
 * Copyright (C) 2020 Atmel Corporation. All rights reserved.
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

#ifndef COUP_APP_WRP_H_INCLUDED
#define COUP_APP_WRP_H_INCLUDED

#include "coup_defs.h"
#include "atpl360.h"

/* G3 working band identifiers */
#define COUP_BAND_CEN_A                0
#define COUP_BAND_CEN_B                1
#define COUP_BAND_FCC                  2
#define COUP_BAND_ARIB                 3

static inline void coup_conf_set_params(atpl360_descriptor_t *px_atpl360_desc, coup_tx_params_t *px_coup_params, uint16_t *pus_equ_hi, uint16_t *pus_equ_vlo)
{
	struct TMacRtPibValue x_pl360_pib;

	/* Write PLC_IC_DRV_CFG. 1 byte */
	x_pl360_pib.m_u8Length = 1;
	x_pl360_pib.m_au8Value[0] = px_coup_params->uc_plc_ic_drv_cfg;
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_PLC_IC_DRIVER_CFG, &x_pl360_pib);

	/* Write DACC_TABLE values. 68 bytes */
	x_pl360_pib.m_u8Length = 17 << 2;
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pul_dacc_table, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_DACC_TABLE_CFG, &x_pl360_pib);

	/* Write NUM_TX_LEVELS. 1 byte */
	x_pl360_pib.m_u8Length = 1;
	x_pl360_pib.m_au8Value[0] = px_coup_params->uc_num_tx_levels;
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_NUM_TX_LEVELS, &x_pl360_pib);

	/* Write MAX_RMS_TABLE values. 32 x 2 bytes (HI and VLO) */
	x_pl360_pib.m_u8Length = MAX_NUM_TX_LEVELS << 2;
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pul_rms_hi, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_MAX_RMS_TABLE_HI, &x_pl360_pib);
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pul_rms_vlo, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_MAX_RMS_TABLE_VLO, &x_pl360_pib);

	/* Write THRESHOLDS_TABLE values. 64 x 2 bytes (HI and VLO) */
	x_pl360_pib.m_u8Length = MAX_NUM_TX_LEVELS << 3;
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pul_th_hi, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_THRESHOLDS_TABLE_HI, &x_pl360_pib);
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pul_th_vlo, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_THRESHOLDS_TABLE_VLO, &x_pl360_pib);

	/* Write GAIN_TABLE values. 6 x 2 bytes (HI and VLO) */
	x_pl360_pib.m_u8Length = 6;
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pus_gain_hi, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_GAIN_TABLE_HI, &x_pl360_pib);
	memcpy(x_pl360_pib.m_au8Value, px_coup_params->pus_gain_vlo, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_GAIN_TABLE_VLO, &x_pl360_pib);

	/* Write Equalization values. num_carriers x 2 x 2 bytes (HI and VLO) */
	x_pl360_pib.m_u8Length = px_coup_params->uc_equ_size;
	memcpy(x_pl360_pib.m_au8Value, pus_equ_hi, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_PREDIST_COEF_TABLE_HI, &x_pl360_pib);
	memcpy(x_pl360_pib.m_au8Value, pus_equ_vlo, x_pl360_pib.m_u8Length);
	px_atpl360_desc->set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_PREDIST_COEF_TABLE_VLO, &x_pl360_pib);
}

#endif /* COUP_APP_WRP_H_INCLUDED */
