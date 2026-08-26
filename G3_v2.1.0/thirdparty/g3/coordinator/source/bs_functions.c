/**
 *
 * \file
 *
 * \brief Coordinator module functions
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
#include <string.h>
#include <stdio.h>
#include <AdpApi.h>
#include <AdpApiTypes.h>
#include <ProcessLbpCoord.h>

#include "bs_functions.h"
#include "bs_api.h"
#include "conf_bs.h"

/* Address selection procedure */
static bool bGetShortAddressFromExtended;
static uint16_t u16InitialShortAddr;

/* Blacklist */
static uint16_t u16BlacklistSize = 0;
static uint8_t puc_blacklist[MAX_LBDS][ADP_ADDRESS_64BITS];

/* LBDs table */
lbds_list_entry_t g_lbds_list[MAX_LBDS];
static uint16_t g_lbds_counter = 0;
static uint16_t g_lbds_list_size = 0;

/**
 * \brief Returns if the short address is valid
 *
 * \param us_short_address  Short address
 *
 * \return true / false.
 */
static bool _is_valid_address(uint16_t us_short_address)
{
	/* Check if the short address is out of the range */
	if (us_short_address < u16InitialShortAddr) {
		return false;
	} else if (us_short_address - u16InitialShortAddr > MAX_LBDS) {
		return false;
	} else {
		return true;
	}
}

/**
 * \brief Returns if the short address is valid
 *
 * \param puc_extended_address  Extended address address
 *
 * \return true / false.
 */
static bool _is_null_address(uint8_t *puc_extended_address)
{
	uint8_t i;
	/* Check if the short address is null */
	for (i = 0; i < ADP_ADDRESS_64BITS; i++) {
		/* If any of the bytes of the address isn't null, return false. */
		if (puc_extended_address[i] != 0x00) {
			return false;
		}
	}
	/* At this point, the address is null */
	return true;
}

/**
 * \brief Function to handle joined devices list as a hash, indexed
 *        by short address
 *
 * \param us_short_address
 *
 * \return true if index found, false otherwise.
 */
static bool _get_ext_addr_by_short(uint16_t us_short_address, uint8_t *puc_extended_address)
{
	uint16_t us_index;
	bool found = false;

	us_index = us_short_address - u16InitialShortAddr;

	if (!_is_null_address(g_lbds_list[us_index].puc_extended_address)) {
		memcpy(puc_extended_address, g_lbds_list[us_index].puc_extended_address, ADP_ADDRESS_64BITS);
		found = true;
	} else {
		LOG_BOOTSTRAP(("[BS] Error: tried to access joined devices list by not-joined short address.\r\n"));
	}

	return found;
}

/**
 * \brief Returns the number of active LBDs
 *
 * \return number of active LBDs
 */
uint16_t get_lbds_count(void)
{
	uint16_t lbd_count, us_idx;

	lbd_count = 0;
	for (us_idx = 0; us_idx < MAX_LBDS; us_idx++) {
		if (!(_is_null_address(g_lbds_list[us_idx].puc_extended_address))) {
			lbd_count++;
		}
	}

	return lbd_count;
}

/**
 * \brief Returns if the device is active in the LBDs list
 *
 * \param us_short_address short address
 *
 * \return 1 (yes) / 0 (no)
 *
 * NOTE: Returns 0 if the address is not active.
 */
uint8_t device_is_in_list(uint16_t us_short_address)
{
	/* Check if the short address is out of the range */
	if (!_is_valid_address(us_short_address)) {
		return (0);
	}

	/* Check if the address of the device is active */
	if (_is_null_address(g_lbds_list[us_short_address - u16InitialShortAddr].puc_extended_address)) {
		return (0);
	} else {
		return (1);
	}
}

/**
 * \brief Deactivates the LBD specified by its short address
 *
 * \param us_short_address short address
 */
void remove_lbds_list_entry(uint16_t us_short_address)
{
	/* Check if the short address is out of the range */
	if (!_is_valid_address(us_short_address)) {
		LOG_BOOTSTRAP(("[BS] Error: attempted to deactivate an address out of range [0x%04x]\r\n", us_short_address));
		return;
	} else {
		/* Check if the address is active */
		if (!_is_null_address(g_lbds_list[us_short_address - u16InitialShortAddr].puc_extended_address)) {
			/* Deactivate address */
			memset(&g_lbds_list[us_short_address -
					u16InitialShortAddr].puc_extended_address, 0, ADP_ADDRESS_64BITS * sizeof(uint8_t));
			g_lbds_counter--;
		} else {
			/* The address is not active -> The device hasn't joined */
			LOG_BOOTSTRAP(("[BS] Error: attempted to deactivate an inactive address [0x%04x]\r\n", us_short_address));
		}
	}

	return;
}

/**
 * \brief Returns a new address that is not in use.
 *  If the end of the LBDs list is not reached, a new address is given instead
 *  of reusing the addresses that became free. This way, the time to reuse an
 *  address is extended.
 *
 * \return short address
 *
 * NOTE: 0x0000 will be returned if the LBDs list is full
 */
uint16_t get_new_address(struct TAdpExtendedAddress *pLbdAddress)
{
	uint16_t us_short_address = 0x0000;

	if (bGetShortAddressFromExtended) {
		if (g_lbds_list_size < MAX_LBDS) {
			us_short_address = (pLbdAddress->m_au8Value[6] << 8) | pLbdAddress->m_au8Value[7];
			g_lbds_list_size++;
		}

		return us_short_address;
	} else {
		uint16_t i = 0;

		/* If the end of the list is not reached, give the next address & increase list size */
		if (g_lbds_list_size < MAX_LBDS) {
			us_short_address = g_lbds_list_size + u16InitialShortAddr;
			g_lbds_list_size++;
			return (us_short_address);
		} else {
			/* End of the list reached: Go through the list to find free positions */
			while ((i < MAX_LBDS)) {
				/* Check if the position is free (null extended address) */
				if (_is_null_address(g_lbds_list[i].puc_extended_address)) {
					/* Free position: Return the address (calculated with the index and the initial short address) */
					us_short_address = i + u16InitialShortAddr;
					return (us_short_address);
				}

				i++;
			}
		}

		/* At this point, there is no free positions in the list. Maximum number of devices reached. */
		LOG_BOOTSTRAP(("[BS] Device node: 0x%04x cannot be registered. Maximum number of nodes reached: %d\r\n", g_lbds_counter, MAX_LBDS));
		return us_short_address;
	}
}

/**
 * \brief Adds a LBD to the devices' list. A short address is assigned.
 *
 * \param puc_extended_address extended address
 * \param us_short_address short address
 * \param bitmap with the configuration (specifies if the LBD is active)
 *
 * \return true / false.
 */
bool add_lbds_list_entry(const uint8_t *puc_extended_address, uint16_t us_short_address)
{
	/* Check if the short address is out of the range */
	if (!_is_valid_address(us_short_address)) {
		LOG_BOOTSTRAP(("[BS] Error: attempted to activate an address out of range [0x%04x]\r\n", us_short_address));
		return false;
	}

	uint16_t us_position = us_short_address - u16InitialShortAddr;
	uint16_t us_dummy_short_address;

	/* Check if the short address is already in use */
	if (!_is_null_address(g_lbds_list[us_position].puc_extended_address)) {
		LOG_BOOTSTRAP(("[BS] Short address already in use [0x%04x], not added to list\r\n", us_short_address));
		return false;
	}

	/* Check if the extended address is already in use */
	if (coord_get_short_addr_by_ext((uint8_t *)puc_extended_address, &us_dummy_short_address)) {
		LOG_BOOTSTRAP(("[BS] Extended address already in use [%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]\r\n",
				*puc_extended_address, *(puc_extended_address + 1), *(puc_extended_address + 2),
				*(puc_extended_address + 3), *(puc_extended_address + 4), *(puc_extended_address + 5),
				*(puc_extended_address + 6), *(puc_extended_address + 7)));
		return false;
	}

	memcpy(g_lbds_list[us_position].puc_extended_address, puc_extended_address, 8);
	LOG_BOOTSTRAP(("[BS] Added address [0x%04x]\r\n", us_short_address));
	g_lbds_counter++;
	LOG_BOOTSTRAP(("[BS] Total num. devices: %d.\r\n", g_lbds_counter));

	return true;
}

/**
 * \brief Function to handle joined devices list as a hash, indexed
 *        by extended address
 *
 * \param
 *
 * \return true if index found, false otherwise.
 */
bool coord_get_short_addr_by_ext(uint8_t *puc_extended_address, uint16_t *pus_short_address)
{
	uint16_t i = 0;
	uint16_t j = 0;
	bool found = false;

	while (j < g_lbds_list_size) {
		if (!_is_null_address(g_lbds_list[j].puc_extended_address)) {
			/* Check if the short address matches */
			found = true;
			for (i = 0; i < ADP_ADDRESS_64BITS; i++) {
				/* If any of the bytes of the address isn't null, go on. */
				if (puc_extended_address[i] != g_lbds_list[j].puc_extended_address[i]) {
					found = false;
					break;
				}
			}
		}

		if (found) {
			*pus_short_address = j + u16InitialShortAddr;
			break;
		}

		j++;
	}

	if (found) {
		LOG_BOOTSTRAP(("[BS] Extended address found in joined devices list with short address 0x%04X.\r\n", *pus_short_address));
	} else {
		LOG_BOOTSTRAP(("[BS] Error: extended address not found in joined devices list.\r\n"));
	}

	return found;
}

void coord_kick_device(uint16_t us_short_address)
{
	struct TAdpExtendedAddress extAddress;

	/* Check if the device had joined the network */
	if (device_is_in_list(us_short_address)) {
		_get_ext_addr_by_short(us_short_address, extAddress.m_au8Value);
		if (LBP_KickDevice(us_short_address, &extAddress)) {
			/* Remove the device from the joined devices list */
			remove_lbds_list_entry(us_short_address);
		}
	}
}

/**
 * \brief Get initial short address
 *
 */
uint16_t get_initial_short_address(void)
{
	return u16InitialShortAddr;
}

/**
 * \brief Set initial short address
 *
 */
bool set_initial_short_address(uint16_t us_short_addr)
{
	if (bGetShortAddressFromExtended) {
		/* Do not allow change initial address */
		return false;
	} else {
		if (us_short_addr == 0) {
			return false;
		} else if (us_short_addr > (0xFFFF - MAX_LBDS)) {
			return false;
		} else {
			u16InitialShortAddr = us_short_addr;
			return true;
		}
	}
}

/**
 * \brief Get ShortAddressFromExtended IB value
 *
 */
bool get_ib_short_address_from_extended(void)
{
	return bGetShortAddressFromExtended;
}

/**
 * \brief Set ShortAddressFromExtended IB value
 *
 */
void set_ib_short_address_from_extended(bool value)
{
	bGetShortAddressFromExtended = value;
}

/**
 * \brief Initialize data
 *
 */
void init_coord_data(void)
{
	/* Initialize LBP blacklist */
	u16BlacklistSize = 0;
	memset(puc_blacklist, 0, MAX_LBDS * ADP_ADDRESS_64BITS);

	/* Initialize LBP list */
	g_lbds_counter = 0;
	g_lbds_list_size = 0;
	memset(g_lbds_list, 0, MAX_LBDS * sizeof(lbds_list_entry_t));

	bGetShortAddressFromExtended = false;
	u16InitialShortAddr = 1;
}

/**
 * \brief Device is in blacklist
 *
 */
uint8_t dev_is_in_blacklist(uint8_t *puc_address)
{
	uint16_t i = 0;
	uint8_t uc_found = 0;

	while (i < u16BlacklistSize) {
		if (!memcmp(puc_address, puc_blacklist[i], ADP_ADDRESS_64BITS)) {
			uc_found = 1;
			break;
		}

		i++;
	}

	return uc_found;
}

/**
 * \brief Add to blacklist
 *
 */
uint8_t add_to_blacklist(uint8_t *puc_address)
{
	/* TO DO: advanced list management */

	uint8_t uc_status = 1;

	if (u16BlacklistSize < MAX_LBDS) {
		memcpy(puc_blacklist[u16BlacklistSize], puc_address, ADP_ADDRESS_64BITS);
		u16BlacklistSize++;
	} else {
		/* Blacklist full - error */
		uc_status = 0;
	}

	return uc_status;
}

/**
 * \brief Remove from blacklist
 *
 */
uint8_t remove_from_blacklist(uint16_t us_index)
{
	uint8_t uc_status = 0;

	/* TO DO: advanced list management */
	if (us_index < MAX_LBDS) {
		memset(puc_blacklist[us_index], 0, ADP_ADDRESS_64BITS);
		uc_status = 1;
	}

	return uc_status;
}
