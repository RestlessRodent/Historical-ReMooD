// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Global XML Parsing

#ifndef __D_XML_H__
#define __D_XML_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "dstrings.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

/* D_XMLBuildAction_t -- Type of action being performed */
typedef enum D_XMLBuildAction_e
{
	DXMLBA_BUILD,									// Building for a WAD
	DXMLBA_CLEARBUILD,								// Cleaning out a WAD
	DXMLBA_COMPOSITE,								// Compositing virtual WADs
	DXMLBA_CLEARCOMPOSITE,							// Clear virtual composite
	
	MAXXMLBUILDACTIONS
} D_XMLBuildAction_t;

/* D_XMLKeyJunk_t -- Type of key junk */
typedef enum D_XMLKeyJunk_e
{
	DXMLKJ_IDENTITY,								// <Identity> -- XML Info
	DXMLKJ_MENU,									// <Menu> -- Menu definitions
	DXMLKJ_WADINDEX,								// <WadIndex> -- WAD Information (seeking)

	NUMXMLKEYJUNKS
} D_XMLKeyJunk_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_XMLPassedData_t -- Data to pass to an XML handler */
typedef struct D_XMLPassedData_s
{
	D_XMLBuildAction_t Action;						// What is happening?
	D_XMLKeyJunk_t KeyJunk;							// What is this?
	WX_WADFile_t* WAD;								// WAD File being checked
	void** PrivateJunk;								// Private Data
	size_t PrivateSize;								// Size of Private Data
	const char* Key;								// Passed Key
	const char* Value;								// Passed Value
	int CheckRetVal;								// Return value of checker
} D_XMLPassedData_t;

/*****************
*** PROTOTYPES ***
*****************/

int D_XMLCheckKey(const char* const a_Key, const char* const a_CheckValue, const char** const a_Next);

void D_WX_XMLBuild(WX_WADFile_t* const a_WAD);
void D_WX_XMLClearBuild(WX_WADFile_t* const a_WAD);
void D_WX_XMLComposite(WX_WADFile_t* const a_WAD);
void D_WX_XMLClearComposite(void);

#endif /* __D_XML_H__ */

