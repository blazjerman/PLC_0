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

#include "sfr.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \defgroup sam_drivers_sfr_group Special Function Registers (SFR)
 *
 * Driver for the Special Function Registers. This driver manages
 * specific aspects of the integrated memory, bridge implementations.
 * processor and other functionalities.
 *
 * @{
 */

/**
 * \brief Set SRAM0 Software Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param ram_size       RAM size
 * \param mem_mode       Memory mode
 *
 * This function enables the specified mode in the indicated memory, disabling the rest.
 */
void sfr_sram0_set_sw_mode(Sfr *p_sfr, const enum ram_size_t ram_size, const enum mem_mode_t mem_mode)
{
	bool b_clkg;

	b_clkg = (bool)(p_sfr->SFR_SRAM0_SW_CFG >> 16);

	switch (ram_size) {
	case RAM_128:
		p_sfr->SFR_SRAM0_SW_CFG = (1 << mem_mode) | (b_clkg << 16);
		break;

	case RAM_256:
		p_sfr->SFR_SRAM0_SW_CFG = (1 << (mem_mode + 4)) | (b_clkg << 16);
		break;

	case RAM_384:
		p_sfr->SFR_SRAM0_SW_CFG = (1 << (mem_mode + 8)) | (b_clkg << 16);
		break;

	case RAM_512:
		p_sfr->SFR_SRAM0_SW_CFG = (1 << (mem_mode + 12)) | (b_clkg << 16);
		break;

	default:
		;
	}
}

/**
 * \brief Enable SRAM0 Software Clock Gating
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_sram0_enable_sw_clockgating(Sfr *p_sfr)
{
	p_sfr->SFR_SRAM0_SW_CFG |= SFR_SRAM0_SW_CFG_CLKG_DIS;
}

/**
 * \brief Disable SRAM0 Software Clock Gating
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_sram0_disable_sw_clockgating(Sfr *p_sfr)
{
	p_sfr->SFR_SRAM0_SW_CFG &= ~SFR_SRAM0_SW_CFG_CLKG_DIS;
}

/**
 * \brief Get SRAM0 Hardware Configuration
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \retval SRAM0 Hardware Configuration
 */
uint32_t sfr_sram0_get_hw_configuration(const Sfr *p_sfr)
{
	return(p_sfr->SFR_SRAM0_HW_CFG);
}

/**
 * \brief Set SRAM0 Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_channel             Channel
 * \param read_margin_port       Read margin port number
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_sram0_set_read_margin(Sfr *p_sfr, const uint8_t uc_channel, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin)
{
	if (uc_channel < 4) {
  		switch (read_margin_port) {
		case RM_PORT_0:
			p_sfr->SFR_SRAM0_CH[uc_channel] |= SFR_SRAM0_CH_RME0 | SFR_SRAM0_CH_RM0(uc_read_margin);
			break;

		case RM_PORT_1:
			p_sfr->SFR_SRAM0_CH[uc_channel] |= SFR_SRAM0_CH_RME1 | SFR_SRAM0_CH_RM1(uc_read_margin);
			break;

		case RM_PORT_2:
			p_sfr->SFR_SRAM0_CH[uc_channel] |= SFR_SRAM0_CH_RME2 | SFR_SRAM0_CH_RM2(uc_read_margin);
			break;

		case RM_PORT_3:
			p_sfr->SFR_SRAM0_CH[uc_channel] |= SFR_SRAM0_CH_RME3 | SFR_SRAM0_CH_RM3(uc_read_margin);
			break;

		default:
			;
		}
	}
}

/**
 * \brief Set SRAM1/2 Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 * \param uc_sram_num    SRAM number
 *
 * This function enables the specified memory mode, disabling the rest.
 */
static void _sfr_sram_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode, const uint8_t uc_sram_num)
{
	p_sfr->SFR_SRAM[uc_sram_num] &= ~SFR_SRAM_LS;
	p_sfr->SFR_SRAM[uc_sram_num] &= ~SFR_SRAM_DS;
	p_sfr->SFR_SRAM[uc_sram_num] &= ~SFR_SRAM_SD;
	p_sfr->SFR_SRAM[uc_sram_num] |= (1 << (mem_mode + 24));
}

/**
 * \brief Set SRAM1 Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_sram1_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	_sfr_sram_set_mode(p_sfr, mem_mode, 0);
}

/**
 * \brief Set SRAM2 Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_sram2_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	_sfr_sram_set_mode(p_sfr, mem_mode, 1);
}

/**
 * \brief Set SRAM1/2 Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param read_margin_port       Read margin port number
 * \param uc_read_margin         Read margin value
 * \param uc_sram_num            SRAM number
 *
 */
static void _sfr_sram_set_read_margin(Sfr *p_sfr, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin, const uint8_t uc_sram_num)
{
	switch (read_margin_port) {
	case RM_PORT_0:
		p_sfr->SFR_SRAM[uc_sram_num] |= SFR_SRAM_RME0 | SFR_SRAM_RM0(uc_read_margin);
		break;

	case RM_PORT_1:
		p_sfr->SFR_SRAM[uc_sram_num] |= SFR_SRAM_RME1 | SFR_SRAM_RM1(uc_read_margin);
		break;

	case RM_PORT_2:
		p_sfr->SFR_SRAM[uc_sram_num] |= SFR_SRAM_RME2 | SFR_SRAM_RM2(uc_read_margin);
		break;

	case RM_PORT_3:
		p_sfr->SFR_SRAM[uc_sram_num] |= SFR_SRAM_RME3 | SFR_SRAM_RM3(uc_read_margin);
		break;

	default:
		;
	}
}


/**
 * \brief Set SRAM1 Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param read_margin_port       Read margin port number
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_sram1_set_read_margin(Sfr *p_sfr, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin)
{
	_sfr_sram_set_read_margin(p_sfr, read_margin_port, uc_read_margin, 0);
}

/**
 * \brief Set SRAM1 Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param read_margin_port       Read margin port number
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_sram2_set_read_margin(Sfr *p_sfr, const enum read_margin_port_t read_margin_port, const uint8_t uc_read_margin)
{
	_sfr_sram_set_read_margin(p_sfr, read_margin_port, uc_read_margin, 1);
}

/**
 * \brief Set CPKCC ROM Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_cpkcc_rom_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	if (mem_mode == MODE_LIGHTSLEEP) {
		p_sfr->SFR_CPKCC &= ~SFR_CPKCC_ROM_SD;
		p_sfr->SFR_CPKCC |= SFR_CPKCC_ROM_LS;
	} else if (mem_mode == MODE_SHUTDOWN) {
		p_sfr->SFR_CPKCC &= ~SFR_CPKCC_ROM_LS;
		p_sfr->SFR_CPKCC |= SFR_CPKCC_ROM_SD;
	}
}

/**
 * \brief Set CPKCC ROM Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_cpkcc_rom_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin)
{
	p_sfr->SFR_CPKCC |= SFR_CPKCC_ROM_RME | SFR_CPKCC_ROM_RM(uc_read_margin);
}

/**
 * \brief Set CPKCC RAM Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_cpkcc_ram_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_CPKCC &= ~SFR_CPKCC_RAM_LS;
	p_sfr->SFR_CPKCC &= ~SFR_CPKCC_RAM_DS;
	p_sfr->SFR_CPKCC &= ~SFR_CPKCC_RAM_SD;
	p_sfr->SFR_CPKCC |= (1 << (mem_mode + 24));
}


/**
 * \brief Set CPKCC RAM Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_cpkcc_ram_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin)
{
	p_sfr->SFR_CPKCC |= SFR_CPKCC_RAM_RME | SFR_CPKCC_RAM_RM(uc_read_margin);
}

/**
 * \brief Set HROMC Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hromc_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	if (mem_mode == MODE_LIGHTSLEEP) {
		p_sfr->SFR_HROMC &= ~SFR_HROMC_SD;
		p_sfr->SFR_CPKCC |= SFR_HROMC_LS;
	} else if (mem_mode == MODE_SHUTDOWN) {
		p_sfr->SFR_HROMC &= ~SFR_HROMC_LS;
		p_sfr->SFR_HROMC |= SFR_HROMC_SD;
	}
}

/**
 * \brief Set HROMC Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_hromc_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin)
{
	p_sfr->SFR_HROMC |= SFR_HROMC_RME | SFR_HROMC_RM(uc_read_margin);
}

/**
 * \brief Set HCACHEI Valid Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcachei_valid_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHEI_VALID &= ~SFR_HCACHEI_VALID_LS;
	p_sfr->SFR_HCACHEI_VALID &= ~SFR_HCACHEI_VALID_DS;
	p_sfr->SFR_HCACHEI_VALID &= ~SFR_HCACHEI_VALID_SD;
	p_sfr->SFR_HCACHEI_VALID |= (1 << (mem_mode + 8));
}


/**
 * \brief Set HCACHEI Valid Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_hcachei_valid_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin)
{
	p_sfr->SFR_HCACHEI_VALID |= SFR_HCACHEI_VALID_RME | SFR_HCACHEI_VALID_RM(uc_read_margin);
}

/**
 * \brief Set HCACHEI Data Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcachei_data_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_LS(0);
	p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_DS(0);
	p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_SD(0);

	switch (mem_mode) {
	case MODE_LIGHTSLEEP:
		p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_LS(1);
		break;

	case MODE_DEEPSLEEP:
		p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_DS(1);
		break;

	case MODE_SHUTDOWN:
		p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_SD(1);
		break;

	default:
		;
	}
}


/**
 * \brief Set HCACHEI Data Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param us_read_margin         Read margin value
 *
 */
void sfr_hcachei_data_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin)
{
	p_sfr->SFR_HCACHEI_DATA |= SFR_HCACHEI_DATA_RME(1) | SFR_HCACHEI_DATA_RM(us_read_margin);
}

/**
 * \brief Set HCACHEI Tag Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcachei_tag_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_LS(0);
	p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_DS(0);
	p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_SD(0);

	switch (mem_mode) {
	case MODE_LIGHTSLEEP:
		p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_LS(1);
		break;

	case MODE_DEEPSLEEP:
		p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_DS(1);
		break;

	case MODE_SHUTDOWN:
		p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_SD(1);
		break;

	default:
		;
	}
}

/**
 * \brief Set HCACHEI Tag Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param us_read_margin         Read margin value
 *
 */
void sfr_hcachei_tag_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin)
{
	p_sfr->SFR_HCACHEI_TAG |= SFR_HCACHEI_TAG_RME(1) | SFR_HCACHEI_TAG_RM(us_read_margin);
}

/**
 * \brief Set HCACHED Valid Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcached_valid_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHED_VALID &= ~SFR_HCACHED_VALID_LS;
	p_sfr->SFR_HCACHED_VALID &= ~SFR_HCACHED_VALID_DS;
	p_sfr->SFR_HCACHED_VALID &= ~SFR_HCACHED_VALID_SD;
	p_sfr->SFR_HCACHED_VALID |= (1 << (mem_mode + 8));
}


/**
 * \brief Set HCACHEI Valid Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param uc_read_margin         Read margin value
 *
 */
void sfr_hcached_valid_set_read_margin(Sfr *p_sfr, const uint8_t uc_read_margin)
{
	p_sfr->SFR_HCACHED_VALID |= SFR_HCACHED_VALID_RME | SFR_HCACHED_VALID_RM(uc_read_margin);
}

/**
 * \brief Set HCACHED Data Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcached_data_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_LS(0);
	p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_DS(0);
	p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_SD(0);

	switch (mem_mode) {
	case MODE_LIGHTSLEEP:
		p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_LS(1);
		break;

	case MODE_DEEPSLEEP:
		p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_DS(1);
		break;

	case MODE_SHUTDOWN:
		p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_SD(1);
		break;

	default:
		;
	}
}

/**
 * \brief Set HCACHED Data Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param us_read_margin         Read margin value
 *
 */
void sfr_hcached_data_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin)
{
	p_sfr->SFR_HCACHED_DATA |= SFR_HCACHED_DATA_RME(1) | SFR_HCACHED_DATA_RM(us_read_margin);
}

/**
 * \brief Set HCACHED Tag Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param mem_mode       Memory mode
 *
 * This function enables the specified memory mode, disabling the rest.
 */
void sfr_hcached_tag_set_mode(Sfr *p_sfr, const enum mem_mode_t mem_mode)
{
	p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_LS(0);
	p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_DS(0);
	p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_SD(0);

	switch (mem_mode) {
	case MODE_LIGHTSLEEP:
		p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_LS(1);
		break;

	case MODE_DEEPSLEEP:
		p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_DS(1);
		break;

	case MODE_SHUTDOWN:
		p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_SD(1);
		break;

	default:
		;
	}
}

/**
 * \brief Set HCACHED Tag Read Margin
 *
 * \param p_sfr                  Pointer to an SFR instance
 * \param us_read_margin         Read margin value
 *
 */
void sfr_hcached_tag_set_read_margin(Sfr *p_sfr, const uint16_t us_read_margin)
{
	p_sfr->SFR_HCACHED_TAG |= SFR_HCACHED_TAG_RME(1) | SFR_HCACHED_TAG_RM(us_read_margin);
}

/**
 * \brief Enable Flash Memory High-Speed Patch
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_flash_enable_patch_bypass(Sfr *p_sfr)
{
	p_sfr->SFR_FLASH |= SFR_FLASH_PATCH_BYPASS;
}

/**
 * \brief Disable Flash Memory High-Speed Patch
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_flash_disable_patch_bypass(Sfr *p_sfr)
{
	p_sfr->SFR_FLASH &= ~SFR_FLASH_PATCH_BYPASS;
}

/**
 * \brief Select PMC PCK3 in Optical Link
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_optical_link_select_pmc(Sfr *p_sfr)
{
	p_sfr->SFR_OPT_LINK |= SFR_OPT_LINK_CLK_SELECT;
}

/**
 * \brief Select PLLA Clock1 in Optical Link
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_optical_link_select_plla(Sfr *p_sfr)
{
	p_sfr->SFR_OPT_LINK &= ~SFR_OPT_LINK_CLK_SELECT;
}

/**
 * \brief Lock JTAG
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_jtag_lock(Sfr *p_sfr)
{
	p_sfr->SFR_JTAG |= SFR_JTAG_JTAG_LOCK;
}

/**
 * \brief Unlock JTAG
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_jtag_unlock(Sfr *p_sfr)
{
	p_sfr->SFR_JTAG &= ~SFR_JTAG_JTAG_LOCK;
}

/**
 * \brief Configure Core Debug
 *
 * \param p_sfr                   Pointer to an SFR instance
 * \param core_num                SWV core selection (0 or 1)
 * \param enable_cross_trig01     Enable cross triggering from core 0 to core 1
 * \param enable_cross_trig10     Enable cross triggering from core 1 to core 0
 */
void sfr_core_debug_configure(Sfr *p_sfr, const bool core_num, const bool enable_cross_trig01, const bool enable_cross_trig10)
{
	p_sfr->SFR_CORE_DEBUG_CFG |= core_num;

	if (enable_cross_trig01) {
		p_sfr->SFR_CORE_DEBUG_CFG |= SFR_CORE_DEBUG_CFG_XTRG0;
	} else {
		p_sfr->SFR_CORE_DEBUG_CFG &= ~SFR_CORE_DEBUG_CFG_XTRG0;
	}

	if (enable_cross_trig10) {
		p_sfr->SFR_CORE_DEBUG_CFG |= SFR_CORE_DEBUG_CFG_XTRG1;
	} else {
		p_sfr->SFR_CORE_DEBUG_CFG &= ~SFR_CORE_DEBUG_CFG_XTRG1;
	}
}

/**
 * \brief Configure AHB2AHB
 *
 * \param p_sfr                Pointer to an SFR instance
 * \param enable_burst8_01     Enable burst conversion to 8 instead of 4 data in Master0_1
 * \param enable_burst8_10     Enable burst conversion to 8 instead of 4 data in Master1_0
 */
void sfr_ahb2ahb_configure(Sfr *p_sfr, const bool enable_burst8_01, const bool enable_burst8_10)
{
	if (enable_burst8_01) {
		p_sfr->SFR_EMAHB2AHB |= SFR_EMAHB2AHB_PFETCH8_0_1;
	} else {
		p_sfr->SFR_EMAHB2AHB &= ~SFR_EMAHB2AHB_PFETCH8_0_1;
	}

	if (enable_burst8_10) {
		p_sfr->SFR_EMAHB2AHB |= SFR_EMAHB2AHB_PFETCH8_1_0;
	} else {
		p_sfr->SFR_EMAHB2AHB &= ~SFR_EMAHB2AHB_PFETCH8_1_0;
	}
}

/**
 * \brief Enable Secure ROM Access
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_secure_enable_rom_access(Sfr *p_sfr)
{
	p_sfr->SFR_SECURE |= SFR_SECURE_ROM_ENA;
}

/**
 * \brief Disable Secure ROM Access
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_secure_disable_rom_access(Sfr *p_sfr)
{
	p_sfr->SFR_SECURE &= ~SFR_SECURE_ROM_ENA;
}

/**
 * \brief Get Secure Bit Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return Secure Mode
 */
uint32_t sfr_secure_bit_get_secure_mode(const Sfr *p_sfr)
{
	return(p_sfr->SFR_SECURE_BIT);
}

/**
 * \brief Set Erase Flash
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param ul_erase       Erase flash configuration
 */
void sfr_erase_flash_set(Sfr *p_sfr, const uint32_t ul_erase)
{
	p_sfr->SFR_ERASE_FLASH_SRAM = ul_erase;
}

/**
 * \brief Get Erase Flash
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return Erase Flash
 */
uint32_t sfr_erase_flash_get(const Sfr *p_sfr)
{
	return(p_sfr->SFR_ERASE_FLASH_SRAM);
}

/**
 * \brief Configure PWM Debug
 *
 * \param p_sfr             Pointer to an SFR instance
 * \param enable_debug0     Enable PWM debug on core 0
 * \param enable_debug1     Enable PWM debug on core 1
 */
void sfr_pwm_debug_configure(Sfr *p_sfr, const bool enable_debug0, const bool enable_debug1)
{
	if (enable_debug0) {
		p_sfr->SFR_PWM_DEBUG |= SFR_PWM_DEBUG_CORE0;
	} else {
		p_sfr->SFR_PWM_DEBUG &= ~SFR_PWM_DEBUG_CORE0;
	}

	if (enable_debug1) {
		p_sfr->SFR_PWM_DEBUG |= SFR_PWM_DEBUG_CORE1;
	} else {
		p_sfr->SFR_PWM_DEBUG &= ~SFR_PWM_DEBUG_CORE1;
	}
}

/**
 * \brief Get FFPI Status
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return FFPI status
 */
uint32_t sfr_ffpi_get_status(const Sfr *p_sfr)
{
	return(p_sfr->SFR_FFPI);
}

/**
 * \brief Enable Improved Wait Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_improved_wait_mode_enable(Sfr *p_sfr)
{
	p_sfr->SFR_WAIT_MODE |= SFR_WAIT_MODE_CONTROL;
}

/**
 * \brief Disable Improved Wait Mode
 *
 * \param p_sfr          Pointer to an SFR instance
 */
void sfr_improved_wait_mode_disable(Sfr *p_sfr)
{
	p_sfr->SFR_WAIT_MODE &= ~SFR_WAIT_MODE_CONTROL;
}

/**
 * \brief Get ROM Code Configuration
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return ROM code configuration
 */
uint32_t sfr_rom_code_get_configuration(Sfr *p_sfr)
{
	return(p_sfr->SFR_ROM_CODE);
}

/**
 * \brief Enable or disable write protection of SFR registers
 *
 * \param p_sfr         Pointer to an SFR instance
 * \param ul_enable     1 to enable, 0 to disable.
 */
void sfr_set_writeprotect(Sfr *p_sfr, const uint32_t ul_enable)
{
	p_sfr->SFR_WPMR = SFR_WPMR_WPKEY_PASSWD | (ul_enable & SFR_WPMR_WPEN);
}

/**
 * \brief Indicate write protect status
 *
 * \param p_sfr     Pointer to an SFR instance
 *
 * \return 0 if no write protect violation occurred, or 16-bit write protect
 * violation source.
 */
uint32_t sfr_get_writeprotect_status(const Sfr *p_sfr)
{
	uint32_t reg_value;

	reg_value = p_sfr->SFR_WPSR;
	if (reg_value & SFR_WPSR_WPVS) {
		return (reg_value & SFR_WPSR_WPSRC_Msk) >> SFR_WPSR_WPSRC_Pos;
	} else {
		return 0;
	}
}

/**
 * \brief Set SFR Spare
 *
 * \param p_sfr          Pointer to an SFR instance
 * \param ul_spare       Value
 */
void sfr_set_spare(Sfr *p_sfr, const uint32_t ul_spare)
{
	p_sfr->SFR_SPARE = ul_spare;
}

/**
 * \brief Get SFR Spare
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return SFR spare register
 */
uint32_t sfr_get_spare(const Sfr *p_sfr)
{
	return(p_sfr->SFR_SPARE);
}

/**
 * \brief Get SFR Version
 *
 * \param p_sfr          Pointer to an SFR instance
 *
 * \return SFR version
 */
uint32_t sfr_get_version(const Sfr *p_sfr)
{
	return(p_sfr->SFR_VERSION);
}
//@}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
