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
// DESCRIPTION:

#include "i_net.h"
#include "doomstat.h"

void LOCAL_NetGet(void)
{
}

void LOCAL_NetSend(void)
{
}

bool_t LOCAL_NetCanSend(void)
{
	return true;
}

void LOCAL_NetFreeNodenum(int nodenum)
{
}

int LOCAL_NetMakeNode(char *address)
{
	return 0;
}

bool_t LOCAL_NetOpenSocket(void)
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

	return 0;
}

