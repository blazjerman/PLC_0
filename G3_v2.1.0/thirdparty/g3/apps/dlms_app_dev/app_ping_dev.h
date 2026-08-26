/**
 * \file
 *
 * \brief Ping application
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

#ifndef APP_PING_DEV_H_INCLUDED
#define APP_PING_DEV_H_INCLUDED

#include "g3_app_config.h"

/* Activate PING Report traces */
#define PING_REPORT_CONSOLE

/* Ping Alarm Configuration */
/* #define DLMS_APP_ENABLE_PING_ALARM */

/* Ping Configuration */
/* Ping Alarm Period For First time after Join to the network*/
#define DLMS_APP_INITIAL_PING_ALARM_TIMER_INTERVAL       1800
/* Ping Alarm Period */
#define DLMS_APP_PING_ALARM_TIMER_INTERVAL       3
/* Ping Alarm Timeout for response */
#define DLMS_APP_PING_ALARM_TIMEOUT              60
/* Ping Alarm Time to live */
#define DLMS_APP_PING_ALARM_TTL                  10
/* Ping Alarm Msg Lenght */
#define DLMS_APP_PING_ALARM_LEN                  1225

void ping_app_timers_update(void);
void ping_app_init(void);
void ping_app_process(void);

#endif /* APP_PING_DEV_H_INCLUDED */
