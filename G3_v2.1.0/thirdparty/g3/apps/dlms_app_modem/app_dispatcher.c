/**
 * \file
 *
 * \brief Dispatcher application to manage data from/to socket & serial port
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
#include <string.h>
#include <stdio.h>

#include <storage/storage.h>
#include <hal/hal.h>

#include "compiler.h"

#include "drivers/g3/network_adapter_g3.h"
#include "app_dispatcher.h"
#include "app_adp_mng.h"
#include "g3_app_config.h"
#include "conf_project.h"
#include "conf_board.h"
#include "oss_if.h"
#include "hdlc.h"
#include "Logger.h"

/* MAC include */
#include "mac_wrapper.h"
#include "serial_buffer.h"

#ifdef SIMPLE_MGMT
	#include "simple_mgmt.h"
#endif /* #ifdef SIMPLE_MGMT */
#ifdef DLMS_MGMT
	#include "dlms_mgmt.h"
#endif /* #ifdef DLMS_MGMT */

/* Add IPv6 - G3-ADP support */
#include "AdpApi.h"
#include "ipv6_mng.h"

#ifdef DLMS_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

extern uint8_t g_u8NetworkJoinStatus;

/* APP 1ms timer */
uint16_t HDLC_iframe_tout;

/* SERIAL_IF input and output buffers */
struct buff_msg app_rx_serial;
struct buff_msg app_tx_serial;

/* DLMS input and output buffers */
struct dlms_msg app_rx_dlms_msg, app_tx_dlms_msg, *app_ptr_rx_dlms_msg, *app_ptr_tx_dlms_msg;

bool g_bHasMeterId;
uint8_t g_auc_ext_address[8]; /* EUI64 */

/* Delay to re-send meter-id request in milliseconds */
  #define CONF_METER_ID_REQ_TIMEOUT  1000u
static uint32_t u32DelayMeterIdReq;

/* Vars for DLMS socket communication */
/* Socket for the UDP over PLC communication */
Socket *spx_dlms_socket;
/* Local IP address */
IpAddr sx_dlms_local_ip_addr;
/* Local Socket PORT */
uint16_t sus_dlms_udp_port;
/* Socket opened */
bool b_is_dlms_socket_open = false;

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
 * Convert functions
 **/
static uint8_t _hex_2_char(uint8_t num)
{
	if (num > 9) {
		return (num + 0x37);
	} else {
		return (num + 0x30);
	}
}

static uint8_t _char_2_hex(uint8_t c)
{
	uint8_t hex;
	if (c >= '0' && c <= '9') {
		hex = (c - '0');
	} else if (c >= 'A' && c <= 'F') {
		hex = (10 + (c - 'A'));
	} else if (c >= 'a' && c <= 'f') {
		hex = (10 + (c - 'a'));
	} else {
		hex = 0;
	}

	return hex;
}

/**
 * \brief Set the meter ID
 *
 * \return puc_tx_buff          Pointer to the data tx buffer
 */
void dispatcher_set_meter_id(uint8_t *pui_meter_id)
{
	uint8_t i, j;
	for (i = 0, j = 0; i < 8; i++, j += 2) {
		g_auc_ext_address[i] = (_char_2_hex(pui_meter_id[j]) << 4) + _char_2_hex(pui_meter_id[j + 1]);
	}
	g_bHasMeterId = true;

#ifdef DLMS_DEBUG_CONSOLE
	LogBuffer(g_auc_ext_address, 8, "\r\nExtended address: ");
	printf("\r\n");
#endif
}

/**
 * \brief Set the meter ID
 *
 * \return puc_tx_buff          Pointer to the data tx buffer
 */
void dispatcher_get_meter_id(uint8_t *pui_meter_id)
{
	uint8_t i;
	uint8_t uc_num, uc_num1;

	#ifdef DLMS_DEBUG_CONSOLE
	LogBuffer(g_auc_ext_address, 8, "\r\nExtended address: ");
	printf("\r\n");
	#endif
	/* convert hex to ascii */
	for (i = 0; i < 8; i++) {
		uc_num  = ((g_auc_ext_address[i] & 0xf0) >> 4);
		uc_num1 = (g_auc_ext_address[i] & 0x0f);
		pui_meter_id[2 * (i)] = _hex_2_char(uc_num);
		pui_meter_id[2 * (i) + 1] = _hex_2_char(uc_num1);
	}
}

/**
 * \brief Timers of DLMS Application
 *
 */
void dispatcher_timers_update(void)
{
	if (u32DelayMeterIdReq) {
		u32DelayMeterIdReq--;
	}

#ifdef DLMS_MGMT
	dlms_mgmt_update_1ms();
#endif /* DLMS_MGMT */

	if (HDLC_iframe_tout) {
		HDLC_iframe_tout--;
	}
}

#if defined(SIMPLE_MGMT)

/**
 * \brief Exchanges sends dlms buffer through serial transparently (do not use HDLC)
 *
 */
static void transparent_dlms_to_serial(void)
{
	/* If there is something to tx in dlms buffer, copy to serial buffer*/
	if (app_ptr_tx_dlms_msg->todo == 1) {
		memcpy(app_tx_serial.msg[app_tx_serial.ptr_wr].buf, app_ptr_tx_dlms_msg->buf, app_ptr_tx_dlms_msg->length);
		app_tx_serial.msg[app_tx_serial.ptr_wr].length = app_ptr_tx_dlms_msg->length;
		app_tx_serial.msg[app_tx_serial.ptr_wr].todo = 1;
		if (++app_tx_serial.ptr_wr == MSG_BUF_SIZE) {
			app_tx_serial.ptr_wr = 0;
		}

		app_ptr_tx_dlms_msg->todo = 0;
	}
}

#endif /* #if defined(SIMPLE_MGMT) */

#ifdef DLMS_MGMT
void hdlc_dlms_to_serial(void)
{
	hdlc_TxProcess(&app_tx_serial.msg[app_tx_serial.ptr_wr], app_ptr_tx_dlms_msg);
	if (++app_tx_serial.ptr_wr == MSG_BUF_SIZE) {
		app_tx_serial.ptr_wr = 0;
	}

	app_ptr_tx_dlms_msg->todo = 0;
}

#endif /* DLMS_MGMT */

/**
 * \brief Process of IPv6 to Serial port tx & rx
 *
 */
static uint8_t ipv6_to_serial(struct buff_msg *ipv6_ptr_rx, struct buff_msg *ipv6_ptr_tx)
{
	size_t tx_bytes, rx_bytes;
	if (ipv6_ptr_tx->ptr_rd != ipv6_ptr_tx->ptr_wr) {
		/* #ifdef DUMP_CONSOLE */
		/*      LogDump(ipv6_ptr_tx->msg[ipv6_ptr_tx->ptr_rd].buf, ipv6_ptr_tx->msg[ipv6_ptr_tx->ptr_rd].length); */
		/* #endif / * DUMP_CONSOLE * / */
		if (udp_socket_send(spx_dlms_socket, &sx_dlms_local_ip_addr, sus_dlms_udp_port, ipv6_ptr_tx->msg[ipv6_ptr_tx->ptr_rd].buf, ipv6_ptr_tx->msg[ipv6_ptr_tx->ptr_rd].length, &tx_bytes)) {
			ipv6_ptr_tx->msg[ipv6_ptr_tx->ptr_rd].todo = 0;
			if (++ipv6_ptr_tx->ptr_rd == MSG_BUF_SIZE) {
				ipv6_ptr_tx->ptr_rd = 0;
			}
		}
	}

	if (udp_socket_receive(spx_dlms_socket, &sx_dlms_local_ip_addr, &sus_dlms_udp_port, ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].buf, SERIAL_BUF_SIZE, &rx_bytes)) {
		if (rx_bytes > 0) {
			if ((ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].buf[0] == 0) && (ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].buf[1] == 1)) {
				/* DATA TRANSFER */

				/* SEND NETWORK MANAGEMENT MESSAGES */

				/* SEND the IPv6 unpacked data frame in DLMS WRAPPER to meter */
				/* #ifdef DUMP_CONSOLE */
				/*        LogDump(ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].buf, (uint16_t)rx_bytes); */
				/* #endif / * DUMP_CONSOLE * / */
				ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].length = (uint16_t)rx_bytes;
				ipv6_ptr_rx->msg[ipv6_ptr_rx->ptr_wr].todo = 1;
				if (++ipv6_ptr_rx->ptr_wr == MSG_BUF_SIZE) {
					ipv6_ptr_rx->ptr_wr = 0;
				}

				return 1;
			}
		}
	}

	return 0;
}

/**
 * \brief Process of Dispatcher Application
 *
 */
void dispatcher_app_process(void)
{
	serial_if_Process(&app_rx_serial, &app_tx_serial, &HDLC_iframe_tout);

	/* PROCESS RECEIVED DATA FROM SERIAL */
	if (app_rx_serial.ptr_rd != app_rx_serial.ptr_wr) {
		if (!g_bHasMeterId) {
#ifdef SIMPLE_MGMT
			simple_mgmt_decode(app_rx_serial.msg[app_rx_serial.ptr_rd].buf, app_rx_serial.msg[app_rx_serial.ptr_rd].length);
			app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
#endif
#ifdef DLMS_MGMT
			if (app_rx_serial.msg[app_rx_serial.ptr_rd].buf[0] == HDLC_START_END_FLAG) {
				/* HDLC RESPONSE */
				hdlc_RxProcess(app_ptr_rx_dlms_msg, &app_rx_serial.msg[app_rx_serial.ptr_rd]);
				if (app_ptr_rx_dlms_msg->todo == 1) {
					dlms_mgmt_decode(app_ptr_rx_dlms_msg->buf, app_ptr_rx_dlms_msg->length);
					app_ptr_rx_dlms_msg->todo = 0;
				}
			} else {
				/* MESSAGE DISCARD */
				app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
			}
#endif /* #ifdef DLMS_MGMT */
			/* Increase pointer of serial read buffer */
			if (++app_rx_serial.ptr_rd == MSG_BUF_SIZE) {
				app_rx_serial.ptr_rd = 0;
			}
		} else {
			if (g_u8NetworkJoinStatus == NJS_NOT_JOINED) {
				/* MESSAGE DISCARD */
				app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
				/* Increase pointer of serial read buffer */
				if (++app_rx_serial.ptr_rd == MSG_BUF_SIZE) {
					app_rx_serial.ptr_rd = 0;
				}
			} else {
				if (app_rx_serial.msg[app_rx_serial.ptr_rd].buf[0] == 0x00) {
					/* DLMS WRAPPER MESSAGE --> ipv6_to_serial */
				} else {
					/* UNRECOGNIZABLE MESSAGE --> DISCARD */
					app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
					/* Increase pointer of serial read buffer */
					if (++app_rx_serial.ptr_rd == MSG_BUF_SIZE) {
						app_rx_serial.ptr_rd = 0;
					}
				}
			}
		}
	}

	/* TX PROCESS TO SERIAL */
	if (!g_bHasMeterId) {
		if (!u32DelayMeterIdReq) {
			/* Request meter-id */
#ifdef SIMPLE_MGMT
			simple_mgmt_send_meter_id_req();
			transparent_dlms_to_serial();
			u32DelayMeterIdReq = CONF_METER_ID_REQ_TIMEOUT;
#endif /* #ifdef SIMPLE_MGMT */
#ifdef DLMS_MGMT
			/* dlms_mgmt_send_meter_id_req(); */
			dlms_mgmt_process();
			hdlc_dlms_to_serial();
#endif /* #ifdef DLMS_MGMT */

			/* u32DelayMeterIdReq = CONF_METER_ID_REQ_TIMEOUT; */
		}
	}

	if (ipv6_mng_ready()) {
		if (b_is_dlms_socket_open && udp_socket_is_open(spx_dlms_socket)) {
			/* Process IPv6 */
			ipv6_to_serial(&app_tx_serial, &app_rx_serial);
		} else {
			/* Open socket */
			b_is_dlms_socket_open = udp_socket_open(spx_dlms_socket, DLMS_SOCKET_PORT);
		}
	}
}

/**
 * \brief Initialize DLMS Application for MODEM
 *
 */
void dispatcher_app_init(void)
{
	/* Init APP SERIAL_IF structures and open SERIAL PORT*/
	app_rx_serial.ptr_wr = 0;
	app_rx_serial.ptr_rd = 0;
	app_tx_serial.ptr_wr = 0;
	app_tx_serial.ptr_rd = 0;
	serial_if_init();

	/* Init APP HDLC structures (RESET HDLC_iframe_tout */
	HDLC_iframe_tout_init(&HDLC_iframe_tout);

	/* Init APP DLMS structures*/
	app_ptr_rx_dlms_msg = &app_rx_dlms_msg;
	app_ptr_tx_dlms_msg = &app_tx_dlms_msg;
	app_rx_dlms_msg.todo = 0;
	app_tx_dlms_msg.todo = 0;

	g_bHasMeterId = false;
	u32DelayMeterIdReq = 0;

#ifdef DLMS_MGMT
	/* Init DLMS mgmt */
	dlms_mgmt_init();
#endif /* DLMS_MGMT */

	/* Initialize socket */
	spx_dlms_socket = udp_socket_initialize();

	if (_enter_conformance_mode()) {
		adp_app_set_conformance_config();
		ipv6_mng_set_conformance_config();
		/* Force G3 call every 1ms for Conformance mode */
		platform_cfg_call_process_rate(1);
	}

	LOG_APP_DEBUG(("[DLMS_APP] DLMS Application: DEVICE MODEM\r\n"));
	_show_version();
}
