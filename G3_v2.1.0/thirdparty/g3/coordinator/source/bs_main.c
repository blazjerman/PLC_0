/**
 *
 * \file
 *
 * \brief Coordinator module
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <AdpApiTypes.h>
#include <AdpApi.h>
#include <mac_wrapper.h>
#include <oss_if.h>

#include "bs_api.h"
#include "bs_functions.h"
#include "ProcessLbpCoord.h"
#include "conf_bs.h"

#define LBP_REKEYING_PHASE_DISTRIBUTE    0
#define LBP_REKEYING_PHASE_ACTIVATE      1

static bool bRekey;
static uint16_t us_rekey_idx = 0;
static uint16_t us_rekey_phase = LBP_REKEYING_PHASE_DISTRIBUTE;

extern lbds_list_entry_t g_lbds_list[MAX_LBDS];

static pf_app_leave_ind_cb_t pf_app_leave_ind_cb;
static pf_app_join_ind_cb_t pf_app_join_ind_cb;

static void _rekeying_process(void)
{
	struct TAdpExtendedAddress x_ext_address;
	uint16_t u16ShortAddr;

	memcpy(x_ext_address.m_au8Value, g_lbds_list[us_rekey_idx].puc_extended_address, ADP_ADDRESS_64BITS);
	u16ShortAddr = us_rekey_idx + get_initial_short_address();
	
	LBP_Rekey(u16ShortAddr, &x_ext_address, (us_rekey_phase == LBP_REKEYING_PHASE_DISTRIBUTE));
}

static void JoinRequestIndication(uint8_t* au8LbdAddress)
{
	uint16_t u16AssignedAddress;
	uint16_t u16DummyShortAddress;
	struct TAdpExtendedAddress extAddress;

	if (!bRekey) {
		/* A new device is willing to Join the Network */
		if (dev_is_in_blacklist(au8LbdAddress)) {
			/* Assign invalid address */
			u16AssignedAddress = 0xFFFF;
		}
		else {
			/* If extended address is already in list, remove it and give a new short address */
			if (coord_get_short_addr_by_ext(au8LbdAddress, &u16DummyShortAddress)) {
				remove_lbds_list_entry(u16DummyShortAddress);
			}

			/* Get a new address for the device. Its extended address will be added to the list when the joining process finishes. */
			memcpy(extAddress.m_au8Value, au8LbdAddress, sizeof(struct TAdpExtendedAddress));
			u16AssignedAddress = get_new_address(&extAddress);
		}
	}
	else {
		/* In case of rekeying, assign any value, except the invalid address, so frame is accepted */
		u16AssignedAddress = 0xFFF0;
	}

	/* Assign Address */
	LBP_ShortAddressAssign(au8LbdAddress, u16AssignedAddress);
}

static void JoinCompleteIndication(uint8_t* au8LbdAddress, uint16_t u16AssignedAddress)
{
	if (bRekey) {
		/* If a re-keying phase is in progress */
		if (us_rekey_idx < get_lbds_count() - 1) {
			us_rekey_idx++;
			/* Continue rekeying process with next node */
			_rekeying_process();
		} else {
			if (us_rekey_phase == LBP_REKEYING_PHASE_DISTRIBUTE) {
				/* All devices have been provided with the new GMK -> next phase */
				us_rekey_phase = LBP_REKEYING_PHASE_ACTIVATE;
				/* Reset the index to send GMK activation to all joined devices. */
				us_rekey_idx = 0;
				/* Call process to start activation phase */
				_rekeying_process();
			} else {
				/* End of re-keying process */
				us_rekey_idx = 0;
				bRekey = false;
				LBP_SetRekeyPhase(false);
				LBP_activate_new_key();
			}
		}
	}
	else {
		/* After the join process finishes, the entry is added to the list */
		add_lbds_list_entry(au8LbdAddress, u16AssignedAddress);
		/* Call app callback if defined */
		if (pf_app_join_ind_cb != NULL) {
			pf_app_join_ind_cb(au8LbdAddress, u16AssignedAddress);
		}
	}
}

static void LeaveIndication(uint16_t u16NetworkAddress)
{
	/* Remove the device from the joined devices list */
	remove_lbds_list_entry(u16NetworkAddress);
	/* Call app callback if defined */
	if (pf_app_leave_ind_cb != NULL) {
		pf_app_leave_ind_cb(u16NetworkAddress);
	}
}

static struct TLbpNotificationsCoord lbpNotificationsCoord = {
	JoinRequestIndication,
	JoinCompleteIndication,
	LeaveIndication
};

/**
 * coord_init.
 *
 */
void coord_init(bool aribBand)
{
	LBP_InitCoord(aribBand);

	/* Set notifications */
	LBP_SetNotificationsCoord(&lbpNotificationsCoord);

	/* Initialize data */
	init_coord_data();

	/* Init function pointers */
	pf_app_leave_ind_cb = NULL;
	pf_app_join_ind_cb = NULL;
}

/**
 * coord_process
 *
 */
void coord_process(void)
{
	LBP_UpdateBootstrapSlots();
}

/**
 * coord_get_lbds_counter.
 *
 */
uint16_t coord_get_lbds_counter(void)
{
	return get_lbds_count();
}

/**
 * coord_get_param.
 *
 */
void coord_get_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, struct t_coord_get_param_confirm *p_get_confirm)
{
	p_get_confirm->ul_attribute_id = ul_attribute_id;
	p_get_confirm->us_attribute_idx = us_attribute_idx;
	p_get_confirm->uc_attribute_length = 0;
	p_get_confirm->uc_status = COORD_STATUS_UNSUPPORTED_PARAMETER;

	if (ul_attribute_id == COORD_IB_DEVICE_LIST) {
		if (us_attribute_idx < MAX_LBDS) {
			uint16_t us_short_address = us_attribute_idx + get_initial_short_address();
			/* Check valid entry */
			if (device_is_in_list(us_short_address)) {
				p_get_confirm->uc_status = COORD_STATUS_OK;
				p_get_confirm->uc_attribute_length = 10;
				p_get_confirm->uc_attribute_value[0] = (uint8_t)(us_short_address & 0x00FF);
				p_get_confirm->uc_attribute_value[1] = (uint8_t)(us_short_address >> 8);
				memcpy(&p_get_confirm->uc_attribute_value[2], g_lbds_list[us_attribute_idx].puc_extended_address, ADP_ADDRESS_64BITS);
			} else {
				/* Empty list entry */
				LOG_BOOTSTRAP(("[BS] LBDs entry not valid for index %hu\r\n", us_attribute_idx));
				p_get_confirm->uc_status = COORD_STATUS_INVALID_VALUE;
				memset(&p_get_confirm->uc_attribute_value[0], 0, 10);
			}
		} else {
			/* Invalid list index */
			LOG_BOOTSTRAP(("[BS] Index %hu out of range of LBDs list\r\n", us_attribute_idx));
			p_get_confirm->uc_status = COORD_STATUS_INVALID_INDEX;
			memset(&p_get_confirm->uc_attribute_value[0], 0, 10);
		}
	} else if (ul_attribute_id == COORD_IB_INITIAL_SHORT_ADDRESS) {
		uint16_t us_initial_short_address = get_initial_short_address();
		p_get_confirm->uc_status = COORD_STATUS_OK;
		p_get_confirm->uc_attribute_length = 2;
		p_get_confirm->uc_attribute_value[0] = (uint8_t)(us_initial_short_address >> 8);
		p_get_confirm->uc_attribute_value[1] = (uint8_t)(us_initial_short_address & 0x00FF);
	} else if (ul_attribute_id == COORD_IB_SHORT_ADDRESS_FROM_EXTENDED) {
		bool short_address_from_extended_val = get_ib_short_address_from_extended();
		p_get_confirm->uc_status = COORD_STATUS_OK;
		p_get_confirm->uc_attribute_length = 1;
		p_get_confirm->uc_attribute_value[0] = (uint8_t)(short_address_from_extended_val);
	} else {
		/* Unknown LBS parameter */
	}
}

/**
 * coord_set_param.
 *
 */
void coord_set_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, uint8_t uc_attribute_len, const uint8_t *puc_attribute_value,
		struct t_coord_set_param_confirm *p_set_confirm)
{
	uint16_t us_tmp_short_addr;

	p_set_confirm->ul_attribute_id = ul_attribute_id;
	p_set_confirm->us_attribute_idx = us_attribute_idx;
	p_set_confirm->uc_status = COORD_STATUS_UNSUPPORTED_PARAMETER;

	switch (ul_attribute_id) {
	case COORD_IB_DEVICE_LIST:
		if (uc_attribute_len == 10) {
			us_tmp_short_addr = ((puc_attribute_value[1] << 8) | puc_attribute_value[0]);
			/* Both short and extended addresses are specified */
			if (add_lbds_list_entry(&puc_attribute_value[2], us_tmp_short_addr)) {
				p_set_confirm->uc_status = COORD_STATUS_OK;
			} else {
				LOG_BOOTSTRAP(("[BS] Address: 0x%04x already in use.\r\n", us_tmp_short_addr));
				p_set_confirm->uc_status = COORD_STATUS_INVALID_VALUE;
			}
		} else {
			/* Wrong parameter size */
			p_set_confirm->uc_status = COORD_STATUS_INVALID_LENGTH;
		}

		break;

	case COORD_IB_INITIAL_SHORT_ADDRESS:
		if (uc_attribute_len == 2) {
			if (get_lbds_count() > 0) {
				/* If there are active LBDs, the initial short address cannot be changed */
				p_set_confirm->uc_status = COORD_STATUS_NOK;
			} else {
				us_tmp_short_addr = ((puc_attribute_value[1] << 8) | puc_attribute_value[0]);

				if (set_initial_short_address(us_tmp_short_addr)) {
					p_set_confirm->uc_status = COORD_STATUS_OK;
				} else {
					p_set_confirm->uc_status = COORD_STATUS_INVALID_VALUE;
				}
			}
		} else {
			/* Wrong parameter size */
			p_set_confirm->uc_status = COORD_STATUS_INVALID_LENGTH;
		}

		break;

	case COORD_IB_ADD_DEVICE_TO_BLACKLIST:
		if (uc_attribute_len == 8) {
			add_to_blacklist((uint8_t *)puc_attribute_value);
			p_set_confirm->uc_status = COORD_STATUS_OK;
		} else {
			/* Wrong parameter size */
			p_set_confirm->uc_status = COORD_STATUS_INVALID_LENGTH;
		}

		break;

	case COORD_IB_SHORT_ADDRESS_FROM_EXTENDED:
		if (uc_attribute_len == 1) {
			set_ib_short_address_from_extended((bool) * puc_attribute_value);
			p_set_confirm->uc_status = COORD_STATUS_OK;
		} else {
			/* Wrong parameter size */
			p_set_confirm->uc_status = COORD_STATUS_INVALID_LENGTH;
		}

		break;

	default:
		/* Unknown LBP parameter */
		break;
	}
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void coord_launch_rekeying(void)
{
	/* If there are devices that joined the network */
	if (get_lbds_count() > 0) {
		/* Start the re-keying process */
		bRekey = true;
		LBP_SetRekeyPhase(true);
		us_rekey_phase = LBP_REKEYING_PHASE_DISTRIBUTE;
		/* Send the first re-keying process */
		us_rekey_idx = 0;
		_rekeying_process();
		LOG_BOOTSTRAP(("[BS] Re-keying launched.\r\n"));
	} else {
		/* Error: no device in the network */
		LOG_BOOTSTRAP(("[BS] Re-keying NOT launched: no device in the network.\r\n"));
	}
}

/**
 * coord_leave_ind_set_cb.
 *
 */
void coord_leave_ind_set_cb(pf_app_leave_ind_cb_t pf_handler)
{
	pf_app_leave_ind_cb = pf_handler;
}

/**
 * coord_join_ind_set_cb.
 *
 */
void coord_join_ind_set_cb(pf_app_join_ind_cb_t pf_handler)
{
	pf_app_join_ind_cb = pf_handler;
}
