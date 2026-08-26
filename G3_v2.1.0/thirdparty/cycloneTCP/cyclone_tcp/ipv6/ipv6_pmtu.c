/**
 * @file ipv6_pmtu.c
 * @brief Path MTU Discovery for IPv6
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

//Switch to the appropriate trace level
#define TRACE_LEVEL IPV6_TRACE_LEVEL

//Dependencies
#include "core/net.h"
#include "core/ip.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_pmtu.h"
#include "ipv6/ndp.h"
#include "ipv6/ndp_cache.h"
#include "debug.h"

//Check TCP/IP stack configuration
#if (IPV6_SUPPORT == ENABLED && IPV6_PMTU_SUPPORT == ENABLED)


/**
 * @brief Retrieve the PMTU for the specified path
 * @param[in] interface Underlying network interface
 * @param[in] destAddr Destination IPv6 address
 * @return PMTU value
 **/

size_t ipv6GetPathMtu(NetInterface *interface, const Ipv6Addr *destAddr)
{
   size_t pathMtu;

#if (NDP_SUPPORT == ENABLED)
   NdpDestCacheEntry *entry;

   //Search the Destination Cache for the specified IPv6 address
   entry = ndpFindDestCacheEntry(interface, destAddr);

   //Check whether a matching entry has been found in the Destination Cache
   if(entry != NULL)
   {
      //Use the existing PMTU estimate
      pathMtu = entry->pathMtu;
   }
   else
   {
      //If no entry exists in the Destination Cache, the PMTU value for
      //the path is assumed to be the MTU of the first-hop link
      pathMtu = interface->ipv6Context.linkMtu;
   }
#else
   //The PMTU value for the path is assumed to be the MTU of the first-hop link
   pathMtu = interface->ipv6Context.linkMtu;
#endif

   //Return the PMTU value
   return pathMtu;
}


/**
 * @brief Update the PMTU for the specified path
 * @param[in] interface Underlying network interface
 * @param[in] destAddr Destination IPv6 address
 * @param[in] tentativePathMtu Tentative PMTU value
 **/

void ipv6UpdatePathMtu(NetInterface *interface,
   const Ipv6Addr *destAddr, size_t tentativePathMtu)
{
#if (NDP_SUPPORT == ENABLED)
   NdpDestCacheEntry *entry;

   //The destination address from the original packet is used to determine
   //which path the message applies to
   entry = ndpFindDestCacheEntry(interface, destAddr);

   //Check whether a matching entry has been found in the Destination Cache
   if(entry != NULL)
   {
      //Compare the tentative PMTU to the existing PMTU
      if(tentativePathMtu < entry->pathMtu)
      {
         //If the tentative PMTU is less than the existing PMTU estimate,
         //the tentative PMTU replaces the existing PMTU
         entry->pathMtu = tentativePathMtu;
      }
   }
#endif
}

#endif
