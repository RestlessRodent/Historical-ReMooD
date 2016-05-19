// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

#include "i_system.h"
#include "doomtype.h"
#include "j.h"

static jclass __Thread = NULL;
static jmethodID __Thread_sleep = NULL;

static jclass __TimeUtils = NULL;
static jmethodID __TimeUtils_getMilliseconds = NULL;

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	// Need to find the timer class?
	if (__TimeUtils == NULL)
	{
		__TimeUtils = J_FindClass("org/remood/remood/core/system/TimeUtils");
		__TimeUtils_getMilliseconds = J_GetStaticMethodID(__TimeUtils,
			"getMilliseconds", "()J");
		
		// Fail?
		if (__TimeUtils_getMilliseconds == NULL)
			I_Error("Could not find __TimeUtils_getMilliseconds");
	}
	
	// Call method
	return (uint32_t)J_CallStaticLongMethod(__TimeUtils,
		__TimeUtils_getMilliseconds);
}

/* I_WaitVBL() -- Wait for vertical blank */
void I_WaitVBL(int count)
{
	// Need to init thread sleep?
	if (__Thread == NULL)
	{
		__Thread = J_FindClass("java/lang/Thread");
		__Thread_sleep = J_GetStaticMethodID(__Thread, "sleep", "(J)V");
	}
	
	// Call sleep
	J_CallStaticVoidMethod(__Thread, __Thread_sleep, (jlong)count);
}

