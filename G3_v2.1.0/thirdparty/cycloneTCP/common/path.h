/**
 * @file path.h
 * @brief Path manipulation helper functions
 *
 * @section License
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Eval.
 *
 * This software is provided in source form for a short-term evaluation only. The
 * evaluation license expires 90 days after the date you first download the software.
 *
 * If you plan to use this software in a commercial product, you are required to
 * purchase a commercial license from Oryx Embedded SARL.
 *
 * After the 90-day evaluation period, you agree to either purchase a commercial
 * license or delete all copies of this software. If you wish to extend the
 * evaluation period, you must contact sales@oryx-embedded.com.
 *
 * This evaluation software is provided "as is" without warranty of any kind.
 * Technical support is available as an option during the evaluation period.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.4
 **/

#ifndef _PATH_H
#define _PATH_H

//Dependencies
#include "os_port.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//Path manipulation helper functions
bool_t pathIsAbsolute(const char_t *path);
bool_t pathIsRelative(const char_t *path);

const char_t *pathFindFileName(const char_t *path);

void pathCanonicalize(char_t *path);

void pathAddSlash(char_t *path, size_t maxLen);
void pathRemoveSlash(char_t *path);

void pathCombine(char_t *path, const char_t *more, size_t maxLen);

bool_t pathMatch(const char_t *path, const char_t *pattern);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
