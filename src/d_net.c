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
// DESCRIPTION: 
//      DOOM Network game communication and protocol,
//      all OS independend parts.
//
//      Implemente a Sliding window protocol without receiver window
//      (out of order reception)
//      This protocol use mix of "goback n" and "selective repeat" implementation
//      The NOTHING packet is send when connection is idle for acknowledge packets

#ifdef _WIN32									/* Win32 WinSock/BSD */
#include <winsock2.h>
#elif !defined(_WIN32) && !defined(__palmos__)	/* BSD */
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#define closesocket close
#elif defined(__palmos__)						/* Palm OS */
#include <INetMgr.h>
#endif

#include "doomdef.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_clisrv.h"
#include "z_zone.h"
#include "i_tcp.h"

//
// NETWORKING
//

FILE *debugfile = NULL;			// put some net info in a file
							  // during the game
							  
void (*I_NetGet) (void);
void (*I_NetSend) (void);
boolean(*I_NetCanSend) (void);
void (*I_NetCloseSocket) (void);
void (*I_NetFreeNodenum) (int nodenum);
int (*I_NetMakeNode) (char *address);
boolean(*I_NetOpenSocket) (void);

// network stats
tic_t statstarttic;
int getbytes = 0;
INT64 sendbytes = 0;
int retransmit = 0, duppacket = 0;
int sendackpacket = 0, getackpacket = 0;
int ticruned = 0, ticmiss = 0;

// globals
int getbps, sendbps;
float lostpercent, duppercent, gamelostpercent;
int packetheaderlength;

// -----------------------------------------------------------------
//  Some stuct and function for acknowledgment of packets
// -----------------------------------------------------------------
#define MAXACKPACKETS    64		// minimum number of nodes
#define MAXACKTOSEND     64
#define URGENTFREESLOTENUM   6
#define ACKTOSENDTIMEOUT  (TICRATE/17)

typedef struct
{
	byte acknum;
	byte nextacknum;
	byte destinationnode;
	tic_t senttime;
	USHORT length;
	USHORT resentnum;
	char pak[MAXPACKETLENGTH];
} ackpak_t;

typedef enum
{
	CLOSE = 1,					// flag is set when connection is closing
} node_flags_t;

// table of packet that was not acknowleged can be resend (the sender window)
static ackpak_t ackpak[MAXACKPACKETS];

typedef struct
{
	// ack return to send (like slinding window protocol)
	byte firstacktosend;

	// when no consecutive packet are received we keep in mind what packet 
	// we already received in a queu 
	byte acktosend_head;
	byte acktosend_tail;
	byte acktosend[MAXACKTOSEND];

	// automaticaly send keep alive packet when not enought trafic
	tic_t lasttimeacktosend_sent;
	// detect connection lost
	tic_t lasttimepacketreceived;

	// flow control : do not sent to mush packet with ack 
	byte remotefirstack;
	byte nextacknum;

	byte flags;
// jacobson tcp timeout evaluation algorithm (Karn variation)
	fixed_t ping;
	fixed_t varping;
	int timeout;				// computed with ping and varping
} node_t;

static node_t nodes[MAXNETNODES];

#define  PINGDEFAULT     ((200*TICRATE*FRACUNIT)/1000)
#define  VARPINGDEFAULT  ( (50*TICRATE*FRACUNIT)/1000)
#define  TIMEOUT(p,v)    (p+4*v+FRACUNIT/2)>>FRACBITS;

#ifdef DEBUGFILE

static void fprintfstring(char *s, byte len)
{
	int i;
	int mode = 0;

	for (i = 0; i < len; i++)
		if (s[i] < 32)
			if (mode == 0)
			{
				fprintf(debugfile, "[%d", (byte) s[i]);
				mode = 1;
			}
			else
				fprintf(debugfile, ",%d", (byte) s[i]);
		else
		{
			if (mode == 1)
			{
				fprintf(debugfile, "]");
				mode = 0;
			}
			fprintf(debugfile, "%c", s[i]);
		}
	if (mode == 1)
		fprintf(debugfile, "]");
	fprintf(debugfile, "\n");
}
#endif

void Internal_Get(void)
{
}

void Internal_Send(void)
{
}

void Internal_FreeNodenum(int nodenum)
{
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
extern boolean D_CheckNetGame(void)
{
	boolean ret = false;

	statstarttic = I_GetTime();

	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	multiplayer = false;

	// only dos version with external driver will return true
	netgame = false;
	if (netgame)
		netgame = false;
	return ret;
}

extern void D_CloseConnection(void)
{
	int i;

	if (netgame)
		netgame = false;
}

/*******************************************************************************
**************************** ReMooD Network Protocol ***************************
*******************************************************************************/

#include "cmp_rle.h"

HandlerHolder_t* HandlerHash[26];
NetworkPlayer_t NetPlayers[MAXPLAYERS];

//////////////////////////////// PROTOCOL FUNCS ////////////////////////////////

//////////////////////////////// NET CONTROL ////////////////////////////////

/* RNET_CreateServer() -- Creates a network game */
int RNET_CreateServer(NetworkHost_t Host)
{
	return 0;
}

/* RNET_Connect() -- Connects to a server */
int RNET_Connect(NetworkHost_t Host)
{
	return 0;
}

/* RNET_Disconnect() -- Destroys server or client connection */
int RNET_Disconnect(int ConnectionID)
{
	return 0;
}

