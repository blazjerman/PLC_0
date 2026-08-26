/**
 * @file dns_cache.h
 * @brief DNS cache management
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

#ifndef _DNS_CACHE_H
#define _DNS_CACHE_H

//Dependencies
#include "core/net.h"
#include "core/socket.h"

//DNS tick interval
#ifndef DNS_TICK_INTERVAL
   #define DNS_TICK_INTERVAL 200
#elif (DNS_TICK_INTERVAL < 10)
   #error DNS_TICK_INTERVAL parameter is not valid
#endif

//Size of DNS cache
#ifndef DNS_CACHE_SIZE
   #define DNS_CACHE_SIZE 8
#elif (DNS_CACHE_SIZE < 1)
   #error DNS_CACHE_SIZE parameter is not valid
#endif

//Maximum length of domain names
#ifndef DNS_MAX_NAME_LEN
   #define DNS_MAX_NAME_LEN 63
#elif (DNS_MAX_NAME_LEN < 1)
   #error DNS_MAX_NAME_LEN parameter is not valid
#endif

//Initial polling interval
#ifndef DNS_CACHE_INIT_POLLING_INTERVAL
   #define DNS_CACHE_INIT_POLLING_INTERVAL 10
#elif (DNS_CACHE_INIT_POLLING_INTERVAL < 1)
   #error DNS_CACHE_INIT_POLLING_INTERVAL parameter is not valid
#endif

//Maximum polling interval
#ifndef DNS_CACHE_MAX_POLLING_INTERVAL
   #define DNS_CACHE_MAX_POLLING_INTERVAL 1000
#elif (DNS_CACHE_MAX_POLLING_INTERVAL < 10)
   #error DNS_CACHE_MAX_POLLING_INTERVAL parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief DNS cache entry states
 **/

typedef enum
{
   DNS_STATE_NONE        = 0,
   DNS_STATE_IN_PROGRESS = 1,
   DNS_STATE_RESOLVED    = 2,
   DNS_STATE_PERMANENT   = 3
} DnsState;


/**
 * @brief DNS cache entry
 **/

typedef struct
{
   DnsState state;                    ///<Entry state
   HostType type;                     ///<IPv4 or IPv6 host?
   HostnameResolver protocol;         ///<Name resolution protocol
   NetInterface *interface;           ///<Underlying network interface
   uint_t dnsServerNum;               ///<This parameter selects between the primary and secondary DNS server
   uint16_t port;                     ///<Port number used by the resolver
   uint16_t id;                       ///<Identifier used to match queries and responses
   char_t name[DNS_MAX_NAME_LEN + 1]; ///<Domain name
   IpAddr ipAddr;                     ///<IP address
   systime_t timestamp;               ///<Time stamp to manage entry lifetime
   systime_t timeout;                 ///<Retransmission timeout
   systime_t maxTimeout;              ///<Maximum retransmission timeout
   uint_t retransmitCount;            ///<Retransmission counter
} DnsCacheEntry;


//Global variables
extern systime_t dnsTickCounter;
extern DnsCacheEntry dnsCache[DNS_CACHE_SIZE];

//DNS related functions
error_t dnsInit(void);

void dnsFlushCache(NetInterface *interface);

DnsCacheEntry *dnsCreateEntry(void);
void dnsDeleteEntry(DnsCacheEntry *entry);

DnsCacheEntry *dnsFindEntry(NetInterface *interface,
   const char_t *name, HostType type, HostnameResolver protocol);

void dnsTick(void);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
