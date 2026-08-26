/**
 * \file
 *
 * \brief Demo App Version Header file.
 *
 * Copyright (c) 2021 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef DEMO_APP_VERSION_H_INCLUDED
#define DEMO_APP_VERSION_H_INCLUDED

/** DEMO_APP_MAJOR_VERSION
  * Demo Application major version
  *
  * @note 99 is default value for development version (master branch)
  */
#define DEMO_APP_MAJOR_VERSION 2

/** DEMO_APP_MINOR_VERSION
  * Demo Application minor version
  *
  * @note 99 is default value for development version (master branch)
  */
#define DEMO_APP_MINOR_VERSION 0

/** DEMO_APP_PATCH_VERSION
  * Demo Application patch version
  *
  * @note 99 is default value for development version (master branch)
  */
#define DEMO_APP_PATCH_VERSION 0

/** DEMO_APP_VERSION
  * Demo Application version (MMmmpp - M(Major); m(minor); p(patch))
  *
  * @note 999999 is default value for development version (master branch)
  */
#define DEMO_APP_VERSION ((DEMO_APP_MAJOR_VERSION)*10000 + (DEMO_APP_MINOR_VERSION)*100 + (DEMO_APP_PATCH_VERSION))

#endif /* DEMO_APP_VERSION_H_INCLUDED */
