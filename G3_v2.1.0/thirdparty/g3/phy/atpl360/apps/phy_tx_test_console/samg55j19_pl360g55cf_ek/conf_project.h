/**
 * \file
 *
 * \brief CONF_EXAMPLE : Example configuration for PLC PHY TX Test Console Application
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

#ifndef CONF_PROJECT_H_INCLUDED
#define CONF_PROJECT_H_INCLUDED

/* PHY Data message start address. */
#define ADDR_APP_PHY_MESSAGE_DATA (IFLASH_ADDR + IFLASH_SIZE - (2 * IFLASH_LOCK_REGION_SIZE))

/* Pin to select G3 FCC/CENELEC-A */
#define SELECT_G3_FCC_PIN              IOPORT_CREATE_PIN(PIOA, 17)

/* PL360 binary at fixed address (0x010E0000) */
/* #define PL360_BIN_ADDR_FIXED */

/* PL360G55CF_EK: Enable selection of PL360 binary depending on G3 band */
#define CONF_MULTIBAND_FCC_CENA

/* PL360G55CF_EK: Enable PLCOUP011 configuration */
#define CONF_ENABLE_C11_CFG

#endif /* CONF_PROJECT_H_INCLUDED */
