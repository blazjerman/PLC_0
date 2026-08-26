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

#ifndef BS_FUNCTIONS_H
#define BS_FUNCTIONS_H

#include <stdbool.h>
#include <bs_api.h>

/* #define LOG_BOOTSTRAP(a)   printf a */
#define LOG_BOOTSTRAP(a)   (void)0

/* In the device list (g_lbds_list), the short address is specified by the */
/* index in the list plus an offste given by initialShortAddr. */
typedef struct {
	uint8_t puc_extended_address[ADP_ADDRESS_64BITS];
} lbds_list_entry_t;

uint16_t get_lbds_count(void);
uint8_t device_is_in_list(uint16_t us_short_address);
void remove_lbds_list_entry(uint16_t us_short_address);
uint16_t get_new_address(struct TAdpExtendedAddress *pLbdAddress);
bool add_lbds_list_entry(const uint8_t *puc_extended_address, uint16_t us_short_address);
void init_coord_data(void);
uint8_t dev_is_in_blacklist(uint8_t *puc_address);
uint8_t add_to_blacklist(uint8_t *puc_address);
uint8_t remove_from_blacklist(uint16_t us_index);
uint16_t get_initial_short_address(void);
bool set_initial_short_address(uint16_t us_short_addr);
bool get_ib_short_address_from_extended(void);
void set_ib_short_address_from_extended(bool value);

#endif /* BS_FUNCTIONS_H */
