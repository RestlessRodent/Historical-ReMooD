// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
// Portions Copyright (C) Odamex <http://odamex.net/>
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
// DESCRIPTION: Odamex Protocol Implementation

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "i_util.h"
#include "ip.h"
#include "ip_prv.h"
#include "console.h"
#include "dstrings.h"

/****************
*** CONSTANTS ***
****************/

#define MAXPACKET		1768					// Max packet size

/* IP_OdamexConnState_t -- Server connection state */
typedef enum IP_OdamexConnState_e
{
	IPOCS_REQUESTINFO,							// Request server info
	IPOCS_WAITINFO,								// Wait for info
	IPOCS_CONNECTED,							// Connected
} IP_OdamexConnState_t;

#define ODA_CHAL						5560020	// Challenge from join
#define ODA_LAUNCHCHAL					777123	// Launcher Challenge ID

/* svc_t -- Server originated message (Odamex) */
typedef enum svc_e
{
	svc_abort,
	svc_full,
	svc_disconnect,
	svc_reserved3,
	svc_playerinfo,			// weapons, ammo, maxammo, raisedweapon for local player
	svc_moveplayer,			// [byte] [int] [int] [int] [int] [byte]
	svc_updatelocalplayer,	// [int] [int] [int] [int] [int]
	svc_pingrequest,		// [SL] 2011-05-11 [long:timestamp]
	svc_updateping,			// [byte] [byte]
	svc_spawnmobj,			//
	svc_disconnectclient,
	svc_loadmap,
	svc_consoleplayer,
	svc_mobjspeedangle,
	svc_explodemissile,		// [short] - netid
	svc_removemobj,
	svc_userinfo,
	svc_movemobj,			// [short] [byte] [int] [int] [int]
	svc_spawnplayer,
	svc_damageplayer,
	svc_killmobj,
	svc_firepistol,			// [byte] - playernum
	svc_fireshotgun,		// [byte] - playernum
	svc_firessg,			// [byte] - playernum
	svc_firechaingun,		// [byte] - playernum
	svc_fireweapon,			// [byte]
	svc_sector,
	svc_print,
	svc_mobjinfo,
	svc_updatefrags,		// [byte] [short]
	svc_teampoints,
	svc_activateline,
	svc_movingsector,
	svc_startsound,
	svc_reconnect,
	svc_exitlevel,
	svc_touchspecial,
	svc_changeweapon,
	svc_reserved42,
	svc_corpse,
	svc_missedpacket,
	svc_soundorigin,
	svc_reserved46,
	svc_reserved47,
	svc_forceteam,			// [Toke] Allows server to change a clients team setting.
	svc_switch,
	svc_reserved50,
	svc_reserved51,
	svc_spawnhiddenplayer,	// [denis] when client can't see player
	svc_updatedeaths,		// [byte] [short]
	svc_ctfevent,			// [Toke - CTF] - [int]
	svc_serversettings,		// 55 [Toke] - informs clients of server settings
	svc_spectate,			// [Nes] - [byte:state], [short:playernum]
	svc_connectclient,
    svc_midprint,
	svc_svgametic,			// [SL] 2011-05-11 - [byte]
	svc_timeleft,
	svc_inttimeleft,		// [ML] For intermission timer
	svc_mobjtranslation,	// [SL] 2011-09-11 - [byte]
	svc_fullupdatedone,		// [SL] Inform client the full update is over
	svc_railtrail,			// [SL] Draw railgun trail and play sound
	svc_readystate,			// [AM] Broadcast ready state to client
	svc_playerstate,		// [SL] Health, armor, and weapon of a player
	svc_warmupstate,		// [AM] Broadcast warmup state to client
	svc_loadwad,			// [SL] Server is changing to a new WAD file

	// for co-op
	svc_mobjstate = 70,
	svc_actor_movedir,
	svc_actor_target,
	svc_actor_tracer,
	svc_damagemobj,

	// for downloading
	svc_wadinfo,			// denis - [ulong:filesize]
	svc_wadchunk,			// denis - [ulong:offset], [ushort:len], [byte[]:data]
		
	// netdemos - NullPoint
	svc_netdemocap = 100,
	svc_netdemostop = 101,
	svc_netdemoloadsnap = 102,

	svc_vote_update = 150, // [AM] - Send the latest voting state to the client.
	svc_maplist = 155, // [AM] - Return a maplist status.
	svc_maplist_update = 156, // [AM] - Send the entire maplist to the client in chunks.
	svc_maplist_index = 157, // [AM] - Send the current and next map index to the client.

	// for compressed packets
	svc_compressed = 200,

	// for when launcher packets go astray
	svc_launcher_challenge = 212,
	svc_challenge = 163,
	svc_max = 255
} svc_t;

/* clc_t -- Client originated message (Odamex) */
typedef enum clc_e
{
	clc_abort,
	clc_reserved1,
	clc_disconnect,
	clc_say,
	clc_move,			// send cmds
	clc_userinfo,		// send userinfo
	clc_pingreply,		// [SL] 2011-05-11 - [long: timestamp]
	clc_rate,
	clc_ack,
	clc_rcon,
	clc_rcon_password,
	clc_changeteam,		// [NightFang] - Change your team [Toke - Teams] Made this actualy work
	clc_ctfcommand,
	clc_spectate,			// denis - [byte:state]
	clc_wantwad,			// denis - string:name, string:hash
	clc_kill,				// denis - suicide
	clc_cheat,				// denis - god, pumpkins, etc
    clc_cheatpulse,         // Russell - one off cheats (idkfa, idfa etc)
	clc_callvote,			// [AM] - Calling a vote
	clc_vote,				// [AM] - Casting a vote
	clc_maplist,			// [AM] - Maplist status request.
	clc_maplist_update,     // [AM] - Request the entire maplist from the server.
	clc_getplayerinfo,
	clc_ready,				// [AM] Toggle ready state.
	clc_spy,				// [SL] Tell server to send info about this player

	// for when launcher packets go astray
	clc_launcher_challenge = 212,
	clc_challenge = 163,
	clc_max = 255
} clc_t;

/*****************
*** STRUCTURES ***
*****************/

#define MAXODASTRING					128		// Maximum length in strings

/* IP_OdaData_t -- Connection Data */
typedef struct IP_OdaData_s
{
	struct IP_Conn_s* Conn;						// Connection
	I_NetSocket_t* Socket;						// Socket to server
	IP_OdamexConnState_t NetMode;				// Network Mode
	tic_t TimeOut;								// Time out time
	
	uint8_t Pack[MAXPACKET];					// Packet Data
	uint8_t* PackAt;							// Current read position
	uint8_t* PackEnd;							// End of packet
	
	uint8_t Out[MAXPACKET];						// Packet Data
	uint8_t* OutAt;								// Current read position
	uint8_t* OutEnd;							// End of packet
	
	int32_t SVToken;							// Server Token
	char SVHost[MAXODASTRING];					// Server Host
} IP_OdaData_t;

/****************
*** FUNCTIONS ***
****************/

#define __REMOOD_MACROMERGE(a,b) a##b

#define __REMOOD_ODAWRITE(n,t,f)\
static void __REMOOD_MACROMERGE(IPS_W,n)(IP_OdaData_t* const a_Data, const t a_Num)\
{\
	if (a_Data->OutAt + sizeof(a_Num) < a_Data->OutEnd)\
		f((t**)&a_Data->OutAt, a_Num);\
}

#define __REMOOD_ODAREAD(n,t,f)\
static t __REMOOD_MACROMERGE(IPS_R,n)(IP_OdaData_t* const a_Data)\
{\
	if (a_Data->PackAt + sizeof(t) < a_Data->PackEnd)\
		return f((t**)&a_Data->PackAt);\
}

__REMOOD_ODAREAD(u8,uint8_t,ReadUInt8);
__REMOOD_ODAREAD(i16,int16_t,LittleReadInt16);
__REMOOD_ODAREAD(i32,int32_t,LittleReadInt32);

__REMOOD_ODAWRITE(u8,uint8_t,WriteUInt8);
__REMOOD_ODAWRITE(i16,int16_t,LittleWriteInt16);
__REMOOD_ODAWRITE(i32,int32_t,LittleWriteInt32);

/* IPS_Rs() -- Reads string */
static size_t IPS_Rs(IP_OdaData_t* const a_Data, char* const a_Dest, const size_t a_Len)
{
	size_t i;
	char Char;
	
	/* Read String */
	i = 0;
	for (;;)
	{
		// Read Character
		Char = IPS_Ru8(a_Data);
		
		// Place in buffer?
		if (i < a_Len - 1)
			a_Dest[i++] = Char;
		
		// Done?
		if (!Char)
			break;
	}
	
	/* Always place NUL at end */
	a_Dest[a_Len - 1] = 0;
	
	/* Return read length */
	return i;
}

#undef __REMOOD_ODAWRITE
#undef __REMOOD_MACROMERGE

/* IPS_OdaWritePacket() -- Writes a single packet */
static bool_t IPS_OdaWritePacket(IP_OdaData_t* const a_Data)
{
	/* Check */
	if (!a_Data)
		return false;
	
	/* Send */
	I_NetSend(a_Data->Socket, &a_Data->Conn->RemAddr.Private, a_Data->Out, a_Data->OutAt - a_Data->Out);
	
	/* Clear */
	memset(a_Data->Out, 0, sizeof(a_Data->Out));
	a_Data->OutAt = a_Data->Out;
	a_Data->OutEnd = &a_Data->Out[MAXPACKET - 1];
	
	/* Always works? */
	return true;
}

/* IPS_OdaReadPacket() -- Reads a single packet */
static bool_t IPS_OdaReadPacket(IP_OdaData_t* const a_Data)
{
	size_t ReadSize;
	I_HostAddress_t FromAddr;
	
	/* Check */
	if (!a_Data)
		return false;
	
	/* Clear */
	memset(a_Data->Pack, 0, sizeof(a_Data->Pack));
	a_Data->PackAt = a_Data->PackEnd = a_Data->Pack;
	
	/* Read */
	ReadSize = I_NetRecv(a_Data->Socket, &FromAddr, a_Data->Pack, MAXPACKET);
	
	// Nothing read?
	if (!ReadSize)
		return false;
	
	// Host does not match server
	if (!I_NetCompareHost(&FromAddr, &a_Data->Conn->RemAddr.Private))
		return false;
	
	/* Was read */
	a_Data->PackEnd += ReadSize;
	return true;
}

/* IP_ODA_VerifyF() -- Verify Protocol */
bool_t IP_ODA_VerifyF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	uint32_t RealPort;
	
	/* Cannot host servers */
	if (a_Flags & IPF_INPUT)
		return false;
	
	/* Check Port */
	// Get the real port
	if (!a_Port)
		RealPort = 10666;
	else
		RealPort = a_Port;
	
	// Range
	if (RealPort <= 0 || RealPort >= 65536)
		return false;
	
	/* Success */
	return true;
}

/* IP_ODA_CreateF() -- Create connection */
struct IP_Conn_s* IP_ODA_CreateF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	struct IP_Addr_s Addr;
	struct IP_Conn_s* New;
	uint16_t Port;
	I_NetSocket_t* Socket;
	IP_OdaData_t* Data;
	int32_t i;
	
	/* Valid Port */
	Port = a_Port;
	if (!Port)
		Port = 10666;
	
	/* Attempt Host Resolution */
	if (!IP_UDPResolveHost(a_Proto, &Addr, a_Host, Port))
		return NULL;
	
	/* Setup Socket */
	// Try making sockets
	for (i = 0; i < IPMAXSOCKTRIES; i++)
	{
		// Create socket to server
		Socket = I_NetOpenSocket(0, NULL, Port + i);
	
		// Worked?
		if (Socket)
			break;
	}
	
	// No socket created
	if (!Socket)
		return NULL;
	
	/* Return new connection allocation */
	New = IP_AllocConn(a_Proto, a_Flags, &Addr);
	
	/* Initialize Mode */
	// Create Data
	New->Size = sizeof(*Data);
	Data = New->Data = Z_Malloc(New->Size, PU_STATIC, NULL);
	
	// Set Data
	Data->Socket = Socket;
	Data->Conn = New;
	Data->OutEnd = &Data->Out[MAXPACKET - 1];
	
	/* Return it */
	return New;
}

/* IP_ODA_RunConnF() -- Runs connection */
void IP_ODA_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
{
	IP_OdaData_t* Data;
	int32_t MsgID;
	uint8_t NumPlayers;
	
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
	
	/* Get Data */
	Data = a_Conn->Data;
	
	/* Handle network packets */
	while (IPS_OdaReadPacket(Data))
		// Not Connected
		if (Data->NetMode == IPOCS_REQUESTINFO ||
			Data->NetMode == IPOCS_WAITINFO)
		{
			// Read Message Type
			MsgID = IPS_Ri32(Data);
			
			// Got launcher challenge
			if (MsgID == 5560020)
			{
				// Information
				CONL_OutputUT(CT_NETWORK, DSTR_IPC_SVGOTINFO, "\n");
				
				// Parse Packet
				Data->SVToken = IPS_Ri32(Data);
				IPS_Rs(Data, Data->SVHost, MAXODASTRING);
				
				D_XNetSetServerName(Data->SVHost);
				
				NumPlayers = IPS_Ru8(Data);
			}
		}
		
		// Connected
		else
			while (Data->PackAt < Data->PackEnd)
			{
				// Read Message Type
				MsgID = IPS_Ru8(Data);
			
				// Which message?
				switch (MsgID)
				{
						// Unknown
					default:
						// Warning
						CONL_OutputUT(CT_NETWORK, DSTR_IPC_UNKODA, "%02x\n", MsgID);
					
						// Disconnect from server
						D_XNetDisconnect(false);
						return;
				}
			}
		
	/* Which Mode? */
	switch (Data->NetMode)
	{
			// Request info from server
		case IPOCS_REQUESTINFO:
			// Information
			CONL_OutputUT(CT_NETWORK, DSTR_IPC_SVREQUESTINFO, "\n");
			
			// Send Message
			IPS_Wi32(Data, ODA_LAUNCHCHAL);
			IPS_OdaWritePacket(Data);
			
			// Change to wait on info
			Data->NetMode = IPOCS_WAITINFO;
			Data->TimeOut = g_ProgramTic + TICRATE;
			break;
		
			// Waiting for server info
		case IPOCS_WAITINFO:
			// Timed out?
			if (g_ProgramTic > Data->TimeOut)
			{
				Data->NetMode = IPOCS_REQUESTINFO;
				Data->TimeOut = 0;
			}
			break;
			
			// Unknown
		default:
			break;
	}
}

/* IP_ODA_DeleteConnF() -- Deletes connection */
void IP_ODA_DeleteConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
{
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
}


