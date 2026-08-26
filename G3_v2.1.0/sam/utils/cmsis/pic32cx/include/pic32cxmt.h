/* ---------------------------------------------------------------------------- */
/*                Microchip Microcontroller Software Support                    */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2022, Microchip Technology Inc.                                */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* Microchip's name may not be used to endorse or promote products derived from */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY MICROCHIP "AS IS" AND ANY EXPRESS  */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES */
/* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT    */
/* ARE DISCLAIMED. IN NO EVENT SHALL MICROCHIP BE LIABLE FOR ANY DIRECT,        */
/* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           */
/* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  */
/* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF     */
/* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.            */
/* ---------------------------------------------------------------------------- */

#ifndef _PIC32CXMT_
#define _PIC32CXMT_

#if defined __PIC32CX1025MTG64__
  #include "pic32cx1025mtg64.h"

#elif defined __PIC32CX1025MTG128__
  #include "pic32cx1025mtg128.h"

#elif defined __PIC32CX2051MTG64__
  #include "pic32cx2051mtg64.h"

#elif defined __PIC32CX2051MTG128__
  #include "pic32cx2051mtg128.h"

#elif defined __PIC32CX5112MTG128__
  #include "pic32cx5112mtg128.h"

#elif defined __PIC32CX1025MTC128__
  #include "pic32cx1025mtc128.h"
#elif defined __PIC32CX1025MTC128S1__
  #include "pic32cx1025mtc128S1.h"

#elif defined __PIC32CX2051MTC128__
  #include "pic32cx2051mtc128.h"
#elif defined __PIC32CX2051MTC128S1__
  #include "pic32cx2051mtc128S1.h"

#elif defined __PIC32CX5112MTC128__
  #include "pic32cx5112mtc128.h"
#elif defined __PIC32CX5112MTC128S1__
  #include "pic32cx5112mtc128S1.h"

#elif defined __PIC32CX1025MTSH128__
  #include "pic32cx1025mtsh128.h"
#elif defined __PIC32CX1025MTSH128S1__
  #include "pic32cx1025mtsh128S1.h"

#elif defined __PIC32CX2051MTSH128__
  #include "pic32cx2051mtsh128.h"
#elif defined __PIC32CX2051MTSH128S1__
  #include "pic32cx2051mtsh128S1.h"

#elif defined __PIC32CX5112MTSH128__
  #include "pic32cx5112mtsh128.h"
#elif defined __PIC32CX5112MTSH128S1__
  #include "pic32cx5112mtsh128S1.h"

#else
  #error Library does not support the specified device.
#endif

#endif /* _PIC32CXMT_ */
