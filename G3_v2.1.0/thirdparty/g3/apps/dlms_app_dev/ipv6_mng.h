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
#ifndef __IPV6_MNG_H__
#define __IPV6_MNG_H__

#include <stdbool.h>
#include <stdint.h>
#include "g3_app_config.h"

#include "drivers/g3/network_adapter_g3.h"

/* Activate IPV6 manager debug */
#define IPV6_MNG_DEBUG_CONSOLE

struct TUDPHeader {
	uint16_t m_u16SrcPort;
	uint16_t m_u16DstPort;
	uint16_t m_u16Length;
	uint16_t m_u16ChkSum;
};

Socket *udp_socket_initialize(void);
bool udp_socket_is_open(Socket *spx_socket);
bool udp_socket_open(Socket *spx_socket, uint16_t socket_port);
bool udp_socket_send(Socket *px_socket, IpAddr *px_dst_ip_addr, uint16_t us_dst_udp_port, const void *puc_buff, size_t us_buff_len, size_t *x_transmitted_bytes);
bool udp_socket_receive(Socket *px_socket, IpAddr *px_src_ip_addr, uint16_t *pus_src_udp_port, const void *puc_buff, size_t us_buff_len, size_t *x_received_bytes);

void ipv6_mng_set_conformance_config(void);
bool ipv6_mng_ready(void);
void ipv6_mng_upd_link(uint16_t u16PanId, uint16_t u16ShortAddr);
void ipv6_mng_initialize(void);

#endif /* __IPV6_MNG_H__ */
