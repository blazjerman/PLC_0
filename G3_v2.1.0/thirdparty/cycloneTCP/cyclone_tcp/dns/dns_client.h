/**
 * @file dns_client.h
 * @brief DNS client (Domain Name System)
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

#ifndef _DNS_CLIENT_H
#define _DNS_CLIENT_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"
#include "core/udp.h"
#include "dns/dns_cache.h"

//DNS client support
#ifndef DNS_CLIENT_SUPPORT
   #define DNS_CLIENT_SUPPORT ENABLED
#elif (DNS_CLIENT_SUPPORT != ENABLED && DNS_CLIENT_SUPPORT != DISABLED)
   #error DNS_CLIENT_SUPPORT parameter is not valid
#endif

//Maximum number of retransmissions of DNS queries
#ifndef DNS_CLIENT_MAX_RETRIES
   #define DNS_CLIENT_MAX_RETRIES 3
#elif (DNS_CLIENT_MAX_RETRIES < 1)
   #error DNS_CLIENT_MAX_RETRIES parameter is not valid
#endif

//Initial retransmission timeout
#ifndef DNS_CLIENT_INIT_TIMEOUT
   #define DNS_CLIENT_INIT_TIMEOUT 1000
#elif (DNS_CLIENT_INIT_TIMEOUT < 1000)
   #error DNS_CLIENT_INIT_TIMEOUT parameter is not valid
#endif

//Maximum retransmission timeout
#ifndef DNS_CLIENT_MAX_TIMEOUT
   #define DNS_CLIENT_MAX_TIMEOUT 5000
#elif (DNS_CLIENT_MAX_TIMEOUT < 1000)
   #error DNS_CLIENT_MAX_TIMEOUT parameter is not valid
#endif

//Minimum cache lifetime for DNS entries
#ifndef DNS_MIN_LIFETIME
   #define DNS_MIN_LIFETIME 1000
#elif (DNS_MIN_LIFETIME < 0)
   #error DNS_MIN_LIFETIME parameter is not valid
#endif

//Maximum cache lifetime for DNS entries
#ifndef DNS_MAX_LIFETIME
   #define DNS_MAX_LIFETIME 3600000
#elif (DNS_MAX_LIFETIME < 1000 || DNS_MAX_LIFETIME < DNS_MIN_LIFETIME)
   #error DNS_MAX_LIFETIME parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//DNS related functions
error_t dnsResolve(NetInterface *interface, const char_t *name,
   HostType type, IpAddr *ipAddr);

error_t dnsSendQuery(DnsCacheEntry *entry);

void dnsProcessResponse(NetInterface *interface, const IpPseudoHeader *pseudoHeader,
   const UdpHeader *udpHeader, const NetBuffer *buffer, size_t offset, void *param);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
