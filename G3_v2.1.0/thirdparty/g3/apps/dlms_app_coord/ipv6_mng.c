/**
 * \file
 *
 * \brief IPv6 management in app (sockets, ping & UDP responder)
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
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
#include <stdio.h>
#include <string.h>
#include "conf_project.h"
#include "g3_app_config.h"

#include "drivers/g3/network_adapter_g3.h"
#include "ipv6_mng.h"
#include "async_ping.h"

#ifdef IPV6_MNG_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

bool b_ipv6_initialized = false;
Ipv6Addr ipv6_local_link_addr;

/**
 * \brief Initialize UDP socket
 * \return                     Pointer to a Socket struct
 */
Socket *udp_socket_initialize(void)
{
	Socket *spx_socket;
	error_t x_error;

	/* Open UDP over PLC socket */
	spx_socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

	/* Failed to open socket? */
	if (!spx_socket) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketOpen()!\r\n"));
		return (Socket *)(NULL);
	}

	/* Associate the socket with the relevant interface */
	x_error = socketBindToInterface(spx_socket, &netInterface[0]);
	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketBindToInterface()!\r\n"));
		return (Socket *)(NULL);
	}

	/* Set timeout for blocking operations */
	x_error = socketSetTimeout(spx_socket, APP_SOCKET_TIMEOUT);
	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketSetTimeout()!\r\n"));
	}

	return spx_socket;
}

/**
 * \brief Check if UDP socket is opened
 * \param px_socket            Socket to send data
 * \return                     true / false
 */
bool udp_socket_is_open(Socket *spx_socket)
{
	if (spx_socket->interface != NULL) {
		/* Check interface link state */
		if (spx_socket->interface->configured && b_ipv6_initialized) {
			return true;
		}
	}

	return false;
}

/**
 * \brief Open UDP socket
 * \param px_socket            Socket to send data
 * \param px_dst_ip_addr       Destination IPv6 address
 * \param us_dst_udp_port      Destination UDP port
 * \param puc_buff             Data buffer to receive
 * \param us_buff_len          Max. bytes to receive
 * \param x_transmitted_bytes  Transmitted bytes (return)
 * \return                     success/fail
 */
bool udp_socket_open(Socket *spx_socket, uint16_t socket_port)
{
	error_t x_error;

	if (!(spx_socket->interface->configured && b_ipv6_initialized)) {
		LOG_APP_DEBUG(("[IPV6_MNG] ERROR in udp_socket_open(): Not initialized.\r\n"));
		return false;
	}

	/* Bind to ANY Address*/
	x_error = socketBind(spx_socket, (const IpAddr *)&IP_ADDR_ANY, socket_port);
	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Cannot bind socket!\r\n"));
		return false;
	}

	/* Associate the socket with the relevant interface */
	x_error = socketBindToInterface(spx_socket, &netInterface[0]);
	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketBindToInterface()!\r\n"));
		return false;
	}

	return true;
}

/**
 * \brief Send data using ipv6 UDP data socket
 * \param px_socket            Socket to send data
 * \param px_dst_ip_addr       Destination IPv6 address
 * \param us_dst_udp_port      Destination UDP port
 * \param puc_buff             Data buffer to send
 * \param us_buff_len          Bytes to send
 * \param x_transmitted_bytes  Transmitted bytes (return)
 * \return                     success/fail
 */
bool udp_socket_send(Socket *px_socket, IpAddr *px_dst_ip_addr, uint16_t us_dst_udp_port, const void *puc_buff, size_t us_buff_len, size_t *x_transmitted_bytes)
{
	error_t x_error;

	if (!px_socket->interface->configured) {
		LOG_APP_DEBUG(("[IPV6_MNG] ERROR in udp_socket_send(): Not initialized.\r\n"));
		return false;
	}

	x_error = socketSendTo(px_socket, px_dst_ip_addr, us_dst_udp_port, puc_buff, us_buff_len, x_transmitted_bytes, SOCKET_FLAG_WAIT_ALL);

	if (x_error != NO_ERROR) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketSendTo()!\r\n"));
		return false;
	} else {
		return true;
	}
}

/**
 * \brief Receive data using ipv6 UDP data socket
 * \param px_socket            Socket to receive data from
 * \param px_src_ip_addr       Source IPv6 address (return)
 * \param us_src_udp_port      Source UDP port (return)
 * \param puc_buff             Data buffer to receive
 * \param us_buff_len          Max. bytes to receive
 * \param x_transmitted_bytes  Received bytes (return)
 * \return                     success/fail
 */
bool udp_socket_receive(Socket *px_socket, IpAddr *px_src_ip_addr, uint16_t *pus_src_udp_port, const void *puc_buff, size_t us_buff_len, size_t *x_received_bytes)
{
	error_t x_error;

	if (!px_socket->interface->configured) {
		LOG_APP_DEBUG(("[IPV6_MNG] ERROR in ipv6_mng_send(): Not initialized.\r\n"));
		return false;
	}

	x_error = socketReceiveFrom(px_socket, px_src_ip_addr, pus_src_udp_port, (void *)puc_buff, us_buff_len, x_received_bytes, SOCKET_FLAG_WAIT_ALL);

	if ((x_error != NO_ERROR) && (x_error != ERROR_TIMEOUT)) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful socketReceiveFrom()!\r\n"));
		return false;
	} else {
		return true;
	}
}

/**
 * \brief Set Conformance configuration on IPv6 management
 *
 */
void ipv6_mng_set_conformance_config(void)
{
	Ipv6Addr multicast_0_addr;
	Ipv6Addr multicast_1_addr;

	/* Set multicast addresses */
	ipv6StringToAddr(APP_IPV6_MULTICAST_ADDR_0, &multicast_0_addr);
	ipv6JoinMulticastGroup(&netInterface[0], &multicast_0_addr);
	ipv6StringToAddr(APP_IPV6_MULTICAST_ADDR_1, &multicast_1_addr);
	ipv6JoinMulticastGroup(&netInterface[0], &multicast_1_addr);
	
	/* Initialize asynchronous ping module */
	async_ping_init();
}

/**
 * \brief Check if IPv6 interface is ready
 * \return                     true / false
 */
bool ipv6_mng_ready(void)
{
	return b_ipv6_initialized;
}

/**
 * \brief Update Link Parameters for New connection
 * \param us_pan_id          G3-PLC Pan-ID
 * \param us_short_addr      G3-PLC short address
 *
 */
void ipv6_mng_upd_link(uint16_t us_pan_id, uint16_t us_short_addr)
{
	error_t x_error;

	/* Set link-local address, based on the PAN_ID and the short address */
	ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &ipv6_local_link_addr);
	/* Adapt the IPv6 address to the G3 connection data */
	ipv6_local_link_addr.b[8] = (uint8_t)(us_pan_id >> 8);
	ipv6_local_link_addr.b[9] = (uint8_t)(us_pan_id & 0xFF);
	ipv6_local_link_addr.b[14] = (uint8_t)(us_short_addr >> 8);
	ipv6_local_link_addr.b[15] = (uint8_t)(us_short_addr & 0xFF);

	x_error = ipv6SetLinkLocalAddr(&netInterface[0], &ipv6_local_link_addr);

	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Cannot set local address!\r\n"));
		return;
	}

	b_ipv6_initialized = true;
}

/**
 * \brief Initialization of IPv6 management
 */
void ipv6_mng_initialize(void)
{
	error_t x_error;
	b_ipv6_initialized = false;

	/* Select the G3 network adapter */
	netSetDriver(&netInterface[0], &g3_adapter);
	x_error = netConfigInterface(&netInterface[0]);
	if (x_error) {
		LOG_APP_DEBUG(("[IPV6_MNG] Unsuccessful netConfigInterface()!\r\n"));
		return;
	}
}
