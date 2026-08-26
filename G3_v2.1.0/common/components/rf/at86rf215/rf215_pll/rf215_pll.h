/**
 *
 * \file
 *
 * \brief RF215 Frequency Synthesizer (PLL).
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

#ifndef RF215_PLL_H_INCLUDE
#define RF215_PLL_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF215 PLL function declaration */
bool rf215_pll_init(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num);
void rf215_pll_trx_reset_event(uint8_t uc_trx_id);
uint32_t rf215_pll_get_fdelta(uint8_t uc_trx_id);
uint32_t rf215_pll_get_chn_freq(uint8_t uc_trx_id);
void rf215_pll_set_chn_cfg(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg_new, uint16_t us_chn_num_new);
bool rf215_pll_check_chn_cfg(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num);

#ifdef __cplusplus
}
#endif

#endif  /* RF215_PLL_H_INCLUDE */
