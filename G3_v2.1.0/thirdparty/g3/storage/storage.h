/**
 *
 * \file
 *
 * \brief Storage definition file
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

#ifndef __STORAGE_H__
#define __STORAGE_H__

/* System includes */
#include <AdpApi.h>
#include <mac_wrapper.h>

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
#define STORAGE_VERSION 3
#elif defined(__RF_MAC__)
#define STORAGE_VERSION 2
#else
#define STORAGE_VERSION 1
#endif

/* struct to store persistent info: header + data */
struct TPersistentInfo {
	uint32_t m_u32StartupCounter; /* First 4 bytes. Also changed from platform. */
	uint16_t m_u16Version;
	uint16_t m_u16Crc16;
	struct TAdpNonVolatileData m_data;
};

void store_persistent_data_GPBR(struct TAdpNonVolatileData *pNonVolatileData);
void store_persistent_info(void);
void load_persistent_info(void);

#endif
