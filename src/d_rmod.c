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
// DESCRIPTION: Global RMOD Parsing

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_rmod.h"
#include "m_menu.h"
#include "console.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/* D_WXRMODPrivate_t -- RMOD Private Data */ 
typedef struct D_WXRMODPrivate_s
{
} D_WXRMODPrivate_t;

/****************
*** FUNCTIONS ***
****************/

/* D_WX_RMODBuild() -- Build RMOD for WAD */
void D_WX_RMODBuild(WX_WADFile_t* const a_WAD)
{
	Z_Table_t* Table, *Sub;
	
	/* Check */
	if (!a_WAD)
		return;
	
	Table = Z_TableCreate("Root");
	
	if (!Z_TableSetValue(Table, "ReMooD", "Is Awesome"))
		CONS_Printf("D_WX_RMODBuild: Failed to add subvalue ReMooD\n");
	if (!Z_TableSetValue(Table, "Doom", "Is Awesome"))
		CONS_Printf("D_WX_RMODBuild: Failed to add subvalue ReMooD\n");
		
	Sub = Z_FindSubTable(Table, "ReMooD", false);
	if (Sub)
		CONS_Printf("D_WX_RMODBuild: found subtable ReMooD?\n");
	
	Sub = Z_FindSubTable(Table, "Woo", false);
	if (Sub)
		CONS_Printf("D_WX_RMODBuild: fount subtable Woo?\n");
		
	Sub = Z_FindSubTable(Table, "Woo", true);
	if (!Sub)
		CONS_Printf("D_WX_RMODBuild: Failed to add subtable Woo\n");
	else
	{
		if (!Z_TableSetValue(Sub, "Foo", "Bar"))
			CONS_Printf("D_WX_RMODBuild: Failed to add subvalue Foo\n");
		if (!Z_TableSetValue(Sub, "Bar", "Foo"))
			CONS_Printf("D_WX_RMODBuild: Failed to add subvalue Foo\n");
		
		
		if (!Z_TableSetValue(Sub, "Foo", "Not Foo"))
			CONS_Printf("D_WX_RMODBuild: Failed to set subvalue Foo\n");
		
		Sub = Z_FindSubTable(Sub, "Foo", true);
		
		if (Sub)
			CONS_Printf("D_WX_RMODBuild: found subtable which really should be entry.\n");
		
	}
	
	if (Z_TableSetValue(Table, "Woo", "Wee"))
		CONS_Printf("D_WX_RMODBuild: Set value of table Woo?\n");
	
	Z_TablePrint(Table, ">");
}

/* D_WX_RMODClearBuild() -- Clear RMOD from WAD */
void D_WX_RMODClearBuild(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* D_WX_RMODComposite() -- Build RMOD composite */
void D_WX_RMODComposite(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* D_WX_RMODClearComposite() -- Clear RMOD composite */
void D_WX_RMODClearComposite(void)
{
}

