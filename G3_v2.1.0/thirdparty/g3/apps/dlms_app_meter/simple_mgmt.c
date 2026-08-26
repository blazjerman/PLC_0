/**
 * \file
 *
 * \brief Simple protocol for meter - modem management
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
#include <stdio.h>
#include <string.h>
#include "conf_project.h"
#include "g3_app_config.h"

#include "Logger.h"
#include "app_dispatcher.h"
#include "hdlc.h"
#include "simple_mgmt.h"

#ifdef SIMPLE_MGMT_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

#ifdef SIMPLE_MGMT

extern struct dlms_msg *app_ptr_rx_dlms_msg;
extern struct dlms_msg *app_ptr_tx_dlms_msg;

/**
 * \brief Calculates a simple 8-bit checksum
 */
static uint8_t _calculate_checksum(const unsigned char *buff, size_t len)
{
	uint8_t sum;
	for (sum = 0; len != 0; len--) {
		sum += *(buff++);
	}
	return sum;
}

/**
 * \brief Send meter-id request
 */
uint8_t simple_mgmt_send_meter_id_req()
{
	uint8_t len;
	len = 0;

	LOG_APP_DEBUG(("[SIMPLE_MGMT] simple_mgmt_send_meter_id_req().\r\n"));
	app_ptr_tx_dlms_msg->length = 0;

	/* Simple management header */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_0;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_1;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_2;
	/* Read command */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_READ;
	/* Data length */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = 2;
	/* Data tag (Meter ID) */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_DATA_METER_ID_0;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_DATA_METER_ID_1;
	/* Checksum */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = _calculate_checksum(app_ptr_tx_dlms_msg->buf, 7);
	/* Frame End */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_END;

	app_ptr_tx_dlms_msg->todo = 1;

	return len;
}

/**
 * \brief Send meter-id response
 */
static uint8_t simple_mgmt_send_meter_id_res(uint8_t *ptr_meter_id)
{
	uint8_t len;
	len = 0;

	LOG_APP_DEBUG(("[SIMPLE_MGMT] simple_mgmt_send_meter_id_res().\r\n"));
	app_ptr_tx_dlms_msg->length = 0;

	/* Simple management header */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_0;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_1;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_HEADER_2;
	/* Read command */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_CONTROL_CODE;
	/* Data length */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = 18;
	/* Data tag (Meter ID) */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_DATA_METER_ID_0;
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_DATA_METER_ID_1;
	/* Copy meter id */
	memcpy(&app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length], ptr_meter_id, 16);
	app_ptr_tx_dlms_msg->length += 16;
	/* Checksum */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = _calculate_checksum(app_ptr_tx_dlms_msg->buf, 23);
	/* Frame End */
	app_ptr_tx_dlms_msg->buf[app_ptr_tx_dlms_msg->length++] = SIMPLE_MGMT_FRAME_END;

	app_ptr_tx_dlms_msg->todo = 1;

	return len;
}

/**
 * \brief Decode simple management messages
 */
bool simple_mgmt_decode(uint8_t *ptr_buff, uint8_t len)
{
	uint8_t calc_chk;
	uint8_t rec_chk;
	uint8_t received_len;
	received_len = 0;

	/* Check minimum length for header + command */
	if (len < 4) {
		return false;
	}

	/* Simple management header */
	if ((ptr_buff[0] != SIMPLE_MGMT_FRAME_HEADER_0) ||
			(ptr_buff[1] != SIMPLE_MGMT_FRAME_HEADER_1) ||
			(ptr_buff[2] != SIMPLE_MGMT_FRAME_HEADER_2)) {
		return false;
	}

	/* Read length */
	received_len = ptr_buff[4];

	/* Check minimum length according to received length */
	if (len < received_len + 7) {
		LOG_APP_DEBUG(("[SIMPLE_MGMT] message smaller than minimum!\r\n"));
		return false;
	}

	/* Test received checksum */
	rec_chk = ptr_buff[5 + received_len];
	calc_chk =      _calculate_checksum(ptr_buff, 5 + received_len);
	/* Check minimum length according to received length */
	if (rec_chk != calc_chk) {
		LOG_APP_DEBUG(("[SIMPLE_MGMT] wrong checksum!\r\n"));
		return false;
	}

	/* decode command */
	switch (ptr_buff[3]) {
	case SIMPLE_MGMT_READ:
		if ((ptr_buff[5] == SIMPLE_MGMT_DATA_METER_ID_0) && (ptr_buff[6] == SIMPLE_MGMT_DATA_METER_ID_1)) {
			uint8_t meter_id[16];
			LOG_APP_DEBUG(("[SIMPLE_MGMT] SIMPLE_MGMT_READ.SIMPLE_MGMT_DATA_METER_ID received.\r\n"));
			dispatcher_get_meter_id(meter_id);
			simple_mgmt_send_meter_id_res(meter_id);
		} else {
			LOG_APP_DEBUG(("[SIMPLE_MGMT] unknown tag!\r\n"));
			return false;
		}

		break;
	}

	return true;
}

#endif /* SIMPLE_MGMT */
