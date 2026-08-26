/**
 *
 * \file
 *
 * \brief Common Serialization file
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "conf_usi.h"
#include "serial_if_common.h"
#include "serial_if_mac.h"
#include "serial_if_adp.h"
#include "usi.h"
#include "conf_project.h"
#include "AdpApi.h"
#include "mac_wrapper.h"
#include "ProcessLbpCoord.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

static enum ESerialMode e_serial_status = SERIAL_MODE_NOT_INITIALIZED;

/**
 * \brief Display SW version in console
 */
static void _show_version( void )
{
	struct TAdpGetConfirm getConfirm;
	struct TAdpMacGetConfirm x_pib_confirm;

#if defined (CONF_BAND_CENELEC_A)
	printf("G3 Band: CENELEC-A\r\n");
#elif defined (CONF_BAND_CENELEC_B)
	printf("G3 Band: CENELEC-B\r\n");
#elif defined (CONF_BAND_FCC)
	printf("G3 Band: FCC\r\n");
#elif defined (CONF_BAND_ARIB)
	printf("G3 Band: ARIB\r\n");
#else
	printf("G3 Band: CENELEC-A\r\n");
#endif

	AdpGetRequestSync(ADP_IB_SOFT_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		printf("G3 stack version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]);
	}

	AdpGetRequestSync(ADP_IB_MANUF_ADP_INTERNAL_VERSION, 0, &getConfirm);
	if ((getConfirm.m_u8Status == G3_SUCCESS) && (getConfirm.m_u8AttributeLength == 6)) {
		printf("ADP version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				getConfirm.m_au8AttributeValue[0],
				getConfirm.m_au8AttributeValue[1],
				getConfirm.m_au8AttributeValue[2],
				getConfirm.m_au8AttributeValue[3],
				getConfirm.m_au8AttributeValue[4],
				getConfirm.m_au8AttributeValue[5]);
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		printf("MAC version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]);
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION_RF, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		printf("MAC RF version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]);
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_MAC_RT_INTERNAL_VERSION, 0, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 6)) {
		printf("MAC RT version: %hu.%hu.%hu Date: 20%hu-%hu-%hu\r\n",
				x_pib_confirm.m_au8AttributeValue[0],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[4],
				x_pib_confirm.m_au8AttributeValue[5]);
	}

	AdpMacGetRequestSync((uint32_t)MAC_WRP_PIB_MANUF_PHY_PARAM, MAC_WRP_PHY_PARAM_VERSION, &x_pib_confirm);
	if ((x_pib_confirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) && (x_pib_confirm.m_u8AttributeLength == 4)) {
		printf("PHY version: %02x.%02x.%02x.%02x\r\n",
				x_pib_confirm.m_au8AttributeValue[3],
				x_pib_confirm.m_au8AttributeValue[2],
				x_pib_confirm.m_au8AttributeValue[1],
				x_pib_confirm.m_au8AttributeValue[0]);
	}

	return;
}

void adp_mac_serial_if_init(void)
{
	/* Set usi callbacks */
	usi_set_callback(PROTOCOL_MAC_G3, serial_if_g3mac_api_parser, MAC_SERIAL_PORT);
	usi_set_callback(PROTOCOL_ADP_G3, serial_if_g3adp_api_parser, ADP_SERIAL_PORT);
}

void adp_mac_serial_if_process(void)
{
	if (e_serial_status == SERIAL_MODE_ADP_DEV) {
		AdpEventHandler();
	} else if (e_serial_status == SERIAL_MODE_ADP_COORD) {
		AdpEventHandler();
		LBP_UpdateBootstrapSlots();
	} else if (e_serial_status == SERIAL_MODE_MAC) {
		MacWrapperEventHandler();
	}
}

void adp_mac_serial_if_set_state(enum ESerialMode e_state)
{
	e_serial_status = e_state;

	switch (e_state) {
	case SERIAL_MODE_MAC:
		printf("Serial mode initialized as MAC\r\n");
		break;

	case SERIAL_MODE_ADP_DEV:
		printf("Serial mode initialized as ADP in Device Mode\r\n");
		break;

	case SERIAL_MODE_ADP_COORD:
		printf("Serial mode initialized as ADP in Coordinator Mode\r\n");
		break;

	case SERIAL_MODE_NOT_INITIALIZED:
		printf("Serial mode Not initialized\r\n");
		break;
	}
	_show_version();
}

enum ESerialMode  adp_mac_serial_if_get_state(void)
{
	return e_serial_status;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
