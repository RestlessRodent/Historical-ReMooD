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
// -----------------------------------------------------------------------------
// Stubbed
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
// DESCRIPTION:

#include "i_net.h"
#include "doomstat.h"

void LOCAL_NetGet(void)
{
}

void LOCAL_NetSend(void)
{
}

boolean LOCAL_NetCanSend(void)
{
	return true;
}

void LOCAL_NetFreeNodenum(int nodenum)
{
}

int LOCAL_NetMakeNode(char *address)
{
}

boolean LOCAL_NetOpenSocket(void)
{
	return true;
}

void LOCAL_NetCloseSocket(void)
{
}


int I_InitTcpNetwork(void)
{
	I_NetGet = &LOCAL_NetGet;
	I_NetSend = &LOCAL_NetSend;
	I_NetCanSend = &LOCAL_NetCanSend;
	I_NetFreeNodenum = &LOCAL_NetFreeNodenum;
	I_NetMakeNode = &LOCAL_NetMakeNode;
	I_NetOpenSocket = &LOCAL_NetOpenSocket;
	I_NetCloseSocket = &LOCAL_NetCloseSocket;
}

