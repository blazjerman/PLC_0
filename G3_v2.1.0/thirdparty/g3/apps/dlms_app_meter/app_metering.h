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

#ifndef METERING_TASK_H_INCLUDED
#define METERING_TASK_H_INCLUDED

#include "conf_project.h"
#ifdef BOARD_SUPPORTS_METERING

#include "metrology.h"
#include "rtcproc.h"

#define METERING_APP_MEASURE_TIME_MS             2000
#define METERING_APP_DISPUPD_TIME_MS             500

enum {
	VA_RMS = Ua,
	IA_RMS = Ia,
	PA_RMS = Pa,
	QA_RMS = Qa,
	SA_RMS = Sa,
	FREQ = Freq,
	ANGLE_A = AngleA,
};

typedef struct metering_info {
	uint32_t Inst_V_rms_L1;
	uint32_t Inst_V_rms_L2;
	uint32_t Inst_V_rms_L3;
	uint32_t Inst_I_rms_L1;
	uint32_t Inst_I_rms_L2;
	uint32_t Inst_I_rms_L3;
	uint32_t Inst_I_rms_L123;
	uint32_t Inst_P_rms_L1;
	uint32_t Inst_P_rms_L2;
	uint32_t Inst_P_rms_L3;
	uint32_t Inst_P_rms_L123;
	uint32_t Inst_Q_rms_L1;
	uint32_t Inst_Q_rms_L2;
	uint32_t Inst_Q_rms_L3;
	uint32_t Inst_Q_rms_L123;
	uint32_t Inst_S_rms_L1;
	uint32_t Inst_S_rms_L2;
	uint32_t Inst_S_rms_L3;
	uint32_t Inst_S_rms_L123;
	uint32_t Freq;
	uint32_t Angle_L1;
	uint32_t Angle_L2;
	uint32_t Angle_L3;
	uint32_t Inst_Power_factor;
	uint64_t Inst_P_import;
	uint64_t Inst_P_export;
	uint64_t Inst_Q_import;
	uint64_t Inst_Q_export;
} metering_info_t;

#define RTC_SIZE (sizeof(rtc_t) / sizeof(uint32_t))

extern metering_info_t sx_meter_info;

void metering_app_init(void);
void metering_app_process(void);
void metering_app_timers_update(void);

void metering_app_get_info(metering_info_t *px_info);

#endif /* BOARD_SUPPORTS_METERING */
#endif /* METERING_TASK_H_INCLUDED */
