/**
 * \file
 *
 * \brief FLEXCOM driver for SAM.
 *
 * Copyright (c) 2014-2020 Microchip Technology Inc. and its subsidiaries.
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

#include "flexcom.h"
#include "sysclk.h"
#include "sleepmgr.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond


/**
 * \brief Enable the FLEXCOM module.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 *
 */
void flexcom_enable(Flexcom *p_flexcom)
{
#if SAMG55
    #define PMC_PCK_PRES_CLK_1  PMC_PCK_PRES(0)
#endif

	sleepmgr_lock_mode(SLEEPMGR_ACTIVE);
	/* Enable PMC clock for FLEXCOM */
#ifdef ID_FLEXCOM7
	 if (p_flexcom == FLEXCOM7) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM7);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_7);
		pmc_switch_pck_to_mck(PMC_PCK_7, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM6
	if (p_flexcom == FLEXCOM6) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM6);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_7);
		pmc_switch_pck_to_mck(PMC_PCK_7, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM5
	if (p_flexcom == FLEXCOM5) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM5);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_7);
		pmc_switch_pck_to_mck(PMC_PCK_7, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM4
	if (p_flexcom == FLEXCOM4) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM4);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_7);
		pmc_switch_pck_to_mck(PMC_PCK_7, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM3
	if (p_flexcom == FLEXCOM3) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM3);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_6);
		pmc_switch_pck_to_mck(PMC_PCK_6, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM2
	if (p_flexcom == FLEXCOM2) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM2);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_6);
		pmc_switch_pck_to_mck(PMC_PCK_6, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM1
	if (p_flexcom == FLEXCOM1) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM1);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_6);
		pmc_switch_pck_to_mck(PMC_PCK_6, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM0
	if (p_flexcom == FLEXCOM0) {
		sysclk_enable_peripheral_clock(ID_FLEXCOM0);
#if !(PIC32CX)
		/* Enable PCK output */
		pmc_disable_pck(PMC_PCK_6);
		pmc_switch_pck_to_mck(PMC_PCK_6, PMC_PCK_PRES_CLK_1);
		pmc_enable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
	{
		Assert(false);
	}
}

/**
 * \brief Disable the FLEXCOM module.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 *
 */
void flexcom_disable(Flexcom *p_flexcom)
{
	sleepmgr_unlock_mode(SLEEPMGR_ACTIVE);
	/* Enable PMC clock for FLEXCOM */
#ifdef ID_FLEXCOM7
	 if (p_flexcom == FLEXCOM7) {
		 sysclk_disable_peripheral_clock(ID_FLEXCOM7);
#if !(PIC32CX)
		 /* Disable PCK output */
		pmc_disable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM6
	if (p_flexcom == FLEXCOM6) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM6);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM5
	if (p_flexcom == FLEXCOM5) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM5);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM4
	if (p_flexcom == FLEXCOM4) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM4);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_7);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM3
	if (p_flexcom == FLEXCOM3) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM3);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM2
	if (p_flexcom == FLEXCOM2) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM2);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM1
	if (p_flexcom == FLEXCOM1) {
		sysclk_disable_peripheral_clock(ID_FLEXCOM1);
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
	} else
#endif
#ifdef ID_FLEXCOM0
	if (p_flexcom == FLEXCOM0) {
#if !(PIC32CX)
		/* Disable PCK output */
		pmc_disable_pck(PMC_PCK_6);
#endif /* !(PIC32CX) */
		sysclk_disable_peripheral_clock(ID_FLEXCOM0);
	} else
#endif
	{
		Assert(false);
	}
}

/**
 * \brief Set the FLEXCOM opration mode.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 * \param opmode  Opration mode.
 *
 */
void flexcom_set_opmode(Flexcom *p_flexcom, enum flexcom_opmode opmode)
{
	p_flexcom->FLEXCOM_MR = opmode;
}

/**
 * \brief Set the FLEXCOM opration mode.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 * \param opmode  Opration mode.
 *
 */
void flexcom_get_opmode(Flexcom *p_flexcom, enum flexcom_opmode *opmode)
{
	*opmode = (enum flexcom_opmode)(p_flexcom->FLEXCOM_MR & FLEXCOM_MR_OPMODE_Msk);
}

/**
 * \brief Write to the FLEXCOM.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 * \param data  Data to be tansfer.
 *
 */
void flexcom_write(Flexcom *p_flexcom, uint32_t data)
{
	p_flexcom->FLEXCOM_THR = data;
}

/**
 * \brief Read the FLEXCOM data.
 *
 * \param p_flexcom  Pointer to a FLEXCOM instance.
 * \param data  Data received.
 *
 */
void flexcom_read(Flexcom *p_flexcom, uint32_t *data)
{
	*data = p_flexcom->FLEXCOM_RHR;
}

#if PIC32CX
/**
 * \brief Get FLEXCOM PDC base address.
 *
 * \param p_flexcom Pointer to a FLEXCOM instance.
 *
 * \return FLEXCOM PDC registers base for PDC driver to access.
 */
Pdc *flexcom_get_pdc_base(Flexcom *p_flexcom)
{
	Pdc *p_pdc_base;

	p_pdc_base = (Pdc *)NULL;

	if (p_flexcom == FLEXCOM0) {
		p_pdc_base = PDC_FLEXCOM0;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM1) {
		p_pdc_base = PDC_FLEXCOM1;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM2) {
		p_pdc_base = PDC_FLEXCOM2;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM3) {
		p_pdc_base = PDC_FLEXCOM3;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM4) {
		p_pdc_base = PDC_FLEXCOM4;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM5) {
		p_pdc_base = PDC_FLEXCOM5;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM6) {
		p_pdc_base = PDC_FLEXCOM6;
		return p_pdc_base;
	}
	else if (p_flexcom == FLEXCOM7) {
		p_pdc_base = PDC_FLEXCOM7;
		return p_pdc_base;
	}

	return p_pdc_base;
}
#endif /* PIC32CX */

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
