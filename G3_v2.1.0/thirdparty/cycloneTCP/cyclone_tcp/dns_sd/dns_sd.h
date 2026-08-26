/**
 * @file dns_sd.h
 * @brief DNS-SD (DNS-Based Service Discovery)
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

#ifndef _DNS_SD_H
#define _DNS_SD_H

//Dependencies
#include "core/net.h"
#include "dns/dns_common.h"
#include "mdns/mdns_common.h"

//DNS-SD support
#ifndef DNS_SD_SUPPORT
   #define DNS_SD_SUPPORT DISABLED
#elif (DNS_SD_SUPPORT != ENABLED && DNS_SD_SUPPORT != DISABLED)
   #error DNS_SD_SUPPORT parameter is not valid
#endif

//DNS-SD tick interval
#ifndef DNS_SD_TICK_INTERVAL
   #define DNS_SD_TICK_INTERVAL 250
#elif (DNS_SD_TICK_INTERVAL < 10)
   #error DNS_SD_TICK_INTERVAL parameter is not valid
#endif

//Maximum number of registered services
#ifndef DNS_SD_SERVICE_LIST_SIZE
   #define DNS_SD_SERVICE_LIST_SIZE 2
#elif (DNS_SD_SERVICE_LIST_SIZE < 1)
   #error DNS_SD_SERVICE_LIST_SIZE parameter is not valid
#endif

//Maximum length of service name
#ifndef DNS_SD_MAX_SERVICE_NAME_LEN
   #define DNS_SD_MAX_SERVICE_NAME_LEN 16
#elif (DNS_SD_MAX_SERVICE_NAME_LEN < 1)
   #error DNS_SD_MAX_SERVICE_NAME_LEN parameter is not valid
#endif

//Maximum length of instance name
#ifndef DNS_SD_MAX_INSTANCE_NAME_LEN
   #define DNS_SD_MAX_INSTANCE_NAME_LEN 32
#elif (DNS_SD_MAX_INSTANCE_NAME_LEN < 1)
   #error DNS_SD_MAX_INSTANCE_NAME_LEN parameter is not valid
#endif

//Maximum length of the discovery-time metadata (TXT record)
#ifndef DNS_SD_MAX_METADATA_LEN
   #define DNS_SD_MAX_METADATA_LEN 128
#elif (DNS_SD_MAX_METADATA_LEN < 1)
   #error DNS_SD_MAX_METADATA_LEN parameter is not valid
#endif

//Default resource record TTL (cache lifetime)
#ifndef DNS_SD_DEFAULT_RR_TTL
   #define DNS_SD_DEFAULT_RR_TTL 120
#elif (DNS_SD_DEFAULT_RR_TTL < 1)
   #error DNS_SD_DEFAULT_RR_TTL parameter is not valid
#endif

//Forward declaration of DnsSdContext structure
struct _DnsSdContext;
#define DnsSdContext struct _DnsSdContext

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief FSM state change callback
 **/

typedef void (*DnsSdStateChangeCallback)(DnsSdContext *context,
   NetInterface *interface, MdnsState state);


/**
 * @brief DNS-SD settings
 **/

typedef struct
{
   NetInterface *interface;                   ///<Underlying network interface
   uint_t numAnnouncements;                   ///<Number of announcement packets
   uint32_t ttl;                              ///<TTL resource record
   DnsSdStateChangeCallback stateChangeEvent; ///<FSM state change event
} DnsSdSettings;


/**
 * @brief DNS-SD service descriptor
 **/

typedef struct
{
   char_t name[DNS_SD_MAX_SERVICE_NAME_LEN + 1]; ///<Service name
   uint16_t priority;                            ///<Priority of the target host
   uint16_t weight;                              ///<Server selection mechanism
   uint16_t port;                                ///<Port on the target host of this service
   uint8_t metadata[DNS_SD_MAX_METADATA_LEN];    ///<Discovery-time metadata (TXT record)
   size_t metadataLength;                        ///<Length of the metadata
} DnsSdService;


/**
 * @brief DNS-SD context
 **/

struct _DnsSdContext
{
   DnsSdSettings settings;                                ///<DNS-SD settings
   bool_t running;                                        ///<DNS-SD is currently running
   MdnsState state;                                       ///<FSM state
   bool_t conflict;                                       ///<Conflict detected
   bool_t tieBreakLost;                                   ///<Tie-break lost
   systime_t timestamp;                                   ///<Timestamp to manage retransmissions
   systime_t timeout;                                     ///<Timeout value
   uint_t retransmitCount;                                ///<Retransmission counter
   char_t instanceName[DNS_SD_MAX_INSTANCE_NAME_LEN + 1]; ///<Service instance name
   DnsSdService serviceList[DNS_SD_SERVICE_LIST_SIZE];    ///<List of registered services
};


//Tick counter to handle periodic operations
extern systime_t dnsSdTickCounter;

//DNS-SD related functions
void dnsSdGetDefaultSettings(DnsSdSettings *settings);
error_t dnsSdInit(DnsSdContext *context, const DnsSdSettings *settings);
error_t dnsSdStart(DnsSdContext *context);
error_t dnsSdStop(DnsSdContext *context);
MdnsState dnsSdGetState(DnsSdContext *context);

error_t dnsSdSetInstanceName(DnsSdContext *context, const char_t *instanceName);

error_t dnsSdRegisterService(DnsSdContext *context, const char_t *serviceName,
   uint16_t priority, uint16_t weight, uint16_t port, const char_t *metadata);

error_t dnsSdUnregisterService(DnsSdContext *context, const char_t *serviceName);

uint_t dnsSdGetNumServices(DnsSdContext *context);
error_t dnsSdStartProbing(DnsSdContext *context);

void dnsSdTick(DnsSdContext *interface);
void dnsSdLinkChangeEvent(DnsSdContext *interface);

void dnsSdChangeState(DnsSdContext *context,
   MdnsState newState, systime_t delay);

void dnsSdChangeInstanceName(DnsSdContext *context);

error_t dnsSdSendProbe(DnsSdContext *context);
error_t dnsSdSendAnnouncement(DnsSdContext *context);
error_t dnsSdSendGoodbye(DnsSdContext *context, const DnsSdService *service);

error_t dnsSdParseQuestion(NetInterface *interface, const MdnsMessage *query,
   size_t offset, const DnsQuestion *question, MdnsMessage *response);

void dnsSdParseNsRecord(NetInterface *interface, const MdnsMessage *query,
   size_t offset, const DnsResourceRecord *record);

void dnsSdParseAnRecord(NetInterface *interface, const MdnsMessage *response,
   size_t offset, const DnsResourceRecord *record);

void dnsSdGenerateAdditionalRecords(NetInterface *interface,
   MdnsMessage *response, bool_t legacyUnicast);

error_t dnsSdAddServiceEnumPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdService *service, uint32_t ttl);

error_t dnsSdAddPtrRecord(NetInterface *interface,
   MdnsMessage *message, const DnsSdService *service, uint32_t ttl);

error_t dnsSdAddSrvRecord(NetInterface *interface, MdnsMessage *message,
   const DnsSdService *service, bool_t cacheFlush, uint32_t ttl);

error_t dnsSdAddTxtRecord(NetInterface *interface, MdnsMessage *message,
   const DnsSdService *service, bool_t cacheFlush, uint32_t ttl);

error_t dnsSdAddNsecRecord(NetInterface *interface, MdnsMessage *message,
   const DnsSdService *service, bool_t cacheFlush, uint32_t ttl);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
