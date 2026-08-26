/**
 * \file
 *
 * \brief Meter Demo Application
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
 * \mainpage Meter Demo Application
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
#include "demo_app_version.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#define STRING_HEADER   "\r\n-- Microchip Demo Meter - "BOARD_NAME " --\r\n" \
	"-- Compiled: "__DATE__ " "__TIME__ " -- \r\n"

/** All interrupt mask. */
#define ALL_INTERRUPT_MASK  0xffffffff

/* System variable structure */
sys_t Vsys;
uint8_t wake_up_source = 0;
uint32_t power_fail_detected = 0;

#define APP_DWDT_TIME_VALUE_MS       4096

#define APP_GPBR_INIT_FLAG           0xA55A

/* Supply Monitor flag */
static volatile bool gb_sm_is_invalid;

/* Tampering Last stamping */
static rtc_time_t tamper_time;
static rtc_date_t tamper_date;

/**
 * \brief SUPC handler.
 */
void SUPC_Handler(void)
{
	uint32_t ul_status;

	ul_status = supc_get_interrupt_status(SUPC);

	if (ul_status & SUPC_ISR_VDD3V3SMEV) {
		gb_sm_is_invalid = supc_monitor_voltage_is_invalid(SUPC);
	}

	/* Go to backup mode, having the option to save in memory the relevant parameters */
	/*if (gb_sm_is_invalid) {
                if (Vsys.mode == NORMAL_MODE) {
                        Vsys.mode = BACKUP_MODE;
                        backup_mode();
                }
        }*/
}

/**
 * \brief Reset handler.
 */
void RSTC_Handler(void)
{
	if (RSTC_WATCHDOG_RESET <= rstc_get_reset_cause(RSTC)) {
		Vsys.mode = RESTART_MODE;
	}
}

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

#if defined(APP_DEMO_DEBUG_CONSOLE) || defined(APP_DEMO_DEBUG_WDG0_RESTART)
/**
 * \brief Configure the debug console UART.
 */
static void configure_dbg_console(void)
{
	usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_CONSOLE_UART_BAUDRATE,
		.charlength = CONF_CONSOLE_UART_CHAR_LENGTH,
		.paritytype = CONF_CONSOLE_UART_PARITY,
		.stopbits = CONF_CONSOLE_UART_STOP_BITS,
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONF_CONSOLE_UART_ID);
	stdio_serial_init(CONF_CONSOLE_UART, &uart_serial_options);
}
#endif

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
static void configure_opto_uart(void)
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
	/* Set debouncers */
	ul_wkup_mode |= SUPC_WUMR_WKUPDBC_32_SK | SUPC_WUMR_FWUPDBC_32_SK;
	supc_set_wakeup_mode(SUPC, ul_wkup_mode);

	/* Set TAMPER (WKUP0) as wake up input */
	supc_set_wakeup_inputs(SUPC, SUPC_WUIR_WKUPEN0, SUPC_WUIR_WKUPT0_LOW);

	/* Set VDDIO Supply Monitor Wake-up Enable, Force Wake-up Pin Wake-up Enable and RTCOUT0 Output Drive Mode*/
	supc_set_backup_mode(SUPC, SUPC_BMR_FWUPEN | SUPC_BMR_VDD3V3SMWKEN | SUPC_BMR_MRTCOUT);
}


/**
 * \brief Configure Supply Monitor
 */
static void configure_supply_monitor(void)
{
#ifdef CONF_APP_VBAT
	/* The VDDBU power source is VBAT when a VDDIO under voltage is detected */
	supc_set_monitor_power_mode(SUPC, 1);
#endif
	/* Set Voltage threshold */
	supc_set_monitor_threshold(SUPC, SUPC_SM_THRESHOLD_2v75);

	/* Set sampling period */
	supc_set_monitor_sampling_period(SUPC, SUPC_SMMR_VDD3V3SMSMPL_ALWAYS_ON);

	/* Configure and enable SUPC interrupt */
	supc_enable_monitor_interrupt(SUPC);
	NVIC_SetPriority((IRQn_Type)ID_SUPC, 0);
	NVIC_EnableIRQ((IRQn_Type)ID_SUPC);

	/* Enable Supply monitor */
	supc_enable_monitor(SUPC);

	/* Update Supply Monitor Flag */
	gb_sm_is_invalid = supc_monitor_voltage_is_invalid(SUPC);
}


/**
 * \brief Wait mode.
 */
static void wait_mode(void)
{

}

/**
 * \brief Backup mode.
 */
static void backup_mode(void)
{
	LOG_APP_DEMO_DEBUG(("Enter in BACKUP_MODE\r\n"));

	/* Disable IPC Interrupt */
	NVIC_DisableIRQ(IPC1_IRQn);
	NVIC_ClearPendingIRQ(IPC1_IRQn);

	/* Disable RTC Interrupt */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);

	/* Save the energy accumulation in memory */
	/* Warning: System must have enough energy to have time to write the data */
	/* VEnergyCtrl.mem_update = MEM_UPDATE_URGENT;
	EnergyProcess(); */

	/* Set VDDIO Supply Monitor Wake-up Enable, Force Wake-up Pin Wake-up Enable and RTCOUT0 Output Drive Mode*/
	supc_set_backup_mode(SUPC, SUPC_BMR_FWUPEN | SUPC_BMR_VDD3V3SMWKEN | SUPC_BMR_MRTCOUT);

	/* If VDDIO is maintained when in Backup mode, all IOs except SLCD IOs
	if used in Backup mode, have to be set back to PIO input pull-up or pull-down
	or output low or high level to avoid power consumption with external on-board devices */
	if (!gb_sm_is_invalid) {
		/* Configure Display */
		cl010_clear_all();
		cl010_show_icon(CL010_ICON_BATTERY_LOW);

		/* Configure PIOs */
		pmc_enable_periph_clk(ID_PIOA);
		pmc_enable_periph_clk(ID_PIOB);
		pmc_enable_periph_clk(ID_PIOC);

		/* ------------ All PIO in input mode with pull up */
		/*	pio_set_input(PIOA, 0xFFFFFFFF, PIO_PULLUP); */
		/*	pio_set_input(PIOB, 0xFFFFFFFF, PIO_PULLUP); */
		/*	pio_set_input(PIOC, 0x000003FF, PIO_PULLUP); */

		/* ------------ All PIO open-drain disable */
		/*	pio_set_multi_driver(PIOA, 0xFFFFFFFF, 1); */
		/*	pio_set_multi_driver(PIOB, 0xFFFFFFFF, 1); */
		/*	pio_set_multi_driver(PIOC, 0x000003FF, 1); */
	}

	/* Initialize the sleep manager, lock Backup mode. */
	sleepmgr_init();
	sleepmgr_lock_mode(SLEEPMGR_BACKUP);
	sleepmgr_enter_sleep();

	/* SW Reset */
	rstc_start_software_reset(RSTC);
}

/**
 * \brief Restart mode.
 */
void restart_mode(void)
{
	slcdc_disable(SLCDC);
	rstc_start_software_reset(RSTC);
}

/**
 * \brief Idle mode.
 */
void idle_mode(void)
{
	LOG_APP_DEMO_DEBUG(("Enter in IDLE_MODE\r\n"));

	while (Vsys.mode == IDLE_MODE)
	{
		dwdt_restart(DWDT, WDT0_ID);
	}
}

/**
 * \brief Normal mode.
 */
void normal_mode(void)
{
	while (Vsys.mode == NORMAL_MODE) {

		dwdt_restart(DWDT, WDT0_ID);

		/* Check METROLOGY events */
		if (VAFE.updataflag) {
			/* Reset AFE update flag */
			__disable_irq();
			VAFE.updataflag = 0;
			__enable_irq();

			LOG_APP_DEMO_WDG0_DEBUG(("Metrology INTERVAL_NUM: %u\r\n", VMetrology.DSP_STATUS.INTERVAL_NUM.WORD >> 16));

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
			VCalibration.har_order = VCalibration.har_order & 0x7F;

			/* Update Energy values */
			EnergyProcess();
		}

		/* Check TAMPER events */
		if (rtc_get_tamper_event_counter(RTC, 0)) {
			volatile uint32_t gpbr0_val;

			rtc_get_tamper_time(RTC, true, &tamper_time.hour, &tamper_time.minute, &tamper_time.second, 0);
			rtc_get_tamper_date(RTC, true, &tamper_date.year, &tamper_date.month, &tamper_date.day,
					    &tamper_date.week, 0);

			gpbr0_val = gpbr_read(GPBR0);

			LOG_APP_DEMO_DEBUG(("Detected TAMPER event: [%02u/%02u/%04u %02u:%02u:%02u] GPBR0[0x%08x]\r\n",
				  tamper_date.month, tamper_date.day, tamper_date.year,
				  tamper_time.hour, tamper_time.minute, tamper_time.second, gpbr0_val));
		}

		/* BaseTimer Process */
		BaseTimerProcess();

		/* Run Tasks from Queue */
		TaskRunFromQueue();
	}
}

/**
 * Perform initialization and state machine.
 */
int main(void)
{
	uint32_t rst_cause;

	/* Configure PMC to avoid Reset by WDT */
	rstc_disable_pmc_reset_on_watchdog_0_reset(RSTC);
	/* rstc_disable_pmc_reset_on_software_reset(RSTC); */

	rst_cause = rstc_get_reset_cause(RSTC);

	/* Initialize the PIC32CX system */
	/* In order the software reset to restart only the Core 0 system, enable the line
	"rstc_disable_pmc_reset_on_software_reset(RSTC);" and add the condition
	"rst_cause == RSTC_SR_RSTTYP_SOFT_RST" to the following sentence */
	if (rst_cause == RSTC_SR_RSTTYP_WDT0_RST) {
		/* Restart only Core0 system */
		sysclk_restart_core0();
	} else {
		/* Initialize full clock system */
		sysclk_init();
	}

	/* Initialize board peripherals */
	board_init();

#ifdef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	dwdt_cfg_t dwdt_cfg;

	/* DWDT timeout will cause MCU reset. */
	/* Default timeout period is set with max value. */
	dwdt_cfg.ul_mode = WDT0_MR_PERIODRST | WDT0_MR_WDIDLEHLT | WDT0_MR_WDDBG0HLT | WDT0_MR_WDDBG1HLT;
	dwdt_cfg.ul_prescaler = WDT0_IL_PRESC_RATIO128;
	dwdt_cfg.ul_rep_time = 0;
	dwdt_cfg.ul_intlvl_time = 0;
	dwdt_cfg.ul_slck = BOARD_FREQ_SLCK_XTAL;
	dwdt_cfg.ul_time = APP_DWDT_TIME_VALUE_MS;
	dwdt_init(DWDT, WDT0_ID, &dwdt_cfg);
#endif

#ifdef ENABLE_CACHE
	struct cmcc_config cmcc_cfg;
	/* Enable Cache */
	cmcc_get_config_defaults(&cmcc_cfg);
	cmcc_cfg.cmcc_monitor_enable = false;
	cmcc_init(CMCC0, &cmcc_cfg);

	cmcc_enable(CMCC0);
#else
	/* Disable Cache */
	cmcc_disable(CMCC0);
#endif

#if defined(APP_DEMO_DEBUG_CONSOLE) || defined(APP_DEMO_DEBUG_WDG0_RESTART)
	/* Configure serial console */
	configure_dbg_console();
	/* Show demo information through Console port */
#if defined(APP_DEMO_DEBUG_CONSOLE)
		LOG_APP_DEMO_DEBUG((STRING_HEADER));
		LOG_APP_DEMO_DEBUG(("-- App Demo Ver. %d\r\n", DEMO_APP_VERSION));
#elif defined(APP_DEMO_DEBUG_WDG0_RESTART)
		LOG_APP_DEMO_WDG0_DEBUG((STRING_HEADER));
		LOG_APP_DEMO_WDG0_DEBUG(("-- App Demo Ver. %d\r\n", DEMO_APP_VERSION));
#endif
#endif

	/* Configure serial command console */
	configure_command_usart();

	/* Configure OPTO uart */
	configure_opto_uart();

	/* Set Normal Mode by default */
	Vsys.mode = NORMAL_MODE;
	/* Read CHIP ID : architecture */
	Vsys.arch_id = chipid_read_arch(CHIPID);

	/* Configure TAMPER detection and FWUP wake up event */
	configure_tamper_fwup();

	/* Enable VDDIO Supply monitor */
	configure_supply_monitor();

	/* Task Initialize queue */
	TaskInit();

	/* Init LCD */
	DisplayInit();
	/* Configure display time loop */
	DisplaySetTimerLoop(3);
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
	/* In order the software reset to restart only the Core 0 system, add the condition
	"rst_cause == RSTC_SR_RSTTYP_SOFT_RST" to the following sentence */
	if (rst_cause == RSTC_SR_RSTTYP_WDT0_RST) {
		CoprocInit(ICM_MONITORING, RESET_TYPE_SOFT);
		LOG_APP_DEMO_WDG0_DEBUG(("\r\n\t-I- Coproc RESTART\r\n"));
	} else {
		CoprocInit(ICM_MONITORING, RESET_TYPE_HARD);
		if (!MetrologyInit()) {
			LOG_APP_DEMO_WDG0_DEBUG(("\r\n\t-I- Coproc INIT...ERROR....reseting\r\n"));
			delay_ms(1000);
			rstc_start_software_reset(RSTC);
		}
		LOG_APP_DEMO_WDG0_DEBUG(("\r\n\t-I- Coproc INIT...OK\r\n"));
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
	gpbr_write(GPBR0, (uint32_t)APP_GPBR_INIT_FLAG);
	LOG_APP_DEMO_DEBUG(("Set GPBR0[0x%08x]\r\n", gpbr_read(GPBR0)));

	/* Main loop */
	while (1) {
		switch (Vsys.mode) {
		case IDLE_MODE:
			idle_mode();
			break;

		case BACKUP_MODE:
			backup_mode();
			break;

		case WAIT_MODE:
			wait_mode();
			break;

		case RESTART_MODE:
			restart_mode();
			break;

		case NORMAL_MODE:
		default:
			normal_mode();
			break;
		}
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
