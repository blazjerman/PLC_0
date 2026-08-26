/**
 *
 * \file
 *
 * \brief Hardware Abstraction Layer file
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
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

#include <stdbool.h>
#include <stdint.h>
#include <hal/hal.h>
#include <string.h>
#if !(SAM4C || SAM4CP || SAM4CM || SAME70)
#include <stdlib.h>
#endif

/* #define LOG_PLATFORM(a)   printf a */
#define LOG_PLATFORM(a)   (void)0

#if (SAM || PIC32CX)
#include "asf.h"
#endif /* (SAM || PIC32CX) */

#include "conf_board.h"
#include "conf_hal.h"
#include "conf_project.h"

#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
#if (BOARD == ATPL360AMB || BOARD == ATPL360MB)
#include "c0216CiZ.h"  /* pending to display update */
#elif (BOARD == ATPL360ASB)
#include "vim878.h"
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
#include "cl010.h"
#include "cl010_font.h"
#include "cl010_segmap.h"
#endif
#endif

#if (BOARD == PL360G55CB_EK) || (BOARD == PL360G55CF_EK)
#define PIN_TC_TIOA           TC_TIOA1_GPIO
#define PIN_TC_TIOA_FLAGS     TC_TIOA1_FLAGS
#define ADC_TRIG_TIO_CH       ADC_TRIG_TIO_CH_1
#define ID_TC_TRIG            ID_TC1
#define TC_TRIG_CHN           1
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTG_EK) || (BOARD == PIC32CXMTC_DB)
#define PIN_TC_TIOA           PIN_TC0_TIOA0
#define PIN_TC_TIOA_FLAGS     PIN_TC0_TIOA0_FLAGS
#define ADC_TRIG_TIO_CH       ADC_TRIG_TIO0_CH_0
#define ID_TC_TRIG            ID_TC0_CHANNEL0
#define TC_TRIG_CHN           0
#else
#define PIN_TC_TIOA           PIN_TC0_TIOA0
#define PIN_TC_TIOA_FLAGS     PIN_TC0_TIOA0_FLAGS
#define ADC_TRIG_TIO_CH       ADC_TRIG_TIO_CH_0
#define ID_TC_TRIG            ID_TC0
#define TC_TRIG_CHN           0
#endif

/* Variable definition for microcontroller mode */
uint32_t ul_led_count_ms_cfg = 0;
uint32_t ul_led_count_ms;
uint8_t uc_led_swap = 0;
volatile uint8_t uc_flag_call_process = 0;
volatile uint8_t uc_flag_call_app_process = 0;
uint32_t ul_call_process_count_ms_cfg = 0;
uint32_t ul_call_process_count_ms = 0;
uint32_t ul_call_app_process_count_ms_cfg = 0;
uint32_t ul_call_app_process_count_ms = 0;
void (*pf_platform_ms_timer_callback)(void) = NULL;
void (*pf_pdd_callback)(void) = NULL;
void platform_pdd_handler(void);
#if (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
#ifdef CONF_BOARD_LCD_EN
static bool b_led_swap_on = false;
#endif
#endif

/* IPv6 stack includes */
#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
#include "core/net.h"
#endif

#ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER
/** ADC sample data */
uint16_t us_adc_value;
/** Size of the power down detector (PDD) power supply ADC measurement buffer */
#define PDD_BUFFER_SIZE     1

/** PDD's Tracking Time*/
#define PDD_TRACKING_TIME         1
/** PDD's Transfer Period */
#define PDD_TRANSFER_PERIOD       1

/** Reference voltage for ADC, in mv. */
#define VOLT_REF        (3300)
#define MAX_DIGITAL     (1023)
#endif /* #ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER */

#if defined(CONF_STORAGE_INTERNAL_FLASH)
/* Address points to last Lock Region in Flash */
#if BOARD == ATPL360AMB || BOARD == ATPL360ASB || BOARD == ATPL360MB
/* Reserve memory to store ATPL360 binary file */
	#define ADDR_APP_DATA (0x010E0000 - IFLASH_LOCK_REGION_SIZE)
#elif defined(_SAM4C16C_0_) || defined(_SAM4CP16C_0_) || defined(_SAM4CMS16C_0_) || defined(_SAM4CMP16C_0_)
	#define ADDR_APP_DATA (IFLASH_CNC_ADDR + IFLASH_SIZE - IFLASH_LOCK_REGION_SIZE)
#elif defined(_SAM4SD32C_)
	#define ADDR_APP_DATA (IFLASH0_ADDR + IFLASH0_SIZE - IFLASH0_LOCK_REGION_SIZE)
#elif defined(_SAM4E16E_) || defined(_SAMG55J19_) || defined(_SAME70Q21_)
	#define ADDR_APP_DATA (IFLASH_ADDR + IFLASH_SIZE - IFLASH_LOCK_REGION_SIZE)
#elif PIC32CX
	#define ADDR_APP_DATA (IFLASH0_CNC_ADDR + IFLASH0_SIZE - IFLASH0_LOCK_REGION_SIZE)
#endif
#endif

#if defined(CONF_STORAGE_AT45DBX_FLASH)
	#include "at45db32.h"
	#define EXT_ADDR_APP_DATA_PAGE 0x007FFC00
#endif

/* Reserve 256 bytes for permanent data, such as EUI-64 */
#define PERMANENT_DATA_RESERVED_SIZE   256

#if BOARD == ATPL360AMB || BOARD == ATPL360ASB || BOARD == ATPL360MB
/* Reserve memory to store ATPL360 binary file */
	#define ADDR_PERMANENT_DATA (0x010E0000 - PERMANENT_DATA_RESERVED_SIZE)
#elif defined(_SAM4C16C_0_) || defined(_SAM4CP16C_0_) || defined(_SAM4CMS16C_0_) || defined(_SAM4CMP16C_0_)
	#define ADDR_PERMANENT_DATA (IFLASH_CNC_ADDR + IFLASH_SIZE - PERMANENT_DATA_RESERVED_SIZE)
/* 0x01000000 + 0x100000 - 0x100 = 0x010FFF00 for _SAM4C16C_0_ */
#elif defined(_SAM4SD32C_)
	#define ADDR_PERMANENT_DATA (IFLASH0_ADDR + IFLASH0_SIZE - PERMANENT_DATA_RESERVED_SIZE)
#elif defined(_SAM4E16E_) || defined(_SAMG55J19_) || defined(_SAME70Q21_)
	#define ADDR_PERMANENT_DATA (IFLASH_ADDR + IFLASH_SIZE - PERMANENT_DATA_RESERVED_SIZE)
#elif PIC32CX
	#define ADDR_PERMANENT_DATA (IFLASH0_CNC_ADDR + IFLASH0_SIZE - PERMANENT_DATA_RESERVED_SIZE)
#endif

#define CHIP_ID_SIZE 4  /* 4 uint32 words (128 bits) */

#define MAC_CONFIG_KEY_0      0xAA
#define MAC_CONFIG_KEY_1      0x55
#define MAC_CONFIG_KEY_SIZE      2
#define EUI64_SIZE               8

#if (SAMV71 || SAMV70 || SAME70)
#define PLATFORM_PDD_PRIO          1
#define PLATFORM_PDD_VDD_MAX   0xFFF
#endif

#if (SAME70) && defined(CONF_BOARD_SDRAMC)
/* Configuration for on-board external SDRAM */
const sdramc_memory_dev_t SDRAM_IS42S16400J_7B2LI = {
	20,
	SDRAMC_MR_MODE_NORMAL,
	{
		SDRAMC_CR_NC_COL8      |  /* 8 column bits */
		SDRAMC_CR_NR_ROW12     |  /* 12 row bits (4K) */
		SDRAMC_CR_NB_BANK4     |  /* 4 banks */
		SDRAMC_CR_CAS_LATENCY3 |  /* CAS Latency 3 */
		SDRAMC_CR_DBW          |  /* 16 bit */
		SDRAMC_CR_TWR(4)       |  /* 4 SDCK cycles minimum */
		SDRAMC_CR_TRC_TRFC(10) |  /* Command period (Ref to Ref / ACT to ACT) 63ns minimum. If SDCK=143MHz minimum TRFC=10 */
		SDRAMC_CR_TRP(3)       |  /* Command period (PRE to ACT) 15 ns min. If SDCK=143MHz minimum TRP=3 */
		SDRAMC_CR_TRCD(3)      |  /* Active Command to read/Write Command delay 15ns. If SDCK=143MHz minimum TRCD=3 */
		SDRAMC_CR_TRAS(7)      |  /* Command period (ACT to PRE)  42ns min. If SDCK=143MHz minimum TRCD=7 */
		SDRAMC_CR_TXSR(11)        /* Exit self-refresh to active time  70ns Min. If SDCK=143MHz minimum TRCD=11 */
	},
};
#endif

/* The mem page is 512 bytes. R&W the whole page ensure previous data of the page is not deleted */
union memory_union {
	uint32_t u32StartUpCounter; /* The first 4 bytes store the startup counter */
	uint8_t u8MemPage[IFLASH_PAGE_SIZE];
};
union memory_union mem;

#if defined(CONF_EUI64_FROM_USER_SIGNATURE) || defined(CONF_STORAGE_USER_SIGNATURE)
/* Buufer to handle User Signature */
static uint8_t auc_user_sig_container[IFLASH_PAGE_SIZE];
#endif

#if defined(CONF_EUI64_FROM_USER_SIGNATURE)
/* Intermediate buffer for EUI64 validation on User Signature */
static uint8_t uc_eui64_buf[MAC_CONFIG_KEY_SIZE + EUI64_SIZE];
#endif

/* Function declarations */
void __user_low_level_init(void);

/**
 * \brief Low level initialization called from Reset_Handler, before calling
 * to main and before data initialization. This function should not use
 * initialized data.
 */
void __user_low_level_init(void)
{
#if BOARD == PIC32CXMTSH_DB || BOARD == PIC32CXMTC_DB || BOARD == PIC32CXMTG_EK
	/* Enable coprocessor peripherals to allow access to PIOD */
	uint32_t read_reg;

	/* Assert coprocessor reset and reset its peripheral */
	read_reg = RSTC->RSTC_MR;
	read_reg &= ~(RSTC_MR_KEY_Msk | RSTC_MR_CPEREN | RSTC_MR_CPROCEN);
	read_reg |= RSTC_MR_KEY_PASSWD;
	RSTC->RSTC_MR = read_reg;

	/* Enables Coprocessor Bus Master Clock */
	PMC->PMC_SCER = PMC_SCER_CPBMCK | PMC_SCER_CPKEY_PASSWD;

	/* Set coprocessor clock prescaler */
	read_reg = PMC->PMC_CPU_CKR;
	read_reg &= ~PMC_CPU_CKR_CPPRES_Msk;
	PMC->PMC_CPU_CKR = read_reg | PMC_CPU_CKR_CPPRES(1);

	/*  Wait to PMC_SR_CPMCKRDY = 1 */
	while ((PMC->PMC_SR & PMC_SR_CPMCKRDY) == 0) {
	}

	/* Release coprocessor peripheral reset */
	RSTC->RSTC_MR |= (RSTC_MR_KEY_PASSWD | RSTC_MR_CPEREN);

	/* Set coprocessor clock prescaler */
	read_reg = PMC->PMC_CPU_CKR;
	read_reg &= ~PMC_CPU_CKR_CPPRES_Msk;
	PMC->PMC_CPU_CKR = read_reg | PMC_CPU_CKR_CPPRES(0);

	/*  Wait to PMC_SR_CPMCKRDY = 1 */
	while ((PMC->PMC_SR & PMC_SR_CPMCKRDY) == 0) {
	}
#endif

	/* Critical initialization of PLC pins */
	pplc_if_crit_init();
}

/**********************************************************************************************************************/
/************************************************ --------------------- ***********************************************/
/************************************************ RANDOM NUM GENERATION ***********************************************/
/************************************************ --------------------- ***********************************************/
/**********************************************************************************************************************/

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_random_init(void)
{
#if !(SAM4C || SAM4CP || SAM4CM || SAME70 || PIC32CX)
	uint32_t ul_random_num;
#endif

	platform_init_storage();

	platform_read_storage(sizeof(union memory_union), &mem);
	mem.u32StartUpCounter++;

	LOG_PLATFORM(("Start-up counter: %lu\n", mem.u32StartUpCounter));

#if (SAM4C || SAM4CP || SAM4CM || SAME70 || PIC32CX)
	/* Configure PMC */
	pmc_enable_periph_clk(ID_TRNG);
	/* Enable TRNG */
#if PIC32CX
	trng_enable(TRNG, sysclk_get_peripheral_hz());
#else
	trng_enable(TRNG);
#endif
	LOG_PLATFORM(("TRNG init\r\n"));
#else
	ul_random_num = DWT->CYCCNT;

	ul_random_num = 36969 * (ul_random_num & 65535) + (ul_random_num >> 16);
	mem.u32StartUpCounter = 18000 * (mem.u32StartUpCounter & 65535) + (mem.u32StartUpCounter >> 16);

	ul_random_num = (ul_random_num << 16) + mem.u32StartUpCounter;

	/* Initialize seed */
	srand(ul_random_num);

	LOG_PLATFORM(("RANDOM SEED: %d\r\n", ul_random_num));
#endif
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
uint16_t platform_random_16(void)
{
	uint16_t us_random_num;

#if (SAM4C || SAM4CP || SAM4CM || SAME70 || PIC32CX)
	while ((trng_get_interrupt_status(TRNG) & TRNG_ISR_DATRDY) != TRNG_ISR_DATRDY) {
	}
	us_random_num = (uint16_t)(trng_read_output_data(TRNG));
#else
	us_random_num = rand() & 0xFF;
	us_random_num = (us_random_num << 8) | (rand() & 0xFF);
#endif
	return us_random_num;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
uint32_t platform_random_32(void)
{
	uint32_t ul_random_num;

#if (SAM4C || SAM4CP || SAM4CM || SAME70 || PIC32CX)
	while ((trng_get_interrupt_status(TRNG) & TRNG_ISR_DATRDY) != TRNG_ISR_DATRDY) {
	}
	ul_random_num = trng_read_output_data(TRNG);
#else
	ul_random_num = rand() & 0xFF;
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);
#endif
	return ul_random_num;
}

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************   EUI64 AND STORAGE  ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_init_storage(void)
{
#if defined(CONF_STORAGE_AT45DBX_FLASH)
	at45db32_init();
	/* Wait for at45db32 to be ready after initialization */
	at45db32_wait_is_ready();
#endif
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
bool platform_write_storage(uint32_t u32Length, const void *pData)
{
	bool bRet = false;

	Disable_global_interrupt();

	/* Write to flash */
#ifdef CONF_STORAGE_INTERNAL_FLASH
	uint32_t u32AlignedLen;
	/* Get length in multiples of 8 bytes  */
	u32AlignedLen = u32Length >> 3;
	if (u32Length & 0x07) {
		u32AlignedLen++;
	}
	u32AlignedLen <<= 3;

	/* unlocks the needed space depending on "data_len" */
	flash_unlock((uint32_t)ADDR_APP_DATA, (uint32_t)ADDR_APP_DATA + u32AlignedLen, 0, 0);

	/* Writes "data_len" bytes from "data_buffer" to flash */
	bRet = (flash_write((uint32_t)ADDR_APP_DATA, pData, u32AlignedLen, 0) == FLASH_RC_OK);
#endif

#ifdef CONF_STORAGE_AT45DBX_FLASH
	/* Wait for at45db32 to be ready */
	at45db32_wait_is_ready();

	/* Write data to mem through buf 1*/
	bRet = at45db32_send_cmd(AT45DBX_PR_PAGE_TH_BUF1, EXT_ADDR_APP_DATA_PAGE, (uint8_t *)pData, (uint16_t)u32Length);
#endif

#ifdef CONF_STORAGE_USER_SIGNATURE
	/* Write to User Signature */
	memcpy(&auc_user_sig_container[G3CFG_OFFSET_USER_SIGN], pData, u32Length);
	platform_write_user_signature(&auc_user_sig_container);
	bRet = true;
#endif

	Enable_global_interrupt();

	return bRet;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
bool platform_read_storage(uint32_t u32Length, void *pData)
{
	bool bRet = false;
	Disable_global_interrupt();

#ifdef CONF_STORAGE_INTERNAL_FLASH
	/* Read from flash (direct operation): reads "data_len" bytes from flash to "data_buffer" */
	memcpy(pData, (uint8_t *)ADDR_APP_DATA, u32Length);
	bRet = true;
#endif

#ifdef CONF_STORAGE_AT45DBX_FLASH
	/* Wait for at45db32 to be ready */
	at45db32_wait_is_ready();
	/* Read data from memory */
	bRet = at45db32_send_cmd(AT45DBX_RD_PAGE, EXT_ADDR_APP_DATA_PAGE, (uint8_t *)pData, (uint16_t)u32Length);
#endif

#ifdef CONF_STORAGE_USER_SIGNATURE
	/* Read from User Signature */
	platform_read_user_signature(&auc_user_sig_container);
	memcpy(pData, &auc_user_sig_container[G3CFG_OFFSET_USER_SIGN], u32Length);
	bRet = true;
#endif

	Enable_global_interrupt();

	return bRet;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_erase_storage(uint32_t u32Length)
{
	Disable_global_interrupt();

#ifdef CONF_STORAGE_INTERNAL_FLASH
	uint32_t u32AlignedLen;
	/* Get length in multiples of 8 bytes  */
	u32AlignedLen = u32Length >> 3;
	if (u32Length & 0x07) {
		u32AlignedLen++;
	}
	u32AlignedLen <<= 3;

	/* unlocks the needed space depending on "data_len" */
	flash_unlock((uint32_t)ADDR_APP_DATA, (uint32_t)ADDR_APP_DATA + u32AlignedLen, 0, 0);

	/* Erases the region for the persistent data (the minimum erasable size is 16 pages) */
	flash_erase_page((uint32_t)ADDR_APP_DATA, IFLASH_ERASE_PAGES_16);
#endif

#ifdef CONF_STORAGE_AT45DBX_FLASH
	UNUSED(u32Length);

	/* Wait for at45db32 to be ready */
	at45db32_wait_is_ready();

	/* Erase page */
	at45db32_send_cmd(AT45DBX_ER_PAGE, EXT_ADDR_APP_DATA_PAGE, 0, 0);

	/* Wait for at45db32 to be ready */
	at45db32_wait_is_ready();
#endif

#ifdef CONF_STORAGE_USER_SIGNATURE
	UNUSED(u32Length);

	/* Erase User Signature */
	platform_erase_user_signature();
#endif

	Enable_global_interrupt();
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_init_eui64(uint8_t *eui64)
{
#ifdef CONF_EUI64_FROM_USER_SIGNATURE
	uint8_t uc_i;
	bool b_mac_update;

	/* Set default value */
	eui64[0] = 0x31;
	eui64[1] = 0x30;
	eui64[2] = 0x36;
	eui64[3] = 0x33;
	eui64[4] = 0x4C;
	eui64[5] = 0x50;
	eui64[6] = 0x54;
	eui64[7] = 0x41;

	b_mac_update = false;

	if (b_mac_update) {
		uc_eui64_buf[0] = MAC_CONFIG_KEY_0;
		uc_eui64_buf[1] = MAC_CONFIG_KEY_1;
		uc_eui64_buf[2] =  0x41; /* A */
		uc_eui64_buf[3] =  0x54; /* T */
		uc_eui64_buf[4] =  0x50; /* P */
		uc_eui64_buf[5] =  0x4C; /* L */
		uc_eui64_buf[6] =  0x30; /* 0 */
		uc_eui64_buf[7] =  0x30; /* 0 */
		uc_eui64_buf[8] =  0x30; /* 0 */
		uc_eui64_buf[9] =  0x30; /* 0 */

		/* Read User Signature */
		platform_read_user_signature(&auc_user_sig_container);
		/* Copy EUI64 value */
		memcpy(&auc_user_sig_container[MACCFG_OFFSET_USER_SIGN], uc_eui64_buf, sizeof(uc_eui64_buf));
		/* Erase User Signature */
		platform_erase_user_signature();
		/* Write User Signature */
		platform_write_user_signature(&auc_user_sig_container);
	}

	LOG_PLATFORM(("Look for EUI64 previously configured in User Signature\r\n"));

	/* Read from User Signature */
	platform_read_user_signature(&auc_user_sig_container);
	memcpy(uc_eui64_buf, &auc_user_sig_container[MACCFG_OFFSET_USER_SIGN], sizeof(uc_eui64_buf));
	/* Check eui64 integrity */
	if (((uc_eui64_buf[0] == MAC_CONFIG_KEY_0) && (uc_eui64_buf[1] == MAC_CONFIG_KEY_1)) ||
			((uc_eui64_buf[1] == MAC_CONFIG_KEY_0) && (uc_eui64_buf[0] == MAC_CONFIG_KEY_1))) {
		/* Valid EUI64 address found, overwrite parameter */
		for (uc_i = 0; uc_i < EUI64_SIZE; uc_i++) {
			eui64[EUI64_SIZE - 1 - uc_i] = uc_eui64_buf[MAC_CONFIG_KEY_SIZE + uc_i];
		}
		LOG_PLATFORM(("Overwriting EUI64 with value found in User Signature "));
	} else {
		/* Else, do not overwrite parameter */
		LOG_PLATFORM(("No EUI64 found, using default "));
	}
#endif

#ifdef CONF_EUI64_FROM_CHIP_ID
	uint8_t auc_chip_id_container[16]; /* 128 bits */
	uint32_t res;

	LOG_PLATFORM(("Get EUI64 from Chip ID\r\n"));

	/* Read from Chip ID */
	res = platform_read_chip_id(&auc_chip_id_container);
	if (res == FLASH_RC_OK) {
		eui64[7] = auc_chip_id_container[4];
		eui64[6] = auc_chip_id_container[5];
		eui64[5] = auc_chip_id_container[6];
		eui64[4] = auc_chip_id_container[7];
		eui64[3] = (auc_chip_id_container[8] << 4) | (auc_chip_id_container[9] & 0x0F);
		eui64[2] = (auc_chip_id_container[10] << 4) | (auc_chip_id_container[11] & 0x0F);
		eui64[1] = (auc_chip_id_container[12] << 4) | (auc_chip_id_container[13] & 0x0F);
		eui64[0] = (auc_chip_id_container[14] << 4) | (auc_chip_id_container[15] & 0x0F);
		LOG_PLATFORM(("Overwriting EUI64 with Chip ID "));
	} else {
		/* Else, do not overwrite parameter */
		LOG_PLATFORM(("Could not read Chip ID, using default "));
	}
#endif

	LOG_PLATFORM(("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
			eui64[7], eui64[6], eui64[5], eui64[4], eui64[3], eui64[2], eui64[1], eui64[0]));
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
uint32_t platform_read_chip_id(void *chip_id_buf)
{
	return flash_read_unique_id((uint32_t *)chip_id_buf, CHIP_ID_SIZE);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_read_user_signature(void *user_sign_buf)
{
#if PIC32CX
	flash_read_user_signature((uint64_t *)user_sign_buf, IFLASH_USR_SIG_BLOCK_0, 0, USER_SIG_PAGE_SIZE);
#else
	flash_read_user_signature((uint64_t *)user_sign_buf, USER_SIGNATURE_SIZE);
#endif
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_erase_user_signature(void)
{
#if PIC32CX
	flash_erase_user_signature(IFLASH_USR_SIG_BLOCK_0);
#else
	flash_erase_user_signature();
#endif
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
void platform_write_user_signature(void *user_sign_buf)
{
#if PIC32CX
	flash_write_user_signature((const void *)user_sign_buf, IFLASH_USR_SIG_BLOCK_0, 0, USER_SIG_PAGE_SIZE);
#else
	flash_write_user_signature((const void *)user_sign_buf, USER_SIGNATURE_SIZE);
#endif
}

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************ POWER DOWN DETECTION ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/**
 * Power Down detection will call a function when outage is detected.
 * Current G3 applications set this callback to a function in the
 * storage module, in order to save parameters on non-volatile memory.
 * User may use the callback to implement a message sending using the
 * Last Gasp feature defined in G3 Specification.
 *
 * It is up to the user to ensure that the platform used is capable
 * to maintain the MCU running long enough between Power Down detection
 * and complete shut down.
 * If this time is not long enough, some tasks may not be completed
 * by the callback function.
 */

#ifdef PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR

void SUPC_Handler(void)
{
	uint32_t ul_status;

#if PIC32CX
	ul_status = supc_get_interrupt_status(SUPC);
#else
	ul_status = supc_get_status(SUPC);
#endif
	UNUSED(ul_status);

	if (pf_pdd_callback != NULL) {
		/* Launch user-defined PDD callback */
		pf_pdd_callback();
	}
}

#endif /* PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR */

#ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)

/**
 * \brief Read converted data through PDC channel.
 *
 * \param p_adc The pointer of adc peripheral.
 * \param p_s_buffer The destination buffer.
 * \param ul_size The size of the buffer.
 */
static uint32_t adc_read_buffer(Adc *p_adc, uint16_t *p_s_buffer, uint32_t ul_size)
{
	/* Check if the first PDC bank is free. */
	if ((p_adc->ADC_RCR == 0) && (p_adc->ADC_RNCR == 0)) {
		p_adc->ADC_RPR = (uint32_t)p_s_buffer;
		p_adc->ADC_RCR = ul_size;
		p_adc->ADC_PTCR = ADC_PTCR_RXTEN;

		return 1;
	} else { /* Check if the second PDC bank is free. */
		if (p_adc->ADC_RNCR == 0) {
			p_adc->ADC_RNPR = (uint32_t)p_s_buffer;
			p_adc->ADC_RNCR = ul_size;

			return 1;
		} else {
			return 0;
		}
	}
}

/**
 * \brief Interrupt handler for the ADC
 *        (Used for power down detection)
 */
#if SAMG55
void ADC_PDD_Handler(void);

void ADC_PDD_Handler(void)
#else
void ADC_Handler(void)
#endif
{
	static bool b_info_saved = true; /* True to avoid saving info in flash when the */
	/* board starts up with USB power supply. */

	/* Read the ADC value with a PDC transfer */
#if SAMG
	if (adc_get_interrupt_status(ADC) & (1 << PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY)) {
		us_adc_value = adc_channel_get_value(ADC, PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY);
#else
	if ((adc_get_status(ADC) & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF) {
		adc_read_buffer(ADC, &us_adc_value, PDD_BUFFER_SIZE);
		/* Only keep sample value, and discard channel number. */
		us_adc_value &= ADC_LCDR_LDATA_Msk;
#endif

		LOG_PLATFORM(("%d\r\n", us_adc_value));

		/* Check if Power Supply is under power down threshold */
		if (us_adc_value < PLATFORM_PDD_VDD_CRITICAL_THRESHOLD) {
			if (!b_info_saved) {
				if (pf_pdd_callback != NULL) {
					/* Launch user-defined PDD callback */
					pf_pdd_callback();
				}

				b_info_saved = true;
				LOG_PLATFORM(("Power down detected! Backup in flash done.\n"));
			}
		} else if (b_info_saved) { /* Power Supply level is in normal range */
			/* Configure flag so that info is saved when the next power down is detected */
			b_info_saved = false;
		}
	}
}

/**
 * \brief Configure to trigger ADC by TIOA output of timer.
 */
static void configure_time_trigger(void)
{
	uint32_t ul_div = 1;
	uint32_t ul_tc_clks = 0;
#if (PIC32CX)
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
#else
	uint32_t ul_sysclk = sysclk_get_peripheral_hz();
#endif
	uint32_t ul_f_obj = PLATFORM_PDD_POWER_SUPPLY_MONITORING_FREQ;

	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_TC_TRIG);

	/* TIOA configuration */
	gpio_configure_pin(PIN_TC_TIOA, PIN_TC_TIOA_FLAGS);

	/* Configure TC for a ul_f_obj frequency and trigger on RC compare. */
	tc_find_mck_divisor(ul_f_obj, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, TC_TRIG_CHN, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
	TC0->TC_CHANNEL[TC_TRIG_CHN].TC_RA = ((ul_sysclk / ul_div) / ul_f_obj) / 2;
	TC0->TC_CHANNEL[TC_TRIG_CHN].TC_RC = ((ul_sysclk / ul_div) / ul_f_obj) / 1;

	/* Start the Timer. */
	tc_start(TC0, TC_TRIG_CHN);

	/* Set TIOA0 trigger. */
#if SAMG
	adc_set_trigger(ADC, ADC_TRIG_TIO_CH);
#else
	adc_configure_trigger(ADC, ADC_TRIG_TIO_CH, 0);
#endif
}

#elif (SAMV71 || SAMV70 || SAME70)

/**
 * \brief Configure to trigger AFEC by TIOA output of timer.
 */
static void configure_time_trigger(void)
{
	uint32_t ul_div = 1;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_peripheral_hz();
	uint32_t ul_f_obj = PLATFORM_PDD_POWER_SUPPLY_MONITORING_FREQ;

	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_TC_TRIG);

	/* TIOA configuration */
	gpio_configure_pin(PIN_TC0_TIOA0, PIN_TC0_TIOA0_FLAGS);

	/* Configure TC for a ul_f_obj frequency and trigger on RC compare. */
	tc_find_mck_divisor(ul_f_obj, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, TC_TRIG_CHN, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
	TC0->TC_CHANNEL[TC_TRIG_CHN].TC_RA = ((ul_sysclk / ul_div) / ul_f_obj) / 2;
	TC0->TC_CHANNEL[TC_TRIG_CHN].TC_RC = ((ul_sysclk / ul_div) / ul_f_obj) / 1;

	/* Start the Timer */
	tc_start(TC0, TC_TRIG_CHN);

	/* Set TIOA0 trigger for AFEC */
	afec_set_trigger(AFEC0, AFEC_TRIG_TIO_CH_0);
}

/**
 * \brief Interrupt handler for the ADC
 *        (Used for power down detection)
 */
void platform_pdd_handler(void)
{
	static bool b_info_saved = true; /* True to avoid saving info in flash when the */
	/* board starts up with USB power supply. */

	/* Read the AFEC value */
	AFEC0->AFEC_CSELR = 0;
	us_adc_value = AFEC0->AFEC_CDR;

	LOG_PLATFORM(("%d ", us_adc_value));

	/* Check if Power Supply is under power down threshold */
	if ((us_adc_value < PLATFORM_PDD_VDD_CRITICAL_THRESHOLD)) {
		if (!b_info_saved) {
			if (pf_pdd_callback != NULL) {
				/* Launch user-defined PDD callback */
				pf_pdd_callback();
			}

			b_info_saved = true;
			LOG_PLATFORM(("Power down detected! Backup in flash done.\n"));
		}
	} else if (b_info_saved) { /* Power Supply level is in normal range */
		/* Configure flag so that info is saved when the next power down is detected */
		b_info_saved = false;
	}
}

#endif

#endif /* #ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER */

#if defined(PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR) || defined(PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER)

/**
 * \brief Initialize HW necessary to detect Power Down.
 */
void platform_init_power_down_det(void)
{
#ifdef PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR
	/* Enable SUPC CLK */
	pmc_enable_periph_clk(ID_SUPC);
#if PIC32CX
	/* Set Voltage threshold */
	supc_set_monitor_threshold(SUPC, SUPC_SM_THRESHOLD_2v96);
	/* Set sampling period */
	supc_set_monitor_sampling_period(SUPC, SUPC_SMMR_VDD3V3SMSMPL_ALWAYS_ON);
	/* Configure and enable SUPC interrupt */
	supc_enable_monitor_interrupt(SUPC);
	NVIC_SetPriority((IRQn_Type)ID_SUPC, 0);
	NVIC_EnableIRQ((IRQn_Type)ID_SUPC);
	/* Enable Supply monitor */
	supc_enable_monitor(SUPC);
#else
	/* Set continuous monitoring (every SLCK cycle) */
	supc_set_monitor_sampling_period(SUPC, 0x00000100);
	/* Threshold set to 3.04 Volts */
	supc_set_monitor_threshold(SUPC, 0x0000000C);
	/* Configure monitor to raise interrupt */
	supc_enable_monitor_interrupt(SUPC);
	/* Configure and enable SUPC interrupt */
	NVIC_SetPriority((IRQn_Type)ID_SUPC, 0);
	NVIC_EnableIRQ((IRQn_Type)ID_SUPC);
#endif
#endif /* PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR */

#ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);

	/* Initialize ADC. */

	/*
	 * Formula: ADCClock = MCK / ( (PRESCAL+1) * 2 )
	 * For example, MCK = 64MHZ, PRESCAL = 4, then:
	 * ADCClock = 64 / ((4+1) * 2) = 6.4MHz;
	 */

#if SAMG
	struct adc_config adc_cfg;

	adc_enable();

	/* Get the ADC default configurations: */
	adc_get_config_defaults(&adc_cfg);

	/* Initialize the ADC Module: */
	adc_init(ADC, &adc_cfg);

	/* Set ADC callback for PDD */
	adc_set_callback(ADC, (enum adc_interrupt_source)PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY, ADC_PDD_Handler, 10);

	/* Enable Channel: */
	adc_channel_enable(ADC, PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY);

#else /* SAMG */

	/* Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 6.4MHz = 10 us
	 */
	adc_init(ADC, sysclk_get_peripheral_hz(), 6400000, ADC_STARTUP_TIME_4);

	/* Init ADC data value. */
	us_adc_value = 0;

	/* Set ADC timing. */
	adc_configure_timing(ADC, PDD_TRACKING_TIME);

	/* Enable channel number tag. */
	adc_enable_tag(ADC);

	/* Disable sequencer. */
	adc_stop_sequencer(ADC);

	/* Enable channels. */
	adc_enable_channel(ADC, PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY);

	/* Disable power save. */
	adc_configure_power_save(ADC, 0);
#endif /* SAMG */

	/* Transfer with PDC. */
	adc_read_buffer(ADC, &us_adc_value, PDD_BUFFER_SIZE);
	/* Enable PDC channel interrupt. */
#if SAMG55
	adc_enable_interrupt(ADC, (enum adc_interrupt_source)PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY);
#else
	adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
#endif

	/* Enable ADC interrupt. */
	NVIC_EnableIRQ(ADC_IRQn);

	/* Configure time trigger and start convertion. */
	configure_time_trigger();
#endif /* (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM) */

#if (SAMV71 || SAMV70 || SAME70)
	pmc_enable_periph_clk(ID_PIOD);

	afec_enable(AFEC0);

	struct afec_config afec_cfg;

	afec_get_config_defaults(&afec_cfg);

	afec_init(AFEC0, &afec_cfg);

	/* Configure AFEC channel to be triggered by the TC */
	configure_time_trigger();

	struct afec_ch_config afec_ch_cfg;

	afec_ch_get_config_defaults(&afec_ch_cfg);

	afec_ch_cfg.gain = AFEC_GAINVALUE_0;

	afec_ch_set_config(AFEC0,
			PLATFORM_PDD_AFEC_CHANNEL_POWER_SUPPLY,
			&afec_ch_cfg);

	afec_channel_set_analog_offset(AFEC0,
			PLATFORM_PDD_AFEC_CHANNEL_POWER_SUPPLY,
			0x200);

	/* Set PDD callback */
	afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_0, platform_pdd_handler, PLATFORM_PDD_PRIO);

	/* Enable channel for Vdd */
	afec_channel_enable(AFEC0, PLATFORM_PDD_AFEC_CHANNEL_POWER_SUPPLY);
#endif /* (SAMV71 || SAMV70 || SAME70) */
#endif /* PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER */

	/* Initialize PDD user-defined callback */
	pf_pdd_callback = NULL;
}

/**
 * \brief Set up Power Down Detector callback.
 *
 */
void platform_set_pdd_callback(void (*pf_user_callback)(void))
{
	pf_pdd_callback = pf_user_callback;
}

#endif /* defined(PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR) || defined(PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER) */

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************   HW TIMERS CONFIG   ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/**
 * \internal
 * \brief Timer interrupt handler.
 *
 */
void _platform_tc_1ms_handler(void)
{
	volatile uint32_t ul_dummy;
	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC_1MS, TC_1MS_CHN);
	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/* Execute user 1ms timer callback, if defined */
	if (pf_platform_ms_timer_callback != NULL) {
		pf_platform_ms_timer_callback();
	}

	if (ul_call_process_count_ms != 0) {
		ul_call_process_count_ms--;
		if (ul_call_process_count_ms == 0) {
			/* Signal call to process */
			uc_flag_call_process = 1;
			ul_call_process_count_ms = ul_call_process_count_ms_cfg;
		}
	}

	if (ul_call_app_process_count_ms != 0) {
		ul_call_app_process_count_ms--;
		if (ul_call_app_process_count_ms == 0) {
			/* Signal call to process */
			uc_flag_call_app_process = 1;
			ul_call_app_process_count_ms = ul_call_app_process_count_ms_cfg;
		}
	}

	if (!ul_led_count_ms--) {
		ul_led_count_ms = ul_led_count_ms_cfg;
		uc_led_swap = 1;
	}
}

/**
 * \brief Set up 1ms timer.
 *
 */
void platform_init_1ms_timer(void)
{
	uint32_t ul_div, ul_tcclks;
	uint32_t ul_sysclk = 0;

	/* Configure PMC */
	pmc_enable_periph_clk(ID_TC_1MS);

	ul_tcclks = 2;

#if (!PIC32CX)
	ul_sysclk = sysclk_get_peripheral_hz();
	/* MCK = ul_sysclk -> tcclks = 2 : TCLK3 = MCK/32 Hz -> ul_div = (MCK/32)/1000 to get 1ms timer */
	ul_div = ul_sysclk / 32000;
	tc_init(TC_1MS, TC_1MS_CHN, ul_tcclks | TC_CMR_CPCTRG);
#else
	ul_sysclk = sysclk_get_cpu_hz();
	/* MCK = ul_sysclk -> tcclks = 2 : TCLK3 = MCK/32 Hz -> ul_div = (MCK/32)/1000 to get 1ms timer */
	ul_div = ul_sysclk / 32000;
	tc_init(TC_1MS, TC_1MS_CHN, ul_tcclks | TC_CMR_CPCTRG, 0);
#endif

	tc_write_rc(TC_1MS, TC_1MS_CHN, ul_div);

	/* Configure and enable interrupt on RC compare */
	NVIC_SetPriority((IRQn_Type)ID_TC_1MS, 0);
	NVIC_EnableIRQ((IRQn_Type)ID_TC_1MS);
	tc_enable_interrupt(TC_1MS, TC_1MS_CHN, TC_IER_CPCS);

	/* Start the timer. TC1, chanel 0 = TC3 */
	tc_start(TC_1MS, TC_1MS_CHN);
}

/**
 * \brief Set up LED blinking count (ms).
 *
 */
void platform_set_ms_callback(void (*pf_user_callback)(void))
{
	pf_platform_ms_timer_callback = pf_user_callback;
}

/**
 * \brief Set up LED blinking count (ms).
 *
 */
void platform_led_cfg_blink_rate(uint32_t ul_count_cfg)
{
	ul_led_count_ms_cfg = ul_count_cfg;
}

/**
 * \brief Update LED status according to the ms timer.
 *
 */
void platform_led_update(void)
{
	if (uc_led_swap) {
		uc_led_swap = 0;
#if (BOARD == SAM4CMP_DB || BOARD == SAM4CMS_DB)
		LED_Toggle(LED4);
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
	#ifdef CONF_BOARD_LCD_EN
		if (b_led_swap_on) {
			cl010_clear_icon(CL010_ICON_PHASE_1);
			b_led_swap_on = false;
		} else {
			cl010_show_icon(CL010_ICON_PHASE_1);
			b_led_swap_on = true;
		}
	#endif
#else
		LED_Toggle(LED0);
#endif
	}
}

/**
 * \brief Update LED status according to raising interrupt.
 *
 */
void platform_led_int_toggle(void)
{
#if (BOARD == SAM4CMP_DB || BOARD == SAM4CMS_DB || BOARD == PIC32CXMTSH_DB || BOARD == SAMG55_XPLAINED_PRO || BOARD == PIC32CXMTC_DB)
	/* Do nothing */
#else
	LED_Toggle(LED1);
#endif
}

/**
 * \brief Enable LED relative to raising interrupt.
 *
 */
void platform_led_int_on(void)
{
#if (BOARD == SAM4CMP_DB || BOARD == SAM4CMS_DB || BOARD == PIC32CXMTSH_DB || BOARD == SAMG55_XPLAINED_PRO || BOARD == PIC32CXMTC_DB)
	/* Do nothing */
#else
	LED_On(LED1);
#endif
}

/**
 * \brief Disable LED relative to raising interrupt.
 *
 */
void platform_led_int_off(void)
{
#if (BOARD == SAM4CMP_DB || BOARD == SAM4CMS_DB || BOARD == PIC32CXMTSH_DB || BOARD == SAMG55_XPLAINED_PRO || BOARD == PIC32CXMTC_DB)
	/* Do nothing */
#else
	LED_Off(LED1);
#endif
}

/**
 * \brief Configure the number of milliseconds to wait before consecutive
 *        calls to the stack process.
 */
void platform_cfg_call_process_rate(uint32_t ul_user_call_process_rate)
{
	ul_call_process_count_ms_cfg = ul_user_call_process_rate;
	ul_call_process_count_ms = ul_user_call_process_rate;
}

/**
 * \brief Configure the number of milliseconds to wait before consecutive
 *        calls to the application process.
 */
void platform_cfg_call_app_process_rate(uint32_t ul_user_call_app_process_rate)
{
	ul_call_app_process_count_ms_cfg = ul_user_call_app_process_rate;
	ul_call_app_process_count_ms = ul_user_call_app_process_rate;
}

/**
 * \brief Update call process flag according to the ms timer.
 *
 */
uint8_t platform_flag_call_process(void)
{
	uint8_t uc_result = 0;

	if (uc_flag_call_process) {
		uc_result = 1;
		uc_flag_call_process = 0;
	}

	return(uc_result);
}

/**
 * \brief Update call process flag according to the ms timer.
 *
 */
uint8_t platform_flag_call_app_process(void)
{
	uint8_t uc_result = 0;

	if (uc_flag_call_app_process) {
		uc_result = 1;
		uc_flag_call_app_process = 0;
	}

	return(uc_result);
}

/**********************************************************************************************************************/
/*********************************************** ---------------------- ***********************************************/
/*********************************************** GENERAL INITIALIZATION ***********************************************/
/*********************************************** ---------------------- ***********************************************/
/**********************************************************************************************************************/

/**
 *  Configure UART console.
 */
/* [main_console_configure] */
static void configure_dbg_console(void)
{
#ifdef CONF_BOARD_UDC_CONSOLE
	/* Configure console UDC (USB). */
#if SAMG55
	stdio_udc_init(UDP, (void *)usb_wrp_udc_putchar, (void *)usb_wrp_udc_getchar, (void *)usb_wrp_udc_start);
#else
	stdio_udc_init(USBHS, (void *)usb_wrp_udc_putchar, (void *)usb_wrp_udc_getchar, (void *)usb_wrp_udc_start);
#endif
#else
#ifdef CONF_UART
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONF_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
#endif
#endif
}

/**
 * \brief Configure the hardware.
 */
void platform_init_hw(void)
{
#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
	status_code_t status;
#endif

	/* ASF function to setup clocking. */
	sysclk_init();

	/* Ensure all priority bits are assigned as preemption priority bits. */
	NVIC_SetPriorityGrouping(__NVIC_PRIO_BITS);

	/* Atmel library function to setup for the evaluation kit being used. */
	board_init();

#if PIC32CX
#if defined(CONF_EUI64_FROM_USER_SIGNATURE) || defined(CONF_STORAGE_USER_SIGNATURE)
	/* Disable Write Protection */
	efc_disable_write_protection(SEFC0);

	/* Unlock Key Bus */
	efc_unlock_key_bus_transfer(SEFC0);

	/* Set User Signature Rights */
	efc_set_usr_rights(SEFC0, EEFC_USR_WRENUSB1 | EEFC_USR_RDENUSB1);
#endif
#endif

#if (SAME70) && defined(CONF_BOARD_SDRAMC)
	/* Initialize SDRAM controller */
	sysclk_enable_peripheral_clock(ID_SDRAMC);
	matrix_set_nandflash_cs(CCFG_SMCNFCS_SDRAMEN);
	sdramc_init((sdramc_memory_dev_t *)&SDRAM_IS42S16400J_7B2LI, sysclk_get_peripheral_hz());
	sdram_enable_unaligned_support();
#endif

#ifdef PLATFORM_LCD_SIGNALLING_ENABLE
#if (BOARD == ATPL360AMB)
	status = (status_code_t)c0216CiZ_init();
	if (status != STATUS_OK) {
		/* puts("-- LCD Initialization fails! --\r\n"); */
		while (1) {
		}
	}

	c0216CiZ_set_cursor(C0216CiZ_LINE_UP, 0);
	c0216CiZ_show((const char *)"ATPL360AMB    G3");
	c0216CiZ_set_cursor(C0216CiZ_LINE_DOWN, 0);
	c0216CiZ_show((const char *)PLATFORM_LCD_TEXT);
#elif (BOARD == ATPL360MB)
	status = (status_code_t)c0216CiZ_init();
	if (status != STATUS_OK) {
		/* puts("-- LCD Initialization fails! --\r\n"); */
		while (1) {
		}
	}

	c0216CiZ_set_cursor(C0216CiZ_LINE_UP, 0);
	c0216CiZ_show((const char *)"ATPL360MB    G3");
	c0216CiZ_set_cursor(C0216CiZ_LINE_DOWN, 0);
	c0216CiZ_show((const char *)PLATFORM_LCD_TEXT);
#elif (BOARD == ATPL360ASB)
	/* Initialize the vim878 LCD glass component. */
	status = vim878_init();
	if (status != STATUS_OK) {
		/* puts("-- LCD Initialization fails! --\r\n"); */
		while (1) {
		}
	}

	vim878_set_contrast(1);
	vim878_clear_all();
	vim878_show_text((const uint8_t *)PLATFORM_LCD_TEXT);
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
	/* Initialize the CL010 LCD glass component. */
	status = cl010_init();
	if (status != STATUS_OK) {
		printf("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	cl010_clear_all();
	cl010_show_icon(CL010_ICON_MICROCHIP);
	cl010_show_numeric_string(CL010_LINE_DOWN, (const uint8_t *)"0000360");
	cl010_show_icon(CL010_ICON_DOT_5);
	cl010_show_icon(CL010_ICON_P_PLUS);
	cl010_show_icon(CL010_ICON_P_MINUS);
#else
	(void)status;
#       error No known Display configured
#endif
#endif

	/* Configure Console over UART */
	configure_dbg_console();
}
