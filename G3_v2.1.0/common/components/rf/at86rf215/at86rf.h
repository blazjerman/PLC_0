/**
 *
 * \file
 *
 * \brief API driver for AT86RF215 RF transceiver.
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

#ifndef AT86RF_H_INCLUDED
#define AT86RF_H_INCLUDED

/* AT86RF215 includes */
#include "at86rf_defs.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/**
 * \ingroup rf_group
 * \defgroup at86rf215_group AT86RF215 component
 *
 * This component provides an interface to configure, control and manage the
 * AT86RF215 device through SPI and RST and IRQ pins.
 *
 * @{
 */

/* ! \name AT86RF215 Interface */
/* @{ */
/* AT86RF215 Interface function declaration */
void at86rf_init(at86rf_hal_wrapper_t *px_hal_wrp, at86rf_callbacks_t *px_callbacks);
at86rf_res_t at86rf_enable(at86rf_phy_ini_params_t *px_ini_params_09, at86rf_phy_ini_params_t *px_ini_params_24);
at86rf_res_t at86rf_disable(void);
void at86rf_event_handler(void);
void at86rf_tx_req(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params);
at86rf_cca_res_t at86rf_cca_req(uint8_t uc_trx_id, at86rf_cca_t uc_cca_mode);
void at86rf_set_chn_req(uint8_t uc_trx_id, at86rf_set_chn_req_params_t *px_params);
uint8_t at86rf_pib_get_len(at86rf_pib_attr_t uc_attr);
at86rf_res_t at86rf_pib_get(uint8_t uc_trx_id, at86rf_pib_attr_t us_attr, void *p_val);
at86rf_res_t at86rf_pib_set(uint8_t uc_trx_id, at86rf_pib_attr_t us_attr, void *p_val);
at86rf_res_t at86rf_get_msg_duration(uint8_t uc_trx_id, at86rf_mod_frame_params_t *px_mod_params, uint16_t us_psdu_len, uint32_t *pul_duration);

#ifdef AT86RF_ADDONS_ENABLE
void at86rf_addon_command(uint8_t *puc_msg, uint16_t us_len);

#endif

/* @} */

/* @} */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* AT86RF_H_INCLUDED */
