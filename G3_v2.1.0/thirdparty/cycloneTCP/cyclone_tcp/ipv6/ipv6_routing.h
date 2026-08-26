/**
 * @file ipv6_routing.h
 * @brief IPv6 routing
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

#ifndef _IPV6_ROUTING_H
#define _IPV6_ROUTING_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//IPv6 routing support
#ifndef IPV6_ROUTING_SUPPORT
   #define IPV6_ROUTING_SUPPORT DISABLED
#elif (IPV6_ROUTING_SUPPORT != ENABLED && IPV6_ROUTING_SUPPORT != DISABLED)
   #error IPV6_ROUTING_SUPPORT parameter is not valid
#endif

//Size of the IPv6 routing table
#ifndef IPV6_ROUTING_TABLE_SIZE
   #define IPV6_ROUTING_TABLE_SIZE 8
#elif (IPV6_ROUTING_TABLE_SIZE < 1)
   #error IPV6_ROUTING_TABLE_SIZE parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Routing table entry
 **/

typedef struct
{
   bool_t valid;            ///<Valid entry
   Ipv6Addr prefix;         ///<Destination
   uint_t prefixLen;        ///<IPv6 prefix length
   NetInterface *interface; ///<Outgoing network interface
   Ipv6Addr nextHop;        ///<Next hop
   uint_t metric;           ///<Metric value
} Ipv6RoutingTableEntry;


//IPv6 routing related functions
error_t ipv6InitRouting(void);
error_t ipv6EnableRouting(NetInterface *interface, bool_t enable);

error_t ipv6AddRoute(const Ipv6Addr *prefix, uint_t prefixLen,
   NetInterface *interface, const Ipv6Addr *nextHop, uint_t metric);

error_t ipv6DeleteRoute(const Ipv6Addr *prefix, uint_t prefixLen);
error_t ipv6DeleteAllRoutes(void);

error_t ipv6ForwardPacket(NetInterface *srcInterface,
   NetBuffer *ipPacket, size_t ipPacketOffset);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
