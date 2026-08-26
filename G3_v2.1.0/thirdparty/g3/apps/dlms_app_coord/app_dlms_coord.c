/**
 * \file
 *
 * \brief DLMS_APP : DLMS example application for ATMEL G3 Coordinator
 *
 * Copyright (c) 2017 Atmel Corporation. All rights reserved.
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
#include "compiler.h"
#include "AdpApi.h"
#include "drivers/g3/network_adapter_g3.h"
#include "app_dlms_coord.h"

#include "app_adp_mng.h"
#include "g3_app_config.h"
#include "oss_if.h"
#include "async_ping.h"
#include "ipv6_mng.h"
#include "app_dispatcher.h"
#include "dlms_cli_lib.h"
#include "dlms_cli_data.h"

#define ADP_PATH_METRIC_TYPE       0

#ifdef DLMS_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

#ifdef DLMS_REPORT_CONSOLE
#       define LOG_APP_REPORT(a)   printf a
#else
#       define LOG_APP_REPORT(a)   (void)0
#endif

#define NUM_SINGLE_OBJECTS        32
#define NUM_TABLE_OBJECTS          4
#define NUM_OBJECTS (NUM_SINGLE_OBJECTS + NUM_TABLE_OBJECTS)
#define NUM_OBJECTS_PER_REQUEST   4
/* All objects must be configured here */
static const dlms_object_t object_list[NUM_OBJECTS] = {
	/* Single small objects to be requested in a list */
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_DATA_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_DATA_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_CMD_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_CMD_PACKET_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_CSMA_FAIL_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_CSMA_NO_ACK_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_BAD_CRC_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_TX_DATA_BROADCAST_COUNT},
	{{0, 0, 29, 0, 0, 255}, 90, IC90_MAC_RX_DATA_BROADCAST_COUNT},
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
	/* Table & big objects */
	{{0, 0, 1, 0, 0, 255}, 8, IC08_TIME},
	{{1, 0, 99, 1, 0, 255}, 7, IC07_BUFFER},
	{{0, 0, 29, 1, 0, 255}, 91, IC91_MAC_NEIGHBOUR_TABLE},
	{{0, 0, 29, 2, 0, 255}, 92, IC92_ADP_ROUTING_TABLE}
};

static uint8_t spuc_date_time_start[] = {0x07, 0xDE, 0x05, 0x15, 0x01, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80};
static uint8_t spuc_date_time_end[] = {0x07, 0xDE, 0x05, 0x17, 0x01, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80};

/** Association configuration: in this example, four possible associations: MGMT(1), READ(2), FW(3) and PUBLIC(16) */
#define ASSOC_MGMT_IDX             0
#define ASSOC_READ_IDX             1
#define ASSOC_FW_IDX               2
#define ASSOC_PUBLIC_IDX           3

static assoc_conf_t px_assoc_conf[DLMS_MAX_ASSOC] = {
	{0x0001, 0x0001, LLS_FIXED_PWD, "00000002", COSEM_LOW_LEVEL_SEC},    /*  MGMT  */
	{0x0001, 0x0002, LLS_FIXED_PWD, "00000001", COSEM_LOW_LEVEL_SEC},    /*  READ  */
	{0x0001, 0x0003, LLS_FIXED_PWD, "00000003", COSEM_LOW_LEVEL_SEC},    /*   FW   */
	{0x0001, 0x0010, LLS_FIXED_PWD, "--------", COSEM_LOWEST_LEVEL_SEC}  /* PUBLIC */
};

/** Node info */
static node_info_t spx_node_info[DLMS_MAX_DEV_NUM];

typedef struct x_cycles_stat {
	uint8_t puc_extended_address[EXT_ADDR_LEN];
	uint32_t ul_total;
	uint32_t ul_success;
	uint32_t ul_errors;
	uint32_t ul_current_cycle_time;
	uint32_t ul_mean_cycle_time;
	bool b_last_cycle_success : 1;
	bool b_present_cycle : 1;
	/* cppcheck-suppress unusedStructMember */
	uint8_t uc_reserved : 6; /* available bits */
} x_cycles_stat_t;

enum {
	STATE_IDLE,              /* Initial cycle wait start */
	STATE_PATH_LIST_REQ,     /* Path request */
	STATE_PATH_WAIT_CFM,     /* Path request wait confirm */
	STATE_START_NEW_CYCLE,   /* Start new cycle */
	STATE_CYCLE_NEXT_NODE,   /* Association request */
	STATE_WAIT_ASSOC_RESP,   /* Wait association response */
	STATE_REQ_OBJECT,        /* Object request */
	STATE_WAIT_REQ_RESP,     /* Wait object response */
	STATE_RELEASE_REQUEST,   /* End cycle. Request release association */
	STATE_WAIT_RELEASE_RESP, /* Wait association response */
}
uc_state_cycles;

typedef struct x_addr_list {
	uint16_t short_addr;
	uint16_t stats_idx;
	bool is_connected;
} x_addr_list_t;

/* Current status variables */
static uint16_t sus_num_nodes_ever_connected;
static dlms_cli_result_t uc_current_req_result;
static uint16_t sus_current_node_idx;
static uint8_t suc_current_object_idx;
static uint8_t sus_req_objects;
static x_addr_list_t spx_current_addr_list[DLMS_MAX_DEV_NUM];
static uint16_t sus_current_cycle_num_nodes;

static x_cycles_stat_t sx_cycles_stat[DLMS_MAX_DEV_NUM];

static uint32_t sul_dlms_start_timer;
static uint32_t sul_next_cycle_timer;
static uint32_t sul_data_timeout_timer;
#ifdef DLMS_APP_ENABLE_PATH_REQ
static uint16_t us_num_path_nodes;
static struct TAdpPathDiscoveryConfirm sx_path_nodes[MAX_LBDS];
#endif

static uint32_t ul_start_time_cycle, ul_start_time_node_cycle, ul_start_time_object;
static uint32_t ul_absolute_time = 0;

x_node_list_t px_node_list[MAX_LBDS];
uint16_t sus_num_reg_nodes;
uint32_t sul_cycle_counter;

/**
 * \brief Display SW version in console
 */
static void _show_version( void )
{
	struct TAdpGetConfirm getConfirm;
	struct TAdpMacGetConfirm x_pib_confirm;

#if defined (CONF_BAND_CENELEC_A)
	LOG_APP_REPORT(("G3 Band: CENELEC-A\r\n"));
#elif defined (CONF_BAND_CENELEC_B)
	LOG_APP_REPORT(("G3 Band: CENELEC-B\r\n"));
#elif defined (CONF_BAND_FCC)
	LOG_APP_REPORT(("G3 Band: FCC\r\n"));
#elif defined (CONF_BAND_ARIB)
	LOG_APP_REPORT(("G3 Band: ARIB\r\n"));
#else
	LOG_APP_REPORT(("G3 Band: CENELEC-A\r\n"));
#endif

	AdpGetRequestSync(ADP_IB_SOFT_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		LOG_APP_REPORT(("G3 stack version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]));
	}

	AdpGetRequestSync(ADP_IB_MANUF_ADP_INTERNAL_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		LOG_APP_REPORT(("ADP version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_REPORT(("MAC version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION_RF, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_REPORT(("MAC RF version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_RT_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		LOG_APP_REPORT(("MAC_RT version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]));
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_PHY_PARAM, MAC_WRP_PHY_PARAM_VERSION, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 4)) {
		LOG_APP_REPORT(("PHY version: %02x.%02x.%02x.%02x\r\n",
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[0]));
	}

	return;
}

#ifdef DLMS_REPORT_CONSOLE

static char c_log_full_cycles_time_query[100];
static char c_log_cycles_step_query[600];

static bool _is_valid_address(uint8_t *addr)
{
	uint8_t uc_idx;
	bool valid = false;

	for (uc_idx = 0; uc_idx < EXT_ADDR_LEN; uc_idx++) {
		if ((*addr) != 0) {
			valid = true;
			break;
		}

		addr++;
	}

	return valid;
}

static void _log_full_cycles_time(uint32_t ul_cycleTime)
{
	uint8_t puc_ext_addr_ascii[24];
	uint16_t us_node_idx = 0;
	uint16_t us_processed_nodes = 0;

	sprintf(c_log_full_cycles_time_query, "cycle time cost: %u ms.", ul_cycleTime);

	LOG_APP_REPORT(("\r\n[CYCLE %4u] [SUM] ------------------------------------------------------------------------------------\r\n", sul_cycle_counter));
	LOG_APP_REPORT(("[CYCLE %4u] [SUM] Cycle summary: %s\r\n", sul_cycle_counter, c_log_full_cycles_time_query));
	LOG_APP_REPORT(("[CYCLE %4u] [SUM] ------------------------------------------------------------------------------------\r\n", sul_cycle_counter));

	/* Summary nodes  */
	while (us_node_idx < DLMS_MAX_DEV_NUM) {
		uint32_t ul_availability;

		/* Ever connected? */
		if (_is_valid_address(sx_cycles_stat[us_node_idx].puc_extended_address)) {
			if (sx_cycles_stat[us_node_idx].ul_success + sx_cycles_stat[us_node_idx].ul_errors) {
				ul_availability
					= (((sx_cycles_stat[us_node_idx].ul_success *
						100) / (sx_cycles_stat[us_node_idx].ul_success + sx_cycles_stat[us_node_idx].ul_errors) * 100) / 100);
			} else {
				ul_availability = 0;
			}

			sprintf((char *)puc_ext_addr_ascii, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
					sx_cycles_stat[us_node_idx].puc_extended_address[0],
					sx_cycles_stat[us_node_idx].puc_extended_address[1],
					sx_cycles_stat[us_node_idx].puc_extended_address[2],
					sx_cycles_stat[us_node_idx].puc_extended_address[3],
					sx_cycles_stat[us_node_idx].puc_extended_address[4],
					sx_cycles_stat[us_node_idx].puc_extended_address[5],
					sx_cycles_stat[us_node_idx].puc_extended_address[6],
					sx_cycles_stat[us_node_idx].puc_extended_address[7]);

			if (sx_cycles_stat[us_node_idx].b_last_cycle_success) {
				sprintf(c_log_cycles_step_query, "%s  Total: %5u  Ok: %5u  Fail: %5u  Rate: %3u%%  Cycle:%8u ms. Mean:%8u ms.",
						puc_ext_addr_ascii,
						sx_cycles_stat[us_node_idx].ul_total,
						sx_cycles_stat[us_node_idx].ul_success,
						sx_cycles_stat[us_node_idx].ul_errors,
						ul_availability,
						sx_cycles_stat[us_node_idx].ul_current_cycle_time,
						sx_cycles_stat[us_node_idx].ul_mean_cycle_time);
			} else if (sx_cycles_stat[us_node_idx].b_present_cycle) {
				sprintf(c_log_cycles_step_query, "%s  Total: %5u  Ok: %5u  Fail: %5u  Rate: %3u%%  Cycle:     FAILED. Mean:%8u ms.",
						puc_ext_addr_ascii,
						sx_cycles_stat[us_node_idx].ul_total,
						sx_cycles_stat[us_node_idx].ul_success,
						sx_cycles_stat[us_node_idx].ul_errors,
						ul_availability,
						sx_cycles_stat[us_node_idx].ul_mean_cycle_time);
			} else {
				sprintf(c_log_cycles_step_query, "%s  Total: %5u  Ok: %5u  Fail: %5u  Rate: %3u%%  Cycle: NOT CYCLED. Mean:%8u ms.",
						puc_ext_addr_ascii,
						sx_cycles_stat[us_node_idx].ul_total,
						sx_cycles_stat[us_node_idx].ul_success,
						sx_cycles_stat[us_node_idx].ul_errors,
						ul_availability,
						sx_cycles_stat[us_node_idx].ul_mean_cycle_time);
			}

			sx_cycles_stat[us_node_idx].b_last_cycle_success = false; /* reset flag */

			LOG_APP_REPORT(("[CYCLE %4u] [SUM] %s\r\n", sul_cycle_counter, c_log_cycles_step_query));
			us_processed_nodes++;
			if (us_processed_nodes >= sus_num_nodes_ever_connected) {
				break;
			}
		}

		us_node_idx++;
	}

	LOG_APP_REPORT(("[CYCLE %4u] [SUM] ------------------------------------------------------------------------------------\r\n\r\n", sul_cycle_counter));
}

static void _print_request_step(uint16_t us_node, uint8_t uc_object_idx)
{
	ul_start_time_object = ul_absolute_time;

	LOG_APP_REPORT(("[CYCLE %4u] REQUEST: Short address: %04hu object: %d\r\n",
			sul_cycle_counter,
			px_node_list[us_node].us_short_address,
			uc_object_idx));
}

static void _print_result_step(uint8_t uc_error)
{
	uint8_t puc_msg[11] = {0};

	switch (uc_state_cycles) {
	case STATE_CYCLE_NEXT_NODE:
		strncpy((char *)puc_msg, "SEND_AARQ ", 10);
		break;

	case STATE_WAIT_ASSOC_RESP:
		strncpy((char *)puc_msg, "AARE_RESP ", 10);
		break;

	case STATE_REQ_OBJECT:
		strncpy((char *)puc_msg, "REQ_OBJ   ", 10);
		break;

	case STATE_WAIT_REQ_RESP:
		strncpy((char *)puc_msg, "OBJ_RESP  ", 10);
		break;

	case STATE_RELEASE_REQUEST:
		strncpy((char *)puc_msg, "SEND_RLRQ ", 10);
		break;

	case STATE_WAIT_RELEASE_RESP:
		strncpy((char *)puc_msg, "RLRE_RESP ", 10);
		break;

	default:
		strncpy((char *)puc_msg, "BAD_STATE ", 10);
		break;
	}

	if (uc_error == DLMS_SUCCESS) {
		LOG_APP_REPORT(("[CYCLE %4u] %s: Time %8u ms. OK\r\n", sul_cycle_counter, puc_msg, ul_absolute_time - ul_start_time_object));
	} else {
		LOG_APP_REPORT(("[CYCLE %4u] %s: Time %8u ms. ERROR=%hhu\r\n", sul_cycle_counter, puc_msg, ul_absolute_time - ul_start_time_object, uc_error));
	}
}

#endif

/**
 * \brief Update internal counters.
 *
 */
void dlms_app_update_1ms(void)
{
	if (sul_dlms_start_timer) {
		sul_dlms_start_timer--;
	}

	if (sul_next_cycle_timer) {
		sul_next_cycle_timer--;
	}

	if (sul_data_timeout_timer) {
		sul_data_timeout_timer--;
	}

	ul_absolute_time++;
}

/**
 * Convert functions
 **/
static uint8_t _dlms_app_hex_2_string(uint8_t num)
{
	if (num > 9) {
		return (num + 0x37);
	} else {
		return (num + 0x30);
	}
}

static bool _dlms_app_get_alg_passwd(uint16_t us_stats_idx, uint8_t *puc_passwd)
{
	uint8_t puc_serial[14];
	uint8_t i;
	uint8_t uc_num, uc_num1;

	*puc_serial = 'A';
	*(puc_serial + 1) = 'T';
	*(puc_serial + 2) = 'M';
	/* convert hex to ascii */
	for (i = 1; i < 6; i++) {
		uc_num  = ((sx_cycles_stat[us_stats_idx].puc_extended_address[i] & 0xF0) >> 4);
		uc_num1 = (sx_cycles_stat[us_stats_idx].puc_extended_address[i] & 0x0F);
		puc_serial[2 * (i) + 1] = _dlms_app_hex_2_string(uc_num);
		puc_serial[2 * (i) + 2] = _dlms_app_hex_2_string(uc_num1);
	}

	*(puc_serial + 13) = 0x00;

	memcpy(&puc_passwd[0], &puc_serial[0], 3);
	memcpy(&puc_passwd[3], &puc_serial[8], 5);

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
	size_t x_transmitted_bytes;
	uint8_t *puc_tx_buff;
	uint16_t us_tx_buf_len = 0;

	puc_tx_buff = dispatcher_get_tx_buff();

	us_tx_buf_len = dlms_cli_add_wrapper_header(puc_tx_buff, us_dst_wport, us_src_wport, puc_buff, us_buff_len);

	dispatcher_send(puc_tx_buff, us_tx_buf_len, &x_transmitted_bytes, us_short_address);

	if (x_transmitted_bytes == us_tx_buf_len) {
		uc_current_req_result = DLMS_WAITING;
		sul_data_timeout_timer = DLMS_TIME_WAIT_RESPONSE;
	} else {
		sul_data_timeout_timer = 0;
		uc_current_req_result = DLMS_TX_ERROR;
	}
}

/**
 * \brief DLMS library request response callback
 *
 */
static void _dlms_app_response_cb(uint16_t us_short_address, uint16_t uc_dst, uint16_t uc_src, dlms_cli_result_t x_result, bool b_last_frag)
{
	(void)uc_dst;
	(void)uc_src;

	if (us_short_address == spx_current_addr_list[sus_current_node_idx].short_addr) {
		if (b_last_frag) {
			uc_current_req_result = x_result;
		}
	}
}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void dlms_app_process(void)
{
	uint16_t us_node_idx, us_stats_idx, us_processed_nodes = 0;
	uint8_t uc_assoc_idx = ASSOC_MGMT_IDX;

	switch (uc_state_cycles) {
	case STATE_IDLE:
		if (!sul_dlms_start_timer && sus_num_reg_nodes) {
#ifdef DLMS_APP_ENABLE_PATH_REQ
			/* Get Path Nodes Info */
			us_num_path_nodes = 0;
			LOG_APP_DEBUG(("\r\n[DLMS_APP] dlms_app_process: STATE_PATH_LIST_REQ\r\n"));
			uc_state_cycles = STATE_PATH_LIST_REQ;
#else
			/* Start Cycles */
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: STATE_START_NEW_CYCLE\r\n"));
			uc_state_cycles = STATE_START_NEW_CYCLE;
			sul_next_cycle_timer = 0;
#endif
		}

		break;

#ifdef DLMS_APP_ENABLE_PATH_REQ
	case STATE_PATH_LIST_REQ:
		if (sus_num_reg_nodes == us_num_path_nodes) {
			/* Start Cycles */
			uc_state_cycles = STATE_START_NEW_CYCLE;
		} else {
			AdpPathDiscoveryRequest(px_node_list[us_num_path_nodes].us_short_address, ADP_PATH_METRIC_TYPE);
			uc_state_cycles = STATE_PATH_WAIT_CFM;
		}
		break;
#endif

	case STATE_START_NEW_CYCLE:
		if (!sul_next_cycle_timer && sus_num_reg_nodes) {
			/* Reset nodes in cycle list */
			for (us_node_idx = 0; us_node_idx < DLMS_MAX_DEV_NUM; us_node_idx++) {
				spx_current_addr_list[us_node_idx].stats_idx = 0xFFFF; /* invalid */
				spx_current_addr_list[us_node_idx].is_connected = false;
				spx_current_addr_list[us_node_idx].short_addr = LBS_INVALID_SHORT_ADDRESS;
				sx_cycles_stat[us_node_idx].b_present_cycle = false;
			}

			/* Take a picture of connected nodes at this moment */
			for (us_node_idx = 0; us_processed_nodes < sus_num_reg_nodes; us_node_idx++) {
				if (px_node_list[us_node_idx].us_short_address != LBS_INVALID_SHORT_ADDRESS) {
					/* Add node to cycle list */
					/* Look for node statistics index */
					for (us_stats_idx = 0; us_stats_idx < sus_num_nodes_ever_connected; us_stats_idx++) {
						if (!memcmp(px_node_list[us_node_idx].puc_extended_address,
								sx_cycles_stat[us_stats_idx].puc_extended_address, EXT_ADDR_LEN)) {
							break;
						}
					}

					/* Check if statistics index is correct */
					if (us_stats_idx != sus_num_nodes_ever_connected) {
						/* Add node to nodes in cycle list */
						LOG_APP_REPORT(("[CYCLE %4u] dlms_app_process: Position: %d -> [%hu]\r\n", sul_cycle_counter, us_node_idx,
								px_node_list[us_node_idx].us_short_address));
						spx_current_addr_list[us_processed_nodes].stats_idx = us_stats_idx;
						spx_current_addr_list[us_processed_nodes].short_addr = px_node_list[us_node_idx].us_short_address;
						spx_current_addr_list[us_processed_nodes].is_connected = true;
						sx_cycles_stat[us_stats_idx].b_present_cycle = true;
						us_processed_nodes++;
						if (us_processed_nodes == DLMS_MAX_DEV_NUM) {
							break;
						}
					}
				}

				if (us_node_idx >= DLMS_MAX_DEV_NUM) {
					break;
				}
			}

			if (us_processed_nodes) {
				LOG_APP_REPORT(("[CYCLE %4u] dlms_app_process: %d nodes registered.\r\n", sul_cycle_counter, sus_num_reg_nodes));
				LOG_APP_REPORT(("[CYCLE %4u] **************************************************\r\n", sul_cycle_counter));
				LOG_APP_REPORT(("[CYCLE %4u] ***************** START CYCLE %u ******************\r\n", sul_cycle_counter, sul_cycle_counter));
				LOG_APP_REPORT(("[CYCLE %4u] *************** NODES IN CYCLE %4hu **************\r\n", sul_cycle_counter, us_processed_nodes));
				LOG_APP_REPORT(("[CYCLE %4u] **************************************************\r\n", sul_cycle_counter));
#ifdef DLMS_DEBUG_CONSOLE
				ul_start_time_cycle = ul_absolute_time;
#endif

				sus_current_cycle_num_nodes = us_processed_nodes;
				sus_current_node_idx = 0xFFFF;
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: STATE_CYCLE_NEXT_NODE\r\n"));
				uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			} else {
				sul_next_cycle_timer = DLMS_TIME_BETWEEEN_CYCLES;
			}
		}

		break;

	case STATE_CYCLE_NEXT_NODE:
		if (sus_current_node_idx != 0xFFFF) {
			/* UPDATE PREVIOUS NODE MEAN CYCLE TIME */
			uint32_t ul_last_cycle_time = ul_absolute_time - ul_start_time_node_cycle;
			us_node_idx = spx_current_addr_list[sus_current_node_idx].stats_idx;
			sx_cycles_stat[us_node_idx].ul_total++;
			/* Update only if cycle has been successful */
			if (sx_cycles_stat[us_node_idx].b_last_cycle_success) {
				sx_cycles_stat[us_node_idx].ul_success++;
				sx_cycles_stat[us_node_idx].ul_current_cycle_time = ul_last_cycle_time;
				sx_cycles_stat[us_node_idx].ul_mean_cycle_time
					= (sx_cycles_stat[us_node_idx].ul_mean_cycle_time *
						(sx_cycles_stat[us_node_idx].ul_success - 1) + ul_last_cycle_time) / sx_cycles_stat[us_node_idx].ul_success;
			} else {
				sx_cycles_stat[us_node_idx].ul_errors++;
			}
		}

		sus_current_node_idx++;
		ul_start_time_node_cycle = ul_absolute_time;
		if ((sus_current_node_idx < sus_current_cycle_num_nodes) && (sus_current_node_idx != DLMS_MAX_DEV_NUM)) {
			/* Reset object to request */
			suc_current_object_idx = 0;

			/* Check if node is disconnected */
			if (!spx_current_addr_list[sus_current_node_idx].is_connected) {
#ifdef DLMS_REPORT_CONSOLE
				_print_result_step(DLMS_DISCONNECTED);
#endif
				break;
			}

			if (px_assoc_conf[uc_assoc_idx].auth == COSEM_LOWEST_LEVEL_SEC) {
				if (DLMS_AA_IDX_ERROR == dlms_cli_aarq_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx, NULL)) {
					uc_current_req_result = DLMS_AA_IDX_ERROR;
				}
			} else {
				switch (px_assoc_conf[uc_assoc_idx].pwd_type) {
				case LLS_ALG_1_PWD:
					if (_dlms_app_get_alg_passwd(spx_current_addr_list[sus_current_node_idx].stats_idx,
							px_assoc_conf[uc_assoc_idx].password)) {
						if (DLMS_AA_IDX_ERROR ==
								dlms_cli_aarq_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx,
								px_assoc_conf[uc_assoc_idx].password)) {
							uc_current_req_result = DLMS_AA_IDX_ERROR;
						}
					} else {
						uc_current_req_result = DLMS_DISCONNECTED;
					}

					break;

				default:
				case LLS_FIXED_PWD:
					if (DLMS_AA_IDX_ERROR ==
							dlms_cli_aarq_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx,
							px_assoc_conf[uc_assoc_idx].password)) {
						uc_current_req_result = DLMS_AA_IDX_ERROR;
					}

					break;
				}
			}

			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_WAIT_ASSOC_RESP\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_WAIT_ASSOC_RESP;
		} else {
			/* Last node has just been cycled */
#ifdef DLMS_REPORT_CONSOLE
			_log_full_cycles_time(ul_absolute_time - ul_start_time_cycle);
#endif
			sul_cycle_counter++;
#ifdef DLMS_APP_ENABLE_PATH_REQ
			/* Get Path Nodes Info before each complete cycle */
			us_num_path_nodes = 0;
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_PATH_LIST_REQ\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_PATH_LIST_REQ;
#else
			if (sus_num_reg_nodes) {
				/* Next Cycles */
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_START_NEW_CYCLE\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_START_NEW_CYCLE;
				sul_next_cycle_timer = DLMS_TIME_BETWEEEN_CYCLES;
			} else {
				/* No nodes, wait DLMS_TIME_WAITING_IDLE to start another cycle */
				uc_state_cycles = STATE_IDLE;

				/* Init local vars */
				sul_dlms_start_timer = DLMS_TIME_WAITING_IDLE;
				sul_next_cycle_timer = 0;
				sul_data_timeout_timer = 0;
			}
#endif
		}

		break;

	case STATE_WAIT_ASSOC_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_SUCCESS:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_REQ_OBJECT\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_REQ_OBJECT;
			uc_current_req_result = DLMS_WAITING;
			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
#ifdef DLMS_REPORT_CONSOLE
				_print_result_step(DLMS_TIMEOUT);
#endif
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_CYCLE_NEXT_NODE;
				break;
			}

			break;

		default:
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(uc_current_req_result);
#endif
			uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			break;
		}

		break;

	case STATE_REQ_OBJECT:
#ifdef DLMS_REPORT_CONSOLE
		_print_request_step(sus_current_node_idx, suc_current_object_idx);
#endif
		/* Check if node is disconnected */
		if (!spx_current_addr_list[sus_current_node_idx].is_connected) {
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(DLMS_DISCONNECTED);
#endif
			break;
		}

		if (object_list[suc_current_object_idx].class_id == 7) {
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
			sus_req_objects = 1;
			if (DLMS_FORMAT_ERROR == dlms_cli_obj_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx,
					object_list[suc_current_object_idx], &x_sel_access)) {
				uc_current_req_result = DLMS_FORMAT_ERROR;
			}
		} else {
			printf("\r\nPending %hhu objects\r\n", sus_req_objects);
			if (suc_current_object_idx < NUM_SINGLE_OBJECTS) {
				/* Several single objects will be requested */
				sus_req_objects = NUM_SINGLE_OBJECTS - suc_current_object_idx;

				if (sus_req_objects > NUM_OBJECTS_PER_REQUEST) {
					sus_req_objects = NUM_OBJECTS_PER_REQUEST;
				}
			} else {
				/* One table object will be requested */
				sus_req_objects = 1;
			}

			printf("\r\nRequesting %hhu objects\r\n", sus_req_objects);

			if (sus_req_objects == 1) {
				if (DLMS_FORMAT_ERROR == dlms_cli_obj_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx,
						object_list[suc_current_object_idx], NULL)) {
					uc_current_req_result = DLMS_FORMAT_ERROR;
				}
			} else {
				if (DLMS_FORMAT_ERROR == dlms_cli_list_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx,
						(dlms_object_t *)&object_list[suc_current_object_idx], sus_req_objects, NULL)) {
					uc_current_req_result = DLMS_FORMAT_ERROR;
				}
			}
		}

		LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_WAIT_REQ_RESP\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
		uc_state_cycles = STATE_WAIT_REQ_RESP;
		break;

	case STATE_WAIT_REQ_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_SUCCESS:
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(uc_current_req_result);
#endif

			suc_current_object_idx += sus_req_objects;
			if (suc_current_object_idx < NUM_OBJECTS) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_REQ_OBJECT\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_REQ_OBJECT;
			} else {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_RELEASE_REQUEST\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_RELEASE_REQUEST;
			}

			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
#ifdef DLMS_REPORT_CONSOLE
				_print_result_step(DLMS_TIMEOUT);
#endif
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_CYCLE_NEXT_NODE;
				break;
			}

			break;

		case DLMS_RX_FAIL:
			/* Server reported an error when getting the object */
			/* Reflect ERROR in log report, but do not consider cycle failed, do the same as in SUCCESS result */
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(uc_current_req_result);
#endif

			suc_current_object_idx += sus_req_objects;
			if (suc_current_object_idx < NUM_OBJECTS) {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_REQ_OBJECT\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_REQ_OBJECT;
			} else {
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_RELEASE_REQUEST\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_RELEASE_REQUEST;
			}

			break;

		default:
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(uc_current_req_result);
#endif
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			break;
		}

		break;

	case STATE_RELEASE_REQUEST:
		/* Check if node is disconnected */
		if (!spx_current_addr_list[sus_current_node_idx].is_connected) {
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(DLMS_DISCONNECTED);
#endif
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  RLRQ DISCONNECTED ---> STATE_CYCLE_NEXT_NODE\r\n",
					spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			break;
		}

		dlms_cli_rlrq_request(spx_current_addr_list[sus_current_node_idx].short_addr, uc_assoc_idx, RLRQ_URGENT);
		LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_WAIT_RELEASE_RESP\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
		uc_state_cycles = STATE_WAIT_RELEASE_RESP;
		break;

	case STATE_WAIT_RELEASE_RESP:
		/* Check request result */
		switch (uc_current_req_result) {
		case DLMS_RELEASED:
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			us_node_idx = spx_current_addr_list[sus_current_node_idx].stats_idx;
			sx_cycles_stat[us_node_idx].b_last_cycle_success = true;
			break;

		case DLMS_WAITING:
			/* Check if waiting timer expired */
			if (!sul_data_timeout_timer) {
#ifdef DLMS_REPORT_CONSOLE
				_print_result_step(DLMS_TIMEOUT);
#endif
				LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n",
						spx_current_addr_list[sus_current_node_idx].short_addr));
				uc_state_cycles = STATE_CYCLE_NEXT_NODE;
				break;
			}

			break;

		default:
#ifdef DLMS_REPORT_CONSOLE
			_print_result_step(uc_current_req_result);
#endif
			LOG_APP_DEBUG(("[DLMS_APP] dlms_app_process: %hu  STATE_CYCLE_NEXT_NODE\r\n", spx_current_addr_list[sus_current_node_idx].short_addr));
			uc_state_cycles = STATE_CYCLE_NEXT_NODE;
			break;
		}

		break;

	default:
		break;
	}
}

/**
 * \brief Join indication handler
 *
 * \param puc_extended_address  extended address of the joining node
 * \param us_short_address      short address
 */
void dlms_app_join_node(uint8_t *puc_extended_address, uint16_t us_short_address)
{
	uint8_t puc_ext_addr_ascii[24];
	uint16_t us_node_idx, us_stats_idx;

	sprintf((char *)puc_ext_addr_ascii, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
			puc_extended_address[0], puc_extended_address[1], puc_extended_address[2],
			puc_extended_address[3], puc_extended_address[4], puc_extended_address[5],
			puc_extended_address[6], puc_extended_address[7]);

	LOG_APP_DEBUG(("[DLMS_APP] dlms_app_join_node: short_address = %hu extended addr %s\r\n", us_short_address, puc_ext_addr_ascii));

	sus_num_reg_nodes = app_update_registered_nodes(&px_node_list);

	if (sus_num_nodes_ever_connected >= DLMS_MAX_DEV_NUM) {
		return;
	}

	dlms_cli_con_opened(us_short_address);

	/* Check if node has previous statistics */
	for (us_stats_idx = 0; us_stats_idx < sus_num_nodes_ever_connected; us_stats_idx++) {
		if (!memcmp(sx_cycles_stat[us_stats_idx].puc_extended_address, puc_extended_address, EXT_ADDR_LEN)) {
			break;
		}
	}

	/* If not, increase nodes ever connected and record its MAC addr */
	if (us_stats_idx == sus_num_nodes_ever_connected) {
		memcpy(sx_cycles_stat[sus_num_nodes_ever_connected].puc_extended_address, puc_extended_address, EXT_ADDR_LEN);
		sus_num_nodes_ever_connected++;
	}

	for (us_node_idx = 0; us_node_idx < sus_current_cycle_num_nodes; us_node_idx++) {
		/* Set node as connected */
		if (spx_current_addr_list[us_node_idx].short_addr == us_short_address) {
			spx_current_addr_list[us_node_idx].is_connected = true;
			break;
		}
	}

#ifdef DLMS_APP_WAIT_REG_NODES
	/* Restart Waiting Timer */
	sul_dlms_start_timer = DLMS_TIME_WAITING_IDLE;
#endif
}

/**
 * \brief Leave Node Handler
 *
 */
void dlms_app_leave_node(uint16_t us_short_address)
{
	uint16_t us_node_idx;

	LOG_APP_DEBUG(("[DLMS_APP] dlms_app_leave_node: short_address = %hu\r\n", us_short_address));

	dlms_cli_con_closed(us_short_address);

	/* Reset node in cycle list */
	for (us_node_idx = 0; us_node_idx < sus_current_cycle_num_nodes; us_node_idx++) {
		/* If found, set node as disconnected */
		if (spx_current_addr_list[us_node_idx].short_addr == us_short_address) {
			spx_current_addr_list[us_node_idx].is_connected = false;
			break;
		}
	}

	sus_num_reg_nodes = app_update_registered_nodes(&px_node_list);

#ifdef DLMS_APP_WAIT_REG_NODES
	/* Restart Waiting Timer */
	sul_dlms_start_timer = DLMS_TIME_WAITING_IDLE;
#endif
}

/**
 * \brief Path Request Confirmation Handler
 *
 */
void dlms_app_path_node_cfm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
#ifdef DLMS_APP_ENABLE_PATH_REQ
	struct TAdpPathDiscoveryConfirm *px_path_node;

	/* Use us_num_path_nodes as index of the path table */
	px_path_node = &sx_path_nodes[us_num_path_nodes];

	/* Update PATH info */
	memcpy(px_path_node, pPathDiscoveryConfirm, sizeof(struct TAdpPathDiscoveryConfirm));

	/* Update next node */
	us_num_path_nodes++;
	uc_state_cycles = STATE_PATH_LIST_REQ;
#else
	(void)pPathDiscoveryConfirm;
#endif
}

/**
 * \brief Create main Cycles Application task and create timer to update internal counters.
 *
 */
void dlms_app_init(void)
{
	uint16_t us_node_idx;

	/* DLMS Client lib init */
	dlms_cli_init(px_assoc_conf, DLMS_MAX_ASSOC, spx_node_info, DLMS_MAX_DEV_NUM, _dlms_app_request_cb, _dlms_app_response_cb);
	dlms_cli_conf_obis(1, 0, 99, 1, 0, 255, 7, obis_1_0_99_1_0_255_cb);
	dlms_cli_conf_obis(0, 0, 1, 0, 0, 255, 8, obis_0_0_1_0_0_255_cb);
	/* G3 Profile specific */
	dlms_cli_conf_obis(0, 0, 29, 0, 0, 255, 90, obis_0_0_29_0_0_255_cb);
	dlms_cli_conf_obis(0, 0, 29, 1, 0, 255, 91, obis_0_0_29_1_0_255_cb);
	dlms_cli_conf_obis(0, 0, 29, 2, 0, 255, 92, obis_0_0_29_2_0_255_cb);

	/* Init variables and status */
	sus_num_nodes_ever_connected = 0;
	uc_state_cycles = STATE_IDLE;
	sus_num_reg_nodes = 0;

	/* Clear Node Information */
	memset(px_node_list, 0, sizeof(px_node_list));
	memset(sx_cycles_stat, 0, sizeof(sx_cycles_stat));

	/* Current cycle node list init */
	sus_current_cycle_num_nodes = 0;
	for (us_node_idx = 0; us_node_idx < DLMS_MAX_DEV_NUM; us_node_idx++) {
		spx_current_addr_list[us_node_idx].short_addr = LBS_INVALID_SHORT_ADDRESS;
		spx_current_addr_list[us_node_idx].stats_idx = 0xFFFF; /* invalid */
		spx_current_addr_list[us_node_idx].is_connected = false;
	}

	for (us_node_idx = 0; us_node_idx < MAX_LBDS; us_node_idx++) {
		px_node_list[us_node_idx].us_short_address = LBS_INVALID_SHORT_ADDRESS;
	}

	LOG_APP_DEBUG(("[DLMS_APP] DLMS APP Application: COORDINATOR\r\n"));
	_show_version();
	LOG_APP_DEBUG(("[DLMS_APP] dlms_app_init: STATE_WAIT_START_CYCLES\r\n"));

	/* Init local vars */
	sul_dlms_start_timer = DLMS_TIME_WAITING_IDLE;
	sul_next_cycle_timer = 0;
	sul_data_timeout_timer = 0;

	sul_cycle_counter = 1;

#ifdef DLMS_APP_ENABLE_PATH_REQ
	us_num_path_nodes = 0;
	memset(sx_path_nodes, 0, sizeof(sx_path_nodes));
#endif
}
