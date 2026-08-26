/**
 *
 * \file
 *
 * \brief Storage implementation file
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/* System includes */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <hal/hal.h>
#include <conf_hal.h>
#include <gpbr.h>
#include <oss_if.h>

/* Storage includes */
#include "storage.h"

/* #define LOG_STORAGE(a)   printf a */
#define LOG_STORAGE(a)   (void)0

#if defined(_SAMG55J19_)
#define RSTC_SR_RSTTYP_GeneralReset   RSTC_SR_RSTTYP_GENERAL_RST
#endif

static struct TPersistentInfo persistentInfo;

static uint16_t Crc16(const uint8_t *pu8Data, uint32_t u32Length, uint16_t u16Poly, uint16_t u16Crc);
static void _set_persistent_data(struct TAdpNonVolatileData *data);
static void _read_persistent_data_GPBR(struct TAdpNonVolatileData *data);
static void _write_persistent_data_GPBR(struct TAdpNonVolatileData *data);
static bool _update_persistent_data_GPBR(struct TAdpNonVolatileData *data, bool b_upd_info);

/**
 * \brief Stores persistent data.
 *
 * \remarks This function assumes that local variables are updated before storing info.
 */
void store_persistent_info(void)
{
	LOG_STORAGE(("PDD_CB: Persistent data stored.\r\n"));

	/* Persistent info Header */
	persistentInfo.m_u16Version = STORAGE_VERSION;
	persistentInfo.m_u16Crc16 = Crc16((const uint8_t *)(&persistentInfo.m_data), sizeof(struct TAdpNonVolatileData), 0x1021, 0xFFFF);

	/* Write internal data to the persistent storage */
	platform_write_storage(sizeof(struct TPersistentInfo), &persistentInfo);
}

/**
 * \brief Stores persistent data in GPBR.
 *
 * \remarks This function is intended to be triggered every time G3 stack informs of parameters change.
 */
void store_persistent_data_GPBR(struct TAdpNonVolatileData *pNonVolatileData)
{
	memcpy(&persistentInfo.m_data, pNonVolatileData, sizeof(struct TAdpNonVolatileData));
	_write_persistent_data_GPBR(&persistentInfo.m_data);

	LOG_STORAGE(("Persistent data stored in GPBR (fr_cnt: 0x%04x).\r\n", persistentInfo.m_data.m_u32FrameCounter));
}

/**
 * \brief Loads persistent data
 *
 * \remarks sets callbacks to store persistent data periodically and on power off.
 *
 */
void load_persistent_info(void)
{
	bool b_upd_info;
	LOG_STORAGE(("Loading persistent data...\r\n"));

#if defined(PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR) || defined(PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER)
	/* Set callback for power down */
	platform_set_pdd_callback(&store_persistent_info);
	LOG_STORAGE(("Callback to store persistent data set.\r\n"));
#endif

	b_upd_info = true;

	platform_init_storage();

	/* Read the persistent storage */
	if (platform_read_storage(sizeof(struct TPersistentInfo), &persistentInfo)) {
		/* Check the CRC */
		uint16_t u16Crc16 = Crc16((const uint8_t *)(&persistentInfo.m_data), sizeof(struct TAdpNonVolatileData), 0x1021, 0xFFFF);

		if (persistentInfo.m_u16Crc16 != u16Crc16) {
			LOG_STORAGE(("load_persistent_info() CRC error. Read: %u, Calc: %u\r\n", persistentInfo.m_u16Crc16, u16Crc16));
			b_upd_info = false;
		} else if (persistentInfo.m_u16Version != STORAGE_VERSION) {
			LOG_STORAGE(("load_persistent_info() storage version error.\r\n"));
			b_upd_info = false;
		}
	} else {
		LOG_STORAGE(("load_persistent_info() unable to read storage.\r\n"));
		b_upd_info = false;
	}

	/* Increment startup counter */
	persistentInfo.m_u32StartupCounter++;

	/* Set Values to G3 Stack */
	if (_update_persistent_data_GPBR(&persistentInfo.m_data, b_upd_info)) {
		_set_persistent_data(&persistentInfo.m_data);
		LOG_STORAGE(("Persistent data loaded. Startup Counter: %d\r\n", persistentInfo.m_u32StartupCounter));
	}

	/* Pre-erase flash page for further quick writing on power-down */
	platform_erase_storage(sizeof(struct TPersistentInfo));
}

/**
 * \brief Calculates CRC-16
 *
 */
static uint16_t Crc16(const uint8_t *pu8Data, uint32_t u32Length, uint16_t u16Poly, uint16_t u16Crc)
{
	uint8_t u8Index;
	while (u32Length--) {
		u16Crc ^= (*pu8Data++) << 8;
		for (u8Index = 0; u8Index < 8; u8Index++) {
			u16Crc = (u16Crc << 1) ^ ((u16Crc & 0x8000U) ? u16Poly : 0);
		}
	}
	return u16Crc;
}

/**
 * \brief Sets stored info in the G3 stack
 *
 * \param info     Pointer to the persistent info
 *
 */
static void _set_persistent_data(struct TAdpNonVolatileData *data)
{
	struct TAdpMacSetConfirm macSetConfirm;
	struct TAdpSetConfirm adpSetConfirm;

	/* Check for invalid data */
	/* In case memory is corrupted, avoid invalid security frame counters */
	if (data->m_u32FrameCounterRF == 0xFFFFFFFF) {
		data->m_u32FrameCounterRF = 0;
	}
	if (data->m_u32FrameCounter == 0xFFFFFFFF) {
		data->m_u32FrameCounter = 0;
	}

	/* Write internal data to the stack */
#if defined(__PLC_MAC__)
	AdpMacSetRequestSync(MAC_WRP_PIB_FRAME_COUNTER, 0, sizeof(data->m_u32FrameCounter),
			(const uint8_t *)(&data->m_u32FrameCounter), &macSetConfirm);
#endif
#if defined(__RF_MAC__)
	AdpMacSetRequestSync(MAC_WRP_PIB_FRAME_COUNTER_RF, 0, sizeof(data->m_u32FrameCounterRF),
			(const uint8_t *)(&data->m_u32FrameCounterRF), &macSetConfirm);
#endif
	AdpSetRequestSync(ADP_IB_MANUF_DISCOVER_SEQUENCE_NUMBER, 0, sizeof(data->m_u16DiscoverSeqNumber),
			(const uint8_t *)(&data->m_u16DiscoverSeqNumber), &adpSetConfirm);
	AdpSetRequestSync(ADP_IB_MANUF_BROADCAST_SEQUENCE_NUMBER, 0, sizeof(data->m_u8BroadcastSeqNumber),
			(const uint8_t *)(&data->m_u8BroadcastSeqNumber), &adpSetConfirm);
}

/**
 * \brief Reads the persistent info from the GPBR registers.
 *
 * \param info     Pointer to the persistent info
 *
 */
static void _read_persistent_data_GPBR(struct TAdpNonVolatileData *data)
{
	data->m_u16DiscoverSeqNumber = gpbr_read(GPBR0);
	data->m_u8BroadcastSeqNumber = (uint8_t)(gpbr_read(GPBR0) >> 16);
	data->m_u32FrameCounter = 0;
	data->m_u32FrameCounterRF = 0;
#if defined(__PLC_MAC__)
	data->m_u32FrameCounter = gpbr_read(GPBR1);
#endif
#if defined(__RF_MAC__)
	data->m_u32FrameCounterRF = gpbr_read(GPBR2);
#endif
}

/**
 * \brief Writes the persistent info in the GPBR registers.
 *
 * \param info     Pointer to the persistent info
 *
 */
static void _write_persistent_data_GPBR(struct TAdpNonVolatileData *data)
{
	uint32_t u32aux = 0;
	u32aux = data->m_u16DiscoverSeqNumber;
	u32aux += ((uint32_t)data->m_u8BroadcastSeqNumber) << 16;
	gpbr_write(GPBR0, u32aux);
#if defined(__PLC_MAC__)
	u32aux = data->m_u32FrameCounter;
	gpbr_write(GPBR1, u32aux);
#endif
#if defined(__RF_MAC__)
	u32aux = data->m_u32FrameCounterRF;
	gpbr_write(GPBR2, u32aux);
#endif
}

/**
 * \brief Updates the persistent info from the GPBR registers.
 *
 * \param info     Pointer to the persistent info
 *
 */
static bool _update_persistent_data_GPBR(struct TAdpNonVolatileData *data, bool b_upd_info)
{
	bool res;

	res = false;

	/* Check last reset type and synchronize status of FC_KEY values between GPBR and FLASH */
#if (SAMG || SAM4E || SAME70 || PIC32CX)
	if ((RSTC_RSTC_SR & RSTC_SR_RSTTYP_Msk) != RSTC_SR_RSTTYP_GENERAL_RST) {
#else
	if ((RSTC_RSTC_SR & RSTC_SR_RSTTYP_Msk) != RSTC_SR_RSTTYP_GeneralReset) {
#endif
		/* Not a power down reset: read from GPBR */
		_read_persistent_data_GPBR(data);
		if (data->m_u32FrameCounter < 0xFFFFFFFF) {
			res = true;
			LOG_STORAGE(("Not a power down reset: read from GPBR.\r\n"));
		} else {
			LOG_STORAGE(("Not a power down reset: GPBR invalid.\r\n"));
		}
	} else {
		/* Power down reset */
		if (b_upd_info) {
			/* Update GPBR */
			_write_persistent_data_GPBR(data);
			res = true;
			LOG_STORAGE(("Power down reset: update GPBR.\r\n"));
		} else {
			/* Do nothing, GPBR's update in each tx */
			LOG_STORAGE(("Power down reset: Do Nothing.\r\n"));
		}
	}

	return res;
}
