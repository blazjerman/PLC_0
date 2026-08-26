/**
 * \file
 *
 * \brief Metering Example Application
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "conf_project.h"
#ifdef BOARD_SUPPORTS_METERING
#include "asf.h"

#include "compiler.h"
#include "conf_project.h"
#include "conf_hal.h"

#include "app_metering.h"
#include "metrology.h"
#include "shared_memory.h"
#include "rtcproc.h"

metering_info_t sx_meter_info;

#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
static char spuc_disp_buff[17];
static uint32_t sul_disp_counter;
static volatile bool sb_disp_update;

typedef enum {
	disp_v,
	disp_i,
	disp_p,
} disp_state_t;

static disp_state_t suc_disp_state;
#endif

static bool sb_update_met_data;

static void _update_metering_data(void)
{
	/* Get time stamp */

	/* Set flag to get metering data in process function */
	sb_update_met_data = true;
}

static void _init_metrology_fw(void)
{
	extern char met_bin_start;

	/* Copy core1 binary image to SRAM1 */
	memcpy((char *)IRAM1_ADDR, (char *)&met_bin_start, IRAM1_SIZE);

	/* Reset core1 dsp processor */
	REG_RSTC_CPMR |= RSTC_CPMR_CPROCEN | RSTC_CPMR_CPKEY(RSTC_CP_KEY_VAL);

	/* Initialize metrology fw */
	metrology_init();

	/* Set Metering Data Update callback */
	metrology_update_met_data_set_callback(_update_metering_data);

	/* Initialize RTC */
	rtc_init();
}

static void _metrology_data_update(void)
{
	ReadRTCProcess();
	metrology_data_refresh_proc();
	sx_meter_info.Inst_V_rms_L1 =   VAFE.RMS[Ua];
	sx_meter_info.Inst_V_rms_L2 =   VAFE.RMS[Ub];
	sx_meter_info.Inst_V_rms_L3 =   VAFE.RMS[Uc];
	sx_meter_info.Inst_I_rms_L1 =   VAFE.RMS[Ia];
	sx_meter_info.Inst_I_rms_L2 =   VAFE.RMS[Ib];
	sx_meter_info.Inst_I_rms_L3 =   VAFE.RMS[Ic];
	sx_meter_info.Inst_P_rms_L123 = VAFE.RMS[Pt];
	sx_meter_info.Inst_P_rms_L1 =   VAFE.RMS[Pa];
	sx_meter_info.Inst_P_rms_L2 =   VAFE.RMS[Pb];
	sx_meter_info.Inst_P_rms_L3 =   VAFE.RMS[Pc];
	sx_meter_info.Inst_Q_rms_L123 = VAFE.RMS[Qt];
	sx_meter_info.Inst_Q_rms_L1 =   VAFE.RMS[Qa];
	sx_meter_info.Inst_Q_rms_L2 =   VAFE.RMS[Qb];
	sx_meter_info.Inst_Q_rms_L3 =   VAFE.RMS[Qc];
	sx_meter_info.Inst_S_rms_L123 = VAFE.RMS[St];
	sx_meter_info.Inst_S_rms_L1 =   VAFE.RMS[Sa];
	sx_meter_info.Inst_S_rms_L2 =   VAFE.RMS[Sb];
	sx_meter_info.Inst_S_rms_L3 =   VAFE.RMS[Sc];
	sx_meter_info.Freq =            VAFE.RMS[Freq];
	sx_meter_info.Angle_L1 =        VAFE.RMS[AngleA];
	sx_meter_info.Angle_L2 =        VAFE.RMS[AngleB];
	sx_meter_info.Angle_L3 =        VAFE.RMS[AngleC];
	sx_meter_info.Inst_I_rms_L123 = VAFE.RMS[Ia] +          /* TBR */
			VAFE.RMS[Ib] +
			VAFE.RMS[Ic];
	/*	sx_meter_info.Inst_Power_factor = ; */
	/*	sx_meter_info.Inst_P_import = ; */
	/*	sx_meter_info.Inst_P_export = ; */
	/*	sx_meter_info.Inst_Q_import = ; */
	/*	sx_meter_info.Inst_Q_export = ; */
}

void metering_app_timers_update(void)
{
#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
	if (!sul_disp_counter--) {
		sb_disp_update = true;
		sul_disp_counter = METERING_APP_DISPUPD_TIME_MS;
	}
#endif
}

void metering_app_init(void)
{
	/* Configure IPC for the communication between cores */
	configure_ipc();

	/* Enable ATSENSE clock */
	/* Enable PIOA clock */
	pmc_enable_periph_clk(ID_PIOA);
	/* Configure PIO controllers to periph mode */
	pio_set_peripheral(PIOA, PIO_PERIPH_A, (uint32_t)(PIO_PA29A_PCK1));
	/* PCK 1 = output PLLA clk/2 on PA29 */
	pmc_switch_pck_to_pllack(1, PMC_PCK_PRES_CLK_2);
	pmc_enable_pck(PMC_PCK_1);

	/* Init metrology FW */
	_init_metrology_fw();

	/* Init local vars */
	sb_update_met_data = false;
	memset(&sx_meter_info, 0, sizeof(metering_info_t));

#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
	sul_disp_counter = METERING_APP_DISPUPD_TIME_MS;
	sb_disp_update = false;
	suc_disp_state = disp_v;
#endif
}

void metering_app_process(void)
{
	if (sb_update_met_data) {
		sb_update_met_data = false;
		_metrology_data_update();
	}

#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
	if (sb_disp_update) {
		sb_disp_update = false;

		c0216CiZ_set_cursor(C0216CiZ_LINE_UP, 0);

		switch (suc_disp_state) {
		case disp_v:
			sprintf(spuc_disp_buff, "Vrms = %04u mV ", sx_meter_info.Inst_V_rms_L1);
			suc_disp_state = disp_i;
			break;

		case disp_i:
			sprintf(spuc_disp_buff, "Irms = %04u mA  ", sx_meter_info.Inst_I_rms_L1);
			suc_disp_state = disp_p;
			break;

		case disp_p:
			sprintf(spuc_disp_buff, "Prms = %04u W   ", sx_meter_info.Inst_P_rms_L1 / 10);
			suc_disp_state = disp_v;
			break;
		}

		c0216CiZ_show((const char *)spuc_disp_buff);
	}
#endif
}

void metering_app_get_info(metering_info_t *px_info)
{
	memcpy(px_info, &sx_meter_info, sizeof(metering_info_t));
}

#endif /* BOARD_SUPPORTS_METERING */
