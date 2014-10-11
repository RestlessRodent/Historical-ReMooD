// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: C Compiling Stubs

#if defined(__REMOOD_USECCSTUB)

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/



#if defined(__palmos__)
	#include <PalmOS.h>
	#include <StringMgr.h>
	#include <VFSMgr.h>
	#include <PceNativeCall.h>
#endif

/**************
*** GLOBALS ***
**************/

const void *gEmulStateP = 0;
Call68KFuncType *gCall68KFuncP = 0;

/****************
*** FUNCTIONS ***
****************/

/* mkdir() -- Makes directory */
int mkdir(const char* const a_PathName)
{
	// NOT IMPLEMENTED
	return 0;
}

/* socket() -- Opens Socket */
int socket(int domain, int type, int protocol)
{
#if defined(__palmos__)
	/* Palm OS */
	return -1;
	
#else
	/* NOT IMPLEMENTED */
	return -1;
#endif
}

/*****************************************************************************/

#endif /* __REMOOD_USECCSTUB */

