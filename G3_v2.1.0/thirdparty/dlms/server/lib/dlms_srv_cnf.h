/**
 * \file
 *
 * \brief DLMS_SRV_LIB : DLMS server lib
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
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

#ifndef DLMS_SERVER_CNF_H_INCLUDED
#define DLMS_SERVER_CNF_H_INCLUDED

/*   Conformance ::= [APPLICATION 31] IMPLICIT BIT STRING (SIZE(24)) {
 * -- the bit is set when the corresponding service or functionality is available
 * 0   reserved (0)                        (0),
 * 0   reserved (0)                        (1),
 * 0   reserved (0)                        (2),
 * 0   read                                (3),
 * 0   write                               (4),
 * 0   unconfirmed-write                   (5),
 * 0   reserved (0)                        (6),
 * 0   reserved (0)                        (7),
 *
 * 0   attribute0-supported-with-SET       (8),
 * 0   priority-mgmt-supported             (9),
 * 0   attribute0-supported-with-GET       (10),
 * 1   block-transfer-with-get             (11),
 * 0   block-transfer-with-set             (12),
 * 0   block-transfer-with-action          (13),
 * 0   multiple-references                 (14),
 * 0   information-report                  (15),
 *
 * 0   reserved (0)                        (16),
 * 0   reserved (0)                        (17),
 * 0   parameterized-access                (18),
 * 1   get                                 (19),
 * 0   set                                 (20),
 * 1   selective-access                    (21),
 * 0   event-notification                  (22),
 * 0   action                              (23)
 */
#define DEVICE_CONFORMANCE              0x001014
#define DLMS_VERSION                    6
#define MAX_APDU_SIZE_SEND              0x00F7

#define DLMS_MAX_ASSOC                  4
#define DLMS_MAX_OBIS_NUMBER            60
#define LLS_PASSWORD_LEN                8

#endif /* DLMS_SERVER_CNF_H_INCLUDED */
