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
// DESCRIPTION: System specific network interface stuff.

#ifndef __I_NET__
#define __I_NET__

#ifdef __GNUG__
#pragma interface
#endif

#include "doomtype.h"

#define MAXPACKETLENGTH  1450	// For use in a LAN
#define INETPACKETLENGTH 512	// For use on the internet

extern short hardware_MAXPACKETLENGTH;
extern int net_bandwidth;		// in byte/s

// to be defined by the network driver
extern void (*I_NetGet) (void);	// return packet in doomcom struct
extern void (*I_NetSend) (void);	// send packet within doomcom struct
extern boolean(*I_NetCanSend) (void);	// ask to driver if all is ok to send data now
extern void (*I_NetFreeNodenum) (int nodenum);	// close a connection 
extern int (*I_NetMakeNode) (char *address);	// open a connection with specified address
extern boolean(*I_NetOpenSocket) (void);	// opend all connections
extern void (*I_NetCloseSocket) (void);	// close all connections no more allow geting any packet

boolean I_InitNetwork(void);

#endif

