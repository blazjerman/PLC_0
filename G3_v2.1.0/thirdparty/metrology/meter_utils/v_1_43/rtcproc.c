/**
 * \file
 *
 * \brief RTC process
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

#include <string.h>
#include <stdio.h>

#include "rtcproc.h"
#include "asf.h"
/* ------------------------------------------------------------------- */
const uint8_t TB_MonthDayTable[12] = {0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31};
const uint8_t Month_WeekTable[12] = {0x00, 0x03, 0x03, 0x06, 0x01, 0x04, 0x06, 0x02, 0x05, 0x00, 0x03, 0x05};

rtc_str VRTC;
extern uint8_t wake_up_source;
/* ------------------------------------------------------------------- */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* //=================================================================== */
/* //decription    ::  rtc interrupt */
/* //function      ::  RTC_Handler */
/* //input         ::  none */
/* //output        ::  none */
/* //call          ::  none */
/* //effect        ::  VBaseTimer.T5ms */
/* //=================================================================== */
/* /// ** */
/* // * \brief Interrupt handler for the RTC. */
/* // * */
/* // * \retval SECOND_FG		Second marker for taking data from DSP. */
/* // * \retval MINUTE_FG		Minute marker for accumulating power measurements. */
/* // * \retval QTR_HOUR_FG	15 Minutes marker for accumulating power measurements. */
/* // * \retval TIME_CHANGE_FG Time change marker for accumulating power measurements. */
/* // * / */
/* void RTC_Handler( void ) */
/* { */
/*    static uint32_t rtc_wakeup = 0, ul_status = 0; */
/*  */
/*    ul_status = rtc_get_status( RTC ); */
/*  */
/*    / * Second increment interrupt * / */
/*    if ( ( ul_status & RTC_SR_SEC ) == RTC_SR_SEC ) */
/*    { */
/*        rtc_disable_interrupt( RTC, RTC_IDR_SECDIS ); */
/*        rtc_clear_status( RTC, RTC_SCCR_SECCLR ); */
/*        rtc_enable_interrupt( RTC, RTC_IER_SECEN ); */
/*    } */
/*    else if ( ( ul_status & RTC_SR_ALARM ) == RTC_SR_ALARM ) */
/*    { */
/*        rtc_disable_interrupt( RTC, RTC_IDR_ALRDIS ); */
/*  */
/*        rtc_get_time( RTC, &rtc_wakeup, &rtc_wakeup, &rtc_wakeup ); */
/*        rtc_wakeup = rtc_wakeup + 4; */
/*        if ( rtc_wakeup > 59 ) */
/*        { */
/*            rtc_wakeup = 0; */
/*        } */
/*        rtc_set_time_alarm( RTC, 0, 0, 0, 0, 1, rtc_wakeup ); */
/*  */
/*        rtc_clear_status( RTC, RTC_IER_ALREN ); */
/*        rtc_enable_interrupt( RTC, RTC_IER_ALREN ); */
/*  */
/*        if ( ioport_get_pin_level( GPIO_POWER_FAIL ) )          // Power supply detected */
/*        { */
/*            rstc_start_software_reset( RSTC );      // MCU reset */
/*        } */
/*        wake_up_source = 1; */
/*    } */
/*    else if ( ( ul_status & RTC_SR_TIMEV ) == RTC_SR_TIMEV ) */
/*    { */
/*        rtc_disable_interrupt( RTC, RTC_IDR_TIMDIS ); */
/*        rtc_clear_status( RTC, RTC_IDR_TIMDIS ); */
/*        rtc_enable_interrupt( RTC, RTC_IER_TIMEN ); */
/*    } */
/*  */
/*    //#if IPC_ENABLE */
/*    //#else */
/*    //      sys.events |= SECOND_FG; */
/*    //#endif */
/*    //		rtc_get_time(RTC, &rtc.time.hour, &rtc.time.minute, &rtc.time.second); */
/*    //    rtc_get_time( RTC, &ul_status, &ul_status, &rtc_wakeup ); */
/*    // */
/*    //		if ( !rtc.time.second) */
/*    //			sys.events |= MINUTE_FG; */
/*    // */
/*    / * Clear RTC second interrupt * / */
/*    //	} */
/*  */
/* } */
/* //========================================================= */
/* //Description     ::		bcd verify */
/* //Function        ::		BCD_Verify */
/* //Input           ::		bcddata */
/* //Output          ::		if failure return 1 else 0 */
/* //Call            ::		none */
/* //Effect          :: */
/* //========================================================= */
/* uint8_t	BCD_Verify( uint8_t BCDData ) */
/* { */
/*    if ( ( BCDData & 0x0f ) > 0x09 || ( BCDData & 0xf0 ) > 0x90 ) */
/*    { */
/*        return ( 1 ); */
/*    } */
/*    else */
/*    { */
/*        return ( 0 ); */
/*    } */
/* } */
/* //========================================================= */
/* //Description     ::		n byte bcd verify */
/* //Function        ::		N_Byte_BCDVerify */
/* //Input           ::		ptr->first bcd,len=N */
/* //Output          ::		if failure return 1 else 0 */
/* //Call            ::		none */
/* //Effect          :: */
/* //========================================================= */
/* uint8_t	N_Byte_BCDVerify( uint8_t *ptr, uint8_t len ) */
/* { */
/*    uint8_t i; */
/*    for ( i = 0; i < len; i++ ) */
/*    { */
/*        if ( BCD_Verify( *( ptr + i ) ) ) */
/*        { */
/*            break; */
/*        } */
/*    } */
/*    if ( i == len ) */
/*    { */
/*        return 0x00; */
/*    } */
/*    else */
/*    { */
/*        return 0x01; */
/*    } */
/* } */
/* //========================================================= */
/* //decription	::	rtc.time data verify */
/* //function		::	Time_FormatChk */
/* //Input			::	PP_Time->ss mm hh */
/* //Output		::	1->rtc.time data is failure , 0 is ok */
/* //call			::	N_Byte_BCDVerify */
/* //effect		:: */
/* //========================================================= */
/* uint8_t	Time_FormatChk( uint8_t *PP_Time ) */
/* { */
/*    if ( N_Byte_BCDVerify( PP_Time, 3 ) ) */
/*    { */
/*        return 0x01; */
/*    } */
/*    if ( *PP_Time > 0x59 ) */
/*    { */
/*        return 0x01; */
/*    } */
/*    if ( *( PP_Time + 1 ) > 0x59 ) */
/*    { */
/*        return 0x01; */
/*    } */
/*    if ( *( PP_Time + 2 ) > 0x23 ) */
/*    { */
/*        return 0x01; */
/*    } */
/*  */
/*    return 0x00; */
/* } */
/* //========================================================= */
/* //decription	::	rtc.date data verify */
/* //function		::	Date_FormatChk */
/* //Input			::	PP_Date->week day month year */
/* //Output		::	1->rtc.time data is failure , 0 is ok */
/* //call			::	N_Byte_BCDVerify,BCD_8BIN */
/* //effect		:: */
/* //========================================================= */
/* uint8_t	Date_FormatChk( uint8_t *PP_Date ) */
/* { */
/*    uint8_t	Year, Month, Day; */
/*    uint8_t	Temp1, Temp2; */
/*  */
/*    if ( N_Byte_BCDVerify( PP_Date, 4 ) ) */
/*    { */
/*        return 0x01; */
/*    } */
/*  */
/*    if ( *( PP_Date + 2 ) == 0 || *( PP_Date + 2 ) > 0x12 ) */
/*    { */
/*        return	0x01; */
/*    } */
/*  */
/*    Temp1 = TB_MonthDayTable[( BCD_8BIN( *( PP_Date + 2 ) ) - 1 )]; */
/*    if ( ( ( BCD_8BIN( *( PP_Date + 3 ) ) ) & 0x03 ) == 0 ) */
/*    { */
/*        if ( *( PP_Date + 2 ) == 0x02 ) */
/*        { */
/*            Temp1++;    //if leap year then februray is 29 days */
/*        } */
/*    } */
/*    if ( *( PP_Date + 1 ) == 0 || *( PP_Date + 1 ) > Temp1 ) */
/*    { */
/*        return ( 1 ); */
/*    } */
/*  */
/*    Year = BCD_8BIN( *( PP_Date + 3 ) ); */
/*    Month = BCD_8BIN( *( PP_Date + 2 ) ); */
/*    Day = BCD_8BIN( *( PP_Date + 1 ) ); */
/*  */
/*    Temp1	=	Year / 4; */
/*    Temp2	=	Year + Temp1;		//366 days one year. */
/*    Temp2	=	Temp2 % 0x07; */
/*    Temp2	=	Temp2 + Day + Month_WeekTable[Month - 1]; */
/*    if ( Year % 4 == 0 && Month < 3 ) */
/*    { */
/*        Temp2   -=	1; */
/*    } */
/*    Temp2 += 6;				//2000/1/1 is Sat. */
/*    *PP_Date = Temp2 % 7; */
/*  */
/*    return	( 0 ); */
/* } */
/* //========================================================= */
/* //decription	::	rtc data verify */
/* //function		::	RTCData_Verify */
/* //Input			::	ptr->ss mm hh week dd mm yy */
/* //Output		::	1->rtc data is failure , 0 is ok */
/* //call			::	Time_FormatChk,Date_FormatChk */
/* //effect		:: */
/* //========================================================= */
/* uint8_t	RTCData_Verify( uint8_t *ptr ) */
/* { */
/*    if ( Time_FormatChk( ptr ) || Date_FormatChk( ptr + 3 ) ) */
/*    { */
/*        return	( 1 ); */
/*    } */
/*    else */
/*    { */
/*        return ( 0 ); */
/*    } */
/* } */

/* =================================================================== */
/* decription    ::  init RTC process */
/* function      ::  rtc_init */
/* input         ::  none */
/* output        ::  none */
/* call          ::  rtc_set_time,rtc_set_date */
/* effect        ::  VRTC */
/* =================================================================== */
void    rtc_init(void)
{
	date_str rtcdata;
	rtc_time_t rtctime;
	rtc_str temprtc;

	temprtc = VRTC;
	rtctime.mode = 0;
	rtctime.hour = 8;
	rtctime.minute = 0;
	rtctime.second = 0;
	rtcdata.day = 12;
	rtcdata.week = 3;
	rtcdata.month = 10;
	rtcdata.year = 2016;
	rtc_set_time( RTC, rtctime.hour, rtctime.minute, rtctime.second );
	rtc_set_date( RTC, rtcdata.year, rtcdata.month, rtcdata.day, rtcdata.week );
}

/* =================================================================== */
/* decription    ::  read RTC process */
/* function      ::  ReadRTCProcess */
/* input         ::  none */
/* output        ::  none */
/* call          ::  rtc_get_time,rtc_get_date,PutTaskIntoQue */
/* effect        ::  VRTC */
/* =================================================================== */
void    ReadRTCProcess( void )
{
	date_str rtcdata;
	rtc_time_t rtctime;
	rtc_str temprtc;
	temprtc = VRTC;

	rtc_get_time( RTC, &rtctime.hour, &rtctime.minute, &rtctime.second );
	rtc_get_date( RTC, &rtcdata.year, &rtcdata.month, &rtcdata.day, &rtcdata.week );
	VRTC.second = (uint8_t)((rtctime.second / 10) << 4);
	VRTC.second |= (uint8_t)(rtctime.second % 10);
	VRTC.minute = (uint8_t)((rtctime.minute / 10) << 4);
	VRTC.minute |= (uint8_t)(rtctime.minute % 10);
	VRTC.hour = (uint8_t)((rtctime.hour / 10) << 4);
	VRTC.hour |= (uint8_t)(rtctime.hour % 10);
	VRTC.day = (uint8_t)((rtcdata.day / 10) << 4);
	VRTC.day |= (uint8_t)(rtcdata.day % 10);
	VRTC.month = (uint8_t)((rtcdata.month / 10) << 4);
	VRTC.month |= (uint8_t)(rtcdata.month % 10);
	VRTC.year = (uint8_t)(((rtcdata.year % 100) / 10) << 4);
	VRTC.year |= (uint8_t)(rtcdata.year % 10);
	VRTC.week = (uint8_t)(rtcdata.week % 10);
	/*    //----------------------------------------------------- */
	/*    if ( temprtc.minute != VRTC.minute ) */
	/*    { */
	/*        PutTaskIntoQue( TOUProc ); */
	/*    } */
	/*    if ( temprtc.hour != VRTC.hour ) */
	/*    { */
	/*        PutTaskIntoQue( HistoryDataSave ); */
	/*    } */
	/*    //----------------------------------------------------- */
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
