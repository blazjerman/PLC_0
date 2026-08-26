/**
 *
 * \file
 *
 * \brief Common Serialization Header file
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

#ifndef SERIAL_IF_SERIAL_IF_COMMON_H
#define SERIAL_IF_SERIAL_IF_COMMON_H

#include <stdint.h>

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
	#endif
/**INDENT-ON**/
/* / @endcond */

/* Defines the working mode */
enum ESerialMode {
	SERIAL_MODE_NOT_INITIALIZED, /* allowed: AdpInitialize, MacInitialize and CoordInitialize) */
	SERIAL_MODE_MAC,  /* access to only MAC layer (MacGet and MacSet operations are allowed) */
	SERIAL_MODE_ADP_DEV,  /* access to ADP layer and LBP in Device mode (MacGet and MacSet operations are allowed) */
	SERIAL_MODE_ADP_COORD  /* access to ADP layer and LBP in Coordinator mode (MacGet and MacSet operations are allowed) */
};

void adp_mac_serial_if_init(void);
void adp_mac_serial_if_process(void);

void adp_mac_serial_if_set_state(enum ESerialMode e_state);
enum ESerialMode  adp_mac_serial_if_get_state(void);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* SERIAL_IF_SERIAL_IF_COMMON_H */
