/**
 * \file
 *
 * \brief Simple protocol for meter - modem management
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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
#ifndef __SIMPLE_MGMT_H__
#define __SIMPLE_MGMT_H__

#include <stdbool.h>
#include <stdint.h>

/* #define SIMPLE_MGMT_DEBUG_CONSOLE */

/*** Constants *************************************************************/

#define SIMPLE_MGMT_FRAME_HEADER_0      0xFC
#define SIMPLE_MGMT_FRAME_HEADER_1      0xAA
#define SIMPLE_MGMT_FRAME_HEADER_2      0xFC
#define SIMPLE_MGMT_READ                                                0x11
#define SIMPLE_MGMT_CONTROL_CODE                0x91
#define SIMPLE_MGMT_FRAME_END                   0xFB

#define SIMPLE_MGMT_DATA_METER_ID_0 0xC0
#define SIMPLE_MGMT_DATA_METER_ID_1 0x32

uint8_t simple_mgmt_send_meter_id_req(void);
bool simple_mgmt_decode(uint8_t *ptr_buff, uint8_t len);

#endif /* __SIMPLE_MGMT_H__ */
