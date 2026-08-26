/**
 * \file
 *
 * \brief Special Function Registers (SFR) driver for PIC32CX.
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

#ifndef SFR_H_INCLUDED
#define SFR_H_INCLUDED

#include "compiler.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* Definitions for RAM sizes */
enum ram_size_t {
	RAM_128 = 0, /* RAM size 128KB */
	RAM_256,     /* RAM size 256KB */
	RAM_384,     /* RAM size 384KB */
	RAM_512      /* RAM size 256KB */
};

/* Definitions for memory mode */
enum mem_mode_t {
	MODE_LIGHTSLEEP = 0,
	MODE_DEEPSLEEP,
	MODE_SHUTDOWN
};

/* Definitions for read margin port */
enum read_margin_port_t {
	RM_PORT_0 = 0,
	RM_PORT_1,
	RM_PORT_2,
	RM_PORT_3
};

/* SRAM0 functions */
void sfr_sram0_set_sw_mode(Sfr *p_sfr, const enum ram_size_t ram_size, const enum mem_mode_t mem_mode);
void sfr_sram0_enable_sw_clockgating(Sfr *p_sfr);
void sfr_sram0_disable_sw_clockgating(Sfr *p_sfr);
uint32_t sfr_sram0_get_hw_configuration(const Sfr *p_sfr);
void sfr_sram0_set_read_margin(Sfr *p_sfr, const uint8_t uc_channel, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin);

/* SRAM1 functions */
void sfr_sram1_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_sram1_set_read_margin(Sfr *p_sfr, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin);

/* SRAM2 functions */
void sfr_sram2_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_sram2_set_read_margin(Sfr *p_sfr, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin);

/* CPKCC memory functions */
void sfr_cpkcc_rom_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_cpkcc_rom_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin);
void sfr_cpkcc_ram_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_cpkcc_ram_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin);

/* HROMC memory functions */
void sfr_hromc_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hromc_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin);

/* HCACHEI valid memory functions */
void sfr_hcachei_valid_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcachei_valid_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin);

/* HCACHEI data memory functions */
void sfr_hcachei_data_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcachei_data_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin);

/* HCACHEI tag memory functions */
void sfr_hcachei_tag_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcachei_tag_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin);

/* HCACHED valid memory functions */
void sfr_hcached_valid_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcached_valid_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin);

/* HCACHED data memory functions */
void sfr_hcached_data_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcached_data_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin);

/* HCACHED tag memory functions */
void sfr_hcached_tag_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode);
void sfr_hcached_tag_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin);

/* Flash memory functions */
void sfr_flash_enable_patch_bypass(Sfr *p_sfr);
void sfr_flash_disable_patch_bypass(Sfr *p_sfr);

/* Optical link functions */
void sfr_optical_link_select_pmc(Sfr *p_sfr);
void sfr_optical_link_select_plla(Sfr *p_sfr);

/* JTAG functions */
void sfr_jtag_lock(Sfr *p_sfr);
void sfr_jtag_unlock(Sfr *p_sfr);

/* Core debug functions */
void sfr_core_debug_configure(Sfr *p_sfr, const bool core_num, const bool enable_cross_trig01, const bool enable_cross_trig10);

/* AHB2AHB functions */
void sfr_ahb2ahb_configure(Sfr *p_sfr, const bool enable_burst8_01, const bool enable_burst8_10);

/* Secure functions */
void sfr_secure_enable_rom_access(Sfr *p_sfr);
void sfr_secure_disable_rom_access(Sfr *p_sfr);

/* Secure bit functions */
uint32_t sfr_secure_bit_get_secure_mode(const Sfr *p_sfr);

/* Erase flash functions */
void sfr_erase_flash_set(Sfr *p_sfr, const uint32_t ul_erase);
uint32_t sfr_erase_flash_get(const Sfr *p_sfr);

/* PWM debug functions */
void sfr_pwm_debug_configure(Sfr *p_sfr, const bool enable_debug0, const bool enable_debug1);

/* FFPI functions */
uint32_t sfr_ffpi_get_status(const Sfr *p_sfr);

/* Improved wait mode functions */
void sfr_improved_wait_mode_enable(Sfr *p_sfr);
void sfr_improved_wait_mode_disable(Sfr *p_sfr);

/* ROM code functions */
uint32_t sfr_rom_code_get_configuration(Sfr *p_sfr);

/* SFR general functions */
void sfr_set_writeprotect(Sfr *p_sfr, const uint32_t ul_enable);
uint32_t sfr_get_writeprotect_status(const Sfr *p_sfr);
void sfr_set_spare(Sfr *p_sfr, const uint32_t ul_spare);
uint32_t sfr_get_spare(const Sfr *p_sfr);
uint32_t sfr_get_version(const Sfr *p_sfr);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* SFR_H_INCLUDED */
