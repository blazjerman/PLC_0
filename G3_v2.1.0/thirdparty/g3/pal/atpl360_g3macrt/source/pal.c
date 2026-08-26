#include <string.h>
#include "pal.h"
#include "hal/hal.h"
#include "pplc_if.h"
#include "conf_pplc_if.h"
#include "conf_project.h"
#include "conf_usi.h"

/* ATPL360 includes */
#include "atpl360.h"
#include "atpl360_comm.h"
#include "conf_atpl360.h"
#include "coup_tx_config.h"

/* Mac Rt includes */
#include "MacRt.h"
#include "MacRtDefs.h"
#include "MacRtConstants.h"
#include "MacRtMib.h"

/* Mac includes */
#include "mac_wrapper_defs.h"

#ifdef PAL_DEBUG_ENABLE
#define LOG_PAL_DEBUG(a)   printf a
#else
#define LOG_PAL_DEBUG(a)   (void)0
#endif

#ifdef ENABLE_SNIFFER
#define CONF_PHY_SNIFFER_MODE
#endif

/* Data indication Led */
#define COUNT_SWAP_LED    50
static uint32_t sul_count = 0;

/* Upper callbacks info to restore in case of reset from upper layers */
static struct TMacRtNotifications macPalNotifications = {0};

/* HAL functions */
static atpl360_hal_wrapper_t sx_atpl360_hal_wrp;

/* ATPL360 descriptor definition */
static atpl360_descriptor_t sx_atpl360_desc;
static uint8_t suc_err_unexpected;
static uint8_t suc_err_critical;
static uint8_t suc_err_reset;
static uint8_t suc_err_none;
static uint8_t suc_reset_app;
static uint8_t sb_exception_pend;
static uint32_t sul_bin_addr;
static uint32_t sul_bin_size;

/* Control of last transmission in case of unexpected reset */
static bool sb_tx_cfm_pending;

/* PL360 band plan */
static uint8_t suc_phy_band;

/* Flag to control whether PLC transceiver is available */
static bool sb_trx_available;

/* RT MIB backup struct */
static struct TMacRtMib sx_mac_rt_mib;

/* RT MIB default struct at start-up */
static const struct TMacRtMib g_MacRtMibDefaults = {
	0, // m_u32CsmaNoAckCount
	0, // m_u32BadCrcCount
	0, // m_u32RxSegmentDecodeErrorCount
	0, // m_u32RxMACRepetitionCount
	0, // m_u32RxOtherDestinationCount
	0xFFFF, // m_u16PanId
	0xFFFF, // m_u16ShortAddress
	1, // m_u16POSRecentEntries
	0xFFFF, // m_u16RcCoord: set RC_COORD to its maximum value of 0xFFFF
	{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}, // m_ToneMask
	{{0}}, // m_ExtendedAddress
	{{0}}, // m_ForcedToneMap
	{{0}}, // m_ForcedToneMapOnTMResponse
	7, // m_u8HighPriorityWindowSize
	25, // m_u8CsmaFairnessLimit
	8, // m_u8A
	5, // m_u8K
	255, // m_u8MinCwAttempts
	8, // m_u8MaxBe
	0, // m_u8Bsn -- random
	0, // m_u8Dsn -- random
	50, // m_u8MaxCsmaBackoffs
	5, // m_u8MaxFrameRetries
	3, // m_u8MinBe
	0, // m_u8ForcedModScheme
	0, // m_u8ForcedModType
	0, // m_u8ForcedModSchemeOnTMResponse
	0, // m_u8ForcedModTypeOnTMResponse
	0, // m_u8RetriesToForceRobo
	0, // m_u8TransmitAtten
	255, // m_u8POSTableEntryTtl
	120, // m_u8POSRecentEntryThreshold
	60, // m_u8TrickleMinLQI
	3, // m_u8DuplicateDetectionTtl
	10, // m_u8TmrTtl
	12, // m_u8BeaconRandomizationWindowLength
	8, // m_u8PreambleLength
	false, // m_bBroadcastMaxCwEnable
	false, // m_bCoordinator
	false, // m_bPromiscuousMode
	false, // m_bMacSniffer
	false, // m_bLBPFrameReceived
	false, // m_bLNGFrameReceived
	false, // m_bTxHighPriority
};

#ifdef CONF_PHY_SNIFFER_MODE
#include "usi.h"
static x_usi_serial_cmd_params_t sx_usi_msg;

/* PHY sniffer buffer */
static uint8_t spuc_phy_sniffer_buf[div8_ceil(sizeof(phy_snf_frm_t)) << 3];

/**
 * \brief Handler to receive serial data from Sniffer APP.
 */
static uint8_t _handler_app_serial_event(uint8_t *px_serial_data, uint16_t us_len)
{
	(void)px_serial_data;
	(void)us_len;
	return USI_STATUS_OK;
}

static void _phy_sniffer_handler(uint16_t us_len)
{
	/* Send to USI */
	sx_usi_msg.us_len = us_len;
	usi_send_cmd(&sx_usi_msg);
}

void pal_sniffer_mode_enable(void)
{
	if (!sb_trx_available) {
		/* Ignore request */
		return;
	}

	/* Set buffer from PAL */
	atpl360_sniffer_mode_enable(spuc_phy_sniffer_buf, _phy_sniffer_handler);
}

void pal_sniffer_mode_disable(void)
{
	if (!sb_trx_available) {
		/* Ignore request */
		return;
	}

	/* Reset pointer to the callback function */
	atpl360_sniffer_mode_disable();
}

#endif

static void _process_exception_event_cb(atpl360_exception_t exception)
{
	switch (exception) {
	case ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS:
		suc_err_unexpected++;
		break;

	case ATPL360_EXCEPTION_SPI_CRITICAL_ERROR:
		suc_err_critical++;
		break;

	case ATPL360_EXCEPTION_RESET:
		suc_err_reset++;
		break;

	default:
		suc_err_none++;
	}

	LOG_PAL_DEBUG(("_exception_event_cb %u\r\n", exception));

	sb_exception_pend = true;
}

static void _upd_mib_backup_info(enum EMacRtPibAttribute eAttribute, uint8_t *pValue)
{
	switch (eAttribute) {
	case MAC_RT_PIB_CSMA_NO_ACK_COUNT:
		sx_mac_rt_mib.m_u32CsmaNoAckCount = *(uint32_t *)pValue;
		break;

	case MAC_RT_PIB_BAD_CRC_COUNT:
		sx_mac_rt_mib.m_u32BadCrcCount = *(uint32_t *)pValue;
		break;

	case MAC_RT_PIB_MANUF_RX_SEGMENT_DECODE_ERROR_COUNT:
		sx_mac_rt_mib.m_u32RxSegmentDecodeErrorCount = *(uint32_t *)pValue;
		break;

	case MAC_RT_PIB_PAN_ID:
		sx_mac_rt_mib.m_nPanId = *(uint16_t *)pValue;
		break;

	case MAC_RT_PIB_SHORT_ADDRESS:
		sx_mac_rt_mib.m_nShortAddress = *(uint16_t *)pValue;
		break;

	case MAC_RT_PIB_TONE_MASK:
		memcpy((uint8_t *)&sx_mac_rt_mib.m_ToneMask, pValue, sizeof(struct TRtToneMask));
		break;

	case MAC_RT_PIB_MANUF_EXTENDED_ADDRESS:
		memcpy((uint8_t *)&sx_mac_rt_mib.m_ExtendedAddress, pValue, sizeof(struct TExtAddress));
		break;

	case MAC_RT_PIB_MANUF_FORCED_TONEMAP:
		memcpy((uint8_t *)&sx_mac_rt_mib.m_ForcedToneMap, pValue, sizeof(struct TRtToneMap));
		break;

	case MAC_RT_PIB_HIGH_PRIORITY_WINDOW_SIZE:
		sx_mac_rt_mib.m_u8HighPriorityWindowSize = *pValue;
		break;

	case MAC_RT_PIB_CSMA_FAIRNESS_LIMIT:
		sx_mac_rt_mib.m_u8CsmaFairnessLimit = *pValue;
		break;

	case MAC_RT_PIB_A:
		sx_mac_rt_mib.m_u8A = *pValue;
		break;

	case MAC_RT_PIB_K:
		sx_mac_rt_mib.m_u8K = *pValue;
		break;

	case MAC_RT_PIB_MIN_CW_ATTEMPTS:
		sx_mac_rt_mib.m_u8MinCwAttempts = *pValue;
		break;

	case MAC_RT_PIB_MAX_BE:
		sx_mac_rt_mib.m_u8MaxBe = *pValue;
		break;

	case MAC_RT_PIB_MAX_CSMA_BACKOFFS:
		sx_mac_rt_mib.m_u8MaxCsmaBackoffs = *pValue;
		break;

	case MAC_RT_PIB_MAX_FRAME_RETRIES:
		sx_mac_rt_mib.m_u8MaxFrameRetries = *pValue;
		break;

	case MAC_RT_PIB_MIN_BE:
		sx_mac_rt_mib.m_u8MinBe = *pValue;
		break;

	case MAC_RT_PIB_MANUF_FORCED_MOD_SCHEME:
		sx_mac_rt_mib.m_u8ForcedModScheme = *pValue;
		break;

	case MAC_RT_PIB_MANUF_FORCED_MOD_TYPE:
		sx_mac_rt_mib.m_u8ForcedModType = *pValue;
		break;

	case MAC_RT_PIB_MANUF_RETRIES_LEFT_TO_FORCE_ROBO:
		sx_mac_rt_mib.m_u8RetriesToForceRobo = *pValue;
		break;

	case MAC_RT_PIB_TRANSMIT_ATTEN:
		sx_mac_rt_mib.m_u8TransmitAtten = *pValue;
		break;

	case MAC_RT_PIB_BROADCAST_MAX_CW_ENABLE:
		sx_mac_rt_mib.m_bBroadcastMaxCwEnable = *pValue;
		break;

	case MAC_RT_PIB_PREAMBLE_LENGTH:
		sx_mac_rt_mib.m_u8PreambleLength = *pValue;
		break;

	case MAC_RT_PIB_GET_SET_ALL_MIB:
		memcpy((uint8_t *)&sx_mac_rt_mib, pValue, sizeof(sx_mac_rt_mib));
		break;

	default:
		break;
	}
}

static void _read_mib_backup_info(void)
{
	struct TMacRtPibValue x_pib_info;

	if (sx_atpl360_desc.get_req(MAC_RT_PIB_GET_SET_ALL_MIB, 0, &x_pib_info) == MAC_RT_STATUS_SUCCESS) {
		/* Update MIB info from PL360 */
		memcpy(&sx_mac_rt_mib, x_pib_info.m_au8Value, sizeof(sx_mac_rt_mib));
	} else {
		/* Update MIB info from default values */
		sx_mac_rt_mib = g_MacRtMibDefaults;
	}
}

static void _restore_mib_backup_info(void)
{
	struct TMacRtPibValue x_pib_info;

	x_pib_info.m_u8Length = sizeof(sx_mac_rt_mib);
	memcpy(&x_pib_info.m_au8Value, (uint8_t *)&sx_mac_rt_mib, sizeof(sx_mac_rt_mib));
	sx_atpl360_desc.set_req(MAC_RT_PIB_GET_SET_ALL_MIB, 0, &x_pib_info);
}

static void _process_data_ind_cb(uint8_t *pMrtsdu, uint16_t u16MrtsduLen)
{
	LOG_PAL_DEBUG(("_process_data_ind_cb %u\r\n", u16MrtsduLen));

	if (!sb_trx_available) {
		/* Indication not propagated */
		return;
	}

	sul_count = COUNT_SWAP_LED;
	platform_led_int_on();

	if (macPalNotifications.m_pDataIndication) {
		macPalNotifications.m_pDataIndication(pMrtsdu, u16MrtsduLen);
	}
}

static void _process_tx_cfm_cb(enum EMacRtStatus eStatus, bool bUpdateTimestamp)
{
	LOG_PAL_DEBUG(("_process_tx_cfm_cb 0x%02x\r\n", eStatus));

	/* Reset tx_cfm flag */
	sb_tx_cfm_pending = false;

	if (macPalNotifications.m_pMacRtTxConfirm) {
		macPalNotifications.m_pMacRtTxConfirm(eStatus, bUpdateTimestamp);
	}
}

static void _process_rx_params_ind_cb(struct TMacRtRxParameters *pParameters)
{
	if (macPalNotifications.m_pMacRtRxParamsIndication) {
		macPalNotifications.m_pMacRtRxParamsIndication(pParameters);
	}
}

static void _process_mac_snf_ind_cb(uint8_t *pMrtsdu, uint16_t u16MrtsduLen)
{
	if (macPalNotifications.m_pSnifferIndication) {
		macPalNotifications.m_pSnifferIndication(pMrtsdu, u16MrtsduLen);
	}
}

static void _process_comm_status_ind_cb(uint8_t *pMrtsdu)
{
	if (macPalNotifications.m_pCommStatusIndication) {
		macPalNotifications.m_pCommStatusIndication(pMrtsdu);
	}
}

static void _restore_configuration(void)
{
	LOG_PAL_DEBUG(("_restore_configuration\r\n"));

	/* Configure Coupling and TX parameters */
	pl360_g3_coup_tx_config(&sx_atpl360_desc, suc_phy_band);

	/* Restore MIB */
	_restore_mib_backup_info();

	/* Report error confirm in case of transmission cfm is pending */
	if (sb_tx_cfm_pending) {
		sb_tx_cfm_pending = false;
		_process_tx_cfm_cb(MAC_RT_STATUS_DENIED, false);
	}
}

/**
 * \brief Get ATPL360 binary addressing.
 *
 * \param pul_address   Pointer to store the initial address of ATPL360 binary data
 * \param u8Band        PHY band: MAC_WRP_BAND_FCC = 2, Otherwise MAC_WRP_BAND_CENELEC_A
 *
 * \return Size of ATPL360 binary file
 */
static uint32_t _get_atpl360_bin_addressing(uint32_t *pul_address, uint8_t u8Band)
{
	uint32_t ul_bin_addr;
	uint8_t *puc_bin_start;
	uint8_t *puc_bin_end;

#if defined(CONF_MULTIBAND_FCC_CENA) || BOARD == PL360G55CF_EK
    #if defined (__CC_ARM)
	if (u8Band == MAC_WRP_BAND_FCC) {
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
	} else {
		extern uint8_t atpl_bin_cena_start[];
		extern uint8_t atpl_bin_cena_end[];
		ul_bin_addr = (int)(atpl_bin_cena_start - 1);
		puc_bin_start = atpl_bin_cena_start - 1;
		puc_bin_end = atpl_bin_cena_end;
	}

    #elif defined (__GNUC__)
	if (u8Band == MAC_WRP_BAND_FCC) {
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
	} else {
		extern uint8_t atpl_bin_cena_start;
		extern uint8_t atpl_bin_cena_end;
		ul_bin_addr = (int)&atpl_bin_cena_start;
		puc_bin_start = (int)&atpl_bin_cena_start;
		puc_bin_end = (int)&atpl_bin_cena_end;
	}

    #elif defined (__ICCARM__)
	if (u8Band == MAC_WRP_BAND_FCC) {
		#pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
	} else {
		#pragma section = "P_atpl_bin_cena"
		extern uint8_t atpl_bin_cena;
		ul_bin_addr = (int)&atpl_bin_cena;
		puc_bin_start = __section_begin("P_atpl_bin_cena");
		puc_bin_end = __section_end("P_atpl_bin_cena");
	}

    #else
    #error This compiler is not supported for now.
    #endif
#elif defined(CONF_MULTIBAND_FCC_CENB)
    #if defined (__CC_ARM)
	if (u8Band == MAC_WRP_BAND_FCC) {
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
	} else {
		extern uint8_t atpl_bin_cenb_start[];
		extern uint8_t atpl_bin_cenb_end[];
		ul_bin_addr = (int)(atpl_bin_cenb_start - 1);
		puc_bin_start = atpl_bin_cenb_start - 1;
		puc_bin_end = atpl_bin_cenb_end;
	}

    #elif defined (__GNUC__)
	if (u8Band == MAC_WRP_BAND_FCC) {
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
	} else {
		extern uint8_t atpl_bin_cenb_start;
		extern uint8_t atpl_bin_cenb_end;
		ul_bin_addr = (int)&atpl_bin_cenb_start;
		puc_bin_start = (int)&atpl_bin_cenb_start;
		puc_bin_end = (int)&atpl_bin_cenb_end;
	}

    #elif defined (__ICCARM__)
	if (u8Band == MAC_WRP_BAND_FCC) {
		#pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
	} else {
		#pragma section = "P_atpl_bin_cenb"
		extern uint8_t atpl_bin_cenb;
		ul_bin_addr = (int)&atpl_bin_cenb;
		puc_bin_start = __section_begin("P_atpl_bin_cenb");
		puc_bin_end = __section_end("P_atpl_bin_cenb");
	}

    #else
    #error This compiler is not supported for now.
    #endif
#else
	(void)u8Band;
    #if defined (__CC_ARM)
	extern uint8_t atpl_bin_start[];
	extern uint8_t atpl_bin_end[];
	ul_bin_addr = (int)(atpl_bin_start - 1);
	puc_bin_start = atpl_bin_start - 1;
	puc_bin_end = atpl_bin_end;
    #elif defined (__GNUC__)
	extern uint8_t atpl_bin_start;
	extern uint8_t atpl_bin_end;
	ul_bin_addr = (int)&atpl_bin_start;
	puc_bin_start = (int)&atpl_bin_start;
	puc_bin_end = (int)&atpl_bin_end;
    #elif defined (__ICCARM__)
    #pragma section = "P_atpl_bin"
	extern uint8_t atpl_bin;
	ul_bin_addr = (int)&atpl_bin;
	puc_bin_start = __section_begin("P_atpl_bin");
	puc_bin_end = __section_end("P_atpl_bin");
    #else
    #error This compiler is not supported for now.
  #endif
#endif
	*pul_address = ul_bin_addr;
	/* cppcheck-suppress deadpointer */
	return ((uint32_t)puc_bin_end - (uint32_t)puc_bin_start);
}

void MacRtInitialize(uint8_t u8Band, struct TMacRtNotifications *pNotifications)
{
	atpl360_dev_callbacks_t x_atpl360_cbs;
	uint8_t uc_ret;

	sb_tx_cfm_pending = false;

	suc_err_unexpected = 0;
	suc_err_critical = 0;
	suc_err_reset = 0;
	suc_err_none = 0;
	suc_reset_app = 0;

	/* Capture G3 Phy band */
	suc_phy_band = u8Band;

	/* Init ATPL360 */
	sx_atpl360_hal_wrp.plc_init = pplc_if_init;
	sx_atpl360_hal_wrp.plc_reset = pplc_if_reset;
	sx_atpl360_hal_wrp.plc_set_stby_mode = pplc_if_set_stby_mode;
	sx_atpl360_hal_wrp.plc_set_handler = pplc_if_set_handler;
	sx_atpl360_hal_wrp.plc_send_boot_cmd = pplc_if_send_boot_cmd;
	sx_atpl360_hal_wrp.plc_write_read_cmd = pplc_if_send_wrrd_cmd;
	sx_atpl360_hal_wrp.plc_enable_int = pplc_if_enable_interrupt;
	sx_atpl360_hal_wrp.plc_delay = pplc_if_delay;
	sx_atpl360_hal_wrp.plc_get_thw = pplc_if_get_thermal_warning;
	atpl360_init(&sx_atpl360_desc, &sx_atpl360_hal_wrp);

	/* Set phy callbacks */
	macPalNotifications = *pNotifications;

	x_atpl360_cbs.tx_cfm = _process_tx_cfm_cb;
	x_atpl360_cbs.data_ind = _process_data_ind_cb;
	x_atpl360_cbs.mac_sniffer_ind = _process_mac_snf_ind_cb;
	x_atpl360_cbs.comm_status_ind = _process_comm_status_ind_cb;
	x_atpl360_cbs.rx_parameters_ind = _process_rx_params_ind_cb;
	x_atpl360_cbs.exception_cb = _process_exception_event_cb;
#ifdef PPLC_STBY_GPIO
	x_atpl360_cbs.sleep_mode_cb = _restore_configuration;
#else
	x_atpl360_cbs.sleep_mode_cb = NULL;
#endif
	x_atpl360_cbs.debug_mode_cb = _restore_configuration;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);
	
#ifdef CONF_PHY_SNIFFER_MODE
	/* Set USI callback : Sniffer iface */
	usi_set_callback(PROTOCOL_SNIF_G3, _handler_app_serial_event, PHY_SNIFFER_SERIAL_PORT);
#endif

	/* ATPL360 bin file addressing */
	sul_bin_size = _get_atpl360_bin_addressing(&sul_bin_addr, suc_phy_band);

	/* Init ATPL360 */
	uc_ret = atpl360_enable(sul_bin_addr, sul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		LOG_PAL_DEBUG(("\r\n MacRtInitialize ERROR!(%d)\r\n", uc_ret));
		/* No transceiver available */
		sb_trx_available = false;
		/* clear exception control and return */
		sb_exception_pend = false;
		return;
	}

	/* Trasnceiver available */
	sb_trx_available = true;

	/* clear exception control */
	sb_exception_pend = false;

	/* Configure Coupling and TX parameters */
	pl360_g3_coup_tx_config(&sx_atpl360_desc, suc_phy_band);

	/* Get MIB backup info by default */
	_read_mib_backup_info();

#ifdef CONF_PHY_SNIFFER_MODE
	/* Init USI */
	sx_usi_msg.uc_protocol_type = PROTOCOL_SNIF_G3;
	sx_usi_msg.ptr_buf = spuc_phy_sniffer_buf;

	/* Enable PHY Sniffer Mode */
	pal_sniffer_mode_enable();
#endif

	LOG_PAL_DEBUG(("MacRtInitialize ok\r\n"));
}

void MacRtEventHandler(void)
{
	if (!sb_trx_available) {
		/* Return without calling lower layers */
		return;
	}

	/* Manage ATPL360 exceptions */
	if (sb_exception_pend) {
		sb_exception_pend = false;

		/* Restore Configuration */
		_restore_configuration();
	}

	/* Check ATPL360 pending events */
	atpl360_handle_events();

	/* Manage Data Indication Led */
	if (sul_count) {
		if (!--sul_count) {
			platform_led_int_off();
		}
	}
}

void MacRtTxRequest(uint8_t *pTxSdu, uint16_t u16TxSduLen)
{
	LOG_PAL_DEBUG(("MacRtTxRequest 0x%02x\r\n", u16TxSduLen));

	if (sb_trx_available) {
		sb_tx_cfm_pending = true;
		sx_atpl360_desc.tx_request(pTxSdu, u16TxSduLen);
	} else {
		sb_tx_cfm_pending = false;
		/* Notify callback error */
		_process_tx_cfm_cb(MAC_RT_STATUS_DENIED, false);
	}
}

void MacRtResetRequest(bool bResetMib)
{
	atpl360_dev_callbacks_t x_atpl360_cbs;
	uint8_t uc_ret;

	LOG_PAL_DEBUG(("MacRtResetRequest\r\n"));

	suc_reset_app++;

	/* Init ATPL360 */
	atpl360_init(&sx_atpl360_desc, &sx_atpl360_hal_wrp);

	/* Set phy callbacks */
	x_atpl360_cbs.tx_cfm = _process_tx_cfm_cb;
	x_atpl360_cbs.data_ind = _process_data_ind_cb;
	x_atpl360_cbs.mac_sniffer_ind = _process_mac_snf_ind_cb;
	x_atpl360_cbs.comm_status_ind = _process_comm_status_ind_cb;
	x_atpl360_cbs.rx_parameters_ind = _process_rx_params_ind_cb;
	x_atpl360_cbs.exception_cb = _process_exception_event_cb;
#ifdef PPLC_STBY_GPIO
	x_atpl360_cbs.sleep_mode_cb = _restore_configuration;
#else
	x_atpl360_cbs.sleep_mode_cb = NULL;
#endif
	x_atpl360_cbs.debug_mode_cb = _restore_configuration;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);

	/* Reset ATPL360 */
	uc_ret = atpl360_enable(sul_bin_addr, sul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		printf("\r\nmain: atpl360_enable call error!(%d)\r\n", uc_ret);
		/* No transceiver available */
		sb_trx_available = false;
		/* Set MIB defaults */
		sx_mac_rt_mib = g_MacRtMibDefaults;
		/* Clear exception control */
		sb_exception_pend = false;
	}
	else {
		/* Trasnceiver available */
		sb_trx_available = true;
		
		/* Clear exception control */
		sb_exception_pend = false;

		/* Configure Coupling and TX parameters */
		pl360_g3_coup_tx_config(&sx_atpl360_desc, suc_phy_band);

		/* Restore MIB */
		if (!bResetMib) {
			/* Restore MIB */
			_restore_mib_backup_info();
		} else {
			/* Read MIB by default */
			_read_mib_backup_info();
		}
	}
}

uint32_t MacRtGetPhyTime(void)
{
	LOG_PAL_DEBUG(("MacRtGetPhyTime\r\n"));
	if (sb_trx_available) {
		return sx_atpl360_desc.get_timer_ref();
	}
	else {
		/* return dummy data */
		return 0xAA55AA55;
	}
}

void MacRtSetCoordinator(void)
{
	LOG_PAL_DEBUG(("MacRtSetCoordinator\r\n"));
	if (!sb_trx_available) {
		/* Ignore request */
		return;
	}
	sx_atpl360_desc.set_coordinator();
}

enum EMacRtStatus MacRtGetRequestSync(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, struct TMacRtPibValue *pValue)
{
	enum EMacRtStatus uc_res;

	if (!sb_trx_available) {
		/* Ignore request */
		return MAC_RT_STATUS_INVALID_PARAMETER;
	}

	uc_res = sx_atpl360_desc.get_req(eAttribute, u16Index, pValue);
	LOG_PAL_DEBUG(("MacRtGetRequestSync 0x%08x %u\r\n", eAttribute, uc_res));

	return uc_res;
}

enum EMacRtStatus MacRtSetRequestSync(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, const struct TMacRtPibValue *pValue)
{
	enum EMacRtStatus uc_res;

	if (!sb_trx_available) {
		/* Ignore request */
		return MAC_RT_STATUS_INVALID_PARAMETER;
	}

	_upd_mib_backup_info(eAttribute, (uint8_t *)pValue->m_au8Value);

	uc_res = sx_atpl360_desc.set_req(eAttribute, u16Index, pValue);
	LOG_PAL_DEBUG(("MacRtSetRequestSync 0x%08x %u\r\n", eAttribute, uc_res));

	return uc_res;
}
