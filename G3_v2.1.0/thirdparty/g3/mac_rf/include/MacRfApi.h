/**
 *
 * \file
 *
 * \brief RF MAC Layer API file
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

#ifndef MAC_RF_API_H_
#define MAC_RF_API_H_

#include <MacRfMib.h>

struct TMacNotificationsRF;

/**********************************************************************************************************************/
/** Use this function to initialize the RF MAC layer. The RF MAC layer should be initialized before doing any other operation.
 * The APIs cannot be mixed, if the stack is initialized in ADP mode then only the ADP functions can be used and if the
 * stack is initialized in MAC mode then only MAC functions can be used.
 * @param pNotifications Structure with callbacks used by the MAC layer to notify the upper layer about specific events
 * @param pTables Structure containing pointers to, and sizes of Mac RF Tables (defined outside RF Mac layer and accessed through Pointers)
 **********************************************************************************************************************/
void MacInitializeRF(struct TMacNotificationsRF *pNotifications, struct TMacTablesRF *pTables);

/**********************************************************************************************************************/
/** This function should be called at least every millisecond in order to allow the G3 stack to run and execute its
 * internal tasks.
 **********************************************************************************************************************/
void MacEventHandlerRF(void);

/**********************************************************************************************************************/
/** The MacMcpsDataRequestRF primitive requests the transfer of upper layer PDU to another device or multiple devices.
 * Parameters from struct TMcpsDataRequest
 ***********************************************************************************************************************
 * @param pParameters Request parameters
 **********************************************************************************************************************/
void MacMcpsDataRequestRF(struct TMcpsDataRequest *pParameters);

/**********************************************************************************************************************/
/** The MacMlmeGetRequestSyncRF primitive allows the upper layer to get the value of an attribute from the MAC RF information
 * base in synchronous way.
 ***********************************************************************************************************************
 * @param eAttribute IB identifier
 * @param u16Index IB index
 * @param pValue pointer to results
 **********************************************************************************************************************/
enum EMacStatus MacMlmeGetRequestSyncRF(enum EMacPibAttributeRF eAttribute, uint16_t u16Index, struct TMacPibValue *pValue);

/**********************************************************************************************************************/
/** The MacMlmeSetRequestSyncRF primitive allows the upper layer to set the value of an attribute in the MAC RF information
 * base in synchronous way.
 ***********************************************************************************************************************
 * @param eAttribute IB identifier
 * @param u16Index IB index
 * @param pValue pointer to IB new values
 **********************************************************************************************************************/
enum EMacStatus MacMlmeSetRequestSyncRF(enum EMacPibAttributeRF eAttribute, uint16_t u16Index, const struct TMacPibValue *pValue);

/**********************************************************************************************************************/
/** The MacMlmeResetRequestRF primitive performs a reset of the mac sublayer and allows the resetting of the RF MIB
 * attributes.
 ***********************************************************************************************************************
 * @param pParameters Parameters of the request
 **********************************************************************************************************************/
void MacMlmeResetRequestRF(struct TMlmeResetRequest *pParameters);

/**********************************************************************************************************************/
/** The MacMlmeScanRequestRF primitive allows the upper layer to scan for networks operating in its POS.
 ***********************************************************************************************************************
 * @param pParameters Parameters of the request.
 **********************************************************************************************************************/
void MacMlmeScanRequestRF(struct TMlmeScanRequest *pParameters);

/**********************************************************************************************************************/
/** The MacMlmeStartRequestRF primitive allows the upper layer to request the starting of a new network.
 ***********************************************************************************************************************
 * @param pParameters The parameters of the request
 **********************************************************************************************************************/
void MacMlmeStartRequestRF(struct TMlmeStartRequest *pParameters);

struct TMacNotificationsRF {
  MacMcpsDataConfirm m_McpsDataConfirm;
  MacMcpsDataIndication m_McpsDataIndication;
  MacMlmeResetConfirm m_MlmeResetConfirm;
  MacMlmeBeaconNotify m_MlmeBeaconNotifyIndication;
  MacMlmeScanConfirm m_MlmeScanConfirm;
  MacMlmeStartConfirm m_MlmeStartConfirm;
  MacMlmeCommStatusIndication m_MlmeCommStatusIndication;
  MacMcpsMacSnifferIndication m_McpsMacSnifferIndication;
};

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
