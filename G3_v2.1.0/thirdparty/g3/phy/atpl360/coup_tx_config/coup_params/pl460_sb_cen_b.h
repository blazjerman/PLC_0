/**
 * \file
 *
 * \brief PL460 Single Branch G3 CENELEC-B Coupling and TX configuration.
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

#ifndef PL460_SB_CEN_B_H_INCLUDED
#define PL460_SB_CEN_B_H_INCLUDED

/**************************************************************************************************/
/* Coupling and TX parameters for G3 CENELEC-B. ***************************************************/
/* Recommended to use PL460 with CEN-B (Single Branch). *******************************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

/* ATPL360_REG_MAX_RMS_TABLE_HI , ATPL360_REG_MAX_RMS_TABLE_VLO. */
/* Target RMS_CALC in HI / VLO mode for dynamic gain (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1 or 2). */
/* 8 values, corresponding to first 8 TX attenuation levels (3 dB step). */
#define MAX_RMS_HI_VALUES_CEN_B              {1126, 800, 568, 404, 287, 204, 146, 104}

#define MAX_RMS_VLO_VALUES_CEN_B             {3900, 2805, 1996, 1418, 1007, 715, 509, 362}

/* ATPL360_REG_THRESHOLDS_TABLE_HI, ATPL360_REG_THRESHOLDS_TABLE_VLO. */
/* Thresholds to change impedance mode (ATPL360_REG_CFG_AUTODETECT_IMPEDANCE = 1) from HI / VLO mode. */
/* HI first 8 values (1 per TX level): thresholds to change from HI to LO (0 to disable). */
/* HI next 8 values (1 per TX level): thresholds to change from HI to VLO. When RMS_CALC is below threshold, impedance mode changes to VLO (0 to disable). */
/* VLO first 8 values (1 per TX level): thresholds to change from VLO to LO (0 to disable). */
/* VLO next 8 values (1 per TX level): thresholds to change from VLO to HI. When RMS_CALC is above threshold, impedance mode changes to HI (>=100000 to disable). */
#define TH_HI_VALUES_CEN_B                   {0, 0, 0, 0, 0, 0, 0, 0, 998, 709, 504, 358, 254, 182, 129, 93}

#define TH_VLO_VALUES_CEN_B                  {0, 0, 0, 0, 0, 0, 0, 0, 3900, 2805, 1996, 1418, 1007, 715, 509, 362}

/* ATPL360_REG_PREDIST_COEF_TABLE_HI, ATPL360_REG_PREDIST_COEF_TABLE_VLO. Equalization values for HI / VLO mode. */
/* Specific gain for each carrier to equalize transmission and compensate HW filter frequency response. */
#define PREDIST_COEF_HI_VALUES_CEN_B         {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF}

#define PREDIST_COEF_VLO_VALUES_CEN_B        {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF}

/* ATPL360_REG_GAIN_TABLE_HI, ATPL360_REG_GAIN_TABLE_VLO. Gain values for HI / VLO mode {GAIN_INI, GAIN_MIN, GAIN_MAX}. */
#define IFFT_GAIN_HI_INI_CEN_B               221
#define IFFT_GAIN_HI_MIN_CEN_B               110
#define IFFT_GAIN_HI_MAX_CEN_B               496

#define IFFT_GAIN_VLO_INI_CEN_B              786
#define IFFT_GAIN_VLO_MIN_CEN_B              350
#define IFFT_GAIN_VLO_MAX_CEN_B              883

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 CENELEC-B with PL460. */
#define DACC_CFG_TABLE_CEN_B                 {0x00000000, 0x00000000, 0x00000100, 0x00000100, 0x00000000, 0x00000000, 0x9C1800FF, 0x19191919, \
					      0x00000000, 0x00000000, 0x00000004, 0x00000355, 0x00000000, 0x001020F0, 0x00000355, 0x00000000, 0x001020FF}

/* ATPL360_REG_NUM_TX_LEVELS: Default value 8 */
#define NUM_TX_LEVELS_CEN_B                  8

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver used (PL460 Single Branch) */
#define PLC_IC_DRV_CFG_CEN_B                 0x05

#endif /* PL460_SB_CEN_B_H_INCLUDED */
