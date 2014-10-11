// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: C Compiling Stubs

#ifndef __CCSTUB_H__
#define __CCSTUB_H__

#if defined(__REMOOD_USECCSTUB)

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/* Palm OS Hacks */
#if defined(__REMOOD_SYSTEM_PALMOS)
	// Prevent Palm OS's fixed point routines from taking shape
	#define __FIXEDMATH_H__
	
	// Standard Includes
	#include <NetMgr.h>							// Networking
	#include <VFSMgr.h>							// File Handling
	#include <unix_stdarg.h>					// va_list
	#include <StringMgr.h>						// Strings
#endif

/*******************
*** DEFINE HACKS ***
*******************/

/* Palm OS Hacks */
#if defined(__REMOOD_SYSTEM_PALMOS)
	// Map VFS File to C one
	typedef FileRef FILE;
	
	// Missing Types
	typedef __PTRDIFF_TYPE__ ptrdiff_t;
	
	// Max int
	#ifndef INT_MAX
		#define INT_MAX __INT_MAX__
	#endif
	
	// Min int
	#ifndef INT_MIN
		#define INT_MIN 0x80000000L
	#endif
	
	// Max Short
	#ifndef SHRT_MAX
		#define SHRT_MAX __SHRT_MAX__
	#endif
	
	// Char bits
	#ifndef CHAR_BIT
		#define CHAR_BIT __CHAR_BIT__
	#endif
	
	// offsetof() macro goes by different name
	#ifndef offsetof
		#define offsetof(t,m) OffsetOf(t,m)
	#endif
	
	// File Handling
	#define SEEK_SET vfsOriginBeginning
	#define SEEK_END vfsOriginEnd
	#define SEEK_CUR vfsOriginCurrent
	
	#define R_OK vfsFileAttrReadOnly
	#define W_OK vfsFileAttrReadOnly	// All files are writeable mostly
	
	// Functions that can just be defined away
	#define strcasecmp StrCaselessCompare
	#define strncasecmp StrNCaselessCompare
	#define strcmp StrCompare
	#define strncmp StrNCompare
	#define memmove MemMove
	#define memcpy MemMove
	#define memcmp MemCmp
	#define malloc MemPtrNew
	#define atoi StrAToI
	#define memset(s,c,n) MemSet((s), (n), (c))
	#define strlen StrLen
	#define strcat StrCat
	#define strncat StrNCat
	#define strncpy StrNCopy
	#define strcpy StrCopy
	//#define isalnum TxtCharIsAlNum
	//#define isspace TxtCharIsSpace
	//#define isdigit TxtCharIsDigit
	//#define isxdigit TxtCharIsHex
	#define strchr StrChr
#endif

/*****************
*** PROTOTYPES ***
*****************/

int mkdir(const char* const a_PathName);
int socket(int domain, int type, int protocol);

/*****************************************************************************/

#endif /* __REMOOD_USECCSTUB */

#endif /* __CCSTUB_H__ */

