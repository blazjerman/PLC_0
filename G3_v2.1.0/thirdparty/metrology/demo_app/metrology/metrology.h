/**
 * \file
 *
 * \brief Metrology Module Header file.
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

#ifndef METROLOGY_H_INCLUDED
#define METROLOGY_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "compiler.h"

/* ! Waveform capture buffer length register offset */
#define MET_CAPTURE_BUFF_LENGTH_OFFSET      46
/* ! Waveform capture buffer address register offset */
#define MET_CAPTURE_ADDR_OFFSET             47
/* ! Waveform capture buffer length */
#define MET_CAPTURE_BUFF_LEN                8000
/* ! Buffer */
extern int32_t VCapture_Buff[];

typedef enum {
	STATE_CTRL_Os            = 0,
	FEATURE_CTRL0_Os         = 1,
	FEATURE_CTRL1_Os         = 2,
	METER_TYPE_Os            = 3,
	M_Os                     = 4,
	N_MAX_Os                 = 5,
	PULSE0_CTRL_Os           = 6,
	PULSE1_CTRL_Os           = 7,
	PULSE2_CTRL_Os           = 8,
	P_K_T_Os                 = 9,
	Q_K_T_Os                 = 10,
	I_K_T_Os                 = 11,
	CREEP_THR_P_Os           = 12,
	CREEP_THR_Q_Os           = 13,
	CREEP_THR_I_Os           = 14,
	POWER_OFFSET_CTRL_Os     = 15,
	POWER_OFFSET_P_Os        = 16,
	POWER_OFFSET_Q_Os        = 17,
	SWELL_THR_VA_Os          = 18,
	SWELL_THR_VB_Os          = 19,
	SWELL_THR_VC_Os          = 20,
	SAG_THR_VA_Os            = 21,
	SAG_THR_VB_Os            = 22,
	SAG_THR_VC_Os            = 23,
	K_IA_Os                  = 24,
	K_VA_Os                  = 25,
	K_IB_Os                  = 26,
	K_VB_Os                  = 27,
	K_IC_Os                  = 28,
	K_VC_Os                  = 29,
	K_IN_Os                  = 30,
	CAL_M_IA_Os              = 31,
	CAL_M_VA_Os              = 32,
	CAL_M_IB_Os              = 33,
	CAL_M_VB_Os              = 34,
	CAL_M_IC_Os              = 35,
	CAL_M_VC_Os              = 36,
	CAL_M_IN_Os              = 37,
	CAL_PH_IA_Os             = 38,
	CAL_PH_VA_Os             = 39,
	CAL_PH_IB_Os             = 40,
	CAL_PH_VB_Os             = 41,
	CAL_PH_IC_Os             = 42,
	CAL_PH_VC_Os             = 43,
	CAL_PH_IN_Os         	 = 44,
	CAPTURE_CTRL_Os          = 45,
	CAPTURE_BUFF_SIZE_Os     = 46,
	CAPTRUE_ADDR_Os          = 47,
	RESERVED_C48_Os          = 48,
	RESERVED_C49_Os          = 49,
	RESERVED_C50_Os          = 50,
	ATsense_CTRL_20_23_Os    = 51,
	ATsense_CTRL_24_27_Os    = 52,
	ATsense_CTRL_28_2B_Os    = 53,
	RESERVED_C54_Os          = 54,
	POWER_OFFSET_P_A_Os      = 55,
	POWER_OFFSET_P_B_Os      = 56,
	POWER_OFFSET_P_C_Os      = 57,
	POWER_OFFSET_Q_A_Os      = 58,
	POWER_OFFSET_Q_B_Os      = 59,
	POWER_OFFSET_Q_C_Os      = 60,
} DSP_CTRL_OS_TYPE;

/* =================================================================== */
/* ---Primary 32-bit Input Control Register Array Size Definitions---- */
/* ------------------------------------------------------------------- */
/* ---STATE_CTRL 00------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t STATE_CTRL      :       4;     /* status control */
		uint32_t                 :       28;
	} BIT;
} STATE_CTRL_TYPE;

#define STATE_CTRL_Pos                  0
#define STATE_CTRL_Mask                 (0x0F << STATE_CTRL_Pos)
#define STATE_CTRL_Reset                0
#define STATE_CTRL_Init                 1
#define STATE_CTRL_Run                  2

/* ---FEATURE_CTRL0 01---------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t RZC_CHAN_SELECT     : 3; /* Raw Zero-Crossing Channel Select and enable */
		uint32_t RZC_DIR             : 1; /* Raw Zero-Crossing Direction Selection */
		uint32_t SYNCH               : 2; /* Active Voltage Channel Selection */
		uint32_t                     : 2;
		uint32_t PHASE_A_EN          : 1; /* Phase A enable */
		uint32_t PHASE_B_EN          : 1; /* Phase B enable */
		uint32_t PHASE_C_EN          : 1; /* Phase C enable */
		uint32_t                     : 1;
		uint32_t MAX_INT_SELECT      : 1; /* Max Integration period select */
		uint32_t                     : 1;
		uint32_t F9_PULSE_SEL        : 1; /* Meter_Type_09 Pulse Select */
		uint32_t                     : 16;
		/* //    uint32_t			            : 15; */
		/* //    uint32_t	DEBUG_METROLOGY     : 1; // set this bit while debugging core-1 code, to avoid application (core-0) reset of metrology (core-1). */
	} BIT;
} FEATURE_CTRL0_TYPE;

#define RZC_CHAN_SELECT_Pos             0
#define RZC_CHAN_SELECT_Mask            (0x07 << RZC_CHAN_SELECT_Pos)
#define RZC_CHAN_SELECT_Dis             0x00
#define RZC_CHAN_SELECT_V1              0x01
#define RZC_CHAN_SELECT_V2              0x03
#define RZC_CHAN_SELECT_V3              0x05

#define RZC_DIR_Pos                     3
#define RZC_DIR_Mask                    (0x01 << RZC_DIR_Pos)
#define RZC_DIR_Positive                0x00
#define RZC_DIR_Negative                0x01

#define SYNCH_Pos                       4
#define SYNCH_Mask                      (0x03 << SYNCH_Pos)
#define SYNCH_Active_Phase              0x00
#define SYNCH_Phase_A                   0x01
#define SYNCH_Phase_B                   0x02
#define SYNCH_Phase_C                   0x03

#define PHASE_A_EN_Pos                  8
#define PHASE_A_EN_Mask                 (0x01 << PHASE_A_EN_Pos)
#define PHASE_A_EN_Dis                  0x00
#define PHASE_A_EN_En                   0x01

#define PHASE_B_EN_Pos                  9
#define PHASE_B_EN_Mask                 (0x01 << PHASE_B_EN_Pos)
#define PHASE_B_EN_Dis                  0x00
#define PHASE_B_EN_En                   0x01

#define PHASE_C_EN_Pos                  10
#define PHASE_C_EN_Mask                 (0x01 << PHASE_C_EN_Pos)
#define PHASE_C_EN_Dis                  0x00
#define PHASE_C_EN_En                   0x01

#define MAX_INT_SELECT_Pos              12
#define MAX_INT_SELECT_Mask             (0x01 << MAX_INT_SELECT_Pos)

#define F9_PULSE_SEL_Pos                15
#define F9_PULSE_SEL_Mask               (0x01 << F9_PULSE_SEL_Pos)
#define F9_PULSE_SEL_V1I1               0
#define F9_PULSE_SEL_V1I0               1

/* ---FEATURE_CTRL1 02---------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t CREEP_I_EN      : 1; /* Current creep threshold function enable */
		uint32_t CREEP_Q_EN      : 1; /* Reactive power creep threshold function enable */
		uint32_t CREEP_P_EN      : 1; /* Active power creep threshold function enable */
		uint32_t                 : 5;
		uint32_t HARMONIC_m_REQ  : 6; /* Request number of harmonic for analysis */
		uint32_t                 : 1;
		uint32_t HARMONIC_EN     : 1; /* Enable Harmonic Analysis */
		uint32_t                 : 3;
		uint32_t I_MAX_RESET     : 1; /* Reset All I_x_MAX value */
		uint32_t                 : 3;
		uint32_t V_MAX_RESET     : 1; /* Reset All v_x_MAX values */
		uint32_t                 : 8;
	} BIT;
} FEATURE_CTRL1_TYPE;

#define CREEP_I_EN_Pos                  0
#define CREEP_I_EN_Mask                 (0x01 << CREEP_I_EN_Pos)
#define CREEP_I_EN_Dis                  0
#define CREEP_I_EN_En                   1

#define CREEP_Q_EN_Pos                  1
#define CREEP_Q_EN_Mask                 (0x01 << CREEP_Q_EN_Pos)
#define CREEP_Q_EN_Dis                  0
#define CREEP_Q_EN_En                   1

#define CREEP_P_EN_Pos                  2
#define CREEP_P_EN_Mask                 (0x01 << CREEP_P_EN_Pos)
#define CREEP_P_EN_Dis                  0
#define CREEP_P_EN_En                   1

#define HARMONIC_m_REQ_Pos              8
#define HARMONIC_m_REQ_Mask             0x3F

#define HARMONIC_EN_Pos                 15
#define HARMONIC_EN_Mask                (0x01 << HARMONIC_m_REQ_Pos)
#define HARMONIC_EN_Dis                 0
#define HARMONIC_EN_En                  1

#define I_MAX_RESET_Pos                 19
#define I_MAX_RESET_Mask                (0x01 << I_MAX_RESET_Pos)
#define I_MAX_RESET_En                  1

#define V_MAX_RESET_Pos                 23
#define V_MAX_RESET_Mask                (0x01 << V_MAX_RESET_Pos)
#define V_MAX_RESET_En                  1

/* ---METER_TYPE 03------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t SENSOR_TYPE_I_A         : 2;
		uint32_t SENSOR_TYPE_V_A         : 2;
		uint32_t SENSOR_TYPE_I_B         : 2;
		uint32_t SENSOR_TYPE_V_B         : 2;
		uint32_t SENSOR_TYPE_I_C         : 2;
		uint32_t SENSOR_TYPE_V_C         : 2;
		uint32_t SENSOR_TYPE_I_N         : 2;
		uint32_t                         : 10;
		uint32_t SERVICE_TYPE            : 4;
		uint32_t MISSING_PHASE           : 2; /* Missing phase voltage select */
		uint32_t                         : 1;
		uint32_t DtoY_TRANSFORM          : 1; /* Transformation to equivalent 4WY */
	} BIT;
} METER_TYPE_TYPE;

#define SENSOR_TYPE_I_A_Pos             0
#define SENSOR_TYPE_I_A_Mask            (0x03 << SENSOR_TYPE_I_A_Pos)
#define SENSOR_TYPE_V_A_Pos             2
#define SENSOR_TYPE_V_A_Mask            (0x03 << SENSOR_TYPE_V_A_Pos)

#define SENSOR_TYPE_I_B_Pos             4
#define SENSOR_TYPE_I_B_Mask            (0x03 << SENSOR_TYPE_I_B_Pos)
#define SENSOR_TYPE_V_B_Pos             6
#define SENSOR_TYPE_V_B_Mask            (0x03 << SENSOR_TYPE_V_B_Pos)

#define SENSOR_TYPE_I_C_Pos             8
#define SENSOR_TYPE_I_C_Mask            (0x03 << SENSOR_TYPE_I_C_Pos)
#define SENSOR_TYPE_V_C_Pos             10
#define SENSOR_TYPE_V_C_Mask            (0x03 << SENSOR_TYPE_V_C_Pos)

#define SENSOR_TYPE_I_N_Pos             12
#define SENSOR_TYPE_I_N_Mask            (0x03 << SENSOR_TYPE_I_N_Pos)

#define SENSOR_TYPE_CT                  0       /* current transformer */
#define SENSOR_TYPE_SHUNT               1       /* resistive shunt */
#define SENSOR_TYPE_Rogowski            2       /* rogowski coil */
#define SENSOR_TYPE_VRD                 3       /* resistive divider */

#define SERVICE_TYPE_Pos                24
#define SERVICE_TYPE_Mask               (0x0F << SERVICE_TYPE_Pos)
#define TYPE_3P4WY_3E_3V3I              0
#define TYPE_3P4WD_3E_3V3I              1
#define TYPE_3P3WY_2P5E_2V3I            2
#define TYPE_3P4WD_2E_3V2I              3
#define TYPE_3P3WD_2E_2V2I              4
#define TYPE_2P3W_2E_2V2I               5
#define TYPE_1P3W_1P5E_1V2I             6
#define TYPE_1P2W_1E_1V1I               7
#define TYPE_1P3W_1E_2V1I               8
#define TYPE_1P2W_1E_1V2I_I0            9
#define TYPE_2P3W_2E_2V2I_I0            10

#define MISSING_PHASE_Pos               28
#define MISSING_PHASE_Mask              (0x03 << MISSING_PHASE_Pos)
#define MISSING_PHASE_A                 0
#define MISSING_PHASE_B                 1
#define MISSING_PHASE_C                 2

#define DtoY_TRANSFORM_Pos              31
#define DtoY_TRANSFORM_Mask             (0x01 << DtoY_TRANSFORM_Pos)
#define DtoY_TRANSFORM_Dis              0
#define DtoY_TRANSFORM_En               1

/* ---M 04---------------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t MV             : 12;
		uint32_t                : 20;
	} BIT;
} M_TYPE_TYPE;

#define MV_Pos                          0
#define MV_Mask                         (0xFFF << MV_Pos)

/* ---N_MAX 05------------------------------------ */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t MAX            : 24;
		uint32_t                : 8;
	} BIT;
} N_MAX_TYPE_TYPE;

#define MAX_Pos                         0
#define MAX_Mask                        (0xFFFFFF << MAX_Pos)

/* ---PULSEx_CTRL 06/07/08------------------------ */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t PCx_WIDTH       : 16; /* pulse width, =[WIDTH (SEC)]*118.784/2.5*10^-6/102.4=[WIDTH]*464000 */
		uint32_t PCx_TYPE        : 8; /* pulse type selection */
		uint32_t PCx_POLARITY    : 1; /* pulse polarity */
		uint32_t PCx_OVERRIDE    : 1; /* pulse override control */
		uint32_t PCx_ACC_HOLD    : 1; /* pulse accumulation hold control */
		uint32_t                 : 1;
		uint32_t PCx_DETENT      : 2; /* total absolute value */
		uint32_t                 : 1;
		uint32_t PCx_ENABLE      : 1; /* pulse output enable */
	} BIT;
} PULSE_CTRL_TYPE;

#define PCx_WIDTH_Pos                   0
#define PCx_WIDTH_Mask                  (0xFFFF << PCx_WIDTH_Pos)
#define PCx_TYPE_Pos                    16
#define PCx_TYPE_Mask                   (0xFF << PCx_TYPE_Pos)
#define PCx_TYPE_P_T                    0x00
#define PCx_TYPE_P_T_F                  0x01
#define PCx_TYPE_Q_T                    0x02
#define PCx_TYPE_Q_T_F                  0x03
#define PCx_TYPE_I_T                    0x04
#define PCx_TYPE_I_T_F                  0x05
#define PCx_POLARITY_Pos                24
#define PCx_POLARITY_Mask               (0x01 << PCx_POLARITY_Pos)
#define PCx_POLARITY_Low                0x00
#define PCx_POLARITY_High               0x01
#define PCx_OVERRIDE_Pos                25
#define PCx_OVERRIDE_Mask               (0x01 << PCx_OVERRIDE_Pos)
#define PCx_OVERRIDE_Normal             0x00
#define PCx_OVERRIDE_Dis                0x01
#define PCx_ACC_HOLD_Pos                26
#define PCx_ACC_HOLD_Mask               (0x01 << PCx_ACC_HOLD_Pos)
#define PCx_ACC_HOLD_En                 0x00
#define PCx_ACC_HOLD_Dis                0x01
#define PCx_DETENT_Pos                  28
#define PCx_DETENT_Mask                 (0x03 << PCx_DETENT_Pos)
#define PCx_DETENT_NET                  0x00
#define PCx_DENTENT_ABSOLUTE            0x01
#define PCx_DENTENT_DELIVERED           0x02
#define PCx_DENTENT_GENERATE            0x03
#define PCx_ENABLE_Pos                  31
#define PCx_ENABLE_Mask                 (0x01 << PCx_ENABLE_Pos)
#define PCx_ENABLE_Dis                  0x00
#define PCx_ENABLE_En                   0x01

/* ---P_K_T 09------------------------------------ */
/* uQ8.24 */
/* ---Q_K_T 10------------------------------------ */
/* uQ8.24 */
/* ---I_K_T 11------------------------------------ */
/* uQ8.24 */
/* ---CREEP_THR_P 12------------------------------ */
/* uQ2.30 */
/* ---CREEP_THR_Q 13------------------------------ */
/* uQ2.30 */
/* ---CREEP_THR_I 14------------------------------ */
/* uQ12.20 */

/* ---POWER_OFFSET_CTRL 15------------------------ */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t                         : 16;
		uint32_t Q_OFFSET_ACC_A          : 1;
		uint32_t Q_OFFSET_ACC_B          : 1;
		uint32_t Q_OFFSET_ACC_C          : 1;
		uint32_t                         : 1;
		uint32_t P_OFFSET_ACC_A          : 1;
		uint32_t P_OFFSET_ACC_B          : 1;
		uint32_t P_OFFSET_ACC_C          : 1;
		uint32_t                         : 4;
		uint32_t Q_OFFSET_PUL            : 1;
		uint32_t                         : 3;
		uint32_t P_OFFSET_PUL            : 1;
	} BIT;
} POWER_OS_CTRL_TYPE;

#define Q_OFFSET_ACC_A_Pos              16
#define Q_OFFSET_ACC_A_Mask             (0x01 << Q_OFFSET_ACC_A_Pos)
#define Q_OFFSET_ACC_A_Dis              0x00
#define Q_OFFSET_ACC_A_En               0x01
#define Q_OFFSET_ACC_B_Pos              17
#define Q_OFFSET_ACC_B_Mask             (0x01 << Q_OFFSET_ACC_B_Pos)
#define Q_OFFSET_ACC_B_Dis              0x00
#define Q_OFFSET_ACC_B_En               0x01
#define Q_OFFSET_ACC_C_Pos              18
#define Q_OFFSET_ACC_C_Mask             (0x01 << Q_OFFSET_ACC_C_Pos)
#define Q_OFFSET_ACC_C_Dis              0x00
#define Q_OFFSET_ACC_C_En               0x01
#define P_OFFSET_ACC_A_Pos              20
#define P_OFFSET_ACC_A_Mask             (0x01 << P_OFFSET_ACC_A_Pos)
#define P_OFFSET_ACC_A_Dis              0x00
#define P_OFFSET_ACC_A_En               0x01
#define P_OFFSET_ACC_B_Pos              21
#define P_OFFSET_ACC_B_Mask             (0x01 << P_OFFSET_ACC_B_Pos)
#define P_OFFSET_ACC_B_Dis              0x00
#define P_OFFSET_ACC_B_En               0x01
#define P_OFFSET_ACC_C_Pos              22
#define P_OFFSET_ACC_C_Mask             (0x01 << P_OFFSET_ACC_C_Pos)
#define P_OFFSET_ACC_C_Dis              0x00
#define P_OFFSET_ACC_C_En               0x01
#define Q_OFFSET_PUL_Pos                27
#define Q_OFFSET_PUL_Mask               (0x01 << Q_OFFSET_PUL_Pos)
#define Q_OFFSET_PUL_Dis                0x00
#define Q_OFFSET_PUL_En                 0x01
#define P_OFFSET_PUL_Pos                31
#define P_OFFSET_PUL_Mask               (0x01 << P_OFFSET_PUL_Pos)
#define P_OFFSET_PUL_Dis                0x00
#define P_OFFSET_PUL_En                 0x01

/* ---POWER_OFFSET_P 16--------------------------- */
/* sQ1.30 */
/* ---POWER_OFFSET_Q 17--------------------------- */
/* sQ1.30 */

/* ---SWELL_THR_VA 18----------------------------- */
/* uQ0.32 */
/* ---SWELL_THR_VB 19----------------------------- */
/* uQ0.32 */
/* ---SWELL_THR_VC 20----------------------------- */
/* uQ0.32 */

/* ---SAG_THR_VA 21------------------------------- */
/* uQ0.32 */
/* ---SAG_THR_VB 22------------------------------- */
/* uQ0.32 */
/* ---SAG_THR_VC 23------------------------------- */
/* uQ0.32 */

/* ---K_IA 24------------------------------------- */
/* uQ22.10 */
/* ---K_VA 25------------------------------------- */
/* uQ22.10 */
/* ---K_IB 26------------------------------------- */
/* uQ22.10 */
/* ---K_VB 27------------------------------------- */
/* uQ22.10 */
/* ---K_IC 28------------------------------------- */
/* uQ22.10 */
/* ---K_VC 29------------------------------------- */
/* uQ22.10 */
/* ---K_IN 30------------------------------------- */
/* uQ22.10 */
/* ---CAL_M_IA 31--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_VA 32--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_IB 33--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_VB 34--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_IC 35--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_VC 36--------------------------------- */
/* sQ2.29 */
/* ---CAL_M_IN 37--------------------------------- */
/* sQ2.29 */

/* ---CAL_PH_IA 38-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_VA 39-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_IB 40-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_VB 41-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_IC 42-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_VC 43-------------------------------- */
/* sQ0.31 */
/* ---CAL_PH_IN 44-------------------------------- */
/* sQ0.31 */

/* ---CAPTURE_CTRL 45----------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t CAPTURE_TYPE            : 2; /* select on-shot or continuous */
		uint32_t                         : 2;
		uint32_t CAPTURE_SOURCE          : 3; /* capture source select */
		uint32_t                         : 1;
		uint32_t CH_SEL_IA               : 1; /* capture channel Ia */
		uint32_t CH_SEL_VA               : 1; /* capture channel Va */
		uint32_t CH_SEL_IB               : 1; /* capture channel Ib */
		uint32_t CH_SEL_VB               : 1; /* capture channel Vb */
		uint32_t CH_SEL_IC               : 1; /* capture channel Ic */
		uint32_t CH_SEL_VC               : 1; /* capture channel Vc */
		uint32_t CH_SEL_IN               : 1; /* capture channel In */
		uint32_t                         : 16;
		uint32_t CAPTURE_EN              : 1; /* enable waveform capture */
	} BIT;
} CAPTURE_CTRL_TYPE;

#define CAPTURE_TYPE_Pos                        0
#define CAPTURE_TYPE_Mask                       (0x03 << CAPTURE_TYPE_Pos)
#define CAPTURE_TYPE_One_shot                   0x00
#define CAPTURE_TYPE_Continuous                 0x01
#define CAPTURE_SOURCE_Pos                      4
#define CAPTURE_SOURCE_Mask                     (0x07 << CAPTURE_SOURCE_Pos)
#define CCAPTURE_SOURCE_16k                     0x00
#define CCAPTURE_SOURCE_4k                      0x01
#define CCAPTURE_SOURCE_4k_Fund                 0x02
#define CH_SEL_IA_Pos                           8
#define CH_SEL_IA_Mask                          (0x01 << CH_SEL_IA_Pos)
#define CH_SEL_IA_Dis                           0x00
#define CH_SEL_IA_En                            0x01
#define CH_SEL_VA_Pos                           9
#define CH_SEL_VA_Mask                          (0x01 << CH_SEL_VA_Pos)
#define CH_SEL_VA_Dis                           0x00
#define CH_SEL_VA_En                            0x01
#define CH_SEL_IB_Pos                           10
#define CH_SEL_IB_Mask                          (0x01 << CH_SEL_IB_Pos)
#define CH_SEL_IB_Dis                           0x00
#define CH_SEL_IB_En                            0x01
#define CH_SEL_VB_Pos                           11
#define CH_SEL_VB_Mask                          (0x01 << CH_SEL_VB_Pos)
#define CH_SEL_VB_Dis                           0x00
#define CH_SEL_VB_En                            0x01
#define CH_SEL_IC_Pos                           12
#define CH_SEL_IC_Mask                          (0x01 << CH_SEL_IC_Pos)
#define CH_SEL_IC_Dis                           0x00
#define CH_SEL_IC_En                            0x01
#define CH_SEL_VC_Pos                           13
#define CH_SEL_VC_Mask                          (0x01 << CH_SEL_VC_Pos)
#define CH_SEL_VC_Dis                           0x00
#define CH_SEL_VC_En                            0x01
#define CH_SEL_IN_Pos                           14
#define CH_SEL_IN_Mask                          (0x01 << CH_SEL_IN_Pos)
#define CH_SEL_IN_Dis                           0x00
#define CH_SEL_IN_En                            0x01
#define CAPTURE_EN_Pos                          31
#define CAPTURE_EN_Mask                         (0x01 << CAPTURE_EN_Pos)
#define CAPTURE_EN_Dis                          0x00
#define CCAPTURE_EN_En                          0x01

/* ---CAPTURE_BUFF_SIZE 46------------------------ */
/* uQ24.0 */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t CP_BUF_SIZE             : 24;
		uint32_t                         : 8;
	} BIT;
} CAPTURE_BUF_SIZE_TYPE;

/* ---CAPTURE_ADDR 47----------------------------- */
/* uQ32.0 */
/* ---RESERVED_C48--------------------------------- */
/* ---RESERVED_C49--------------------------------- */
/* ---RESERVED_C50--------------------------------- */

/* ---ATSENSE_CTRL_20_23 51----------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t I0_ON           : 1;
		uint32_t TEMPMEAS        : 1;
		uint32_t                 : 2;
		uint32_t I0_GAIN         : 2;
		uint32_t                 : 2;
		uint32_t I1_ON           : 1;
		uint32_t                 : 3;
		uint32_t I1_GAIN         : 2;
		uint32_t                 : 2;
		uint32_t V1_ON           : 1;
		uint32_t                 : 7;
		uint32_t I2_ON           : 1;
		uint32_t                 : 3;
		uint32_t I2_GAIN         : 2;
		uint32_t                 : 2;
	} BIT;
} ATSENSE_CTRL1_TYPE;

#define I0_ON_Pos                       0
#define I0_ON_Mask                      (0x01 << I0_ON_Pos)
#define I0_ON_Dis                       0
#define I0_ON_En                        1
#define TEMPMEAS_Pos                    1
#define TEMPMEAS_Mask                   (0x01 << TEMPMEAS_Pos)
#define TEMPMEAS_I0                     0
#define TEMPMEAS_TEMP                   1
#define I0_GAIN_Pos                     4
#define I0_GAIN_Mask                    (0x03 << I0_GAIN_Pos)
#define I0_GAIN_1                       0
#define I0_GAIN_2                       1
#define I0_GAIN_4                       2
#define I0_GAIN_8                       3
#define I1_ON_Pos                       8
#define I1_ON_Mask                      (0x01 << I1_ON_Pos)
#define I1_ON_Dis                       0
#define I1_ON_En                        1
#define I1_GAIN_Pos                     12
#define I1_GAIN_Mask                    (0x03 << I1_GAIN_Pos)
#define I1_GAIN_1                       0
#define I1_GAIN_2                       1
#define I1_GAIN_4                       2
#define I1_GAIN_8                       3
#define V1_ON_Pos                       16
#define V1_ON_Mask                      (0x01 << V1_ON_Pos)
#define V1_ON_Dis                       0
#define V1_ON_En                        1
#define I2_ON_Pos                       24
#define I2_ON_Mask                      (0x01 << I2_ON_Pos)
#define I2_ON_Dis                       0
#define I2_ON_En                        1
#define I2_GAIN_Pos                     28
#define I2_GAIN_Mask                    (0x03 << I2_GAIN_Pos)
#define I2_GAIN_1                       0
#define I2_GAIN_2                       1
#define I2_GAIN_4                       2
#define I2_GAIN_8                       3

/* ---ATSENSE_CTRL_24_27 52----------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t V2_ON           : 1;
		uint32_t                 : 7;
		uint32_t I3_ON           : 1;
		uint32_t                 : 3;
		uint32_t I3_GAIN         : 2;
		uint32_t                 : 2;
		uint32_t V3_ON           : 1;
		uint32_t                 : 7;
		uint32_t ONBIAS          : 1;
		uint32_t ONREF           : 1;
		uint32_t ONLDO           : 1;
		uint32_t                 : 5;
	} BIT;
} ATSENSE_CTRL2_TYPE;

#define V2_ON_Pos                       0
#define V2_ON_Mask                      (0x01 << V2_ON_Pos)
#define V2_ON_Dis                       0
#define V2_ON_En                        1
#define I3_ON_Pos                       8
#define I3_ON_Mask                      (0x01 << I3_ON_Pos)
#define I3_ON_Dis                       0
#define I3_ON_En                        1
#define I3_GAIN_Pos                     12
#define I3_GAIN_Mask                    (0x03 << I3_GAIN_Pos)
#define I3_GAIN_1                       0
#define I3_GAIN_2                       1
#define I3_GAIN_4                       2
#define I3_GAIN_8                       3
#define V3_ON_Pos                       16
#define V3_ON_Mask                      (0x01 << V3_ON_Pos)
#define V3_ON_Dis                       0
#define V3_ON_En                        1
#define ONBIAS_Pos                      24
#define ONBIAS_Mask                     (0x01 << ONBIAS_Pos)
#define ONBIAS_Dis                      0
#define ONBIAS_En                       1
#define ONREF_Pos                       25
#define ONREF_Mask                      (0x01 << ONREF_Pos)
#define ONREF_Dis                       0
#define ONREF_En                        1
#define ONLDO_Pos                       26
#define ONLDO_Mask                      (0x01 << ONLDO_Pos)
#define ONLDO_Dis                       0
#define ONLDO_En                        1

/* ---ATSENSE_CTRL_28_2B 53----------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t OSR             : 2;
		uint32_t                 : 2;
		uint32_t MSB_MODE        : 1;
		uint32_t                 : 27;
	} BIT;
} ATSENSE_CTRL3_TYPE;

#define OSR_Pos                         0
#define OSR_Mask                        (0x03 << OSR_Pos)
#define OSR_8                           0
#define OSR_16                          1
#define OSR_32                          2
#define OSR_64                          3
#define MSB_MODE_Pos                    4
#define MSB_MODE_Mask                   (0x01 << MSB_MODE_Pos)
#define MSB_MODE_32                     0
#define MSB_MODE_16                     1

/* ---RESERVED_C54--------------------------------- */

/* ---POWER_OFFSET_P_A 55------------------------- */
/* sQ23.40 */
/* ---POWER_OFFSET_P_B 56------------------------- */
/* sQ23.40 */
/* ---POWER_OFFSET_P_C 57------------------------- */
/* sQ23.40 */
/* ---POWER_OFFSET_Q_A 58------------------------- */
/* sQ23.40 */
/* ---POWER_OFFSET_Q_B 59------------------------- */
/* sQ23.40 */
/* ---POWER_OFFSET_Q_C 60------------------------- */
/* sQ23.40 */

typedef struct {
	STATE_CTRL_TYPE STATE_CTRL;            /* uQ32.0 */
	FEATURE_CTRL0_TYPE FEATURE_CTRL0;
	FEATURE_CTRL1_TYPE FEATURE_CTRL1;
	METER_TYPE_TYPE METER_TYPE;

	M_TYPE_TYPE M;
	N_MAX_TYPE_TYPE N_MAX;
	PULSE_CTRL_TYPE PULSE0_CTRL;
	PULSE_CTRL_TYPE PULSE1_CTRL;

	PULSE_CTRL_TYPE PULSE2_CTRL;
	uint32_t P_K_T;            /* uQ8.24  Wh/pulse */
	uint32_t Q_K_T;            /* uQ8.24  Wh/pulse */
	uint32_t I_K_T;            /* uQ8.24  Wh/pulse */

	uint32_t CREEP_THR_P;
	uint32_t CREEP_THR_Q;
	uint32_t CREEP_THR_I;
	POWER_OS_CTRL_TYPE POWER_OFFSET_CTRL;

	int32_t POWER_OFFSET_P;    /* sQ1.30 */
	int32_t POWER_OFFSET_Q;    /* sQ1.30 */
	uint32_t SWELL_THR_VA;
	uint32_t SWELL_THR_VB;

	uint32_t SWELL_THR_VC;
	uint32_t SAG_THR_VA;
	uint32_t SAG_THR_VB;
	uint32_t SAG_THR_VC;

	uint32_t K_IA;             /* uQ22.10 */
	uint32_t K_VA;             /* uQ22.10 */
	uint32_t K_IB;
	uint32_t K_VB;

	uint32_t K_IC;
	uint32_t K_VC;
	uint32_t K_IN;
	int32_t CAL_M_IA;          /* sQ2.29 */

	int32_t CAL_M_VA;
	int32_t CAL_M_IB;
	int32_t CAL_M_VB;
	int32_t CAL_M_IC;

	int32_t CAL_M_VC;
	int32_t CAL_M_IN;
	int32_t CAL_PH_IA;         /* sQ0.31 */
	int32_t CAL_PH_VA;

	int32_t CAL_PH_IB;
	int32_t CAL_PH_VB;
	int32_t CAL_PH_IC;
	int32_t CAL_PH_VC;

	int32_t CAL_PH_IN;
	CAPTURE_CTRL_TYPE CAPTURE_CTRL;                 /* CAPTURE_CTRL */
	CAPTURE_BUF_SIZE_TYPE CAPTURE_BUFF_SIZE;        /* CAPTURE_BUFF_SIZE */
	uint32_t CAPTURE_ADDR;                          /* CAPTURE_ADDR */

	uint32_t RESERVED_C48;
	uint32_t RESERVED_C49;
	uint32_t RESERVED_C50;
	ATSENSE_CTRL1_TYPE ATSENSE_CTRL_20_23;

	ATSENSE_CTRL2_TYPE ATSENSE_CTRL_24_27;
	ATSENSE_CTRL3_TYPE ATSENSE_CTRL_28_2B;
	uint32_t RESERVED_C54;
	int32_t POWER_OFFSET_P_A;

	int32_t POWER_OFFSET_P_B;
	int32_t POWER_OFFSET_P_C;
	int32_t POWER_OFFSET_Q_A;
	int32_t POWER_OFFSET_Q_B;

	int32_t POWER_OFFSET_Q_C;
} DSP_CTRL_TYPE, *p_DSP_CTRL;

#define DSP_CONTROL_SIZE (sizeof(DSP_CTRL_TYPE) / sizeof(uint32_t))

/* ===Metrology Status Registers================== */
/* ---VERSION 00---------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t FIRMWARE_REV            : 8;
		uint32_t FIRMWARE_MINOR_REV      : 8;
		uint32_t FIRMWARE_MAJOR_REV      : 8;
		uint32_t                         : 8;
	} BIT;
} VERSION_TYPE;

/* ---STATUS 01----------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t ST              : 4;
		uint32_t                 : 28;
	} BIT;
} STATUS_TYPE;

#define ST_Pos                          0
#define ST_Mask                         (0x0F << ST_Pos)
#define STATUS_HALT                     0
#define STATUS_RESET                    1
#define STATUS_INIT_DSP                 2
#define STATUS_DSP_READY                3
#define STATUS_INIT_ATSENSE             4
#define STATUS_ATSENSE_READY            5
#define STATUS_READY                    6
#define STATUS_DSP_SETTLING             7
#define STATUS_DSP_RUNNING              8

/* ---STATE_FLAG 02------------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t PH_A_ACTIVE     : 1;
		uint32_t PH_B_ACTIVE     : 1;
		uint32_t PH_C_ACTIVE     : 1;
		uint32_t TIMING_Vx       : 2;
		uint32_t FREQ_LOCKED     : 1;
		uint32_t                 : 1;
		uint32_t ATSENSE_FAIL    : 1;
		uint32_t HARMONIC_m_CONF : 8;
		uint32_t CREEP_DET_IA    : 1;
		uint32_t CREEP_DET_IB    : 1;
		uint32_t CREEP_DET_IC    : 1;
		uint32_t CREEP_DET_Q     : 1;
		uint32_t CREEP_DET_P     : 1;
		uint32_t                 : 3;
		uint32_t SAG_DET_VA      : 1;
		uint32_t SAG_DET_VB      : 1;
		uint32_t SAG_DET_VC      : 1;
		uint32_t                 : 1;
		uint32_t SWELL_DET_VA    : 1;
		uint32_t SWELL_DET_VB    : 1;
		uint32_t SWELL_DET_VC    : 1;
		uint32_t                 : 1;
	} BIT;
} STATE_FLAG_TYPE;

/* ---CAPTURE_STATUS 03--------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t CAPTURE_SIZE    : 24;        //CAPTURE_OFFSET
		uint32_t CAPTURE_STATE   : 4;
		uint32_t                 : 3;
		uint32_t CAPTURE_WRAP    : 1;
	} BIT;
} CAPTURE_STATUS_TYPE;

/* ---INTERVAL_NUM 04----------------------------- */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t HALFCYCLE_COUNT         : 16;
		uint32_t INTERVAL_NUM            : 16;
	} BIT;
} INTERVAL_NUM_TYPE;

/* ---N 05---------------------------------------- */
/* uQ16.0 */

/* ---PH_OFFSET 06-------------------------------- */
/* sQ0.31 */

/* ---FREQ 07------------------------------------- */
/* uQ20.12 */

/* ---FREQ_VA 08---------------------------------- */
/* uQ20.12 */
/* ---FREQ_VB 09---------------------------------- */
/* uQ20.12 */
/* ---FREQ_VC 10---------------------------------- */
/* uQ20.12 */

/* ---RESERVED_S11--------------------------------- */

/* ---TEMPERATURE 12------------------------------ */
/* sQ23.8 */

/* ---I_A_MAX 13---------------------------------- */
/* sQ2.29 */
/* ---I_B_MAX 14---------------------------------- */
/* sQ2.29 */
/* ---I_C_MAX 15---------------------------------- */
/* sQ2.29 */
/* ---I_Nm_MAX 16--------------------------------- */
/* sQ2.29 */
/* ---I_Ni_MAX 17--------------------------------- */
/* sQ2.29 */
/* ---V_A_MAX 18---------------------------------- */
/* sQ2.29 */
/* ---V_B_MAX 19---------------------------------- */
/* sQ2.29 */
/* ---V_C_MAX 20---------------------------------- */
/* sQ2.29 */

/* ---FEATURES 21--------------------------------- */
/* uQ32.0 */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t DEBUG_MODES             : 1;
		uint32_t ATSENSE_HALF_CLK        : 1;
		uint32_t SYNTHESIZE              : 1;
		uint32_t DUPLICATE_V1            : 1;
		uint32_t                         : 4;
		uint32_t HALF_COPROC_CLK         : 1;
		uint32_t I_N_MUXING              : 1;
		uint32_t PQ_OFFSET_X             : 1;
		uint32_t RZC_DETECT              : 1;
		uint32_t CORE_CLK_SPEED          : 4;
		uint32_t DFT_ENABLED             : 1;
		uint32_t CREEP                   : 1;
		uint32_t CAPTURE                 : 1;
		uint32_t ROGOWSKI_DC_REMOVE      : 1;
		uint32_t                         : 8;
		uint32_t METER_TYPE_10           : 1;
		uint32_t METER_TYPE_09           : 1;
		uint32_t SINGLEPHASE             : 1;
		uint32_t POLYPHASE               : 1;
	} BIT;
} FEATURES_TYPE;

/* ---RESERVED_S22--------------------------------- */
/* ---RESERVED_S23--------------------------------- */
/* ---RESERVED_S24--------------------------------- */
/* ---RESERVED_S25--------------------------------- */
/* ---RESERVED_S26--------------------------------- */
/* ---RESERVED_S27--------------------------------- */
/* ---RESERVED_S28--------------------------------- */
/* ---RESERVED_S29--------------------------------- */

/* ---ZC_N_VA 30---------------------------------- */
/* uQ20.12 */

/* ---ZC_N_VB 31---------------------------------- */
/* uQ20.12 */
/* ---ZC_N_VC 32---------------------------------- */
/* uQ20.12 */

/* ---ATsensee_CAL_41_44 33----------------------- */
/* uQ32.0 */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t REF_TEL_11_8    : 4;
		uint32_t                 : 4;
		uint32_t REF_TEL_7_0     : 8;
		uint32_t TEMP_TL_11_8    : 4;
		uint32_t                 : 4;
		uint32_t TEMP_TL_7_0     : 8;
	} BIT;
} ATSENSE_CAL_41_44_TYPE;

/* ---ATsensee_CAL_45_48 34----------------------- */
/* uQ32.0 */
typedef union {
	uint32_t WORD;
	struct {
		uint32_t REF_TEH_11_8    : 4;
		uint32_t                 : 4;
		uint32_t REF_TEH_7_0     : 8;
		uint32_t TEMP_TH_11_8    : 4;
		uint32_t                 : 4;
		uint32_t TEMP_TH_7_0     : 8;
	} BIT;
} ATSENSE_CAL_45_48_TYPE;

typedef struct {
	VERSION_TYPE VERSION;                   /* uQ32.0 */
	STATUS_TYPE STATUS;
	STATE_FLAG_TYPE STATE_FLAG;
	CAPTURE_STATUS_TYPE CAPTURE_STATUS;

	INTERVAL_NUM_TYPE INTERVAL_NUM;
	uint32_t N;
	int32_t PH_OFFSET;                      /* sQ31.0 */
	uint32_t FREQ;                          /* uQ20.12 */

	uint32_t FREQ_VA;                       /* uQ8.24 */
	uint32_t FREQ_VB;
	uint32_t FREQ_VC;
	uint32_t RESERVED_S11;

	int32_t TEMPERATURE;                    /* sQ23.8 */
	int32_t I_A_MAX;                        /* sQ1.30 */
	int32_t I_B_MAX;
	int32_t I_C_MAX;

	int32_t I_Nm_MAX;
	int32_t I_Ni_MAX;
	int32_t V_A_MAX;
	int32_t V_B_MAX;

	int32_t V_C_MAX;
	FEATURES_TYPE FEATURES;
	uint32_t RESERVED_S22;
	uint32_t RESERVED_S23;

	uint32_t RESERVED_S24;
	uint32_t RESERVED_S25;
	uint32_t RESERVED_S26;
	uint32_t RESERVED_S27;

	uint32_t RESERVED_S28;
	uint32_t RESERVED_S29;
	uint32_t ZC_N_VA;
	uint32_t ZC_N_VB;

	uint32_t ZC_N_VC;
	ATSENSE_CAL_41_44_TYPE ATsense_CAL_41_44;
	ATSENSE_CAL_45_48_TYPE ATsense_CAL_45_48;
} DSP_ST_TYPE, *p_DSP_ST;

#define DSP_ST_SIZE (sizeof(DSP_ST_TYPE) / sizeof(uint32_t))

/* ===Accumulated Out Registers=================== */
/* ---I_A 00-------------------------------------- */
/* uQ24.40 */
/* ---I_B 01-------------------------------------- */
/* uQ24.40 */
/* ---I_C 02-------------------------------------- */
/* uQ24.40 */
/* ---I_Ni 03------------------------------------- */
/* uQ44.20 */
/* ---I_Nm 04------------------------------------- */
/* uQ24.40 */
/* ---I_A_F 05------------------------------------ */
/* uQ24.40 */
/* ---I_B_F 06------------------------------------ */
/* uQ24.40 */
/* ---I_C_F 07------------------------------------ */
/* uQ24.40 */
/* ---I_Nmi 08------------------------------------ */
/* uQ44.20 */
/* ---I_Nm_F      --------------------------------- */
/* uQ24.40 */

/* ---RESERVED_A10--------------------------------- */
/* ---RESERVED_A11--------------------------------- */
/* ---RESERVED_A12--------------------------------- */
/* ---RESERVED_A13--------------------------------- */
/* ---RESERVED_A14--------------------------------- */

/* ---P_A 15-------------------------------------- */
/* sQ23.40 */
/* ---P_B 16-------------------------------------- */
/* sQ23.40 */
/* ---P_C 17-------------------------------------- */
/* sQ23.40 */
/* ---P_A_F 18------------------------------------ */
/* sQ23.40 */
/* ---P_B_F 19------------------------------------ */
/* sQ23.40 */
/* ---P_C_F 20------------------------------------ */
/* sQ23.40 */
/* ---P_N 21-------------------------------------- */
/* sQ23.40 */
/* ---P_N_F 22------------------------------------ */
/* sQ23.40 */

/* ---RESERVED_A23--------------------------------- */

/* ---Q_A 24-------------------------------------- */
/* sQ23.40 */
/* ---Q_B 25-------------------------------------- */
/* sQ23.40 */
/* ---Q_C 26-------------------------------------- */
/* sQ23.40 */
/* ---Q_A_F 27------------------------------------ */
/* sQ23.40 */
/* ---Q_B_F 28------------------------------------ */
/* sQ23.40 */
/* ---Q_C_F 29------------------------------------ */
/* sQ23.40 */
/* ---Q_N 30 ------------------------------------- */
/* sQ23.40 */
/* ---Q_N_F 31------------------------------------ */
/* sQ23.40 */

/* ---RESERVED_A32--------------------------------- */

/* ---V_A 33-------------------------------------- */
/* uQ24.40 */
/* ---V_B 34-------------------------------------- */
/* uQ24.40 */
/* ---V_C 35-------------------------------------- */
/* uQ24.40 */
/* ---RESERVED_A36--------------------------------- */

/* ---V_A_F 37------------------------------------ */
/* uQ24.40 */
/* ---V_B_F 38------------------------------------ */
/* uQ24.40 */
/* ---V_C_F 39------------------------------------ */
/* uQ24.40 */
/* ---RESERVED_A40--------------------------------- */

/* ---V_AB 41-------------------------------------- */
/* uQ24.40 */
/* ---V_BC 42-------------------------------------- */
/* uQ24.40 */
/* ---V_CA 43-------------------------------------- */
/* uQ24.40 */
/* ---V_AB_F 44----------------------------------- */
/* uQ24.40 */
/* ---V_BC_F 45----------------------------------- */
/* uQ24.40 */
/* ---V_CA_F 46----------------------------------- */
/* uQ24.40 */

/* ---RESERVED_A47--------------------------------- */
/* ---RESERVED_A48--------------------------------- */
/* ---RESERVED_A49--------------------------------- */

/* ---ACC_T0 50----------------------------------- */
/* sQ33.30 */
/* ---ACC_T1 51----------------------------------- */
/* sQ33.30 */
/* ---ACC_T2 52----------------------------------- */
/* sQ33.30 */

/* ---RESERVED_A53--------------------------------- */
/* ---RESERVED_A54--------------------------------- */

typedef struct {
	uint64_t I_A;                   /* uQ24.40 */
	uint64_t I_B;
	uint64_t I_C;
	uint64_t I_Ni;                  /* uQ44.20 */

	uint64_t I_Nm;                  /* uQ24.40 */
	uint64_t I_A_F;                 /* uQ24.40 */
	uint64_t I_B_F;
	uint64_t I_C_F;

	uint64_t I_Nmi;                 /* uQ44.20 */
	uint64_t I_Nm_F;
	uint64_t RESERVED_A10;
	uint64_t RESERVED_A11;

	uint64_t RESERVED_A12;
	uint64_t RESERVED_A13;
	uint64_t RESERVED_A14;
	int64_t P_A;                    /* sQ23.40 */

	int64_t P_B;
	int64_t P_C;
	int64_t P_A_F;
	int64_t P_B_F;

	int64_t P_C_F;
	uint64_t P_N;
	uint64_t P_N_F;
	uint64_t RESERVED_A23;

	int64_t Q_A;
	int64_t Q_B;
	int64_t Q_C;
	int64_t Q_A_F;

	int64_t Q_B_F;
	int64_t Q_C_F;
	uint64_t Q_N;
	uint64_t Q_N_F;

	uint64_t RESERVED_A32;
	uint64_t V_A;                   /* uQ24.40 */
	uint64_t V_B;
	uint64_t V_C;

	uint64_t RESERVED_A36;
	uint64_t V_A_F;
	uint64_t V_B_F;
	uint64_t V_C_F;

	uint64_t RESERVED_A40;
	uint64_t V_AB;
	uint64_t V_BC;
	uint64_t V_CA;

	uint64_t V_AB_F;
	uint64_t V_BC_F;
	uint64_t V_CA_F;
	uint64_t RESERVED_A47;

	uint64_t RESERVED_A48;
	uint64_t RESERVED_A49;
	int64_t ACC_T0;                 /* sQ33.30 */
	int64_t ACC_T1;

	int64_t ACC_T2;
	uint64_t RESERVED_A53;
	uint64_t RESERVED_A54;
} DSP_ACC_TYPE, *p_DSP_ACC;

#define DSP_ACC_SIZE (sizeof(DSP_ACC_TYPE) / sizeof(uint64_t))

/* ===Metrology Har Out Registers================= */
/* ---I_A_m_R 00---------------------------------- */
/* sQ23.8 */
/* ---V_A_m_R 01---------------------------------- */
/* sQ23.8 */
/* ---I_B_m_R 02---------------------------------- */
/* sQ23.8 */
/* ---V_B_m_R 03---------------------------------- */
/* sQ23.8 */
/* ---I_C_m_R 04---------------------------------- */
/* sQ23.8 */
/* ---V_C_m_R 05---------------------------------- */
/* sQ23.8 */
/* ---I_A_m_I 06---------------------------------- */
/* sQ23.8 */
/* ---V_A_m_I 07---------------------------------- */
/* sQ23.8 */
/* ---I_B_m_I 08---------------------------------- */
/* sQ23.8 */
/* ---V_B_m_I 09---------------------------------- */
/* sQ23.8 */
/* ---I_C_m_I 10---------------------------------- */
/* sQ23.8 */
/* ---V_C_m_I 11---------------------------------- */
/* sQ23.8 */

/* ---RESERVED_H12--------------------------------- */
/* ---RESERVED_H13--------------------------------- */
/* ---RESERVED_H14--------------------------------- */

typedef struct {
	int32_t I_A_m_R;        /* sQ25.6 */
	int32_t V_A_m_R;
	int32_t I_B_m_R;
	int32_t V_B_m_R;

	int32_t I_C_m_R;
	int32_t V_C_m_R;
        int32_t I_N_m_R;
	int32_t I_A_m_I;
        
	int32_t V_A_m_I;
	int32_t I_B_m_I;
	int32_t V_B_m_I;
	int32_t I_C_m_I;
        
	int32_t V_C_m_I;
        int32_t I_N_m_I;
	uint32_t RESERVED_H14;
} DSP_HAR_TYPE, *p_DSP_HAR;

typedef struct {
	double Irms_A_m;
	double Irms_B_m;
	double Irms_C_m;
        double Irms_N_m;
	double Vrms_A_m;
	double Vrms_B_m;
	double Vrms_C_m;
} metrology_har_t;

#define DSP_HAR_SIZE (sizeof(DSP_HAR_TYPE) / sizeof(uint32_t))

/* --------------------------------------------------------- */
#define DSP_CTRL_ST_CTRL_Reset  0
#define DSP_CTRL_ST_CTRL_Init   1
#define DSP_CTRL_ST_CTRL_Run    2
#define DSP_CTRL_ST_CTRL_Hold   3

typedef enum {
	iSReset     = 0,
	iSInit      = 1,
	iSRun       = 2,
	iSHold      = 3,
} DSP_CTRL_ST_CTRL_TYPE;
/* --------------------------------------------------------- */

/* --------------------------------------------------------- */
#define  FREQ_Q         12
#define  GAIN_P_K_T_Q   24
#define  GAIN_VI_Q      10
#define  RMS_DIV_G      1024    /* (1<<GAIN_VI_Q) */
#define  CAL_VI_Q       29
#define  CAL_PH_Q       31
#define  RMS_Q          40
#define  RMS_DIV_Q      0x10000000000  /* (1<<RMS_Q) */

#define  RMS_Inx_Q      (20)
#define  RMS_DIV_Inx_Q  0x100000 /* (1<< RMS_Inx_Q ) */

#define  RMS_PQ_SYMB    0x8000000000000000       /* p/q symbol bit */
#define  RMS_HARMONIC   0x80000000

typedef struct {
	//volatile uint32_t DSP_INIT_FLAG;
	DSP_CTRL_TYPE DSP_CTRL;
	DSP_ST_TYPE DSP_STATUS;
	DSP_ACC_TYPE DSP_ACC;
	DSP_HAR_TYPE DSP_HAR;
	volatile uint32_t DSP_INIT_FLAG;
} metrology_t;

typedef enum {
	CAL_FREE_ST   = 0,
	CAL_AMP_ST    = 1,
	CAL_PHASE_ST  = 2,
	CAL_AMP_PH_ST = 3,
} CAL_METER_ST_TYPE;

typedef union {
	uint32_t aim_p;
	uint32_t aim_an;
} CAL_P_ANGLE_UNION;

typedef struct {
	uint8_t state;
	uint32_t line_id;
	uint32_t mc;            /* meter const =3200 imp/kwh */
	uint32_t sensortype;    /* 0=CT,1=shunt,2=rogowski */
	uint32_t freq;          /* frequency  *100 */
	uint32_t gain_i;        /* current pga */
	uint32_t k_i;           /* CT current transform ratio *1000 */
	uint32_t rl;            /* CT resistor load *100 */
	uint32_t k_u;           /* voltage divider ratio *1000 */
	uint32_t aim_va;        /* aim voltage  *1000 */
	uint32_t aim_ia;        /* aim current  *10000 */
	uint32_t angle_a;       /* phase a angle *1000 */
	uint32_t aim_vb;        /* aim voltage  *1000 */
	uint32_t aim_ib;        /* aim current  *10000 */
	uint32_t angle_b;       /* phase c angle *1000 */
	uint32_t aim_vc;        /* aim voltage  *1000 */
	uint32_t aim_ic;        /* aim current  *10000 */
	uint32_t angle_c;       /* phase c angle *1000 */
	uint32_t dsp_update_num;
	uint64_t dsp_acc_ia;
	uint64_t dsp_acc_ib;
	uint64_t dsp_acc_ic;
	uint64_t dsp_acc_in;
	uint64_t dsp_acc_ua;
	uint64_t dsp_acc_ub;
	uint64_t dsp_acc_uc;
	uint64_t dsp_acc_un;
	int64_t dsp_acc_pa;
	int64_t dsp_acc_pb;
	int64_t dsp_acc_pc;
	int64_t dsp_acc_qa;
	int64_t dsp_acc_qb;
	int64_t dsp_acc_qc;
	uint8_t har_order;
	uint32_t feature_ctrl0_copy;	/* To save the FEATURE_CTRL0 value (modified during calibration)*/
} metrology_cal_t;

typedef enum {
	Ph_A        = 1,
	Ph_B        = 2,
	Ph_C        = 3,
	Ph_N        = 4,
	Ph_T        = 5,

	V_A         = 1,
	V_B         = 2,
	V_C         = 3,
	V_N         = 4,

	I_A         = 1,
	I_B         = 2,
	I_C         = 3,
	I_N         = 4,

	V_ID        = 1,
	I_ID        = 2,
	Ph_ID       = 3
} Ph_V_I_ID_TYPE;

typedef enum {
	CT        = 0,
	SHUNT     = 1,
	ROGOWSKI  = 2,
	SENSOR_NUM_TYPE
} SENSOR_TYPE;

typedef union {
	uint32_t WORD;
	struct {
		uint32_t pa_dir          : 1;
		uint32_t pb_dir          : 1;
		uint32_t pc_dir          : 1;
		uint32_t pt_dir          : 1;
		uint32_t qa_dir          : 1;
		uint32_t qb_dir          : 1;
		uint32_t qc_dir          : 1;
		uint32_t qt_dir          : 1;

		uint32_t sag_a           : 1;
		uint32_t sag_b           : 1;
		uint32_t sag_c           : 1;
		uint32_t                 : 1;
		uint32_t swell_a         : 1;
		uint32_t swell_b         : 1;
		uint32_t swell_c         : 1;

		uint32_t pa_rev          : 1;
		uint32_t pb_rev          : 1;
		uint32_t pc_rev          : 1;
		uint32_t pt_rev          : 1;
		uint32_t             : 13;
	} BIT;
} AFE_ST_TYPE;

typedef enum {
	PEnergy = 0,
	QEnergy = 1,
	AbsAdd = 0,
	AlgAdd = 1
} M_ENERGY_TYPE;

typedef enum {
	Ua = 0,
	Ub,
	Uc,
	Ia = 3,
	Ib,
	Ic,
	Ini = 6,
	Inm,
	Inmi,
	Pt = 9,
	Pa = 10,
	Pb,
	Pc,
	Qt = 13,
	Qa,
	Qb,
	Qc = 16,
	St = 17,
	Sa,
	Sb = 19,
	Sc,
	Freq = 21,
	AngleA = 22,
	AngleB,
	AngleC,
	AngleN,
	RMS_NUM_MAX
} RMS_TYPE;

typedef struct {
	uint32_t updataflag;
	uint32_t energy;
	AFE_ST_TYPE ST;
	uint32_t RMS[RMS_NUM_MAX];
} metrology_afe_t;

/* ------------------------------------------------------------------- */
#define DSP_CTRL_FRAME_NUM_MAX  16
#define DSP_ST_FRAME_NUM_MAX    9
#define DSP_ACC_FRAME_NUM_MAX   14
#define DSP_HAR_FRAME_NUM_MAX   3

#define DSP_CTRL_REG_NUM           61
extern const uint8_t *dsp_ctrl_header[];
extern const uint32_t *dsp_ctrl_str[];

#define DSP_ST_REG_NUM             35
extern const uint8_t *dsp_st_header[];
extern const uint32_t *dsp_st_str[];

#define DSP_ACC_REG_NUM            55
extern const uint8_t *dsp_acc_header[];
extern const uint64_t *dsp_acc_str[];

#define DSP_HAR_REG_NUM            14
#define DSP_HAR_MAX_ORDER          21
extern const uint8_t *dsp_har_header[];
extern const uint32_t *dsp_har_str[];

#define DSP_HRR_REG_NUM            7
extern const uint8_t *dsp_hrr_header[];
extern const uint32_t *dsp_hrr_str[];

extern metrology_t VMetrology;
extern metrology_cal_t VCalibration;
extern metrology_afe_t VAFE;

typedef void (*pf_har_callback)(metrology_har_t *p_har_calc);
typedef void (*pf_cal_callback)(uint8_t error);

uint32_t MetrologyInit(void);
void MetrologyLoadDefault(void);
void MetrologyRefreshCrtl(void);
uint16_t MetrologyUpdateExtMem(void);
void MetrologyCalibMeterInit(void);
void MetrologyCalibMeter(void);
void MetrologyProcess(void);
void MetrologySetHarmonicOrder(uint8_t order);
uint8_t MetrologyGetHarmonicOrder(void);
uint8_t MetrologyHarmonicIsReady(void);
void MetrologyHarmonicsProcess(void);
void MetrologySetHarmonicsCallback(pf_har_callback har_cb);
void MetrologySetCalibrationCallback(pf_cal_callback cal_cb);

uint32_t MetrologyGetCaptureData(int32_t **data);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* METROLOGY_H_INCLUDED */
