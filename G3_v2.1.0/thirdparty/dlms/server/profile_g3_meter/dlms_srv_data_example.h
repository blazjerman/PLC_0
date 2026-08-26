/**
 * \file
 *
 * \brief DLMS_SRV_LIB : DLMS server lib: G3 Profile
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
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

#ifndef DLMS_SERVER_DATA_EXAMPLE_H_INCLUDED
#define DLMS_SERVER_DATA_EXAMPLE_H_INCLUDED

#include <stdint.h>
#include "dlms_srv_types.h"

/** Size of each load PRIME Companion standard load profile register */
#define SIZE_LOAD_PROFILE_REG            48
/** Size of each mac_neighbour_table entry */
#define SIZE_MAC_NEIGHBOUR_TABLE_ENTRY   29
/** Size of each adp_routing_profile entry */
#define SIZE_ADP_ROUTING_PROFILE_ENTRY   18
/** Size of each mac_neighbour_table entry */
#define SIZE_EXTENDED_ADDR_TABLE_ENTRY   15

#ifndef CONF_MAX_NEIGHBOUR_DLMS_SNAPSHOT_SIZE
	#define CONF_MAX_NEIGHBOUR_DLMS_SNAPSHOT_SIZE   75
#endif

#ifndef CONF_MAX_ROUTING_DLMS_SNAPSHOT_SIZE
	#define CONF_MAX_ROUTING_DLMS_SNAPSHOT_SIZE     150
#endif

#define MAX_NEIGHBOUR_SNAPSHOT_SIZE   CONF_MAX_NEIGHBOUR_DLMS_SNAPSHOT_SIZE
#define MAX_ROUTING_SNAPSHOT_SIZE     CONF_MAX_ROUTING_DLMS_SNAPSHOT_SIZE

/** Load profile register */
typedef struct {
	uint8_t puc_date_time[SIZE_DATE_TIME];
	uint8_t uc_amr_status;
	uint32_t us_act_import;
	uint32_t us_act_export;
	uint32_t us_react_q_i;
	uint32_t us_react_q_ii;
	uint32_t us_react_q_iii;
	uint32_t us_react_q_iv;
} load_profile_reg_t;

/** DLMS PRIME Profile specific functions */
data_access_result_t obis_0_0_1_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_28_7_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_21_0_5_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_21_0_6_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_1_0_99_1_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_96_1_x_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_1_0_0_2_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_1_0_1_8_x_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);

/** DLMS G3 Profile specific functions */
enum EMacWrpPibAttribute ic90_to_g3_mac(uint8_t attr);
enum EMacWrpPibAttribute ic91_to_g3_mac(uint8_t attr);
uint32_t ic92_to_g3_adp(uint8_t attr);
data_access_result_t obis_0_0_29_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_29_1_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_29_2_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);
data_access_result_t obis_0_0_29_2_1_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr); /* MANUFACTURER OBIS */
data_access_result_t obis_0_0_29_2_2_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr); /* MANUFACTURER OBIS */
data_access_result_t obis_0_0_29_2_3_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr); /* MANUFACTURER OBIS */
data_access_result_t obis_1_0_0_2_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);

/* Common functions */
void dlms_srv_data_init_example(meter_params_t *);
data_access_result_t obis_0_0_40_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr);

#endif /* DLMS_SERVER_DATA_EXAMPLE_H_INCLUDED */
