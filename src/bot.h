// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Bot Code

#ifndef __BOT_H__
#define __BOT_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/*****************
*** STRUCTURES ***
*****************/

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/****************
*** FUNCTIONS ***
****************/

void BOT_Init(void);
void BOT_Ticker(void);
void BOT_Add(const int32_t a_ArgC, const char** const a_ArgV);
void BOT_RegisterLevel(void);

void BOT_DestroyByPort(SN_Port_t* const a_Port);
void BOT_LeaveStasis(SN_Port_t* const a_Port);
void BOT_EnterStatis(SN_Port_t* const a_Port);

/*****************************************************************************/

#endif /* __BOT_H__ */

