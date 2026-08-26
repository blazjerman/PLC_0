/**
 * @file ipcp.h
 * @brief IPCP (PPP Internet Protocol Control Protocol)
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

#ifndef _IPCP_H
#define _IPCP_H

//Dependencies
#include "core/net.h"
#include "ppp/ppp.h"

//Subnet mask
#define IPCP_DEFAULT_SUBNET_MASK IPV4_ADDR(255, 255, 255, 255)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief IPCP option types
 **/

typedef enum
{
   IPCP_OPTION_IP_ADDRESSES     = 1,   ///<IP-Addresses
   IPCP_OPTION_IP_COMP_PROTOCOL = 2,   ///<IP-Compression-Protocol
   IPCP_OPTION_IP_ADDRESS       = 3,   ///<IP-Address
   IPCP_OPTION_PRIMARY_DNS      = 129, ///<Primary-DNS-Server-Address
   IPCP_OPTION_PRIMARY_NBNS     = 130, ///<Primary-NBNS-Server-Address
   IPCP_OPTION_SECONDARY_DNS    = 131, ///<Secondary-DNS-Server-Address
   IPCP_OPTION_SECONDARY_NBNS   = 132  ///<Secondary-NBNS-Server-Address
} IpcpOptionType;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief IP-Addresses option
 **/

typedef __start_packed struct
{
   uint8_t type;        //0
   uint8_t length;      //1
   Ipv4Addr srcIpAddr;  //2-5
   Ipv4Addr destIpAddr; //6-9
} __end_packed IpcpIpAddressesOption;


/**
 * @brief IP-Compression-Protocol option
 **/

typedef __start_packed struct
{
   uint8_t type;      //0
   uint8_t length;    //1
   uint16_t protocol; //2-3
   uint8_t data[];    //4
} __end_packed IpcpIpCompProtocolOption;


/**
 * @brief IP-Address option
 **/

typedef __start_packed struct
{
   uint8_t type;    //0
   uint8_t length;  //1
   Ipv4Addr ipAddr; //2-5
} __end_packed IpcpIpAddressOption;


/**
 * @brief Primary-DNS-Server-Address option
 **/

typedef __start_packed struct
{
   uint8_t type;    //0
   uint8_t length;  //1
   Ipv4Addr ipAddr; //2-5
} __end_packed IpcpPrimaryDnsOption;


/**
 * @brief Primary-NBNS-Server-Address option
 **/

typedef __start_packed struct
{
   uint8_t type;    //0
   uint8_t length;  //1
   Ipv4Addr ipAddr; //2-5
} __end_packed IpcpPrimaryNbnsOption;


/**
 * @brief Secondary-DNS-Server-Address option
 **/

typedef __start_packed struct
{
   uint8_t type;    //0
   uint8_t length;  //1
   Ipv4Addr ipAddr; //2-5
} __end_packed IpcpSecondaryDnsOption;


/**
 * @brief Secondary-NBNS-Server-Address option
 **/

typedef __start_packed struct
{
   uint8_t type;    //0
   uint8_t length;  //1
   Ipv4Addr ipAddr; //2-5
} __end_packed IpcpSecondaryNbnsOption;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


//IPCP FSM events
error_t ipcpOpen(PppContext *context);
error_t ipcpClose(PppContext *context);

void ipcpTick(PppContext *context);

void ipcpProcessPacket(PppContext *context, const PppPacket *packet, size_t length);

error_t ipcpProcessConfigureReq(PppContext *context,
   const PppConfigurePacket *configureReqPacket);

error_t ipcpProcessConfigureAck(PppContext *context,
   const PppConfigurePacket *configureAckPacket);

error_t ipcpProcessConfigureNak(PppContext *context,
   const PppConfigurePacket *configureNakPacket);

error_t ipcpProcessConfigureReject(PppContext *context,
   const PppConfigurePacket *configureRejPacket);

error_t ipcpProcessTerminateReq(PppContext *context,
   const PppTerminatePacket *terminateReqPacket);

error_t ipcpProcessTerminateAck(PppContext *context,
   const PppTerminatePacket *terminateAckPacket);

error_t ipcpProcessCodeRej(PppContext *context,
   const PppCodeRejPacket *codeRejPacket);

error_t ipcpProcessUnknownCode(PppContext *context,
   const PppPacket *packet);

//IPCP FSM callback functions
void ipcpThisLayerUp(PppContext *context);
void ipcpThisLayerDown(PppContext *context);
void ipcpThisLayerStarted(PppContext *context);
void ipcpThisLayerFinished(PppContext *context);

void ipcpInitRestartCount(PppContext *context, uint_t value);
void ipcpZeroRestartCount(PppContext *context);

error_t ipcpSendConfigureReq(PppContext *context);

error_t ipcpSendConfigureAck(PppContext *context,
   const PppConfigurePacket *configureReqPacket);

error_t ipcpSendConfigureNak(PppContext *context,
   const PppConfigurePacket *configureReqPacket);

error_t ipcpSendConfigureRej(PppContext *context,
   const PppConfigurePacket *configureReqPacket);

error_t ipcpSendTerminateReq(PppContext *context);

error_t ipcpSendTerminateAck(PppContext *context,
   const PppTerminatePacket *terminateReqPacket);

error_t ipcpSendCodeRej(PppContext *context, const PppPacket *packet);

//IPCP options checking
error_t ipcpParseOption(PppContext *context, PppOption *option,
   size_t inPacketLen, PppConfigurePacket *outPacket);

error_t ipcpParseIpAddressOption(PppContext *context,
   IpcpIpAddressOption *option, PppConfigurePacket *outPacket);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
