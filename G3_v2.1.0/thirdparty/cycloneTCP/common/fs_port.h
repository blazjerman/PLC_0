/**
 * @file fs_port.h
 * @brief File system abstraction layer
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

#ifndef _FS_PORT_H
#define _FS_PORT_H

//Dependencies
#include "fs_port_config.h"
#include "os_port.h"
#include "date_time.h"
#include "error.h"

//Number of files that can be opened simultaneously
#ifndef FS_MAX_FILES
   #define FS_MAX_FILES 8
#elif (FS_MAX_FILES < 1)
   #error FS_MAX_FILES parameter is not valid
#endif

//Number of directories that can be opened simultaneously
#ifndef FS_MAX_DIRS
   #define FS_MAX_DIRS 8
#elif (FS_MAX_DIRS < 1)
   #error FS_MAX_DIRS parameter is not valid
#endif

//Maximum filename length
#ifndef FS_MAX_NAME_LEN
   #define FS_MAX_NAME_LEN 127
#elif (FS_MAX_NAME_LEN < 11)
   #error FS_MAX_NAME_LEN parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief File attributes
 **/

typedef enum
{
   FS_FILE_ATTR_READ_ONLY   = 0x01,
   FS_FILE_ATTR_HIDDEN      = 0x02,
   FS_FILE_ATTR_SYSTEM      = 0x04,
   FS_FILE_ATTR_VOLUME_NAME = 0x08,
   FS_FILE_ATTR_DIRECTORY   = 0x10,
   FS_FILE_ATTR_ARCHIVE     = 0x20
} FsFileAttributes;


/**
 * @brief File access mode
 **/

typedef enum
{
   FS_FILE_MODE_READ   = 1,
   FS_FILE_MODE_WRITE  = 2,
   FS_FILE_MODE_CREATE = 4,
   FS_FILE_MODE_TRUNC  = 8
} FsFileMode;


/**
 * @brief File seek origin
 **/

typedef enum
{
   FS_SEEK_SET = 0,
   FS_SEEK_CUR = 1,
   FS_SEEK_END = 2
} FsSeekOrigin;


/**
 * @brief Directory entry
 **/

typedef struct
{
   uint32_t attributes;
   uint32_t size;
   DateTime modified;
   char_t name[FS_MAX_NAME_LEN + 1];
} FsDirEntry;


/**
 * @brief File handle
 **/

typedef void FsFile;


/**
 * @brief Directory handle
 **/

typedef void FsDir;


//File system abstraction layer
error_t fsInit(void);

bool_t fsFileExists(const char_t *path);
error_t fsGetFileSize(const char_t *path, uint32_t *size);
error_t fsRenameFile(const char_t *oldPath, const char_t *newPath);
error_t fsDeleteFile(const char_t *path);

FsFile *fsOpenFile(const char_t *path, uint_t mode);
error_t fsSeekFile(FsFile *file, int_t offset, uint_t origin);
error_t fsWriteFile(FsFile *file, void *data, size_t length);
error_t fsReadFile(FsFile *file, void *data, size_t size, size_t *length);
void fsCloseFile(FsFile *file);

bool_t fsDirExists(const char_t *path);
error_t fsCreateDir(const char_t *path);
error_t fsRemoveDir(const char_t *path);

FsDir *fsOpenDir(const char_t *path);
error_t fsReadDir(FsDir *dir, FsDirEntry *dirEntry);
void fsCloseDir(FsDir *dir);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
