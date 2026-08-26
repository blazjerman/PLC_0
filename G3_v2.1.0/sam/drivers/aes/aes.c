/**
 *
 * \file
 *
 * \brief SAM Advanced Encryption Standard driver.
 *
 * This file defines a useful set of functions for the AES on SAM devices.
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

#include <aes.h>
#include <sysclk.h>
#include <sleepmgr.h>

/**
 * \internal
 * \brief AES callback function pointer
 */
aes_callback_t aes_callback_pointer[AES_INTERRUPT_SOURCE_NUM];

#if !PIC32CX
/**
 * \brief Initializes an AES configuration structure to defaults.
 *
 * Initializes the specified AES configuration structure to a set of
 * known default values.
 *
 * \note This function should be called to initialize <i>all</i> new instances of
 * AES configuration structures before they are further modified by the user
 * application.
 *
 *  The default configuration is as follows:
 *  - Data encryption
 *  - 128-bit AES key size
 *  - 128-bit cipher feedback size
 *  - Manual start mode
 *  - Electronic Codebook (ECB) mode
 *  - Last output data mode is disabled
 *  - No extra delay
 *
 *  \param[out] p_cfg Pointer to an AES configuration structure
 */
void aes_get_config_defaults(
		struct aes_config *const p_cfg)
#else
/**
 * \brief Initializes an AES configuration structure to defaults.
 *
 * Initializes the specified AES configuration structure to a set of
 * known default values.
 *
 * \note This function should be called to initialize <i>all</i> new instances of
 * AES configuration structures before they are further modified by the user
 * application.
 *
 *  The default configuration is as follows:
 *  - Data encryption
 *  - 128-bit AES key size
 *  - 128-bit cipher feedback size
 *  - Manual start mode
 *  - Electronic Codebook (ECB) mode
 *  - Last output data mode is disabled
 *  - No extra delay
 *  - No tamper detection 
 *  - AES algorithm
 *  - Block processing end disabled 
 *  - No auto padding
 *
 *  \param[out] p_cfg Pointer to an AES configuration structure
 *  \param[out] p_ap_cfg Pointer to an AES auto padding configuration structure
 */
void aes_get_config_defaults(
		struct aes_config *const p_cfg, 
		struct aes_ap_config *const p_ap_cfg)
#endif /* PIC32CX */
{
	/* Sanity check arguments */
	Assert(p_cfg);

	/* Default configuration values */
	p_cfg->encrypt_mode = AES_ENCRYPTION;
	p_cfg->key_size = AES_KEY_SIZE_128;
	p_cfg->start_mode = AES_MANUAL_START;
	p_cfg->opmode = AES_ECB_MODE;
	p_cfg->cfb_size = AES_CFB_SIZE_128;
	p_cfg->lod = false;
	p_cfg->gtag_en = false;
	p_cfg->processing_delay = 0;
#if PIC32CX
	p_cfg->tampclr = false;
	p_cfg->bpe = false;
	p_cfg->algo = AES_ALGO_AES;

	p_ap_cfg->apen = false;
	p_ap_cfg->apm = AES_AUTO_PADDING_IPSEC;
	p_ap_cfg->padlen = 0;
	p_ap_cfg->nhead = 0;
#endif /* PIC32CX */
}

#if !PIC32CX
/**
 * \brief Initialize the AES module.
 *
 * \param[out] p_aes Module hardware register base address pointer
 * \param[in] p_cfg  Pointer to an AES configuration structure
 */
void aes_init(
		Aes *const p_aes,
		struct aes_config *const p_cfg)
#else
/**
 * \brief Initialize the AES module.
 *
 * \param[out] p_aes Module hardware register base address pointer
 * \param[in] p_cfg  Pointer to an AES configuration structure
 * \param[in] p_ap_cfg  Pointer to an AES auto padding configuration structure
 */
void aes_init(
		Aes *const p_aes,
		struct aes_config *const p_cfg,
		struct aes_ap_config *const p_ap_cfg)
#endif /* PIC32CX */
{
	/* Sanity check arguments */
	Assert(p_aes);
	Assert(p_cfg);
#if PIC32CX
	Assert(p_ap_cfg);
#endif

	/* Enable clock for AES */
	sysclk_enable_peripheral_clock(ID_AES);

	/* Perform a software reset */
	aes_reset(p_aes);

	/* Initialize the AES with new configurations */
#if !PIC32CX
	aes_set_config(p_aes, p_cfg);
#else
	aes_set_config(p_aes, p_cfg, p_ap_cfg);
#endif

	/* Disable clock for AES */
	sysclk_disable_peripheral_clock(ID_AES);
}

/**
 * \brief Enable the AES module.
 */
void aes_enable(void)
{
	sysclk_enable_peripheral_clock(ID_AES);
	sleepmgr_lock_mode(SLEEPMGR_SLEEP_WFI);
}

/**
 * \brief Disable the AES module.
 */
void aes_disable(void)
{
	sysclk_disable_peripheral_clock(ID_AES);
	sleepmgr_unlock_mode(SLEEPMGR_SLEEP_WFI);
}

#if !PIC32CX
/**
 * \brief Configure the AES module.
 *
 * \param[out] p_aes Module hardware register base address pointer
 * \param[in] p_cfg  Pointer to an AES configuration structure
 */
void aes_set_config(
		Aes *const p_aes,
		struct aes_config *const p_cfg)
#else
/**
 * \brief Configure the AES module.
 *
 * \param[out] p_aes Module hardware register base address pointer
 * \param[in] p_cfg  Pointer to an AES configuration structure
 * \param[in] p_cfg  Pointer to an AES auto padding configuration structure
 */
void aes_set_config(
		Aes *const p_aes,
		struct aes_config *const p_cfg,
		struct aes_ap_config *const p_ap_cfg)
#endif /* PIC32CX */
{
	uint32_t ul_mode = 0;

	/* Validate arguments. */
	Assert(p_aes);
	Assert(p_cfg);
	
	/* Set processing mode */
	if (p_cfg->encrypt_mode) {
		ul_mode |= AES_MR_CIPHER;
	}

	/* Active dual buffer in DMA mode */
#if !PIC32CX
	if (p_cfg->start_mode == AES_IDATAR0_START) {
#else
	if ((p_cfg->start_mode == AES_IDATAR0_START) && (!p_ap_cfg->apen)) {
#endif /* PIC32CX */
		ul_mode |= AES_MR_DUALBUFF_ACTIVE;
	}

	/* Set start mode */
	ul_mode |= (p_cfg->start_mode << AES_MR_SMOD_Pos);

	/* Set key size */
	ul_mode |= (p_cfg->key_size << AES_MR_KEYSIZE_Pos);

	/* Set Confidentiality mode */
	ul_mode |= (p_cfg->opmode << AES_MR_OPMOD_Pos);

	/* Set CFB size */
	ul_mode |= (p_cfg->cfb_size << AES_MR_CFBS_Pos);

	if (p_cfg->lod) {
		ul_mode |= AES_MR_LOD;
	}

	#if (SAM4C || SAM4CP || SAM4CM || SAMV70 || SAMV71 || SAME70 || SAMS70 || PIC32CX)
	if ((p_cfg->opmode == AES_GCM_MODE) && (p_cfg->gtag_en == true)) {
		ul_mode |= AES_MR_GTAGEN;
	}
	#endif /* SAM4C || SAM4CP || SAM4CM || SAMV70 || SAMV71 || SAME70 || SAMS70 || PIC32CX */

	ul_mode |= AES_MR_PROCDLY(p_cfg->processing_delay);

	#if PIC32CX
	if (p_cfg->tampclr) {
		ul_mode |= AES_MR_TAMPCLR;
	}
	#endif /* PIC32CX */

	ul_mode |= AES_MR_CKEY_PASSWD;

	p_aes->AES_MR = ul_mode;

	#if PIC32CX
	p_aes->AES_EMR = 0;

	if (p_cfg->bpe) {
		p_aes->AES_EMR |= AES_EMR_BPE; 
	}

	if (p_cfg->algo == AES_ALGO_ARIA) {
		p_aes->AES_EMR |= AES_EMR_ALGO_ARIA;
	}

	if (p_ap_cfg->apen) {
		p_aes->AES_EMR |= AES_EMR_APEN;
	}

	if (p_ap_cfg->apm == AES_AUTO_PADDING_SSL) {
		p_aes->AES_EMR |= AES_EMR_APM_SSL;
	}

	p_aes->AES_EMR |= AES_EMR_PADLEN(p_ap_cfg->padlen);
	
	p_aes->AES_EMR |= AES_EMR_NHEAD(p_ap_cfg->nhead);
	#endif /* PIC32CX */
}

#if PIC32CX
/**
 * \brief Select protocol layer improved performance mode.
 *
 * \param p_aes Module hardware register base address pointer
 * \param plip_mode   Protocol layer improved performance mode
 */
void aes_set_plip_mode(Aes *p_aes, enum aes_plip_mode plip_mode)
{
	if (plip_mode == AES_PLIP_CIPHER) {
		p_aes->AES_EMR &= ~AES_EMR_PLIPD;
	} else {
		p_aes->AES_EMR |= AES_EMR_PLIPD;
	}
}

/**
 * \brief Select key.
 *
 * \param p_aes Module hardware register base address pointer
 * \param ksel  Key selection
 */
void aes_key_select(Aes *p_aes, enum aes_ksel ksel)
{
	if (ksel == AES_KEY_FIRST) {
		p_aes->AES_EMR &= ~AES_EMR_KSEL;
	} else {
		p_aes->AES_EMR |= AES_EMR_KSEL;
	}
}

/**
 * \brief Select private key.
 *
 * \param p_aes Module hardware register base address pointer
 * \param pkrs  Private key register selection
 */
void aes_key_select_private(Aes *p_aes, enum aes_pkrs pkrs)
{
	if (pkrs == AES_PRIVATE_KEY_INTERNAL) {
		p_aes->AES_EMR |= AES_EMR_PKRS;
	} else {
		p_aes->AES_EMR &= ~AES_EMR_PKRS;
	}
}

/**
 * \brief Set auto padding byte counter.
 *
 * \param p_aes      Pointer to an AES instance.
 * \param ul_count   Byte count.
 */
void aes_set_byte_count(Aes *p_aes, uint32_t ul_count)
{
	p_aes->AES_BCNT = ul_count;
}

/**
 * \brief Get auto padding byte counter.
 *
 * \param p_aes      Pointer to an AES instance.
 *
 * \return Auto padding byte counter.
 */
uint32_t aes_get_byte_count(Aes *p_aes)
{
	return (p_aes->AES_BCNT);
}

/**
 * \brief Enable or disable write protection of AES registers.
 *
 * \param p_aes                     Pointer to an AES instance.
 * \param enable                    1 to enable, 0 to disable
 * \param int_enable                1 to enable, 0 to disable
 * \param control_enable            1 to enable, 0 to disable
 * \param first_error_report_enable 1 to enable, 0 to disable
 * \param uc_action                 Action on abnormal event detection
 */
void aes_set_writeprotect(Aes *p_aes, bool enable, bool int_enable, bool control_enable, bool first_error_report_enable, uint8_t uc_action)
{
	uint32_t ul_reg;

	ul_reg = AES_WPMR_WPKEY_PASSWD;

	if (enable) {
		ul_reg |= AES_WPMR_WPEN;
	}

	if (int_enable) {
		ul_reg |= AES_WPMR_WPITEN;
	}
	  
	if (control_enable) {
		ul_reg |= AES_WPMR_WPCREN;
	}

	if (first_error_report_enable) {
		ul_reg |= AES_WPMR_FIRSTE;
	}

	ul_reg |= AES_WPMR_ACTION(uc_action);

	p_aes->AES_WPMR = ul_reg;
}


/**
 * \brief Indicate write protect status.
 *
 * \param p_aes Pointer to an AES instance.
 *
 * \return Write protect status.
 */
uint32_t aes_get_writeprotect_status(Aes *p_aes)
{
	return (p_aes->AES_WPSR);
}
#endif /* PIC32CX */

/**
 * \brief Write the 128/192/256-bit cryptographic key.
 *
 * \param[out] p_aes Module hardware register base address pointer
 * \param[in]  p_key Pointer to 4/6/8 contiguous 32-bit words
 *
 * \note The key size depends on the current AES configuration.
 */
void aes_write_key(
		Aes *const p_aes,
		const uint32_t *p_key)
{
	uint32_t i, key_length = 0;

	/* Validate arguments. */
	Assert(p_aes);
	Assert(p_key);
	
	switch ((p_aes->AES_MR & AES_MR_KEYSIZE_Msk) >>
			AES_MR_KEYSIZE_Pos) {
	case 0: /* 128bit cryptographic key */
		key_length = 4;
		break;

	case 1: /* 192bit cryptographic key */
		key_length = 6;
		break;

	case 2: /* 256bit cryptographic key */
		key_length = 8;
		break;

	default:
		break;
	}

	for (i = 0; i < key_length; i++) {
		p_aes->AES_KEYWR[i] = *p_key;
		p_key++;
	}
}

/**
 * \brief Write the initialization vector (for the CBC, CFB, OFB, CTR & GCM
 * cipher modes).
 *
 * \param[out] p_aes   Module hardware register base address pointer
 * \param[in] p_vector Pointer to four contiguous 32-bit words
 */
void aes_write_initvector(
		Aes *const p_aes,
		const uint32_t *p_vector)
{
	uint32_t i;

	/* Validate arguments. */
	Assert(p_aes);
	
	for (i = 0; i < 4; i++) {
		p_aes->AES_IVR[i] = *p_vector;
		p_vector++;
	}
}

/**
 * \brief Write the input data (four consecutive 32-bit words).
 *
 * \param[out] p_aes              Module hardware register base address pointer
 * \param[in] p_input_data_buffer Pointer to an input data buffer
 */
void aes_write_input_data(
		Aes *const p_aes,
		const uint32_t *p_input_data_buffer)
{
	uint32_t i;

	/* Validate arguments. */
	Assert(p_aes);
	Assert(p_input_data_buffer);
	
	for (i = 0; i < 4; i++) {
		p_aes->AES_IDATAR[i] = *p_input_data_buffer;
		p_input_data_buffer++;
	}
}

/**
 * \brief Read the output data.
 *
 * \note The data buffer that holds the processed data must be large enough to hold
 * four consecutive 32-bit words.
 *
 * \param[in] p_aes                 Module hardware register base address pointer
 * \param[in] *p_output_data_buffer Pointer to an output buffer
 */
void aes_read_output_data(
		Aes *const p_aes,
		uint32_t *p_output_data_buffer)
{
	uint32_t i;

	/* Validate arguments. */
	Assert(p_aes);
	Assert(p_output_data_buffer);
	
	for (i = 0; i < 4; i++) {
		*p_output_data_buffer = p_aes->AES_ODATAR[i];
		p_output_data_buffer++;
	}
}

#if SAM4C || SAM4CP || SAM4CM || PIC32CX || defined(__DOXYGEN__)
/**
 * \brief Get AES PDC base address.
 *
 * \note This function is only available on SAM4C devices.
 *
 * \param[in] p_aes Module hardware register base address pointer
 *
 * \return The PDC registers base address for the AES module.
 */
Pdc *aes_get_pdc_base(
		Aes *p_aes)
{
	/* Validate arguments. */
	Assert(p_aes);
	
	Pdc *p_pdc_base;
	if (p_aes == AES) {
		p_pdc_base = PDC_AES;
	} else {
		p_pdc_base = NULL;
	}

	return p_pdc_base;
}
#endif /* SAM4C || SAM4CP || SAM4CM || PIC32CX || defined(__DOXYGEN__) */

/**
 * \brief Set the AES interrupt callback.
 *
 * \param[out] p_aes    Module hardware register base address pointer
 * \param[in] source    Interrupt source
 * \param[in] callback  Interrupt callback function pointer
 * \param[in] irq_level Interrupt priority level
 */
void aes_set_callback(
		Aes *const p_aes,
		aes_interrupt_source_t source,
		aes_callback_t callback,
		uint8_t irq_level)
{
	/* Validate arguments. */
	Assert(p_aes);
	
	if (source == AES_INTERRUPT_DATA_READY) {
		aes_callback_pointer[0] = callback;
	} else if (source == AES_INTERRUPT_UNSPECIFIED_REGISTER_ACCESS) {
		aes_callback_pointer[1] = callback;
	} 

#if SAM4C || SAM4CP || SAM4CM
	else if (source == AES_INTERRUPT_END_OF_RECEIVE_BUFFER) {
		aes_callback_pointer[2] = callback;
	} else if (source == AES_INTERRUPT_END_OF_TRANSMIT_BUFFER) {
		aes_callback_pointer[3] = callback;
	} else if (source == AES_INTERRUPT_RECEIVE_BUFFER_FULL) {
		aes_callback_pointer[4] = callback;
	} else if (source == AES_INTERRUPT_TRANSMIT_BUFFER_FULL) {
		aes_callback_pointer[5] = callback;
	}
#elif SAMV70 || SAMV71 || SAME70 || SAMS70
	else if ((source == AES_INTERRUPT_TAG_READY)) {
		aes_callback_pointer[2] = callback;
	}
#elif PIC32CX
	else if ((source == AES_INTERRUPT_TAG_READY)) {
		aes_callback_pointer[2] = callback;
	} else if (source == AES_INTERRUPT_END_OF_RECEIVE_BUFFER) {
		aes_callback_pointer[3] = callback;
	} else if (source == AES_INTERRUPT_END_OF_TRANSMIT_BUFFER) {
		aes_callback_pointer[4] = callback;
	} else if (source == AES_INTERRUPT_RECEIVE_BUFFER_FULL) {
		aes_callback_pointer[5] = callback;
	} else if (source == AES_INTERRUPT_TRANSMIT_BUFFER_FULL) {
		aes_callback_pointer[6] = callback;
	} else if (source == AES_INTERRUPT_SECURITY_SAFETY_EVENT) {
		aes_callback_pointer[7] = callback;
	} else if (source == AES_INTERRUPT_PADDING_LENGTH_ERROR_EVENT) {
		aes_callback_pointer[8] = callback;
	} else if (source == AES_INTERRUPT_END_PADDING_EVENT) {
		aes_callback_pointer[9] = callback;
	}	
#endif /* SAM4C || SAM4CP || SAM4CM */

	irq_register_handler((IRQn_Type)AES_IRQn, irq_level);
	aes_enable_interrupt(p_aes, source);
}

/**
 * \internal The AES interrupt handler.
 */
void AES_Handler(void)
{
	uint32_t status = aes_read_interrupt_status(AES);
	uint32_t mask = aes_read_interrupt_mask(AES);

	if ((status & AES_ISR_DATRDY) && (mask & AES_IMR_DATRDY)) {
		if (aes_callback_pointer[0]) {
			aes_callback_pointer[0]();
		}
	}

	if ((status & AES_ISR_URAD) && (mask & AES_IMR_URAD)) {
		if (aes_callback_pointer[1]) {
			aes_callback_pointer[1]();
		}
	}

#if SAM4C || SAM4CP || SAM4CM
	if ((status & AES_ISR_ENDRX) && (mask & AES_IMR_ENDRX)) {
		if (aes_callback_pointer[2]) {
			aes_callback_pointer[2]();
		}
	}

	if ((status & AES_ISR_ENDTX) && (mask & AES_IMR_ENDTX)) {
		if (aes_callback_pointer[3]) {
			aes_callback_pointer[3]();
		}
	}

	if ((status & AES_ISR_RXBUFF) && (mask & AES_IMR_RXBUFF)) {
		if (aes_callback_pointer[4]) {
			aes_callback_pointer[4]();
		}
	}

	if ((status & AES_ISR_TXBUFE) && (mask & AES_IMR_TXBUFE)) {
		if (aes_callback_pointer[5]) {
			aes_callback_pointer[5]();
		}
	}
#elif SAMV70 || SAMV71 || SAME70 || SAMS70
	if ((status & AES_IER_TAGRDY) && (mask & AES_IER_TAGRDY)) {
		if (aes_callback_pointer[2]) {
			aes_callback_pointer[2]();
		}
	}
#elif PIC32CX
	if ((status & AES_IER_TAGRDY) && (mask & AES_IER_TAGRDY)) {
		if (aes_callback_pointer[2]) {
			aes_callback_pointer[2]();
		}
	}

	if ((status & AES_ISR_ENDRX) && (mask & AES_IMR_ENDRX)) {
		if (aes_callback_pointer[3]) {
			aes_callback_pointer[3]();
		}
	}

	if ((status & AES_ISR_ENDTX) && (mask & AES_IMR_ENDTX)) {
		if (aes_callback_pointer[4]) {
			aes_callback_pointer[4]();
		}
	}

	if ((status & AES_ISR_RXBUFF) && (mask & AES_IMR_RXBUFF)) {
		if (aes_callback_pointer[5]) {
			aes_callback_pointer[5]();
		}
	}

	if ((status & AES_ISR_TXBUFE) && (mask & AES_IMR_TXBUFE)) {
		if (aes_callback_pointer[6]) {
			aes_callback_pointer[6]();
		}
	}

	if ((status & AES_ISR_SECE) && (mask & AES_IMR_SECE)) {
		if (aes_callback_pointer[7]) {
			aes_callback_pointer[7]();
		}
	}

	if ((status & AES_ISR_PLENERR) && (mask & AES_IMR_PLENERR)) {
		if (aes_callback_pointer[8]) {
			aes_callback_pointer[8]();
		}
	}

	if ((status & AES_ISR_EOPAD) && (mask & AES_IMR_EOPAD)) {
		if (aes_callback_pointer[9]) {
			aes_callback_pointer[9]();
		}
	}
#endif /* SAM4C || SAM4CP || SAM4CM */
}
