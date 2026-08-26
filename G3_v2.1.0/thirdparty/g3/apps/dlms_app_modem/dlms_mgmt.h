/**
 * \file
 *
 * \brief DLMS for meter - modem management
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
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
#ifndef __DLMS_MGMT_H__
#define __DLMS_MGMT_H__

#include <stdbool.h>
#include <stdint.h>

/* #define DLMS_MGMT_DEBUG_CONSOLE */

/*** Constants *************************************************************/

/* Timeout for messages in ms */
#define DLMS_TIME_WAIT_RESPONSE               50000
#define OBJECTS_PER_REQUEST                   20 /* MAX_OBJECTS_PER_REQUEST */

#define DLMS_MGMT_VERSION  0x0001
#define DLMS_MGMT_SRC_PORT 0xFF00
#define DLMS_MGMT_DST_PORT 0x0000

void dlms_net_mgmt(void);
void dlms_mgmt_update_1ms(void);
void send_counters(void);
bool dlms_mgmt_decode(uint8_t *ptr_buff, uint8_t len);
void dlms_mgmt_process(void);
void dlms_mgmt_init(void);

#endif /* __DLMS_MGMT_H__ */
