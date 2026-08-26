/**
 * \file
 *
 * \brief Meter Demo Application including G3 PLC connectivity
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

/**
 * \mainpage Meter Demo Application (G3 PLC)
 *
 * \section Purpose
 *
 * The Meter Demo application is a simple interrupt driven firmware with event flags (SECOND_FG,
 * USER_CMD_FG, …) to schedule different processing such as second processing, terminal user
 * command processing, … The Meter Demo application runs on top of the ASF (device driver,
 * services and components). So, the low level firmware driver (ASF) handles the hardware setup
 * and controls for both Metrology firmware and Meter firmware.
 *
 * \section Requirements
 *
 * This package must be used with Metrology boards. (PIC32CX)
 *
 * \section Description
 *
 * The Meter firmware exchange control and data with Metrology firmware via the DSP buffers.
 * On the other hand, it also communicates to the user via Terminal interface.
 * It also includes PLC connectivity through G3 standard protocol.
 *
 * \section Usage
 *
 * -# Build the program and download it inside the evaluation board. Please
 *    refer to the
 *    <a href="http://www.microchip.com/dyn/resources/prod_documents/doc6224.pdf">
 *    SAM-BA User Guide</a>, the
 *    <a href="http://ww1.microchip.com/downloads/en/appnotes/doc6310.pdf">
 *    GNU-Based Software Development</a>
 *    application note or to the
 *    <a href="ftp://ftp.iar.se/WWWfiles/arm/Guides/EWARM_UserGuide.ENU.pdf">
 *    IAR EWARM User Guide</a>,
 *    depending on your chosen solution.
 * -# On the computer, open and configure a terminal application
 *    (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *   - 115200 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 * -# Start the application.
 * -# LED should start blinking on the board. In the terminal window, the
 *    following text should appear (values depend on the board and chip used):
 *    \code
 *      -- Meter Demo Application xxx --
 *      -- xxxxxx-xx
 *      -- Compiled: xxx xx xxxx xx:xx:xx --
 * \endcode
 *
 */

#include "asf.h"
#include "conf_board.h"
#include "conf_command_serial.h"

#include "basetime.h"
#include "command.h"
#include "demand.h"
#include "display.h"
#include "energy.h"
#include "event.h"
#include "extmem.h"
#include "history.h"
#include "task.h"
#include "metrology.h"
#include "presskey.h"
#include "rtcproc.h"
#include "coproc.h"
#include "tou.h"
#include "utils.h"
#include "main.h"
#include "app_demo_meter.h"
#include "demo_app_version.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#define STRING_HEADER   "\r\n-- Microchip Demo Meter (G3 PLC) - "BOARD_NAME " --\r\n" \
	"-- Compiled: "__DATE__ " "__TIME__ " -- \r\n"

/** All interrupt mask. */
#define ALL_INTERRUPT_MASK  0xffffffff

/* System variable structure */
sys_t Vsys;
uint8_t wake_up_source = 0;

#define APP_GPBR_INIT_FLAG           0xA55A

/* Tampering Last stamping */
static rtc_time_t tamper_time;
static rtc_date_t tamper_date;

#ifdef ENABLE_CHECK_METUPD_TIMER
/**
 * \brief Metrology Update Timer callback.
 */
static void _metrology_update_timer_expires(void)
{
	/* Restart Core 1 and Metrology module */
	__disable_irq();

	/* Init Core 1 */
	CoprocInit(ICM_MONITORING, RESET_TYPE_HARD);

	/* Init Metrology */
	if (!MetrologyInit()) {
		rstc_start_software_reset(RSTC);
	}

	__enable_irq();
}
#endif

/**
 * \brief Init additional debug PIOs.
 */
static void _initialize_debug_pios(void )
{
#if (defined(USE_MIKROBUS_PIN_AN_AS_OUTPUT) || defined(USE_MIKROBUS_PIN_AN_FOR_END_OF_INTEGRATION_TOGGLE))
	/* Configure MIKROBUS_PIN_AN as a general DEBUG output pin for Core0 or Core1 use */
	ioport_set_pin_dir(MIKROBUS_PIN_AN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(MIKROBUS_PIN_AN, IOPORT_PIN_LEVEL_LOW);
#endif

#ifdef USE_MIKROBUS_PIN_PWM_AS_OUTPUT
	/* Configure MIKROBUS_PIN_PWM as a general DEBUG output pin for Core0 or Core1 use */
	ioport_set_pin_dir(MIKROBUS_PIN_PWM, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(MIKROBUS_PIN_PWM, IOPORT_PIN_LEVEL_LOW);
#endif

#if (defined(USE_MIKROBUS_PIN_RST_AS_OUTPUT) || defined(USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE) || defined(USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT))
	/* Configure MIKROBUS_PIN_RST as output for Raw Zero-Crossings or debug purposes */
	ioport_set_pin_dir(MIKROBUS_PIN_RST, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(MIKROBUS_PIN_RST, IOPORT_PIN_LEVEL_LOW);
#endif
}

/**
 * \brief Configure the command console UART.
 */
static void configure_command_usart(void)
{
	usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_COMMAND_UART_BAUDRATE,
		.charlength = CONF_COMMAND_UART_CHAR_LENGTH,
		.paritytype = CONF_COMMAND_UART_PARITY,
		.stopbits = CONF_COMMAND_UART_STOP_BITS,
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONF_COMMAND_UART_ID);
	usart_serial_init((Usart *)CONF_COMMAND_UART, &uart_serial_options);
}

/**
 * \brief Configure the OPTO UART.
 */
static void configure_opto_usart(void)
{
	usart_serial_options_t opto_serial_options = {
		.baudrate = CONF_OPTO_UART_BAUDRATE,
		.paritytype = CONF_OPTO_UART_PARITY,
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONF_OPTO_UART_ID);
	usart_serial_init((Usart *)CONF_OPTO_UART, &opto_serial_options);
}

/**
 * \brief Configure TAMPER detection and FWUP wake up pin
 */
static void configure_tamper_fwup(void)
{
	uint32_t ul_wkup_mode;

	/* Configure RTCOUT0 waveform as 32 Hz */
	rtc_set_waveform(RTC, 0, RTCOUT_FREQ_32HZ);

	/* Configure RTC for inmediate clear of partial GPBR0-11 */
	rtc_enable_tamper_input(RTC, RTC_TCR_TAMPEN0);
	rtc_enable_tamper_clear(RTC, 0);

	/* Configure WKUP0 as TAMPER detection */
	ul_wkup_mode = SUPC_WUMR_LPDBCEN0 | SUPC_WUMR_LPDBC0_2_RTCOUT;
	/* Set debouncer for FWUP */
	ul_wkup_mode |= SUPC_WUMR_FWUPDBC_32_SK;
	supc_set_wakeup_mode(SUPC, ul_wkup_mode);

	/* Set TAMPER (WKUP0) as wake up input */
	supc_set_wakeup_inputs(SUPC, SUPC_WUIR_WKUPEN0, SUPC_WUIR_WKUPT0_LOW);
}

/**
 * \brief Reset mode.
 */
void demo_meter_app_reset(void)
{
	slcdc_disable(SLCDC);
	rstc_start_software_reset(RSTC);
}

/**
 * \brief Initialization of Metering Demo Application.
 */
void demo_meter_app_init(void)
{
	/* Show demo information through Console port. Console port has been init previously (OSS) */
	LOG_APP_DEMO_DEBUG((STRING_HEADER));
	LOG_APP_DEMO_DEBUG(("-- App Demo Ver. %d\r\n", DEMO_APP_VERSION));

	/* Configure serial command console */
	configure_command_usart();

	/* Configure OPTO uart */
	configure_opto_usart();

	/* Configure TAMPER detection and FWUP wake up event */
	configure_tamper_fwup();

	/* Task Initialize queue */
	TaskInit();

	/* Init LCD */
	DisplayInit();
	/* Configure display time loop */
	DisplaySetTimerLoop(2);
	/* Configure display measurements */
	DisplayAddLoopInfo(DISPLAY_BOARD_ID);
	DisplayAddLoopInfo(DISPLAY_VERSION);
	DisplayAddLoopInfo(DISPLAY_TOTAL_ENERGY);
	DisplayAddLoopInfo(DISPLAY_TOU1_ENERGY);
	DisplayAddLoopInfo(DISPLAY_TOU2_ENERGY);
	DisplayAddLoopInfo(DISPLAY_TOU3_ENERGY);
	DisplayAddLoopInfo(DISPLAY_TOU4_ENERGY);
	DisplayAddLoopInfo(DISPLAY_RTC_TIME);
	DisplayAddLoopInfo(DISPLAY_RTC_DATE);
	DisplayAddLoopInfo(DISPLAY_VA_RMS);
	DisplayAddLoopInfo(DISPLAY_VB_RMS);
#if BOARD==PIC32CXMTC_DB
	DisplayAddLoopInfo(DISPLAY_VC_RMS);
#endif
	DisplayAddLoopInfo(DISPLAY_IA_RMS);
	DisplayAddLoopInfo(DISPLAY_IB_RMS);
#if BOARD==PIC32CXMTC_DB
	DisplayAddLoopInfo(DISPLAY_IC_RMS);
#endif
	DisplayAddLoopInfo(DISPLAY_TOTAL_MAX_DEMAND);
	DisplayAddLoopInfo(DISPLAY_TOU1_MAX_DEMAND);
	DisplayAddLoopInfo(DISPLAY_TOU2_MAX_DEMAND);
	DisplayAddLoopInfo(DISPLAY_TOU3_MAX_DEMAND);
	DisplayAddLoopInfo(DISPLAY_TOU4_MAX_DEMAND);

	/* Init External Memory */
	ExtMemInit();

	/* Init RTC */
	RTCProcInit();

	/* Init presskey */
	PressKeyInit();

	/* Init TOU */
	TOUInit();

	/* Init Energy */
	EnergyInit(ENERGY_TOU_MEM_THRESHOLD);

	/* History Init */
	HistoryInit();

	/* Init Demand */
	DemandInit();

	/* Initialization of the System Events */
	EventInit();

	/* Communication Init */
	CommandInit();

	/* Init Core 1 */
	CoprocInit(ICM_MONITORING, RESET_TYPE_HARD);

	/* Init Metrology */
	if (!MetrologyInit()) {
		rstc_start_software_reset(RSTC);
	}

	/* Init Base Timer */
	BaseTimerInit();

#ifdef ENABLE_CHECK_METUPD_TIMER
	/* Set metrology update timer value in ms */
	BaseTimerSetMetUpdTimer(3000);
	/* Set callback to check Metrology update timer (3 seconds ) */
	BaseTimerSetMetUpdCallback(_metrology_update_timer_expires);
#endif

	/* Initialization of the system */
	_initialize_debug_pios();

	/* Write value in GPBR as initialization flag example */
	/* In case of TAMPER detection, this value will be erased */
	gpbr_write(APP_DEMO_GPBR_REG, (uint32_t)APP_GPBR_INIT_FLAG);
	LOG_APP_DEMO_DEBUG(("Set APP_DEMO_GPBR_REG[0x%08x]\r\n", gpbr_read(APP_DEMO_GPBR_REG)));
}

/**
 * \brief Process of Metering Demo Application.
 */
void demo_meter_app_process(void)
{
	/* Check METROLOGY events */
	if (VAFE.updataflag) {
		/* Reset AFE update flag */
		__disable_irq();
		VAFE.updataflag = 0;
		__enable_irq();

#ifdef ENABLE_CHECK_METUPD_TIMER
		/* Restart Metrology Update check Timer according to the current integration period */
		if (VMetrology.DSP_CTRL.M.WORD == 0) {
			/* 3 * int.period (1 sec) = 3 * 1000 */
			BaseTimerSetMetUpdTimer(3000);
		} else {
			/* 3 * int.period = 3 * 1000 / M */
			BaseTimerSetMetUpdTimer(3000 / VMetrology.DSP_CTRL.M.WORD);
		}
#endif

		/* Update metrology values */
		MetrologyProcess();

		/* Check Harmonics */
		if (MetrologyHarmonicIsReady()) {
			/* Launch Harmonic analysis */
			MetrologyHarmonicsProcess();
		}

		/* Update Energy values */
		EnergyProcess();

		/* Update Demand values */
		DemandUpdate(VAFE.RMS[Pt]);
	}

	/* Check TAMPER events */
	if (rtc_get_tamper_event_counter(RTC, 0)) {
		uint32_t gpbr_val;

		rtc_get_tamper_time(RTC, true, &tamper_time.hour, &tamper_time.minute, &tamper_time.second, 0);
		rtc_get_tamper_date(RTC, true, &tamper_date.year, &tamper_date.month, &tamper_date.day,
				    &tamper_date.week, 0);

		gpbr_val = gpbr_read(APP_DEMO_GPBR_REG);

		LOG_APP_DEMO_DEBUG(("Detected TAMPER event: [%02u/%02u/%04u %02u:%02u:%02u] APP_DEMO_GPBR_REG[0x%08x]\r\n",
			  tamper_date.month, tamper_date.day, tamper_date.year,
			  tamper_time.hour, tamper_time.minute, tamper_time.second, gpbr_val));
	}

	/* BaseTimer Process */
	BaseTimerProcess();

	/* Run Tasks from Queue */
	TaskRunFromQueue();
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
