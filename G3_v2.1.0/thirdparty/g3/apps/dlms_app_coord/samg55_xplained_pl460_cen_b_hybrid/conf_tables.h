/**
 * \file
 *
 * \brief Configuration of G3 Table Sizes
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
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

#ifndef CONF_TABLES_H_INCLUDED
#define CONF_TABLES_H_INCLUDED

/* Define ADP Layer Buffer Count */
#define CONF_COUNT_ADP_BUFFERS_1280              1
#define CONF_COUNT_ADP_BUFFERS_400               6
#define CONF_COUNT_ADP_BUFFERS_100               6

/* Define ADP Layer Rx Fragmented Buffers Count */
#define CONF_ADP_FRAGMENTED_TRANSFER_TABLE_SIZE    3

/* Define ADP Layer Table Sizes */
#define CONF_ADP_ROUTING_TABLE_SIZE              400
#define CONF_ADP_ROUTING_SET_SIZE                75
#define CONF_LOADNG_RREP_GENERATION_TABLE_SIZE   10

/* Define IPv6 Buffer Count */
#define CONF_NET_MEM_POOL_BUFFER_COUNT           4

/* Define DLMS Snapshot Table Sizes */
#define CONF_MAX_NEIGHBOUR_DLMS_SNAPSHOT_SIZE    100
#define CONF_MAX_ROUTING_DLMS_SNAPSHOT_SIZE      400

/* Define Number of DLMS registered devices */
#define CONF_DLMS_MAX_DEV_NUM                    250

/* Define MAC RF Layer Table Sizes */
#define CONF_MAC_POS_TABLE_ENTRIES_RF            (100)
#define CONF_MAC_DSN_TABLE_ENTRIES_RF            (8)

#endif /* CONF_TABLES_H_INCLUDED */
