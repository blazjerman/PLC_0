/**
 * \file
 *
 * \brief TOU Module file.
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

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include <string.h>
#include "compiler.h"
#include "rtcproc.h"
#include "tou.h"
#include "extmem.h"
#include "demand.h"
#include "utils.h"

tou_t VTou;

static tou_t VTouDefault = {
	3,
	{{TOU_RATE_2, 8, 30},
	{TOU_RATE_3, 10, 30},
	{TOU_RATE_1, 18, 30},
	{TOU_RATE_4, 22, 00},
	{INVALID, 0, 0},
	{INVALID, 0, 0},
	{INVALID, 0, 0},
	{INVALID, 0, 0}},
	4
};

/**
 * \brief Time of Use init.
 */
void TOUInit(void)
{
	/* Init Time of Use data */
	/* Read TOU data from External memory */
	ExtMemRead(MEM_REG_TOU_ID, &VTou);

	if (VTou.time_slot_idx == 0xFF) {
		/* Init External Memory VTOU Data */
		VTou = VTouDefault;
		/* Write TOU data to External memory */
		ExtMemWrite(MEM_REG_TOU_ID, &VTou);
	}

	/* Execute first TOU Process to set TOU time slot */
	TOUProcess();
}

/**
 * \brief Time of Use processing.
 */
void TOUProcess(void)
{
	uint8_t i;

	/* Find the Next Time Slot */
	for (i = 0; i < VTou.num; i++) {
		if (VRTC.time.hour < VTou.time_slot[i].hour ||
		   (VRTC.time.hour == VTou.time_slot[i].hour && VRTC.time.minute < VTou.time_slot[i].minute)) {
			break;
		}
	}

	/* Select the previous one */
	if (i == 0) {
		i = VTou.num - 1;
	} else {
		i--;
	}

	/* Check TOU time slot selection */
	if (VTou.time_slot_idx != i) {
		VTou.time_slot_idx = i;
		/* Clear demand buffers */
		memset(&VDemand.calc_tou, 0, sizeof(demand_calc_t));
		memset(&VDemand.calc_total, 0, sizeof(demand_calc_t));
		/* Write TOU data to External memory */
		ExtMemWrite(MEM_REG_TOU_ID, &VTou);

		LOG_APP_DEMO_DEBUG(("TOU Process: Change Time Slot[%d]: TOU%d\r\n", i, VTou.time_slot[i].rate_id + 1));
	} else {
		LOG_APP_DEMO_DEBUG(("TOU Process: Nothing to do\r\n"));
	}
}

/**
 * \brief Time of Use init.
 *
 * \param rateID    Energy rate to get demand value
 * \param Time slot index, INVALID in case of rateID is not found.
 */
uint8_t TOUGetTimeSlotIdx(tou_rate_id_t rateID)
{
	uint8_t i;

	/* Find the Time Slot */
	for (i = 0; i < TOU_MAX_TIME_SLOTS; i++) {
		if (VTou.time_slot[i].rate_id == rateID) {
			return i;
		}
	}

	return INVALID;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
