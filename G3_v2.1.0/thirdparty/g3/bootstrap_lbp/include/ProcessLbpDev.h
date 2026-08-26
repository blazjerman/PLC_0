/**
 *
 * \file
 *
 * \brief LBP Processes for Device
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

#include "Timer.h"
#include "AdpApi.h"
#include "AdpSharedTypes.h"
#include "LbpDefs.h"
#include "mac_wrapper_defs.h"

#ifndef __PROCESS_LBP_DEV_H__
#define __PROCESS_LBP_DEV_H__

/* LBP notifications */
struct TLbpNotificationsDev {
	AdpNetworkJoinConfirm fnctJoinConfirm;
	AdpNetworkLeaveConfirm fnctLeaveConfirm;
	AdpNetworkLeaveIndication fnctLeaveIndication;
};

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/

/** The LBP_InitDev primitive restarts the bootstrap protocol
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void LBP_InitDev(void);

/** The LBP_SetNotificationsDev primitive sets the callbacks to be invoked when events have to be notified
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void LBP_SetNotificationsDev(struct TLbpNotificationsDev *pNotifications);

/** The LBP_SetParamDev primitive allows the upper layer to set the value of a parameter in LBP IB.
 ***********************************************************************************************************************
 * @param u32AttributeId IB attribute ID.
 * @param u16AttributeIndex Index when attribute is a list.
 * @param u8AttributeLen Length of value to set.
 * @param pu8AttributeValue Pointer to array containing value.
 * @param pGetConfirm Result info of the Get operation.
 **********************************************************************************************************************/
void LBP_SetParamDev(uint32_t u32AttributeId, uint16_t u16AttributeIndex, uint8_t u8AttributeLen,
		const uint8_t *pu8AttributeValue, struct TLbpSetParamConfirm *pSetConfirm);

/** The LBP_ForceRegister function is used for testing purposes to force the device register in the network
 * without going through the bootstrap process
 ***********************************************************************************************************************
 * @param pEUI64Address Pointer to EUI64 address of the node.
 * @param u16ShortAddress The 16-bit network address to be set
 * @param u16PanId The 16-bit PAN Id to be set
 * @param pGMK Pointer to Group Master Key to set
 **********************************************************************************************************************/
void LBP_ForceRegister(struct TAdpExtendedAddress *pEUI64Address, uint16_t u16ShortAddress, uint16_t u16PanId,
		struct TGroupMasterKey *pGMK);

#endif

/**********************************************************************************************************************/

/** @}
 **********************************************************************************************************************/
