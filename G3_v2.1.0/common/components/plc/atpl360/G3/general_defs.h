/**
 * \file
 *
 * \brief ATPL360_Host General definitions.
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

#ifndef GENERAL_DEFS_H_INCLUDED
#define GENERAL_DEFS_H_INCLUDED

#include "compiler.h"
#include "conf_atpl360.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* ! Carriers for Cenelec-A bandplan */
#define NUM_CARRIERS_CENELEC_A                     36
/* ! Carriers for FCC bandplan */
#define NUM_CARRIERS_FCC                           72
/* ! Carriers for ARIB bandplan */
#define NUM_CARRIERS_ARIB                          54
/* ! Carriers for Cenelec-B bandplan */
#define NUM_CARRIERS_CENELEC_B                     16

/* ! Subbands for Cenelec-A bandplan */
#define NUM_SUBBANDS_CENELEC_A                     6
/* ! Subbands for FCC bandplan */
#define NUM_SUBBANDS_FCC                           24
/* ! Subbands for ARIB bandplan */
#define NUM_SUBBANDS_ARIB                          18
/* ! Subbands for Cenelec-B bandplan */
#define NUM_SUBBANDS_CENELEC_B                     4

/*! \name ATPL360 work band identifiers
 */
/* ! @{ */
/* ! CENELEC A Band Plan (35 - 91 Khz) */
#define ATPL360_WB_CENELEC_A                       1
/* ! FCC Band Plan (154 - 488 Khz) */
#define ATPL360_WB_FCC                             2
/* ! ARIB Band Plan (154 - 404 Khz) */
#define ATPL360_WB_ARIB                            3
/* ! CENELEC-B Band Plan (98 - 122 Khz) */
#define ATPL360_WB_CENELEC_B                       4
/* ! @} */

/** Bootloader wait time (in ms): Time needed for PL360 FW initialization */
#define ATPL360_BOOT_WAIT_TIME_MS                  3

/** List of parameter identifiers */
typedef enum {
	ATPL360_HOST_DESCRIPTION_ID = 0x0100,
	ATPL360_HOST_MODEL_ID  = 0x010A,
	ATPL360_HOST_PHY_ID = 0x010C,
	ATPL360_HOST_PRODUCT_ID = 0x0110,
	ATPL360_HOST_VERSION_ID = 0x0112,
	ATPL360_HOST_BAND_ID = 0x0116,
	ATPL360_TIME_REF_ID = 0x0200,
	ATPL360_SLEEP_MODE_ID = 0x0201,
	ATPL360_DEBUG_SET_ID = 0x0202,
	ATPL360_DEBUG_READ_ID = 0x0203,
	/* Always has to appear at the end of the table */
	ATPL360_IB_PARAM_END = 6
} atpl360_id_param_t;

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif   /* GENERAL_DEFS_H_INCLUDED */
