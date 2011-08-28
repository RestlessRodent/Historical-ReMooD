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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@gmail.com>
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
// DESCRIPTION: Common Network Stuff

/***************
*** INCLUDES ***
***************/

#include "i_util.h"
#include "i_net.h"

/*************
*** LOCALS ***
*************/

static I_NetDriver_t** l_NetDrivers;			// Music drivers
static size_t l_NumNetDrivers;					// Number of music drivers

/****************
*** FUNCTIONS ***
****************/

/* I_AddNetDriver() -- Adds a new network driver */
bool_t I_AddNetDriver(I_NetDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Attempt driver initialization */
	if (a_Driver->Init && !a_Driver->Init(a_Driver))
		return false;
	
	/* Find a blank spot */
	for (i = 0; i < l_NumNetDrivers; i++)
		if (!l_NetDrivers[i])
		{
			l_NetDrivers[i] = a_Driver;
			break;
		}
	
	// did not find one
	if (i == l_NumNetDrivers)
	{
		// Resize the list
		Z_ResizeArray(&l_NetDrivers, sizeof(*l_NetDrivers), l_NumNetDrivers, l_NumNetDrivers + 1);
		l_NetDrivers[l_NumNetDrivers++] = a_Driver;
	}
	
	/* Call the success routine, if it exists */
	if (a_Driver->Success)
		a_Driver->Success(a_Driver);
	
	/* Success */	
	return true;
}

/* I_RemoveNetDriver() -- Removes a network driver */
bool_t I_RemoveNetDriver(I_NetDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Find driver */
	for (i = 0; i < l_NumNetDrivers; i++)
		if (l_NetDrivers[i] == a_Driver)
		{
			// Call destroy function
			if (l_NetDrivers[i]->Destroy)
				l_NetDrivers[i]->Destroy(a_Driver);
			l_NetDrivers[i] = NULL;
			
			return true;
		}
	
	/* Driver not loaded */
	return false;
}

/* I_FindNetDriver() -- Find a network driver that can play this format */
I_NetDriver_t* I_FindNetDriver(const char* const a_Protocol)
{
	I_NetDriver_t* Best = NULL;
	size_t i;
	const char* p;
	
	/* Go through every driver */
	for (i = 0; i < l_NumNetDrivers; i++)
		if (l_NetDrivers[i])
		{
			// Get protocol
			p = l_NetDrivers[i]->Protocols;
			
			// Check for match
			while (p[0])
			{
				// See if it is our protocol
				if (strcasecmp(p, a_Protocol) == 0)
				{
					// Check for best
					if (!Best || (Best && l_NetDrivers[i]->Priority > Best->Priority))
						Best = l_NetDrivers[i];
					
					// Always break since it was found anyway
					break;
				}
			}
		}
	
	/* Return the best driver, if any */
	return Best;
}

/* I_InitNetwork() -- Initializes the network */
bool_t I_InitNetwork(void)
{
	/* Add interface specific network drivers */
	if (!I_NetDriverInit())
		CONS_Printf("I_InitNetwork: Failed to initialize interface network drivers.\n");
	
	/* Success! */
	return true;
}

