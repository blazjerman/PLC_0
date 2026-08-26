/**
 * \file
 *
 * \brief Example general configuration.
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

/* Define work band. In this project Multiband is supported (CONF_MULTIBAND_FCC_CENA). Thus, the defined work band is the band used by default. */
#define CONF_BAND_CENELEC_A
/* #define CONF_BAND_FCC */

/* Enable setting Tone Mask (Static Notching) in order to not use all band carriers */
/* If the following line is uncommented, ATPL360_REG_TONE_MASK is set at initialization with value defined in TONE_MASK_STATIC_NOTCHING */
/* #define CONF_TONEMASK_STATIC_NOTCHING */

/* For this project, CENELEC-A and FCC binaries are linked, so Multiband is supported */
#define CONF_MULTIBAND_FCC_CENA

/* PL360G55CF_EK uses PLCOUP011_v1 by default */
#define CONF_ENABLE_C11_CFG

#ifdef CONF_TONEMASK_STATIC_NOTCHING
/* Each carrier corresponding to the band can be notched (no energy is sent in those carriers) */
/* Each carrier is represented by one byte (0: carrier used; 1: carrier notched). By default it is all 0's in PL360 device */
/* The length is the number of carriers corresponding to the band in use (see "general_defs.h"). In this case 36 (CENELEC-A) */
/* The same Tone Mask must be set in both transmitter and receiver. Otherwise they don't understand each other */
	#define TONE_MASK_STATIC_NOTCHING     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif

#endif /* CONF_PROJECT_H_INCLUDED */
