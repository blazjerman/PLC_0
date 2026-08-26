/**
 * \file
 *
 * \brief DLMS_CLI_LIB : DLMS client lib: G3 Profile
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
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

#include "dlms_cli_lib.h"
#include "dlms_cli_data.h"
#include "dlms_cli_data_example.h"

/**
 * \brief OBIS 1-0:99.1.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_1_0_99_1_0_255_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	return obis_1_0_99_1_0_255_example_cb(us_short_addr, px_assoc_info, uc_attr, puc_rx_data, us_data_len);
}

/**
 * \brief OBIS 0-0:1.0.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_1_0_0_255_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	return obis_0_0_1_0_0_255_example_cb(us_short_addr, px_assoc_info, uc_attr, puc_rx_data, us_data_len);
}

/**
 * \brief OBIS 0-0:29.0.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_0_0_255_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	return obis_0_0_29_0_0_255_example_cb(us_short_addr, px_assoc_info, uc_attr, puc_rx_data, us_data_len);
}

/**
 * \brief OBIS 0-0:29.1.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_1_0_255_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	return obis_0_0_29_1_0_255_example_cb(us_short_addr, px_assoc_info, uc_attr, puc_rx_data, us_data_len);
}

/**
 * \brief OBIS 0-0:29.2.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_2_0_255_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	return obis_0_0_29_2_0_255_example_cb(us_short_addr, px_assoc_info, uc_attr, puc_rx_data, us_data_len);
}
