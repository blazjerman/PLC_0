/**
 *
 * \file
 *
 * \brief Timer of 1 us service HAL (PIC32CX).
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
#include "conf_timer_1us.h"
#include "sysclk.h"
#include "pmc.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

#if (TIMER_1US_TC_CHN == 0)
/**
 * \brief Timer of 1us HAL initialization (PIC32CX). GCLK [TC_ID] will be
 * configured to generate the lowest frequency >= 1MHz and the selected clock
 * source in TC (TCCLKS) will be GCLK [TC_ID]. GCLK [TC_ID] only available for
 * TC channel 0
 *
 * \param[out] pul_freq_mhz_q27 Exact frequency (>= 1MHz) in MHz with 27 comma
 * bits (uQ5.27)
 *
 * \return TC clock source (TCCLKS)
 */
uint32_t timer_1us_hal_init(uint32_t *pul_freq_mhz_q27)
{
	uint32_t ul_mck_hz;
	uint32_t ul_prescaler_aux;
	uint8_t uc_prescaler;

	/* Get MCK0 frequency in Hz (max 240MHz) */
	ul_mck_hz = sysclk_get_cpu_hz();

	/* Compute divider for lowest frequency >= 1MHz */
	uc_prescaler = (uint8_t)(ul_mck_hz / 1000000);

	/* Configure GCLK [TC_ID] as MCK0 / GCLKDIV >= 1 MHz */
	pmc_configure_generic(TIMER_1US_ID_TC, PMC_PCR_GCLKCSS_MCK0, PMC_PCR_GCLKDIV(uc_prescaler - 1));
	pmc_enable_generic_clk(TIMER_1US_ID_TC);

	/* Compute configured PCK3 frequency in MHz [uQ5.27] */
	ul_prescaler_aux = (uint32_t)uc_prescaler * 1000000;
	*pul_freq_mhz_q27 = (uint32_t)div_round((uint64_t)ul_mck_hz << 27, ul_prescaler_aux);

	/* GCLK [TC_ID] used as source for TC (TCCLKS) */
	return TC_CMR_TCCLKS_TIMER_CLOCK1;
}
#else
/**
 * \brief Timer of 1us HAL initialization (PIC32CX). Select clock source in TC
 * (TCCLKS) for frequency >= 1MHz.. GCLK [TC_ID] only available for TC channel 0
 *
 * \param[out] pul_freq_mhz_q27 Exact frequency (>= 1MHz) in MHz with 27 comma
 * bits (uQ5.27)
 *
 * \return TC clock source (TCCLKS)
 */
uint32_t timer_1us_hal_init(uint32_t *pul_freq_mhz_q27)
{
	uint32_t ul_mck_hz;
	uint32_t ul_tcclks;
	uint32_t ul_prescaler_aux;
	uint8_t uc_prescaler;

	/* Get MCK0 frequency in Hz (max 240MHz) */
	ul_mck_hz = sysclk_get_cpu_hz();

	/* Select divisor [128,32,8] of MCK for lowest TC frequency >= 1MHz */
	if (ul_mck_hz >= 128000000) {
		ul_tcclks = TC_CMR_TCCLKS_TIMER_CLOCK4;
		uc_prescaler = 128;
	} else if (ul_mck_hz >= 32000000) {
		ul_tcclks = TC_CMR_TCCLKS_TIMER_CLOCK3;
		uc_prescaler = 32;
	} else {
		ul_tcclks = TC_CMR_TCCLKS_TIMER_CLOCK2;
		uc_prescaler = 8;
	}

	/* Compute configured TC frequency in MHz [uQ5.27] */
	ul_prescaler_aux = (uint32_t)uc_prescaler * 1000000;
	*pul_freq_mhz_q27 = (uint32_t)div_round((uint64_t)ul_mck_hz << 27, ul_prescaler_aux);
	return ul_tcclks;
}
#endif

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
