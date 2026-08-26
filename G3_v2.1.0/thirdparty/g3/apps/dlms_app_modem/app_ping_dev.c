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

#include "app_ping_dev.h"
#include "Logger.h"
#include "error.h"

#include "app_adp_mng.h"
#include "drivers/g3/network_adapter_g3.h"
#include "async_ping.h"

#ifdef PING_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
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

#if defined(DLMS_APP_ENABLE_PING_ALARM)
/* PING stats */
static uint32_t ul_pings_sent = 0;
static uint32_t ul_pings_successful = 0;
static uint32_t ul_ping_errors = 0;
static uint8_t uc_ping_sent = 0;
#endif /* #if defined(DLMS_APP_ENABLE_PING_ALARM) */

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

/**
 * \brief Process of Dispatcher Application
 *
 */
void ping_app_process(void)
{
#ifdef DLMS_APP_ENABLE_PING_ALARM
	/* Process incoming pings */
	error_t x_error;

	/* Process pings */
	if (uc_ping_sent) {
		systime_t roundTripTime;

		x_error = async_ping_rcv( &roundTripTime);
		if (x_error == ERROR_TIMEOUT) {
			/* LOG_APP_DEBUG(("[DLMS_APP] Ping Process Timeout  Error: %d \r\n", x_error)); */
		} else if (x_error != NO_ERROR) {
			ul_ping_errors += 1;
			LOG_APP_DEBUG(("[DLMS_APP] Unsuccessful Ping!  Error: %d \r\n", x_error));
			LOG_APP_DEBUG(("[DLMS_APP] Ping Report  -->  Pings sent: %u  ; Successful Ping: %u; Errors: %u Success Rate : %.1f\r\n", ul_pings_sent,
					ul_pings_successful, ul_ping_errors, (100.0 * (double)ul_pings_successful) / ((double)ul_pings_sent)));
			uc_ping_sent = 0;
		} else {
			ul_pings_successful += 1;
			LOG_APP_DEBUG(("[DLMS_APP] Ping Successful! RoundTripTime: %u\r\n", roundTripTime));
			LOG_APP_DEBUG(("[DLMS_APP] Ping Report  -->  Pings sent: %u  ; Successful Ping: %u; Errors: %u Success Rate : %.1f\r\n", ul_pings_sent,
					ul_pings_successful, ul_ping_errors, (100.0 * (double)ul_pings_successful) / ((double)ul_pings_sent)));
			uc_ping_sent = 0;
		}
	}

	/* Process outgoing pings */

	if ((u32PingTimer == 0xFFFFFFFF) && (g_u8NetworkJoinStatus == NJS_JOINED)) {
		/* Start timer for the first time */
		u32PingTimer = DLMS_APP_INITIAL_PING_ALARM_TIMER_INTERVAL * 1000;
	} else if (u32PingTimer == 0) {
		/* Relaunch timer */
		error_t x_error;
		/* Local IP address */
		static IpAddr s_remoteIpAddr;

		s_remoteIpAddr.length = sizeof(Ipv6Addr);

		/* Set link-local address, based on the PAN_ID and the short address */
		ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &s_remoteIpAddr.ipv6Addr);
		/* Adapt the IPv6 address to the G3 connection data */
		s_remoteIpAddr.ipv6Addr.b[8] = (uint8_t)(sx_local_ip_addr.ipv6Addr.b[8]);
		s_remoteIpAddr.ipv6Addr.b[9] = (uint8_t)(sx_local_ip_addr.ipv6Addr.b[9]);
		s_remoteIpAddr.ipv6Addr.b[14] = (uint8_t)(0);
		s_remoteIpAddr.ipv6Addr.b[15] = (uint8_t)(0);

		if (!uc_ping_sent) {
			x_error
				= async_ping_send(&netInterface[0], &s_remoteIpAddr, DLMS_APP_PING_ALARM_LEN, DLMS_APP_PING_ALARM_TTL, DLMS_APP_PING_ALARM_TIMEOUT *
					1000);

			if (x_error == NO_ERROR) {
				LOG_APP_DEBUG(("\r\n[DLMS_APP] Ping Sent ok!\r\n"));
				ul_pings_sent += 1;
				uc_ping_sent = 1;
			} else {
				LOG_APP_DEBUG(("\r\n[DLMS_APP] Fail sending Ping!  Error: %d \r\n", x_error));
			}
		} else {
			LOG_APP_DEBUG(("\r\n[DLMS_APP] Ping Skipped!\r\n"));
		}

		u32PingTimer = DLMS_APP_PING_ALARM_TIMER_INTERVAL * 1000;
	}
#endif /* #ifdef DLMS_APP_ENABLE_PING_ALARM */
}

/**
 * \brief Initialize ping Application
 *
 */
void ping_app_init(void)
{
#ifdef DLMS_APP_ENABLE_PING_ALARM
	/* Initialize asynchronous ping module */
	async_ping_init();
#endif /* #ifdef DLMS_APP_ENABLE_PING_ALARM */
}
