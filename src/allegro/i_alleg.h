// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Allegro Trickery! Fun!

#ifndef __I_ALLEG_H__
#define __I_ALLEG_H__

// DJGPP's Allegro explodes if this isn't included first
#if defined(__DJGPP__)
	#include <stdint.h>
#endif

#include <allegro.h>

// Include winalleg on Windows since it conflicts!
#if defined(_WIN32)
	#define ALLEGRO_NO_MAGIC_MAIN	// Breaks with mingw-w64

	#include <winalleg.h>
#endif

// If we are on MSVC, we MUST undefine the following...
// Because a typedef in doomtype.h gives:
//     ?I_GetTimeMS@@YA_IXZ
// While allegro's #defines make it:
//     ?I_GetTimeMS@@YAIXZ
// Which causes linker problems. So for the sake of allegro and
// compat on MSVC (which lacks stdint) clear out these headers.
#if defined(_MSC_VER)
	#ifdef int8_t
		#undef int8_t
	#endif
	#ifdef int16_t
		#undef int16_t
	#endif
	#ifdef int32_t
		#undef int32_t
	#endif
	#ifdef int64_t
		#undef int64_t
	#endif
	#ifdef uint8_t
		#undef uint8_t
	#endif
	#ifdef uint16_t
		#undef uint16_t
	#endif
	#ifdef uint32_t
		#undef uint32_t
	#endif
	#ifdef uint64_t
		#undef uint64_t
	#endif
	#ifdef intptr_t
		#undef intptr_t
	#endif
	#ifdef uintptr_t
		#undef uintptr_t
	#endif

	#include "doomtype.h"
#else
	// Allegro should have included stdint, so no worry here
	#define __REMOOD_IGNORE_FIXEDTYPES
#endif

#endif /* __I_ALLEG_H__ */