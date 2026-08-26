/**
 * \file
 *
 * \brief RTC Process Handler
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
#include "conf_demo_app.h"
#include "rtc.h"
#include "rtcproc.h"
#include "task.h"
#include "display.h"
#include "event.h"
#include "tou.h"
#include "energy.h"
#include "history.h"
#include "demand.h"
#include "utils.h"
#include "command.h"
#include "extmem.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

rtc_t VRTC;

const uint8_t Month_DayTable[12] = {0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31};
const char Months_DateTable[12][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const uint8_t Day_weekTableD[12] = {6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

static uint8_t suc_rtc_save_min_counter = RTC_MIN_STORE_BACKUP;

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

static bool _rtc_value_is_later (rtc_t * rtc_value)
{
	rtc_t reg_value;

	rtc_get_time(RTC, &reg_value.time.hour, &reg_value.time.minute, &reg_value.time.second);
	rtc_get_date(RTC, &reg_value.date.year, &reg_value.date.month, &reg_value.date.day, &reg_value.date.week);

	if (rtc_value->date.year < reg_value.date.year) {
		return false;
	}

	if (rtc_value->date.month < reg_value.date.month) {
		return false;
	}

	if (rtc_value->date.day < reg_value.date.day) {
		return false;
	}

	if (rtc_value->time.hour < reg_value.time.hour) {
		return false;
	}

	if (rtc_value->time.minute < reg_value.time.minute) {
		return false;
	}

	if (rtc_value->time.second < reg_value.time.second) {
		return false;
	}

	return true;

}

/**
 * \brief Interrupt handler for the RTC.
 *
 * \note Second marker for taking data from DSP.
 * \note Minute marker for accumulating power measurements.
 * \note 15 Minutes marker for accumulating power measurements.
 * \note Time change marker for accumulating power measurements.
 */
void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	if ((ul_status & RTC_SR_TIMEV) == RTC_SR_TIMEV) {
		uint32_t new_day, new_month;

		/** 1 Interrupt per HOUR **/
		rtc_disable_interrupt(RTC, RTC_IDR_TIMDIS);
		rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
		rtc_enable_interrupt(RTC, RTC_IER_TIMEN);

		/* Checking the day changing to handle history data */
		rtc_get_date(RTC, 0, 0, &new_day, 0);
		if (new_day != VRTC.date.day) {
			/* Checking the month changing to handle history data */
			rtc_get_date(RTC, 0, &new_month, 0, 0);
			if (new_month != VRTC.date.month) {
				VHistory.status.clear_month = (uint8_t)new_month;
				VDemand.clear_month = (uint8_t)new_month;
			} else {
				VHistory.status.clear_day = (uint8_t)new_day;
			}

			/* Update VRTC date */
			rtc_get_date(RTC, &VRTC.date.year, &VRTC.date.month, &VRTC.date.day, &VRTC.date.week);

			/* Launch History Task */
			VHistory.status.upd_energy = 1;
			TaskPutIntoQueue(HistoryProcess);

		}

		/** Launch Tasks per HOUR **/

		LOG_APP_DEMO_DEBUG(("RTC_Handler: RTC_SR_TIMEV[%d]\r\n", (uint8_t)VRTC.time.hour));

	} else if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		uint32_t new_minute;

		/** 1 Interrupt per MINUTE **/
		/* Disable RTC interrupt */
		rtc_disable_interrupt(RTC, RTC_IDR_ALRDIS);
		/* Clear Status */
		rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
		/* Enable RTC interrupt */
		rtc_enable_interrupt(RTC, RTC_IER_ALREN);

		/** Launch Tasks per MINUTE **/
		/* Check Maximum Demand values */
		TaskPutIntoQueue(DemandProcess);
		/* Check Time Slot for the next Energy period. Launch after Energy Process */
		TaskPutIntoQueue(TOUProcess);

		/* Check if there are previous History Updates (Originated by day/month changes) */
		if (VHistory.status.upd_energy == 0) {
			/* Check TIME History Update period : Update Energy in History */
			rtc_get_time(RTC, 0, &new_minute, 0);
			if ((new_minute % CONF_APP_TIME_HISTORY_UPD) == 0) {
				VHistory.status.upd_energy = 1;
				/* Launch History Task */
				TaskPutIntoQueue(HistoryProcess);
			}
		}

		/* Save RTC value in external memory */
		if (--suc_rtc_save_min_counter == 0) {
			suc_rtc_save_min_counter = RTC_MIN_STORE_BACKUP;
			TaskPutIntoQueue(RTCProcWriteInExtMem);
		}

		LOG_APP_DEMO_DEBUG(("RTC_Handler: RTC_SR_ALARM[%d]\r\n", (uint8_t)VRTC.time.minute));

	} else if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {

		/** 1 Interrupt per SECOND **/
		rtc_disable_interrupt(RTC, RTC_IDR_SECDIS);
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
		rtc_enable_interrupt(RTC, RTC_IER_SECEN);

		/* Update RTC timestamp */
		rtc_get_time(RTC, &VRTC.time.hour, &VRTC.time.minute, &VRTC.time.second);

		/* Update RTC datestamp if it is uninitialized */
		if (VRTC.date.year == 0) {
			rtc_get_date(RTC, &VRTC.date.year, &VRTC.date.month,
			     &VRTC.date.day, &VRTC.date.week);
		}

                /* Set Energy flag to store energy in external memory in next EnergyProcess call */
		VEnergyCtrl.mem_update = MEM_UPDATE_PERIODICAL;

		/* Update Demand values */
		DemandUpdate(VAFE.RMS[Pt]);

		/** Launch Tasks per SECOND **/
		TaskPutIntoQueue(EventProcess);
		if (VCom.lamptimer) {
			VCom.lamptimer--;
		}

                if (--VDisplay.timer == 0) {
                        TaskPutIntoQueue(DisplayChangeInfo);
                } else {
                        TaskPutIntoQueue(DisplayProcess);
                }

		LOG_APP_DEMO_DEBUG(("RTC_Handler: RTC_SR_SEC[%02d/%02d %02d:%02d:%02d]\r\n",
			(uint8_t)VRTC.date.month, (uint8_t)VRTC.date.day,
			(uint8_t)VRTC.time.hour, (uint8_t)VRTC.time.minute, (uint8_t)VRTC.time.second));

	} else {
		rtc_clear_status(RTC, RTC_SCCR_ACKCLR | RTC_SCCR_TDERRCLR | RTC_SCCR_CALCLR);
	}
}

/**
 * \brief Init RTC process handler.
 */
void RTCProcInit(void)
{
	char puc_buff[15];
	uint16_t us_year, y, d;
	uint8_t uc_hr, uc_min, uc_sec, uc_mon, uc_day, uc_week;
	uint8_t uc_idx;
	uint8_t a, m;
	bool upd_extmem = false;

	/* Disable RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);

	/* Read TOU data from External memory */
	ExtMemRead(MEM_REG_RTC_ID, &VRTC);

	if (VRTC.key != RTC_KEY_MEM) {
		// idata is invalid

		/* Default RTC configuration, 24-hour mode */
		rtc_set_hour_mode(RTC, 0);

		/* Set Last Compilation time */
		sprintf(puc_buff, "%s", __TIME__);
		uc_hr = (puc_buff[0] - 0x30) * 10 + (puc_buff[1] - 0x30);
		uc_min = (puc_buff[3] - 0x30) * 10 + (puc_buff[4] - 0x30);
		uc_sec = (puc_buff[6] - 0x30) * 10 + (puc_buff[7] - 0x30);

		// Update RTC peripheral
		rtc_set_time(RTC, uc_hr, uc_min, uc_sec);

		/* Set Compiler date */
		sprintf(puc_buff, "%s", __DATE__);
		uc_mon = 0;
		for (uc_idx = 0; uc_idx < 12; uc_idx++) {
			if (memcmp(&Months_DateTable[uc_idx], puc_buff, 3) == 0) {
				uc_mon = uc_idx + 1;
				break;
			}
		}

		us_year = (puc_buff[7] - 0x30) * 1000 + (puc_buff[8] - 0x30) * 100 +
			(puc_buff[9] - 0x30) * 10 + (puc_buff[10] - 0x30);
		uc_day = puc_buff[4] == 0x20 ? 0 : (puc_buff[4] - 0x30) * 10;
		uc_day += (puc_buff[5] - 0x30);

		/* Get day of the week */
		a = (14 - uc_mon) / 12;
		y = us_year - ((14 - uc_mon) / 12);
		m = uc_mon + (12 * a) - 2;
		d = uc_day + y + y/4 - y/100 + y/400 + ((31*m)/12);
		uc_week = (d % 7);
		if (uc_week == 0) {
			uc_week = 7;
		}

		// Update RTC peripheral
		rtc_set_date(RTC, us_year, uc_mon, uc_day, uc_week);

		upd_extmem = true;

	} else {
		// data from external memory is valid
		if (_rtc_value_is_later(&VRTC)) {
			// VRTC value from memory is later than the current RTC value
			// Update RTC peripheral
			rtc_set_time(RTC, VRTC.time.hour, VRTC.time.minute, VRTC.time.second);
			rtc_set_date(RTC, VRTC.date.year, VRTC.date.month, VRTC.date.day, VRTC.date.week);
		} else {
			// VRTC value from memory is before than the current RTC value
			upd_extmem = true;
		}
	}

	/* Set ALARM each minute */
	rtc_set_time_alarm(RTC, 0, 0, 0, 0, 1, 0);

	/* Set TIME EVENT each hour */
	rtc_set_time_event(RTC, 1);

	/* Update RTC timestamp */
	VRTC.key = RTC_KEY_MEM;
	rtc_get_time(RTC, &VRTC.time.hour, &VRTC.time.minute, &VRTC.time.second);
	rtc_get_date(RTC, &VRTC.date.year, &VRTC.date.month, &VRTC.date.day, &VRTC.date.week);

	if (upd_extmem) {
		ExtMemWrite(MEM_REG_RTC_ID, &VRTC);
	}

	/* Configure RTC interrupts */
	rtc_enable_interrupt(RTC, RTC_IER_SECEN | RTC_IER_ALREN | RTC_IER_TIMEN);
	NVIC_EnableIRQ(RTC_IRQn);
}

/**
 * \brief Set TIME data in BCD format.
 *
 * \param bcd_hr   Hour in BCD format.
 * \param bcd_min  Minute in BCD format.
 * \param bcd_sec  Second in BCD format.
 *
 * \return if failure return 1 else 0.
 */
uint8_t RTCProcSetTimeBCD(uint8_t bcd_hr, uint8_t bcd_min, uint8_t bcd_sec)
{
	uint8_t hr, min, sec;

	/* Get Deciaml values */
	hr = (bcd_hr >> 4) * 10 + (bcd_hr & 0x0f);
	min = (bcd_min >> 4) * 10 + (bcd_min & 0x0f);
	sec = (bcd_sec >> 4) * 10 + (bcd_sec & 0x0f);

	/* Check hours */
	if (hr > 23) {
		return 1;
	}

	/* Check minutes */
	if (min > 59) {
		return 1;
	}

	/* Check seconds */
	if (sec > 59) {
		return 1;
	}

	rtc_set_time(RTC, hr, min, sec);

	return 0;
}

/**
 * \brief Set DATE data in BCD format.
 *
 * \param bcd_year   Year in BCD format.
 * \param bcd_month  Month in BCD format.
 * \param bcd_day    Day in BCD format.
 * \param bcd_week   Day of the Week in BCD format.
 *
 * \return if failure return 1 else 0.
 */
uint8_t RTCProcSetDateBCD(uint8_t bcd_year, uint8_t bcd_month, uint8_t bcd_day, uint8_t bcd_day_week)
{
	uint8_t year, month, day, week;
	uint8_t max_day;

	/* Get Decimal Values */
	year = 2000 + (bcd_year >> 4) * 10 + (bcd_year & 0x0f);
	month = (bcd_month >> 4) * 10 + (bcd_month & 0x0f);
	day = (bcd_day >> 4) * 10 + (bcd_day & 0x0f);
	week = (bcd_day_week >> 4) * 10 + (bcd_day_week & 0x0f);

	/* Check month */
	if (month > 12) {
		return 1;
	}

	/* Check day of the month */
	if (month == 1) {
		max_day = (year % 4) ? Month_DayTable[1] : Month_DayTable[1] + 1;
	} else {
		max_day = Month_DayTable[month - 1];
	}

	if (bcd_day > max_day) {
		return 1;
	}

	/* Check day of the week */
	if (bcd_day_week > 7) {
		return 1;
	}

	rtc_set_date(RTC, year, month, day, week);

	return 0;
}

/**
 * \brief Get number of days of the passed month.
 *
 * \param year   Year in decimal format.
 * \param month  Month in decimal format.
 *
 * \return The last Day of the month in decimal format. 0 in case of error in arguments.
 */
uint8_t RTCProcGetDaysOfMonth(uint8_t year, uint8_t month)
{
	uint8_t last_day;

	/* get month index */
	month--;

	/* Check month */
	if (month > 11) {
		return 0;
	}

	/* Check day of the month (index) [Feb = 1] */
	if (month == 1) {
		last_day = (year % 4) ? Month_DayTable[1] : Month_DayTable[1] + 1;
	} else {
		last_day = Month_DayTable[month];
	}

	return (last_day >> 4) * 10 + (last_day & 0x0f);
}

/**
 * \brief Set new RTC data, including time and date information.
 *
 * \param new_rtc   Pointer to new RTC data.
 *
 * \return if failure return 1 else 0.
 */
uint8_t RTCProcSetNewRTC(rtc_t* new_rtc)
{
	uint8_t max_day;

	/* Check month */
	if (new_rtc->date.month > 12) {
		return 1;
	}

	/* Check day of the month */
	if (new_rtc->date.month == 1) {
		max_day = (new_rtc->date.year % 4) ? Month_DayTable[1] : Month_DayTable[1] + 1;
	} else {
		max_day = Month_DayTable[new_rtc->date.month - 1];
	}

	if (new_rtc->date.day > max_day) {
		return 1;
	}

	/* Check day of the week */
	if (new_rtc->date.week > 7) {
		return 1;
	}

	/* Check hours */
	if (new_rtc->time.hour > 23) {
		return 1;
	}

	/* Check minutes */
	if (new_rtc->time.minute > 59) {
		return 1;
	}

	/* Check seconds */
	if (new_rtc->time.second > 59) {
		return 1;
	}

	rtc_set_time(RTC, new_rtc->time.hour, new_rtc->time.minute, new_rtc->time.second);
	rtc_set_date(RTC, new_rtc->date.year, new_rtc->date.month, new_rtc->date.day, new_rtc->date.week);

	/* Update global RTC data */
	VRTC = *new_rtc;
	VRTC.key = RTC_KEY_MEM;

	return 0;
}

/**
 * \brief Write VRTC data in external memory.
 */
void RTCProcWriteInExtMem(void)
{
	rtc_t rtc_value;

	rtc_disable_interrupt(RTC, RTC_IDR_SECDIS | RTC_IDR_ALRDIS | RTC_IDR_TIMDIS);
	memcpy(&rtc_value, &VRTC, sizeof(rtc_t));
	rtc_enable_interrupt(RTC, RTC_IER_SECEN | RTC_IER_ALREN | RTC_IER_TIMEN);

	ExtMemWrite(MEM_REG_RTC_ID, &rtc_value);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
