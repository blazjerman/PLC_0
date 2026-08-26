/**
 * @file ppp_misc.h
 * @brief PPP miscellaneous functions
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

#ifndef _PPP_MISC_H
#define _PPP_MISC_H

//Dependencies
#include "core/net.h"
#include "ppp/ppp.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//PPP related functions
error_t pppSendConfigureAckNak(PppContext *context,
   const PppConfigurePacket *configureReqPacket, PppProtocol protocol, PppCode code);

error_t pppSendTerminateReq(PppContext *context,
   uint8_t identifier, PppProtocol protocol);

error_t pppSendTerminateAck(PppContext *context,
   uint8_t identifier, PppProtocol protocol);

error_t pppSendCodeRej(PppContext *context, const PppPacket *packet,
   uint8_t identifier, PppProtocol protocol);

error_t pppSendProtocolRej(PppContext *context, uint8_t identifier,
   uint16_t protocol, const uint8_t *information, size_t length);

error_t pppSendEchoRep(PppContext *context,
   const PppEchoPacket *echoReqPacket, PppProtocol protocol);

error_t pppAddOption(PppConfigurePacket *packet, uint8_t optionType,
   const void *optionValue, uint8_t optionLen);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
