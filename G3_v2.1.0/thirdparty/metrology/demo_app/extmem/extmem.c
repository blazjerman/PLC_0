/**
 * \file
 *
 * \brief External Memory Handler
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

#include "matrix.h"
#include "sst26vfxxx.h"
#include "extmem.h"

#include "command.h"
#include "metrology.h"
#include "event.h"
#include "tou.h"
#include "energy.h"
#include "history.h"
#include "demand.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

static struct qspid_t s_qspid = {QSPI, 0, 0, 0};
static struct qspi_config_t s_mode_config = {QSPI_MEM_MODE, false, QSPI_LASTXFER, 0, 0, 0, 0, 0, 0, false, false, false};
static uint16_t sus_mem_reg_size[MEM_REG_IDS];

#if BOARD==PIC32CXMTSH_DB
/* JEDEC-ID of SST26VF memory assembled on PIC32CXMTSH board */
static uint8_t s_jedec_id[3] = {0xBF, 0x26, 0x41};
#elif BOARD==PIC32CXMTC_DB
/* JEDEC-ID of SST26VF memory assembled on PIC32CXMTC board */
static uint8_t s_jedec_id[3] = {0xBF, 0x26, 0x43};
#endif

static uint32_t _extmem_id_to_address(extmemreg_t mem_reg_id)
{
	uint32_t address = 0;
	extmemreg_t idx;

	for (idx = MEM_REG_COMMS_ID; idx < MEM_REG_IDS; idx++) {
		if (idx == mem_reg_id) {
			break;
		}
		address += ((sus_mem_reg_size[idx] >> 12) + 1);
	}

	address <<= 12;

	return address;
}

/**
 * \brief Init external memory.
 *
 * \return Size of the read data, 0 on ERROR.
 */
status_code_t ExtMemInit(void)
{
	status_code_t status;
	uint8_t uc_register;
	uint8_t jedec_id_tmp[3] = {0};

	/* Configure Peripheral CLK and Generic CLK */
	pmc_configure_generic(ID_QSPI, PMC_PCR_GCLKCSS_PLLACK1, PMC_PCR_GCLKDIV(99));  /* 2 MHz */

	/* Enable Generick Clock */
	pmc_enable_generic_clk(ID_QSPI);
	/* Enable Peripheral clock */
	pmc_enable_periph_clk(ID_QSPI);

	/* Configure MATRIX to provide access to QSPI in mem mode */
	/* SST26VF016B(16Mbit) : 2 MBytes*/
	matrix_set_slave_protected_region_top(SLAVE_MATRIX1_ID_QSPI, MATRIX_SIZE_2MB);

	/* QSPI memory mode configure */
	status = sst26vfxxx_initialize(s_qspid.qspi_hw, &s_mode_config, 1);
	if (status != STATUS_OK) {
		return ERR_UNSUPPORTED_DEV;
	}

	/* Reset SST26VF memory */
	sst26vfxxx_reset(&s_qspid);

	/* Read JEDEC-ID */
	sst26vfxxx_read_jedec_id(&s_qspid, jedec_id_tmp);

	/* Write SST26VF configuration register */
	uc_register = SSTV26FXXX_STATUS_IOC_DISABLED | SSTV26FXXX_STATUS_WPEN_DISABLED;
	sst26vfxxx_write_configuration(&s_qspid, uc_register);

	/* Check JEDEC-ID */
	sst26vfxxx_read_jedec_id(&s_qspid, jedec_id_tmp);
	if (memcmp(jedec_id_tmp, s_jedec_id, 3) != 0) {
		return ERR_UNSUPPORTED_DEV;
	}

	/* Global Protection Unlock */
	sst26vfxxx_global_protection_unlock(&s_qspid);

	/* Init Region sizes */
	sus_mem_reg_size[MEM_REG_COMMS_ID] = sizeof(command_t);
	sus_mem_reg_size[MEM_REG_METROLOGY_ID] = sizeof(metrology_t);
	sus_mem_reg_size[MEM_REG_EVENTS_ID] = sizeof(events_t);
	sus_mem_reg_size[MEM_REG_ENERGY_ID] = sizeof(energy_t);
	sus_mem_reg_size[MEM_REG_TOU_ID] = sizeof(tou_t);
	sus_mem_reg_size[MEM_REG_DEMAND_ID] = sizeof(demand_t);
	sus_mem_reg_size[MEM_REG_HISTORY_ID] = sizeof(history_t);
	sus_mem_reg_size[MEM_REG_RTC_ID] = sizeof(rtc_t);

	return STATUS_OK;
}

/**
 * \brief Get UNIQUE ID from the external memory.
 *
 * \param ptr_data     Pointer to a data buffer to store UNIQUE ID.
 * \param len          Length in bytes if the Unique ID.
 *
 * \return Size of the UNIQUE ID.
 */
uint8_t ExtMemGetUniqueId(uint8_t *puc_data, uint8_t len)
{
	uint8_t uc_size, i;
	uint8_t tmp_buf[8];

	uc_size = sst26vfxxx_read_unique_id(&s_qspid, tmp_buf);

	/* Protect against memory corruption */
	if (uc_size > len) {
		uc_size = len;
	}

	/* Adapt to printable data */
	for (i = 0; i < uc_size; i++, puc_data++) {
		*puc_data = tmp_buf[i];
		if (*puc_data < 0x20) {
			*puc_data = *puc_data + 30;
		}
		if (*puc_data > 0x7D) {
			*puc_data = *puc_data - 30;
		}
	}

	return uc_size;
}

/**
 * \brief Get data from the external memory.
 *
 * \param mem_reg_id   Identify the region to read data.
 * \param ptr_data     Pointer to a data structure to store data.
 *
 * \return Size of the read data, 0 on ERROR.
 */
uint16_t ExtMemRead(extmemreg_t mem_reg_id, void *ptr_data)
{
	uint32_t ul_addr;
	uint16_t us_size;

	ul_addr = _extmem_id_to_address(mem_reg_id);
	us_size = sus_mem_reg_size[mem_reg_id];

	sst26vfxxx_read_quad(&s_qspid, (uint8_t *)ptr_data, us_size, ul_addr, 0);

	return us_size;
}

/**
 * \brief Get data from the external memory.
 *
 * \param mem_reg_id   Identify the region to read data.
 * \param ptr_data     Pointer to a data structure to store data.
 * \param size         Size in bytes of the data to read.
 *
 * \return Size of the read data, 0 on ERROR.
 */
uint16_t ExtMemReadSize(extmemreg_t mem_reg_id, void *ptr_data, uint16_t size)
{
	uint32_t ul_addr;

	ul_addr = _extmem_id_to_address(mem_reg_id);

	sst26vfxxx_read_quad(&s_qspid, (uint8_t *)ptr_data, size, ul_addr, 0);

	return size;
}

/**
 * \brief Set data to the external memory. Important: Full sector will be erased
 * before write command.
 *
 * \param mem_reg_id   Identify the region to write data.
 * \param ptr_data     Pointer to a data structure to write in external mem.
 *
 * \return Size of the write data, 0 on ERROR.
 */
uint16_t ExtMemWrite(extmemreg_t mem_reg_id, void *ptr_data)
{
	uint32_t ul_addr;
	uint16_t us_size;
	uint8_t uc_num_sectors, i;

	ul_addr = _extmem_id_to_address(mem_reg_id);
	us_size = sus_mem_reg_size[mem_reg_id];

	uc_num_sectors = us_size >> 12;
	if (us_size % 4096) {
		uc_num_sectors++;
	}

	for (i=0; i < uc_num_sectors; i++) {
		/* Erase next sector */
		if (sst26vfxxx_erase_sector(&s_qspid, ul_addr + (i << 12)) != SSTV26FXXX_SUCCESS) {
			return 0;
		}
	}

	sst26vfxxx_write_quad(&s_qspid, (uint8_t *)ptr_data, us_size, ul_addr, 0, 0);

	return us_size;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
