/**
 * \file
 *
 * \brief Coupling and TX configuration definitions (G3).
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

#ifndef COUP_DEFS_H_INCLUDED
#define COUP_DEFS_H_INCLUDED

/* Maximum value of ATPL360_REG_NUM_TX_LEVELS */
/* Number of TX attenuation levels (3 dB step) suppoting automatic TX mode */
#define MAX_NUM_TX_LEVELS                8

/* Equalization number of coefficients (number of carriers) for each G3 band */
#define COUP_EQU_NUM_COEF_CEN_A          36
#define COUP_EQU_NUM_COEF_FCC            72
#define COUP_EQU_NUM_COEF_ARIB           54
#define COUP_EQU_NUM_COEF_CEN_B          16

/* Struct definition of Coupling and TX parameters. Equalization not included (size depends on G3 band) */
typedef struct coup_tx_params {
	uint32_t pul_rms_hi[MAX_NUM_TX_LEVELS];
	uint32_t pul_rms_vlo[MAX_NUM_TX_LEVELS];
	uint32_t pul_th_hi[MAX_NUM_TX_LEVELS << 1];
	uint32_t pul_th_vlo[MAX_NUM_TX_LEVELS << 1];
	uint32_t pul_dacc_table[17];
	uint16_t pus_gain_hi[3];
	uint16_t pus_gain_vlo[3];
	uint8_t uc_num_tx_levels;
	uint8_t uc_equ_size;
	uint8_t uc_plc_ic_drv_cfg;
} coup_tx_params_t;

#endif /* COUP_DEFS_H_INCLUDED */
