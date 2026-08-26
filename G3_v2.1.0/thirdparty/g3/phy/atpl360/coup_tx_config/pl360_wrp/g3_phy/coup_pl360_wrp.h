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
#define COUP_BAND_CEN_A                ATPL360_WB_CENELEC_A
#define COUP_BAND_FCC                  ATPL360_WB_FCC
#define COUP_BAND_CEN_B                ATPL360_WB_CENELEC_B
#define COUP_BAND_ARIB                 ATPL360_WB_ARIB

static inline void coup_conf_set_params(atpl360_descriptor_t *px_atpl360_desc, coup_tx_params_t *px_coup_params, uint16_t *pus_equ_hi, uint16_t *pus_equ_vlo)
{
	/* Write PLC_IC_DRV_CFG. 1 byte */
	px_atpl360_desc->set_config(ATPL360_REG_PLC_IC_DRIVER_CFG, &px_coup_params->uc_plc_ic_drv_cfg, 1);

	/* Write DACC_TABLE values. 68 bytes */
	px_atpl360_desc->set_config(ATPL360_REG_DACC_TABLE_CFG, px_coup_params->pul_dacc_table, 17 << 2);

	/* Write NUM_TX_LEVELS. 1 byte */
	px_atpl360_desc->set_config(ATPL360_REG_NUM_TX_LEVELS, &px_coup_params->uc_num_tx_levels, 1);

	/* Write MAX_RMS_TABLE values. 32 x 2 bytes (HI and VLO) */
	px_atpl360_desc->set_config(ATPL360_REG_MAX_RMS_TABLE_HI, px_coup_params->pul_rms_hi, MAX_NUM_TX_LEVELS << 2);
	px_atpl360_desc->set_config(ATPL360_REG_MAX_RMS_TABLE_VLO, px_coup_params->pul_rms_vlo, MAX_NUM_TX_LEVELS << 2);

	/* Write THRESHOLDS_TABLE values. 64 x 2 bytes (HI and VLO) */
	px_atpl360_desc->set_config(ATPL360_REG_THRESHOLDS_TABLE_HI, px_coup_params->pul_th_hi, MAX_NUM_TX_LEVELS << 3);
	px_atpl360_desc->set_config(ATPL360_REG_THRESHOLDS_TABLE_VLO, px_coup_params->pul_th_vlo, MAX_NUM_TX_LEVELS << 3);

	/* Write GAIN_TABLE values. 6 x 2 bytes (HI and VLO) */
	px_atpl360_desc->set_config(ATPL360_REG_GAIN_TABLE_HI, px_coup_params->pus_gain_hi, 6);
	px_atpl360_desc->set_config(ATPL360_REG_GAIN_TABLE_VLO, px_coup_params->pus_gain_vlo, 6);

	/* Write Equalization values. num_carriers x 2 x 2 bytes (HI and VLO) */
	px_atpl360_desc->set_config(ATPL360_REG_PREDIST_COEF_TABLE_HI, pus_equ_hi, px_coup_params->uc_equ_size);
	px_atpl360_desc->set_config(ATPL360_REG_PREDIST_COEF_TABLE_VLO, pus_equ_vlo, px_coup_params->uc_equ_size);
}

#endif /* COUP_APP_WRP_H_INCLUDED */
