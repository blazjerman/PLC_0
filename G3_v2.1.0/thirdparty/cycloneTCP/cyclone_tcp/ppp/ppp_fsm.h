/**
 * @file ppp_fsm.h
 * @brief PPP finite state machine
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

#ifndef _PPP_FSM_H
#define _PPP_FSM_H

//Dependencies
#include "core/net.h"
#include "ppp/ppp.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief This-Layer-Up callback function
 **/

typedef void (*PppThisLayerUp)(PppContext *context);


/**
 * @brief This-Layer-Down callback function
 **/

typedef void (*PppThisLayerDown)(PppContext *context);


/**
 * @brief This-Layer-Started callback function
 **/

typedef void (*PppThisLayerStarted)(PppContext *context);


/**
 * @brief This-Layer-Finished callback function
 **/

typedef void (*PppThisLayerFinished)(PppContext *context);


/**
 * @brief Initialize-Restart-Count callback function
 **/

typedef void (*PppInitRestartCount)(PppContext *context, uint_t value);


/**
 * @brief Zero-Restart-Count callback function
 **/

typedef void (*PppZeroRestartCount)(PppContext *context);


/**
 * @brief Send-Configure-Request callback function
 **/

typedef error_t (*PppSendConfigureReq)(PppContext *context);


/**
 * @brief Send-Configure-Ack callback function
 **/

typedef error_t (*PppSendConfigureAck)(PppContext *context,
   const PppConfigurePacket *configureReqPacket);


/**
 * @brief Send-Configure-Nak callback function
 **/

typedef error_t (*PppSendConfigureNak)(PppContext *context,
   const PppConfigurePacket *configureReqPacket);


/**
 * @brief Send-Configure-Reject callback function
 **/

typedef error_t (*PppSendConfigureRej)(PppContext *context,
   const PppConfigurePacket *configureReqPacket);


/**
 * @brief Send-Terminate-Request callback function
 **/

typedef error_t (*PppSendTerminateReq)(PppContext *context);


/**
 * @brief Send-Terminate-Ack callback function
 **/

typedef error_t (*PppSendTerminateAck)(PppContext *context,
   const PppTerminatePacket *terminateReqPacket);


/**
 * @brief Send-Code-Reject callback function
 **/

typedef error_t (*PppSendCodeRej)(PppContext *context,
   const PppPacket *packet);


/**
 * @brief Send-Echo-Reply callback function
 **/

typedef error_t (*PppSendEchoRep)(PppContext *context,
   const PppEchoPacket *echoReqPacket);


/**
 *@brief PPP FSM actions
 **/

typedef struct
{
   PppThisLayerUp thisLayerUp;
   PppThisLayerDown thisLayerDown;
   PppThisLayerStarted thisLayerStarted;
   PppThisLayerFinished thisLayerFinished;
   PppInitRestartCount initRestartCount;
   PppZeroRestartCount zeroRestartCount;
   PppSendConfigureReq sendConfigureReq;
   PppSendConfigureAck sendConfigureAck;
   PppSendConfigureNak sendConfigureNak;
   PppSendConfigureRej sendConfigureRej;
   PppSendTerminateReq sendTerminateReq;
   PppSendTerminateAck sendTerminateAck;
   PppSendCodeRej sendCodeRej;
   PppSendEchoRep sendEchoRep;
} PppCallbacks;


//PPP FSM events
void pppUpEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppDownEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppOpenEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppCloseEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppTimeoutEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppRcvConfigureReqEvent(PppContext *context, PppFsm *fsm, const PppCallbacks *callbacks,
   const PppConfigurePacket *configureReqPacket, PppCode code);

void pppRcvConfigureAckEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppRcvConfigureNakEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppRcvTerminateReqEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks, const PppTerminatePacket *terminateReqPacket);

void pppRcvTerminateAckEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks);

void pppRcvUnknownCodeEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks, const PppPacket *packet);

void pppRcvCodeRejEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks, bool_t acceptable);

void pppRcvEchoReqEvent(PppContext *context, PppFsm *fsm,
   const PppCallbacks *callbacks, const PppEchoPacket *echoReqPacket);

void pppChangeState(PppFsm *fsm, PppState newState);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
