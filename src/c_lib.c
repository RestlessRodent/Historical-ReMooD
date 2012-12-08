// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// ----------------------------------------------------------------------------
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Missing C Stuff

/***************
*** INCLUDES ***
***************/

#include "c_lib.h"
#include "m_fixed.h"

/****************
*** FUNCTIONS ***
****************/

/* C_strupr() -- Uppercase characters */
char* C_strupr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
		
	/* Run */
	x = s;
	while (*x)
	{
		*x = toupper(*x);
		x++;
	}
	
	return s;
}

/* C_strlwr() -- Lowercase characters */
char* C_strlwr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
		
	/* Run */
	x = s;
	while (*x)
	{
		*x = tolower(*x);
		x++;
	}
	
	return s;
}

/* C_strtoi32() -- Convert string to 32-bit */
int32_t C_strtoi32(const char* a_NPtr, char** a_EndPtr, int a_Base)
{
	long LongVal;
	
	/* Call native func */
	LongVal = strtol(a_NPtr, a_EndPtr, a_Base);
	
	/* Limit */
	if (LongVal > INT_MAX)
		LongVal = INT_MAX;
	else if (LongVal < INT_MIN)
		LongVal = INT_MIN;
	
	/* Return */
	return LongVal;
}

/* C_strtou32() -- Convert string to 32-bit (unsigned) */
uint32_t C_strtou32(const char* a_NPtr, char** a_EndPtr, int a_Base)
{
	unsigned long LongVal;
	
	/* Call native func */
	LongVal = strtoul(a_NPtr, a_EndPtr, a_Base);
	
	/* Limit */
	if (LongVal > UINT_MAX)
		LongVal = UINT_MAX;
	
	/* Return */
	return LongVal;
}

/* C_strtofx() -- Convert string to fixed */
int32_t C_strtofx(const char* a_NPtr, char** a_EndPtr)
{
#define FRACLIMIT 8
	fixed_t Temp, SubFrac;
	const char* c;
	bool_t Deci, Neg, FirstChar;
	int32_t Int, Frac;
	uint8_t FracCount;
	
	/* Check */
	if (!a_NPtr)
		return 0;
	
	/* Init */
	Temp = 0;
	Int = Frac = 0;
	FracCount = 0;
	Deci = false;
	Neg = false;
	FirstChar = true;
	
	/* Start reading */
	for (c = a_NPtr; *c; c++, FirstChar = false)
	{
		// Initial Space
		while (FirstChar && *c && isspace(*c))
			c++;
		
		// Negative?
		if (FirstChar && *c == '-')
			Neg = true;
		
		// Positive?
		else if (FirstChar && *c == '+')
			Neg = false;
		
		// Decimal Point
		else if (!FirstChar && !Deci && *c == '.')
			Deci = true;
		
		// Number
		else if (*c >= '0' && *c <= '9')
		{
			// Before decimal point
			if (!Deci)
			{
				Int *= 10;
				Int += (int32_t)(*c - '0');
			}
			
			// After decimal point
			else
			{
				if (FracCount < FRACLIMIT)
				{
					Frac *= 10;
					Frac += (int32_t)(*c - '0');
					FracCount++;
				}
			}
		}
		
		// Illegal?
		else
			break;
	}
	
	/* Limit Integers */
	if (Int < INT32_C(-32767))
		Int = INT32_C(-32767);
	else if (Int > INT32_C(32767))
		Int = INT32_C(32767);
	
	/* Limit Fraction */
	// If above FRACLIMIT places, decrease places
	while (FracCount > FRACLIMIT)
	{
		Frac /= 10;
		FracCount--;
	}
	
	// If below FRACLIMIT places, increase places
	while (FracCount < FRACLIMIT)
	{
		Frac *= 10;
		FracCount++;
	}
	
	// Setup Fraction
		// 1 / 65536 = 0.000015258789
#if 1
	SubFrac = Frac / INT32_C(1526);
#else
	SubFrac = 0;
	while (Frac > INT32_C(1526))
	{
		SubFrac++;
		Frac -= 1526;
	}
#endif
	
	// Combine
	Temp = (Int << FRACBITS) + SubFrac;
	
	/* Set left off pointer */
	if (a_EndPtr)
		*a_EndPtr = c;
	
	/* Return value */
	return Temp;
#undef FRACLIMIT
}

/**************************************
*** BYTE OPERATIONS FROM DOOMTYPE.H ***
**************************************/

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

#define BP_READ(w,x) x BP_MERGE(Read,w)(const x** const Ptr)\
	{\
		x Ret = 0;\
		uint8_t* p8;\
		size_t i;\
		\
		if (!Ptr || !(*Ptr))\
			return 0;\
		\
		p8 = (uint8_t*)*Ptr;\
		for (i = 0; i < sizeof(x); i++)\
			((uint8_t*)&Ret)[i] = p8[i];\
		\
		(*Ptr)++;\
		return Ret;\
	}
#else

/* Normal Pointer Access */
#define BP_READ(w,x) x BP_MERGE(Read,w)(const x** const Ptr)\
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

BP_READ(Int8R, int8_t)
BP_READ(Int16R, int16_t)
BP_READ(Int32R, int32_t)
BP_READ(Int64R, int64_t)
BP_READ(UInt8R, uint8_t)
BP_READ(UInt16R, uint16_t)
BP_READ(UInt32R, uint32_t)
BP_READ(UInt64R, uint64_t)

#define BP_WRITE(w,x) void BP_MERGE(Write,w)(x** const Ptr, const x Val)\
{\
	if (!Ptr || !(*Ptr))\
		return;\
	**Ptr = Val;\
	(*Ptr)++;\
}
BP_WRITE(Int8R, int8_t)
BP_WRITE(Int16R, int16_t)
BP_WRITE(Int32R, int32_t)
BP_WRITE(Int64R, int64_t)
BP_WRITE(UInt8R, uint8_t)
BP_WRITE(UInt16R, uint16_t)
BP_WRITE(UInt32R, uint32_t)
BP_WRITE(UInt64R, uint64_t)
#undef BP_READ
#undef BP_WRITE

#define BP_READREDO(w,x) x BP_MERGE(Read,w)R(const x** const Ptr)\

/* WriteString() -- Write a string of any length (bad) */
void WriteString(uint8_t** const Out, uint8_t* const String)
{
	size_t i;
	
	// Loop
	for (i = 0; String[i]; i++)
		WriteUInt8(Out, (uint8_t)String[i]);
	WriteUInt8(Out, 0);
}

/* WriteStringN() -- Write a string of n length */
void WriteStringN(uint8_t** const Out, uint8_t* const String, const size_t Count)
{
	size_t i;
	
	// Loop
	for (i = 0; i < Count && String[i]; i++)
		WriteUInt8(Out, (uint8_t)String[i]);
	for (; i < Count; i++)
		WriteUInt8(Out, 0);
}

/* ReadCompressedUInt16() -- Reads a "compressed" uint16_t */
// Numerical Range: 0 - 32767
uint16_t ReadCompressedUInt16(const void** const p)
{
	uint16_t ReadVal;
	
	/* Check */
	if (!p || !*p)
		return 0;
		
	/* Read in first value */
	ReadVal = ReadUInt8((const uint8_t** const)p);
	
	// Compressed?
	if (ReadVal & 0x80U)
	{
		ReadVal &= 0x7FU;		// Don't remember upper bit
		ReadVal <<= 8;
		ReadVal |= ReadUInt8((const uint8_t** const)p);
	}
	
	/* Return result */
	return ReadVal;
}

/* WriteCompressedUInt16() -- Writes a "compressed" uint16_t */
// Numerical Range: 0 - 32767
void WriteCompressedUInt16(void** const p, const uint16_t Value)
{
	/* Check */
	if (!p || !*p)
		return;
		
	/* Write in high value */
	if (Value > 0x7FU)
		WriteUInt8((uint8_t** const)p, ((Value & 0x7F00) >> 8) | 0x80U);
		
	/* Write in low value */
	WriteUInt8((uint8_t** const)p, Value & 0xFFU);
}

/********************
*** BYTE SWAPPING ***
********************/

/* SwapUInt16() -- Swap 16-bits */
uint16_t SwapUInt16(const uint16_t In)
{
	return ((In & 0xFFU) << 8) | ((In & 0xFF00U) >> 8);
}

/* SwapUInt32() -- Swap 32-bits */
uint32_t SwapUInt32(const uint32_t In)
{
	return ((In & 0xFFU) << 24) | ((In & 0xFF00U) << 8) | ((In & 0xFF0000U) >> 8) | ((In & 0xFF000000U) >> 24);
}

/* SwapUInt64() -- Swap 64-bits */
uint64_t SwapUInt64(const uint64_t In)
{
	return (((In >> 56)) |
	        ((In >> 40) & __REMOOD_ULL_SUFFIX(0x000000000000FF00)) |
	        ((In >> 24) & __REMOOD_ULL_SUFFIX(0x0000000000FF0000)) |
	        ((In >> 8) & __REMOOD_ULL_SUFFIX(0x00000000FF000000)) |
	        ((In << 8) & __REMOOD_ULL_SUFFIX(0x000000FF00000000)) |
	        ((In << 24) & __REMOOD_ULL_SUFFIX(0x0000FF0000000000)) |
			((In << 40) & __REMOOD_ULL_SUFFIX(0x00FF000000000000)) |
			((In << 56) & __REMOOD_ULL_SUFFIX(0xFF00000000000000)));
}

/* SwapInt16() -- Swap 16-bits */
int16_t SwapInt16(const int16_t In)
{
	return (int16_t)SwapUInt16((uint16_t)In);
}

/* SwapInt32() -- Swap 32-bits */
int32_t SwapInt32(const int32_t In)
{
	return (int32_t)SwapUInt32((uint32_t)In);
}

/* SwapInt64() -- Swap 64-bits */
int64_t SwapInt64(const int64_t In)
{
	return (int64_t)SwapUInt64((uint64_t)In);
}

/* Little swapping */
#if defined(__REMOOD_BIG_ENDIAN)
#define LS_x(w,x) x BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#else
#define LS_x(w,x) x BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return In;\
	}
#endif

LS_x(Int16, int16_t);
LS_x(UInt16, uint16_t);
LS_x(Int32, int32_t);
LS_x(UInt32, uint32_t);
LS_x(Int64, int64_t);
LS_x(UInt64, uint64_t);
#undef LS_x

/* Big swapping */
#if defined(__REMOOD_BIG_ENDIAN)
#define BS_x(w,x) x BP_MERGE(BigSwap,w)(const x In)\
	{\
		return In;\
	}
#else
#define BS_x(w,x) x BP_MERGE(BigSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#endif
BS_x(Int16, int16_t);
BS_x(UInt16, uint16_t);
BS_x(Int32, int32_t);
BS_x(UInt32, uint32_t);
BS_x(Int64, int64_t);
BS_x(UInt64, uint64_t);
#undef BS_x

/*** Reading/Writing Little Endian Data ***/
#if defined(__REMOOD_BIG_ENDIAN)
#define BPLREAD_x(w,x) x BP_MERGE(LittleRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Swap,w)(BP_MERGE(Read,w)(Ptr));\
	}
#else
#define BPLREAD_x(w,x) x BP_MERGE(LittleRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Read,w)(Ptr);\
	}
#endif
#if defined(__REMOOD_BIG_ENDIAN)
#define BPLWRITE_x(w,x) void BP_MERGE(LittleWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, BP_MERGE(Swap,w)(Val));\
	}
#else
#define BPLWRITE_x(w,x) void BP_MERGE(LittleWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, Val);\
	}
#endif
BPLREAD_x(Int16, int16_t)
BPLREAD_x(UInt16, uint16_t)
BPLREAD_x(Int32, int32_t)
BPLREAD_x(UInt32, uint32_t)
BPLREAD_x(Int64, int64_t)
BPLREAD_x(UInt64, uint64_t)
BPLWRITE_x(Int16, int16_t)
BPLWRITE_x(UInt16, uint16_t)
BPLWRITE_x(Int32, int32_t)
BPLWRITE_x(UInt32, uint32_t)
BPLWRITE_x(Int64, int64_t)
BPLWRITE_x(UInt64, uint64_t)
#undef BPLREAD_x
#undef BPLWRITE_x


/*** Reading/Writing Big Endian Data ***/
#if !defined(__REMOOD_BIG_ENDIAN)
#define BPBREAD_x(w,x) x BP_MERGE(BigRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Swap,w)(BP_MERGE(Read,w)(Ptr));\
	}
#else
#define BPBREAD_x(w,x) x BP_MERGE(BigRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Read,w)(Ptr);\
	}
#endif
#if !defined(__REMOOD_BIG_ENDIAN)
#define BPBWRITE_x(w,x) void BP_MERGE(BigWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, BP_MERGE(Swap,w)(Val));\
	}
#else
#define BPBWRITE_x(w,x) void BP_MERGE(BigWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, Val);\
	}
#endif
BPBREAD_x(Int16, int16_t)
BPBREAD_x(UInt16, uint16_t)
BPBREAD_x(Int32, int32_t)
BPBREAD_x(UInt32, uint32_t)
BPBREAD_x(Int64, int64_t)
BPBREAD_x(UInt64, uint64_t)
BPBWRITE_x(Int16, int16_t)
BPBWRITE_x(UInt16, uint16_t)
BPBWRITE_x(Int32, int32_t)
BPBWRITE_x(UInt32, uint32_t)
BPBWRITE_x(Int64, int64_t)
BPBWRITE_x(UInt64, uint64_t)
#undef BPBREAD_x
#undef BPBWRITE_x

/* End */
#undef BP_MERGE



