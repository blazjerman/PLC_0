/**
 * \file
 *
 * \brief Meter Demo : Event module
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

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include "compiler.h"
#include "rtcproc.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */
	
#define EVENT_LOG_MAX_NUMBER         10
	
typedef enum {
	NO_EVENT = 0,
	EVENT_START,
	EVENT_END, 
	EVENT_HOLDING_START,
	EVENT_HOLDING_END,
	EVENT_INVALID = 0xFF
} event_status_t;

typedef enum {
	SAG_UA_EVENT = 1,
	SAG_UB_EVENT = 2,
	SAG_UC_EVENT = 3,
	POW_UA_EVENT = 5,
	POW_UB_EVENT = 6,
	POW_UC_EVENT = 7
} event_id_t;

typedef struct {
	uint16_t counter;
	rtc_time_t time_start;
	rtc_time_t time_end;
	rtc_date_t date_start;
	rtc_date_t date_end;
} event_data_t;

typedef struct {
	uint8_t startnum;
	uint8_t endnum;
	event_status_t status;
	event_data_t datalog[EVENT_LOG_MAX_NUMBER];
} event_info_t;

typedef struct {
	event_info_t Volt_Sag_A;
	event_info_t Volt_Sag_B;
	event_info_t Volt_Sag_C;
	event_info_t Power_Rev_A;
	event_info_t Power_Rev_B;
	event_info_t Power_Rev_C;
} events_t;

extern events_t VEvent;

void EventInit(void);
void EventProcess(void);
void EventClear(void);
uint8_t EventGetData(event_data_t *ev_data, event_id_t id, uint8_t last_num);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* EVENT_H_INCLUDED */
