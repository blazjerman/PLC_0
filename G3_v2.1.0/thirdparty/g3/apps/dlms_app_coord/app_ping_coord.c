/**
 * \file
 *
 * \brief Ping application
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"

#include "app_ping_coord.h"
#include "Logger.h"
#include "error.h"

#include "app_adp_mng.h"
#include "app_dlms_coord.h"
#include "conf_bs.h"
#include "drivers/g3/network_adapter_g3.h"
#include "async_ping.h"

#ifdef PING_REPORT_CONSOLE
#       define PING_APP_REPORT(a)   printf a
#else
#       define PING_APP_REPORT(a)   (void)0
#endif

#ifndef DLMS_APP_PING_ALARM_TIMEOUT
	#define DLMS_APP_PING_ALARM_TIMEOUT              180
#endif
#ifndef DLMS_APP_PING_ALARM_TTL
	#define DLMS_APP_PING_ALARM_TTL                  10
#endif
#ifndef DLMS_APP_PING_ALARM_LEN
	#define DLMS_APP_PING_ALARM_LEN                  100
#endif

extern uint8_t g_u8NetworkJoinStatus;
extern IpAddr sx_local_ip_addr;

/* Timers */
static uint32_t u32PingTimer = 0xFFFFFFFF;

#ifdef DLMS_APP_ENABLE_PING_CYCLE
static bool sb_available_buffer;
static bool sb_ping_sent;

typedef struct {
	uint32_t ul_pings_sent;
	uint32_t ul_pings_successful;
	uint32_t ul_ping_errors;
} s_ping_cycle_node_stats_t;

struct {
	s_ping_cycle_node_stats_t as_ping_cycle_node_stats[MAX_LBDS];
	uint32_t ul_current_node_ping;
}
s_ping_cycle_status;

extern x_node_list_t px_node_list[MAX_LBDS];
extern uint16_t sus_num_reg_nodes;
extern uint32_t sul_cycle_counter;
#endif

/**
 * \brief Set or clear available buffer flag
 *
 */
void ping_app_set_buffer_ready(bool b_buffer_ready)
{
#ifdef DLMS_APP_ENABLE_PING_CYCLE
	sb_available_buffer = b_buffer_ready;
#else
	(void)b_buffer_ready;
#endif
}

/**
 * \brief Timers of Ping Application
 *
 */
void ping_app_timers_update(void)
{
	if ((u32PingTimer) && (u32PingTimer < 0xFFFFFFFF)) {
		u32PingTimer--;
	}
}

#ifdef DLMS_APP_ENABLE_PING_CYCLE
static void _ipv6_mng_ping_init_stats(void)
{
	uint32_t ul_i;
	s_ping_cycle_status.ul_current_node_ping = 0;
	for (ul_i = 0; ul_i < MAX_LBDS; ul_i++) {
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_pings_sent = 0;
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_pings_successful = 0;
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_ping_errors = 0;
	}
}

static error_t _ipv6_mng_ping_send_to_dev(uint32_t ul_dev_idx)
{
	IpAddr x_src_ip_addr;
	error_t x_error;

	/* Set link-local address, based on the PAN_ID and the short address */
	ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &x_src_ip_addr.ipv6Addr);
	/* Adapt the IPv6 address to the G3 connection data */
	x_src_ip_addr.ipv6Addr.b[8] = (uint8_t)(G3_COORDINATOR_PAN_ID >> 8);
	x_src_ip_addr.ipv6Addr.b[9] = (uint8_t)(G3_COORDINATOR_PAN_ID & 0xFF);
	x_src_ip_addr.ipv6Addr.b[14] = (uint8_t)(px_node_list[ul_dev_idx].us_short_address >> 8);
	x_src_ip_addr.ipv6Addr.b[15] = (uint8_t)(px_node_list[ul_dev_idx].us_short_address & 0xFF);
	x_src_ip_addr.length = sizeof(Ipv6Addr);

	x_error = async_ping_send(&netInterface[0], &x_src_ip_addr, DLMS_APP_PING_CYCLE_LEN, DLMS_APP_PING_CYCLE_TTL, DLMS_APP_PING_CYCLE_TIMEOUT * 1000);
	return x_error;
}

#endif

/**
 * \brief Process of Dispatcher Application
 *
 */
void ping_app_process(void)
{
#ifdef DLMS_APP_ENABLE_PING_CYCLE
	error_t x_error;
	systime_t roundTripTime;
	uint16_t us_node_idx;

	if (sb_ping_sent) {
		x_error = async_ping_rcv( &roundTripTime);
		if (x_error == ERROR_TIMEOUT) {
			PING_APP_REPORT(("[DLMS_APP] Ping Process Timeout  Error: %d \r\n", x_error));
		} else {
			if (x_error != NO_ERROR) {
				s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_ping_errors += 1;
				PING_APP_REPORT(("[CYCLE %4u] Unsuccessful Ping! to 0x%04x Error: %d \r\n", sul_cycle_counter,
						px_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address, x_error));
				sb_ping_sent = 0;
			} else {
				s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_pings_successful += 1;
				PING_APP_REPORT(("[CYCLE %4u] Ping Successful to 0x%04x! RoundTripTime: %d\r\n", sul_cycle_counter,
						px_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address, (int)roundTripTime));
				sb_ping_sent = 0;
			}

			s_ping_cycle_status.ul_current_node_ping++;

			if (s_ping_cycle_status.ul_current_node_ping == sus_num_reg_nodes) {
				s_ping_cycle_status.ul_current_node_ping = 0;
				for (us_node_idx = 0; us_node_idx < sus_num_reg_nodes; us_node_idx++) {
					PING_APP_REPORT((
								"[CYCLE %4u] Ping Report node 0x%04x  -->  Pings sent: %u  ; Successful Ping: %u; Errors: %u Success Rate : %.1f\r\n",
								sul_cycle_counter,
								px_node_list[us_node_idx].us_short_address,
								s_ping_cycle_status.as_ping_cycle_node_stats[us_node_idx].ul_pings_sent,
								s_ping_cycle_status.as_ping_cycle_node_stats[us_node_idx].ul_pings_successful,
								s_ping_cycle_status.as_ping_cycle_node_stats[us_node_idx].ul_ping_errors,
								(100.0 *
								(double)s_ping_cycle_status.as_ping_cycle_node_stats[us_node_idx].ul_pings_successful) /
								((double)s_ping_cycle_status.as_ping_cycle_node_stats[us_node_idx].ul_pings_sent)));
				}
			}
		}
	}

	if (!sb_ping_sent) {
		if (sus_num_reg_nodes && sb_available_buffer) {
			x_error = _ipv6_mng_ping_send_to_dev(s_ping_cycle_status.ul_current_node_ping);
			if (x_error == NO_ERROR) {
				PING_APP_REPORT(("\r\n[DLMS_APP] Ping to 0x%04x Sent ok!\r\n",
						px_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address));
				s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_pings_sent += 1;
				sb_ping_sent = true;
			} else {
				PING_APP_REPORT(("\r\n[DLMS_APP] Fail sending Ping to 0x%04x!  Error: %d \r\n",
						px_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address, x_error));
			}
		}
	}
#endif
}

/**
 * \brief Initialize ping Application
 *
 */
void ping_app_init(void)
{
#ifdef DLMS_APP_ENABLE_PING_CYCLE
	sb_available_buffer = 1;
	/* Initialize asynchronous ping module */
	async_ping_init();
	sb_ping_sent = false;
	_ipv6_mng_ping_init_stats();
#endif
}
