/**
 * \file
 *
 * \brief Dispatcher application to manage data from/to socket & DLMS.
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <storage/storage.h>
#include <hal/hal.h>

#include "compiler.h"

#include "drivers/g3/network_adapter_g3.h"
#include "ipv6_mng.h"
#include "app_dlms_coord.h"
#include "app_dispatcher.h"
#include "g3_app_config.h"
#include "conf_project.h"
#include "conf_board.h"
#include "conf_oss.h"
#include "oss_if.h"
#include "Logger.h"

#include "dlms_cli_lib.h"
#include "dlms_cli_data.h"

/* MAC include */
#include "mac_wrapper.h"

#ifdef DLMS_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

/* Maximum length of IPv6 PDUs for DLMS application */
#define MAX_LENGTH_IPv6_PDU 1200

/* IPv6 input and output buffers and received length */
static uint8_t puc_rx_buff[MAX_LENGTH_IPv6_PDU - IPv6_HEADER_LENGTH - UDP_HEADER_LENGTH];
static uint8_t puc_tx_buff[MAX_LENGTH_IPv6_PDU - IPv6_HEADER_LENGTH - UDP_HEADER_LENGTH];

bool g_bHasMeterId;
uint8_t g_auc_ext_address[8]; /* EUI64 */

/* Vars for DLMS socket communication */
/* Socket for the UDP over PLC communication */
Socket *spx_dlms_socket;
/* Local IP address */
IpAddr sx_dlms_received_ip_addr;
/* Local Socket PORT */
uint16_t sus_dlms_udp_port;
/* Socket opened */
bool b_is_dlms_socket_open = false;

/**
 * \brief Display SW version in console
 */
static void _show_version( void )
{
	struct TAdpGetConfirm getConfirm;
	struct TAdpMacGetConfirm x_pib_confirm;

#if defined (CONF_BAND_CENELEC_A)
	LOG_APP_DEBUG(("[DLMS_APP] G3 Band: CENELEC-A\r\n"));
#elif defined (CONF_BAND_CENELEC_B)
	LOG_APP_DEBUG(("[DLMS_APP] G3 Band: CENELEC-B\r\n"));
#elif defined (CONF_BAND_FCC)
	LOG_APP_DEBUG(("[DLMS_APP] G3 Band: FCC\r\n"));
#elif defined (CONF_BAND_ARIB)
	LOG_APP_DEBUG(("[DLMS_APP] G3 Band: ARIB\r\n"));
#else
	LOG_APP_DEBUG(("[DLMS_APP] G3 Band: CENELEC-A\r\n"));
#endif

	AdpGetRequestSync(ADP_IB_SOFT_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		LOG_APP_DEBUG(("[DLMS_APP] G3 stack version: %hhu.%hhu.%hhu Date: 20%hhu-%hhu-%hhu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]));
	}

	AdpGetRequestSync(ADP_IB_MANUF_ADP_INTERNAL_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		LOG_APP_DEBUG(("[DLMS_APP] ADP version: %hhu.%hhu.%hhu Date: 20%hhu-%hhu-%hhu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_DEBUG(("[DLMS_APP] MAC version: %hhu.%hhu.%hhu Date: 20%hhu-%hhu-%hhu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION_RF, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_DEBUG(("[DLMS_APP] MAC RF version: %hhu.%hhu.%hhu Date: 20%hhu-%hhu-%hhu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_RT_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_DEBUG(("[DLMS_APP] MAC_RT version: %hhu.%hhu.%hhu Date: 20%hhu-%hhu-%hhu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_PHY_PARAM, MAC_WRP_PHY_PARAM_VERSION, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 4)) {
		LOG_APP_DEBUG(("[DLMS_APP] PHY version: %02x.%02x.%02x.%02x\r\n",
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[0]));
	}

	return;
}

/**
 * \brief Function to decide whether Conformance configuration has to be set
 *        This function is intended to be implemented by the user using its own mechanisms
 *
 */
static bool _enter_conformance_mode(void)
{
#ifdef APP_CONFORMANCE_TEST
	return true;

#else
	return false;
#endif
}

/**
 * \brief Get the tx buffer
 *
 * \return puc_tx_buff          Pointer to the data tx buffer
 */
uint8_t *dispatcher_get_tx_buff(void)
{
	return puc_tx_buff;
}

/**
 * \brief Get the rx buffer
 *
 * \return puc_rx_buff          Pointer to the data rx buffer
 */
uint8_t *dispatcher_get_rx_buff(void)
{
	return puc_rx_buff;
}

/**
 * \brief Send data using ipv6 UDP data socket
 *
 * \param puc_buff             Data buffer to send
 * \param us_buff_len          Bytes to send
 * \param x_transmitted_bytes  Transmitted bytes (return)
 * \param us_dst_addr          Destination short address
 */
void dispatcher_send(const void *puc_buff, size_t us_buff_len, size_t *x_transmitted_bytes, uint16_t us_dst_addr)
{
	IpAddr x_dst_ip_addr;

	ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &x_dst_ip_addr.ipv6Addr);

	/* Adapt the IPv6 address to the G3 connection data */
	x_dst_ip_addr.ipv6Addr.b[8] = (uint8_t)(G3_COORDINATOR_PAN_ID >> 8);
	x_dst_ip_addr.ipv6Addr.b[9] = (uint8_t)(G3_COORDINATOR_PAN_ID & 0xFF);
	x_dst_ip_addr.ipv6Addr.b[14] = (uint8_t)(us_dst_addr >> 8);
	x_dst_ip_addr.ipv6Addr.b[15] = (uint8_t)(us_dst_addr & 0xFF);
	x_dst_ip_addr.length = sizeof(Ipv6Addr);

	udp_socket_send(spx_dlms_socket, &x_dst_ip_addr, DLMS_SOCKET_PORT, puc_buff, us_buff_len, x_transmitted_bytes);
}

/**
 * \brief Timers of DLMS Application
 *
 */
void dispatcher_timers_update(void)
{
	dlms_app_update_1ms();
}

/**
 * \brief Process of DLMS Application
 *
 */
void dispatcher_app_process(void)
{
	size_t rx_bytes = 0;

	if (_enter_conformance_mode()) {
		/* Do not cycle nodes, to avoid injecting traffic during the conformance test. */
	} else {
		uint16_t us_short_addr;

		if (ipv6_mng_ready()) {
			if (b_is_dlms_socket_open && udp_socket_is_open(spx_dlms_socket)) {
				/* Send received data to dlms client library */
				if (udp_socket_receive(spx_dlms_socket, &sx_dlms_received_ip_addr, &sus_dlms_udp_port, puc_rx_buff, MAX_LENGTH_IPv6_PDU, &rx_bytes)) {
					if (rx_bytes > 0) {
						/* Get short address from last two bytes of ipv6 addr */
						us_short_addr = sx_dlms_received_ip_addr.ipv6Addr.b[14] << 8;
						us_short_addr |= sx_dlms_received_ip_addr.ipv6Addr.b[15];
						dlms_cli_wrapper_data_ind(us_short_addr, puc_rx_buff, rx_bytes);
					}
				}

				dlms_cli_process();
				dlms_app_process();
			} else {
				/* Open socket */
				b_is_dlms_socket_open = udp_socket_open(spx_dlms_socket, DLMS_SOCKET_PORT);
			}
		}
	}
}

/**
 * \brief Initialize DLMS Application
 *
 */
void dispatcher_app_init(void)
{
	/* struct TAdpMacGetConfirm getConfirm; */

	/* Get MAC (EUI64) */
	platform_init_eui64(g_auc_ext_address);

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
	/* Initialize UDP-IP over G3 */
	ipv6_mng_initialize();
#endif

	dlms_app_init();

	if (_enter_conformance_mode()) {
		ipv6_mng_set_conformance_config();
		/* Force G3 call every 1ms for Conformance mode */
		platform_cfg_call_process_rate(1);
	}

	/* Initialize socket */
	spx_dlms_socket = udp_socket_initialize();

	LOG_APP_DEBUG(("[DLMS_APP] DLMS Application: COORDINATOR\r\n"));
	_show_version();
}
