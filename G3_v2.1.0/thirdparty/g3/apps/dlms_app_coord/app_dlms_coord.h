/**
 * \file
 *
 * \brief DLMS_APP : DLMS example application for ATMEL G3 Coordinator
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

#ifndef DLMS_APP_COORD_H_INCLUDED
#define DLMS_APP_COORD_H_INCLUDED

#include "conf_project.h"
#include "conf_oss.h"
#include "conf_bs.h"
#include "AdpApi.h"

/* --- DEFINE APP BEHAVIOUR --- */
/* Adapt DLMS APP start cycles dynamically to network stability. Always before the TIMER_WAITING_START has expired */
#define DLMS_APP_WAIT_REG_NODES

/**************** App feature habilitation ************************/

/**** Enable DLMS APP path request. PREQ will request to nodes always before launch next cycle ****/
/* #define DLMS_APP_ENABLE_PATH_REQ */

/* Activate DLMS Report traces */
#define DLMS_REPORT_CONSOLE

/* Activate DLMS debug */
/* #define DLMS_DEBUG_CONSOLE */

/* --- DEFINE APP LENGTHS --- */
/* Maximum length of IPv6 PDUs for DLMS application */
#define MAX_LENGTH_IPv6_PDU                   1200

/* IPv6 header length */
#define IPv6_HEADER_LENGTH                    40

/* UDP header length */
#define UDP_HEADER_LENGTH                     8

/* G3 Coordinator PAN defined by Bootstrap module */
#define DLMS_G3_COORD_PAN_ID                  G3_COORDINATOR_PAN_ID

/* --- DEFINE APP TIMERS --- */
/* Time between cycles in ms */
#define DLMS_TIME_BETWEEEN_CYCLES             20000

/* Timeout for messages in ms */
#define DLMS_TIME_WAIT_RESPONSE               50000

/* Time between messages in ms */
#define TIMER_BETWEEN_MESSAGES                600

/* Time to wait before start cycling in ms (approx. 60 sec per device) */
#define DLMS_TIME_WAITING_IDLE                120000 /* 2 minutes */

/* Time max PATH REQ cfm in ms (only in case of uncomment DLMS_APP_ENABLE_PATH_REQ) */
#define TIME_MAX_WAIT_PREQ_CFM                120000 /* 2 min */

/* Time max Path request process in ms (only in case of uncomment DLMS_APP_ENABLE_PATH_REQ) */
#define TIME_MAX_BETWEEN_PREQ_WITOUT_DATA     60000 /* 1 min */

/* Extended address length */
#define EXT_ADDR_LEN     8 /* bytes */

typedef struct x_node_list {
	uint8_t puc_extended_address[EXT_ADDR_LEN];
	uint16_t us_short_address;
} x_node_list_t;

void dlms_app_init(void);
void dlms_app_update_1ms(void);
void dlms_app_process(void);
void dlms_app_join_node(uint8_t *puc_extended_address, uint16_t us_short_address);
void dlms_app_leave_node(uint16_t us_short_address);
void dlms_app_path_node_cfm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm);

#endif /* DLMS_APP_COORD_H_INCLUDED */
