/**
 *
 * \file
 *
 * \brief MIB Serialization Header file
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

#ifndef THIRDPARTY_G3_MAC_ADDONS_SERIAL_IF_SERIAL_IF_MIB_COMMON_H_
#define THIRDPARTY_G3_MAC_ADDONS_SERIAL_IF_SERIAL_IF_MIB_COMMON_H_

#include <stdint.h>
#include "mac_wrapper.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
	#endif
/**INDENT-ON**/
/* / @endcond */

void process_MIB_get_request(uint8_t *puc_serial_data, enum EMacWrpPibAttribute *eAttribute, uint16_t *u16AttributeIndex);
void process_MIB_set_request(uint8_t *puc_serial_data, enum EMacWrpPibAttribute *eAttribute, uint16_t *u16AttributeIndex, struct TMacWrpPibValue *pibValue);

uint8_t process_MIB_get_confirm(uint8_t *puc_serial_data, enum EMacWrpStatus eGetStatus, enum EMacWrpPibAttribute eAttribute,
                                uint16_t u16Index, struct TMacWrpPibValue *pibValue);
uint8_t process_MIB_set_confirm(uint8_t *puc_serial_data, enum EMacWrpStatus eGetStatus, enum EMacWrpPibAttribute eAttribute, uint16_t u16Index);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* THIRDPARTY_G3_MAC_ADDONS_SERIAL_IF_SERIAL_IF_MIB_COMMON_H_ */
