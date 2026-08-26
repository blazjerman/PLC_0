/**
 * \file
 *
 * \brief Display Module.
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
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
#include "cl010.h"

#include "command.h"
#include "display.h"
#include "energy.h"
#include "task.h"
#include "demand.h"
#include "metrology.h"
#include "rtcproc.h"
#include "utils.h"
#include "demo_app_version.h"

#include "conf_demo_app.h"

display_t VDisplay;

static uint8_t spuc_app_info[8] = {0};
static bool sb_app_info_en;
static display_info_t spuc_app_loop_info[DISPLAY_MAX_TYPE];
static uint8_t spuc_app_loop_max;
static uint8_t spuc_app_loop_idx;

#define DISPLAY_TIMER_LOOP_INIT              2

/**
 * \brief Change Display Info.
 */
void DisplayChangeInfo(void)
{
	if (spuc_app_loop_max <= DISPLAY_MAX_TYPE) {

		VDisplay.info = spuc_app_loop_info[spuc_app_loop_idx];

		if ((VDisplay.direction & FORWARD) == FORWARD) {
			if (++spuc_app_loop_idx >= spuc_app_loop_max) {
				spuc_app_loop_idx = 0;
			}
		} else if ((VDisplay.direction & BACKWARD) == BACKWARD) {
			if (--spuc_app_loop_idx >= spuc_app_loop_max) {
				spuc_app_loop_idx = spuc_app_loop_max - 1;
			}
		}

		/* Reload Timer Loop */
		VDisplay.timer = VDisplay.timer_ref;

		TaskPutIntoQueue(DisplayProcess);
	}
}

/**
 * \brief Refresh display Process.
 */
void DisplayProcess(void)
{
	uint64_t total;
	uint64_t upd_symbols = 1;
	uint32_t temp, temp_dec;
	uint8_t buff1[9];

	if (VDisplay.info != 0xFF) {
		cl010_clear_all();
	}
	/* Init internal buffer */
	memset(buff1, 0, sizeof(buff1));

	switch (VDisplay.info) {
	case DISPLAY_TOTAL_ENERGY:
	{
		total = VEnergy.tou1_acc + VEnergy.tou2_acc + VEnergy.tou3_acc + VEnergy.tou4_acc;

		/* Check magnitud to select units to show */
		if (total > 999999999) {
			/* Format: xxxxxx.xx kWh */
			cl010_show_units(CL010_UNIT_kWh);
			total = total/100000;
			temp = total/100;
			temp_dec = total%100;
			sprintf((char *)buff1, "%6u%02u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_2);
		} else {
			/* Format: xxxxx.xxx Wh */
			cl010_show_units(CL010_UNIT_Wh);
			total=total/10;
                        temp = total/1000;
			temp_dec = total%1000;
			sprintf((char *)buff1, "%5u%03u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_1);
		}

		cl010_show_numeric_string(CL010_LINE_UP, buff1);
                cl010_show_icon(CL010_ICON_CUM);
                sprintf((char *)buff1, "   1234");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU1_ENERGY:
	{
		total = VEnergy.tou1_acc;

		/* Check magnitud to select units to show */
		if (total > 999999999) {
			/* Format: xxxxxx.xx kWh */
			cl010_show_units(CL010_UNIT_kWh);
			total = total/100000;
			temp = total/100;
			temp_dec = total%100;
			sprintf((char *)buff1, "%6u%02u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_2);
		} else {
			/* Format: xxxxx.xxx Wh */
			cl010_show_units(CL010_UNIT_Wh);
			total=total/10;
                        temp = total/1000;
			temp_dec = total%1000;
			sprintf((char *)buff1, "%5u%03u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_1);
		}

		cl010_show_numeric_string(CL010_LINE_UP, buff1);
                cl010_show_icon(CL010_ICON_CUM);
                sprintf((char *)buff1, "      1");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU2_ENERGY:
	{
		total = VEnergy.tou2_acc;

		/* Check magnitud to select units to show */
		if (total > 999999999) {
			/* Format: xxxxxx.xx kWh */
			cl010_show_units(CL010_UNIT_kWh);
			total = total/100000;
			temp = total/100;
			temp_dec = total%100;
			sprintf((char *)buff1, "%6u%02u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_2);
		} else {
			/* Format: xxxxx.xxx Wh */
			cl010_show_units(CL010_UNIT_Wh);
			total=total/10;
                        temp = total/1000;
			temp_dec = total%1000;
			sprintf((char *)buff1, "%5u%03u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_1);
		}

		cl010_show_numeric_string(CL010_LINE_UP, buff1);
                cl010_show_icon(CL010_ICON_CUM);
                sprintf((char *)buff1, "      2");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU3_ENERGY:
	{
		total = VEnergy.tou3_acc;

		/* Check magnitud to select units to show */
		if (total > 999999999) {
			/* Format: xxxxxx.xx kWh */
			cl010_show_units(CL010_UNIT_kWh);
			total = total/100000;
			temp = total/100;
			temp_dec = total%100;
			sprintf((char *)buff1, "%6u%02u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_2);
		} else {
			/* Format: xxxxx.xxx Wh */
			cl010_show_units(CL010_UNIT_Wh);
			total=total/10;
                        temp = total/1000;
			temp_dec = total%1000;
			sprintf((char *)buff1, "%5u%03u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_1);
		}

		cl010_show_numeric_string(CL010_LINE_UP, buff1);
                cl010_show_icon(CL010_ICON_CUM);
                sprintf((char *)buff1, "      3");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU4_ENERGY:
	{
		total = VEnergy.tou4_acc;

		/* Check magnitud to select units to show */
		if (total > 999999999) {
			/* Format: xxxxxx.xx kWh */
			cl010_show_units(CL010_UNIT_kWh);
			total = total/100000;
			temp = total/100;
			temp_dec = total%100;
			sprintf((char *)buff1, "%6u%02u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_2);
		} else {
			/* Format: xxxxx.xxx Wh */
			cl010_show_units(CL010_UNIT_Wh);
			total=total/10;
                        temp = total/1000;
			temp_dec = total%1000;
			sprintf((char *)buff1, "%5u%03u", temp, temp_dec);
                        cl010_show_icon(CL010_ICON_DOT_1);
		}

		cl010_show_numeric_string(CL010_LINE_UP, buff1);
                cl010_show_icon(CL010_ICON_CUM);
                sprintf((char *)buff1, "      4");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_VA_RMS:
	{
		temp_dec = VAFE.RMS[Ua]%10000;
		temp = VAFE.RMS[Ua]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_V);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L1);
	}
	break;

	case DISPLAY_VB_RMS:
	{
		temp_dec = VAFE.RMS[Ub]%10000;
		temp = VAFE.RMS[Ub]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_V);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L2);
	}
	break;

	case DISPLAY_VC_RMS:
	{
		temp_dec = VAFE.RMS[Uc]%10000;
		temp = VAFE.RMS[Uc]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_V);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L3);
	}
	break;

	case DISPLAY_IA_RMS:
	{
		temp_dec = VAFE.RMS[Ia]%10000;
		temp = VAFE.RMS[Ia]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_A);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L1);
	}
	break;

	case DISPLAY_IB_RMS:
	{
		temp_dec = VAFE.RMS[Ib]%10000;
		temp = VAFE.RMS[Ib]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_A);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L2);
	}
	break;

	case DISPLAY_IC_RMS:
	{
		temp_dec = VAFE.RMS[Ic]%10000;
		temp = VAFE.RMS[Ic]/10000;
		sprintf((char *)buff1, "%5u%03u", temp, (temp_dec/10));
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_A);
		cl010_show_icon(CL010_ICON_DOT_1);
		cl010_show_icon(CL010_ICON_L3);
	}
	break;

	case DISPLAY_RTC_TIME:
	{
		sprintf((char *)buff1, "%02d%02d%02d  ", (uint8_t)VRTC.time.hour,
			(uint8_t)VRTC.time.minute, (uint8_t)VRTC.time.second);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_icon(CL010_ICON_TIME);
		cl010_show_icon(CL010_ICON_COL_1);
		cl010_show_icon(CL010_ICON_COL_2);
		upd_symbols = 0;
	}
	break;

	case DISPLAY_RTC_DATE:
	{
		uint8_t uc_year;

		uc_year = VRTC.date.year - (VRTC.date.year / 100) * 100;
		sprintf((char *)buff1, "%02d%02d%02d  ", uc_year, (uint8_t)VRTC.date.month,
			(uint8_t)VRTC.date.day);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_icon(CL010_ICON_DATE);
		cl010_show_icon(CL010_ICON_COL_1);
		cl010_show_icon(CL010_ICON_COL_2);
		upd_symbols = 0;
	}
	break;

	case DISPLAY_TOTAL_MAX_DEMAND:
	{
		uint8_t uc_month_idx;

		uc_month_idx = VRTC.date.month - 1;
		sprintf((char *)buff1, "%8u", VDemand.max_month[uc_month_idx][TOUALL].max);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_W);
		cl010_show_icon(CL010_ICON_MD);
                sprintf((char *)buff1, "   1234");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU1_MAX_DEMAND:
	{
		uint8_t uc_month_idx;

		uc_month_idx = VRTC.date.month - 1;
		sprintf((char *)buff1, "%8u", VDemand.max_month[uc_month_idx][0].max);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_W);
		cl010_show_icon(CL010_ICON_MD);
                sprintf((char *)buff1, "     01");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU2_MAX_DEMAND:
	{
		uint8_t uc_month_idx;

		uc_month_idx = VRTC.date.month - 1;
		sprintf((char *)buff1, "%8u", VDemand.max_month[uc_month_idx][1].max);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_W);
		cl010_show_icon(CL010_ICON_MD);
                sprintf((char *)buff1, "     02");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU3_MAX_DEMAND:
	{
		uint8_t uc_month_idx;

		uc_month_idx = VRTC.date.month - 1;
		sprintf((char *)buff1, "%8u", VDemand.max_month[uc_month_idx][2].max);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_W);
		cl010_show_icon(CL010_ICON_MD);
                sprintf((char *)buff1, "     03");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_TOU4_MAX_DEMAND:
	{
		uint8_t uc_month_idx;

		uc_month_idx = VRTC.date.month - 1;
		sprintf((char *)buff1, "%8u", VDemand.max_month[uc_month_idx][3].max);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		cl010_show_units(CL010_UNIT_W);
		cl010_show_icon(CL010_ICON_MD);
                sprintf((char *)buff1, "     04");
		cl010_show_numeric_string(CL010_LINE_DOWN, buff1);
	}
	break;

	case DISPLAY_APP_INFO:
	{
		cl010_show_numeric_string(CL010_LINE_UP, spuc_app_info);
		upd_symbols = 0;
	}
	break;

	case DISPLAY_BOARD_ID:
	{
		sprintf((char *)buff1, "%08x", DISPLAY_BOARD_VERSION); /* conf_demo_app.h */
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		upd_symbols = 0;
	}
	break;

	case DISPLAY_VERSION:
	{
		sprintf((char *)buff1, "%d", DEMO_APP_VERSION);
		cl010_show_numeric_string(CL010_LINE_UP, buff1);
		upd_symbols = 0;
	}
	break;

	default:
		break;
	}

	if (upd_symbols) {
		if (VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VA == 1 && VAFE.ST.BIT.sag_a == 1) {
			/* "A alarm" phase */
			cl010_show_icon(CL010_ICON_PHASE_1);
		}

		if ((VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VA + VAFE.ST.BIT.sag_a) == 1) {
			if ((VDisplay.timer & 0x01) == 0x01) {
				/* "A alarm" phase */
				cl010_show_icon(CL010_ICON_PHASE_1);
			}
		}

		if (VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VB == 1 && VAFE.ST.BIT.sag_b == 1) {
			/* "B alarm" phase */
			cl010_show_icon(CL010_ICON_PHASE_2);
		}

		if ((VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VB + VAFE.ST.BIT.sag_b) == 1) {
			if ((VDisplay.timer & 0x01) == 0x01) {
				/* "B alarm" phase */
				cl010_show_icon(CL010_ICON_PHASE_2);
			}
		}
#if BOARD==PIC32CXMTC_DB
		if (VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VC == 1 && VAFE.ST.BIT.sag_c == 1) {
			/* "C alarm" phase */
			cl010_show_icon(CL010_ICON_PHASE_3);
		}

		if ((VMetrology.DSP_STATUS.STATE_FLAG.BIT.SAG_DET_VC + VAFE.ST.BIT.sag_c) == 1) {
			if ((VDisplay.timer & 0x01) == 0x01) {
				/* "C alarm" phase */
				cl010_show_icon(CL010_ICON_PHASE_3);
			}
		}
#endif

		if (VAFE.ST.BIT.pt_dir == 1) {
			/* active power is reverse */
			cl010_show_icon(CL010_ICON_P_MINUS);
		} else {
			/* active power is forward */
			cl010_show_icon(CL010_ICON_P_PLUS);
		}

		if (VAFE.ST.BIT.qt_dir == 1) {
			/* reactive power is reverse */
			cl010_show_icon(CL010_ICON_Q_MINUS);
		} else {
			/* reactive power is forward */
			cl010_show_icon(CL010_ICON_Q_PLUS);
		}

		/* ---display communication symbol----------------------- */
		if (VCom.lamptimer) {
			cl010_show_icon(CL010_ICON_COMM_SIGNAL_LOW);
			cl010_show_icon(CL010_ICON_COMM_SIGNAL_MED);
			cl010_show_icon(CL010_ICON_COMM_SIGNAL_HIG);
		} else {
			cl010_clear_icon(CL010_ICON_COMM_SIGNAL_LOW);
			cl010_clear_icon(CL010_ICON_COMM_SIGNAL_MED);
			cl010_clear_icon(CL010_ICON_COMM_SIGNAL_HIG);
		}
	}

	/* --- display MCHP logo -------------------------------------- */
	cl010_show_icon(CL010_ICON_MICROCHIP);
}

/**
 * \brief Init display module.
 */
status_code_t DisplayInit(void)
{
	status_code_t status;

	/* Initialize the CL010 LCD glass component. */
	status = cl010_init();
	if (status != STATUS_OK) {
		return STATUS_ERR_BUSY;
	}

	/* Show All symbols */
	cl010_show_all();

	/* Show Icons */
	cl010_show_icon(CL010_ICON_MICROCHIP);
	cl010_show_icon(CL010_ICON_LCD);

	/* Set Startup display info */
	spuc_app_loop_max = 0;
	spuc_app_loop_idx = 0;

	/* Clear flag of enable application message */
	sb_app_info_en = false;

        /* Set display timer */
        VDisplay.timer = DISPLAY_TIMER_LOOP_INIT;

	/* Set display loop direction by default */
	VDisplay.direction = FORWARD;

	/* Init display info */
	VDisplay.info = (display_info_t)0xFF;

	return STATUS_OK;
}

/**
 * \brief Set information to show in display from an external application.
 *
 * \param msg     Pointer to application message to show (only digits 0 - 9)
 * \param len     Length of the message (max 8 digits)
 */
void DisplaySetAppInfo(char *msg, uint8_t len)
{
	if (len > sizeof(spuc_app_info)) {
		len = sizeof(spuc_app_info);
	}

	memcpy(spuc_app_info, msg, len);

	if (!sb_app_info_en) {
		/* Set enable flag */
		sb_app_info_en = true;
		/* Add APP display info */
		DisplayAddLoopInfo(DISPLAY_APP_INFO);
	}

	/* Change Display info and Restart Timer */
	VDisplay.timer = VDisplay.timer_ref;
	VDisplay.info = DISPLAY_APP_INFO;
	TaskPutIntoQueue(DisplayProcess);
}

/**
 * \brief Timer in seconds for changing the info in loop mode.
 *
 * \param time_sec     Timer in seconds for changing the info in loop mode.
 */
void DisplaySetTimerLoop(uint32_t time_sec)
{
	if (time_sec) {
		/* Set Timer Reference for loop mode */
		VDisplay.timer_ref = time_sec;
	}
}

/**
 * \brief Add display info to the display loop.
 *
 * \param info     Display info to add to the display loop.
 */
void DisplayAddLoopInfo(display_info_t info)
{
	if ((info < DISPLAY_MAX_TYPE) && (spuc_app_loop_max < DISPLAY_MAX_TYPE)) {
		/* Add info to display */
		spuc_app_loop_info[spuc_app_loop_max++] = info;
		/* Reload display timer */
		//VDisplay.timer = VDisplay.timer_ref;
		/* Restart display info */
		//VDisplay.info = info;
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

/**
 * \}
 */
