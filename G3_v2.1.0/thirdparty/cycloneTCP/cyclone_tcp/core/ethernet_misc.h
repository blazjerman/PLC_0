/**
 * @file ethernet_misc.h
 * @brief Helper functions for Ethernet
 *
 * @section License
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Eval.
 *
 * This software is provided in source form for a short-term evaluation only. The
 * evaluation license expires 90 days after the date you first download the software.
 *
 * If you plan to use this software in a commercial product, you are required to
 * purchase a commercial license from Oryx Embedded SARL.
 *
 * After the 90-day evaluation period, you agree to either purchase a commercial
 * license or delete all copies of this software. If you wish to extend the
 * evaluation period, you must contact sales@oryx-embedded.com.
 *
 * This evaluation software is provided "as is" without warranty of any kind.
 * Technical support is available as an option during the evaluation period.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.4
 **/

#ifndef _ETHERNET_MISC_H
#define _ETHERNET_MISC_H

//Dependencies
#include "core/net.h"
#include "core/ethernet.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//Ethernet related constants
extern const uint8_t ethPadding[64];

//Ethernet related functions
error_t ethPadFrame(NetBuffer *buffer, size_t *length);

error_t ethEncodeVlanTag(NetBuffer *buffer, size_t *offset, uint16_t vlanId,
   uint16_t type);

error_t ethDecodeVlanTag(const uint8_t *frame, size_t length, uint16_t *vlanId,
   uint16_t *type);

error_t ethCheckDestAddr(NetInterface *interface, const MacAddr *macAddr);

void ethUpdateInStats(NetInterface *interface, const MacAddr *destMacAddr);

void ethUpdateOutStats(NetInterface *interface, const MacAddr *destMacAddr,
   size_t length);

void ethUpdateErrorStats(NetInterface *interface, error_t error);

uint32_t ethCalcCrc(const void *data, size_t length);
uint32_t ethCalcCrcEx(const NetBuffer *buffer, size_t offset, size_t length);

error_t ethCheckCrc(NetInterface *interface, const uint8_t *frame,
   size_t length);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
