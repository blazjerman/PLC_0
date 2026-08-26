/**
 * \file
 *
 * \brief PLC PRIME at45db32 serial flash service
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
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

#ifndef AT45DB32_H_INCLUDED
#define AT45DB32_H_INCLUDED

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup plc_group
 * \defgroup at45db32_plc_group PLC PRIME serial flash service
 *
 * This module provides access to AT45DB32 serial falsh memory in FU process.
 *
 * @{
 */

/*! \name AT45DBX Commands
 */
/* ! @{ */
#define AT45DBX_RD_STATUS_REG        0xD7              /* !< Status Register Read (Serial/8-bit Mode). */
#define AT45DBX_RD_MNFCT_DEV_ID_SM   0x9F              /* !< Manufacturer and Device ID Read (Serial Mode). */
#define AT45DBX_WR_BUF1              0x84              /* !< Buffer 1 Write (Serial/8-bit Mode). */
#define AT45DBX_RD_BUF1_AF_8M        0x54              /* !< Buffer 1 Read, Any-Frequency Mode (8-bit Mode). */
#define AT45DBX_PR_PAGE_TH_BUF1      0x82              /* !< Main Memory Page Program through Buffer 1 (built-in erase) (Serial/8-bit Mode). */
#define AT45DBX_RD_PAGE              0xD2              /* !< Main Memory Page Read (Serial/8-bit Mode). */
#define AT45DBX_XFR_PAGE_TO_BUF1     0x53              /* !< Main Memory Page to Buffer 1 Transfer (Serial/8-bit Mode). */
#define AT45DBX_PR_BUF1_TO_PAGE_ER   0x83              /* !< Buffer 1 to Main Memory Page Program with Built-in Erase (Serial/8-bit Mode). */
#define AT45DBX_ER_SECTOR            0x7C              /* !< Sector Erase (Serial/8-bit Mode). */
#define AT45DBX_ER_PAGE              0x81              /* !< Page Erase (Serial/8-bit Mode). */
#define AT45DBX_PAGE_BIN_CONFIG      0x3D2A80A6        /* !< Page Size Configuration Commands (Binary mode). */
/* ! @} */

/*! \name AT45DB32 Addressing
 */
/* ! @{ */
#define AT45DB32_PAGE_SIZE           512
#define AT45DB32_PAGE_ADDR_MASK      0x003FFE00
#define AT45DB32_BUFF_ADDR_MASK      0x000001FF
/* ! @} */

/* \name PLC PRIME AT45DB32 interface */
/* @{ */
void at45db32_init(void);
uint8_t at45db32_send_cmd(uint8_t uc_cmd, uint32_t ul_addr, uint8_t *puc_data, uint16_t uc_len);
void at45db32_wait_is_ready(void);

/* @} */

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */

#endif /* AT45DB32_H_INCLUDED */
