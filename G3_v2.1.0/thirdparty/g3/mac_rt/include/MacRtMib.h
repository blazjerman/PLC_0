/**
 *
 * \file
 *
 * \brief PLC MAC RT Layer Information Base file
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

#ifndef MAC_RT_MIB_H_
#define MAC_RT_MIB_H_

#include <MacRtDefs.h>

enum EMacRtPibAttribute {
  MAC_RT_PIB_MAX_BE = 0x00000047,
  MAC_RT_PIB_BSN = 0x00000049,
  MAC_RT_PIB_DSN = 0x0000004C,
  MAC_RT_PIB_MAX_CSMA_BACKOFFS = 0x0000004E,
  MAC_RT_PIB_MIN_BE = 0x0000004F,
  MAC_RT_PIB_PAN_ID = 0x00000050,
  MAC_RT_PIB_PROMISCUOUS_MODE = 0x00000051,
  MAC_RT_PIB_SHORT_ADDRESS = 0x00000053,
  MAC_RT_PIB_MAX_FRAME_RETRIES = 0x00000059,
  MAC_RT_PIB_DUPLICATE_DETECTION_TTL = 0x00000078,
  MAC_RT_PIB_HIGH_PRIORITY_WINDOW_SIZE = 0x00000100,
  MAC_RT_PIB_CSMA_NO_ACK_COUNT = 0x00000106,
  MAC_RT_PIB_BAD_CRC_COUNT = 0x00000109,
  MAC_RT_PIB_NEIGHBOUR_TABLE = 0x0000010A,
  MAC_RT_PIB_CSMA_FAIRNESS_LIMIT = 0x0000010C,
  MAC_RT_PIB_TMR_TTL = 0x0000010D,
  MAC_RT_PIB_POS_TABLE_ENTRY_TTL = 0x0000010E,
  MAC_RT_PIB_RC_COORD = 0x0000010F,
  MAC_RT_PIB_TONE_MASK = 0x00000110,
  MAC_RT_PIB_BEACON_RANDOMIZATION_WINDOW_LENGTH = 0x00000111,
  MAC_RT_PIB_A = 0x00000112,
  MAC_RT_PIB_K = 0x00000113,
  MAC_RT_PIB_MIN_CW_ATTEMPTS = 0x00000114,
  MAC_RT_PIB_CENELEC_LEGACY_MODE = 0x00000115,
  MAC_RT_PIB_FCC_LEGACY_MODE = 0x00000116,
  MAC_RT_PIB_BROADCAST_MAX_CW_ENABLE = 0x0000011E,
  MAC_RT_PIB_TRANSMIT_ATTEN = 0x0000011F,
  MAC_RT_PIB_POS_TABLE = 0x00000120,
  MAC_RT_PIB_POS_RECENT_ENTRY_THRESHOLD = 0x00000121,
  MAC_RT_PIB_POS_RECENT_ENTRIES = 0x00000122,
  MAC_RT_PIB_PREAMBLE_LENGTH = 0x00000124,
  // manufacturer specific
  // Extended address of this node.
  MAC_RT_PIB_MANUF_EXTENDED_ADDRESS = 0x08000001,
  // provides access to neighbour table by short address (transmitted as index)
  MAC_RT_PIB_MANUF_NEIGHBOUR_TABLE_ELEMENT = 0x08000002,
  // returns the maximum number of tones used by the band
  MAC_RT_PIB_MANUF_BAND_INFORMATION = 0x08000003,
  // Forces Modulation Scheme in every transmitted frame
  // 0 - Not forced, 1 - Force Differential, 2 - Force Coherent
  MAC_RT_PIB_MANUF_FORCED_MOD_SCHEME = 0x08000007,
  // Forces Modulation Type in every transmitted frame
  // 0 - Not forced, 1 - Force BPSK_ROBO, 2 - Force BPSK, 3 - Force QPSK, 4 - Force 8PSK
  MAC_RT_PIB_MANUF_FORCED_MOD_TYPE = 0x08000008,
  // Forces ToneMap in every transmitted frame
  // {0} - Not forced, other value will be used as tonemap
  MAC_RT_PIB_MANUF_FORCED_TONEMAP = 0x08000009,
  // Forces Modulation Scheme bit in Tone Map Response
  // 0 - Not forced, 1 - Force Differential, 2 - Force Coherent
  MAC_RT_PIB_MANUF_FORCED_MOD_SCHEME_ON_TMRESPONSE = 0x0800000A,
  // Forces Modulation Type bits in Tone Map Response
  // 0 - Not forced, 1 - Force BPSK_ROBO, 2 - Force BPSK, 3 - Force QPSK, 4 - Force 8PSK
  MAC_RT_PIB_MANUF_FORCED_MOD_TYPE_ON_TMRESPONSE = 0x0800000B,
  // Forces ToneMap field Tone Map Response
  // {0} - Not forced, other value will be used as tonemap field
  MAC_RT_PIB_MANUF_FORCED_TONEMAP_ON_TMRESPONSE = 0x0800000C,
  // Indicates whether an LBP frame for other destination has been received
  MAC_RT_PIB_MANUF_LBP_FRAME_RECEIVED = 0x0800000F,
  // Indicates whether an LBP frame for other destination has been received
  MAC_RT_PIB_MANUF_LNG_FRAME_RECEIVED = 0x08000010,
  // Gets number of valid elements in the Neighbour Table
  MAC_RT_PIB_MANUF_NEIGHBOUR_TABLE_COUNT = 0x08000012,
  // Gets number of discarded packets due to Other Destination
  MAC_RT_PIB_MANUF_RX_OTHER_DESTINATION_COUNT = 0x08000013,
  // Gets number of discarded packets due to MAC Repetition
  MAC_RT_PIB_MANUF_RX_MAC_REPETITION_COUNT = 0x08000015,
  // Gets number of discarded packets due to Segment Decode Error
  MAC_RT_PIB_MANUF_RX_SEGMENT_DECODE_ERROR_COUNT = 0x0800001C,
  // Enables MAC Sniffer
  MAC_RT_PIB_MANUF_ENABLE_MAC_SNIFFER = 0x0800001D,
  // Gets number of valid elements in the POS Table
  MAC_RT_PIB_MANUF_POS_TABLE_COUNT = 0x0800001E,
  // Gets or Sets number of retires left before forcing ROBO mode
  MAC_RT_PIB_MANUF_RETRIES_LEFT_TO_FORCE_ROBO = 0x0800001F,
  // Gets internal MAC RT version
  MAC_RT_PIB_MANUF_MAC_RT_INTERNAL_VERSION = 0x08000022,
  // Enable/Disable Sleep Mode
  MAC_RT_PIB_SLEEP_MODE = 0x08000024,
  // Set PLC Debug Mode
  MAC_RT_PIB_DEBUG_SET = 0x08000025,
  // Read PL360 Debug Information
  MAC_RT_PIB_DEBUG_READ = 0x08000026,
  // Provides access to POS table by short address (referenced as index)
  MAC_RT_PIB_MANUF_POS_TABLE_ELEMENT = 0x08000027,
  // Minimum LQI to consider a neighbour for Trickle
  MAC_RT_PIB_MANUF_TRICKLE_MIN_LQI = 0x08000028,
  /* LQI for a given neighbour, which short address will be indicated by index. 8 bits. */
  MAC_RT_PIB_MANUF_NEIGHBOUR_LQI = 0x08000029,
  /* Best LQI found in neighbour table. 8 bits. */
  MAC_RT_PIB_MANUF_BEST_LQI = 0x0800002A,
  /* Flag to indicate whether next transmission is in High Priority window. 8 bits. */
  MAC_RT_PIB_TX_HIGH_PRIORITY = 0x0800002B,
  /* Resets TMR TTL for the Short Address contained in Index. */
  MAC_RT_PIB_MANUF_RESET_TMR_TTL = 0x0800002E,
  // IB used to set the complete MIB structure at once
  MAC_RT_PIB_GET_SET_ALL_MIB = 0x08000100,
  // Gets or sets a parameter in Phy layer. Index will be used to contain PHY parameter ID.
  // Check 'enum EPhyParam' below for available Phy parameter IDs
  MAC_RT_PIB_MANUF_PHY_PARAM = 0x08000020
};

enum EPhyParam {
  // Phy layer version number. 32 bits.
  PHY_PARAM_VERSION = 0x010c,
  // Correctly transmitted frame count. 32 bits.
  PHY_PARAM_TX_TOTAL = 0x0110,
  // Transmitted bytes count. 32 bits.
  PHY_PARAM_TX_TOTAL_BYTES = 0x0114,
  // Transmission errors count. 32 bits.
  PHY_PARAM_TX_TOTAL_ERRORS = 0x0118,
  // Transmission failure due to already in transmission. 32 bits.
  PHY_PARAM_BAD_BUSY_TX = 0x011C,
  // Transmission failure due to busy channel. 32 bits.
  PHY_PARAM_TX_BAD_BUSY_CHANNEL = 0x0120,
  // Bad len in message (too short - too long). 32 bits.
  PHY_PARAM_TX_BAD_LEN = 0x0124,
  // Message to transmit in bad format. 32 bits.
  PHY_PARAM_TX_BAD_FORMAT = 0x0128,
  // Timeout error in transmission. 32 bits.
  PHY_PARAM_TX_TIMEOUT = 0x012C,
  // Received correctly messages count. 32 bits.
  PHY_PARAM_RX_TOTAL = 0x0130,
  // Received bytes count. 32 bits.
  PHY_PARAM_RX_TOTAL_BYTES = 0x0134,
  // Reception RS errors count. 32 bits.
  PHY_PARAM_RX_RS_ERRORS = 0x0138,
  // Reception Exceptions count. 32 bits.
  PHY_PARAM_RX_EXCEPTIONS = 0x013C,
  // Bad len in message (too short - too long). 32 bits.
  PHY_PARAM_RX_BAD_LEN = 0x0140,
  // Bad CRC in received FCH. 32 bits.
  PHY_PARAM_RX_BAD_CRC_FCH = 0x0144,
  // CRC correct but invalid protocol. 32 bits.
  PHY_PARAM_RX_FALSE_POSITIVE = 0x0148,
  // Received message in bad format. 32 bits.
  PHY_PARAM_RX_BAD_FORMAT = 0x014C,
  // Time between noise captures (in ms). 32 bits.
  PHY_PARAM_TIME_BETWEEN_NOISE_CAPTURES = 0x0158,
  // Auto detect impedance
  PHY_PARAM_CFG_AUTODETECT_BRANCH = 0x0161,
  // Manual impedance configuration
  PHY_PARAM_CFG_IMPEDANCE = 0x0162,
  // Indicate if notch filter is active or not. 8 bits.
  PHY_PARAM_RRC_NOTCH_ACTIVE = 0x0163,
  // Index of the notch filter. 8 bits.
  PHY_PARAM_RRC_NOTCH_INDEX = 0x0164,
  // Enable periodic noise autodetect and adaptation. 8 bits.
  PHY_PARAM_ENABLE_AUTO_NOISE_CAPTURE = 0x0166,
  // Noise detection timer reloaded after a correct reception. 8 bits.
  PHY_PARAM_DELAY_NOISE_CAPTURE_AFTER_RX = 0x0167,
  // Disable PLC Tx/Rx. 8 bits.
  PHY_PARAM_PLC_DISABLE = 0x016A,
  // Indicate noise power in dBuV for the noisier carrier
  PHY_PARAM_NOISE_PEAK_POWER = 0x016B,
  // LQI value of the last received message
  PHY_PARAM_LAST_MSG_LQI = 0x016C,
  // LQI value of the last received message
  PHY_PARAM_LAST_MSG_RSSI = 0x016D,
  // Success transmission of ACK packets
  PHY_PARAM_ACK_TX_CFM = 0x016E,
  // Duration in ms of the last received message
  PHY_PARAM_LAST_MSG_DURATION = 0x016F,
  // Inform PHY layer about enabled modulations on TMR
  PHY_PARAM_TONE_MAP_RSP_ENABLED_MODS = 0x0174,
  // Reset Phy Statistics
  PHY_PARAM_RESET_PHY_STATS = 0x0176,
  // Set number pf SyncP symbols in preamble
  PHY_PARAM_PREAMBLE_NUM_SYNCP = 0x0177
};

struct TMacRtMib {
  uint32_t m_u32CsmaNoAckCount;
  uint32_t m_u32BadCrcCount;
  uint32_t m_u32RxSegmentDecodeErrorCount;
  uint32_t m_u32RxMACRepetitionCount;
  uint32_t m_u32RxOtherDestinationCount;
  uint16_t m_nPanId;
  uint16_t m_nShortAddress;
  uint16_t m_u16POSRecentEntries;
  uint16_t m_u16RcCoord;
  struct TRtToneMask m_ToneMask;
  struct TExtAddress m_ExtendedAddress;
  struct TRtToneMap m_ForcedToneMap;
  struct TRtToneMap m_ForcedToneMapOnTMResponse;
  uint8_t m_u8HighPriorityWindowSize;
  uint8_t m_u8CsmaFairnessLimit;
  uint8_t m_u8A;
  uint8_t m_u8K;
  uint8_t m_u8MinCwAttempts;
  uint8_t m_u8MaxBe;
  uint8_t m_u8Bsn;
  uint8_t m_u8Dsn;
  uint8_t m_u8MaxCsmaBackoffs;
  uint8_t m_u8MaxFrameRetries;
  uint8_t m_u8MinBe;
  uint8_t m_u8ForcedModScheme;
  uint8_t m_u8ForcedModType;
  uint8_t m_u8ForcedModSchemeOnTMResponse;
  uint8_t m_u8ForcedModTypeOnTMResponse;
  uint8_t m_u8RetriesToForceRobo;
  uint8_t m_u8TransmitAtten;
  uint8_t m_u8POSTableEntryTtl;
  uint8_t m_u8POSRecentEntryThreshold;
  uint8_t m_u8TrickleMinLQI;
  uint8_t m_u8DuplicateDetectionTtl;
  uint8_t m_u8TmrTtl;
  uint8_t m_u8BeaconRandomizationWindowLength;
  uint8_t m_u8PreambleLength;
  bool m_bBroadcastMaxCwEnable;
  bool m_bCoordinator;
  bool m_bPromiscuousMode;
  bool m_bMacSniffer;
  bool m_bLBPFrameReceived;
  bool m_bLNGFrameReceived;
  bool m_bTxHighPriority;
};

#define MAC_RT_PIB_MAX_VALUE_LENGTH (144)

struct TMacRtPibValue {
  uint8_t m_u8Length;
  uint8_t m_au8Value[MAC_RT_PIB_MAX_VALUE_LENGTH];
};

enum EMacRtStatus MacRtGetRequestSync(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, struct TMacRtPibValue *pValue);
enum EMacRtStatus MacRtSetRequestSync(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, const struct TMacRtPibValue *pValue);

struct TMacRtNeighbourEntry * MacRtMibGetNeighbourEntry(uint16_t nShortAddress);
void ResetNeighbourTmrTtl(uint16_t nShortAddress);
void UpdateNeighbourTable(uint16_t nShortAddress, uint8_t u8PhaseDifferential, struct TRtToneMapResponse *pResponse);
struct TMacRtPOSEntry * MacRtMibGetPOSEntry(uint16_t nShortAddress);
void UpdatePOSTable(uint16_t nShortAddress, uint8_t u8LinkQuality);
void AdjustPOSTimers(uint32_t u32Elapsed);
void AdjustNeighbourTimers(uint32_t u32Elapsed);

void MacRtMibReset(void);
void MacRtMibInitialize(void);
void MacRtSetCoordinator(void);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
