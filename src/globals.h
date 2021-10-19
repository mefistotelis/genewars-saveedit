/******************************************************************************/
/** @file globals.h
 * Global configuration file.
 * @par Purpose:
 *     Header file with global definitions for GeneWars Savegame Editor.
 * @par Comment:
 *     Defines basic includes and definitions, used in whole program.
 * @author   Jon Skeet, Tomasz Lis
 * @date     08 sep 1998 - 22 Jul 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef GWSAV_GLOBALS_H
#define GWSAV_GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#if defined(unix) && !defined(GO32)
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#if !defined(stricmp)
#define stricmp strcasecmp
#endif
#elif defined(MSDOS)
#include <dos.h>
#include <process.h>
#endif

#if defined(BUILD_DLL)
# define DLLIMPORT __declspec (dllexport)
#elif defined(BUILD_SINGLE)
# define DLLIMPORT 
#else // Not defined BUILD_DLL
# define DLLIMPORT __declspec (dllimport)
#endif

// Basic Definitions

#if defined(unix) && !defined (GO32)
#define SEPARATOR "/"
#else
#define SEPARATOR "\\"
#endif

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// Buffer sizes

#define DISKPATH_SIZE   256
#define LINEMSG_SIZE    160
#define READ_BUFSIZE    256

#define PROGRAM_NAME "GWSavEd"

// Return values for verification functions
#define VERIF_ERROR   0
#define VERIF_OK      1
#define VERIF_WARN    2

// Return values for all other functions
#define ERR_NONE           0
#define ERR_INTERNAL     -31
// Note: error codes -1..-79 are reserved standard C library errors with sign reverted.
//    these are defined in errno.h
#define ERR_BASE_RNC      -90

#endif // GWSAV_GLOBALS_H
