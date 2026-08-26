/**
 * \file
 *
 * \brief PLCOUP007 G3 CENELEC-A Coupling and TX configuration.
 *
 * Copyright (C) 2020 Atmel Corporation. All rights reserved.
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

#ifndef PLCOUP007_CEN_A_H_INCLUDED
#define PLCOUP007_CEN_A_H_INCLUDED

/********************************************************************************************************/
/* Coupling and TX parameters for G3 CENELEC-A. *********************************************************/
/* Recommended to use PLCOUP007_v2/PLCOUP008_v2/PLCOUP011_v1 coupling board (CENELEC-A Single Branch). **/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. **************************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. ********/
/********************************************************************************************************/

/* ATPL360_REG_MAX_RMS_TABLE_HI , ATPL360_REG_MAX_RMS_TABLE_VLO. */
/* Target RMS_CALC in HI / VLO mode for dynamic gain (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1 or 2). */
/* 8 values, corresponding to first 8 TX attenuation levels (3 dB step). */
#define MAX_RMS_HI_VALUES_CEN_A              {1991, 1381, 976, 695, 495, 351, 250, 179}

#define MAX_RMS_VLO_VALUES_CEN_A             {6356, 4706, 3317, 2308, 1602, 1112, 778, 546}

/* ATPL360_REG_THRESHOLDS_TABLE_HI, ATPL360_REG_THRESHOLDS_TABLE_VLO. */
/* Thresholds to change impedance mode (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1) from HI / VLO mode. */
/* HI first 8 values (1 per TX level): thresholds to change from HI to LO (0 to disable). */
/* HI next 8 values (1 per TX level): thresholds to change from HI to VLO. When RMS_CALC is below threshold, impedance mode changes to VLO (0 to disable). */
/* VLO first 8 values (1 per TX level): thresholds to change from VLO to LO (0 to disable). */
/* VLO next 8 values (1 per TX level): thresholds to change from VLO to HI. When RMS_CALC is above threshold, impedance mode changes to HI (>=100000 to disable). */
#define TH_HI_VALUES_CEN_A                   {0, 0, 0, 0, 0, 0, 0, 0, 1685, 1173, 828, 589, 419, 298, 212, 151}

#define TH_VLO_VALUES_CEN_A                  {0, 0, 0, 0, 0, 0, 0, 0, 8988, 6370, 4466, 3119, 2171, 1512, 1061, 752}

/* ATPL360_REG_PREDIST_COEF_TABLE_HI, ATPL360_REG_PREDIST_COEF_TABLE_VLO. Equalization values for HI / VLO mode. */
/* Specific gain for each carrier to equalize transmission and compensate HW filter frequency response. */
#define PREDIST_COEF_HI_VALUES_CEN_A         {0x670A, 0x660F, 0x676A, 0x6A6B, 0x6F3F, 0x7440, 0x74ED, 0x7792, 0x762D, 0x7530, 0x7938, 0x7C0A, 0x7C2A, 0x7B0E, \
					      0x7AF2, 0x784B, 0x7899, 0x76F9, 0x76D6, 0x769F, 0x775D, 0x70C0, 0x6EB9, 0x6F18, 0x6F1E, 0x6FA2, 0x6862, 0x67C9, \
					      0x68F9, 0x68A5, 0x6CA3, 0x7153, 0x7533, 0x750B, 0x7B59, 0x7FFF}

#define PREDIST_COEF_VLO_VALUES_CEN_A        {0x7FFF, 0x7DB1, 0x7CE6, 0x7B36, 0x772F, 0x7472, 0x70AA, 0x6BC2, 0x682D, 0x6618, 0x6384, 0x6210, 0x61D7, 0x6244, \
					      0x6269, 0x63A8, 0x6528, 0x65CC, 0x67F6, 0x693B, 0x6B13, 0x6C29, 0x6D43, 0x6E26, 0x6D70, 0x6C94, 0x6BB5, 0x6AC9, \
					      0x6A5F, 0x6B65, 0x6B8C, 0x6A62, 0x6CEC, 0x6D5A, 0x6F9D, 0x6FD3}

/* ATPL360_REG_GAIN_TABLE_HI, ATPL360_REG_GAIN_TABLE_VLO. Gain values for HI / VLO mode {GAIN_INI, GAIN_MIN, GAIN_MAX}. */
#define IFFT_GAIN_HI_INI_CEN_A               142
#define IFFT_GAIN_HI_MIN_CEN_A               70
#define IFFT_GAIN_HI_MAX_CEN_A               336

#define IFFT_GAIN_VLO_INI_CEN_A              474
#define IFFT_GAIN_VLO_MIN_CEN_A              230
#define IFFT_GAIN_VLO_MAX_CEN_A              597

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 CENELEC-A with PLCOUP007/PLCOUP008/PLCOUP011 (only use Branch 0). */
#define DACC_CFG_TABLE_CEN_A                 {0x00000000, 0x00002120, 0x0000073F, 0x00003F3F, 0x00000333, 0x00000000, 0xA20000FF, 0x14141414, \
					      0x00002020, 0x00000044, 0x0FD20004, 0x00000355, 0x0F000000, 0x001020F0, 0x00000355, 0x0F000000, 0x001020FF}

/* ATPL360_REG_NUM_TX_LEVELS: Default value 8. */
#define NUM_TX_LEVELS_CEN_A                  8

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver not used (PL360 + PLCOUPxxx) */
#define PLC_IC_DRV_CFG_CEN_A                 0x00

#endif /* PLCOUP007_CEN_A_H_INCLUDED */
