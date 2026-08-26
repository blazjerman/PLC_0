/**
 * \file
 *
 * \brief PIC32CX architecture specific IOPORT service implementation header file.
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
#ifndef IOPORT_PIC32CX_H
#define IOPORT_PIC32CX_H

#include <sysclk.h>

#define IOPORT_CREATE_PIN(port, pin)    ((IOPORT_ ## port) * 32 + (pin))
#define IOPORT_BASE_ADDRESS             (uintptr_t)PIOA
#define IOPORT_PIO_OFFSET               ((uintptr_t)PIOB - (uintptr_t)PIOA)

#define IOPORT_PIOA     0
#define IOPORT_PIOB     1
#define IOPORT_PIOC     2
#ifdef ID_PIOD
#define IOPORT_PIOD     3
#endif

/**
 * \weakgroup ioport_group
 * \section ioport_modes IOPORT Modes
 *
 * For details on these please see the SAM Manual.
 *
 * @{
 */

/** \name IOPORT Mode bit definitions */
/** @{ */
#define IOPORT_MODE_MUX_MASK            (0x7 << 0) /*!< MUX bits mask */

#define IOPORT_MODE_MUX_A               (1 << 0)   /*!< MUX function A */
#define IOPORT_MODE_MUX_B               (2 << 0)   /*!< MUX function B */
#define IOPORT_MODE_MUX_C               (3 << 0)   /*!< MUX function C */
#ifdef ID_PIOD
#define IOPORT_MODE_MUX_D               (4 << 0)   /*!< MUX function D */
#endif

#define IOPORT_MODE_DEFAULT             (0)
#define IOPORT_MODE_PULLUP              (1 << 3)   /*!< Pull-up */
#define IOPORT_MODE_PULLDOWN            (1 << 4)   /*!< Pull-down */
#define IOPORT_MODE_OPEN_DRAIN          (1 << 5)   /*!< Open drain */
#define IOPORT_MODE_GLITCH_FILTER       (1 << 6)   /*!< Glitch filter */
#define IOPORT_MODE_DEBOUNCE            (1 << 7)   /*!< Input debounce */
/** @} */

/** @} */

typedef uint32_t ioport_mode_t;
typedef uint32_t ioport_pin_t;
typedef uint32_t ioport_port_t;
typedef uint32_t ioport_port_mask_t;

__always_inline static ioport_port_t arch_ioport_pin_to_port_id(ioport_pin_t pin)
{
	return pin >> 5;
}

__always_inline static PioGroup *arch_ioport_port_to_base(ioport_port_t port)
{
	if (port == IOPORT_PIOA) {
		return (PioGroup *)(uintptr_t)PIOA;
	} else if (port == IOPORT_PIOB) {
		return (PioGroup *)(uintptr_t)PIOB;
	} else if (port == IOPORT_PIOC) {
		return (PioGroup *)(uintptr_t)PIOC;
	}

#ifdef ID_PIOD
	else if (port == IOPORT_PIOD) {
		return (PioGroup *)(uintptr_t)PIOD;
	}
#endif
	return 0;
}

__always_inline static void arch_ioport_enable_port(ioport_port_t port,
		ioport_port_mask_t mask)
{
	(void)port;
	(void)mask;
}

__always_inline static void arch_ioport_disable_port(ioport_port_t port,
		ioport_port_mask_t mask)
{
	(void)port;
	(void)mask;
}

__always_inline static void arch_ioport_enable_pin(ioport_pin_t pin)
{
	(void)pin;
}

__always_inline static void arch_ioport_disable_pin(ioport_pin_t pin)
{
	(void)pin;
}

__always_inline static PioGroup *arch_ioport_pin_to_base(ioport_pin_t pin)
{
	return arch_ioport_port_to_base(arch_ioport_pin_to_port_id(pin));
}

__always_inline static ioport_port_mask_t arch_ioport_pin_to_mask(ioport_pin_t pin)
{
	return 1U << (pin & 0x1F);
}

__always_inline static void arch_ioport_init(void)
{
#ifdef ID_PIOA
	sysclk_enable_peripheral_clock(ID_PIOA);
#endif
#ifdef ID_PIOD
#if (PIC32CX)
#ifdef CONFIG_CPCLK_ENABLE
	sysclk_enable_peripheral_clock(ID_PIOD);
#endif
#else
	sysclk_enable_peripheral_clock(ID_PIOD);
#endif
#endif
}

__always_inline static void arch_ioport_set_port_mode(ioport_port_t port,
		ioport_port_mask_t mask, ioport_mode_t mode)
{
	PioGroup *pio = arch_ioport_port_to_base(port);
	uint32_t ul_cfgr = 0;

	/* Select mask */
	pio->PIO_MSKR = mask;

	if (mode & IOPORT_MODE_PULLUP) {
		ul_cfgr |= PIO_CFGR_PUEN;
	}

	if (mode & IOPORT_MODE_PULLDOWN) {
		ul_cfgr |= PIO_CFGR_PDEN;
	}

	if (mode & IOPORT_MODE_OPEN_DRAIN) {
		ul_cfgr |= PIO_CFGR_OPD;
	}

	if (mode & (IOPORT_MODE_GLITCH_FILTER)) {
		ul_cfgr |= PIO_CFGR_IFEN;
	}

	if (mode & (IOPORT_MODE_DEBOUNCE)) {
		ul_cfgr |= PIO_CFGR_IFSCEN;
	}

	if ((mode & IOPORT_MODE_MUX_MASK) == IOPORT_MODE_MUX_A) {
		ul_cfgr |= PIO_CFGR_FUNC_PERIPH_A;
	} else if ((mode & IOPORT_MODE_MUX_MASK) == IOPORT_MODE_MUX_B) {
		ul_cfgr |= PIO_CFGR_FUNC_PERIPH_B;
	} else if ((mode & IOPORT_MODE_MUX_MASK) == IOPORT_MODE_MUX_C) {
		ul_cfgr |= PIO_CFGR_FUNC_PERIPH_C;
	}

#ifdef ID_PIOD
	else if ((mode & IOPORT_MODE_MUX_MASK) == IOPORT_MODE_MUX_D) {
		ul_cfgr |= PIO_CFGR_FUNC_PERIPH_D;
	}
#endif

	/* Set configuration */
	pio->PIO_CFGR = ul_cfgr;
}

__always_inline static void arch_ioport_set_pin_mode(ioport_pin_t pin,
		ioport_mode_t mode)
{
	arch_ioport_set_port_mode(arch_ioport_pin_to_port_id(pin),
			arch_ioport_pin_to_mask(pin), mode);
}

__always_inline static void arch_ioport_set_port_dir(ioport_port_t port,
		ioport_port_mask_t mask, enum ioport_direction group_direction)
{
	PioGroup *pio = arch_ioport_port_to_base(port);

	/* Select mask */
	pio->PIO_MSKR = mask;

	if (group_direction == IOPORT_DIR_OUTPUT) {
		pio->PIO_CFGR |= PIO_CFGR_DIR;
	} else if (group_direction == IOPORT_DIR_INPUT) {
		pio->PIO_CFGR &= ~PIO_CFGR_DIR;
	}
}

__always_inline static void arch_ioport_set_pin_dir(ioport_pin_t pin,
		enum ioport_direction dir)
{
	PioGroup *pio = arch_ioport_pin_to_base(pin);

	/* Select mask */
	pio->PIO_MSKR = arch_ioport_pin_to_mask(pin);
	/* Select GPIO Function */
	pio->PIO_CFGR &= ~PIO_CFGR_FUNC_Msk;

	if (dir == IOPORT_DIR_OUTPUT) {
		pio->PIO_CFGR |= PIO_CFGR_DIR;
	} else if (dir == IOPORT_DIR_INPUT) {
		pio->PIO_CFGR &= ~PIO_CFGR_DIR;
	}
}

__always_inline static void arch_ioport_set_pin_level(ioport_pin_t pin,
		bool level)
{
	PioGroup *pio = arch_ioport_pin_to_base(pin);

	if (level) {
		pio->PIO_SODR = arch_ioport_pin_to_mask(pin);
	} else {
		pio->PIO_CODR = arch_ioport_pin_to_mask(pin);
	}
}

__always_inline static void arch_ioport_set_port_level(ioport_port_t port,
		ioport_port_mask_t mask, enum ioport_value level)
{
	PioGroup *pio = arch_ioport_port_to_base(port);

	if (level) {
		pio->PIO_SODR = mask;
	} else {
		pio->PIO_CODR = mask;
	}
}

__always_inline static bool arch_ioport_get_pin_level(ioport_pin_t pin)
{
	PioGroup *port = arch_ioport_pin_to_base(pin);

	return port->PIO_PDSR & arch_ioport_pin_to_mask(pin);
}

__always_inline static ioport_port_mask_t arch_ioport_get_port_level(
		ioport_port_t port, ioport_port_mask_t mask)
{
	PioGroup *pio = arch_ioport_port_to_base(port);

	return pio->PIO_PDSR & mask;
}

__always_inline static void arch_ioport_toggle_pin_level(ioport_pin_t pin)
{
	PioGroup *port = arch_ioport_pin_to_base(pin);
	ioport_port_mask_t mask = arch_ioport_pin_to_mask(pin);

	if (port->PIO_PDSR & arch_ioport_pin_to_mask(pin)) {
		port->PIO_CODR = mask;
	} else {
		port->PIO_SODR = mask;
	}
}

__always_inline static void arch_ioport_toggle_port_level(ioport_port_t port,
		ioport_port_mask_t mask)
{
	PioGroup *pio = arch_ioport_port_to_base(port);

	/* Select mask */
	pio->PIO_MSKR = mask;

	/* Toggle status */
	pio->PIO_ODSR ^= mask;
}

__always_inline static void arch_ioport_set_port_sense_mode(ioport_port_t port,
		ioport_port_mask_t mask, enum ioport_sense pin_sense)
{
	PioGroup *pio = arch_ioport_port_to_base(port);
	uint32_t ul_cfgr = 0;

	/* Select mask */
	pio->PIO_MSKR = mask;

	/* Read configuration: If several bits are set in PIO_MSKRx, then
	 * the read configuration in PIO_CFGRx is the configuration of
	 * the I/O line with the lowest index.
	 */
	ul_cfgr = pio->PIO_CFGR;

	/* Clear Event Selection */
	ul_cfgr &= ~PIO_CFGR_EVTSEL_Msk;

	/* Set Event Selection */
	switch (pin_sense) {
	case IOPORT_SENSE_LEVEL_LOW:
		ul_cfgr |= PIO_CFGR_EVTSEL_LOW;
		break;

	case IOPORT_SENSE_LEVEL_HIGH:
		ul_cfgr |= PIO_CFGR_EVTSEL_HIGH;
		break;

	case IOPORT_SENSE_FALLING:
		ul_cfgr |= PIO_CFGR_EVTSEL_FALLING;
		break;

	case IOPORT_SENSE_RISING:
		ul_cfgr |= PIO_CFGR_EVTSEL_RISING;
		break;

	default:
		ul_cfgr |= PIO_CFGR_EVTSEL_BOTH;
		return;
	}

	/* Set configuration */
	pio->PIO_CFGR = ul_cfgr;
}

__always_inline static void arch_ioport_set_pin_sense_mode(ioport_pin_t pin,
		enum ioport_sense pin_sense)
{
	arch_ioport_set_port_sense_mode(arch_ioport_pin_to_port_id(pin),
			arch_ioport_pin_to_mask(pin), pin_sense);
}

#endif /* IOPORT_PIC32CX_H */
