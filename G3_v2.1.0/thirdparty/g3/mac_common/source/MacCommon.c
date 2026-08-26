/**
 *
 * \file
 *
 * \brief Common MAC Layer implementation file
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
#include <string.h>
#include <MacCommon.h>

extern uint32_t oss_get_up_time_ms(void);
#ifdef __PLC_MAC__
extern enum EMacStatus MacSetMacRtAttributeSync(enum EMacCommonPibAttribute eAttribute, uint16_t u16Index, const struct TMacPibValue *pValue);
#endif

struct TMacMibCommon g_MacMibCommon;

static const struct TMacMibCommon g_MacMibDefaultsCommon = {
  0xFFFF, // m_u16PanId
  {0}, // m_ExtendedAddress
  0xFFFF, // m_u16ShortAddress
  false, // m_bPromiscuousMode
  {0}, // m_aKeyTable
  0xFFFF, // m_u16RcCoord: set RC_COORD to its maximum value of 0xFFFF
  255, // m_u8POSTableEntryTtl
  120, // m_u8POSRecentEntryThreshold
  false, // m_bCoordinator
};

static bool alreadyInitialized = false;

void MacCommonInitialize(void)
{
  if (!alreadyInitialized) {
    g_MacMibCommon = g_MacMibDefaultsCommon;
  }
}

void MacCommonReset(void)
{
  alreadyInitialized = false;
  MacCommonInitialize();
}

static enum EMacStatus MacPibGetExtendedAddress(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_ExtendedAddress);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_ExtendedAddress, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetExtendedAddress(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_ExtendedAddress)) {
    memcpy(&g_MacMibCommon.m_ExtendedAddress, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetPanId(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_nPanId);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_nPanId, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetPanId(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_nPanId)) {
    memcpy(&g_MacMibCommon.m_nPanId, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetPromiscuousMode(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = 1;
  pValue->m_au8Value[0] = (g_MacMibCommon.m_bPromiscuousMode) ? 1 : 0;
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetPromiscuousMode(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  uint8_t u8Value;
  memcpy(&u8Value, pValue->m_au8Value, sizeof(u8Value));
  if ((pValue->m_u8Length == sizeof(u8Value)) && (u8Value <= 1)) {
    g_MacMibCommon.m_bPromiscuousMode = u8Value != 0;
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetShortAddress(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_nShortAddress);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_nShortAddress, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetShortAddress(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_nShortAddress)) {
    memcpy(&g_MacMibCommon.m_nShortAddress, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetRcCoord(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_u16RcCoord);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_u16RcCoord, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetRcCoord(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_u16RcCoord)) {
    memcpy(&g_MacMibCommon.m_u16RcCoord, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibSetKeyTable(uint16_t u16Index, const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus;
  if (u16Index < MAC_KEY_TABLE_ENTRIES) {
    if (pValue->m_u8Length == MAC_SECURITY_KEY_LENGTH) {
      if (!g_MacMibCommon.m_aKeyTable[u16Index].m_bValid ||
        (memcmp(&g_MacMibCommon.m_aKeyTable[u16Index].m_au8Key, pValue->m_au8Value, MAC_SECURITY_KEY_LENGTH) != 0)) {
        // Set value if invalid entry or different key
        memcpy(&g_MacMibCommon.m_aKeyTable[u16Index].m_au8Key, pValue->m_au8Value, MAC_SECURITY_KEY_LENGTH);
        g_MacMibCommon.m_aKeyTable[u16Index].m_bValid = true;
      }
      eStatus = MAC_STATUS_SUCCESS;
    }
    else if (pValue->m_u8Length == 0) {
      g_MacMibCommon.m_aKeyTable[u16Index].m_bValid = false;
      eStatus = MAC_STATUS_SUCCESS;
    }
    else {
      eStatus = MAC_STATUS_INVALID_PARAMETER;
    }
  }
  else {
    eStatus = MAC_STATUS_INVALID_INDEX;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetPOSTableEntryTtl(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_u8POSTableEntryTtl);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_u8POSTableEntryTtl, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetPOSTableEntryTtl(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_u8POSTableEntryTtl)) {
    memcpy(&g_MacMibCommon.m_u8POSTableEntryTtl, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

static enum EMacStatus MacPibGetPOSRecentEntryThreshold(struct TMacPibValue *pValue)
{
  pValue->m_u8Length = sizeof(g_MacMibCommon.m_u8POSRecentEntryThreshold);
  memcpy(pValue->m_au8Value, &g_MacMibCommon.m_u8POSRecentEntryThreshold, pValue->m_u8Length);
  return MAC_STATUS_SUCCESS;
}

static enum EMacStatus MacPibSetPOSRecentEntryThreshold(const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus = MAC_STATUS_SUCCESS;
  if (pValue->m_u8Length == sizeof(g_MacMibCommon.m_u8POSRecentEntryThreshold)) {
    memcpy(&g_MacMibCommon.m_u8POSRecentEntryThreshold, pValue->m_au8Value, pValue->m_u8Length);
  }
  else {
    eStatus = MAC_STATUS_INVALID_PARAMETER;
  }
  return eStatus;
}

enum EMacStatus MacCommonGetRequestSync(enum EMacCommonPibAttribute eAttribute, uint16_t u16Index, struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus;
  bool bArray = (eAttribute == MAC_COMMON_PIB_KEY_TABLE);
  if (!bArray && (u16Index != 0)) {
    eStatus = MAC_STATUS_INVALID_INDEX;
  }
  else {
    switch (eAttribute) {
      case MAC_COMMON_PIB_PAN_ID:
        eStatus = MacPibGetPanId(pValue);
        break;
      case MAC_COMMON_PIB_PROMISCUOUS_MODE:
        eStatus = MacPibGetPromiscuousMode(pValue);
        break;
      case MAC_COMMON_PIB_SHORT_ADDRESS:
        eStatus = MacPibGetShortAddress(pValue);
        break;
      case MAC_COMMON_PIB_RC_COORD:
        eStatus = MacPibGetRcCoord(pValue);
        break;
      case MAC_COMMON_PIB_EXTENDED_ADDRESS:
        eStatus = MacPibGetExtendedAddress(pValue);
        break;
      case MAC_COMMON_PIB_POS_TABLE_ENTRY_TTL:
        eStatus = MacPibGetPOSTableEntryTtl(pValue);
        break;
      case MAC_COMMON_PIB_POS_RECENT_ENTRY_THRESHOLD:
        eStatus = MacPibGetPOSRecentEntryThreshold(pValue);
        break;
      case MAC_COMMON_PIB_KEY_TABLE:
        eStatus = MAC_STATUS_UNAVAILABLE_KEY;
        break;

      default:
        eStatus = MAC_STATUS_UNSUPPORTED_ATTRIBUTE;
        break;
    }
  }

  if (eStatus != MAC_STATUS_SUCCESS) {
    pValue->m_u8Length = 0;
  }
  return eStatus;
}

enum EMacStatus MacCommonSetRequestSync(enum EMacCommonPibAttribute eAttribute, uint16_t u16Index, const struct TMacPibValue *pValue)
{
  enum EMacStatus eStatus;
  bool bArray = (eAttribute == MAC_COMMON_PIB_KEY_TABLE);
  if (!bArray && (u16Index != 0)) {
    eStatus = MAC_STATUS_INVALID_INDEX;
  }
  else {
    switch (eAttribute) {
      case MAC_COMMON_PIB_PAN_ID:
        eStatus = MacPibSetPanId(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_PROMISCUOUS_MODE:
        eStatus = MacPibSetPromiscuousMode(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_SHORT_ADDRESS:
        eStatus = MacPibSetShortAddress(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_RC_COORD:
        eStatus = MacPibSetRcCoord(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_EXTENDED_ADDRESS:
        eStatus = MacPibSetExtendedAddress(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_POS_TABLE_ENTRY_TTL:
        eStatus = MacPibSetPOSTableEntryTtl(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_POS_RECENT_ENTRY_THRESHOLD:
        eStatus = MacPibSetPOSRecentEntryThreshold(pValue);
#ifdef __PLC_MAC__
        if (eStatus == MAC_STATUS_SUCCESS) {
          // Ignore result at MacRt level, as it depends on availability of PLC interface, which may be unavailable
          MacSetMacRtAttributeSync(eAttribute, u16Index, pValue);
        }
#endif
        break;
      case MAC_COMMON_PIB_KEY_TABLE:
        eStatus = MacPibSetKeyTable(u16Index, pValue);
        break;

      default:
        eStatus = MAC_STATUS_UNSUPPORTED_ATTRIBUTE;
        break;
    }
  }
  return eStatus;
}

uint32_t MacCommonGetMsCounter(void)
{
  return oss_get_up_time_ms();
}

