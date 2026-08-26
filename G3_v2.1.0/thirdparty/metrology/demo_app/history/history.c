/**
 * \file
 *
 * \brief Meter Demo : History module
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
#include "extmem.h"
#include "history.h"
#include "energy.h"
#include "rtcproc.h"
#include "utils.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

history_t VHistory;

/**
 * \brief History initialize.
 */
void HistoryInit(void)
{
	uint32_t temp;

	/* Check if external memory has been written previously at least once */
	ExtMemReadSize(MEM_REG_HISTORY_ID, &temp, sizeof(temp));

	if (temp == 0xFFFFFFFF) {
		/* Init External Memory region */
		memset(&VHistory, 0, sizeof(history_t));
		/* Update History data to External memory */
		ExtMemWrite(MEM_REG_HISTORY_ID, &VHistory);
	} else {
		/* Update History data from External memory */
		ExtMemRead(MEM_REG_HISTORY_ID, &VHistory);
		/* Restore energy data from memory data */
		VHistory.status.clear_day = 0;
		VHistory.status.clear_month = 0;
		VHistory.status.upd_energy = 0;
	}
}

/**
 * \brief History Data Save process.
 */
void HistoryProcess(void)
{
	if (VHistory.status.upd_energy) {
		uint8_t uc_month_idx, uc_day_idx;
		bool clr_energy_data = false;

		if (VHistory.status.clear_day) {
			/* Select the current month */
			uc_month_idx = VRTC.date.month - 1;
			/* Select 1 day before the day to be cleared */
			uc_day_idx = VHistory.status.clear_day - 2;
			/* Set flag to clear energy : Change to new day */
			clr_energy_data = true;
		} else if (VHistory.status.clear_month) {
			/* Select the previous month */
			if (VHistory.status.clear_month == 1) {
				/* Select december: 12 - 1 (index) */
				uc_month_idx = 11;
				/* Select the last day of december: 31 - 1 (index) */
				uc_day_idx = 30;
			} else {
				/* Select 1 month before the month to be cleared (index) */
				uc_month_idx = VHistory.status.clear_month - 2;
				/* Select the last day of the previous month (index) */
				uc_day_idx = RTCProcGetDaysOfMonth(VRTC.date.year, uc_month_idx + 1) - 1;
			}
			/* Set flag to clear energy : Change to new month */
			clr_energy_data = true;
		} else {
			/* Select the current month */
			uc_month_idx = VRTC.date.month - 1;
			/* Select the current day */
			uc_day_idx = VRTC.date.day - 1;
		}

		/* Update History data : Take care with indexes of month/day */
		VHistory.energy[uc_month_idx][uc_day_idx] = VEnergy;

		/* Clear Energy data if needed */
		if (clr_energy_data) {
			memset(&VEnergy, 0, sizeof(VEnergy));
		}

		/* Update History data to External memory */
		ExtMemWrite(MEM_REG_HISTORY_ID, &VHistory);

		/* Clear flag */
		VHistory.status.upd_energy = 0;

		LOG_APP_DEMO_DEBUG(("History Process: Update Energy[%d][%d]\r\n",
			  uc_day_idx + 1, uc_month_idx + 1));
	}

	if (VHistory.status.clear_day) {
		/* Clear day of the current month */
		HistoryClearDay(VRTC.date.month, VHistory.status.clear_day);
		/* Clear flag */
		VHistory.status.clear_day = 0;
	}

	if (VHistory.status.clear_month) {
		/* Clear Month */
		HistoryClearMonth(VHistory.status.clear_month);
		/* Clear flag */
		VHistory.status.clear_month = 0;
	}
}

/**
 * \brief Clear All History data.
 */
void HistoryClearAll(void)
{
	/* Clear History data */
	memset(&VHistory, 0, sizeof(VHistory));

	/* Update History data to External memory */
	ExtMemWrite(MEM_REG_HISTORY_ID, &VHistory);
}

/**
 * \brief Clear History data for a specific month
 *
 * \param uc_month   Month [1-12] to clear history data
 */
void HistoryClearMonth(uint8_t uc_month)
{
	/* get month index */
	uc_month--;

	if (uc_month < 12) {
		/* Clear History data */
		memset(&VHistory.energy[uc_month], 0, sizeof(energy_t) * HISTORY_DAYS);

		/* Update History data to External memory */
		ExtMemWrite(MEM_REG_HISTORY_ID, &VHistory);

		LOG_APP_DEMO_DEBUG(("History Process: Clear Month[%d]\r\n", uc_month + 1));
	}
}

/**
 * \brief Clear History data for a specific day of the current month
 *
 * \param uc_month   Month [1-12] to clear history data
 * \param uc_day     Day [1-31] to clear history data
 */
void HistoryClearDay(uint8_t uc_month, uint8_t uc_day)
{
	/* get day index */
	uc_day--;
	/* get month index */
	uc_month--;

	if (uc_day < 31) {
		/* Clear History data : Take care with indexes of month */
		memset(&VHistory.energy[uc_month][uc_day], 0, sizeof(energy_t));

		/* Update History data to External memory */
		ExtMemWrite(MEM_REG_HISTORY_ID, &VHistory);

		LOG_APP_DEMO_DEBUG(("History Process: Clear Day[%d][%d]\r\n",
			  uc_day + 1, uc_month + 1));
	}
}

/**
 * \brief Get Energy data for a specific month
 *
 * \param uc_month   Month [1-12] to read history data
 */
void HistoryGetEnergyMonth(energy_t *ptr_energy, uint8_t uc_month)
{
	uint8_t day_idx, day_max;

	/* Get number of days of the month */
	day_max = RTCProcGetDaysOfMonth(VRTC.date.year, uc_month);

	/* get month index */
	uc_month--;

	/* Init ptr_energy */
	ptr_energy->tou1_acc = 0;
	ptr_energy->tou2_acc = 0;
	ptr_energy->tou3_acc = 0;
	ptr_energy->tou4_acc = 0;

	if (uc_month < 12) {
		for (day_idx = 0; day_idx < day_max; day_idx++) {
			ptr_energy->tou1_acc += VHistory.energy[uc_month][day_idx].tou1_acc;
			ptr_energy->tou2_acc += VHistory.energy[uc_month][day_idx].tou2_acc;
			ptr_energy->tou3_acc += VHistory.energy[uc_month][day_idx].tou3_acc;
			ptr_energy->tou4_acc += VHistory.energy[uc_month][day_idx].tou4_acc;
		}
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
