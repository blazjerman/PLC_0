/**
 * \file
 *
 * \brief Dispatcher application to manage data from/to serial port & DLMS
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

#include <hal/hal.h>

#include "compiler.h"

#include "app_dispatcher.h"
#include "g3_app_config.h"
#include "conf_project.h"
#include "conf_board.h"
#include "oss_if.h"
#include "hdlc.h"
#include "Logger.h"
#include "serial_buffer.h"

#ifdef SIMPLE_MGMT
	#include "simple_mgmt.h"
#endif /* #ifdef SIMPLE_MGMT */

/* Add DLMS support */
	#include "dlms_dev.h"
	#include "dlms_srv_lib.h"
	#include "dlms_srv_data.h"

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

/**
 * \brief Display SW version in console
 */
static void _show_version( void )
{
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

/**
 * \brief Set the meter ID
 *
 * \return puc_tx_buff          Pointer to the data tx buffer
 */
void dispatcher_set_meter_id(uint8_t *pui_meter_id)
{
	/* Only for MODEM */
	UNUSED(pui_meter_id);
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
	if (HDLC_iframe_tout) {
		HDLC_iframe_tout--;
	}
}

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
 * \brief Process of DLMS Application for METER
 *
 */
void dispatcher_app_process(void)
{
	serial_if_Process(&app_rx_serial, &app_tx_serial, &HDLC_iframe_tout);

	/* PROCESS RECEIVED DATA FROM SERIAL */
	if (app_rx_serial.ptr_rd != app_rx_serial.ptr_wr) {
#ifdef SIMPLE_MGMT
		if (simple_mgmt_decode(app_rx_serial.msg[app_rx_serial.ptr_rd].buf, app_rx_serial.msg[app_rx_serial.ptr_rd].length)) {
			app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
		}
#endif
#ifdef DLMS_MGMT
		if (app_rx_serial.msg[app_rx_serial.ptr_rd].buf[0] == HDLC_START_END_FLAG) {
			/* HDLC RESPONSE */
			hdlc_RxProcess(app_ptr_rx_dlms_msg, &app_rx_serial.msg[app_rx_serial.ptr_rd]);
			if (app_ptr_rx_dlms_msg->todo == 1) {
				/* Pass the data to DLMS Server (DLMS over HDLC comes without wrapper) */
				/* #ifdef DLMS_DEBUG_CONSOLE */
				/*        LogBuffer(app_ptr_rx_dlms_msg->buf, app_ptr_rx_dlms_msg->length, "\r\n*** dlms_srv_data_ind(): "); */
				/*        printf("\r\n"); */
				/* #endif */
				/* ToDo: Extract addresses from HDLC */
				dlms_srv_data_ind(DLMS_MGMT_DST_PORT /* us_dst_wport */, DLMS_MGMT_SRC_PORT /* us_src_wport */, app_ptr_rx_dlms_msg->buf, app_ptr_rx_dlms_msg->length);
				app_ptr_rx_dlms_msg->todo = 0;
				dlms_srv_process();
			}
		}
#endif /* #ifdef DLMS_MGMT */

		/* If serial Rx hasn't been processed, pass to DLMS wrapper */
		if (app_rx_serial.msg[app_rx_serial.ptr_rd].todo == 1) {
			dlms_srv_wrapper_data_ind(app_rx_serial.msg[app_rx_serial.ptr_rd].buf, app_rx_serial.msg[app_rx_serial.ptr_rd].length);
			app_rx_serial.msg[app_rx_serial.ptr_rd].todo = 0;
		}

		/* Increase pointer of serial read buffer */
		if (++app_rx_serial.ptr_rd == MSG_BUF_SIZE) {
			app_rx_serial.ptr_rd = 0;
		}
	}

	/* Process DLMS */

	/* TX PROCESS TO SERIAL */
	if (app_ptr_tx_dlms_msg->todo == 1) {
		/* If there is something to tx, it is mgmt, not dlms wrapper. Use HDLC if required */
#ifdef SIMPLE_MGMT
		transparent_dlms_to_serial();
#endif
#ifdef DLMS_MGMT
		hdlc_dlms_to_serial();
#endif
	} else {
		dlms_srv_process();
		/* If there is something to tx, it is dlms wrapper, not mgmt. Copy to serial buffer to tx */
		transparent_dlms_to_serial();
	}
}

/**
 * \brief Initialize DLMS Application for METER
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

	/* Get MAC / Meter-id */
	platform_init_eui64(g_auc_ext_address);
	g_bHasMeterId = true;

	dlms_app_init();

	LOG_APP_DEBUG(("[DLMS_APP] DLMS Application: DEVICE METER\r\n"));
	_show_version();
}
