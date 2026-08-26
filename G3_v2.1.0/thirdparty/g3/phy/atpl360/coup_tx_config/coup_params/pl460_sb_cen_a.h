/**
 * \file
 *
 * \brief PL460 Single Branch G3 CENELEC-A Coupling and TX configuration.
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

#ifndef PL460_SB_CEN_A_H_INCLUDED
#define PL460_SB_CEN_A_H_INCLUDED

/**************************************************************************************************/
/* Coupling and TX parameters for G3 CENELEC-A. ***************************************************/
/* Recommended to use PL460 with CEN-A (Single Branch). *******************************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

/* ATPL360_REG_MAX_RMS_TABLE_HI , ATPL360_REG_MAX_RMS_TABLE_VLO. */
/* Target RMS_CALC in HI / VLO mode for dynamic gain (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1 or 2). */
/* 8 values, corresponding to first 8 TX attenuation levels (3 dB step). */
#define MAX_RMS_HI_VALUES_CEN_A              {2226, 1586, 1132, 805, 573, 408, 290, 206}

#define MAX_RMS_VLO_VALUES_CEN_A             {5920, 4604, 3331, 2374, 1686, 1193, 846, 599}

/* ATPL360_REG_THRESHOLDS_TABLE_HI, ATPL360_REG_THRESHOLDS_TABLE_VLO. */
/* Thresholds to change impedance mode (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1) from HI / VLO mode. */
/* HI first 8 values (1 per TX level): thresholds to change from HI to LO (0 to disable). */
/* HI next 8 values (1 per TX level): thresholds to change from HI to VLO. When RMS_CALC is below threshold, impedance mode changes to VLO (0 to disable). */
/* VLO first 8 values (1 per TX level): thresholds to change from VLO to LO (0 to disable). */
/* VLO next 8 values (1 per TX level): thresholds to change from VLO to HI. When RMS_CALC is above threshold, impedance mode changes to HI (>=100000 to disable). */
#define TH_HI_VALUES_CEN_A                   {0, 0, 0, 0, 0, 0, 0, 0, 1884, 1341, 955, 677, 483, 341, 243, 173}

#define TH_VLO_VALUES_CEN_A                  {0, 0, 0, 0, 0, 0, 0, 0, 9551, 6881, 4936, 3541, 2532, 1805, 1290, 922}

/* ATPL360_REG_PREDIST_COEF_TABLE_HI, ATPL360_REG_PREDIST_COEF_TABLE_VLO. Equalization values for HI / VLO mode. */
/* Specific gain for each carrier to equalize transmission and compensate HW filter frequency response. */
#define PREDIST_COEF_HI_VALUES_CEN_A         {0x5620, 0x59C7, 0x5E1E, 0x6333, 0x698B, 0x6F03, 0x72CD, 0x760E, 0x7904, 0x7B57, 0x7D2C, 0x7E72, 0x7F0F, 0x7FC6, \
					      0x7FFF, 0x7ED1, 0x7D11, 0x7BCE, 0x7A1A, 0x777C, 0x7496, 0x720F, 0x6F8E, 0x6BE0, 0x6780, 0x6357, 0x5F5E, 0x5C0C, \
					      0x597B, 0x5782, 0x572D, 0x57A2, 0x5823, 0x59F2, 0x5D86, 0x6153}

#define PREDIST_COEF_VLO_VALUES_CEN_A        {0x7FFF, 0x7F81, 0x7E57, 0x7C6F, 0x7A35, 0x771F, 0x730B, 0x6E99, 0x6A40, 0x6654, 0x62C6, 0x5F77, 0x5CE6, 0x5B68, \
					      0x5A7B, 0x5A08, 0x5A66, 0x5BAD, 0x5D58, 0x5F29, 0x6109, 0x6338, 0x6539, 0x6686, 0x672E, 0x67D2, 0x686D, 0x68D2, \
					      0x68F6, 0x6927, 0x6995, 0x6989, 0x68C3, 0x68D1, 0x69AA, 0x6AC3}

/* ATPL360_REG_GAIN_TABLE_HI, ATPL360_REG_GAIN_TABLE_VLO. Gain values for HI / VLO mode {GAIN_INI, GAIN_MIN, GAIN_MAX}. */
#define IFFT_GAIN_HI_INI_CEN_A               126
#define IFFT_GAIN_HI_MIN_CEN_A               60
#define IFFT_GAIN_HI_MAX_CEN_A               336

#define IFFT_GAIN_VLO_INI_CEN_A              532
#define IFFT_GAIN_VLO_MIN_CEN_A              230
#define IFFT_GAIN_VLO_MAX_CEN_A              597

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 CENELEC-A with PL460. */
#define DACC_CFG_TABLE_CEN_A                 {0x00000000, 0x00000000, 0x00000100, 0x00000100, 0x00000000, 0x00000000, 0x9D5C00FF, 0x14141414, \
					      0x00000000, 0x00000000, 0x00000004, 0x00000355, 0x00000000, 0x001020F0, 0x00000355, 0x00000000, 0x001020FF}

/* ATPL360_REG_NUM_TX_LEVELS: Default value 8. */
#define NUM_TX_LEVELS_CEN_A                  8

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver used (PL460 Single Branch) */
#define PLC_IC_DRV_CFG_CEN_A                 0x05

#endif /* PL460_SB_CEN_A_H_INCLUDED */
