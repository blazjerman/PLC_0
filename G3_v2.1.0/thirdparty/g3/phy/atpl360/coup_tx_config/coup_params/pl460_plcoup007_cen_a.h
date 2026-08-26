/**
 * \file
 *
 * \brief PL460 + PLCOUP007 G3 CENELEC-A Coupling and TX configuration.
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

#ifndef PL460_PLCOUP007_CEN_A_H_INCLUDED
#define PL460_PLCOUP007_CEN_A_H_INCLUDED

/***************************************************************************************************************/
/* Coupling and TX parameters for G3 CENELEC-A. ****************************************************************/
/* Recommended to use PL460 + PLCOUP007_v2/PLCOUP008_v2/PLCOUP011_v1 coupling board (CENELEC-A Single Branch) **/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. *********************************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. ***************/
/***************************************************************************************************************/

/***************************************************************************************************************/
/* Configuration based on PLCOUP007 ****************************************************************************/
/***************************************************************************************************************/
#include "plcoup007_cen_a.h"

/* ATPL360_REG_DACC_TABLE_CFG: Configuration for G3 CENELEC-A 1 with PL460 + PLCOUP007/PLCOUP008/PLCOUP011 (only use Branch 1). */
#undef DACC_CFG_TABLE_CEN_A
#define DACC_CFG_TABLE_CEN_A                {0x00000000, 0x21200000, 0x073F0000, 0x3F3F0000, 0x00000CCC, 0x00000000, 0xA20000FF, 0x14141414, \
					     0x20200000, 0x00004400, 0x0FD20004, 0x000003AA, 0xF0000000, 0x001020F0, 0x000003AA, 0xF0000000, 0x001020FF}

/* ATPL360_REG_PLC_IC_DRIVER_CFG: PLC IC Driver not used (PL460 + PLCOUPxxx) */
#undef PLC_IC_DRV_CFG_CEN_A
#define PLC_IC_DRV_CFG_CEN_A                 0x08

#endif /* PL460_PLCOUP007_CEN_A_H_INCLUDED */
