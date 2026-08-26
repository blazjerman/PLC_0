/**
 * \file
 *
 * \brief PPLC interface Configuration.  [PLCBOARD - PL485]
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

#ifndef CONF_PPLC_IF_H_INCLUDE
#define CONF_PPLC_IF_H_INCLUDE

/* Select the SPI module that PPLC is connected to */
#define PPLC_SPI_MODULE               ATPL360_SPI

/* Chip select used by PPLC internal peripheral  */
#define PPLC_CS                       ATPL360_SPI_CS

/* Interruption pin used by PPLC internal peripheral */
#define PPLC_INT_GPIO                 ATPL360_INT_GPIO
#define PPLC_INT_FLAGS                ATPL360_INT_FLAGS
#define PPLC_INT_SENSE                ATPL360_INT_SENSE

#define PPLC_INT_MASK                 ATPL360_INT_MASK
#define PPLC_INT_PIO                  ATPL360_INT_PIO
#define PPLC_INT_ID                   ATPL360_INT_ID
#define PPLC_INT_TYPE                 ATPL360_INT_TYPE
#define PPLC_INT_ATTR                 ATPL360_INT_ATTR
#define PPLC_INT_IRQn                 ATPL360_INT_IRQn

/* Programmable Clock Settings (Hz) */
#define PPLC_CLOCK                    8000000

/* ATPL360 Reset pin definition */
#define PPLC_RESET_GPIO               ATPL360_RESET_GPIO
#define PPLC_RESET_ACTIVE_LEVEL       ATPL360_RESET_ACTIVE_LEVEL
#define PPLC_RESET_INACTIVE_LEVEL     ATPL360_RESET_INACTIVE_LEVEL

/* ATPL360 LDO Enable pin definition */
#define PPLC_LDO_EN_GPIO              ATPL360_LDO_EN_GPIO
#define PPLC_LDO_EN_ACTIVE_LEVEL      ATPL360_LDO_EN_ACTIVE_LEVEL
#define PPLC_LDO_EN_INACTIVE_LEVEL    ATPL360_LDO_EN_INACTIVE_LEVEL

/* ATPL360 STBY pin definition */
#define PPLC_STBY_GPIO                ATPL360_STBY_GPIO
#define PPLC_STBY_ACTIVE_LEVEL        ATPL360_STBY_ACTIVE_LEVEL
#define PPLC_STBY_INACTIVE_LEVEL      ATPL360_STBY_INACTIVE_LEVEL

/* ATPL360 Carrier Detect pin definition */
#define PPLC_CD_GPIO                  ATPL360_CD_GPIO

#endif  /* CONF_PPLC_IF_H_INCLUDE */
