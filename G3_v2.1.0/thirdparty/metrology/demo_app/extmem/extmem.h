/**
 * \file
 *
 * \brief External Memory Header
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
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

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef EXTMEM_H_INCLUDED
#define EXTMEM_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "compiler.h"
#include "status_codes.h"

/* Addressing calculated for SST26VF QSPI flash memory. Size of erasable sectors: 4KB (0x0000 - 0x0FFF) */
/* Page Addresses */
typedef enum {
	MEM_REG_COMMS_ID = 0,
	MEM_REG_METROLOGY_ID,
	MEM_REG_ENERGY_ID,
	MEM_REG_EVENTS_ID,
	MEM_REG_TOU_ID,
	MEM_REG_DEMAND_ID,
	MEM_REG_HISTORY_ID,
	MEM_REG_RTC_ID,
	MEM_REG_IDS
} extmemreg_t;

status_code_t ExtMemInit(void);
uint8_t ExtMemGetUniqueId(uint8_t *puc_data, uint8_t len);
uint16_t ExtMemRead(extmemreg_t mem_reg_id, void *ptr_data);
uint16_t ExtMemReadSize(extmemreg_t mem_reg_id, void *ptr_data, uint16_t size);
uint16_t ExtMemWrite(extmemreg_t mem_reg_id, void *ptr_data);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* EXTMEM_H_INCLUDED */
