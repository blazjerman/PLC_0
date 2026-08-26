/**
 * \file
 *
 * \brief DLMS_CLI_LIB : DLMS client lib
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

#ifdef WIN32
#include <stdint.h>
#endif

#if defined (__CC_ARM)
#pragma anon_unions
#endif

#ifndef DLMS_CLIENT_TYPES_H_INCLUDED
#define DLMS_CLIENT_TYPES_H_INCLUDED

#define LEN_ID_PIB_FW_VERSION 16 /* PRIME Fw version length */
#define SIZE_EUI48            6  /* EUI48 size in bytes */
#define SIZE_LN               6  /* DLMS logical name size */
#define SIZE_BOOLEAN          1  /* DLMS boolean size */
#define SIZE_DOUBLE_LONG      4  /* DLMS double long size */
#define SIZE_DOUBLE_LONG_U    4  /* DLMS double long unsigned size */
#define SIZE_INTEGER          1  /* DLMS integer size */
#define SIZE_LONG             2  /* DLMS long size */
#define SIZE_UNSIGNED         1  /* DLMS unsigned size */
#define SIZE_LONG_U           2  /* DLMS long unsigned size */
#define SIZE_LONG_64          8  /* DLMS long 64 size */
#define SIZE_LONG_64_U        8  /* DLMS long 64 unsigned size */
#define SIZE_ENUM             1  /* DLMS enum size */
#define SIZE_FLOAT_32         4  /* DLMS float 32 size */
#define SIZE_FLOAT_64         8  /* DLMS float 64 size */
#define SIZE_DATE_TIME        12 /* DLMS date_time size */
#define SIZE_DATE             5  /* DLMS date size */
#define SIZE_TIME             4  /* DLMS time size */

typedef enum {
	/* SIMPLE DATA TYPES */
	DT_NULL_DATA            = 0,
	DT_BOOLEAN              = 3, /* true - false */
	DT_BIT_STRING           = 4,
	DT_DOUBLE_LONG          = 5, /* int32_t */
	DT_DOUBLE_LONG_UNSIGNED = 6, /* uint32_t */
	DT_OCTET_STRING         = 9,
	DT_VISIBLE_STRING       = 10,
	DT_UTF8_STRING          = 12,
	DT_BCD                  = 13,
	DT_INTEGER              = 15, /* int8_t */
	DT_LONG                 = 16, /* int16_t */
	DT_UNSIGNED             = 17, /* uint8_t */
	DT_LONG_UNSIGNED        = 18, /* uint16_t */
	DT_LONG_64              = 20, /* int64_t */
	DT_LONG_64_UNSIGNED     = 21, /* uint64_t */
	DT_ENUM                 = 22, /* 1 byte */
	DT_FLOAT_32             = 23, /* float (not float_t) */
	DT_FLOAT_64             = 24, /* double (not double_t) */
	DT_DATE_TIME            = 25, /* octet string (12 bytes) */
	DT_DATE                 = 26, /* octet string (5 bytes) */
	DT_TIME                 = 27, /* octet string (4 bytes) */
	/* COMPLEX DATA TYPES */
	DT_ARRAY                = 1,
	DT_STRUCTURE            = 2,
	DT_COMPACT_ARRAY        = 19
} data_type_t;

typedef enum {
	RLRQ_NORMAL = 0,
	RLRQ_URGENT = 1,
	RLRQ_USER_DEFINED = 30,
} rlrq_reason_t;

typedef enum {
	RLRE_NORMAL = 0,
	RLRE_NOT_FINISHED = 1,
	RLRE_USER_DEFINED = 30,
} rlre_reason_t;

/** Data-Access-Result ::= ENUMERATED */
typedef enum {
	DAR_SUCCESS             = 0,            /* success */
	DAR_HW_FAULT            = 1,            /* hardware-fault */
	DAR_TEMP_FAILURE        = 2,            /* temporary-failure */
	DAR_RW_DENIED           = 3,            /* read-write-denied */
	DAR_OBJ_UNDEF           = 4,            /* object-undefined */
	DAR_OBJ_CLASS_INCONS    = 9,            /* object-class-inconsistent */
	DAR_OBJ_UNAVAIL         = 11,           /* object-unavailable */
	DAR_TYPE_UNMATCHED      = 12,           /* type-unmatched */
	DAR_SCOPE_VIOLATED      = 13,           /* scope-of-access-violated */
	DAR_DATA_BLK_UNAVAIL    = 14,           /* data-block-unavailable */
	DAR_LONG_GET_ABRT       = 15,           /* long-get-aborted */
	DAR_NO_LONG_GET_IN_PROG = 16,           /* no-long-get-in-process */
	DAR_LONG_SET_ABRT       = 17,           /* long-set-aborted */
	DAR_NO_LONG_SET_IN_PROG = 18,           /* no-long-set-in-process */
	DAR_DATA_BLK_NUM_INVAL  = 19,           /* data-block-number-invalid */
	DAR_OTHER_REASON        = 250,          /* other-reason */
} data_access_result_t;

/** access-selector requested object */
typedef enum {
	SEL_NONE               = 0,             /* no selector info */
	SEL_IC07               = 7,             /* class_id = 7 attr 2 */
	SEL_IC12               = 12,            /* class_id = 12 attr 3 */
	SEL_IC15               = 15,            /* class_id = 15 attr 2 */
} access_sel_t;

/** access-selector ::= Unsigned8 */
typedef enum {
	SEL_IC07_RANGE         = 1,            /* range_descriptor */
	SEL_IC07_ENTRY         = 2,            /* entry_descriptor */
} access_selector_ic07_t;

/** ConfirmedServiceError CHOICE */
typedef enum {
	CSE_INITIATE_ERROR = 1,
	CSE_READ = 5,
	CSE_WRITE = 6,
} conf_serv_error_t;

/** ServiceError CHOICE */
typedef enum {
	SE_HW_RESOURCE = 1,            /* hardware-resource */
	SE_SERVICE = 3,                /* service */
	SE_DEFINITION = 4,             /* definition */
	SE_ACCESS = 5,                 /* access */
	SE_INITIATE = 6,               /* initiate */
} serv_error_t;

/** hardware-resource [1] IMPLICIT ENUMERATED */
typedef enum {
	HR_OTHER = 0,                  /* other */
	HR_MEM_UNAVAIL = 1,            /* memory-unavailable */
	HR_PROC_RES_UNAVAIL = 2,       /* processor-resource-unavailable */
	HR_MASS_STORAGE_UNAVAIL = 3,   /* mass-storage-unavailable */
	HR_OTHER_UNAVAIL = 4,          /* other-resource-unavailable */
} hw_resource_t;

/** service [3] IMPLICIT ENUMERATED */
typedef enum {
	S_OTHER = 0,                   /* other */
	S_PDU_SIZE = 1,                /* pdu-size */
	S_SERV_UNSUPPORTED = 2,        /* service-unsupported */
} service_t;

/** definition [4] IMPLICIT ENUMERATED */
typedef enum {
	DEF_OTHER = 0,                 /* other */
	DEF_OBJ_UNDEF = 1,             /* object-undefined */
	DEF_OBJ_CLASS_INCONSISTENT = 2, /* object-class-inconsistent */
	DEF_OBJ_ATTR_INCONSISTENT = 3, /* object-attribute-inconsistent */
} definition_t;

/** access [5] IMPLICIT ENUMERATED */
typedef enum {
	ACS_OTHER               = 0,   /* other */
	ACS_SCOPE_VIOLATED      = 1,   /* scope-of-access-violated */
	ACS_OBJ_ACCESS_VIOLATED = 2,   /* object-access-violated */
	ACS_HW_FAULT            = 3,   /* hardware-fault */
	ACS_OBJ_UNAVAIL         = 4,   /* object-unavailable */
} access_t;

/** initiate [6] IMPLICIT ENUMERATED */
typedef enum {
	INI_OTHER = 0,                 /* other */
	INI_DLMS_VER_TOO_LOW   = 1,    /* dlms-version-too-low */
	INI_INCOMPATIBLE_CONF  = 2,    /* incompatible-conformance */
	INI_PDU_SIZE_TOO_SHORT = 3,    /* pdu-size-too-short */
	INI_REF_BY_VDE_HANDLER = 4,    /* refused-by-vde-handler */
} initiate_t;

/** Get-request CHOICE */
typedef enum {
	GET_REQUEST_NORMAL    = 1,     /* get-request-normal */
	GET_REQUEST_NEXT      = 2,     /* get-request-next */
	GET_REQUEST_WITH_LIST = 3,     /* get-request-with-list */
} get_request_t;

/** Get-request CHOICE */
typedef enum {
	SET_REQUEST_NORMAL    = 1,     /* get-request-normal */
	SET_REQUEST_WITH_LIST = 4,     /* get-request-with-list */
} set_request_t;

typedef enum {
	IC01_LOGICAL_NAME = 1,
	IC01_VALUE
} ic01_attr_t;

typedef enum {
	IC03_LOGICAL_NAME = 1,
	IC03_VALUE,
	IC03_SCALER_UNIT
} ic03_attr_t;

typedef enum {
	IC07_LOGICAL_NAME = 1,
	IC07_BUFFER,
	IC07_CAPTURE_OBJECTS,
	IC07_CAPTURE_PERIOD,
	IC07_SORT_METHOD,
	IC07_SORT_OBJECT,
	IC07_ENTRIES_IN_USE,
	IC07_PROFILE_ENTRIES
} ic07_attr_t;

typedef enum {
	IC08_LOGICAL_NAME = 1,
	IC08_TIME,
	IC08_TIME_ZONE,
	IC08_STATUS,
	IC08_DAYLIGHT_BEGIN,
	IC08_DAYLIGHT_END,
	IC08_DAYLIGHT_DEV,
	IC08_DAYLIGHT_ENABLED,
	IC08_CLK_BASE
} ic08_attr_t;

typedef enum {
	IC86_LOGICAL_NAME = 1,
	IC86_FW_VERSION,
	IC86_VENDOR_ID,
	IC86_PRODUCT_ID
} ic86_attr_t;

typedef enum {
	IC90_LOGICAL_NAME = 1,
	IC90_MAC_TX_DATA_PACKET_COUNT,
	IC90_MAC_RX_DATA_PACKET_COUNT,
	IC90_MAC_TX_CMD_PACKET_COUNT,
	IC90_MAC_RX_CMD_PACKET_COUNT,
	IC90_MAC_CSMA_FAIL_COUNT,
	IC90_MAC_CSMA_NO_ACK_COUNT,
	IC90_MAC_BAD_CRC_COUNT,
	IC90_MAC_TX_DATA_BROADCAST_COUNT,
	IC90_MAC_RX_DATA_BROADCAST_COUNT,
} ic90_attr_t;

typedef enum {
	IC91_LOGICAL_NAME = 1,
	IC91_MAC_SHORT_ADDRESS,
	IC91_MAC_RC_COORD,
	IC91_MAC_PAN_ID,
	IC91_MAC_TONE_MASK = 7,
	IC91_MAC_TMR_TTL,
	IC91_MAC_MAX_FRAME_RETRIES,
	IC91_MAC_NEIGHBOUR_TABLE_ENTRY_TTL,
	IC91_MAC_NEIGHBOUR_TABLE,
	IC91_MAC_HIGH_PRIORITY_WINDOW_SIZE,
	IC91_MAC_CSMA_FAIRNESS_LIMIT,
	IC91_MAC_BEACON_RANDOMIZATION_WINDOW_LENGTH,
	IC91_MAC_A,
	IC91_MAC_K,
	IC91_MAC_MIN_CW_ATTEMPTS,
	IC91_MAC_CENELEC_LEGACY_MODE,
	IC91_MAC_FCC_LEGACY_MODE,
	IC91_MAC_MAX_BE = 20,
	IC91_MAC_MAX_CSMA_BACKOFFS,
	IC91_MAC_MIN_BE
} ic91_attr_t;

typedef enum {
	IC92_LOGICAL_NAME = 1,
	IC92_ADP_MAX_HOPS,
	IC92_ADP_WEAK_LQI_VALUE,
	IC92_ADP_SECURITY_LEVEL,
	IC92_ADP_PREFIX_TABLE,
	IC92_ADP_ROUTING_CONFIGURATION,
	IC92_ADP_BROADCAST_LOG_TABLE_ENTRY_TTL,
	IC92_ADP_ROUTING_TABLE,
	IC92_ADP_CONTEXT_INFORMATION_TABLE,
	IC92_ADP_BLACKLIST_TABLE,
	IC92_ADP_BROADCAST_LOG_TABLE,
	IC92_ADP_GROUP_TABLE,
	IC92_ADP_MAX_JOIN_WAIT_TIME,
	IC92_ADP_PATH_DISCOVERY_TIME,
	IC92_ADP_ACTIVE_KEY_INDEX,
	IC92_ADP_METRIC_TYPE,
	IC92_ADP_COORD_SHORT_ADDRESS,
	IC92_ADP_DISABLE_DEFAULT_ROUTING,
	IC92_ADP_DEVICE_TYPE
} ic92_attr_t;

typedef struct {
	uint8_t year_hi;
	uint8_t year_lo;
	uint8_t month;
	uint8_t day_month;
	uint8_t day_week;
} dt_date_t;

typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t hundredths;
} dt_time_t;

typedef struct {
	uint8_t year_hi;
	uint8_t year_lo;
	uint8_t month;
	uint8_t day_month;
	uint8_t day_week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t hundredths;
	uint8_t dev_hi;
	uint8_t dev_lo;
	uint8_t clk_status;
} dt_datetime_t;

typedef struct {
	uint8_t mac_addr[6];
} eui48_t;

typedef struct dlms_object {
	uint8_t obis_code[SIZE_EUI48];
	uint16_t class_id;
	uint8_t attr;
} dlms_object_t;

#define SEL_ACCES_FROM_TO_MAX_SIZE 100
typedef struct {
	data_type_t type;
	uint8_t value[SEL_ACCES_FROM_TO_MAX_SIZE];
	uint16_t length; /* maximum 12 bytes for date-time */
} data_object_t;

typedef struct {
	dlms_object_t restricting_object;
	data_object_t from;
	data_object_t to;
	access_selector_ic07_t selector_ic07;
} access_range_ic07_t;

typedef struct {
	uint32_t from_entry;
	uint32_t to_entry;
	uint16_t from_value;
	uint16_t to_value;
} access_entry_ic07_t;

typedef enum {
	COSEM_LN_REFERENCING = 1,
	COSEM_SN_REFERENCING = 2
} cosem_referencing_t;

typedef enum {
	COSEM_LOWEST_LEVEL_SEC       = 0,
	COSEM_LOW_LEVEL_SEC          = 1,
	COSEM_HIGH_LEVEL_SEC         = 2,
	COSEM_HIGH_LEVEL_SEC_MD5     = 3,
	COSEM_HIGH_LEVEL_SEC_SHA_1   = 4,
	COSEM_HIGH_LEVEL_SEC_GMAC    = 5,
	COSEM_HIGH_LEVEL_SEC_SHA_256 = 6,
	COSEM_HIGH_LEVEL_SEC_ECDSA   = 7
} cosem_auth_mech_t;

typedef enum {
	LLS_FIXED_PWD = 0,
	LLS_ALG_1_PWD = 1,
} password_type_t;

/** Association configuration */
typedef struct {
	uint16_t us_source;
	uint16_t us_destination;
	password_type_t pwd_type;
	uint8_t password[LLS_PASSWORD_LEN + 1];
	cosem_auth_mech_t auth;
} assoc_conf_t;

typedef enum {
	DLMS_ONE_BLOCK          = 0,
	DLMS_FIRST_BLOCK        = 1,
	DLMS_NEXT_BLOCK         = 2,
	DLMS_LAST_BLOCK         = 3
} long_get_state_t;

typedef struct {
	uint8_t remaining_bytes[2 * MAX_APDU_SIZE_SEND];
	uint32_t block;
	uint16_t block_raw_len;
	uint16_t num_remaining_bytes;
	uint16_t total_num_regs;
	long_get_state_t state;
} long_get_t;

typedef struct {
	uint16_t obis_idx;
	uint8_t attr;
} current_req_t;

typedef struct {
	access_sel_t selector;
	union {
		access_entry_ic07_t ic07_entry;
		access_range_ic07_t ic07_range;
	};
} access_selector_t;

typedef enum {
	ASSOC_NOT_ASSOCIATED    = 0,
	ASSOC_WAIT_AARE         = 1,
	ASSOC_WAIT_RLRE         = 2,
	ASSOC_ASSOCIATED        = 3,
} assoc_state_t;

/** Association info */
typedef struct {
	uint16_t us_destination;
	uint16_t us_source;
	uint8_t uc_assoc_idx;
	uint8_t error;
	cosem_auth_mech_t auth_mech;
	current_req_t current_req;
	long_get_t long_get;
	access_selector_t access_selector;
} assoc_info_t;

/** Association info */
typedef struct {
	assoc_state_t assoc_state[DLMS_MAX_ASSOC];
	uint16_t max_apdu_size[DLMS_MAX_ASSOC];
	uint32_t conformance[DLMS_MAX_ASSOC];
	uint16_t us_short_addr;
} node_info_t;

typedef data_access_result_t (*dlms_cli_obis_cb_t)(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr_meth, uint8_t *puc_rx_data, uint16_t us_data_len);

typedef struct {
	uint8_t obis_code[SIZE_EUI48];
	uint16_t class_id;
	dlms_cli_obis_cb_t obis_callback;
} obis_element_conf_t;

/** Connection status: one 4-32 connection per node */
typedef struct {
	uint8_t meter_serial[13 + 1];
	uint8_t pib_fw_version[LEN_ID_PIB_FW_VERSION];
	uint8_t pib_vendor_id[2];
	uint16_t pib_product_id[2];
} meter_params_t;

#endif /* DLMS_CLIENT_TYPES_H_INCLUDED */
