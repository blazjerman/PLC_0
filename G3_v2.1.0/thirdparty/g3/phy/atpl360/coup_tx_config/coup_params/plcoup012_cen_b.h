/**
 * \file
 *
 * \brief PLCOUP012 G3 CENELEC-B Coupling and TX configuration.
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

#ifndef PLCOUP012_CEN_B_H_INCLUDED
#define PLCOUP012_CEN_B_H_INCLUDED

/************************************************************************************************************************/
/* Coupling and TX parameters for G3 CENELEC-B. *************************************************************************/
/* Recommended to use PLCOUP012_v1 / PLCOUP013_v1 coupling board (CENELEC-B Single Branch without external amplifier). **/
/************************************************************************************************************************/

/************************************************************************************************************************/
/* PLCOUP012 (using PL360 internal driver, without external amplifier). *************************************************/
/* Same configuration for HI and VLO modes and fixed gain (MAX_RMS and thresholds not needed). **************************/
/************************************************************************************************************************/

#define MAX_RMS_HI_VALUES_CEN_B              {0, 0, 0, 0, 0, 0, 0, 0}
#define MAX_RMS_VLO_VALUES_CEN_B             {0, 0, 0, 0, 0, 0, 0, 0}

#define TH_HI_VALUES_CEN_B                   {0, 0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
#define TH_VLO_VALUES_CEN_B                  {0, 0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}

#define PREDIST_COEF_HI_VALUES_CEN_B         {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF}

#define PREDIST_COEF_VLO_VALUES_CEN_B        {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
					      0x7FFF, 0x7FFF}

#define IFFT_GAIN_HI_INI_CEN_B               645
#define IFFT_GAIN_HI_MIN_CEN_B               645
#define IFFT_GAIN_HI_MAX_CEN_B               645

#define IFFT_GAIN_VLO_INI_CEN_B              645
#define IFFT_GAIN_VLO_MIN_CEN_B              645
#define IFFT_GAIN_VLO_MAX_CEN_B              645

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 CENELEC-B with PLCOUP012/PLCOUP013 (only use 4 EMIT pins with full driving). */
#define DACC_CFG_TABLE_CEN_B                 {0x00000000, 0x00000000, 0x3F3F3F3F, 0x3F3F3F3F, 0x00000FFF, 0x00000000, 0xA0BC00FF, 0x19191919, \
					      0x00000000, 0x00000000, 0x0FD20004, 0x000000FF, 0x0F000000, 0x00102000, 0x000000FF, 0x0F000000, 0x00102000}

/* ATPL360_REG_NUM_TX_LEVELS: Default value 8 */
#define NUM_TX_LEVELS_CEN_B                  8

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver not used (PL360 + PLCOUPxxx) */
#define PLC_IC_DRV_CFG_CEN_B                 0x00

#endif /* PLCOUP012_CEN_B_H_INCLUDED */
