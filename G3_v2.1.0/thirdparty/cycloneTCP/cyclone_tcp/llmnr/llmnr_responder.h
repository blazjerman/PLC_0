/**
 * @file llmnr_responder.h
 * @brief LLMNR responder (Link-Local Multicast Name Resolution)
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

#ifndef _LLMNR_RESPONDER_H
#define _LLMNR_RESPONDER_H

//Dependencies
#include "core/net.h"
#include "core/udp.h"
#include "dns/dns_common.h"
#include "llmnr/llmnr_common.h"

//LLMNR responder support
#ifndef LLMNR_RESPONDER_SUPPORT
   #define LLMNR_RESPONDER_SUPPORT DISABLED
#elif (LLMNR_RESPONDER_SUPPORT != ENABLED && LLMNR_RESPONDER_SUPPORT != DISABLED)
   #error LLMNR_RESPONDER_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//LLMNR related functions
error_t llmnrResponderInit(NetInterface *interface);

void llmnrProcessQuery(NetInterface *interface, const IpPseudoHeader *pseudoHeader,
   const UdpHeader *udpHeader, const NetBuffer *buffer, size_t offset, void *param);

error_t llmnrSendResponse(NetInterface *interface, const IpAddr *destIpAddr,
   uint16_t destPort, uint16_t id, uint16_t qtype, uint16_t qclass);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
