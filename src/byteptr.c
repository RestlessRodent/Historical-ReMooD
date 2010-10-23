// Emacs style mode select   -*- C++ -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@remood.org)
// Project Co-Leader: RedZTag                (jostol@remood.org)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// Copyright (C) 2007-2010 GhostlyDeath (ghostlydeath@remood.org)
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Correct READ, WRITE memory handling

#include "byteptr.h"
#include "doomdef.h"

#define BP_MERGE(a,b) a##b

#if defined(__arm__) || defined(_M_ARM) || defined(__sparc__) || defined(__sparc)
	/* Access to pointer data for system that can't handle unaligned access */
	// Lets say we have the following data:
	// { 01  23  45  67  |  89  AB  CD  EF }
	//      [*DEREF           ]
	// On normal systems we can just dereference as normal, but on some systems
	// such as ARM, we cannot do this. Instead we have to dereference both sides
	// then merge the data together.
	// Or we could just read byte by byte.

	#define BP_READ(x) x BP_MERGE(Read,x)(x** const Ptr)\
	{\
		x Ret = 0;\
		UInt8* p8;\
		size_t i;\
		\
		if (!Ptr || !(*Ptr))\
			return 0;\
		\
		p8 = (UInt8*)*Ptr;\
		for (i = 0; i < sizeof(x); i++)\
			((UInt8*)&Ret)[i] = p8[i];\
		\
		(*Ptr)++;\
		return Ret;\
	}
#else
	/* Normal Pointer Access */
	#define BP_READ(x) x BP_MERGE(Read,x)(x** const Ptr)\
	{\
		x Ret;\
		\
		if (!Ptr || !(*Ptr))\
			return 0;\
		\
		Ret = **Ptr;\
		(*Ptr)++;\
		return Ret;\
	}
#endif

BP_READ(Int8)
BP_READ(Int16)
BP_READ(Int32)
BP_READ(UInt8)
BP_READ(UInt16)
BP_READ(UInt32)
BP_READ(wchar_t)

void ReadStringN(char** const Ptr, char* Dest, size_t n)
{
	size_t i = 0;

	if (!Ptr || !(*Ptr) || !Dest || n == 0)
		return;
		
	/* Copy into */
	while (**Ptr && i < n)
	{
		*Dest = **Ptr;
		Dest++;
		(*Ptr)++;
		i++;
	}
	
	/* Remaining? */
	while (i < n)
	{
		*Dest = 0;
		Dest++;
		(*Ptr)++;	// GhostlyDeath <March 17, 2010> -- This wasn't here before so I decided to make it a padded read...
		i++;
	}
}

#define BP_WRITE(x) void BP_MERGE(Write,x)(x** const Ptr, const x Val)\
{\
	if (!Ptr || !(*Ptr))\
		return;\
	**Ptr = Val;\
	(*Ptr)++;\
}

BP_WRITE(Int8)
BP_WRITE(Int16)
BP_WRITE(Int32) 
BP_WRITE(UInt8)
BP_WRITE(UInt16)
BP_WRITE(UInt32)
BP_WRITE(wchar_t)

void WriteString(char** const Ptr, const char* Val)
{
	if (!Ptr || !(*Ptr) || !Val)
		return;

	while (*Val)
	{
		**Ptr = *Val;
		(*Ptr)++;
		Val++;
	}
}

void WriteStringN(char** const Ptr, const char* Val, const size_t n)
{
	size_t i = 0;

	if (!Ptr || !(*Ptr) || !Val || n == 0)
		return;

	while (*Val && i < n)
	{
		**Ptr = *Val;
		(*Ptr)++;
		Val++;
		i++;
	}

	while (i < n)				// Pad
	{
		**Ptr = 0;
		(*Ptr)++;
		i++;
	}
}

#if !defined(_REMOOD_NOINT64)
	BP_READ(Int64)
	BP_READ(UInt64)
	BP_WRITE(Int64)
	BP_WRITE(UInt64)
#endif

