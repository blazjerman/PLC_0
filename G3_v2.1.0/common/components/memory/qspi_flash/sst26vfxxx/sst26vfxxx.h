/**
 * \file
 *
 * \brief QSPI flash memory driver for S25FL116K.
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

#ifndef SSTV26FXXX_H
#define SSTV26FXXX_H

#include "qspi.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/** Operation is success */
#define SSTV26FXXX_SUCCESS                          0
/** Device is protected, operation cannot be carried out. */
#define SSTV26FXXX_ERROR_PROTECTED                  1
/** Device is busy executing a command. */
#define SSTV26FXXX_ERROR_BUSY                       2
/** There was a problem while trying to program page data. */
#define SSTV26FXXX_ERROR_PROGRAM                    3
/** There was an SPI communication error. */
#define SSTV26FXXX_ERROR_SPI                        4
/** Device cannot be protected. */
#define SSTV26FXXX_ERROR_UNPROTECTED                5

/** Device ready/busy status bit. */
#define SSTV26FXXX_STATUS_BUSY                      (1 << 0)
/** Device is ready. */
#define SSTV26FXXX_STATUS_BUSY_READY                (0 << 0)
/** Device is busy with internal operations. */
#define SSTV26FXXX_STATUS_BUSY_BUSY                 (1 << 0)
/** Write enable latch status bit. */
#define SSTV26FXXX_STATUS_WEL                       (1 << 1)
/** Device is not write enabled. */
#define SSTV26FXXX_STATUS_WEL_DISABLED              (0 << 1)
/** Device is write enabled. */
#define SSTV26FXXX_STATUS_WEL_ENABLED               (1 << 1)
/** Write Suspend-Erase status bit. */
#define SSTV26FXXX_STATUS_WSE                       (1 << 2)
/** Device erase is not suspended. */
#define SSTV26FXXX_STATUS_WSE_DISABLED              (0 << 2)
/** Device erase is suspended. */
#define SSTV26FXXX_STATUS_WSE_ENABLED               (1 << 2)
/** Write protect pin status bit. */
#define SSTV26FXXX_STATUS_WSP                       (1 << 3)
/** Device program is not suspended. */
#define SSTV26FXXX_STATUS_WSP_DISABLED              (0 << 3)
/** Device program is suspended. */
#define SSTV26FXXX_STATUS_WSP_ENABLED               (1 << 3)
/** Write Protection Lock-Down status bit. */
#define SSTV26FXXX_STATUS_WPLD                      (1 << 4)
/** Write Protection Lock-Down is disabled. */
#define SSTV26FXXX_STATUS_WPLD_DISABLED             (0 << 4)
/** Write Protection Lock-Down is enabled. */
#define SSTV26FXXX_STATUS_WPLD_ENABLED              (1 << 4)
/** Security ID status bit. */
#define SSTV26FXXX_STATUS_SEC                       (1 << 5)
/** Security ID space is not locked. */
#define SSTV26FXXX_STATUS_SEC_UNLOCKED              (0 << 5)
/** Security ID space is locked. */
#define SSTV26FXXX_STATUS_SEC_LOCKED                (1 << 5)

/** I/O Configuration bit for SPI Mode. */
#define SSTV26FXXX_CONFIG_IOC                       (1 << 1)
/** WP# and HOLD# pins are enabled. */
#define SSTV26FXXX_STATUS_IOC_ENABLED               (0 << 1)
/** WP# and HOLD# pins are disabled. */
#define SSTV26FXXX_STATUS_IOC_DISABLED              (1 << 1)

/** Block-Protection Volatility State bit. */
#define SSTV26FXXX_CONFIG_BPNV                      (1 << 3)
/** Any block has been permanently locked. */
#define SSTV26FXXX_STATUS_BPNV_PERM_LOCKED          (0 << 3)
/** No memory block has been permanently locked. */
#define SSTV26FXXX_STATUS_BPNV_NO_PERM_LOCKED       (1 << 3)

/** Write-Protection Pin (WP#) Enable bit. */
#define SSTV26FXXX_CONFIG_WPEN                      (1 << 7)
/** WP# is disabled. */
#define SSTV26FXXX_STATUS_WPEN_DISABLED             (0 << 7)
/** WP# is enabled. */
#define SSTV26FXXX_STATUS_WPEN_ENABLED              (1 << 7)

/** Configuration Inst.: Reset Enable. */
#define SSTV26FXXX_RESET_ENABLE                     0x66
/** Configuration Inst.: Reset Memory. */
#define SSTV26FXXX_RESET_MEMORY                     0x99
/** Configuration Inst.: Enable QUAD I/O. */
#define SSTV26FXXX_ENABLE_QUAD_IO                   0x38
/** Configuration Inst.: Reset QUAD I/O. */
#define SSTV26FXXX_RESET_EQUAD_IO                   0xFF
/** Configuration Inst.: Read Status Register. */
#define SSTV26FXXX_READ_STATUS                      0x05
/** Configuration Inst.: Write Configuration Register. */
#define SSTV26FXXX_WRITE_CONFIG                     0x01
/** Configuration Inst.: Read Configuration Reg. */
#define SSTV26FXXX_READ_CONFIG                      0x35

/** Read Inst.: Read Memory. */
#define SSTV26FXXX_READ_MEMORY                      0x03
/** Read Inst.: Read memory at High Speed. */
#define SSTV26FXXX_READ_MEMORY_HIGH_SPEED           0x0B
/** Read Inst.: SPI QUAD Output Read. */
#define SSTV26FXXX_SPI_QUAD_OUTPUT_READ             0x6B
/** Read Inst.: SPI QUAD I/O Read. */
#define SSTV26FXXX_SPI_QUAD_IO_READ                 0xEB
/** Read Inst.: SPI DUAL Output Read. */
#define SSTV26FXXX_SPI_DUAL_OUTPUT_READ             0x3B
/** Read Inst.: SPI DUAL I/O Read. */
#define SSTV26FXXX_SPI_DUAL_IO_READ                 0xBB
/** Read Inst.: Set Burst Length. */
#define SSTV26FXXX_SET_BURST_LEN                    0xC0
/** Read Inst.: SQI Read Burst with Wrap. */
#define SSTV26FXXX_SQI_READ_BURST                   0x0C
/** Read Inst.: SPI Read Burst with Wrap. */
#define SSTV26FXXX_SPI_READ_BURST                   0xEC

/** Identification Inst.: JEDEC-ID Read. */
#define SSTV26FXXX_READ_JDEC_ID                     0x9F
/** Identification Inst.: Quad I/O J-ID Read. */
#define SSTV26FXXX_READ_QUAD_JDEC_ID                0xAF
/** Identification Inst.: Serial Flash Discoverable Parameters. */
#define SSTV26FXXX_READ_SERIAL_FLASH_PARAMS         0x5A

/** Write Inst.: Write Enable. */
#define SSTV26FXXX_WRITE_ENABLE                     0x06
/** Write Inst.: Write Disable. */
#define SSTV26FXXX_WRITE_DISABLE                    0x04
/** Write Inst.: Erase 4 KBytes of Memory Array. */
#define SSTV26FXXX_ERASE_SECTOR_4KB                 0x20
/** Write Inst.: Erase 64, 32 or 8 KBytes of Memory Array. */
#define SSTV26FXXX_ERASE_BLOCK_MEMORY               0xD8
/** Write Inst.: Erase Full Array. */
#define SSTV26FXXX_ERASE_FULL                       0xC7
/** Write Inst.: Page Program. */
#define SSTV26FXXX_PAGE_PROGRAM                     0x02
/** Write Inst.: SQI Quad Page Program. */
#define SSTV26FXXX_SQO_QUAD_PAGE_PROGRAM            0x32
/** Write Inst.: Suspends Program/Erase. */
#define SSTV26FXXX_SUSPEND_PE                       0xB0
/** Write Inst.: Resumes Program/Erase. */
#define SSTV26FXXX_RESUME_PE                        0x30

/** Protection Inst.: Read Block-Protection Register. */
#define SSTV26FXXX_READ_BPR                         0x72
/** Protection Inst.: Write Block-Protection Register. */
#define SSTV26FXXX_WRITE_BPR                        0x42
/** Protection Inst.: Lock Down Block-Protection Register. */
#define SSTV26FXXX_LOCK_DOWN_BPR                    0x8D
/** Protection Inst.: non-Volatile Write Lock-Down Register. */
#define SSTV26FXXX_NV_WRITE_LOCK_LDR                0xE8
/** Protection Inst.: Global Block Protection Unlock. */
#define SSTV26FXXX_GLOBAL_BP_UNLOCK                 0x98
/** Protection Inst.: Read Security ID. */
#define SSTV26FXXX_READ_SECURITY_ID                 0x88
/** Protection Inst.: Program User Security ID area. */
#define SSTV26FXXX_PROGRAM_USER_SEC_ID_AREA         0xA5
/** Protection Inst.: Lockout Security ID Programming. */
#define SSTV26FXXX_LOCKOUT_SEC_ID_PROG              0x85

/** Power Saving Inst.: Deep Power-down mode. */
#define SSTV26FXXX_DEEP_POWERDOWN                   0xB9
/** Power Saving Inst.: Release from Deep Power-down mode. */
#define SSTV26FXXX_RELEASE_DEEP_POWERDOWN           0xAB

/** QSPI Flash Manufacturer JEDEC ID */
#define SSTV26FXXX_ATMEL_SPI_FLASH                  0x1F
#define SSTV26FXXX_ST_SPI_FLASH                     0x20
#define SSTV26FXXX_WINBOND_SPI_FLASH                0xEF
#define SSTV26FXXX_MACRONIX_SPI_FLASH               0xC2
#define SSTV26FXXX_SST_SPI_FLASH                    0xBF

enum block_size {
	SSTV26FXXX_SIZE_4K = 0,
	SSTV26FXXX_SIZE_8K,
	SSTV26FXXX_SIZE_32K,
	SSTV26FXXX_SIZE_64K
};

enum status_code sst26vfxxx_initialize(Qspi *qspi, struct qspi_config_t *mode_config,
		uint32_t use_default_config);

void sst26vfxxx_reset(struct qspid_t *qspid);
void sst26vfxxx_enable_quad_mode(struct qspid_t *qspid);
void sst26vfxxx_disable_quad_mode(struct qspid_t *qspid);
void sst26vfxxx_quad_reset(struct qspid_t *qspid);
uint8_t sst26vfxxx_read_status(struct qspid_t *qspid);
void sst26vfxxx_write_configuration(struct qspid_t *qspid, uint8_t config);
uint8_t sst26vfxxx_read_configuration(struct qspid_t *qspid);

void sst26vfxxx_read_jedec_id(struct qspid_t *qspid, uint8_t *ptr_id);
void sst26vfxxx_read_quad_jedec_id(struct qspid_t *qspid, uint8_t *ptr_id);

void sst26vfxxx_write_disable(struct qspid_t *qspid);
void sst26vfxxx_write_enable(struct qspid_t *qspid);
uint8_t sst26vfxxx_erase_sector(struct qspid_t *qspid, uint32_t address);
uint8_t sst26vfxxx_erase_block(struct qspid_t *qspid, uint32_t address);
uint8_t sst26vfxxx_erase_chip(struct qspid_t *qspid);

uint8_t sst26vfxxx_write_spi(struct qspid_t *qspid, uint8_t *pdata, uint32_t size, uint32_t address);
uint8_t sst26vfxxx_write(struct qspid_t *qspid, uint8_t *pdata, uint32_t size,
		uint32_t address, uint8_t secure, uint8_t aes_en);
uint8_t sst26vfxxx_write_quad(struct qspid_t *qspid, uint8_t *pdata, uint32_t size,
		uint32_t address, uint8_t secure, uint8_t aes_en);

uint8_t sst26vfxxx_read_spi(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address);
uint8_t sst26vfxxx_read(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en);
uint8_t sst26vfxxx_read_dual(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en);
uint8_t sst26vfxxx_read_quad(struct qspid_t *qspid, uint8_t *data, uint32_t size, uint32_t address, uint8_t aes_en);

void sst26vfxxx_read_flash_parameters(struct qspid_t *qspid, uint32_t address,
		uint8_t *ptr_data, uint16_t data_len);

void sst26vfxxx_read_block_protection_reg(struct qspid_t *qspid, uint8_t *ptr_bpr);
void sst26vfxxx_write_block_protection_reg(struct qspid_t *qspid, uint8_t *ptr_bpr);
void sst26vfxxx_lockdown_block_protection_reg(struct qspid_t *qspid);
void sst26vfxxx_nonvolatile_write_lockdown_reg(struct qspid_t *qspid, uint8_t *ptr_nvwldr);
void sst26vfxxx_global_protection_unlock(struct qspid_t *qspid);
uint8_t sst26vfxxx_read_unique_id(struct qspid_t *qspid, uint8_t *ptr_data);
uint16_t sst26vfxxx_read_user_security_id(struct qspid_t *qspid, uint32_t address, uint8_t *ptr_data,
		uint16_t data_len);
uint16_t sst26vfxxx_program_user_security_id(struct qspid_t *qspid, uint32_t address, uint8_t *ptr_data,
		uint16_t data_len);
void sst26vfxxx_lockout_security_id(struct qspid_t *qspid);

void sst26vfxxx_enable_powerdown(struct qspid_t *qspid);
uint8_t sst26vfxxx_release_powerdown(struct qspid_t *qspid);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* #ifndef S25FL1_H */
