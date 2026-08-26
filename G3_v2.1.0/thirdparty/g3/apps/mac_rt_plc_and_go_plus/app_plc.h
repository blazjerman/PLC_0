/**
 * \file
 *
 * \brief APP_PLC : Application to manage PL360 G3 MAC RT Layer.
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
#ifndef APP_PLC_H
#define APP_PLC_H

struct TAppMacRtMhr {
	struct TMacRtFc m_Fc;
	uint8_t m_u8SequenceNumber;
	uint16_t m_nDestinationPanIdentifier;
	uint16_t m_DestinationShortAddress;
	uint16_t m_nSourcePanIdentifier;
	uint16_t m_SourceShortAddress;
};

void app_plc_init(void);
void app_plc_process(void);
bool app_plc_send_msg(uint8_t *puc_data_buff, uint16_t us_data_len);

bool app_plc_set_band(uint8_t uc_band);
uint8_t app_plc_get_phy_band(void);
uint16_t app_plc_get_mac_src_address(void);
uint16_t app_plc_get_mac_dst_address(void);
uint8_t app_plc_get_ack_request(void);
void app_plc_set_mac_src_address(uint16_t address);
void app_plc_set_mac_dst_address(uint16_t address);
void app_plc_set_mac_panid(uint16_t pan_id);
void app_plc_set_ack_request(uint8_t ack_request);

#endif  /* APP_PLC_H */
