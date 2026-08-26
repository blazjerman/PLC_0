/**
 * \file
 *
 * \brief QSPI flash memory driver for SSTV26FXXX.
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
 * \defgroup group_common_components_memory_qspi_flash_sst26vfxxx QSPI Flash SSTV26FXXX Series
 *
 * Low-level driver for the SSTV26FXXX Series QSPI Flash controller. This driver provides access to the main
 * features of the SSTV26FXXX Series QSPI Flash.
 *
 * \{
 */

#include <board.h>
#include <assert.h>
#include "stdlib.h"
#include "string.h"
#include "sst26vfxxx.h"
#include "delay.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

struct qspi_inst_frame_t *dev_instance;
struct qspi_inst_frame_t *mem_instance;

#define PAGE_SIZE       256

static uint16_t _get_size_first_fragment(uint32_t address, uint16_t size)
{
	uint16_t size_frag;

	if (address & 0x000000FF) {
		if (size > PAGE_SIZE) {
			size_frag = PAGE_SIZE - (address & 0xFF);
		} else {
			size_frag = size - (address & 0xFF);
		}
	} else {
		if (size > PAGE_SIZE) {
			size_frag = PAGE_SIZE;
		} else {
			size_frag = size;
		}
	}

	return size_frag;
}

/**
 * \brief Initialize QSPI serial memory mode.
 * \param qspi                      Pointer to an SST26VF qspid_t struct.
 * \param mode_config               Configure settings to config qspid.
 * \param use_default_config Config QSPI use default configures.
 * \return status SST26VFXXX initialize result.
 */
enum status_code sst26vfxxx_initialize(Qspi *qspi, struct qspi_config_t *mode_config, uint32_t use_default_config)
{
	enum status_code status = STATUS_OK;

	dev_instance = (struct qspi_inst_frame_t *)malloc(sizeof(struct qspi_inst_frame_t));
	memset(dev_instance, 0, sizeof(struct qspi_inst_frame_t));

	mem_instance = (struct qspi_inst_frame_t *)malloc(sizeof(struct qspi_inst_frame_t));
	memset(mem_instance, 0, sizeof(struct qspi_inst_frame_t));

	if (use_default_config) {
		qspi_get_config_default(mode_config);
	}

	if ((mode_config->serial_memory_mode == QSPI_MEM_MODE) &&
		((mode_config->clock_phase == 1) || (mode_config->clock_polarity == 1))) {
			return ERR_INVALID_ARG;

	}

	status = qspi_initialize(qspi, mode_config);
	return status;
}

/**
 * \brief Send command to the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param instr  Instruction to be execute.
 * \param tx_data  Data buffer to send data.
 * \param rx_data  Data buffer to receive data.
 * \param read_write  Read/write access.
 * \param size  Data size to be read/write.
 */
static void sst26vfxxx_exec_command(struct qspid_t *qspid, uint8_t instr, uint8_t *tx_data,
		uint8_t *rx_data, enum qspi_access read_write, uint32_t size)
{
	qspid->qspi_command.instruction = instr;
	dev_instance->inst_frame.bm.b_inst_en = 1;
	qspid->qspi_frame = dev_instance;
	qspid->qspi_buffer.data_tx = tx_data;
	qspid->qspi_buffer.data_rx = rx_data;

	/* Configure Registers/Commands Access */
	dev_instance->inst_frame.bm.b_tfr_type = 0;
	dev_instance->inst_frame.bm.b_smrm = 1;

	if (read_write == QSPI_CMD_ACCESS) {
		/* Registers/Commands Access */
		if (size) {
			dev_instance->inst_frame.bm.b_data_en = 1;
			if (tx_data) {
				dev_instance->inst_frame.bm.b_apb_tfr_type = 0;
				qspid->qspi_buffer.tx_data_size = size;
			} else if (rx_data) {
				dev_instance->inst_frame.bm.b_apb_tfr_type = 1;
				qspid->qspi_buffer.rx_data_size = size;
			} else {
				/* ERROR in paramters */
				return;
			}
		} else {
			dev_instance->inst_frame.bm.b_data_en = 0;
			dev_instance->inst_frame.bm.b_apb_tfr_type = 0;
		}
	} else if (read_write == QSPI_WRITE_ACCESS) {
		dev_instance->inst_frame.bm.b_apb_tfr_type = 0;
		dev_instance->inst_frame.bm.b_data_en = 1;
		qspid->qspi_buffer.tx_data_size = size;
	} else {
		dev_instance->inst_frame.bm.b_apb_tfr_type = 1;
		dev_instance->inst_frame.bm.b_data_en = 1;
		qspid->qspi_buffer.rx_data_size = size;
	}

	qspi_flash_execute_command(qspid, read_write);
}

/**
 * \brief Read/write data from serial flash memory.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct..
 * \param instr Instruction to be execute.
 * \param addr  Address to send frame.
 * \param tx_data  Data buffer to send data.
 * \param rx_data  Data buffer to receive data.
 * \param read_write  Read/write access.
 * \param size  Data size to be read/write.
 * \param secure  Enable or disable scramble on QSPI.
 * \param aes_en  Enable or disable AES on QSPI.
 */
static void sst26vfxxx_memory_access(struct qspid_t *qspid, uint8_t instr, uint32_t addr,
		uint8_t *tx_data, uint8_t *rx_data, enum qspi_access read_write,
		uint32_t size, uint8_t secure, uint8_t aes_en)
{
	qspid->qspi_command.instruction = instr;
	qspid->qspi_buffer.data_tx = tx_data;
	qspid->qspi_buffer.data_rx = rx_data;
	mem_instance->addr = addr;
	mem_instance->inst_frame.bm.b_inst_en = 1;
	mem_instance->inst_frame.bm.b_data_en = 1;
	mem_instance->inst_frame.bm.b_addr_en = 1;
	mem_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
	qspid->qspi_frame = mem_instance;

	/* Configure memory Array Access */
	mem_instance->inst_frame.bm.b_tfr_type = 1;
	mem_instance->inst_frame.bm.b_smrm = 0;

	if (read_write == QSPI_WRITE_ACCESS) {
		qspid->qspi_buffer.tx_data_size = size;
	} else {
		qspid->qspi_buffer.rx_data_size = size;
	}

	qspi_flash_access_memory(qspid, read_write, secure, aes_en);
}

/**
 * \brief sst26vfxxx reset.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_reset(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_RESET_ENABLE, 0, 0, QSPI_CMD_ACCESS, 0);
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_RESET_MEMORY, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Enables QUAD I/O
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_enable_quad_mode(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_ENABLE_QUAD_IO, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Disables QUAD I/O
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_disable_quad_mode(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_RESET_EQUAD_IO, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief sst26vfxxx reset Quad I/O.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_quad_reset(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_RESET_EQUAD_IO, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Reads and returns the status register of the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \return QSPI status register
 */
uint8_t sst26vfxxx_read_status(struct qspid_t *qspid)
{
	uint8_t status;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_STATUS, 0, &status, QSPI_CMD_ACCESS, 1);
	return status;
}

/**
 * \brief Reads and returns the status register of the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \return QSPI status1
 */
void sst26vfxxx_write_configuration(struct qspid_t *qspid, uint8_t config)
{
	uint8_t value[2];

	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	value[1] = config;
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_WRITE_CONFIG, value, 0, QSPI_CMD_ACCESS, 2);
}

/**
 * \brief Reads and returns the configuration register of the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \return QSPI configuration register
 */
uint8_t sst26vfxxx_read_configuration(struct qspid_t *qspid)
{
	uint8_t value;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_CONFIG, 0, &value, QSPI_CMD_ACCESS, 1);
	return (value);
}

/**
 * \brief Reads and returns the serial flash Jedec ID.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_id Pointer where id read from chip is stored.
 */
void sst26vfxxx_read_jedec_id(struct qspid_t *qspid, uint8_t *ptr_id)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_JDEC_ID, 0, ptr_id, QSPI_CMD_ACCESS, 3);
}

/**
 * \brief Reads and returns the Quad serial flash Jedec ID.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_id Pointer where id read from chip is stored.
 */
void sst26vfxxx_read_quad_jedec_id(struct qspid_t *qspid, uint8_t *ptr_id)
{
	/* Configure WIDTH */
	dev_instance->inst_frame.bm.b_width = QSPI_IFR_QUAD_CMD;

	/* Configure Dummy bytes */
	dev_instance->inst_frame.bm.b_dummy_cycles = 2;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_QUAD_JDEC_ID, 0, ptr_id, QSPI_CMD_ACCESS, 3);
}

/**
 * \brief Read information describing the characteristics of the memory device.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param address  Address to get data.
 * \param ptr_data  Pointer where information is stored.
 * \param data_len  Length of the data information to read.
 */
void sst26vfxxx_read_flash_parameters(struct qspid_t *qspid, uint32_t address,
		uint8_t *ptr_data, uint16_t data_len)
{
	/* Configure address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
	dev_instance->addr = address;

	/* Configure Dummy bytes */
	dev_instance->inst_frame.bm.b_dummy_cycles = 8;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_SERIAL_FLASH_PARAMS, 0, ptr_data,
			QSPI_CMD_ACCESS, data_len);
}

/**
 * \brief Disable write option to serial flash memory.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_write_disable(struct qspid_t *qspid)
{
	uint8_t status = 0;

	while (status & SSTV26FXXX_STATUS_WEL) {
		sst26vfxxx_exec_command(qspid, SSTV26FXXX_WRITE_DISABLE, 0, 0, QSPI_CMD_ACCESS, 0);
		status = sst26vfxxx_read_status(qspid);
	}
}

/**
 * \brief Enable write option to serial flash memory.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_write_enable(struct qspid_t *qspid)
{
	uint8_t status = 0;

	while (!(status & SSTV26FXXX_STATUS_WEL)) {
		sst26vfxxx_exec_command(qspid, SSTV26FXXX_WRITE_ENABLE, 0, 0, QSPI_CMD_ACCESS, 0);
		status = sst26vfxxx_read_status(qspid);
	}
}

/**
 * \brief  Erases the specified 4KB sector of the serial firmware dataflash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param address  Address of the 4KB sector to erase.
 *
 * \return SSTV26FXXX_SUCCESS if successful; otherwise ERROR_BUSY if it is busy executing a command.
 */
uint8_t sst26vfxxx_erase_sector(struct qspid_t *qspid, uint32_t address)
{
	uint8_t status;

	/* Check that the flash is ready and unprotected */
	status = sst26vfxxx_read_status(qspid);
	if (status & SSTV26FXXX_STATUS_BUSY) {
		return SSTV26FXXX_ERROR_BUSY;
	}

	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
	dev_instance->addr = address & 0xFFFFF000;

	/* Start the block erase command */
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_ERASE_SECTOR_4KB, 0, 0, QSPI_CMD_ACCESS, 0);

	/* Wait for transfer to finish */
	while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		delay_ms(5);
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief  Erases the specified block of the serial firmware dataflash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param address  Address of the block to erase.
 *
 * \return 0 if successful; otherwise returns ERROR_BUSY if it is busy executing a command.
 * \note Block sizes can be 8 KByte, 32 KByte or 64 KByte depending on address.
 */
uint8_t sst26vfxxx_erase_block(struct qspid_t *qspid, uint32_t address)
{
	uint8_t status;

	/* Check that the flash is ready and unprotected */
	status = sst26vfxxx_read_status(qspid);
	if (status & SSTV26FXXX_STATUS_BUSY) {
		return SSTV26FXXX_ERROR_BUSY;
	}

	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
	if ((address < 0x8000) || (address >= 0x1F8000)) {
		/* 8 KB block */
		dev_instance->addr = address & 0xFFFFE000;
	} else if ((address < 0xFFFF) || (address >= 0x1F0000)) {
		/* 32 KB block */
		dev_instance->addr = address & 0xFFFF8000;
	} else {
		/* 64 KB block */
		dev_instance->addr = address & 0xFFFF0000;
	}

	/* Start the block erase command */
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_ERASE_BLOCK_MEMORY, 0, 0, QSPI_CMD_ACCESS, 0);

	/* Wait for transfer to finish */
	while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		delay_ms(5);
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief Erases all the content of the memory chip.
 *
 * \param qspid  Pointer to an S25FL1 qspid_t struct.
 *
 * \return SSTV26FXXX_SUCCESS if successful; otherwise ERROR_BUSY if it is busy
 * executing a command.
 */
uint8_t sst26vfxxx_erase_chip(struct qspid_t *qspid)
{
	uint8_t status;

	/* Check that the flash is ready and unprotected */
	status = sst26vfxxx_read_status(qspid);
	if (status & SSTV26FXXX_STATUS_BUSY) {
		return SSTV26FXXX_ERROR_BUSY;
	}

	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	/* Start the chip erase command */
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_ERASE_FULL, 0, 0, QSPI_CMD_ACCESS, 0);

	/* Wait for transfer to finish */
	while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		delay_ms(10);
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief Writes data at the specified address on the serial firmware dataflash. The
 * page(s) to program must have been erased prior to writing. This function
 * handles page boundary crossing automatically.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param pData  Data buffer.
 * \param size  Number of bytes in buffer.
 * \param address  Write address.
 * \param secure  Enable or disable scramble on QSPI.
 * \param aes_en  Enable or disable AES on QSPI.
 *
 * \return SSTV26FXXX_SUCCESS if successful; otherwise, returns ERROR_PROGRAM is there has
 * been an error during the data programming.
 */
uint8_t sst26vfxxx_write(struct qspid_t *qspid, uint8_t *pdata, uint32_t size,
		uint32_t address, uint8_t secure, uint8_t aes_en)
{
	uint32_t size_frag;

	/* Configure WIDTH */
	mem_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;
	mem_instance->inst_frame.bm.b_opt_en = 0;
	mem_instance->inst_frame.bm.b_dummy_cycles = 0;

	/* Manage Page Boundary (256 bytes) */
	size_frag = _get_size_first_fragment(address, size);

	while (size) {
		/* Write Enable */
		sst26vfxxx_write_enable(qspid);

		sst26vfxxx_memory_access(qspid, SSTV26FXXX_PAGE_PROGRAM, address, pdata, 0, QSPI_WRITE_ACCESS, size_frag, secure, aes_en);

		/* Wait for transfer to finish */
		while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		}

		/* Update counters */
		size -= size_frag;
		address += size_frag;
		pdata += size_frag;

		if (size < PAGE_SIZE) {
			size_frag = size;
		} else {
			size_frag = PAGE_SIZE;
		}
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief Writes data in QUAD mode at the specified address on the serial firmware dataflash.
 * The page(s) to program must have been erased prior to writing. This function
 * handles page boundary crossing automatically.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param pData  Data buffer.
 * \param size  Number of bytes in buffer.
 * \param address  Write address.
 * \param secure  Enable or disable scramble on QSPI.
 * \param aes_en  Enable or disable AES on QSPI.
 *
 * \return SSTV26FXXX_SUCCESS if successful; otherwise, returns ERROR_PROGRAM is there has
 * been an error during the data programming.
 */
uint8_t sst26vfxxx_write_quad(struct qspid_t *qspid, uint8_t *pdata, uint32_t size,
		uint32_t address, uint8_t secure, uint8_t aes_en)
{
	uint32_t size_frag;

	/* Configure WIDTH */
	mem_instance->inst_frame.bm.b_width = QSPI_IFR_QUAD_IO;
	mem_instance->inst_frame.bm.b_opt_en = 0;
	mem_instance->inst_frame.bm.b_dummy_cycles = 0;

	/* Manage Page Boundary (256 bytes) */
	size_frag = _get_size_first_fragment(address, size);

	while (size) {
		/* Write Enable */
		sst26vfxxx_write_enable(qspid);

		sst26vfxxx_memory_access(qspid, SSTV26FXXX_SQO_QUAD_PAGE_PROGRAM, address, pdata, 0, QSPI_WRITE_ACCESS, size_frag, secure, aes_en);

		/* Wait for transfer to finish */
		while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		}

		/* Update counters */
		size -= size_frag;
		address += size_frag;
		pdata += size_frag;

		if (size < PAGE_SIZE) {
			size_frag = size;
		} else {
			size_frag = PAGE_SIZE;
		}
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief Writes data at the specified address on the serial firmware dataflash. The
 * page(s) to program must have been erased prior to writing. This function
 * handles page boundary crossing automatically.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param pData  Data buffer.
 * \param size  Number of bytes in buffer.
 * \param address  Write address.
 * \param secure  Enable or disable scramble on QSPI.
 *
 * \return SSTV26FXXX_SUCCESS if successful; otherwise, returns ERROR_PROGRAM is there has
 * been an error during the data programming.
 */
uint8_t sst26vfxxx_write_spi(struct qspid_t *qspid, uint8_t *pdata, uint32_t size, uint32_t address)
{
	uint32_t size_frag;

	/* Manage Page Boundary (256 bytes) */
	size_frag = _get_size_first_fragment(address, size);

	while (size) {
		/* Write Enable */
		sst26vfxxx_write_enable(qspid);

		/* Set Address */
		dev_instance->inst_frame.bm.b_addr_en = 1;
		dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
		dev_instance->addr = address;

		sst26vfxxx_exec_command(qspid, SSTV26FXXX_PAGE_PROGRAM, pdata, 0, QSPI_WRITE_ACCESS, size_frag);

		/* Wait for transfer to finish */
		while (sst26vfxxx_read_status(qspid) & SSTV26FXXX_STATUS_BUSY) {
		}

		/* Update counters */
		size -= size_frag;
		address += size_frag;
		pdata += size_frag;

		if (size < PAGE_SIZE) {
			size_frag = size;
		} else {
			size_frag = PAGE_SIZE;
		}
	}

	return SSTV26FXXX_SUCCESS;
}

/**
 * \brief Reads data from the specified address on the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param data  Data buffer.
 * \param size  Number of bytes to read.
 * \param address  Read address.
 * \param aes_en  Enable or disable AES on QSPI.
 *
 * \return 0 if successful; otherwise, fail.
 */
uint8_t sst26vfxxx_read(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en)
{
	mem_instance->inst_frame.bm.b_dummy_cycles = 8;
	mem_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;
	sst26vfxxx_memory_access(qspid, SSTV26FXXX_READ_MEMORY_HIGH_SPEED, address, 0, data, QSPI_READ_ACCESS, size, 0, aes_en);

	return 0;
}

/**
 * \brief Reads data in DUAL mode from the specified address on the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param data  Data buffer.
 * \param size  Number of bytes to read.
 * \param address  Read address.
 * \param aes_en  Enable or disable AES on QSPI.
 *
 * \return 0 if successful; otherwise, fail.
 */
uint8_t sst26vfxxx_read_dual(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en)
{
	mem_instance->inst_frame.bm.b_width = QSPI_IFR_DUAL_IO;
	mem_instance->inst_frame.bm.b_opt_en = 1;
	mem_instance->inst_frame.bm.b_opt_len = QSPI_IFR_OPTION_8_BIT;
	mem_instance->inst_frame.bm.b_dummy_cycles = 0;
	sst26vfxxx_memory_access(qspid, SSTV26FXXX_SPI_DUAL_IO_READ, address, 0, data, QSPI_READ_ACCESS, size, 0, aes_en);

	return 0;
}

/**
 * \brief Reads data in QUAD mode from the specified address on the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param data  Data buffer.
 * \param size  Number of bytes to read.
 * \param address  Read address.
 * \param aes_en  Enable or disable AES on QSPI.
 *
 * \return 0 if successful; otherwise, fail.
 */
uint8_t sst26vfxxx_read_quad(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en)
{
	mem_instance->inst_frame.bm.b_width = QSPI_IFR_QUAD_IO;
	mem_instance->inst_frame.bm.b_opt_en = 1;
	mem_instance->inst_frame.bm.b_opt_len = QSPI_IFR_OPTION_8_BIT;
	mem_instance->inst_frame.bm.b_dummy_cycles = 4;
	sst26vfxxx_memory_access(qspid, SSTV26FXXX_SPI_QUAD_IO_READ, address, 0, data, QSPI_READ_ACCESS, size, 0, aes_en);

	return 0;
}

/**
 * \brief Reads data from the specified address on the serial flash.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param data  Data buffer.
 * \param size  Number of bytes to read.
 * \param address  Read address.
 *
 * \return 0 if successful; otherwise, fail.
 */
uint8_t sst26vfxxx_read_spi(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address)
{
	dev_instance->inst_frame.bm.b_dummy_cycles = 8;
	dev_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_24_BIT;
	dev_instance->addr = address;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_MEMORY_HIGH_SPEED, 0, data, QSPI_READ_ACCESS, size);

	return 0;
}

/**
 * \brief Reads data from the Block Protection register.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_bpr  Pointer where Block Protection Data will be stored.
 */
void sst26vfxxx_read_block_protection_reg(struct qspid_t *qspid, uint8_t *ptr_bpr)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_BPR, 0, ptr_bpr, QSPI_CMD_ACCESS, 6);
}

/**
 * \brief Write data to the Block Protection register.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_bpr  Pointer where is the Block Protection Data to write.
 */
void sst26vfxxx_write_block_protection_reg(struct qspid_t *qspid, uint8_t *ptr_bpr)
{
	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_WRITE_BPR, ptr_bpr, 0, QSPI_CMD_ACCESS, 6);
}

/**
 * \brief Lock-Down Block Protection register.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_lockdown_block_protection_reg(struct qspid_t *qspid)
{
	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_LOCK_DOWN_BPR, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Non-Volatile Write Lock-Down Register.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_bpr  Pointer where is the Non-Volatile Write Lock-Down Data to write.
 */
void sst26vfxxx_nonvolatile_write_lockdown_reg(struct qspid_t *qspid, uint8_t *ptr_nvwldr)
{
	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_LOCK_DOWN_BPR, ptr_nvwldr, 0, QSPI_CMD_ACCESS, 6);
}

/**
 * \brief The Global Block-Protection Unlock (ULBPR) instruction clears all write-protection
 * bits in the Block-Protection register, except for those bits that have been locked down
 * with the nVWLDR command.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_global_protection_unlock(struct qspid_t *qspid)
{
	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_GLOBAL_BP_UNLOCK, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Enable power down mode.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_enable_powerdown(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_DEEP_POWERDOWN, 0, 0, QSPI_CMD_ACCESS, 0);
}

/**
 * \brief Release from power down mode.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \return Device ID of the memory.
 */
uint8_t sst26vfxxx_release_powerdown(struct qspid_t *qspid)
{
	uint8_t dev_id;

	/* Configure Dummy bytes */
	dev_instance->inst_frame.bm.b_dummy_cycles = 24;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_RELEASE_DEEP_POWERDOWN, 0, &dev_id,
			QSPI_CMD_ACCESS, 1);

	return dev_id;
}

/**
 * \brief Read Unique ID.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_data Pointer to buffer where Unique ID will be stored.
 *
 * \return Length of the UNIQUE ID.
 */
uint8_t sst26vfxxx_read_unique_id(struct qspid_t *qspid, uint8_t *ptr_data)
{
	dev_instance->inst_frame.bm.b_dummy_cycles = 8;
	dev_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_16_BIT;
	dev_instance->addr = 0x0000;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_SECURITY_ID, 0, ptr_data, QSPI_READ_ACCESS, 8);

	return 8;
}

/**
 * \brief Read User Security Region.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_data Pointer to buffer where user Security Data will be stored.
 *
 * \return Length of read data.
 */
uint16_t sst26vfxxx_read_user_security_id(struct qspid_t *qspid, uint32_t address, uint8_t *ptr_data,
		uint16_t data_len)
{
	if (address < 0x0008) {
		return 0;
	}

	/* Data length protection */
	if (data_len > 2040) {
		data_len = 2040;
	}

	dev_instance->inst_frame.bm.b_dummy_cycles = 8;
	dev_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_16_BIT;
	dev_instance->addr = address;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_READ_SECURITY_ID, 0, ptr_data, QSPI_READ_ACCESS, data_len);

	return data_len;
}

/**
 * \brief Program User Security Region.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 * \param ptr_data Pointer to the data to write in user Security Data.
 *
 * \return Length of written data.
 */
uint16_t sst26vfxxx_program_user_security_id(struct qspid_t *qspid, uint32_t address, uint8_t *ptr_data,
		uint16_t data_len)
{
	if (address < 0x0008) {
		return 0;
	}

	/* Write Enable */
	sst26vfxxx_write_enable(qspid);

	/* Data length protection */
	if (data_len > 2040) {
		data_len = 2040;
	}

	dev_instance->inst_frame.bm.b_dummy_cycles = 0;
	dev_instance->inst_frame.bm.b_width = QSPI_IFR_SINGLE_BIT_SPI;

	/* Set Address */
	dev_instance->inst_frame.bm.b_addr_en = 1;
	dev_instance->inst_frame.bm.b_addr_len = QSPI_IFR_ADDRESS_16_BIT;
	dev_instance->addr = address;

	sst26vfxxx_exec_command(qspid, SSTV26FXXX_PROGRAM_USER_SEC_ID_AREA, ptr_data, 0, QSPI_WRITE_ACCESS, data_len);

	return data_len;
}

/**
 * \brief Lockout Security ID.
 *
 * \param qspid  Pointer to an SST26VF qspid_t struct.
 */
void sst26vfxxx_lockout_security_id(struct qspid_t *qspid)
{
	sst26vfxxx_exec_command(qspid, SSTV26FXXX_LOCKOUT_SEC_ID_PROG, 0, 0, QSPI_CMD_ACCESS, 0);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
