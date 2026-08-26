/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>
#include "status_codes.h"
#include "conf_board.h"
#ifdef CONSOLE_UART
#include "uart_serial.h"
#endif

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int c, FILE *f)
{
#ifdef CONSOLE_UART
	return (usart_serial_putchar((usart_if)CONSOLE_UART, c));
#else
	return 0;
#endif
}

int fgetc(FILE *f)
{
	uint8_t uc_c;
#ifdef CONSOLE_UART
	usart_serial_getchar((usart_if)CONSOLE_UART, &uc_c);
#else
	uc_c = 0xFF;
#endif
	return (uc_c);
}

int ferror(FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}

void _ttywrch(int c)
{
	return;
}

int fflush(FILE *f)
{
	return 0;
}
	

void _sys_exit(int return_code)
{
label:  goto label;  /* endless loop */
}
