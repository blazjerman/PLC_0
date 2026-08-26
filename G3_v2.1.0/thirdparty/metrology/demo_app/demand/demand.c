/**
 * \file
 *
 * \brief Meter Demo : Demand module
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
#include "rtcproc.h"
#include "demand.h"
#include "utils.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

demand_t VDemand;

/**
 * \brief Calculate maxim demand per period.
 * Maximum demand is calculated as the average of the instanteneous power considering a time interval of 15 minutes
 * It's calculated using a fixed time window of 15 minutes, not a sliding window (calculation done each minute).
 * The VDemand includes 60 registers, so the user can modify the code to increase the interval or to use a sliding window
 *
 * \param rateID    Energy rate to get demand value
 */
static void _getMaxDemand(tou_rate_id_t rateID)
{
	demand_calc_t *ptr_dem_calc;
	demand_max_t *ptr_dem_max;
	uint32_t ul_dem_max;
	uint8_t uc_offset, uc_idx;

	/* Set pointer to structure to get calculations */
	if (rateID == TOUALL) {
		ptr_dem_calc = &VDemand.calc_total;
	} else {
		ptr_dem_calc = &VDemand.calc_tou;
	}

	/* VRTC minute is used as the offset of demand calc buffer */
	/* Measurement is completed for the previous minute */
	uc_offset = (VRTC.time.minute == 0)? 59 : (VRTC.time.minute - 1);
	/* New hour --> clear buffer */
	if (uc_offset == 0) {
		for (uc_idx = 0; uc_idx <= 59; uc_idx++) {
			ptr_dem_calc->buff[uc_idx]=0;
		}
	}
	/* Store accumulated demand in the current minute */
	ptr_dem_calc->buff[uc_offset] = ptr_dem_calc->demand_acc;
	/* Restart demand for next period */
	ptr_dem_calc->demand_acc = 0;

	/* Get demand mean value */
	ul_dem_max = 0;
	for (uc_idx = ((uc_offset / 15) *15); uc_idx < (15 + (uc_offset / 15) *15); uc_idx++) {
		ul_dem_max += ptr_dem_calc->buff[uc_idx];
	}
	ul_dem_max /= 9000; /* Units are 0.1W, so divided by 10; additionally, 15 minutes, so divided by 15 */
	LOG_APP_DEMO_DEBUG(("DemandProcess: get demand : offset[%d] ul_dem_max[%u]\r\n",
		  uc_offset, ul_dem_max));

	/* Check month change */
	if (VDemand.clear_month) {
		/* Complete the previous month */
		/* Set pointer to the demand max data of the current month - 1 (index) */
		uc_idx = VRTC.date.month - 2;
	} else {
		/* Set pointer to the demand max data of the current month (index) */
		uc_idx = VRTC.date.month - 1;
	}

	ptr_dem_max = &VDemand.max_month[uc_idx][rateID];

	/* Check Max Demand per minute */
	if (ul_dem_max > ptr_dem_max->max) {
		/* Update max demand per minute */
		ptr_dem_max->max = ul_dem_max;
		/* Set Time stamp for max demand per minute */
		ptr_dem_max->time.day = VRTC.date.day;
		ptr_dem_max->time.hour = VRTC.time.hour;
		ptr_dem_max->time.min = VRTC.time.minute;

		/* Update Demand data to External memory */
		ExtMemWrite(MEM_REG_DEMAND_ID, &VDemand);

		LOG_APP_DEMO_DEBUG(("DemandProcess: Update MAX demand[TOU%d][%02d/%02d %02d:%02d], max[%u]\r\n", rateID + 1,
			  uc_idx + 1, ptr_dem_max->time.day, ptr_dem_max->time.hour, ptr_dem_max->time.min, ul_dem_max));
	}
}

/**
 * \brief Demand initialization.
 */
void DemandInit(void)
{
	/* Read Demand data from External memory */
	ExtMemRead(MEM_REG_DEMAND_ID, &VDemand);

	if (VDemand.calc_tou.demand_acc == 0xFFFFFFFF) {
		/* Init External Memory region */
		memset(&VDemand, 0, sizeof(demand_t));
	} else {
		/* Clear Current month */
		/* DemandClearMonth(VRTC.date.month); */

		/* Clear structures to get new values */
		memset(&VDemand.calc_tou, 0, sizeof(demand_calc_t));
		memset(&VDemand.calc_total, 0, sizeof(demand_calc_t));
		VDemand.clear_month = 0;
	}

	/* Update Demand data to External memory */
	ExtMemWrite(MEM_REG_DEMAND_ID, &VDemand);
}

/**
 * \brief Clear all demmand data.
 */
void DemandClear(void)
{
	/* Clear All demand data */
	memset(&VDemand, 0, sizeof(demand_t));

	/* Update Demand data to External memory */
	ExtMemWrite(MEM_REG_DEMAND_ID, &VDemand);
}

/**
 * \brief Demand initialization.
 *
 * \param month    Month to clear demand data
 */
void DemandClearMonth(uint8_t month)
{
	/* Get month index */
	month--;

	if (month < 12) {
		memset(&VDemand.max_month[month], 0, sizeof(demand_max_t) * (TOUALL + 1));

		LOG_APP_DEMO_DEBUG(("DemandClearMonth[%02d]\r\n", month + 1));
	}
}

/**
 * \brief Demand Process. Get maximum demand.
 *
 * \param val   Value to update demand
 */
void DemandUpdate(uint32_t val)
{
	VDemand.calc_tou.demand_acc += val;
	VDemand.calc_total.demand_acc += val;

	LOG_APP_DEMO_DEBUG(("DemandUpdate: Add new demand[%u], total[%u]\r\n", val, VDemand.calc_tou.demand_acc));
}

/**
 * \brief Demand Process. Get maximum demand. It should be called once per minute.
 */
void DemandProcess(void)
{
	/* Get Demand for the current rate */
	_getMaxDemand(VTou.time_slot[VTou.time_slot_idx].rate_id);
	/* Get Demand for TOTAL rates */
	_getMaxDemand(TOUALL);

	/* Check month change */
	if (VDemand.clear_month) {
		/* Clear month to fill it up with the new demand max values (index) */
		DemandClearMonth(VDemand.clear_month - 1);
		/* Reset flag */
		VDemand.clear_month = 0;
	}

}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
