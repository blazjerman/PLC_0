/**
 * \file
 *
 * \brief RTC Process Handler Header
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _RTCPROC_H_INCLUDED
#define _RTCPROC_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#define RTC_KEY_MEM              0xA5A5
#define RTC_MIN_STORE_BACKUP     15

typedef struct {
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} rtc_time_t;

typedef struct {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
} rtc_date_t;

typedef struct {
	uint32_t key;
	rtc_time_t time;
	rtc_date_t date;
} rtc_t;

extern rtc_t VRTC;

void RTCProcInit(void);
uint8_t RTCProcSetTimeBCD(uint8_t bcd_hr, uint8_t bcd_min, uint8_t bcd_sec);
uint8_t RTCProcSetDateBCD(uint8_t bcd_year, uint8_t bcd_month, uint8_t bcd_day, uint8_t bcd_day_week);
uint8_t RTCProcGetDaysOfMonth(uint8_t year, uint8_t month);
uint8_t RTCProcSetNewRTC(rtc_t* new_rtc);
void RTCProcWriteInExtMem(void);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* _RTCPROC_H_INCLUDED */
