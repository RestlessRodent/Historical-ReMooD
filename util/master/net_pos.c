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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: PalmOS Network

#include "network.h"

UInt32 PilotMain(UInt16 cmd, void* cmdPBP, UInt16 launchFlags)
{
	return 0;
}

/* StartServer() -- Starts UDP Server */
int StartServer(UInt16 Port)
{
	return 0;
}

/* StopServer() -- Stops UDP Server */
int StopServer(void)
{
	return 0;
}

/* Send() -- Sends data to someone */
int Send(NetworkHost_t Host, UInt8* OutData, size_t OutLength, size_t OutLengthLimit)
{
	return 0;
}

/* Receive() -- Gets data from someone */
int Receive(NetworkHost_t* Host, UInt8** InData, size_t* InLength, size_t InLengthLimit)
{
	return 0;
}

