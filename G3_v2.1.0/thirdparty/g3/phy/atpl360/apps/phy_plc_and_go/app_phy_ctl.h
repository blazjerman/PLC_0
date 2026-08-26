/**
 * \file
 *
 * \brief PHY_CHAT : G3-PLC Phy PLC And Go Applicationn. Module to manage PL360 PHY Layer
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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
#ifndef PHY_CTL_H
#define PHY_CTL_H

/* Shows info about physical layer and configuration of the PL360 */
#define PHY_CONTROLLER_DEBUG_ENABLE

typedef void (*phy_data_confirm_t)(enum tx_result_values uc_tx_result);
typedef void (*phy_data_indication_t)(uint8_t *puc_data_buf, uint16_t us_data_len, enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type, uint16_t us_rssi, uint8_t uc_lqi);
typedef void (*phy_data_indication_bad_crc_t)(void);
typedef void (*phy_update_tx_cfg_t)(enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type, uint16_t us_max_data_len);

/* Phy Controller API callbacks */
typedef struct phy_ctl_callbacks {
	phy_data_confirm_t phy_ctl_data_confirm;                /* Callback for TX data confirm */
	phy_data_indication_t phy_ctl_rx_msg;                   /* Callback for RX data indication */
	phy_data_indication_bad_crc_t phy_ctl_rx_msg_discarded; /* Callback for RX data indication with bad CRC */
	phy_update_tx_cfg_t phy_ctl_update_tx_configuration;    /* Callback for updating configuration of transmission */
} phy_ctl_callbacks_t;

/* Phy Controller API functions */
void phy_ctl_pl360_init(uint8_t uc_band);
void phy_ctl_set_callbacks(phy_ctl_callbacks_t *px_phy_callbacks);
void phy_ctl_process(void);
uint8_t phy_ctl_send_msg(uint8_t *puc_data_buff, uint16_t us_data_len);
uint16_t phy_ctl_set_mod_scheme(enum mod_schemes uc_scheme);
uint16_t phy_ctl_set_mod_type(enum mod_types uc_mod_type);
enum mod_schemes phy_ctl_get_mod_scheme(void);
enum mod_types phy_ctl_get_mod_type(void);
bool phy_ctl_set_band(uint8_t uc_band);
uint8_t phy_ctl_get_band(void);
uint8_t phy_ctl_get_2_rs_blocks(void);

#endif  /* PHY_CTL_H */
