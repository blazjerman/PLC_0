/**
 * \file
 *
 * \brief DLMS_CLI_LIB : DLMS client lib
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

#ifndef DLMS_CLIENT_LIB_H_INCLUDED
#define DLMS_CLIENT_LIB_H_INCLUDED
#ifndef WIN32
#include "compiler.h"
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "dlms_cli_cnf.h"
#include "dlms_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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
#define MAX_OBJECTS_PER_REQUEST          24

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

#define USR_DIAGN_PROT_VER               2
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

#define AARE_MIN_NUM_TAGS                4

/** Associate-source-diagnostic CHOICE */
#define ACSE_SERVICE_USER                0xA1
#define ACSE_SERVICE_PROVIDER            0xA2

/* AARE result: two most significant bits */
#define AARE_ACCEPTED                    (0x00 << 6)
#define AARE_REJECTED_PERM               (0x01 << 6)
#define AARE_REJECTED_TRAN               (0x02 << 6)
/* AARE Error types */
#define TYPE_OTHER                       0x00
#define TYPE_ACSE_SRV_USR                0x10 /* Associate-source-diagnostic CHOICE [1] */
#define TYPE_ACSE_SRV_PROV               0x20 /* Associate-source-diagnostic CHOICE [2] */
#define TYPE_CONF_SRV_ERR                0x40

/*
 * ----------------------------------------------
 * | RESULT 2 bits | TYPE 2 bits | ERROR 4 bits |
 * ----------------------------------------------
 * */
#define GET_AARE_RESULT(x)               ((x >> 6) & 0x03)
#define GET_AARE_ERR_TYPE(x)             ((x >> 4) & 0x03)
#define GET_AARE_ERROR(x)                ((x >> 0) & 0x0F)

/** acse-service-user [1] MASK_ACSE_SRV_USR */
#define USR_DIAGN_NULL                   0
#define USR_DIAGN_NO_REASON              1
#define USR_DIAGN_APP_CTXT_NAME_NOT_SUP  2
#define USR_DIAGN_CALLING_AP_TITLE_ERR   3
#define USR_DIAGN_CALLING_AP_INV_ID_ERR  4
#define USR_DIAGN_CALLING_AE_QUALIF_ERR  5
#define USR_DIAGN_CALLING_AE_INV_ID_ERR  6
#define USR_DIAGN_CALLED_AP_TITLE_ERR    7
#define USR_DIAGN_CALLED_INV_ID_ERR      8
#define USR_DIAGN_CALLED_AE_QUALIF_ERR   9
#define USR_DIAGN_CALLED_AE_INV_ID_ERR   10
#define USR_DIAGN_AUTH_MECH_NAME_ERR     11
#define USR_DIAGN_AUTH_MECH_NAME_REQ     12
#define USR_DIAGN_AUTH_FAIL              13
#define USR_DIAGN_AUTH_REQ               14
/** acse-service-provider [2] MASK_ACSE_SRV_PROV */
#define PROV_DIAGN_NULL                  0
#define PROV_DIAGN_NO_REASON_GIVEN       1
#define PROV_DIAGN_NO_COMMON_ACSE_VER    2
/** initiate service error MASK_CONF_SRV_ERR */
#define AARQ_INIT_SERV_NO_ERROR          0xFF
#define AARQ_INIT_SERV_ERR_OTHER         0
#define AARQ_INIT_SERV_ERR_LOW_VER       1
#define AARQ_INIT_SERV_ERR_CONFORMANCE   2
#define AARQ_INIT_SERV_ERR_PDUSIZE       3
#define AARQ_INIT_SERV_ERR_VDEHANDLER    4
/** private errors MASK_OTHER */
#define BAD_APDU_LENGTH                  0
#define BAD_USER_INFO_LENGTH             1
#define BAD_SRC_DIAGNOSTIC_LENGTH        2
#define AARE_APDU_TAG_NOT_FOUND          3
#define BAD_AARE_USER_INFO_LEN           4
#define SN_REFERENCING_NOT_SUP           5
#define AARE_APP_CTXT_LEN_ERR            6
#define NEGO_QoS_NOT_SUP                 7
#define BAD_AARE_DLMS_VER                8
#define AARE_FORMAT_ERROR                9
#define BAD_AARE_MIN_APDU_LEN            10
#define BAD_AARE_CONFORMANCE             11
#define AARE_HLS_NOT_SUP                 12
#define AARE_MISSING_TAG                 13

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

/* AARE Tags */
#define AARE_APP_CTXT                    0xA1
#define AARE_RESULT                      0xA2
#define AARE_RESULT_SRC_DIAGNOSTIC       0xA3
#define AARE_RESPONDING_AP_TITLE         0xA4
#define AARE_RESPONDING_AE_QUALIFIER     0xA5
#define AARE_RESPONDING_AP_INVOKE_ID     0xA6
#define AARE_RESPONDING_AE_INVOKE_ID     0xA7
#define AARE_USER_INFO                   0xBE

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

typedef enum {
	DLMS_WAITING          = 0,
	DLMS_SUCCESS          = 1,
	DLMS_TIMEOUT          = 2,
	DLMS_TX_ERROR         = 3,
	DLMS_RX_FAIL          = 4,
	DLMS_DISCONNECTED     = 5,
	DLMS_FORMAT_ERROR     = 6,
	DLMS_RELEASED         = 7,
	DLMS_AA_IDX_ERROR     = 8  /* Application association index error */
} dlms_cli_result_t;

/* DLMS client dlsm_cli_data.h related functions*/
uint8_t dlms_cli_decode_a_xdr_length(uint16_t *pus_value, uint8_t *puc_data);
uint16_t dlms_cli_get_buffer_regs(assoc_info_t *px_assoc_info, uint16_t us_reg_size, uint8_t *puc_data, uint16_t us_data_len, uint8_t *puc_regs);
uint16_t dlms_cli_num_max_nodes(void);

/* User application related functions */
typedef void (*dlms_cli_data_req_cb_t)(uint16_t us_short_address, uint16_t uc_dst, uint16_t uc_src, uint8_t *puc_buff, uint16_t us_buf_len);
typedef void (*dlms_cli_data_resp_cb_t)(uint16_t us_short_address, uint16_t uc_dst, uint16_t uc_src, dlms_cli_result_t x_result, bool b_last_frag);
void dlms_cli_data_ind(uint16_t us_short_addr, uint16_t us_destination, uint16_t us_source, uint8_t *puc_rx_data, uint16_t us_lsdu_len);
void dlms_cli_wrapper_data_ind(uint16_t us_short_addr, uint8_t *puc_wpdu, uint16_t us_wpdu_len);
void dlms_cli_con_opened(uint16_t us_node_idx);
void dlms_cli_con_closed(uint16_t us_node_idx);
uint16_t dlms_cli_conf_obis(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint16_t ic, dlms_cli_obis_cb_t obis_cb);
uint16_t dlms_cli_get_dlms_idx(uint16_t us_short_addr);
uint16_t dlms_cli_add_wrapper_header(uint8_t *puc_wpdu, uint16_t us_dst_wport, uint16_t us_src_wport, uint8_t *puc_apdu, uint16_t us_apdu_len);
dlms_cli_result_t dlms_cli_aarq_request(uint16_t us_short_addr, uint8_t uc_assoc_idx, uint8_t *puc_passwd);
dlms_cli_result_t dlms_cli_rlrq_request(uint16_t us_short_addr, uint8_t uc_assoc_idx, rlrq_reason_t uc_reason);
dlms_cli_result_t dlms_cli_obj_request(uint16_t us_short_addr, uint8_t uc_assoc_idx, dlms_object_t x_object, access_selector_t *px_sel_access);
dlms_cli_result_t dlms_cli_obj_set(uint16_t us_short_addr, uint8_t uc_assoc_idx, dlms_object_t x_object, access_selector_t *px_sel_access, uint8_t *p_value, uint8_t us_length);
dlms_cli_result_t dlms_cli_list_set(uint16_t us_short_addr, uint8_t uc_assoc_idx, dlms_object_t *x_object_list, uint8_t us_list_length, access_selector_t *px_sel_access, uint8_t *p_values, uint8_t us_length);
dlms_cli_result_t dlms_cli_list_request(uint16_t us_short_addr, uint8_t uc_assoc_idx, dlms_object_t *x_object_list, uint8_t us_list_length, access_selector_t *px_sel_access);
void dlms_cli_process(void);
void dlms_cli_init(const assoc_conf_t *px_assoc_conf, uint8_t uc_assoc_num, node_info_t *px_node_info, uint16_t us_dlms_max_nodes, dlms_cli_data_req_cb_t data_req_cb, dlms_cli_data_resp_cb_t data_resp_cb);

#ifdef __cplusplus
}
#endif

/* @} */
#endif /* DLMS_CLIENT_LIB_H_INCLUDED */
