/**
 * \file
 *
 * \brief G3 FCC Coupling Configuration for PL460 FCC (SB / 1.5B).
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

#ifndef COUP_CONF_H_INCLUDED
#define COUP_CONF_H_INCLUDED

/* Include project configuration */
#include "conf_project.h"

/**************************************************************************************************/
/* Coupling and TX parameter configuration. *******************************************************/
/* IMPORTANT!!! The given values were obtained from calibration with MCHP EKs. ********************/
/* For customer HW designs, calibration values should be checked with MCHP PHY Calibration Tool. **/
/**************************************************************************************************/

#ifndef CONF_ENABLE_PL460_FCC_1_5B_CFG
/* Include PL460 FCC-SB coupling parameters (FCC) */
# include "pl460_sb_fcc.h"
#else
/* Include PL460 FCC-1.5B coupling parameters (FCC) */
# include "pl460_1_5b_fcc.h"
#endif

/* Enable Coupling and TX configuration for FCC */
#define CONF_COUP_FCC_ENABLE

#endif /* COUP_CONF_H_INCLUDED */
