/**
 * @file ping.h
 * @brief Ping utility
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

#ifndef _PING_H
#define _PING_H

//Dependencies
#include "core/net.h"
#include "ipv4/icmp.h"
#include "ipv6/icmpv6.h"

//Ping utility support
#ifndef PING_SUPPORT
   #define PING_SUPPORT ENABLED
#elif (PING_SUPPORT != ENABLED && PING_SUPPORT != DISABLED)
   #error PING_SUPPORT parameter is not valid
#endif

//Default timeout value
#ifndef PING_DEFAULT_TIMEOUT
   #define PING_DEFAULT_TIMEOUT 1000
#elif (PING_DEFAULT_TIMEOUT < 0)
   #error PING_DEFAULT_TIMEOUT parameter is not valid
#endif

//Maximum size of the data payload
#ifndef PING_MAX_DATA_SIZE
   #define PING_MAX_DATA_SIZE 32
#elif (PING_MAX_DATA_SIZE < 0)
   #error PING_MAX_DATA_SIZE parameter is not valid
#endif

//Size of the internal buffer
#define PING_BUFFER_SIZE (sizeof(IcmpEchoMessage) + PING_MAX_DATA_SIZE)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Ping context
 **/

typedef struct
{
   NetInterface *interface;
   Socket *socket;
   size_t dataPayloadSize;
   uint16_t identifier;
   uint16_t sequenceNumber;
   systime_t timestamp;
   systime_t timeout;
   systime_t rtt;
   uint8_t buffer[PING_BUFFER_SIZE];
} PingContext;


//Ping related functions
error_t ping(NetInterface *interface, const IpAddr *targetIpAddr,
   size_t size, uint8_t ttl, systime_t timeout, systime_t *rtt);

void pingInit(PingContext *context);
error_t pingSetTimeout(PingContext *context, systime_t timeout);
error_t pingBindToInterface(PingContext *context, NetInterface *interface);

error_t pingSendRequest(PingContext *context,
   const IpAddr *targetIpAddr, size_t size, uint8_t ttl);

error_t pingWaitForReply(PingContext *context,
   IpAddr *targetIpAddr, systime_t *rtt);

void pingRelease(PingContext *context);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
