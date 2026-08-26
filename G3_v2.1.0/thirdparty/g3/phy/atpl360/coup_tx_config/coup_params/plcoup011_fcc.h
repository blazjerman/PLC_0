/**
 * \file
 *
 * \brief PLCOUP011 G3 FCC Coupling and TX configuration.
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

#ifndef PLCOUP011_FCC_H_INCLUDED
#define PLCOUP011_FCC_H_INCLUDED

/**************************************************************************************************/
/* Coupling and TX parameters for G3 FCC. *********************************************************/
/* Recommended to use PLCOUP011_v1 coupling board (FCC Single Branch). ****************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

/**************************************************************************************************/
/* PLCOUP011 configuration based on PLCOUP006 *****************************************************/
/* Same configuration for HI and VLO modes (only VLO branch from PLCOUP006) ***********************/
/* Redefine thresholds to never change HI / VLO TX mode *******************************************/
/**************************************************************************************************/
#include "plcoup006_fcc.h"

#undef MAX_RMS_HI_VALUES_FCC
#define MAX_RMS_HI_VALUES_FCC                MAX_RMS_VLO_VALUES_FCC

#undef TH_HI_VALUES_FCC
#define TH_HI_VALUES_FCC                     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#undef IFFT_GAIN_HI_INI_FCC
#undef IFFT_GAIN_HI_MAX_FCC
#undef IFFT_GAIN_VLO_MIN_FCC
#define IFFT_GAIN_HI_INI_FCC                 IFFT_GAIN_VLO_INI_FCC
#define IFFT_GAIN_HI_MAX_FCC                 IFFT_GAIN_VLO_MAX_FCC
#define IFFT_GAIN_VLO_MIN_FCC                IFFT_GAIN_HI_MIN_FCC

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for PRIME Channel 2 - 8 with PLCOUP011 (use only Branch 1). */
#undef DACC_CFG_TABLE_FCC
#define DACC_CFG_TABLE_FCC                   {0x00000000, 0x21200000, 0x073F0000, 0x3F3F0000, 0x00000CCC, 0x00000000, 0x2A3000FF, 0x1B1B1B1B, \
					      0x10100000, 0x00001100, 0x04380006, 0x000003AA, 0xF0000000, 0x001020FF, 0x000003AA, 0xF0000000, 0x001020FF}

#endif /* PLCOUP011_FCC_H_INCLUDED */
