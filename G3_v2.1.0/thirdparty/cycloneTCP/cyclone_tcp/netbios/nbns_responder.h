/**
 * @file nbns_responder.h
 * @brief NBNS responder (NetBIOS Name Service)
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

#ifndef _NBNS_RESPONDER_H
#define _NBNS_RESPONDER_H

//Dependencies
#include "core/net.h"
#include "core/udp.h"
#include "dns/dns_common.h"
#include "netbios/nbns_common.h"

//NBNS responder support
#ifndef NBNS_RESPONDER_SUPPORT
   #define NBNS_RESPONDER_SUPPORT ENABLED
#elif (NBNS_RESPONDER_SUPPORT != ENABLED && NBNS_RESPONDER_SUPPORT != DISABLED)
   #error NBNS_RESPONDER_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//NBNS related functions
void nbnsProcessQuery(NetInterface *interface, const Ipv4PseudoHeader *pseudoHeader,
   const UdpHeader *udpHeader, const NbnsHeader *message, size_t length);

error_t nbnsSendResponse(NetInterface *interface,
   const IpAddr *destIpAddr, uint16_t destPort, uint16_t id);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
