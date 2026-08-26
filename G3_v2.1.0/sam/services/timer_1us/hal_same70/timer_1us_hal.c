/**
 *
 * \file
 *
 * \brief Timer of 1 us service HAL (SAME70).
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

#include "timer_1us_hal.h"
#include "sysclk.h"
#include "pmc.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \brief Timer of 1us HAL initialization (SAME70). PCK6 is connected to TC.
 * PCK6 will be configured to generate the lowest frequency >= 1MHz and the
 * selected clock source in TC (TCCLKS) will be PCK6.
 *
 * \param[out] pul_freq_mhz_q27 Exact frequency (>= 1MHz) in MHz with 27 comma
 * bits (uQ5.27)
 *
 * \return TC clock source (TCCLKS)
 */
uint32_t timer_1us_hal_init(uint32_t *pul_freq_mhz_q27)
{
	uint32_t ul_mainck_cfg;
	uint32_t ul_mainck_hz;
	uint32_t ul_prescaler_aux;
	uint8_t uc_prescaler;

	/* Get MAINCK frequency in Hz (max 24MHz) */
	/* We can't use MCK (max 300MHz); max prescaler 256 */
	ul_mainck_cfg = PMC->CKGR_MOR;
	if (ul_mainck_cfg & CKGR_MOR_MOSCSEL) {
		if (ul_mainck_cfg & CKGR_MOR_MOSCXTBY) {
			ul_mainck_hz = OSC_MAINCK_BYPASS_HZ;
		} else {
			ul_mainck_hz = OSC_MAINCK_XTAL_HZ;
		}
	} else {
		uint32_t ul_mainck_rc = ul_mainck_cfg & CKGR_MOR_MOSCRCF_Msk;
		if (ul_mainck_rc == CKGR_MOR_MOSCRCF_4_MHz) {
			ul_mainck_hz = OSC_MAINCK_4M_RC_HZ;
		} else if (ul_mainck_rc == CKGR_MOR_MOSCRCF_8_MHz) {
			ul_mainck_hz = OSC_MAINCK_8M_RC_HZ;
		} else {
			ul_mainck_hz = OSC_MAINCK_12M_RC_HZ;
		}
	}

	/* Compute divider for lowest frequency >= 1MHz */
	uc_prescaler = (uint8_t)(ul_mainck_hz / 1000000);

	/* Configure PCK as MAINCK / PRES >= 1 MHz */
	pmc_switch_pck_to_mainck(PMC_PCK_6, PMC_PCK_PRES(uc_prescaler - 1));
	pmc_enable_pck(PMC_PCK_6);

	/* Compute configured PCK6 frequency in MHz [uQ5.27] */
	ul_prescaler_aux = (uint32_t)uc_prescaler * 1000000;
	*pul_freq_mhz_q27 = (uint32_t)div_round((uint64_t)ul_mainck_hz << 27, ul_prescaler_aux);

	/* PCK6 used as source for TC (TCCLKS) */
	return TC_CMR_TCCLKS_TIMER_CLOCK1;
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
