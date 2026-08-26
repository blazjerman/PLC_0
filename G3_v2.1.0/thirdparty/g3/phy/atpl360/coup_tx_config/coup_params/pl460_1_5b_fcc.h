/**
 * \file
 *
 * \brief PL460 1.5 Branch G3 FCC Coupling and TX configuration.
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

#ifndef PL460_1_5B_FCC_H_INCLUDED
#define PL460_1_5B_FCC_H_INCLUDED

/**************************************************************************************************/
/* Coupling and TX parameters for G3 FCC. *********************************************************/
/* Recommended to use PL460 with FCC-1.5B (1.5 Branch). *******************************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

/* ATPL360_REG_MAX_RMS_TABLE_HI , ATPL360_REG_MAX_RMS_TABLE_VLO. */
/* Target RMS_CALC in HI / VLO mode for dynamic gain (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1 or 2). */
/* 8 values, corresponding to first 8 TX attenuation levels (3 dB step). */
#define MAX_RMS_HI_VALUES_FCC                {1064, 763, 549, 394, 283, 204, 148, 108}

#define MAX_RMS_VLO_VALUES_FCC               {3614, 2775, 2009, 1431, 1019, 725, 516, 367}

/* ATPL360_REG_THRESHOLDS_TABLE_HI, ATPL360_REG_THRESHOLDS_TABLE_VLO. */
/* Thresholds to change impedance mode (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1) from HI / VLO mode. */
/* HI first 8 values (1 per TX level): thresholds to change from HI to LO (0 to disable). */
/* HI next 8 values (1 per TX level): thresholds to change from HI to VLO. When RMS_CALC is below threshold, impedance mode changes to VLO (0 to disable). */
/* VLO first 8 values (1 per TX level): thresholds to change from VLO to LO (0 to disable). */
/* VLO next 8 values (1 per TX level): thresholds to change from VLO to HI. When RMS_CALC is above threshold, impedance mode changes to HI (>=100000 to disable). */
#define TH_HI_VALUES_FCC                     {0, 0, 0, 0, 0, 0, 0, 0, 929, 668, 480, 345, 247, 179, 129, 94}

#define TH_VLO_VALUES_FCC                    {0, 0, 0, 0, 0, 0, 0, 0, 9668, 6931, 4955, 3538, 2520, 1793, 1276, 909}

/* ATPL360_REG_PREDIST_COEF_TABLE_HI, ATPL360_REG_PREDIST_COEF_TABLE_VLO. Equalization values for HI / VLO mode. */
/* Specific gain for each carrier to equalize transmission and compensate HW filter frequency response. */
#define PREDIST_COEF_HI_VALUES_FCC           {0x6FFD, 0x6AD0, 0x65CF, 0x6073, 0x5AF7, 0x5618, 0x5158, 0x4CA7, 0x4869, 0x44EC, 0x4222, 0x3FD7, 0x3E4E, 0x3DB9, \
					      0x3DC3, 0x3E05, 0x3E97, 0x3F8B, 0x407B, 0x4130, 0x41D1, 0x4285, 0x4330, 0x4379, 0x4394, 0x43C5, 0x4407, 0x43FA, \
					      0x43C6, 0x43B2, 0x43C5, 0x43B2, 0x435D, 0x4359, 0x43AD, 0x43FB, 0x4437, 0x44CD, 0x45EC, 0x46C7, 0x47D3, 0x48F6, \
					      0x4ABD, 0x4C07, 0x4D9C, 0x4F0B, 0x5125, 0x52CE, 0x5479, 0x564A, 0x5844, 0x5A45, 0x5BE9, 0x5DAC, 0x5F88, 0x617E, \
					      0x62F0, 0x64A8, 0x66AA, 0x68AB, 0x6A56, 0x6BCB, 0x6DC4, 0x6F84, 0x70F2, 0x7268, 0x747B, 0x7660, 0x77D4, 0x79E1, \
					      0x7CD8, 0x7FFF}

#define PREDIST_COEF_VLO_VALUES_FCC          {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF}

/* ATPL360_REG_GAIN_TABLE_HI, ATPL360_REG_GAIN_TABLE_VLO. Gain values for HI / VLO mode {GAIN_INI, GAIN_MIN, GAIN_MAX}. */
#define IFFT_GAIN_HI_INI_FCC                 105
#define IFFT_GAIN_HI_MIN_FCC                 50
#define IFFT_GAIN_HI_MAX_FCC                 256

#define IFFT_GAIN_VLO_INI_FCC                364
#define IFFT_GAIN_VLO_MIN_FCC                180
#define IFFT_GAIN_VLO_MAX_FCC                408

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 FCC with PL460. */
#define DACC_CFG_TABLE_FCC                   {0x00000000, 0x00000000, 0x00000100, 0x00000100, 0x00000000, 0x00000000, 0x4F5000FF, 0x1B1B1B1B, \
					      0x00000000, 0x00000000, 0x00000006, 0x00000355, 0x00000000, 0x001020F0, 0x00000355, 0x00000000, 0x001020FF}

/* ATPL360_REG_NUM_TX_LEVELS: Default value 8. */
#define NUM_TX_LEVELS_FCC                    8

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver used (PL460 1.5 Branch) */
#define PLC_IC_DRV_CFG_FCC                   0x07

#endif /* PL460_1_5B_FCC_H_INCLUDED */
