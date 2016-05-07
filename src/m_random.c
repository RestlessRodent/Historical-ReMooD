// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "j.h"

#include "m_random.h"

static jclass __randomclass = NULL;
static jobject __singlerandom = NULL;
static jmethodID __next = NULL;
static jmethodID __nextSigned = NULL;
static jmethodID __getIndex = NULL;
static jmethodID __setIndex = NULL;
static jobject __globalrandom = NULL;
static jmethodID __stdnextint = NULL;

void __loadRandomClass(void)
{
	jclass stdrandom;
	jmethodID stdcon;
	jmethodID con;
	
	/** Need to load the data? */
	if (__randomclass == NULL)
	{
		__randomclass = (*g_Env)->FindClass(g_Env,
			"org/remood/remood/core/random/DoomRandom");
			
		if (__randomclass == NULL)
			I_Error("JAVA: Could not find DoomRandom!?");
		
		/** Allocate that class. */
		con = (*g_Env)->GetMethodID(g_Env, __randomclass, "<init>", "()V");
		__singlerandom = (*g_Env)->NewObject(g_Env, __randomclass, con);
		
		/** Bind methods. */
		__next = (*g_Env)->GetMethodID(g_Env, __randomclass, "next", "()I");
		__nextSigned = (*g_Env)->GetMethodID(g_Env, __randomclass,
			"nextSigned", "()I");
		__getIndex = (*g_Env)->GetMethodID(g_Env, __randomclass, "getIndex",
			"()I");
		__setIndex = (*g_Env)->GetMethodID(g_Env, __randomclass, "setIndex",
			"(I)Lorg/remood/remood/core/random/DoomRandom;");
		
		/** Setup global random generator. */
		stdrandom = (*g_Env)->FindClass(g_Env, "java/util/Random");
		
		if (stdrandom == NULL)
			I_Error("JAVA: Could not find java/util/Random!?");
		
		stdcon = (*g_Env)->GetMethodID(g_Env, stdrandom, "<init>", "()V");
		__globalrandom = (*g_Env)->NewObject(g_Env, stdrandom, stdcon);
		__stdnextint = (*g_Env)->GetMethodID(g_Env, stdrandom, "nextInt",
			"(I)I");
	}
}

uint8_t P_Random(void)
{
	/** Load PRNG. */
	__loadRandomClass();
	
	/** Get value. */
	return (uint8_t)((*g_Env)->CallIntMethod(g_Env, __singlerandom, __next));
}

int P_SignedRandom(void)
{
	/** Load PRNG. */
	__loadRandomClass();
	
	/** Get value. */
	return (int)((*g_Env)->CallIntMethod(g_Env, __singlerandom, __nextSigned));
}

void M_ClearRandom(void)
{
	/** Traditionally this just calls set. */
	P_SetRandIndex(0);
}

uint8_t P_GetRandIndex(void)
{
	/** Load PRNG. */
	__loadRandomClass();
	
	return (uint8_t)((*g_Env)->CallIntMethod(g_Env, __singlerandom,
		__getIndex));
}

void P_SetRandIndex(uint8_t rindex)
{
	/** Load PRNG. */
	__loadRandomClass();
	
	(*g_Env)->CallObjectMethod(g_Env, __singlerandom, __setIndex, (int)rindex);
}

uint8_t M_Random(void)
{
	/** Load PRNG. */
	__loadRandomClass();
	
	return (uint8_t)((*g_Env)->CallIntMethod(g_Env, __globalrandom,
		__stdnextint, 256));
}


