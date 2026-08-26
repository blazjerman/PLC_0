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

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "string.h"

#include "event.h"
#include "extmem.h"
#include "rtcproc.h"
#include "metrology.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#define EVENT_HOLDING_START_TIME_SEC         10//60
#define EVENT_HOLDING_END_TIME_SEC           10//60

events_t VEvent;

/**
 * \brief Close Open Events.
 *
 * \param id  Event ID
 *
 * \return returns 1 if any event has been modified, otherwise returns 0.
 */
static uint8_t _close_events(event_info_t *ptr_event)
{
	uint8_t uc_result = 0;
	uint8_t uc_idx;

	if (ptr_event->status == EVENT_INVALID) {
		/* Init data */
		memset((uint8_t *)ptr_event, 0, sizeof(event_info_t));
		uc_result = 1;
	} else {
		for (uc_idx = 0; uc_idx < EVENT_LOG_MAX_NUMBER; uc_idx++) {
			if ((ptr_event->datalog[uc_idx].date_start.year != 0) &&
			    (ptr_event->datalog[uc_idx].date_end.year == 0)) {
				ptr_event->datalog[uc_idx].time_end = VRTC.time;
				ptr_event->datalog[uc_idx].date_end = VRTC.date;
				ptr_event->status = NO_EVENT;

				uc_result = 1;
			}
		}
	}
	return uc_result;
}

/**
 * \brief Check Single Event.
 *
 * \param id  Event ID
 *
 * \return returns EVENT_START if an event starts,
 * EVENT_END if sdn event ends, otherwise returns NO_EVENT.
 */
static event_status_t _event_check(event_id_t id)
{
	event_info_t *pEvent = NULL;
	event_data_t *pData;
	uint32_t event_flag = 0;

	switch (id) {
	case SAG_UA_EVENT:
	{
		event_flag = VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VA;
		pEvent = &VEvent.Volt_Sag_A;
	}
	break;

	case SAG_UB_EVENT:
	{
		event_flag = VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VB;
		pEvent = &VEvent.Volt_Sag_B;
	}
	break;

	case SAG_UC_EVENT:
	{
		event_flag = VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VC;
		pEvent = &VEvent.Volt_Sag_C;
	}
	break;

	case POW_UA_EVENT:
	{
		event_flag = VAFE.ST.BIT.pa_dir;
		pEvent = &VEvent.Power_Rev_A;
	}
	break;

	case POW_UB_EVENT:
	{
		event_flag = VAFE.ST.BIT.pb_dir;
		pEvent = &VEvent.Power_Rev_B;
	}
	break;

	case POW_UC_EVENT:
	{
		event_flag = VAFE.ST.BIT.pc_dir;
		pEvent = &VEvent.Power_Rev_C;
	}
	break;
	}

	if (event_flag) {
		/* Start Event: It shoud be maintained for a HOLDING TIME to be registered */
		if (pEvent->status == NO_EVENT) {
			/* Start Holding Start time */
			pEvent->status = EVENT_HOLDING_START;
			pEvent->startnum = EVENT_HOLDING_START_TIME_SEC;
		} else if (pEvent->status == EVENT_HOLDING_START) {
			if (--pEvent->startnum == 0) {
				uint8_t idx;
				/* Register END Event */
				pEvent->status = EVENT_START;
				/* Shift Data log */
				for (idx = EVENT_LOG_MAX_NUMBER - 1; idx > 0; idx--) {
					pEvent->datalog[idx] = pEvent->datalog[idx - 1];
				}
				/* Update Event Data[0]: Start time/date */
				pData = &pEvent->datalog[0];
				pData->counter = pEvent->datalog[1].counter + 1;
				pData->time_start = VRTC.time;
				pData->time_end.hour = 0;
				pData->time_end.minute = 0;
				pData->time_end.second = 0;
				pData->date_start = VRTC.date;
				pData->date_end.year = 0;
				pData->date_end.month = 0;
				pData->date_end.day = 0;
				pData->date_end.week = 0;

				return EVENT_START;
			}
		} else if (pEvent->status == EVENT_HOLDING_END) {
			/* Cancel previous END event during HOLDING time */
			pEvent->status = EVENT_START;
			pEvent->endnum = 0;
		}
	} else {
		/* End Event: It shoud be maintained for EVENT_HOLDING_TIME_SEC sec to be registered */
		if (pEvent->status == EVENT_START) {
			/* Start Holding End time */
			pEvent->status = EVENT_HOLDING_END;
			pEvent->endnum = EVENT_HOLDING_END_TIME_SEC;
		} else if (pEvent->status == EVENT_HOLDING_END) {
			/* Register END Event */
			if (--pEvent->endnum == 0) {
				pEvent->status = NO_EVENT;
				pData = &pEvent->datalog[0];
				/* Update Event Data[0]: End time/date */
				pData->time_end = VRTC.time;
				pData->date_end = VRTC.date;

				return EVENT_END;
			}
		} else if (pEvent->status == EVENT_HOLDING_START) {
			/* Cancel previous START event during HOLDING time */
			pEvent->status = NO_EVENT;
			pEvent->startnum = 0;
		}
	}

	return NO_EVENT;
}

/**
 * \brief Clear Single Event.
 *
 * \param id  Event ID
 */
static void _event_clear(event_id_t id)
{
	event_info_t *pEvent = NULL;

	switch (id) {
	case SAG_UA_EVENT:
	{
		VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VA = 0;
		VAFE.ST.BIT.sag_a = 0;
		pEvent = &VEvent.Volt_Sag_A;
	}
	break;

	case SAG_UB_EVENT:
	{
		VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VB = 0;
		VAFE.ST.BIT.sag_b = 0;
		pEvent = &VEvent.Volt_Sag_B;
	}
	break;

	case SAG_UC_EVENT:
	{
		VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VC = 0;
		VAFE.ST.BIT.sag_c = 0;
		pEvent = &VEvent.Volt_Sag_C;
	}
	break;

	case POW_UA_EVENT:
	{
		VAFE.ST.BIT.pa_dir = 0;
		VAFE.ST.BIT.pa_rev = 0;
		pEvent = &VEvent.Power_Rev_A;
	}
	break;

	case POW_UB_EVENT:
	{
		VAFE.ST.BIT.pb_dir = 0;
		VAFE.ST.BIT.pb_rev = 0;
		pEvent = &VEvent.Power_Rev_B;
	}
	break;

	case POW_UC_EVENT:
	{
		VAFE.ST.BIT.pc_dir = 0;
		VAFE.ST.BIT.pc_rev = 0;
		pEvent = &VEvent.Power_Rev_C;
	}
	break;
	}

	if (pEvent) {
		/* Clear all data */
		memset(pEvent, 0, sizeof(event_info_t));
	}
}

/**
 * \brief Init Events.
 *
 * \note External memory must have been previously initialized.
 */
void EventInit(void)
{
	uint8_t mem_update = 0;

	/* Read Event data from External memory */
	ExtMemRead(MEM_REG_EVENTS_ID, &VEvent);

	/* Check Data integrity */
	if (VEvent.Power_Rev_C.datalog[EVENT_LOG_MAX_NUMBER- 1].time_end.second == 0xFFFFFFFF) {
		memset(&VEvent, 0, sizeof(VEvent));
		mem_update = 1;
	} else {
		/* Close Open Events */
		mem_update = _close_events(&VEvent.Volt_Sag_A);
		mem_update |= _close_events(&VEvent.Volt_Sag_B);
		mem_update |= _close_events(&VEvent.Volt_Sag_C);
		mem_update |= _close_events(&VEvent.Power_Rev_A);
		mem_update |= _close_events(&VEvent.Power_Rev_B);
		mem_update |= _close_events(&VEvent.Power_Rev_C);
	}

	if (mem_update) {
		/* Update Event data to External memory */
		ExtMemWrite(MEM_REG_EVENTS_ID, &VEvent);
	}
}

/**
 * \brief Check All events.
 */
void EventProcess(void)
{
	uint32_t ul_res;
	bool b_memory_upd = false;

	ul_res = _event_check(SAG_UA_EVENT);
	if (ul_res == EVENT_START) {
		/* A phase voltage sag event is start */
		VAFE.ST.BIT.sag_a = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* A phase voltage sag event is end */
		VAFE.ST.BIT.sag_a = 0;
		b_memory_upd = true;
	}

	ul_res = _event_check(SAG_UB_EVENT);
	if (ul_res == EVENT_START) {
	 	/* B phase voltage sag event is start */
		VAFE.ST.BIT.sag_b = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* B phase voltage sag event is end */
		VAFE.ST.BIT.sag_b = 0;
		b_memory_upd = true;
	}

	ul_res = _event_check(SAG_UC_EVENT);
	if (ul_res == EVENT_START) {
		/* C phase voltage sag event is start */
		VAFE.ST.BIT.sag_c = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* C phase voltage sag event is end */
		VAFE.ST.BIT.sag_c = 0;
		b_memory_upd = true;
	}

	ul_res = _event_check(POW_UA_EVENT);
	if (ul_res == EVENT_START) {
		/* A phase power reverse event is start */
		VAFE.ST.BIT.pa_rev = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* A phase power reverse event is end */
		VAFE.ST.BIT.pa_rev = 0;
		b_memory_upd = true;
	}

	ul_res = _event_check(POW_UB_EVENT);
	if (ul_res == EVENT_START) {
		/* B phase power reverse event is start */
		VAFE.ST.BIT.pb_rev = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* B phase power reverse event is end */
		VAFE.ST.BIT.pb_rev = 0;
		b_memory_upd = true;
	}

	ul_res = _event_check(POW_UC_EVENT);
	if (ul_res == EVENT_START) {
		/* C phase power reverse event is start */
		VAFE.ST.BIT.pc_rev = 1;
		b_memory_upd = true;
	} else if (ul_res == EVENT_END) {
		/* C phase power reverse event is end */
		VAFE.ST.BIT.pc_rev = 0;
		b_memory_upd = true;
	}

	if (b_memory_upd) {
		/* Update Event data to External memory */
		ExtMemWrite(MEM_REG_EVENTS_ID, &VEvent);
	}
}

/**
 * \brief Clear All events.
 */
void EventClear(void)
{
	_event_clear(SAG_UA_EVENT);
	_event_clear(SAG_UB_EVENT);
	_event_clear(SAG_UC_EVENT);
	_event_clear(POW_UA_EVENT);
	_event_clear(POW_UB_EVENT);
	_event_clear(POW_UC_EVENT);

	/* Update Event data to External memory */
	ExtMemWrite(MEM_REG_EVENTS_ID, &VEvent);
}

uint8_t EventGetData(event_data_t *ev_data, event_id_t id, uint8_t last_num)
{
	if ((last_num == 0) || (last_num > EVENT_LOG_MAX_NUMBER)) {
		return 0;
	}

	switch (id) {
	case SAG_UA_EVENT:
	{
		*ev_data = VEvent.Volt_Sag_A.datalog[last_num - 1];
	}
	break;

	case SAG_UB_EVENT:
	{
		*ev_data = VEvent.Volt_Sag_B.datalog[last_num - 1];
	}
	break;

	case SAG_UC_EVENT:
	{
		*ev_data = VEvent.Volt_Sag_C.datalog[last_num - 1];
	}
	break;

	case POW_UA_EVENT:
	{
		*ev_data = VEvent.Power_Rev_A.datalog[last_num - 1];
	}
	break;

	case POW_UB_EVENT:
	{
		*ev_data = VEvent.Power_Rev_B.datalog[last_num - 1];
	}
	break;

	case POW_UC_EVENT:
	{
		*ev_data = VEvent.Power_Rev_C.datalog[last_num - 1];
	}
	break;

	default:
		return 0;
	}

	return 1;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
