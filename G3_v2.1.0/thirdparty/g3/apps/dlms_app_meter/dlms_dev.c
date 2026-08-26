/**
 * \file
 *
 * \brief DLMS_APP : DLMS example application for ATMEL G3 Device
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

#include "conf_project.h"

#include "compiler.h"
#include "dlms_dev.h"
#include "app_dispatcher.h"
#include "oss_if.h"
#include "dlms_srv_lib.h"
#include "dlms_srv_data.h"
#include "hdlc.h"

#ifdef DLMS_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

/** Association configuration: in this example, four possible associations: MGMT(1), READ(2), FW(3) and PUBLIC(16) */
static const assoc_conf_t px_assoc_conf[DLMS_MAX_ASSOC] = {
	{DLMS_MGMT_SRC_PORT, DLMS_MGMT_DST_PORT, LLS_FIXED_PWD, "00001234", COSEM_LOW_LEVEL_SEC}, /* MODEM MGMT */
	{0x0001, 0x0001, LLS_FIXED_PWD, "00000002", COSEM_LOW_LEVEL_SEC},    /* MGMT */
	{0x0002, 0x0001, LLS_FIXED_PWD, "00000001", COSEM_LOW_LEVEL_SEC},    /* READ */
	/*	{0x0003, 0x0001, LLS_FIXED_PWD, "00000003", COSEM_LOW_LEVEL_SEC}, */   /* FW */
	{0x0010, 0x0001, LLS_FIXED_PWD, "--------", COSEM_LOWEST_LEVEL_SEC}  /* PUBLIC */
};

/** Meter params */
static meter_params_t sx_meter_params;

extern struct dlms_msg *app_ptr_rx_dlms_msg;
extern struct dlms_msg *app_ptr_tx_dlms_msg;

/**
 * \brief Sending data from DLMS Server lib to 4-32 connection
 *
 * \param us_dst_wport     Destination Wrapper Port
 * \param us_src_wport     Source Wrapper Port
 * \param px_buff          Pointer to the data buffer
 * \param us_buff_len      Length of the data
 */
static void _dlms_app_data_request(uint16_t us_dst_wport, uint16_t us_src_wport, uint8_t *puc_buff, uint16_t us_buff_len)
{
	bool success;

	if ((us_dst_wport == DLMS_MGMT_SRC_PORT) && (us_src_wport == DLMS_MGMT_DST_PORT)) {
		/* Sent over HDLC: NO wrapper required */
		memcpy(app_ptr_tx_dlms_msg->buf, puc_buff, us_buff_len);
		app_ptr_tx_dlms_msg->length = us_buff_len;
	} else {
		/* Sent over IPv6: Wrapper required */
		app_ptr_tx_dlms_msg->length = dlms_srv_add_wrapper_header(app_ptr_tx_dlms_msg->buf, us_dst_wport, us_src_wport, puc_buff, us_buff_len);
	}

	app_ptr_tx_dlms_msg->todo = 1;

	success = true;
	dlms_srv_data_cfm(us_dst_wport, us_src_wport, success);
}

/**
 * \brief Initialize DLMS Application
 *
 */
void dlms_app_init(void)
{
	/*                                 OBIS CODE + IC        READ:  MGMT   READ        FW          PUBLIC       WRITE    */
	obis_element_conf_t x_new_obis = {{0, 0, 0, 0, 0, 0}, 0, {0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00}, {0, 0, 0, 0}, NULL};

	/* Generate Serial Number */
	dispatcher_get_meter_id(sx_meter_params.meter_serial);

	/* DLMS Server lib init */
	dlms_srv_init(px_assoc_conf, DLMS_MAX_ASSOC, &sx_meter_params, _dlms_app_data_request);

	/* DLMS Server objects configuration */
	dlms_srv_conf_obis(1, 0, 0, 2, 0, 255, 1, obis_1_0_0_2_0_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 96, 1, 0, 255, 1, obis_0_0_96_1_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 96, 1, 1, 255, 1, obis_0_0_96_1_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 96, 1, 2, 255, 1, obis_0_0_96_1_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 0, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 10, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 11, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 12, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 13, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 14, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 15, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 16, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 20, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 21, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 22, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 23, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 24, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 25, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 26, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 30, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 31, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 32, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 33, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 34, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 35, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 36, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 1, 8, 255, 255, 3, obis_1_0_1_8_x_255_cb, &x_new_obis);
	dlms_srv_conf_obis(1, 0, 99, 1, 0, 255, 7, obis_1_0_99_1_0_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 21, 0, 5, 255, 7, obis_0_0_21_0_5_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 21, 0, 6, 255, 7, obis_0_0_21_0_6_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 1, 0, 0, 255, 8, obis_0_0_1_0_0_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 28, 7, 0, 255, 86, obis_0_0_28_7_0_255_cb, &x_new_obis);

	/* G3 Server specific objects */
	dlms_srv_conf_obis(0, 0, 29, 0, 0, 255, 90, obis_0_0_29_0_0_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 29, 1, 0, 255, 91, obis_0_0_29_1_0_255_cb, &x_new_obis);
	dlms_srv_conf_obis(0, 0, 29, 2, 0, 255, 92, obis_0_0_29_2_0_255_cb, &x_new_obis);
	/* Manufacturer specific objects */
	dlms_srv_conf_obis(0, 0, 29, 2, 2, 128, 1, obis_0_0_29_2_2_128_cb, &x_new_obis);

	/* Current association object list */
	dlms_srv_conf_obis(0, 0, 40, 0, 0, 255, 15, obis_0_0_40_0_0_255_cb, &x_new_obis);
}
