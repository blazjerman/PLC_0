/**
 * \file
 *
 * \brief Dispatcher application to manage data from/to socket & PLC interface
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

#ifndef APP_DISPATCHER_H_INCLUDED
#define APP_DISPATCHER_H_INCLUDED

#include "g3_app_config.h"

/* Activate DLMS debug */
/* #define DLMS_DEBUG_CONSOLE */
/* Activate packet dump */
/* #define DUMP_CONSOLE */

/* Activate serial debug */
#define SERIAL_DEBUG_CONSOLE

/* Maximum length of IPv6 PDUs for DLMS application */
#define MAX_LENGTH_IPv6_PDU 1200

/*** Structs *************************************************************/
struct dlms_msg {
	uint8_t todo;    /* Flag to indicate that msg has to be served */
	uint8_t buf[MAX_LENGTH_IPv6_PDU]; /* Serial data buffer */
	uint16_t length; /* Length of the data buffer */
};

void dispatcher_get_meter_id(uint8_t *pui_meter_id);

void dispatcher_timers_update(void);

void dispatcher_app_init(void);
void dispatcher_app_process(void);
#endif /* APP_DISPATCHER_H_INCLUDED */
