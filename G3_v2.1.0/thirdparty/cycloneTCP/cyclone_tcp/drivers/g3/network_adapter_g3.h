/**
 * @file network_adapter_g3.h
 * @brief Wrapper layer between cycloneTCP and G3-PLC
 *
 * @section License
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 
#ifndef L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H
#define L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H

#include "core/net.h"
#include "AdpApi.h"

/* Exported G3 network adapter */
extern NicDriver g3_adapter;

void ipv6_receive_packet (struct TAdpDataIndication *pDataIndication);
error_t ipv6_send_packet (NetInterface* interface, const NetBuffer* buffer, size_t offset);
error_t Init (NetInterface* interface);

void 		Tick (NetInterface* interface);
void        RxEventHandler (NetInterface* interface);
void        EnableIrq (NetInterface* interface);
void        DisableIrq (NetInterface* interface);
error_t     SetMacFilter (NetInterface* interface);
void        WritePhyReg (uint8_t opcode, uint8_t  phyAddr, uint8_t  regAddr, uint16_t data);
uint16_t 	ReadPhyReg (uint8_t opcode, uint8_t phyAddr, uint8_t regAddr);

#endif // L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H
