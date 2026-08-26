/**
 * \file
 *
 * \brief DLMS for meter - modem management
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

#include "Logger.h"
#include "AdpApi.h"
#include "MacMib.h"
#include "mac_wrapper_defs.h"
#include "app_dispatcher.h"
#include "hdlc.h"
#include "dlms_cli_lib.h"
#include "dlms_cli_data.h"
#include "dlms_mgmt.h"

#ifdef DLMS_MGMT_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

#ifdef DLMS_MGMT

extern struct dlms_msg *app_ptr_rx_dlms_msg;
extern struct dlms_msg *app_ptr_tx_dlms_msg;
extern bool g_bHasMeterId;

/* Objects to get (to configure stack) */
#define NUM_OBJECTS_TO_GET            24
static const dlms_object_t objects_to_get[NUM_OBJECTS_TO_GET] = {
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_TONE_MASK},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_TMR_TTL},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_MAX_FRAME_RETRIES},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_NEIGHBOUR_TABLE_ENTRY_TTL},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_HIGH_PRIORITY_WINDOW_SIZE},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_CSMA_FAIRNESS_LIMIT},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_BEACON_RANDOMIZATION_WINDOW_LENGTH},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_A},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_K},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_MIN_CW_ATTEMPTS},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_CENELEC_LEGACY_MODE},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_FCC_LEGACY_MODE},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_MAX_BE},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_MAX_CSMA_BACKOFFS},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_MIN_BE},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_MAX_HOPS},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_WEAK_LQI_VALUE},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_SECURITY_LEVEL},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_BROADCAST_LOG_TABLE_ENTRY_TTL},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_MAX_JOIN_WAIT_TIME},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_PATH_DISCOVERY_TIME},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_ACTIVE_KEY_INDEX},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_METRIC_TYPE},
	{{0, 0, 29, 2, 2, 128}, 1, IC01_VALUE}
};

#define NUM_OBJECTS_TO_SET 9
#define VALUES_BUFFER_SIZE NUM_OBJECTS_TO_SET * 5
dlms_object_t objects_to_set[NUM_OBJECTS_TO_SET] = {
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_DATA_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_DATA_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_CMD_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_CMD_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_CSMA_FAIL_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_CSMA_NO_ACK_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_BAD_CRC_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_DATA_BROADCAST_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_DATA_BROADCAST_COUNT},
};

static uint8_t spuc_date_time_start[] = {0x07, 0xDE, 0x05, 0x15, 0x01, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80};
static uint8_t spuc_date_time_end[] = {0x07, 0xDE, 0x05, 0x17, 0x01, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80};

/** Association configuration: in this example, four possible associations: MGMT(1), READ(2), FW(3) and PUBLIC(16) */
#define ASSOC_MGMT_IDX             0
#define ASSOC_READ_IDX             1
#define ASSOC_FW_IDX               2
#define ASSOC_PUBLIC_IDX           3

static assoc_conf_t px_assoc_conf[DLMS_MAX_ASSOC] = {
	{DLMS_MGMT_SRC_PORT, DLMS_MGMT_DST_PORT, LLS_FIXED_PWD, "00001234", COSEM_LOW_LEVEL_SEC},    /*  MODEM MGMT */
	/*	{0x0001, 0x0001, LLS_FIXED_PWD, "00000002", COSEM_LOW_LEVEL_SEC},    / *  MGMT  * / */
	/*	{0x0001, 0x0002, LLS_FIXED_PWD, "00000001", COSEM_LOW_LEVEL_SEC},    / *  READ  * / */
	/*	{0x0001, 0x0003, LLS_FIXED_PWD, "00000003", COSEM_LOW_LEVEL_SEC},    / *   FW   * / */
	/*	{0x0001, 0x0010, LLS_FIXED_PWD, "--------", COSEM_LOWEST_LEVEL_SEC}  / * PUBLIC * / */
};

static enum EMacWrpPibAttribute ic90_to_g3_mac(uint8_t attr)
{
	switch (attr) {
	case IC90_MAC_TX_DATA_PACKET_COUNT:
		return MAC_WRP_PIB_TX_DATA_PACKET_COUNT;

	case IC90_MAC_RX_DATA_PACKET_COUNT:
		return MAC_WRP_PIB_RX_DATA_PACKET_COUNT;

	case IC90_MAC_TX_CMD_PACKET_COUNT:
		return MAC_WRP_PIB_TX_CMD_PACKET_COUNT;

	case IC90_MAC_RX_CMD_PACKET_COUNT:
		return MAC_WRP_PIB_RX_CMD_PACKET_COUNT;

	case IC90_MAC_CSMA_FAIL_COUNT:
		return MAC_WRP_PIB_CSMA_FAIL_COUNT;

	case IC90_MAC_CSMA_NO_ACK_COUNT:
		return MAC_WRP_PIB_CSMA_NO_ACK_COUNT;

	case IC90_MAC_BAD_CRC_COUNT:
		return MAC_WRP_PIB_BAD_CRC_COUNT;

	case IC90_MAC_TX_DATA_BROADCAST_COUNT:
		return MAC_WRP_PIB_TX_DATA_BROADCAST_COUNT;

	case IC90_MAC_RX_DATA_BROADCAST_COUNT:
		return MAC_WRP_PIB_RX_DATA_BROADCAST_COUNT;

	default:
		return (enum EMacWrpPibAttribute)0xFFFFFFFF;
	}
}

/** Node info */
#define DLMS_MAX_DEV_NUM 1 /* Only one device: METER */
static node_info_t spx_node_info[DLMS_MAX_DEV_NUM];

enum {
	STATE_ASSOC_REQ,         /* Association request */
	STATE_WAIT_ASSOC_RESP,   /* Wait association response */
	STATE_REQ_OBJECT,        /* Object request */
	STATE_WAIT_REQ_RESP,     /* Wait object response */
	STATE_RELEASE_REQ,       /* End cycle. Request release association */
	STATE_WAIT_RELEASE_RESP, /* Wait association response */
	STATE_FINISHED,    /* DLMS mgmt finished */
}
uc_state;

static uint16_t us_meter_addr = 0xAABB;
/* static uint8_t password[LLS_PASSWORD_LEN]; */
static uint8_t uc_assoc_idx = ASSOC_MGMT_IDX;

static dlms_cli_result_t uc_current_req_result;
static uint8_t suc_current_object_idx;
static uint8_t uc_num_req_objects;
static uint32_t sul_data_timeout_timer;

/* DLMS CONFIGURATION STAGE FLAG */
uint8_t STACK_CONF = 1;

/* static void copy_IPv6address(uint8_t *buff_out, uint8_t *buff_in) */
/* { */
/*	uint8_t n = 0; */
/*  */
/*	while (n < 16){ */
/*		buff_out[n] = buff_in[n]; */
/*		n++; */
/*	} */
/* } */

void send_counters(void)
{
	uint8_t i, us_val_idx;
	uint32_t u32AttributeId;
	struct TAdpMacGetConfirm adpMacGetConfirm;
	uint8_t puc_val_buff[VALUES_BUFFER_SIZE];

	for (i = 0, us_val_idx = 0; i < NUM_OBJECTS_TO_SET; i++) {
		u32AttributeId = ic90_to_g3_mac(objects_to_set[i].attr);
		if (u32AttributeId) {
			AdpMacGetRequestSync(u32AttributeId, 0, &adpMacGetConfirm);
			if (adpMacGetConfirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) {
				/* Counters will be sent as unsigned integers. Unmatching length will be treated as octet string */
				switch (adpMacGetConfirm.m_u8AttributeLength) {
				case 1:
					puc_val_buff[us_val_idx++] = DT_UNSIGNED;
					break;

				case 2:
					puc_val_buff[us_val_idx++] = DT_LONG_UNSIGNED;
					break;

				case 4:
					puc_val_buff[us_val_idx++] = DT_DOUBLE_LONG_UNSIGNED;
					break;

				default:
					puc_val_buff[us_val_idx++] = DT_OCTET_STRING;
				}
				memcpy(&puc_val_buff[us_val_idx], adpMacGetConfirm.m_au8AttributeValue, adpMacGetConfirm.m_u8AttributeLength);
				us_val_idx += adpMacGetConfirm.m_u8AttributeLength;
			}
		}
	}

	if (DLMS_FORMAT_ERROR == dlms_cli_list_set(us_meter_addr, uc_assoc_idx,
			objects_to_set, NUM_OBJECTS_TO_SET, NULL, puc_val_buff, us_val_idx)) {
		uc_current_req_result = DLMS_FORMAT_ERROR;
	}
}

/**
 * \brief Update internal counters.
 *
 */
void dlms_mgmt_update_1ms(void)
{
	if (sul_data_timeout_timer) {
		sul_data_timeout_timer--;
	}
}

static bool _dlms_app_get_alg_passwd(uint8_t *puc_passwd)
{
	UNUSED(puc_passwd);
	/*	uint8_t puc_serial[14]; */
	/*	uint8_t i; */
	/*	uint8_t uc_num, uc_num1; */
	/*  */
	/*	*puc_serial = 'A'; */
	/*	*(puc_serial + 1) = 'T'; */
	/*	*(puc_serial + 2) = 'M'; */
	/*	/ * convert hex to ascii * / */
	/*	for (i = 1; i < 6; i++) { */
	/*		uc_num  = ((sx_cycles_stat[us_stats_idx].puc_extended_address[i] & 0xF0) >> 4); */
	/*		uc_num1 = (sx_cycles_stat[us_stats_idx].puc_extended_address[i] & 0x0F); */
	/*		puc_serial[2 * (i) + 1] = _dlms_app_hex_2_string(uc_num); */
	/*		puc_serial[2 * (i) + 2] = _dlms_app_hex_2_string(uc_num1); */
	/*	} */
	/*  */
	/*	*(puc_serial + 13) = 0x00; */
	/*  */
	/*	memcpy(&puc_passwd[0], &puc_serial[0], 3); */
	/*	memcpy(&puc_passwd[3], &puc_serial[8], 5); */

	return true;
}

/**
 * \brief Sending data from DLMS Server lib to 4-32 connection
 *
 * \param us_dst_wport     Destination Wrapper Port
 * \param us_src_wport     Source Wrapper Port
 * \param px_buff          Pointer to the data buffer
 * \param us_buff_len      Length of the data
 */
static void _dlms_app_request_cb(uint16_t us_short_address, uint16_t us_dst_wport, uint16_t us_src_wport, uint8_t *puc_buff, uint16_t us_buff_len)
{
	UNUSED(us_short_address);
	UNUSED(us_dst_wport);
	UNUSED(us_src_wport);
	memcpy(app_ptr_tx_dlms_msg->buf, puc_buff, us_buff_len);
	app_ptr_tx_dlms_msg->length = us_buff_len;
	app_ptr_tx_dlms_msg->todo = 1;
	uc_current_req_result = DLMS_WAITING;
	sul_data_timeout_timer = DLMS_TIME_WAIT_RESPONSE;
	hdlc_dlms_to_serial();
}

/**
 * \brief DLMS library request response callback
 *
 */
static void _dlms_app_response_cb(uint16_t us_short_address, uint16_t uc_dst, uint16_t uc_src, dlms_cli_result_t x_result, bool b_last_frag)
{
	UNUSED(us_short_address);
	(void)uc_dst;
	(void)uc_src;

	if (b_last_frag) {
		uc_current_req_result = x_result;
	}
}

/**
 * \brief Decode dlms management messages
 */
bool dlms_mgmt_decode(uint8_t *ptr_buff, uint8_t len)
{
	dlms_cli_data_ind(us_meter_addr, px_assoc_conf[uc_assoc_idx].us_destination, px_assoc_conf[uc_assoc_idx].us_source, ptr_buff, len);
	return true;
}

/**
 * \brief Request required parameters for MODEM through DLMS
 *
 */
void dlms_mgmt_process(void)
{
	switch (uc_state) {
	case STATE_ASSOC_REQ:
		/* Reset object to request */
		suc_current_object_idx = 0;

		if (px_assoc_conf[uc_assoc_idx].auth == COSEM_LOWEST_LEVEL_SEC) {
			if (DLMS_AA_IDX_ERROR == dlms_cli_aarq_request(us_meter_addr, uc_assoc_idx, NULL)) {
				uc_current_req_result = DLMS_AA_IDX_ERROR;
			}
		} else {
			switch (px_assoc_conf[uc_assoc_idx].pwd_type) {
			case LLS_ALG_1_PWD:
				if (_dlms_app_get_alg_passwd(px_assoc_conf[uc_assoc_idx].password)) {
					if (DLMS_AA_IDX_ERROR == dlms_cli_aarq_request(us_meter_addr, uc_assoc_idx, px_assoc_conf[uc_assoc_idx].password)) {
						uc_current_req_result = DLMS_AA_IDX_ERROR;
					}
				} else {
					uc_current_req_result = DLMS_DISCONNECTED;
				}

				break;

			default:
			case LLS_FIXED_PWD:
				if (DLMS_AA_IDX_ERROR == dlms_cli_aarq_request(us_meter_addr, uc_assoc_idx, px_assoc_conf[uc_assoc_idx].password)) {
					uc_current_req_result = DLMS_AA_IDX_ERROR;
				}

				break;
			}
		}

		LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_WAIT_ASSOC_RESP\r\n"));
		uc_state = STATE_WAIT_ASSOC_RESP;
		break;

	case STATE_WAIT_ASSOC_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_SUCCESS:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_REQ_OBJECT\r\n"));
			uc_state = STATE_REQ_OBJECT;
			uc_current_req_result = DLMS_WAITING;
			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_ASSOC_REQ\r\n"));
				uc_state = STATE_ASSOC_REQ;
				break;
			}

			break;

		default:
			uc_state = STATE_ASSOC_REQ;
			break;
		}

		break;

	case STATE_REQ_OBJECT:
		if (objects_to_get[suc_current_object_idx].class_id == 7) {
			access_selector_t x_sel_access;
			x_sel_access.selector = SEL_IC07;
			x_sel_access.ic07_range.selector_ic07 = SEL_IC07_RANGE;
			x_sel_access.ic07_range.restricting_object.attr = 2;
			x_sel_access.ic07_range.restricting_object.obis_code[0] = 0;
			x_sel_access.ic07_range.restricting_object.obis_code[1] = 0;
			x_sel_access.ic07_range.restricting_object.obis_code[2] = 1;
			x_sel_access.ic07_range.restricting_object.obis_code[3] = 0;
			x_sel_access.ic07_range.restricting_object.obis_code[4] = 0;
			x_sel_access.ic07_range.restricting_object.obis_code[5] = 255;
			x_sel_access.ic07_range.restricting_object.class_id = 8;
			x_sel_access.ic07_range.from.type = DT_OCTET_STRING;
			x_sel_access.ic07_range.from.length = SIZE_DATE_TIME;
			x_sel_access.ic07_range.to.type = DT_OCTET_STRING;
			x_sel_access.ic07_range.to.length = SIZE_DATE_TIME;
			memcpy(x_sel_access.ic07_range.from.value, spuc_date_time_start, SIZE_DATE_TIME);
			memcpy(x_sel_access.ic07_range.to.value, spuc_date_time_end, SIZE_DATE_TIME);
			if (DLMS_FORMAT_ERROR == dlms_cli_obj_request(us_meter_addr, uc_assoc_idx,
					objects_to_get[suc_current_object_idx], &x_sel_access)) {
				uc_current_req_result = DLMS_FORMAT_ERROR;
			}
		} else {
			uc_num_req_objects = MIN(OBJECTS_PER_REQUEST, NUM_OBJECTS_TO_GET - suc_current_object_idx);
			uc_num_req_objects = MIN(uc_num_req_objects, MAX_OBJECTS_PER_REQUEST);
			if (uc_num_req_objects > 1) {
				/* Create GET request WITH-LIST */
				if (DLMS_FORMAT_ERROR == dlms_cli_list_request(us_meter_addr, uc_assoc_idx,
						(dlms_object_t *)&objects_to_get[suc_current_object_idx], uc_num_req_objects, NULL)) {
					uc_current_req_result = DLMS_FORMAT_ERROR;
				}
			} else {
				/* Create GET request NORMAL (single object) */
				if (DLMS_FORMAT_ERROR == dlms_cli_obj_request(us_meter_addr, uc_assoc_idx,
						objects_to_get[suc_current_object_idx], NULL)) {
					uc_current_req_result = DLMS_FORMAT_ERROR;
				}
			}
		}

		LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_WAIT_REQ_RESP\r\n"));
		uc_state = STATE_WAIT_REQ_RESP;
		break;

	case STATE_WAIT_REQ_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_SUCCESS:
			suc_current_object_idx += uc_num_req_objects;
			if (suc_current_object_idx < NUM_OBJECTS_TO_GET) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_REQ_OBJECT\r\n"));
				uc_state = STATE_REQ_OBJECT;
			} else {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_RELEASE_REQ\r\n"));
				uc_state = STATE_RELEASE_REQ;
			}

			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_ASSOC_REQ\r\n"));
				uc_state = STATE_ASSOC_REQ;
				break;
			}

			break;

		default:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_ASSOC_REQ\r\n"));
			uc_state = STATE_ASSOC_REQ;
			break;
		}

		break;

	case STATE_RELEASE_REQ:
		dlms_cli_rlrq_request(us_meter_addr, uc_assoc_idx, RLRQ_URGENT);
		LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_WAIT_RELEASE_RESP\r\n"));
		uc_state = STATE_WAIT_RELEASE_RESP;
		break;

	case STATE_WAIT_RELEASE_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_RELEASED:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_FINISHED\r\n"));
			uc_state = STATE_FINISHED;
			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_FINISHED\r\n"));
				uc_state = STATE_FINISHED;
				break;
			}

			break;

		default:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: STATE_FINISHED\r\n"));
			uc_state = STATE_FINISHED;
			break;
		}

		break;

	case STATE_FINISHED:
		LOG_APP_DEBUG(("[DLMS_APP] dlms_mgmt_process: Finished\r\n"));
		g_bHasMeterId = true;
		break;

	default:
		break;
	}

	/* DLMS Client process */
	dlms_cli_process();
}

void dlms_mgmt_init()
{
	uc_state = STATE_ASSOC_REQ;
	sul_data_timeout_timer = 0;

	/* DLMS Client lib init */
	dlms_cli_init(px_assoc_conf, DLMS_MAX_ASSOC, spx_node_info, DLMS_MAX_DEV_NUM, _dlms_app_request_cb, _dlms_app_response_cb);
	dlms_cli_conf_obis(1, 0, 99, 1, 0, 255, 7, obis_1_0_99_1_0_255_cli_cb);
	dlms_cli_conf_obis(0, 0, 1, 0, 0, 255, 8, obis_0_0_1_0_0_255_cli_cb);
	/* G3 Profile specific */
	dlms_cli_conf_obis(0, 0, 29, 0, 0, 255, 91, obis_0_0_29_0_0_255_cli_cb);
	dlms_cli_conf_obis(0, 0, 29, 1, 0, 255, 91, obis_0_0_29_1_0_255_cli_cb);
	dlms_cli_conf_obis(0, 0, 29, 2, 0, 255, 92, obis_0_0_29_2_0_255_cli_cb);
	dlms_cli_conf_obis(0, 0, 29, 2, 2, 128, 1, obis_0_0_29_2_2_128_cli_cb);
}

#endif /* #ifdef DLMS_MGMT */
