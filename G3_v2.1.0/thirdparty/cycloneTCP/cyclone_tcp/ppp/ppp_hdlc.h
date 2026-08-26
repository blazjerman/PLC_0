/**
 * @file ppp_hdlc.h
 * @brief PPP HDLC driver
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

#ifndef _PPP_HDLC_H
#define _PPP_HDLC_H

//Dependencies
#include "core/nic.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//PPP HDLC driver
extern const NicDriver pppHdlcDriver;

//PPP HDLC driver related functions
error_t pppHdlcDriverInit(NetInterface *interface);

void pppHdlcDriverTick(NetInterface *interface);

void pppHdlcDriverEnableIrq(NetInterface *interface);
void pppHdlcDriverDisableIrq(NetInterface *interface);
void pppHdlcDriverEventHandler(NetInterface *interface);

error_t pppHdlcDriverSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset);

error_t pppHdlcDriverReceivePacket(NetInterface *interface);

error_t pppHdlcDriverUpdateMacAddrFilter(NetInterface *interface);

error_t pppHdlcDriverSendAtCommand(NetInterface *interface, const char_t *data);
error_t pppHdlcDriverReceiveAtCommand(NetInterface *interface, char_t *data, size_t size);

error_t pppHdlcDriverPurgeTxBuffer(PppContext *context);
error_t pppHdlcDriverPurgeRxBuffer(PppContext *context);

void pppHdlcDriverWriteTxQueue(PppContext *context, uint8_t c);
uint8_t pppHdlcDriverReadRxQueue(PppContext *context);

bool_t pppHdlcDriverReadTxQueue(NetInterface *interface, int_t *c);
bool_t pppHdlcDriverWriteRxQueue(NetInterface *interface, uint8_t c);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
