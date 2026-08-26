/**
 *
 * \file
 *
 * \brief Coordinator module API
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef BS_API_H
#define BS_API_H

#include <AdpApi.h>

/* Coord parameter get confirm */
struct t_coord_get_param_confirm {
	uint8_t uc_status;
	uint32_t ul_attribute_id;
	uint16_t us_attribute_idx;
	uint8_t uc_attribute_length;
	uint8_t uc_attribute_value[64];
};

/* Coord parameter set confirm */
struct t_coord_set_param_confirm {
	uint8_t uc_status;
	uint32_t ul_attribute_id;
	uint16_t us_attribute_idx;
};

/* List of Coord attribute IDs */
enum coord_ib_attribute {
	COORD_IB_DEVICE_LIST                 = 0x00000000,
	COORD_IB_INITIAL_SHORT_ADDRESS       = 0x00000001,
	COORD_IB_ADD_DEVICE_TO_BLACKLIST     = 0x00000002,
	COORD_IB_SHORT_ADDRESS_FROM_EXTENDED = 0x00000003
};

/* Coord status */
enum coord_status {
	COORD_STATUS_OK = 0,
	COORD_STATUS_NOK,
	COORD_STATUS_UNSUPPORTED_PARAMETER,
	COORD_STATUS_INVALID_INDEX,
	COORD_STATUS_INVALID_LENGTH,
	COORD_STATUS_INVALID_VALUE
};

/* User-defined callback for ADPM-NETWORK-LEAVE indication */
typedef void (*pf_app_leave_ind_cb_t)(uint16_t u16SrcAddr);
/* User-defined callback for ADPM-NETWORK-JOIN indication */
typedef void (*pf_app_join_ind_cb_t)(uint8_t *pExtAddr, uint16_t u16SrcAddr);

/* Bootstrap module initialization */
void coord_init(bool aribBand);

/* Bootstrap module process, must be called at least once a second */
void coord_process(void);

/* LBP parameters external access */
uint16_t coord_get_lbds_counter(void);
void coord_kick_device(uint16_t us_short_address);
void coord_get_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, struct t_coord_get_param_confirm *p_get_confirm);
void coord_set_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, uint8_t uc_attribute_len, const uint8_t *puc_attribute_value,
		struct t_coord_set_param_confirm *p_set_confirm);
bool coord_get_short_addr_by_ext(uint8_t *puc_extended_address, uint16_t *pus_short_address);
void coord_launch_rekeying(void);

/* Notifications for Device Join and Leave */
void coord_leave_ind_set_cb(pf_app_leave_ind_cb_t pf_handler);
void coord_join_ind_set_cb(pf_app_join_ind_cb_t pf_handler);

#endif /* BS_API_H */
