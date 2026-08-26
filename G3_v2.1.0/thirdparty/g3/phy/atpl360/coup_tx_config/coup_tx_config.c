/**
 * \file
 *
 * \brief Couping and TX configuration for PL360 (G3).
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

/* Coupling configuration includes */
#include "coup_tx_config.h"
#include "coup_conf.h"
#include "coup_defs.h"
#include "coup_pl360_wrp.h"

/**************************************************************************************************/
/* Coupling and TX parameter configuration. *******************************************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

#ifdef CONF_COUP_CEN_A_ENABLE
/* CENELEC-A Coupling and TX parameter configuration */

static const coup_tx_params_t spx_coup_params_cen_a = {
	MAX_RMS_HI_VALUES_CEN_A, MAX_RMS_VLO_VALUES_CEN_A,
	TH_HI_VALUES_CEN_A, TH_VLO_VALUES_CEN_A,
	DACC_CFG_TABLE_CEN_A,
	IFFT_GAIN_HI_INI_CEN_A, IFFT_GAIN_HI_MIN_CEN_A, IFFT_GAIN_HI_MAX_CEN_A,
	IFFT_GAIN_VLO_INI_CEN_A, IFFT_GAIN_VLO_MIN_CEN_A, IFFT_GAIN_VLO_MAX_CEN_A,
	NUM_TX_LEVELS_CEN_A,
	COUP_EQU_NUM_COEF_CEN_A << 1,
	PLC_IC_DRV_CFG_CEN_A
};

static const uint16_t spus_equ_hi_cen_a[COUP_EQU_NUM_COEF_CEN_A] = PREDIST_COEF_HI_VALUES_CEN_A;
static const uint16_t spus_equ_vlo_cen_a[COUP_EQU_NUM_COEF_CEN_A] = PREDIST_COEF_VLO_VALUES_CEN_A;

#endif

#ifdef CONF_COUP_CEN_B_ENABLE
/* CENELEC-B Coupling and TX parameter configuration */

static const coup_tx_params_t spx_coup_params_cen_b = {
	MAX_RMS_HI_VALUES_CEN_B, MAX_RMS_VLO_VALUES_CEN_B,
	TH_HI_VALUES_CEN_B, TH_VLO_VALUES_CEN_B,
	DACC_CFG_TABLE_CEN_B,
	IFFT_GAIN_HI_INI_CEN_B, IFFT_GAIN_HI_MIN_CEN_B, IFFT_GAIN_HI_MAX_CEN_B,
	IFFT_GAIN_VLO_INI_CEN_B, IFFT_GAIN_VLO_MIN_CEN_B, IFFT_GAIN_VLO_MAX_CEN_B,
	NUM_TX_LEVELS_CEN_B,
	COUP_EQU_NUM_COEF_CEN_B << 1,
	PLC_IC_DRV_CFG_CEN_B
};

static const uint16_t spus_equ_hi_cen_b[COUP_EQU_NUM_COEF_CEN_B] = PREDIST_COEF_HI_VALUES_CEN_B;
static const uint16_t spus_equ_vlo_cen_b[COUP_EQU_NUM_COEF_CEN_B] = PREDIST_COEF_VLO_VALUES_CEN_B;

#endif

#ifdef CONF_COUP_FCC_ENABLE
/* FCC Coupling and TX parameter configuration */

static const coup_tx_params_t spx_coup_params_fcc = {
	MAX_RMS_HI_VALUES_FCC, MAX_RMS_VLO_VALUES_FCC,
	TH_HI_VALUES_FCC, TH_VLO_VALUES_FCC,
	DACC_CFG_TABLE_FCC,
	IFFT_GAIN_HI_INI_FCC, IFFT_GAIN_HI_MIN_FCC, IFFT_GAIN_HI_MAX_FCC,
	IFFT_GAIN_VLO_INI_FCC, IFFT_GAIN_VLO_MIN_FCC, IFFT_GAIN_VLO_MAX_FCC,
	NUM_TX_LEVELS_FCC,
	COUP_EQU_NUM_COEF_FCC << 1,
	PLC_IC_DRV_CFG_FCC
};

static const uint16_t spus_equ_hi_fcc[COUP_EQU_NUM_COEF_FCC] = PREDIST_COEF_HI_VALUES_FCC;
static const uint16_t spus_equ_vlo_fcc[COUP_EQU_NUM_COEF_FCC] = PREDIST_COEF_VLO_VALUES_FCC;

#endif

#ifdef CONF_COUP_ARIB_ENABLE
/* ARIB Coupling and TX parameter configuration */

static const coup_tx_params_t spx_coup_params_arib = {
	MAX_RMS_HI_VALUES_ARIB, MAX_RMS_VLO_VALUES_ARIB,
	TH_HI_VALUES_ARIB, TH_VLO_VALUES_ARIB,
	DACC_CFG_TABLE_ARIB,
	IFFT_GAIN_HI_INI_ARIB, IFFT_GAIN_HI_MIN_ARIB, IFFT_GAIN_HI_MAX_ARIB,
	IFFT_GAIN_VLO_INI_ARIB, IFFT_GAIN_VLO_MIN_ARIB, IFFT_GAIN_VLO_MAX_ARIB,
	NUM_TX_LEVELS_ARIB,
	COUP_EQU_NUM_COEF_ARIB << 1,
	PLC_IC_DRV_CFG_ARIB
};

static const uint16_t spus_equ_hi_arib[COUP_EQU_NUM_COEF_ARIB] = PREDIST_COEF_HI_VALUES_ARIB;
static const uint16_t spus_equ_vlo_arib[COUP_EQU_NUM_COEF_ARIB] = PREDIST_COEF_VLO_VALUES_ARIB;

#endif

/**
 * \brief Configure Coupling and TX parameters for G3
 *
 * \param px_atpl360_desc Pointer to PL360 controller despriptor
 * \param uc_band G3 band (see coup_pl360_wrp.h)
 */
void pl360_g3_coup_tx_config(atpl360_descriptor_t *px_atpl360_desc, uint8_t uc_band)
{
#if ((!defined(CONF_COUP_CEN_A_ENABLE)) && (!defined(CONF_COUP_CEN_B_ENABLE)) && (!defined(CONF_COUP_FCC_ENABLE)) && (!defined(CONF_COUP_ARIB_ENABLE)))
	UNUSED(px_atpl360_desc);
	UNUSED(uc_band);
#else
	coup_tx_params_t *px_coup_params;
	uint16_t *pus_equ_hi, *pus_equ_vlo;
	bool b_write_config = false;

	/* Initialize to NULL */
	px_coup_params = NULL;
	pus_equ_hi = NULL;
	pus_equ_vlo = NULL;

	if (uc_band == COUP_BAND_CEN_A) {
# ifdef CONF_COUP_CEN_A_ENABLE
		px_coup_params = (coup_tx_params_t *)&spx_coup_params_cen_a;
		pus_equ_hi = (uint16_t *)spus_equ_hi_cen_a;
		pus_equ_vlo = (uint16_t *)spus_equ_vlo_cen_a;

		b_write_config = true;
# endif
	} else if (uc_band == COUP_BAND_CEN_B) {
# ifdef CONF_COUP_CEN_B_ENABLE
		px_coup_params = (coup_tx_params_t *)&spx_coup_params_cen_b;
		pus_equ_hi = (uint16_t *)spus_equ_hi_cen_b;
		pus_equ_vlo = (uint16_t *)spus_equ_vlo_cen_b;

		b_write_config = true;
# endif
	} else if (uc_band == COUP_BAND_FCC) {
# ifdef CONF_COUP_FCC_ENABLE
		px_coup_params = (coup_tx_params_t *)&spx_coup_params_fcc;
		pus_equ_hi = (uint16_t *)spus_equ_hi_fcc;
		pus_equ_vlo = (uint16_t *)spus_equ_vlo_fcc;

		b_write_config = true;
# endif
	} else if (uc_band == COUP_BAND_ARIB) {
# ifdef CONF_COUP_ARIB_ENABLE
		px_coup_params = (coup_tx_params_t *)&spx_coup_params_arib;
		pus_equ_hi = (uint16_t *)spus_equ_hi_arib;
		pus_equ_vlo = (uint16_t *)spus_equ_vlo_arib;

		b_write_config = true;
# endif
	}

	if (b_write_config) {
		coup_conf_set_params(px_atpl360_desc, px_coup_params, pus_equ_hi, pus_equ_vlo);
	}
#endif
}
