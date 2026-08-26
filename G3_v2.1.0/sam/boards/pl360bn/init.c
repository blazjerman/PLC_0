/**
 * \file
 *
 * \brief PL360BN board init.
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
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

#include "compiler.h"
#include "board.h"
#include "conf_board.h"
#include "ioport.h"
#include "pio.h"
#ifdef CONF_BOARD_CONFIG_MPU_AT_INIT
#include "mpu.h"
#endif
#ifdef CONF_BOARD_32K_XTAL
#include "supc.h"
#endif

/**
 * \brief Set peripheral mode for IOPORT pins.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param port IOPORT port to configure
 * \param masks IOPORT pin masks to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_port_peripheral_mode(port, masks, mode) \
	do {\
		ioport_set_port_mode(port, masks, mode);\
		ioport_disable_port(port, masks);\
	} while (0)

/**
 * \brief Set peripheral mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_pin_peripheral_mode(pin, mode) \
	do {\
		ioport_set_pin_mode(pin, mode);\
		ioport_disable_pin(pin);\
	} while (0)

/**
 * \brief Set input mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 * \param sense Sense for interrupt detection (\ref ioport_sense)
 */
#define ioport_set_pin_input_mode(pin, mode, sense) \
	do {\
		ioport_set_pin_dir(pin, IOPORT_DIR_INPUT);\
		ioport_set_pin_mode(pin, mode);\
		ioport_set_pin_sense_mode(pin, sense);\
	} while (0)


#ifdef CONF_BOARD_CONFIG_MPU_AT_INIT
/**
 *	Default memory map
 *	Address range        Memory region      Memory type   Shareability  Cache policy
 *	0x00000000- 0x1FFFFFFF Code             Normal        Non-shareable  WT
 *	0x20000000- 0x3FFFFFFF SRAM             Normal        Non-shareable  WBWA
 *	0x40000000- 0x5FFFFFFF Peripheral       Device        Non-shareable  -
 *	0x60000000- 0x7FFFFFFF RAM              Normal        Non-shareable  WBWA
 *	0x80000000- 0x9FFFFFFF RAM              Normal        Non-shareable  WT
 *	0xA0000000- 0xBFFFFFFF Device           Device        Shareable
 *	0xC0000000- 0xDFFFFFFF Device           Device        Non Shareable
 *	0xE0000000- 0xFFFFFFFF System           -                  -
 */

/**
 * \brief Set up a memory region.
 */
static void _setup_memory_region( void )
{

	uint32_t dw_region_base_addr;
	uint32_t dw_region_attr;

	__DMB();

/**
 *	ITCM memory region --- Normal
 *	START_Addr:-  0x00000000UL
 *	END_Addr:-    0x00400000UL
 */
	dw_region_base_addr =
		ITCM_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_ITCM_REGION;

	dw_region_attr =
		//MPU_AP_PRIVILEGED_READ_WRITE |
                MPU_AP_FULL_ACCESS |
		mpu_cal_mpu_region_size(ITCM_END_ADDRESS - ITCM_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

/**
 *	Internal flash memory region --- Normal read-only
 *	(update to Strongly ordered in write accesses)
 *	START_Addr:-  0x00400000UL
 *	END_Addr:-    0x00600000UL
 */

	dw_region_base_addr =
		IFLASH_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_IFLASH_REGION;

	dw_region_attr =
		//MPU_AP_READONLY |
                MPU_AP_FULL_ACCESS |
		//INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
                STRONGLY_ORDERED_SHAREABLE_TYPE |
		mpu_cal_mpu_region_size(IFLASH_END_ADDRESS - IFLASH_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

/**
 *	DTCM memory region --- Normal
 *	START_Addr:-  0x20000000L
 *	END_Addr:-    0x20400000UL
 */

	/* DTCM memory region */
	dw_region_base_addr =
		DTCM_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_DTCM_REGION;

	dw_region_attr =
		MPU_AP_PRIVILEGED_READ_WRITE |
		mpu_cal_mpu_region_size(DTCM_END_ADDRESS - DTCM_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

/**
 *	SRAM Cacheable memory region --- Normal
 *	START_Addr:-  0x20400000UL
 *	END_Addr:-    0x2043FFFFUL
 */
	/* SRAM memory  region */
	dw_region_base_addr =
		SRAM_FIRST_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_SRAM_REGION_1;

	dw_region_attr =
		MPU_AP_FULL_ACCESS    |
		INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
		mpu_cal_mpu_region_size(SRAM_FIRST_END_ADDRESS - SRAM_FIRST_START_ADDRESS)
		| MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

        /* SRAM memory  region */



/**
 *	Internal SRAM second partition memory region --- Normal
 *	START_Addr:-  0x20440000UL
 *	END_Addr:-    0x2045FFFFUL
 */
	/* SRAM memory region */
	dw_region_base_addr =
		SRAM_SECOND_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_SRAM_REGION_2;

	dw_region_attr =
		MPU_AP_FULL_ACCESS    |
		INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
		mpu_cal_mpu_region_size(SRAM_SECOND_END_ADDRESS - SRAM_SECOND_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

#ifdef MPU_HAS_NOCACHE_REGION
	dw_region_base_addr =
        SRAM_NOCACHE_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_NOCACHE_SRAM_REGION;

    dw_region_attr =
        MPU_AP_FULL_ACCESS    |
        INNER_OUTER_NORMAL_NOCACHE_TYPE( SHAREABLE ) |
        mpu_cal_mpu_region_size(NOCACHE_SRAM_REGION_SIZE) |
        MPU_REGION_ENABLE;

    mpu_set_region( dw_region_base_addr, dw_region_attr);
#endif

/**
 *	Peripheral memory region --- DEVICE Shareable
 *	START_Addr:-  0x40000000UL
 *	END_Addr:-    0x5FFFFFFFUL
 */
	dw_region_base_addr =
		PERIPHERALS_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_PERIPHERALS_REGION;

	dw_region_attr = MPU_AP_FULL_ACCESS |
		MPU_REGION_EXECUTE_NEVER |
		SHAREABLE_DEVICE_TYPE |
		mpu_cal_mpu_region_size(PERIPHERALS_END_ADDRESS - PERIPHERALS_START_ADDRESS)
		|MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);


/**
 *	External EBI memory  memory region --- Strongly Ordered
 *	START_Addr:-  0x60000000UL
 *	END_Addr:-    0x6FFFFFFFUL
 */
	dw_region_base_addr =
		EXT_EBI_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_EXT_EBI_REGION;

	dw_region_attr =
		MPU_AP_FULL_ACCESS |
		/* External memory Must be defined with 'Device' or 'Strongly Ordered' attribute for write accesses (AXI) */
		STRONGLY_ORDERED_SHAREABLE_TYPE |
		mpu_cal_mpu_region_size(EXT_EBI_END_ADDRESS - EXT_EBI_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

/**
 *	SDRAM cacheable memory region --- Normal
 *	START_Addr:-  0x70000000UL
 *	END_Addr:-    0x7FFFFFFFUL
 */
	dw_region_base_addr =
		SDRAM_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_DEFAULT_SDRAM_REGION;

	dw_region_attr =
		MPU_AP_FULL_ACCESS    |
		INNER_NORMAL_WB_RWA_TYPE( SHAREABLE ) |
		mpu_cal_mpu_region_size(SDRAM_END_ADDRESS - SDRAM_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);

/**
 *	QSPI memory region --- Strongly ordered
 *	START_Addr:-  0x80000000UL
 *	END_Addr:-    0x9FFFFFFFUL
 */
	dw_region_base_addr =
		QSPI_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_QSPIMEM_REGION;

	dw_region_attr =
		MPU_AP_FULL_ACCESS |
		STRONGLY_ORDERED_SHAREABLE_TYPE |
		mpu_cal_mpu_region_size(QSPI_END_ADDRESS - QSPI_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);


/**
 *	USB RAM Memory region --- Device
 *	START_Addr:-  0xA0100000UL
 *	END_Addr:-    0xA01FFFFFUL
 */
	dw_region_base_addr =
		USBHSRAM_START_ADDRESS |
		MPU_REGION_VALID |
		MPU_USBHSRAM_REGION;

	dw_region_attr =
		MPU_AP_FULL_ACCESS |
		MPU_REGION_EXECUTE_NEVER |
		SHAREABLE_DEVICE_TYPE |
		mpu_cal_mpu_region_size(USBHSRAM_END_ADDRESS - USBHSRAM_START_ADDRESS) |
		MPU_REGION_ENABLE;

	mpu_set_region( dw_region_base_addr, dw_region_attr);


	/* Enable the memory management fault , Bus Fault, Usage Fault exception */
	SCB->SHCSR |= (SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk
					| SCB_SHCSR_USGFAULTENA_Msk);

	/* Enable the MPU region */
	mpu_enable( MPU_ENABLE | MPU_PRIVDEFENA);

	__DSB();
	__ISB();
}
#endif

#ifdef CONF_BOARD_ENABLE_TCM_AT_INIT
#if defined(__GNUC__)
extern char _itcm_lma, _sitcm, _eitcm;
#endif

/** \brief  TCM memory enable
* The function enables TCM memories
*/
static inline void tcm_enable(void)
{

	__DSB();
	__ISB();

	SCB->ITCMCR = (SCB_ITCMCR_EN_Msk  | SCB_ITCMCR_RMW_Msk | SCB_ITCMCR_RETEN_Msk);
	SCB->DTCMCR = ( SCB_DTCMCR_EN_Msk | SCB_DTCMCR_RMW_Msk | SCB_DTCMCR_RETEN_Msk);

	__DSB();
	__ISB();
}
#else
/** \brief  TCM memory Disable

	The function enables TCM memories
 */
static inline void tcm_disable(void)
{

	__DSB();
	__ISB();
	SCB->ITCMCR &= ~(uint32_t)(1UL);
	SCB->DTCMCR &= ~(uint32_t)SCB_DTCMCR_EN_Msk;
	__DSB();
	__ISB();
}
#endif

void board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
#else
	WDT->WDT_MR = (WDT->WDT_MR & 0xFFFFF000) | 0x7FF; /* 8 secs */
#endif

#ifdef CONF_BOARD_CONFIG_MPU_AT_INIT
	_setup_memory_region();
#endif

#ifdef CONF_BOARD_ENABLE_CACHE
	/* Enabling the Cache */
	SCB_EnableICache();
	SCB_EnableDCache();
#endif

#ifdef CONF_BOARD_32K_XTAL
	supc_switch_sclk_to_32kxtal(SUPC, 0);
#endif


#ifdef CONF_BOARD_ENABLE_TCM_AT_INIT
	/* TCM Configuration */
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB
					| EEFC_FCR_FARG(8));
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_SGPB
					| EEFC_FCR_FARG(7));
	tcm_enable();
#if defined(__GNUC__)
	volatile char *dst = &_sitcm;
	volatile char *src = &_itcm_lma;
	/* copy code_TCM from flash to ITCM */
	while(dst < &_eitcm){
		*dst++ = *src++;
	}
#endif
#else
	/* TCM Configuration */
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB
					| EEFC_FCR_FARG(8));
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_CGPB
					| EEFC_FCR_FARG(7));

	tcm_disable();
#endif

	/* Initialize IOPORTs */
	ioport_init();

	/* Configure the pins connected to LEDs as output and set their
	 * default initial state to high (LEDs off).
	 */
	ioport_set_pin_dir(LED0_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);
	ioport_set_pin_dir(LED1_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED1_GPIO, LED0_INACTIVE_LEVEL);

	/* Configure UART0 pins */
#ifdef CONF_BOARD_UART0
	ioport_set_port_peripheral_mode(PINS_UART0_PORT, PINS_UART0, PINS_UART0_FLAGS);
#endif

#ifdef CONF_BOARD_UART2
	ioport_set_port_peripheral_mode(PINS_UART2_PORT, PINS_UART2, PINS_UART2_FLAGS);
#endif

#ifdef CONF_BOARD_UART4
	ioport_set_port_peripheral_mode(PINS_UART4_PORT, PINS_UART4, PINS_UART4_FLAGS);
#endif

	/* Configure UART1 pins (CONSOLE) */
#ifdef CONF_BOARD_UART_CONSOLE
	switch (CONSOLE_UART_ID){
	case ID_UART0:
		ioport_set_port_peripheral_mode(PINS_UART0_PORT, PINS_UART0, PINS_UART0_FLAGS);
		break;
	case ID_UART2:
		ioport_set_port_peripheral_mode(PINS_UART2_PORT, PINS_UART2, PINS_UART2_FLAGS);
		break;
	case ID_UART4:
		ioport_set_port_peripheral_mode(PINS_UART4_PORT, PINS_UART4, PINS_UART4_FLAGS);
		break;
	}
#endif

	/* Configure USART0 pins */
#ifdef CONF_BOARD_USART0
	/* Configure USART0 RXD pin */
	ioport_set_pin_peripheral_mode(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
	/* Configure USART0 TXD pin */
	ioport_set_pin_peripheral_mode(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);
#endif

	/* Configure USART1 pins */
#ifdef CONF_BOARD_USART1
	/* Configure USART1 RXD pin */
	ioport_set_pin_peripheral_mode(PIN_USART1_RXD_IDX, PIN_USART1_RXD_FLAGS);
	/* Configure USART1 TXD pin */
	ioport_set_pin_peripheral_mode(PIN_USART1_TXD_IDX, PIN_USART1_TXD_FLAGS);
#endif

	/* Configure Xplain PRO SLP pin */
#ifdef CONF_BOARD_XP_SLP
	ioport_set_pin_dir(XP_SLP_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(XP_SLP_GPIO, XP_SLP_INACTIVE_LEVEL);
#endif

	/* Configure USB Detect pin */
#ifdef CONF_BOARD_USB_DETECT
	ioport_set_pin_input_mode(GPIO_USB_DETECT, GPIO_USB_DETECT_FLAGS, GPIO_USB_DETECT_SENSE);
#endif

	/* Configure Shutdown Detect pin */
#ifdef CONF_SHUTDOWN_DETECT
	ioport_set_pin_dir(SHUTDOWN_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(SHUTDOWN_GPIO, SHUTDOWN_INACTIVE_LEVEL);
#endif

#if defined (CONF_BOARD_SD_MMC_HSMCI)
	/* Configure HSMCI pins */
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCDA_GPIO, PIN_HSMCI_MCCDA_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCK_GPIO, PIN_HSMCI_MCCK_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA0_GPIO, PIN_HSMCI_MCDA0_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA1_GPIO, PIN_HSMCI_MCDA1_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA2_GPIO, PIN_HSMCI_MCDA2_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA3_GPIO, PIN_HSMCI_MCDA3_FLAGS);

    /* Configure SD/MMC card detect pin */
	ioport_set_pin_dir(SD_MMC_0_CD_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(SD_MMC_0_CD_GPIO, SD_MMC_0_CD_FLAGS);
#endif

#ifdef CONF_BOARD_KSZ8051MNL
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXC_IDX, PIN_KSZ8051MNL_RXC_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXC_IDX, PIN_KSZ8051MNL_TXC_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXEN_IDX, PIN_KSZ8051MNL_TXEN_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXD3_IDX, PIN_KSZ8051MNL_TXD3_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXD2_IDX, PIN_KSZ8051MNL_TXD2_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXD1_IDX, PIN_KSZ8051MNL_TXD1_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_TXD0_IDX, PIN_KSZ8051MNL_TXD0_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXD3_IDX, PIN_KSZ8051MNL_RXD3_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXD2_IDX, PIN_KSZ8051MNL_RXD2_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXD1_IDX, PIN_KSZ8051MNL_RXD1_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXD0_IDX, PIN_KSZ8051MNL_RXD0_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXER_IDX, PIN_KSZ8051MNL_RXER_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_RXDV_IDX, PIN_KSZ8051MNL_RXDV_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_CRS_IDX, PIN_KSZ8051MNL_CRS_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_COL_IDX, PIN_KSZ8051MNL_COL_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_MDC_IDX, PIN_KSZ8051MNL_MDC_FLAGS);
	ioport_set_pin_peripheral_mode(PIN_KSZ8051MNL_MDIO_IDX, PIN_KSZ8051MNL_MDIO_FLAGS);
	ioport_set_pin_dir(PIN_KSZ8051MNL_INTRP_IDX, IOPORT_DIR_INPUT);
#endif

#if (defined(CONF_BOARD_SPI0) || defined(CONF_BOARD_ATPL250))
	ioport_set_pin_peripheral_mode(SPI_MISO_GPIO, SPI_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(SPI_MOSI_GPIO, SPI_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(SPI_SPCK_GPIO, SPI_SPCK_FLAGS);
#ifdef CONF_BOARD_SPI0_NPCS0
	ioport_set_pin_peripheral_mode(SPI_NPCS0_GPIO, SPI_NPCS0_FLAGS);
#endif
#ifdef CONF_BOARD_SPI0_NPCS1
	ioport_set_pin_peripheral_mode(SPI_NPCS1_GPIO, SPI_NPCS1_FLAGS);
#endif
#ifdef CONF_BOARD_SPI0_NPCS2
 	ioport_set_pin_peripheral_mode(SPI_NPCS2_GPIO, SPI_NPCS2_FLAGS);
#endif
#endif

#if (defined(CONF_BOARD_TWI0) || defined(CONF_BOARD_EDBG_TWI))
	ioport_set_pin_peripheral_mode(TWI0_DATA_GPIO, TWI0_DATA_FLAGS);
	ioport_set_pin_peripheral_mode(TWI0_CLK_GPIO, TWI0_CLK_FLAGS);
#endif

#ifdef CONF_BOARD_SDRAMC
	pio_configure_pin(SDRAM_BA0_PIO, SDRAM_BA0_FLAGS);
	pio_configure_pin(SDRAM_BA1_PIO, SDRAM_BA1_FLAGS);
	pio_configure_pin(SDRAM_SDCK_PIO, SDRAM_SDCK_FLAGS);
	pio_configure_pin(SDRAM_SDCKE_PIO, SDRAM_SDCKE_FLAGS);
	pio_configure_pin(SDRAM_SDCS_PIO, SDRAM_SDCS_FLAGS);
	pio_configure_pin(SDRAM_RAS_PIO, SDRAM_RAS_FLAGS);
	pio_configure_pin(SDRAM_CAS_PIO, SDRAM_CAS_FLAGS);
	pio_configure_pin(SDRAM_SDWE_PIO, SDRAM_SDWE_FLAGS);
	pio_configure_pin(SDRAM_NBS0_PIO, SDRAM_NBS0_FLAGS);
	pio_configure_pin(SDRAM_NBS1_PIO, SDRAM_NBS1_FLAGS);
	pio_configure_pin(SDRAM_A2_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A3_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A4_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A5_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A6_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A7_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A8_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A9_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A10_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A11_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_SDA10_PIO, SDRAM_SDA10_FLAGS);
	pio_configure_pin(SDRAM_A13_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_D0_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D1_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D2_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D3_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D4_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D5_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D6_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D7_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D8_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D9_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D10_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D11_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D12_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D13_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D14_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D15_PIO, SDRAM_D_FLAGS);

	MATRIX->CCFG_SMCNFCS = CCFG_SMCNFCS_SDRAMEN;
#endif

	/* Configure VDD sense GPIO to AFEC0 */
	ioport_set_pin_dir(VDD_SENSE_GPIO, IOPORT_DIR_INPUT);

#ifdef CONF_BOARD_ENABLE_CACHE
	/* Enabling the Cache */
	SCB_EnableICache();
	SCB_EnableDCache();
#endif

}
