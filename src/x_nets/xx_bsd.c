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
// DESCRIPTION: BSD Sockets TCPv4, UDPv4, TCPv6, and UDPv6 Code

/***************
*** INCLUDES ***
***************/

#include "i_util.h"

/*************************
*** BSD SOCKET LIBRARY ***
*************************/

/* XX_BSDNet -- BSD Network driver */
static I_NetDriver_t XX_BSDNet =
{
	"BSD Sockets",								// Name
	"bsd",										// Short
	"udp\0udp6\0udp4\0\0",						// Use both UDPv4 and UDPv6
			// Specifying "udp" uses both UDPv4 and UDPv6 (if possible)
	100,
	
#if 0
	/* Info */
	char Name[MAXDRIVERNAME];					// Name of driver
	char ShortName[MAXDRIVERNAME];				// Short driver name
	char Protocols[MAXPROTOCOLNAMES];			// Protocols
	uint8_t Priority;							// Priority of the driver
	
	/* Functions */
		// Initializes a driver
	bool_t (*Init)(struct I_NetDriver_s* const a_Driver);
		// Destroys a driver
	bool_t (*Destroy)(struct I_NetDriver_s* const a_Driver);
		// Success
	void (*Success)(struct I_NetDriver_s* const a_Driver);
	
	/* Dynamic */
	void* Data;									// Private data
	size_t Size;								// Private size
#endif
};


