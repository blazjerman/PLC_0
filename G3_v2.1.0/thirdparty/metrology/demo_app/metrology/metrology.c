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

#include <math.h>
#include <string.h>

#include "delay.h"
#include "extmem.h"
#include "utils.h"
#include "task.h"
#include "metrology.h"
#include "coproc.h"

metrology_t VMetrology;
metrology_cal_t VCalibration;
metrology_afe_t VAFE;

metrology_har_t VHar_calc;
static pf_har_callback pf_met_har_callback = NULL;
static pf_cal_callback pf_met_cal_callback = NULL;

static uint32_t har_temp[DSP_HAR_SIZE];

int32_t VCapture_Buff[MET_CAPTURE_BUFF_LEN];

DSP_CTRL_TYPE dsp_ctrl_default = {
	{ STATE_CTRL_Run },             /* 00 STATE_CTRL */
#if BOARD == PIC32CXMTC_DB
	{ 0x00000700 },                 /* 01 FEATURE_CTRL0 */
#elif BOARD == PIC32CXMTSH_DB
	{ 0x00000300 },                 /* 01 FEATURE_CTRL0 */
#endif
	{ 0x00000000 },                 /* 02 FEATURE_CTRL1 */
	{ 0x00000CCC },                 /* 03 METER_TYPE sensor_type =0 CT, 1 SHUNT, 2 ROGOWSKI */

	{ 0x00000000 },                 /* 04 M M=50->50Hz M=60->60Hz */
	{ 0x00001130 },                 /* 05 N_MAX 4400=0x1130 */
	{ 0x81009100 },                 /* 06 PULSE0_CTRL: enable pulse, detent NET, Watt-hours, width=80mS=80*464=0x9100 */
	{ 0x81029100 },                 /* 07 PULSE1_CTRL: enable pulse, detent NET, Var-hours, width=80mS=80*464=0x9100 */

	{ 0x11049100 },                 /* 08 PULSE2_CTRL: disable pulse */
	(0x00500000),                   /* 09 P_K_T=(1000/3200)*2^24=52428800   mc=3200 imp/kWh */
	(0x00500000),                   /* 10 Q_K_T */
	(0x00500000),                   /* 11 I_K_T */

	(0x00002E9A),                   /* 12 CREEP_THR_P 2w	0x2E9A=[2/(50*3600)]*2^30 */
	(0x00002E9A),                   /* 13 CREEP_THR_Q 2var 0x2E9A=[2/(50*3600)]*2^30 */
	(0x0000212D),                   /* 14 CREEP_THR_I 5mA K_Ix=617.283 8493=(5/617.283）*2^20 */
	{ 0x00000000 },                 /* 15 POWER_OFFSET_CTRL, Disable Power offset */

	(0x00000000),                   /* 16 POWER_OFFSET_P */
	(0x00000000),                   /* 17 POWER_OFFSET_Q */
	(0x05E84F61),                   /* 18 SWELL_THR_VA	[(220*114%)/1651]^2*2^32 */
	(0x05E84F61),                   /* 19 SWELL_THR_VB */

	(0x05E84F61),                   /* 20 SWELL_THR_VC */
	(0x01A2EC26),                   /* 21 SAG_THR_VA	[(220*60%)/1651]^2*2^32 */
	(0x01A2EC26),                   /* 22 SAG_THR_VB */
	(0x01A2EC26),                   /* 23 SAG_THR_VC */

	(0x0009A523),                   /* 24 K_IA	[Te=2000/3.24]*2^10 */
	(0x0019CC00),                   /* 25 K_VA	1651*2^10 */
	(0x0009A523),                   /* 26 K_IB */
	(0x0019CC00),                   /* 27 K_VB */

	(0x0009A523),                   /* 28 K_IC */
	(0x0019CC00),                   /* 29 K_VC */
	(0x0009A523),                   /* 30 K_IN */
	(0x20000000),                   /* 31 CAL_M_IA */

	(0x20000000),                   /* 32 CAL_M_VA */
	(0x20000000),                   /* 33 CAL_M_IB */
	(0x20000000),                   /* 34 CAL_M_VB */
	(0x20000000),                   /* 35 CAL_M_IC */

	(0x20000000),                   /* 36 CAL_M_VC */
	(0x20000000),                   /* 37 CAL_M_IN */
	(0x00000000),                   /* 38 CAL_PH_IA */
	(0x00000000),                   /* 39 CAL_PH_VA */

	(0x00000000),                   /* 40 CAL_PH_IB */
	(0x00000000),                   /* 41 CAL_PH_VB */
	(0x00000000),                   /* 42 CAL_PH_IC */
	(0x00000000),                   /* 43 CAL_PH_VC */

	(0x00000000),                   /* 44 CAL_PH_IN */
	(0x00000000),                   /* 45 CAPTURE_CTRL */
	(MET_CAPTURE_BUFF_LEN),         /* 46 CAPTURE_BUFF_SIZE */
	(0x00000000),                   /* 47 CAPTURE_ADDR */

	(0x00000000),                   /* 48 RESERVED_C48 */
	(0x00000000),                   /* 49 RESERVED_C49 */
	(0x00000000),                   /* 50 RESERVED_C50 */
	{ 0x01010103 },                 /* 51 ATSENSE_CTRL_20_23: I2GAIN=0,I2ON=1,V1ON=1,I1GAIN=0,I1ON=1,I0GAIN=0,TEMP=1,I0ON=1 */

	{ 0x07010101 },                 /* 52 ATSENSE_CTRL_24_27: ONLDO=1,ONREF=1,ONBIAS=1,V3ON=1,I3GAIN=0,I3ON=1,V2ON=1 */
	{ 0x00000003 },                 /* 53 ATSENSE_CTRL_28_2B: MSB_MODE=0,OSR=3 */
	(0x00000000),                   /* 54 RESERVED_C54 */
	(0x00000000),                   /* 55 POWER_OFFSET_P_A */

	(0x00000000),                   /* 56 POWER_OFFSET_P_B */
	(0x00000000),                   /* 57 POWER_OFFSET_P_C */
	(0x00000000),                   /* 58 POWER_OFFSET_Q_A */
	(0x00000000),                   /* 59 POWER_OFFSET_Q_B */

	(0x00000000)                    /* 60 POWER_OFFSET_Q_C */
};

/* ------------------------------------------------------------------- */
/* ----------------------new dsp define------------------------------- */
/* ------------------------------------------------------------------- */
static uint64_t dsp_null_value = 0;

const uint8_t *dsp_ctrl_header[] = {
	"00 STATE_CTRL",
	"01 FEATURE_CTRL0",
	"02 FEATURE_CTRL1",
	"03 METER_TYPE",
	"04 M",
	"05 N_MAX",
	"06 PULSE_CTRL0",
	"07 PULSE_CTRL1",
	"08 PULSE_CTRL2",
	"09 P_K_T",
	"10 Q_K_T",
	"11 I_K_T",
	"12 CREEP_THR_P",
	"13 CREEP_THR_Q",
	"14 CREEP_THR_I",
	"15 POWER_OS_CTRL",
	"16 POWER_OFFSET_P",
	"17 POWER_OFFSET_Q",
	"18 SWELL_THR_VA",
	"19 SWELL_THR_VB",
	"20 SWELL_THR_VC",
	"21 SAG_THR_VA",
	"22 SAG_THR_VB",
	"23 SAG_THR_VC",
	"24 K_IA",
	"25 K_VA",
	"26 K_IB",
	"27 K_VB",
	"28 K_IC",
	"29 K_VC",
	"30 K_IN",
	"31 CAL_M_IA",
	"32 CAL_M_VA",
	"33 CAL_M_IB",
	"34 CAL_M_VB",
	"35 CAL_M_IC",
	"36 CAL_M_VC",
	"37 CAL_M_IN",
	"38 CAL_PH_IA",
	"39 CAL_PH_VA",
	"40 CAL_PH_IB",
	"41 CAL_PH_VB",
	"42 CAL_PH_IC",
	"43 CAL_PH_VC",
	"44 CAL_PH_IN",
	"45 CAPTURE_CTRL",
	"46 CAPT_BUFF_SIZE",
	"47 CAPTURE_ADDR",
	"48 RESERVED_C48",
	"49 RESERVED_C49",
	"50 RESERVED_C50",
	"51 AT_CTRL_20_23",
	"52 AT_CTRL_24_27",
	"53 AT_CTRL_28_2B",
	"54 RESERVED_C54",
	"55 POWER_OS_P_A",
	"56 POWER_OS_P_B",
	"57 POWER_OS_P_C",
	"58 POWER_OS_Q_A",
	"59 POWER_OS_Q_B",
	"60 POWER_OS_Q_C",
	NULL,
	NULL,
	NULL
};

const uint32_t *dsp_ctrl_str[] = {
	&VMetrology.DSP_CTRL.STATE_CTRL.WORD,
	&VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD,
	&VMetrology.DSP_CTRL.FEATURE_CTRL1.WORD,
	&VMetrology.DSP_CTRL.METER_TYPE.WORD,
	&VMetrology.DSP_CTRL.M.WORD,
	&VMetrology.DSP_CTRL.N_MAX.WORD,
	&VMetrology.DSP_CTRL.PULSE0_CTRL.WORD,
	&VMetrology.DSP_CTRL.PULSE1_CTRL.WORD,
	&VMetrology.DSP_CTRL.PULSE2_CTRL.WORD,
	&VMetrology.DSP_CTRL.P_K_T,
	&VMetrology.DSP_CTRL.Q_K_T,
	&VMetrology.DSP_CTRL.I_K_T,
	&VMetrology.DSP_CTRL.CREEP_THR_P,
	&VMetrology.DSP_CTRL.CREEP_THR_Q,
	&VMetrology.DSP_CTRL.CREEP_THR_I,
	&VMetrology.DSP_CTRL.POWER_OFFSET_CTRL.WORD,
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_P),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_Q),
	&VMetrology.DSP_CTRL.SWELL_THR_VA,
	&VMetrology.DSP_CTRL.SWELL_THR_VB,
	&VMetrology.DSP_CTRL.SWELL_THR_VC,
	&VMetrology.DSP_CTRL.SAG_THR_VA,
	&VMetrology.DSP_CTRL.SAG_THR_VB,
	&VMetrology.DSP_CTRL.SAG_THR_VC,
	&VMetrology.DSP_CTRL.K_IA,
	&VMetrology.DSP_CTRL.K_VA,
	&VMetrology.DSP_CTRL.K_IB,
	&VMetrology.DSP_CTRL.K_VB,
	&VMetrology.DSP_CTRL.K_IC,
	&VMetrology.DSP_CTRL.K_VC,
	&VMetrology.DSP_CTRL.K_IN,
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IN),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IN),
	&VMetrology.DSP_CTRL.CAPTURE_CTRL.WORD,
	&VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.WORD,
	&VMetrology.DSP_CTRL.CAPTURE_ADDR,
	&VMetrology.DSP_CTRL.RESERVED_C48,
	&VMetrology.DSP_CTRL.RESERVED_C49,
	&VMetrology.DSP_CTRL.RESERVED_C50,
	&VMetrology.DSP_CTRL.ATSENSE_CTRL_20_23.WORD,
	&VMetrology.DSP_CTRL.ATSENSE_CTRL_24_27.WORD,
	&VMetrology.DSP_CTRL.ATSENSE_CTRL_28_2B.WORD,
	&VMetrology.DSP_CTRL.RESERVED_C54,
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_P_A),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_P_B),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_P_C),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_Q_A),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_Q_B),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_Q_C),
	(uint32_t *)&dsp_null_value,
	(uint32_t *)&dsp_null_value,
	(uint32_t *)&dsp_null_value
};

const uint8_t *dsp_st_header[] = {
	"00 VERSION",
	"01 STATUS",
	"02 STATE_FLAG",
	"03 CAPTURE_STATUS",
	"04 INTERVAL_NUM",
	"05 N",
	"06 PH_OFFSET",
	"07 FREQ",
	"08 FREQ_VA",
	"09 FREQ_VB",
	"10 FREQ_VC",
	"11 RESERVED_S11",
	"12 TEMPERATURE",
	"13 I_A_MAX",
	"14 I_B_MAX",
	"15 I_C_MAX",
	"16 I_Nm_MAX",
	"17 I_Ni_MAX",
	"18 V_A_MAX",
	"19 V_B_MAX",
	"20 V_C_MAX",
	"21 FEATURES",
	"22 RESERVED_S22",
	"23 RESERVED_S23",
	"24 RESERVED_S24",
	"25 RESERVED_S25",
	"26 RESERVED_S26",
	"27 RESERVED_S27",
	"28 RESERVED_S28",
	"29 RESERVED_S29",
	"30 ZC_N_VA",
	"31 ZC_N_VB",
	"32 ZC_N_VC",
	"33 AT_CAL_41_44",
	"34 AT_CAL_45_48",
	NULL
};

const uint32_t *dsp_st_str[] = {
	&VMetrology.DSP_STATUS.VERSION.WORD,
	&VMetrology.DSP_STATUS.STATUS.WORD,
	&VMetrology.DSP_STATUS.STATE_FLAG.WORD,
	&VMetrology.DSP_STATUS.CAPTURE_STATUS.WORD,
	&VMetrology.DSP_STATUS.INTERVAL_NUM.WORD,
	&VMetrology.DSP_STATUS.N,
	(uint32_t *)(&VMetrology.DSP_STATUS.PH_OFFSET),
	&VMetrology.DSP_STATUS.FREQ,
	&VMetrology.DSP_STATUS.FREQ_VA,
	&VMetrology.DSP_STATUS.FREQ_VB,
	&VMetrology.DSP_STATUS.FREQ_VC,
	&VMetrology.DSP_STATUS.RESERVED_S11,
	(uint32_t *)(&VMetrology.DSP_STATUS.TEMPERATURE),
	(uint32_t *)(&VMetrology.DSP_STATUS.I_A_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.I_B_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.I_C_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.I_Nm_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.I_Ni_MAX),
		(uint32_t *)(&VMetrology.DSP_STATUS.V_A_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.V_B_MAX),
	(uint32_t *)(&VMetrology.DSP_STATUS.V_C_MAX),
	&VMetrology.DSP_STATUS.FEATURES.WORD,
	&VMetrology.DSP_STATUS.RESERVED_S22,
	&VMetrology.DSP_STATUS.RESERVED_S23,
	&VMetrology.DSP_STATUS.RESERVED_S24,
	&VMetrology.DSP_STATUS.RESERVED_S25,
	&VMetrology.DSP_STATUS.RESERVED_S26,
	&VMetrology.DSP_STATUS.RESERVED_S27,
	&VMetrology.DSP_STATUS.RESERVED_S28,
	&VMetrology.DSP_STATUS.RESERVED_S29,
	&VMetrology.DSP_STATUS.ZC_N_VA,
	&VMetrology.DSP_STATUS.ZC_N_VB,
	&VMetrology.DSP_STATUS.ZC_N_VC,
	&VMetrology.DSP_STATUS.ATsense_CAL_41_44.WORD,
	&VMetrology.DSP_STATUS.ATsense_CAL_45_48.WORD,
	(uint32_t *)&dsp_null_value
};

const uint8_t *dsp_acc_header[] = {
	"00 I_A",
	"01 I_B",
	"02 I_C",
	"03 I_Ni",
	"04 I_Nm",
	"05 I_A_F",
	"06 I_B_F",
	"07 I_C_F",
	"08 I_Nmi",
	"09 I_Nm_F",
	"10 RESERVED_A10",
	"11 RESERVED_A11",
	"12 RESERVED_A12",
	"13 RESERVED_A13",
	"14 RESERVED_A14",
	"15 P_A",
	"16 P_B",
	"17 P_C",
	"18 P_A_F",
	"19 P_B_F",
	"20 P_C_F",
	"21 P_N",
	"22 P_N_F",
	"23 RESERVED_A23",
	"24 Q_A",
	"25 Q_B",
	"26 Q_C",
	"27 Q_A_F",
	"28 Q_B_F",
	"29 Q_C_F",
	"30 Q_N",
	"31 Q_N_F",
	"32 RESERVED_A32",
	"33 V_A",
	"34 V_B",
	"35 V_C",
	"36 RESERVED_A36",
	"37 V_A_F",
	"38 V_B_F",
	"39 V_C_F",
	"40 RESERVED_A40",
	"41 V_AB",
	"42 V_BC",
	"43 V_CA",
	"44 V_AB_F",
	"45 V_BC_F",
	"46 V_CA_F",
	"47 RESERVED_A47",
	"48 RESERVED_A48",
	"49 RESERVED_A49",
	"50 ACC_T0",
	"51 ACC_T1",
	"52 ACC_T2",
	"53 RESERVED_A53",
	"54 RESERVED_A54",
	NULL
};

const uint64_t *dsp_acc_str[] = {
	&VMetrology.DSP_ACC.I_A,
	&VMetrology.DSP_ACC.I_B,
	&VMetrology.DSP_ACC.I_C,
	&VMetrology.DSP_ACC.I_Ni,
	&VMetrology.DSP_ACC.I_Nm,
	&VMetrology.DSP_ACC.I_A_F,
	&VMetrology.DSP_ACC.I_B_F,
	&VMetrology.DSP_ACC.I_C_F,
	&VMetrology.DSP_ACC.I_Nmi,
	&VMetrology.DSP_ACC.I_Nm_F,
	&VMetrology.DSP_ACC.RESERVED_A10,
	&VMetrology.DSP_ACC.RESERVED_A11,
	&VMetrology.DSP_ACC.RESERVED_A12,
	&VMetrology.DSP_ACC.RESERVED_A13,
	&VMetrology.DSP_ACC.RESERVED_A14,
	(uint64_t *)(&VMetrology.DSP_ACC.P_A),
	(uint64_t *)(&VMetrology.DSP_ACC.P_B),
	(uint64_t *)(&VMetrology.DSP_ACC.P_C),
	(uint64_t *)(&VMetrology.DSP_ACC.P_A_F),
	(uint64_t *)(&VMetrology.DSP_ACC.P_B_F),
	(uint64_t *)(&VMetrology.DSP_ACC.P_C_F),
	(uint64_t *)(&VMetrology.DSP_ACC.P_N),
	(uint64_t *)(&VMetrology.DSP_ACC.P_N_F),
	&VMetrology.DSP_ACC.RESERVED_A23,
	(uint64_t *)(&VMetrology.DSP_ACC.Q_A),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_B),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_C),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_A_F),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_B_F),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_C_F),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_N),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_N_F),
	&VMetrology.DSP_ACC.RESERVED_A32,
	&VMetrology.DSP_ACC.V_A,
	&VMetrology.DSP_ACC.V_B,
	&VMetrology.DSP_ACC.V_C,
	&VMetrology.DSP_ACC.RESERVED_A36,
	&VMetrology.DSP_ACC.V_A_F,
	&VMetrology.DSP_ACC.V_B_F,
	&VMetrology.DSP_ACC.V_C_F,
	&VMetrology.DSP_ACC.RESERVED_A40,
	&VMetrology.DSP_ACC.V_AB,
	&VMetrology.DSP_ACC.V_BC,
	&VMetrology.DSP_ACC.V_CA,
	&VMetrology.DSP_ACC.V_AB_F,
	&VMetrology.DSP_ACC.V_BC_F,
	&VMetrology.DSP_ACC.V_CA_F,
	&VMetrology.DSP_ACC.RESERVED_A47,
	&VMetrology.DSP_ACC.RESERVED_A48,
	&VMetrology.DSP_ACC.RESERVED_A49,
	(uint64_t *)(&VMetrology.DSP_ACC.ACC_T0),
	(uint64_t *)(&VMetrology.DSP_ACC.ACC_T1),
	(uint64_t *)(&VMetrology.DSP_ACC.ACC_T2),
	&VMetrology.DSP_ACC.RESERVED_A53,
	&VMetrology.DSP_ACC.RESERVED_A54,
	&dsp_null_value,
};

const uint8_t *dsp_har_header[] = {
	"00 I_A_m_R",
	"01 V_A_m_R",
	"02 I_B_m_R",
	"03 V_B_m_R",
	"04 I_C_m_R",
	"05 V_C_m_R",
        "06 I_N_m_R",
	"07 I_A_m_I",
	"08 V_A_m_I",
	"09 I_B_m_I",
	"10 V_B_m_I",
	"11 I_C_m_I",
	"12 V_C_m_I",
        "13 I_N_m_I"
};

const uint32_t *dsp_har_str[] = {
	(uint32_t *)(&VMetrology.DSP_HAR.I_A_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.V_A_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.I_B_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.V_B_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.I_C_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.V_C_m_R),
        (uint32_t *)(&VMetrology.DSP_HAR.I_N_m_R),
	(uint32_t *)(&VMetrology.DSP_HAR.I_A_m_I),
	(uint32_t *)(&VMetrology.DSP_HAR.V_A_m_I),
	(uint32_t *)(&VMetrology.DSP_HAR.I_B_m_I),
	(uint32_t *)(&VMetrology.DSP_HAR.V_B_m_I),
	(uint32_t *)(&VMetrology.DSP_HAR.I_C_m_I),
	(uint32_t *)(&VMetrology.DSP_HAR.V_C_m_I),
        (uint32_t *)(&VMetrology.DSP_HAR.I_N_m_I)
};

const uint8_t *dsp_hrr_header[] = {
	"Irms_Har_A(A)",
	"Irms_Har_B(A)",
	"Irms_Har_C(A)",
        "Irms_Har_N(A)",
	"Vrms_Har_A(V)",
	"Vrms_Har_B(V)",
	"Vrms_Har_C(V)"
};

static void _metrology_set_ctrl_status(DSP_CTRL_ST_CTRL_TYPE iSStatus)
{
	VMetrology.DSP_CTRL.STATE_CTRL.BIT.STATE_CTRL = iSStatus;
	ptr_mem_reg_in->STATE_CTRL.BIT.STATE_CTRL = iSStatus;
}
/* =================================================================== */
/* description	::	calculate voltage or current rms value */
/* function		::	calculate_V_rms */
/* input			::	val,k_x */
/* output		::	calculation value *10000 (0.0001V / 0.0001A) (hex) */
/* call			::	sqrt */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_VI_rms(uint64_t val, uint32_t k_x)
{
	double m;

	m = (double)(val);
	m = (m / RMS_DIV_Q);                    /* m =m/2^40 */
	m = (m / VMetrology.DSP_STATUS.N);
	m = sqrt(m);
	m = m * k_x / RMS_DIV_G;        /* m =m*k_x */
	m = m * 10000;                   /* m =m*1000 */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate neutural current rms value */
/* function		::	calculate_Inx_rms_bcd */
/* input			::	val */
/* output		::	calculation value (0.0001A) (hex) */
/* call			::	sqrt */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_Inx_rms(uint64_t val)
{
	double m;

	m = (double)(val);
	m = (m / RMS_DIV_Inx_Q); /* m =m/2^20 */
	m = (m / VMetrology.DSP_STATUS.N);
	m = sqrt(m);
	m = m * 10000;  /* m =m*1000 */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate angle rms value */
/* function		::	calculate_Angle_rms */
/* input			::	p,q */
/* output		::	calculation value *1000  (angle) xx.xxx (hex MSB is symbol) */
/* call			::	atan2 */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_Angle_rms(int64_t p, int64_t q)
{
	float m;
	int32_t n;

	m = atan2(q, p);
	m = 180 * m;
	m = m * 1000;
	m = m / CONST_Pi;
	n = (int32_t)m;
	if (n < 0) {
		n = ~n + 1;
		n |= 0x80000000;
	}

	return ((uint32_t)(n));
}

/* =================================================================== */
/* description	::	calculate active or reactive power rms value */
/* function		::	calculate_PQ_rms */
/* input			::	val,k_ix,k_vx */
/* output		::	calculation value *10	 (0.1w)  (hex) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_PQ_rms(int64_t val, uint32_t k_ix, uint32_t k_vx)
{
	double m;

	if (val < 0) {
		m = (double)((~val) + 1);
	} else {
		m = (double)(val);
	}

	m = (m / RMS_DIV_Q);                    /* m =m/2^40 sQ23.40 */
	m = (m / VMetrology.DSP_STATUS.N);
	m = (m * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
	m = m * 10;                             /* m =m*10 (0.1w) */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate apparent power rms value */
/* function		::	calculate_S_rms */
/* input			::	p,q */
/* output		::	calculation value *10	 (0.1va) (hex) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_S_rms(int64_t pv, int64_t qv, uint32_t k_ix, uint32_t k_vx)
{
	double m, n;

	if (pv < 0) {
		m = (double)((~pv) + 1);
	} else {
		m = (double)(pv);
	}

	m = (m / RMS_DIV_Q);                    /* m =m/2^40 */
	m = (m / VMetrology.DSP_STATUS.N);
	m = (m * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
	m = m * 10;                             /* m =m*10 (0.1w) */

	if (qv < 0) {
		n = (double)((~qv) + 1);
	} else {
		n = (double)(qv);
	}

	n = (n / RMS_DIV_Q);                    /* n =n/2^40 */
	n = (n / VMetrology.DSP_STATUS.N);
	n = (n * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* n =n*k_v*k_i */
	n = n * 10;                             /* n =n*10 (0.1var) */

	m = m * m; /* p^2 *100 */
	n = n * n; /* q^2 *100 */
	m = sqrt(m + n); /* s=[ 100*((p^2)+(q^2)) ]^0.5 */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	check active or reactive power direction */
/* function		::	_check_PQ_direc */
/* input			::	p,q */
/* output		::	1:reverse, 0:forward */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
static uint32_t _check_PQ_direc(int64_t val)
{
	if (val < 0) {
		return 1;
	} else {
		return 0;
	}
}

/* =================================================================== */
/* description	::	check active or reactive total power direction */
/* function		::	_check_PQt_direc */
/* input			::	pa pb pc */
/* output		::	1:reverse, 0:forward */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
static uint32_t _check_PQt_direc(int64_t val_a, int64_t val_b, int64_t val_c)
{
	if ((val_a + val_b + val_c) < 0) {
		return 1;
	} else {
		return 0;
	}
}

/* =================================================================== */
/* description	::	measure rms value */
/* function		::	_measure_rms */
/* input			::	rms_id */
/* output		::	U = xxxx.xxx (V)                I = xxxx.xxxx (A) */
/*					P = xxxxxxx.x (W)		Q = xxxxxxx.x (Var) */
/*					Fr= xx.xx (Hz) */
/* call			::	calculate_VI_rms,calculate_PQ_rms,_check_PQ_direc */
/* effect		::	VAFE */
/* =================================================================== */
static uint32_t _measure_rms(RMS_TYPE rms_id)
{
	switch (rms_id) {
	case Ua:
	{
		VAFE.RMS[Ua] = _calculate_VI_rms(VMetrology.DSP_ACC.V_A, VMetrology.DSP_CTRL.K_VA);
	}
	break;

	case Ub:
	{
		VAFE.RMS[Ub] = _calculate_VI_rms(VMetrology.DSP_ACC.V_B, VMetrology.DSP_CTRL.K_VB);
	}
	break;

	case Uc:
	{
		VAFE.RMS[Uc] = _calculate_VI_rms(VMetrology.DSP_ACC.V_C, VMetrology.DSP_CTRL.K_VC);
	}
	break;

	case Ia:
	{
		VAFE.RMS[Ia] = _calculate_VI_rms(VMetrology.DSP_ACC.I_A, VMetrology.DSP_CTRL.K_IA);
	}
	break;

	case Ib:
	{
		VAFE.RMS[Ib] = _calculate_VI_rms(VMetrology.DSP_ACC.I_B, VMetrology.DSP_CTRL.K_IB);
	}
	break;

	case Ic:
	{
		VAFE.RMS[Ic] = _calculate_VI_rms(VMetrology.DSP_ACC.I_C, VMetrology.DSP_CTRL.K_IC);
	}
	break;

	case Ini:
	{
		VAFE.RMS[Ini] = _calculate_Inx_rms(VMetrology.DSP_ACC.I_Ni);
	}
	break;

	case Inm:
	{
		VAFE.RMS[Inm] = _calculate_VI_rms(VMetrology.DSP_ACC.I_Nm, VMetrology.DSP_CTRL.K_IN);
	}
	break;

	case Inmi:
	{
		VAFE.RMS[Inmi] = _calculate_Inx_rms(VMetrology.DSP_ACC.I_Nmi);
	}
	break;

	case Pa:
	{
		VAFE.RMS[Pa] = _calculate_PQ_rms(VMetrology.DSP_ACC.P_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA);
		VAFE.ST.BIT.pa_dir = _check_PQ_direc(VMetrology.DSP_ACC.P_A);
	}
	break;

	case Pb:
	{
		VAFE.RMS[Pb] = _calculate_PQ_rms(VMetrology.DSP_ACC.P_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB);
		VAFE.ST.BIT.pb_dir = _check_PQ_direc(VMetrology.DSP_ACC.P_B);
	}
	break;

	case Pc:
	{
		VAFE.RMS[Pc] = _calculate_PQ_rms(VMetrology.DSP_ACC.P_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC);
		VAFE.ST.BIT.pc_dir = _check_PQ_direc(VMetrology.DSP_ACC.P_C);
	}
	break;

	case Qa:
	{
		VAFE.RMS[Qa] = _calculate_PQ_rms(VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA);
		VAFE.ST.BIT.qa_dir = _check_PQ_direc(VMetrology.DSP_ACC.Q_A);
	}
	break;

	case Qb:
	{
		VAFE.RMS[Qb] = _calculate_PQ_rms(VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB);
		VAFE.ST.BIT.qb_dir = _check_PQ_direc(VMetrology.DSP_ACC.Q_B);
	}
	break;

	case Qc:
	{
		VAFE.RMS[Qc] = _calculate_PQ_rms(VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC);
		VAFE.ST.BIT.qc_dir = _check_PQ_direc(VMetrology.DSP_ACC.Q_C);
	}
	break;

	case Sa:
	{
		VAFE.RMS[Sa] = _calculate_S_rms(VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA);
	}
	break;

	case Sb:
	{
		VAFE.RMS[Sb] = _calculate_S_rms(VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB);
	}
	break;

	case Sc:
	{
		VAFE.RMS[Sc] = _calculate_S_rms(VMetrology.DSP_ACC.P_C, VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC);
	}
	break;

	case Pt:
	{
		VAFE.RMS[Pt] = VAFE.RMS[Pa] + VAFE.RMS[Pb] + VAFE.RMS[Pc];
		VAFE.ST.BIT.pt_dir = _check_PQt_direc(VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.P_C);
	}
	break;

	case Qt:
	{
		VAFE.RMS[Qt] = VAFE.RMS[Qa] + VAFE.RMS[Qb] + VAFE.RMS[Qc];
		VAFE.ST.BIT.qt_dir = _check_PQt_direc(VMetrology.DSP_ACC.Q_A, VMetrology.DSP_ACC.Q_B, VMetrology.DSP_ACC.Q_C);
	}
	break;

	case St:
	{
		VAFE.RMS[St] = VAFE.RMS[Sa] + VAFE.RMS[Sb] + VAFE.RMS[Sc];
	}
	break;

	case Freq:
	{
		VAFE.RMS[Freq] = ((VMetrology.DSP_STATUS.FREQ * 100) >> FREQ_Q);       /* xx.xx (Hz) */
	}
	break;

	case AngleA:
	{
		VAFE.RMS[AngleA] = _calculate_Angle_rms(VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.Q_A); /* MSB is symbol */
	}
	break;

	case AngleB:
	{
		VAFE.RMS[AngleB] = _calculate_Angle_rms(VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.Q_B); /* MSB is symbol */
	}
	break;

	case AngleC:
	{
		VAFE.RMS[AngleC] = _calculate_Angle_rms(VMetrology.DSP_ACC.P_C, VMetrology.DSP_ACC.Q_C); /* MSB is symbol */
	}
	break;

	case AngleN:
	{
		VAFE.RMS[AngleN] = _calculate_Angle_rms(VMetrology.DSP_ACC.P_N, VMetrology.DSP_ACC.Q_N); /* MSB is symbol */
	}
	break;
	}
	return (1);
}

/* =================================================================== */
/* description	::	calculate active or reactive energy */
/* function		::	calculate_pq_energy */
/* input			::	mode: 1=alg,0=abs */
/*				::	id:	0=active,1=reactive */
/* output		::	energy (wh) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
static uint32_t _calculate_pq_energy(M_ENERGY_TYPE id, M_ENERGY_TYPE mode)
{
	double m, k;

	m = 0;
	k = 0;
	if (id == PEnergy) {
		/* active energy */
		if (mode == AbsAdd) {
			/* abs */
			if (VMetrology.DSP_ACC.P_A < 0) {
				m = (double)((~VMetrology.DSP_ACC.P_A) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.P_A);
			}

			m = (m * VMetrology.DSP_CTRL.K_IA * VMetrology.DSP_CTRL.K_VA) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
			if (VMetrology.DSP_ACC.P_B < 0) {
				m = (double)((~VMetrology.DSP_ACC.P_B) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.P_B);
			}

			m = (m * VMetrology.DSP_CTRL.K_IB * VMetrology.DSP_CTRL.K_VB) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
			if (VMetrology.DSP_ACC.P_C < 0) {
				m += (double)((~VMetrology.DSP_ACC.P_C) + 1);
			} else {
				m += (double)(VMetrology.DSP_ACC.P_C);
			}

			m = (m * VMetrology.DSP_CTRL.K_IC * VMetrology.DSP_CTRL.K_VC) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
		} else {
			/* algebra */
		}
	} else {
		 /* reactive energy */
		if (mode == 0) {
			/* abs */
			if (VMetrology.DSP_ACC.Q_A < 0) {
				m = (double)((~VMetrology.DSP_ACC.Q_A) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_A);
			}

			m = (m * VMetrology.DSP_CTRL.K_IA * VMetrology.DSP_CTRL.K_VA) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
			if (VMetrology.DSP_ACC.Q_B < 0) {
				m = (double)((~VMetrology.DSP_ACC.Q_B) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_B);
			}

			m = (m * VMetrology.DSP_CTRL.K_IB * VMetrology.DSP_CTRL.K_VB) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
			if (VMetrology.DSP_ACC.Q_C < 0) {
				m = (double)((~VMetrology.DSP_ACC.Q_C) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_C);
			}

			m = (m * VMetrology.DSP_CTRL.K_IC * VMetrology.DSP_CTRL.K_VC) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);            /* k =k/2^40 */
			k += (m / VMetrology.DSP_STATUS.N);
		} else {
			/* algebra */
		}
	}

	/* k =(k/RMS_DIV_Q);  //k =k/2^40 */
	/* k =k/4000;         //k =k/4000 */
	k = k / 3600;         /* xxxxxx (Wh/Varh) */
	k = k * 10000;        /* *10000 (kWh/kVarh) */

	return ((uint32_t)(k));  /* xxxx (kWh/kVarh) */
}

/**
 * \brief Metrology initialization.
 *
 * \return 1 in case of SUCCESS, 0 otherwise.
 * \note External memory must have been previously initialized.
 * Steps for starting the Metrology DSP should be:
 *	    (1) change CONTROL command state to RESET;
 *	    (2) wait for DSP to complete RESET;
 *	    (3) write control registers;
 *	    (4) change CONTROL command state to INIT;
 *	    (5) wait for DSP to complete INIT;
 *          (6) change CONTROL command to RUN;
 *	    (7) wait for DSP is RUNNING.
 */
uint32_t MetrologyInit(void)
{
	uint32_t i;
	uint8_t mem_update = 0;

	/* Init Calibration Parameters */
	memset(&VCalibration, 0, sizeof(VCalibration));
	/* Init AFE data */
	memset(&VAFE, 0, sizeof(VAFE));

	/* ---1---wait for DSP is Reset------------------------- */
	for (i = 0; i < 300; i++) {
		if (VMetrology.DSP_INIT_FLAG == 0x68) {
			break;
		}

		delay_ms(1);
	}

	if (i >= 300) {
		return (0);
	}

	__disable_irq();
	VMetrology.DSP_INIT_FLAG = 0;
	__enable_irq();

	/* Read Metrology data from External memory */
	ExtMemRead(MEM_REG_METROLOGY_ID, &VMetrology);
	_metrology_set_ctrl_status(iSReset);
        if (VMetrology.DSP_INIT_FLAG == 0xFFFFFFFF || VMetrology.DSP_CTRL.ATSENSE_CTRL_28_2B.WORD != 0x03 ) {
		/* External Memory is empty or stored values are not correct */
		MetrologyLoadDefault();
		mem_update = 1;
	}

	/* Check Capture Memory Address and buffer size*/
	if (( VMetrology.DSP_CTRL.CAPTURE_ADDR < ( uint32_t )(&VCapture_Buff[0])) || ( ((VMetrology.DSP_CTRL.CAPTURE_ADDR + VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.WORD*4) > (MET_CAPTURE_BUFF_LEN*4 + ( uint32_t )(&VCapture_Buff[0]))) )) {
	/*if (VMetrology.DSP_CTRL.CAPTURE_ADDR != (uint32_t)VCapture_Buff) { */
		/* update capture address */
		VMetrology.DSP_CTRL.CAPTURE_ADDR = (uint32_t)VCapture_Buff;
		VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.BIT.CP_BUF_SIZE = MET_CAPTURE_BUFF_LEN;
		mem_update = 1;
	}

	/* ----2---set DSP control structure-------------------- */
	UtilsMemCpy((void *)ptr_mem_reg_in, (void *)&VMetrology.DSP_CTRL, sizeof(VMetrology.DSP_CTRL));

	VMetrology.DSP_ACC.ACC_T0 = 0;
	VMetrology.DSP_ACC.ACC_T1 = 0;
	VMetrology.DSP_ACC.ACC_T2 = 0;

	/* ----3---set DSP_CTRL_ST_CTRL = Init------------------- */
	_metrology_set_ctrl_status(iSInit);

	/* ---4---wait for DSP is Initialize-------------------- */
	for (i = 0; i < 1500; i++) {
		if (ptr_mem_reg_out->STATUS.BIT.ST >= STATUS_READY) {
			break;
		}

		delay_ms(1);
	}

	if (i >= 1500) {
		return (0);
	}

	/* ----5---set DSP_CTRL_ST_CTRL = Run-------------------- */
	_metrology_set_ctrl_status(iSRun);

	/* ---6---wait for DSP Running---------------------------- */
	for (i = 0; i < 600; i++) {
		if (ptr_mem_reg_out->STATUS.BIT.ST == STATUS_DSP_RUNNING) {
			break;
		}

		delay_ms(1);
	}

	if (i >= 600) {
		return (0);
	}

	if (mem_update) {
		/* Update Metrology data to External memory */
		ExtMemWrite(MEM_REG_METROLOGY_ID, &VMetrology);
	}

	return (1);
}

/**
 * \brief Load default values of metrology.
 */
void MetrologyLoadDefault(void)
{
	VMetrology.DSP_INIT_FLAG = 0;

	UtilsMemCpy((void *)&VMetrology.DSP_CTRL, (void *)&dsp_ctrl_default, sizeof(VMetrology.DSP_CTRL));

	VMetrology.DSP_CTRL.CAPTURE_ADDR = (uint32_t)(&VCapture_Buff[0]);
	VMetrology.DSP_ACC.ACC_T0 = 0;
	VMetrology.DSP_ACC.ACC_T1 = 0;
	VMetrology.DSP_ACC.ACC_T2 = 0;
}

/**
 * \brief Refresh control register data in shared memory.
 */
void MetrologyRefreshCrtl(void)
{
	UtilsMemCpy((void *)ptr_mem_reg_in, (void *)&VMetrology.DSP_CTRL, sizeof(VMetrology.DSP_CTRL));
}

/**
 * \brief Update Metrology data in external memory.
 */
uint16_t MetrologyUpdateExtMem(void)
{
	uint16_t size;

	/* Update Metrology data to External memory */
	size = ExtMemWrite(MEM_REG_METROLOGY_ID, &VMetrology);
	return size;
}

/**
 * \brief Calibrate meter initialize.
 *
 * \param ptr  Pointer to calibration parameters
 */
void MetrologyCalibMeterInit(void)
{
	uint64_t m;
	uint32_t i;
	metrology_cal_t *ptr;

	/* ----load default -------------------------- */
	MetrologyLoadDefault();

	ptr = &VCalibration;

	/* ---------calculate P_K_t------------------- */
	m = 1000000000 / (ptr->mc); /* p_t_t =(1000/MC)*2^24 */
	m = (m << GAIN_P_K_T_Q);
	m = m / 1000000;
	VMetrology.DSP_CTRL.P_K_T = (uint32_t)(m);
	VMetrology.DSP_CTRL.Q_K_T = VMetrology.DSP_CTRL.P_K_T;
	VMetrology.DSP_CTRL.I_K_T = VMetrology.DSP_CTRL.P_K_T;

	/* ---------configure sensor_type------------- */
	VMetrology.DSP_CTRL.METER_TYPE.BIT.SENSOR_TYPE_I_A = ptr->sensortype;
	VMetrology.DSP_CTRL.METER_TYPE.BIT.SENSOR_TYPE_I_B = ptr->sensortype;
	VMetrology.DSP_CTRL.METER_TYPE.BIT.SENSOR_TYPE_I_C = ptr->sensortype;

	/* -------configure pga gain------------------ */
	if (ptr->gain_i == 8) {
		i = 3;
	} else {
		i = ptr->gain_i >> 1;
	}

	VMetrology.DSP_CTRL.ATSENSE_CTRL_20_23.BIT.I0_GAIN = i;
	VMetrology.DSP_CTRL.ATSENSE_CTRL_20_23.BIT.I1_GAIN = i;
	VMetrology.DSP_CTRL.ATSENSE_CTRL_20_23.BIT.I2_GAIN = i;
	VMetrology.DSP_CTRL.ATSENSE_CTRL_24_27.BIT.I3_GAIN = i;

	/* --------configure K_Ix--------------------------------- */
	if (ptr->sensortype == CT) {
		m = (ptr->k_i);
		m = m * 100 * 1000;     /* Rl*100, Te*1000 */
		m = m / ((ptr->gain_i) * (ptr->rl));
		m = (m << GAIN_VI_Q);
		m = m / 1000000;
		i = (uint32_t)(m);
	} else if (ptr->sensortype == ROGOWSKI) {
		m = (ptr->freq);        /* freq*100 */
		m = (((m * 1000000) * 1000) / ((ptr->k_i) * (ptr->gain_i) * 6));        /* k_ix=f/(k_i*pga*60) */
		m = (m << GAIN_VI_Q);
		m = (m + 500) / 1000;
		i = (uint32_t)(m);
	} else if (ptr->sensortype == SHUNT) {
		m = ((uint64_t)1000000 * (uint64_t)100 * (uint64_t)10000) / ((ptr->gain_i) * (ptr->rl));
		m = (m << GAIN_VI_Q) / 10000;
		i = (uint32_t)(m);
	}

	VMetrology.DSP_CTRL.K_IA = i;
	VMetrology.DSP_CTRL.K_IB = i;
	VMetrology.DSP_CTRL.K_IC = i;
	VMetrology.DSP_CTRL.K_IN = i;

	/* --------configure K_Ux--------------------------------- */
	VMetrology.DSP_CTRL.K_VA = (((ptr->k_u) << GAIN_VI_Q) / 1000);   /* k_u *1000*/
	VMetrology.DSP_CTRL.K_VB = (((ptr->k_u) << GAIN_VI_Q) / 1000);
	VMetrology.DSP_CTRL.K_VC = (((ptr->k_u) << GAIN_VI_Q) / 1000);

	VMetrology.DSP_CTRL.STATE_CTRL.WORD = STATE_CTRL_Run;

	/* ---save default to external memory ------------------ */
	ExtMemWrite(MEM_REG_METROLOGY_ID, &VMetrology);

	/* ------------freshing metrology configure------------- */
	MetrologyRefreshCrtl();
}

/**
 * \brief Calibrate meter.
 *
 * \param ptr  Pointer to calibration parameters
 */
void MetrologyCalibMeter(void)
{
	uint64_t m;
	uint32_t k;
	metrology_cal_t *ptr;
	int64_t measured_angle, correction_angle;

	ptr = &VCalibration;

	if (ptr->dsp_update_num != 3) {
		/* Check VCalibration data*/
		if (ptr->mc == 0 || ptr->freq == 0 || ptr->k_i == 0 || ptr->k_u == 0) {
			if (pf_met_cal_callback) {
				pf_met_cal_callback(1);
			}
			ptr->dsp_update_num = 0;
			return;
		}

		ptr->dsp_acc_ia = 0;
		ptr->dsp_acc_ib = 0;
		ptr->dsp_acc_ic = 0;
		ptr->dsp_acc_in = 0;
		ptr->dsp_acc_ua = 0;
		ptr->dsp_acc_ub = 0;
		ptr->dsp_acc_uc = 0;
		ptr->dsp_acc_pa = 0;
		ptr->dsp_acc_pb = 0;
		ptr->dsp_acc_pc = 0;
		ptr->dsp_acc_qa = 0;
		ptr->dsp_acc_qb = 0;
		ptr->dsp_acc_qc = 0;

		ptr->dsp_update_num = 10;

		/* Save FEATURE_CTRL0 register value, to be restored after calibration */
		ptr->feature_ctrl0_copy = VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD;

		switch (ptr->line_id) {
		case Ph_A:
			VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD = 0x111;
			/* Initialize calibration registers to the default values */
			VMetrology.DSP_CTRL.CAL_M_IA=dsp_ctrl_default.CAL_M_IA;
			ptr_mem_reg_in->CAL_M_IA = VMetrology.DSP_CTRL.CAL_M_IA;
			VMetrology.DSP_CTRL.CAL_M_VA=dsp_ctrl_default.CAL_M_VA;
			ptr_mem_reg_in->CAL_M_VA = VMetrology.DSP_CTRL.CAL_M_VA;
			VMetrology.DSP_CTRL.CAL_PH_IA=dsp_ctrl_default.CAL_PH_IA;
			ptr_mem_reg_in->CAL_PH_IA = VMetrology.DSP_CTRL.CAL_PH_IA;
			break;

		case Ph_B:
			VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD = 0x223;
			/* Initialize calibration registers to the default values */
			VMetrology.DSP_CTRL.CAL_M_IB=dsp_ctrl_default.CAL_M_IB;
			ptr_mem_reg_in->CAL_M_IB = VMetrology.DSP_CTRL.CAL_M_IB;
			VMetrology.DSP_CTRL.CAL_M_VB=dsp_ctrl_default.CAL_M_VB;
			ptr_mem_reg_in->CAL_M_VB = VMetrology.DSP_CTRL.CAL_M_VB;
			VMetrology.DSP_CTRL.CAL_PH_IB=dsp_ctrl_default.CAL_PH_IB;
			ptr_mem_reg_in->CAL_PH_IB = VMetrology.DSP_CTRL.CAL_PH_IB;
			break;

		case Ph_C:
			VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD = 0x435;
			/* Initialize calibration registers to the default values */
			VMetrology.DSP_CTRL.CAL_M_IC=dsp_ctrl_default.CAL_M_IC;
			ptr_mem_reg_in->CAL_M_IC = VMetrology.DSP_CTRL.CAL_M_IC;
			VMetrology.DSP_CTRL.CAL_M_VC=dsp_ctrl_default.CAL_M_VC;
			ptr_mem_reg_in->CAL_M_VC = VMetrology.DSP_CTRL.CAL_M_VC;
			VMetrology.DSP_CTRL.CAL_PH_IC=dsp_ctrl_default.CAL_PH_IC;
			ptr_mem_reg_in->CAL_PH_IC = VMetrology.DSP_CTRL.CAL_PH_IC;
			break;

		default:
			VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD = 0x711;
			/* Initialize calibration registers to the default values */
			VMetrology.DSP_CTRL.CAL_M_IA=dsp_ctrl_default.CAL_M_IA;
			ptr_mem_reg_in->CAL_M_IA = VMetrology.DSP_CTRL.CAL_M_IA;
			VMetrology.DSP_CTRL.CAL_M_VA=dsp_ctrl_default.CAL_M_VA;
			ptr_mem_reg_in->CAL_M_VA = VMetrology.DSP_CTRL.CAL_M_VA;
			VMetrology.DSP_CTRL.CAL_PH_IA=dsp_ctrl_default.CAL_PH_IA;
			ptr_mem_reg_in->CAL_PH_IA = VMetrology.DSP_CTRL.CAL_PH_IA;

			VMetrology.DSP_CTRL.CAL_M_IB=dsp_ctrl_default.CAL_M_IB;
			ptr_mem_reg_in->CAL_M_IB = VMetrology.DSP_CTRL.CAL_M_IB;
			VMetrology.DSP_CTRL.CAL_M_VB=dsp_ctrl_default.CAL_M_VB;
			ptr_mem_reg_in->CAL_M_VB = VMetrology.DSP_CTRL.CAL_M_VB;
			VMetrology.DSP_CTRL.CAL_PH_IB=dsp_ctrl_default.CAL_PH_IB;
			ptr_mem_reg_in->CAL_PH_IB = VMetrology.DSP_CTRL.CAL_PH_IB;

			VMetrology.DSP_CTRL.CAL_M_IC=dsp_ctrl_default.CAL_M_IC;
			ptr_mem_reg_in->CAL_M_IC = VMetrology.DSP_CTRL.CAL_M_IC;
			VMetrology.DSP_CTRL.CAL_M_VC=dsp_ctrl_default.CAL_M_VC;
			ptr_mem_reg_in->CAL_M_VC = VMetrology.DSP_CTRL.CAL_M_VC;
			VMetrology.DSP_CTRL.CAL_PH_IC=dsp_ctrl_default.CAL_PH_IC;
			ptr_mem_reg_in->CAL_PH_IC = VMetrology.DSP_CTRL.CAL_PH_IC;
			break;
		}

		ptr_mem_reg_in->FEATURE_CTRL0.WORD = VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD;

	} else {

		ptr->dsp_update_num = 0;

		switch (ptr->line_id) {
			case Ph_A:
			{
				/* ---------calibration I RMS----------------------------- */
				ptr->dsp_acc_ia = ptr->dsp_acc_ia >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ia, VMetrology.DSP_CTRL.K_IA); /* *10000 */
				m = ptr->aim_ia; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IA = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IA += 1;
				}

				/* ---------calibration V RMS----------------------------- */
				ptr->dsp_acc_ua = ptr->dsp_acc_ua >> 2; /* get average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ua, VMetrology.DSP_CTRL.K_VA); /* *10000 */
				m = ptr->aim_va; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VA = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VA += 1;
				}

				/* ---------calibration Phase ---------------------------- */
				ptr->dsp_acc_pa = ptr->dsp_acc_pa >> 2;
				ptr->dsp_acc_qa = ptr->dsp_acc_qa >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pa, ptr->dsp_acc_qa); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_a;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IA = (uint32_t)(m);

				ptr_mem_reg_in->CAL_M_IA = VMetrology.DSP_CTRL.CAL_M_IA;
				ptr_mem_reg_in->CAL_M_VA = VMetrology.DSP_CTRL.CAL_M_VA;
				ptr_mem_reg_in->CAL_PH_IA = VMetrology.DSP_CTRL.CAL_PH_IA;

			}
			break;

			case Ph_B:
			{
				/* ---------calibration I RMS----------------------------- */
				ptr->dsp_acc_ib = ptr->dsp_acc_ib >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ib, VMetrology.DSP_CTRL.K_IB); /* *10000 */
				m = ptr->aim_ib; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IB = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IB += 1;
				}

				/* ---------calibration V RMS----------------------------- */
				ptr->dsp_acc_ub = ptr->dsp_acc_ub >> 2; /* get average */
				k = _calculate_VI_rms(ptr->dsp_acc_ub, VMetrology.DSP_CTRL.K_VB); /* *10000 */
				m = ptr->aim_vb; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VB = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VB += 1;
				}

				/* ---------calibration Phase ---------------------------- */
				ptr->dsp_acc_pb = ptr->dsp_acc_pb >> 2; /* get average */
				ptr->dsp_acc_qb = ptr->dsp_acc_qb >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pb, ptr->dsp_acc_qb); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_b;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IB = (uint32_t)(m);

				ptr_mem_reg_in->CAL_M_IB = VMetrology.DSP_CTRL.CAL_M_IB;
				ptr_mem_reg_in->CAL_M_VB = VMetrology.DSP_CTRL.CAL_M_VB;
				ptr_mem_reg_in->CAL_PH_IB = VMetrology.DSP_CTRL.CAL_PH_IB;

			}
			break;

			case Ph_C:
			{
				/* ---------calibration I RMS----------------------------- */
				ptr->dsp_acc_ic = ptr->dsp_acc_ic >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ic, VMetrology.DSP_CTRL.K_IC); /* *10000 */
				m = ptr->aim_ic; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IC = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IC += 1;
				}

				/* ---------calibration V RMS----------------------------- */
				ptr->dsp_acc_uc = ptr->dsp_acc_uc >> 2; /* get average */
				k = _calculate_VI_rms(ptr->dsp_acc_uc, VMetrology.DSP_CTRL.K_VC); /* *10000 */
				m = ptr->aim_vc; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VC = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VC += 1;
				}

				/* ---------calibration Phase ---------------------------- */
				ptr->dsp_acc_pc = ptr->dsp_acc_pc >> 2; /* get average */
				ptr->dsp_acc_qc = ptr->dsp_acc_qc >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pc, ptr->dsp_acc_qc); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_c;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IC = (uint32_t)(m);

				ptr_mem_reg_in->CAL_M_IC = VMetrology.DSP_CTRL.CAL_M_IC;
				ptr_mem_reg_in->CAL_M_VC = VMetrology.DSP_CTRL.CAL_M_VC;
				ptr_mem_reg_in->CAL_PH_IC = VMetrology.DSP_CTRL.CAL_PH_IC;

			}
			break;

			case Ph_T:
			default:
			{
				/* ---------calibration I RMS----------------------------- */
				ptr->dsp_acc_ia = ptr->dsp_acc_ia >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ia, VMetrology.DSP_CTRL.K_IA); /* *10000 */
				m = ptr->aim_ia; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IA = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IA += 1;
				}

				ptr->dsp_acc_ib = ptr->dsp_acc_ib >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ib, VMetrology.DSP_CTRL.K_IB); /* *10000 */
				m = ptr->aim_ib; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IB = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IB += 1;
				}

				ptr->dsp_acc_ic = ptr->dsp_acc_ic >> 2; /* get Average value */
				k = _calculate_VI_rms(ptr->dsp_acc_ic, VMetrology.DSP_CTRL.K_IC); /* *10000 */
				m = ptr->aim_ic; /* aim_i is *10000 */
				m = m << CAL_VI_Q; /* new_cal_m_ix=(aim_i/measure_i)*old_cal_m_ix ->0x20000000 Q=29 */
				VMetrology.DSP_CTRL.CAL_M_IC = (uint32_t)(m / k); /* get Integer */
				if ((((m % k) * 10) / k) > 4) {
					VMetrology.DSP_CTRL.CAL_M_IC += 1;
				}

				/* ---------calibration V RMS----------------------------- */
				ptr->dsp_acc_ua = ptr->dsp_acc_ua >> 2; /* get average */
				k = _calculate_VI_rms(ptr->dsp_acc_ua, VMetrology.DSP_CTRL.K_VA); /* *10000 */
				m = ptr->aim_va; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VA = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VA += 1;
				}

				ptr->dsp_acc_ub = ptr->dsp_acc_ub >> 2; /* get average */
				k = _calculate_VI_rms(ptr->dsp_acc_ub, VMetrology.DSP_CTRL.K_VB); /* *10000 */
				m = ptr->aim_vb; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VB = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VB += 1;
				}

				ptr->dsp_acc_uc = ptr->dsp_acc_uc >> 2; /* get average */
				k = _calculate_VI_rms(ptr->dsp_acc_uc, VMetrology.DSP_CTRL.K_VC); /* *10000 */
				m = ptr->aim_vc; /* *1000 */
				m = m << CAL_VI_Q; /* sQ2.29 cal_m_vx->0x20000000 */
				VMetrology.DSP_CTRL.CAL_M_VC = (uint32_t)(m / (k / 10));   /* get Integer */
				if (((m % (k / 10)) * 10) / (k / 10) > 4) {
					VMetrology.DSP_CTRL.CAL_M_VC += 1;
				}

				/* ------------------------------------------------------- */
				ptr_mem_reg_in->CAL_M_IA = VMetrology.DSP_CTRL.CAL_M_IA;
				ptr_mem_reg_in->CAL_M_IB = VMetrology.DSP_CTRL.CAL_M_IB;
				ptr_mem_reg_in->CAL_M_IC = VMetrology.DSP_CTRL.CAL_M_IC;
				ptr_mem_reg_in->CAL_M_VA = VMetrology.DSP_CTRL.CAL_M_VA;
				ptr_mem_reg_in->CAL_M_VB = VMetrology.DSP_CTRL.CAL_M_VB;
				ptr_mem_reg_in->CAL_M_VC = VMetrology.DSP_CTRL.CAL_M_VC;

				/* ---------calibration Phase ---------------------------- */
				ptr->dsp_acc_pa = ptr->dsp_acc_pa >> 2; /* get average */
				ptr->dsp_acc_qa = ptr->dsp_acc_qa >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pa, ptr->dsp_acc_qa); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_a;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IA = (uint32_t)(m);

				ptr_mem_reg_in->CAL_PH_IA = VMetrology.DSP_CTRL.CAL_PH_IA;

				ptr->dsp_acc_pb = ptr->dsp_acc_pb >> 2; /* get average */
				ptr->dsp_acc_qb = ptr->dsp_acc_qb >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pb, ptr->dsp_acc_qb); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_b;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IB = (uint32_t)(m);

				ptr_mem_reg_in->CAL_PH_IB = VMetrology.DSP_CTRL.CAL_PH_IB;

				ptr->dsp_acc_pc = ptr->dsp_acc_pc >> 2; /* get average */
				ptr->dsp_acc_qc = ptr->dsp_acc_qc >> 2;
				k = _calculate_Angle_rms(ptr->dsp_acc_pc, ptr->dsp_acc_qc); /* *1000 */
				measured_angle = k;
				if (k & 0x80000000) {		/* Negative angle (MSB is the sign) */
					measured_angle = 0x80000000 - measured_angle;
				}
				correction_angle = measured_angle - ptr->angle_c;
				/* Correction angle should be between -180 and 180 degrees */
				while (correction_angle < (-180000)) {
					correction_angle += 360000;
				}
				while (correction_angle > 180000 ) {
					correction_angle -= 360000;
				}
				correction_angle = (correction_angle * 7158278827) / ptr->freq;  /* The angles should be normalized to 60 Hz and then converted to BAMS (1 degree is equal to 2^31/180 = 11930464.7111 BAMS) */
				correction_angle = (correction_angle + 50) / 100;
				m = (uint64_t)(abs(correction_angle));
				if (correction_angle < 0) {
					m = (~m) + 1;
				}

				VMetrology.DSP_CTRL.CAL_PH_IC = (uint32_t)(m);

				ptr_mem_reg_in->CAL_PH_IC = VMetrology.DSP_CTRL.CAL_PH_IC;
				__NOP();
				__NOP();

			}
		} /* switch(ph_id) */

		/* Restore FEATURE_CTRL0 */
		VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD = ptr->feature_ctrl0_copy;
		ptr_mem_reg_in->FEATURE_CTRL0.WORD = VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD;

		/* Update calibration data to External memory */
		ExtMemWrite(MEM_REG_METROLOGY_ID, &VMetrology);

		if (pf_met_cal_callback) {
			pf_met_cal_callback(0);
		}
	}
}

/**
 * \brief Metrology Process.
 */
void MetrologyProcess(void)
{
	if (VCalibration.dsp_update_num < 10 && VCalibration.dsp_update_num > 4) {
		VCalibration.dsp_acc_ia += VMetrology.DSP_ACC.I_A;
		VCalibration.dsp_acc_ib += VMetrology.DSP_ACC.I_B;
		VCalibration.dsp_acc_ic += VMetrology.DSP_ACC.I_C;

		VCalibration.dsp_acc_ua += VMetrology.DSP_ACC.V_A;
		VCalibration.dsp_acc_ub += VMetrology.DSP_ACC.V_B;
		VCalibration.dsp_acc_uc += VMetrology.DSP_ACC.V_C;

		VCalibration.dsp_acc_pa += VMetrology.DSP_ACC.P_A;
		VCalibration.dsp_acc_pb += VMetrology.DSP_ACC.P_B;
		VCalibration.dsp_acc_pc += VMetrology.DSP_ACC.P_C;

		VCalibration.dsp_acc_qa += VMetrology.DSP_ACC.Q_A;
		VCalibration.dsp_acc_qb += VMetrology.DSP_ACC.Q_B;
		VCalibration.dsp_acc_qc += VMetrology.DSP_ACC.Q_C;
	}

	if (VCalibration.dsp_update_num != 0) {
		if ((--VCalibration.dsp_update_num) == 5) {
			VCalibration.dsp_update_num = 3;
			/* 4 integration periods elapsed, for increasing the calibration accuracy --> triggle calibration msg */
			TaskPutIntoQueue(MetrologyCalibMeter);
		}
	}

	_measure_rms(Ua);
	_measure_rms(Ub);
	_measure_rms(Uc);

	_measure_rms(Ia);
	_measure_rms(Ib);
	_measure_rms(Ic);

	_measure_rms(Ini);
	_measure_rms(Inm);
	_measure_rms(Inmi);

	_measure_rms(Pa);
	_measure_rms(Pb);
	_measure_rms(Pc);

	_measure_rms(Qa);
	_measure_rms(Qb);
	_measure_rms(Qc);

	_measure_rms(Sa);
	_measure_rms(Sb);
	_measure_rms(Sc);

	_measure_rms(Freq);

	_measure_rms(Pt);
	_measure_rms(Qt);
	_measure_rms(St);

	_measure_rms(AngleA);
	_measure_rms(AngleB);
	_measure_rms(AngleC);
	_measure_rms(AngleN);

	VAFE.energy += _calculate_pq_energy(PEnergy, AbsAdd);

	LOG_APP_DEMO_DEBUG(("Metrology Process: Update VAFE.energy[0x%08x]\r\n", VAFE.energy));
}

/**
 * \brief Set harmonic analysis order.
 *
 * \param order  Harmonic order
 */
void MetrologySetHarmonicOrder(uint8_t order)
{
	VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_m_REQ = order;
	VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_EN = 1;

	ptr_mem_reg_in->FEATURE_CTRL1.BIT.HARMONIC_m_REQ = order;
	ptr_mem_reg_in->FEATURE_CTRL1.BIT.HARMONIC_EN = 1;

	VCalibration.har_order = order | 0x80;	// MSB used to ensure a whole integration period has elapsed
}

/**
 * \brief Get harmonic analysis order.
 *
 * \return Harmonic order
 */
uint8_t MetrologyGetHarmonicOrder(void)
{
	return VMetrology.DSP_STATUS.STATE_FLAG.BIT.HARMONIC_m_CONF;
}

/**
 * \brief Check if Harmonic analysis is ready.
 *
 * \return Harmonic order
 */
uint8_t MetrologyHarmonicIsReady(void)
{
	if (MetrologyGetHarmonicOrder()==VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_m_REQ && VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_EN==1 && VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_m_REQ!=0 && MetrologyGetHarmonicOrder()==VCalibration.har_order && VCalibration.har_order!=0)  {
		return 1;
	} else {
		return 0;
	}
}

/**
 * \brief Metrology Harmonic analysis process.
 */
void MetrologyHarmonicsProcess(void)
{
	double sqrt2;
	uint64_t div;
	uint32_t *ptr_har = (uint32_t *)ptr_mem_har_out;
	uint8_t i;

	for (i=0; i < DSP_HAR_SIZE; i++, ptr_har++) {
		if (*ptr_har > RMS_HARMONIC) {
			har_temp[i] = (~(*ptr_har)) + 1;
		} else {
			har_temp[i] = *ptr_har;
		}
	}

	sqrt2 = sqrt(2);
	div = VMetrology.DSP_STATUS.N;
	div <<= 6; /* format sQ25.6 */

	VHar_calc.Irms_A_m = ((sqrt2 * sqrt((uint64_t)har_temp[0] * (uint64_t)har_temp[0] + (uint64_t)har_temp[7] * (uint64_t)har_temp[7])) / div);
	VHar_calc.Irms_B_m = ((sqrt2 * sqrt((uint64_t)har_temp[2] * (uint64_t)har_temp[2] + (uint64_t)har_temp[9] * (uint64_t)har_temp[9])) / div);
	VHar_calc.Irms_C_m = ((sqrt2 * sqrt((uint64_t)har_temp[4] * (uint64_t)har_temp[4] + (uint64_t)har_temp[11] * (uint64_t)har_temp[11])) / div);
        VHar_calc.Irms_N_m = ((sqrt2 * sqrt((uint64_t)har_temp[6] * (uint64_t)har_temp[6] + (uint64_t)har_temp[13] * (uint64_t)har_temp[13])) / div);

	VHar_calc.Vrms_A_m = ((sqrt2 * sqrt((uint64_t)har_temp[1] * (uint64_t)har_temp[1] + (uint64_t)har_temp[8] * (uint64_t)har_temp[8])) / div);
	VHar_calc.Vrms_B_m = ((sqrt2 * sqrt((uint64_t)har_temp[3] * (uint64_t)har_temp[3] + (uint64_t)har_temp[10] * (uint64_t)har_temp[10])) / div);
	VHar_calc.Vrms_C_m = ((sqrt2 * sqrt((uint64_t)har_temp[5] * (uint64_t)har_temp[5] + (uint64_t)har_temp[12] * (uint64_t)har_temp[12])) / div);

	if (pf_met_har_callback) {
		pf_met_har_callback(&VHar_calc);
	}

	MetrologySetHarmonicOrder(0);

	VMetrology.DSP_CTRL.FEATURE_CTRL1.BIT.HARMONIC_EN = 0;
	ptr_mem_reg_in->FEATURE_CTRL1.BIT.HARMONIC_EN = 0;
}

/**
 * \brief Set Metrology Harmonic callback.
 *
 * \param har_cb  Callback function to call when Harmonic analysis is completed.
 */
void MetrologySetHarmonicsCallback(pf_har_callback har_cb)
{
	pf_met_har_callback = har_cb;
}

/**
 * \brief Set Metrology Calibration callback.
 *
 * \param cal_cb  Callback function to call when Calibration is completed.
 */
void MetrologySetCalibrationCallback(pf_cal_callback cal_cb)
{
	pf_met_cal_callback = cal_cb;
}

/**
 * \brief Get Capture Buffer Data.
 *
 * \param data  Pointer where is the pointer to the capture buffer.
 * \return Number of the 32bit samples of the capture data.
 */
uint32_t MetrologyGetCaptureData(int32_t **data)
{
	*data = (int32_t *)VMetrology.DSP_CTRL.CAPTURE_ADDR; //VCapture_Buff;

	return VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.BIT.CP_BUF_SIZE;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
