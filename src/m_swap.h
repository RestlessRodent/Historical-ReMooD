// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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
// DESCRIPTION:  Endianess handling, swapping 16bit and 32bit.

#ifndef __M_SWAP__
#define __M_SWAP__

#ifdef __GNUG__
#pragma interface
#endif

#include "doomtype.h"

// Endianess handling.
// WAD files are stored little endian.
#ifdef __BIG_ENDIAN__

#define SHORT(x) ((Int16)( \
(((UInt16)(x) & (UInt16)0x00ffU) << 8) \
| \
(((UInt16)(x) & (UInt16)0xff00U) >> 8) )) \

#define LONG(x) ((Int32)( \
(((UInt32)(x) & (UInt32)0x000000ffUL) << 24) \
| \
(((UInt32)(x) & (UInt32)0x0000ff00UL) <<  8) \
| \
(((UInt32)(x) & (UInt32)0x00ff0000UL) >>  8) \
| \
(((UInt32)(x) & (UInt32)0xff000000UL) >> 24) ))

#else
#define SHORT(x)  ((Int16)(x))	//((Int16) x)
#define LONG(x)	  ((Int32)(x))
#define SHORTU(x) ((UInt16)(x))
#define LONGU(x)  ((UInt32)(x))
#define SIZET(x)  ((size_t)(x))
#endif

#define SWAP16(x) ((Int16)( \
(((UInt16)(x) & (UInt16)0x00ffU) << 8) \
| \
(((UInt16)(x) & (UInt16)0xff00U) >> 8) )) \

#define SWAP32(x) ((Int32)( \
(((UInt32)(x) & (UInt32)0x000000ffUL) << 24) \
| \
(((UInt32)(x) & (UInt32)0x0000ff00UL) <<  8) \
| \
(((UInt32)(x) & (UInt32)0x00ff0000UL) >>  8) \
| \
(((UInt32)(x) & (UInt32)0xff000000UL) >> 24) ))

#endif

