/**
 * \file
 *
 * \brief DLMS_SRV_LIB : DLMS server lib
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
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

#ifndef DLMS_SERVER_LIB_H_INCLUDED
#define DLMS_SERVER_LIB_H_INCLUDED

#include "compiler.h"
#include "dlms_srv_cnf.h"
#include "dlms_srv_types.h"

/* #define DLMS_LIB_DEBUG */

/* Calculate the max or min of two numbers */
#ifndef MAX
   #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
   #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MAX_LENGTH_DATA                  1200
#define DLMS_INVALID_INDEX               0xFFFF

/* Security */
#define MAX_PASSWORD_LEN                 16

/* DLMS Tags defines */
#define CONF_SERV_ERR                    0x0E

#define AARQ_APDU                        0x60
#define AARE_APDU                        0x61
#define RLRQ_APDU                        0x62
#define RLRE_APDU                        0x63

#define GET_REQ                          0xC0
#define SET_REQ                          0xC1
#define EVT_REQ                          0xC2
#define ACT_REQ                          0xC3

#define GET_RESP                         0xC4
#define SET_RESP                         0xC5
#define ACT_RESP                         0xC7

#define AARQ_ERR_PROT_VER                2
#define AARQ_PROT_VER_LEN                2
#define ACSE_SERVER_CHOICE               2

/* AARQ authentication related values */
#define AARQ_ACSE_REQ_LEN                2
#define AARQ_ACSE_REQ                    0x80

#define AARQ_AUTH_VALUE_GRAPHIC_STRING   0x80
#define AARQ_AUTH_VALUE_BITSTRING        0x81
#define AARQ_NUM_AUTH_FIELDS             3
#define AARQ_AUTHENTICATION_REQUIRED     0x0E
#define AUTH_VAL_LEN                     16
#define AUTH_VAL_LEN_ENCODED             (AUTH_VAL_LEN + 2)

/** acse-service-user */
#define AARQ_ERR_NULL                    0
#define AARQ_ERR_NO_REASON               1
#define AARQ_ERR_APP_CTXT_NAME_NOT_SUP   2
#define AARQ_ERR_AUTH_MECH_NAME          11
#define AARQ_ERR_AUTH_MECH_NAME_REQUIRED 12
#define AARQ_ERR_AUTH_FAIL               13
#define AARQ_ERR_AUTH_REQUIRED           14

/** initiate service error */
#define AARQ_INIT_SERV_NO_ERROR          0xFF
#define AARQ_INIT_SERV_ERR_OTHER         0
#define AARQ_INIT_SERV_ERR_LOW_VER       1
#define AARQ_INIT_SERV_ERR_CONFORMANCE   2
#define AARQ_INIT_SERV_ERR_PDUSIZE       3
#define AARQ_INIT_SERV_ERR_VDEHANDLER    4

/** Association result */
#define AARQ_ACCEPTED                    0
#define AARQ_REJECTED_PERMANENT          1
#define AARQ_REJECTED_TRANSIENT          2

/* AARQ Tags */
#define AARQ_PROT_VER                    0x80
#define AARQ_APP_CTXT                    0xA1
#define AARQ_CALLED_AP_TITLE             0xA2
#define AARQ_CALLED_AE_QUALIFIER         0xA3
#define AARQ_CALLED_AP_INVOKE_ID         0xA4
#define AARQ_CALLED_AE_INVOKE_ID         0xA5
#define AARQ_CALLING_AP_TITLE            0xA6
#define AARQ_CALLING_AE_QUALIFIER        0xA7
#define AARQ_CALLING_AP_INVOKE_ID        0xA8
#define AARQ_CALLING_AE_INVOKE_ID        0xA9
#define AARQ_ACSE_REQUIREMENTS           0x8A
#define AARQ_AUTH_MECH_NAME              0x8B
#define AARQ_AUTH_VALUE                  0xAC
#define AARQ_IMPL_INFO                   0xBD
#define AARQ_USER_INFO                   0xBE
#define AARQ_CONF_TAG_HI                 0x5F
#define AARQ_CONF_TAG_LO                 0x1F
#define AARQ_MIN_APDU_LEN                0x0C

/** Header length */
#define AARQ_APP_CTXT_HEAD_LEN           9
#define AARQ_AUTH_MECH_NAME_HEAD_LEN     7
#define AARQ_ASSOC_RESULT_HEAD_LEN       4
#define AARQ_USER_INFO_HEAD_LEN          4
#define AARQ_SRC_DIAG_HEAD_LEN           6
#define AARQ_INIT_SERV_NO_ERROR_HEAD_LEN 11
#define AARQ_INIT_SERV_ERROR_HEAD_LEN    7

/** LN referencing vaa-name dummy value 0x0007 */
#define VAA_NAME_HI                      0x00
#define VAA_NAME_LO                      0x07

typedef void (*dlms_srv_data_req_cb_t)(uint16_t uc_dst, uint16_t uc_src, uint8_t *puc_buff, uint16_t us_buf_len);

/* DLMS Server dlsm_srv_data.h related functions*/
obis_element_conf_t dlms_srv_get_obis_from_idx(uint16_t us_obis_idx);
uint16_t dlms_srv_get_num_objects(void);
void dlms_srv_data_init(meter_params_t *);
uint8_t dlms_srv_encode_a_xdr_length(uint16_t us_value, uint8_t *puc_data);
uint16_t dlms_srv_encode_ic15_object(assoc_info_t *px_assoc_info, uint16_t us_obis_idx, uint8_t *puc_data);
void dlms_srv_encode_ic01(assoc_info_t *px_assoc_info, ic01_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic03(assoc_info_t *px_assoc_info, ic03_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic07(assoc_info_t *px_assoc_info, ic07_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic08(assoc_info_t *px_assoc_info, ic08_attr_t uc_attr, void *pv_data);
void dlms_srv_encode_ic15(assoc_info_t *px_assoc_info, ic15_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic86(assoc_info_t *px_assoc_info, ic86_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic90(assoc_info_t *px_assoc_info, ic91_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic91(assoc_info_t *px_assoc_info, ic91_attr_t uc_attr, void *pv_data, uint16_t us_data_len);
void dlms_srv_encode_ic92(assoc_info_t *px_assoc_info, ic92_attr_t uc_attr, void *pv_data, uint16_t us_data_len);

/* User application related functions */
void dlms_srv_data_ind(uint16_t us_destination, uint16_t us_source, uint8_t *puc_data, uint16_t us_lsdu_len);
void dlms_srv_wrapper_data_ind(uint8_t *puc_wpdu, uint16_t us_wpdu_len);
uint16_t dlms_srv_add_wrapper_header(uint8_t *puc_wpdu, uint16_t us_dst_wport, uint16_t us_src_wport, uint8_t *puc_apdu, uint16_t us_apdu_len);
void dlms_srv_data_cfm(uint16_t us_dst, uint16_t us_src, bool b_success);
void dlms_srv_432_conn_close(void);
uint16_t dlms_srv_conf_obis(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint16_t ic, dlms_srv_obis_cb_t obis_cb, obis_element_conf_t *obis_element);
void dlms_srv_process(void);
void dlms_srv_init(const assoc_conf_t *px_assoc_conf, uint8_t uc_assoc_num, meter_params_t *px_meter_params, dlms_srv_data_req_cb_t data_req_cb);

/* @} */
#endif /* DLMS_SERVER_LIB_H_INCLUDED */
