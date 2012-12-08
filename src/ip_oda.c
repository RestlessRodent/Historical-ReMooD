// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// Portions Copyright (C) Odamex <http://odamex.net/>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
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
#include "z_mlzo.h"
#include "p_demcmp.h"

/****************
*** CONSTANTS ***
****************/

#define MAXPACKET		1768					// Max packet size
#define MAXLZOLEN		2048					// Max LZO Length

/* IP_OdamexConnState_t -- Server connection state */
typedef enum IP_OdamexConnState_e
{
	IPOCS_REQUESTINFO,							// Request server info
	IPOCS_WAITINFO,								// Wait for info
	IPOCS_STARTSYNC,							// Synchronize
	IPOCS_WAITSYNC,								// Wait for sync
	IPOCS_CONNECTED,							// Connected
} IP_OdamexConnState_t;

#define ODA_CHAL						5560020	// Challenge from join
#define ODA_LAUNCHCHAL					777123	// Launcher Challenge ID

#define ODA_VERSION							65	// 0.4+ Version
#define ODA_GAMEVER					(0*256+61)	// Game Version
#define ODA_NUMTEAMS						2	// Supported Teams
#define ODA_NUMWEAPONS					9		// Standard Doom Guns

#define ODA_RATE						1000	// Game Rate

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
	uint8_t SyncTries;							// Synchronization Retries
	
	uint8_t Pack[MAXPACKET];					// Packet Data
	uint8_t* PackAt;							// Current read position
	uint8_t* PackEnd;							// End of packet
	
	uint8_t Out[MAXPACKET];						// Packet Data
	uint8_t* OutAt;								// Current read position
	uint8_t* OutEnd;							// End of packet
	
	uint8_t LZOBuf[MAXLZOLEN];					// LZO Buffer
	
	int32_t SVToken;							// Server Token
	char SVHost[MAXODASTRING];					// Server Host
	uint8_t SVVersion;							// Server Version
	uint32_t SVPacketNum;						// Packet Number from server
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
	return 0;\
}

__REMOOD_ODAREAD(u8,uint8_t,ReadUInt8);
__REMOOD_ODAREAD(i16,int16_t,LittleReadInt16);
__REMOOD_ODAREAD(i32,int32_t,LittleReadInt32);

__REMOOD_ODAWRITE(u8,uint8_t,WriteUInt8);
__REMOOD_ODAWRITE(i16,int16_t,LittleWriteInt16);
__REMOOD_ODAWRITE(i32,int32_t,LittleWriteInt32);

// Markers
#define IPS_Rm IPS_Ru8
#define IPS_Wm IPS_Wu8

// Bool
#define IPS_Rb IPS_Ru8
#define IPS_Wb IPS_Wu8

#define IPS_Pu8(d) ((uint8_t)(*d->PackAt))

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
		if (a_Dest && a_Len > 0)
			if (i < a_Len - 1)
				a_Dest[i++] = Char;
		
		// Done?
		if (!Char)
			break;
	}
	
	/* Always place NUL at end */
	if (a_Dest && a_Len > 0)
		a_Dest[a_Len - 1] = 0;
	
	/* Return read length */
	return i;
}

/* IPS_Ws() -- Writes string */
static void IPS_Ws(IP_OdaData_t* const a_Data, char* const a_Src)
{
	char* p;
	
	/* Send String */
	for (p = a_Src; *p; p++)
		IPS_Wu8(a_Data, *p);	
	IPS_Wu8(a_Data, 0);
}

#undef __REMOOD_ODAWRITE
#undef __REMOOD_MACROMERGE

/* IPS_ODA_WritePacket() -- Writes a single packet */
static bool_t IPS_ODA_WritePacket(IP_OdaData_t* const a_Data)
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

/* IPS_ODA_ReadPacket() -- Reads a single packet */
static bool_t IPS_ODA_ReadPacket(IP_OdaData_t* const a_Data)
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

/* IPS_ODA_ColorRtoO() -- Converts ReMooD color to Odamex Color */
static int32_t IPS_ODA_ColorRtoO(const uint8_t a_Color)
{
	// TODO FIXME: RGBize
	return 0;
}

/* IPS_ODA_SendProfile() -- Send player profile */
static void IPS_ODA_SendProfile(IP_OdaData_t* const a_Data)
{
	size_t i;
	D_ProfileEx_t* Prof;
	
	/* Locate Profile */
	// Odamex only supports 1 client per connection, so use said client
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				if (g_XPlays[i]->Profile)
				{
					Prof = g_XPlays[i]->Profile;
					break;
				}
	
	// Try first screen then?
	if (D_ScrSplitHasPlayer(0))
		if (g_Splits[0].Profile)
			Prof = g_Splits[0].Profile;
	
	// If no profile, then use guest or whatever default there is
	Prof = g_KeyDefaultProfile;
	
	/* Write packet */
	IPS_Wm(a_Data, clc_userinfo);
	
	IPS_Ws(a_Data, Prof->DisplayName);
	IPS_Wu8(a_Data, Prof->Color);	// TODO FIXME: CTF Team
	IPS_Wi32(a_Data, 0);	// TODO FIXME: Gender
	IPS_Wi32(a_Data, IPS_ODA_ColorRtoO(Prof->Color));
	IPS_Ws(a_Data, "");	// TODO FIXME: Skin
	IPS_Wi32(a_Data, 16384);	// TODO FIXME: Autoaim Distance
	IPS_Wb(a_Data, false);	// Do not unlag
	IPS_Wb(a_Data, false);	// Do not predict weapons
	IPS_Wu8(a_Data, 3);	// TODO FIXME: Update Rate (Default 3)
	IPS_Wu8(a_Data, 2);	// TODO FIXME: Switch Weapon? 2 == Always
	
	// TODO FIXME: Weapon Order
	for (i = 0; i < ODA_NUMWEAPONS; i++)
		IPS_Wu8(a_Data, i);
}

/* IPS_ODA_ReadChal() -- Read challenge */
static bool_t IPS_ODA_ReadChal(IP_OdaData_t* const a_Data)
{
#define BUFSIZE 128
#define MAXWADS 32
	char Buf[BUFSIZE];
	int32_t WADp, i;
	uint8_t NumPlayers, NumWADs;
	const WL_WADFile_t* WADs[MAXWADS];
	const WL_WADFile_t* FoundWAD;
	bool_t TeamPlayOn;
	
	/* Inform client */
	CONL_OutputUT(CT_NETWORK, DSTR_IPC_SVGOTINFO, "\n");
	
	/* Init */
	TeamPlayOn = false;
	
	/* Parse Base Header */
	a_Data->SVToken = IPS_Ri32(a_Data);
	IPS_Rs(a_Data, a_Data->SVHost, MAXODASTRING);
	
	D_XNetSetServerName(a_Data->SVHost);
	
	NumPlayers = IPS_Ru8(a_Data);
	IPS_Ru8(a_Data);	// Max Players
	
	// Ignore Map
	IPS_Rs(a_Data, NULL, 0);
	
	/* Handle WADs */
	// Clean before WADs
	WADp = 0;
	memset(WADs, 0, sizeof(WADs));
	
	// Load
	NumWADs = IPS_Ru8(a_Data);
	for (i = 0; i < NumWADs; i++)
	{
		// Read from stream
		IPS_Rs(a_Data, Buf, BUFSIZE);
		
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_SERVERWAD, "%s\n", Buf);
		
		// Attempt opening of WAD
		FoundWAD = WL_OpenWAD(Buf);
		
		// Not found? Will need to download
		if (!FoundWAD)
		{
			// TODO FIXME
			return false;
		}
		
		// Append to list
		if (WADp < MAXWADS - 1)
			WADs[WADp++] = FoundWAD;
	}
	
	IPS_Ru8(a_Data);	// DM
	IPS_Ru8(a_Data);	// Skill
	
	// Teamplay/CTF
	if (IPS_Ru8(a_Data))
		TeamPlayOn = true;
	if (IPS_Ru8(a_Data))
		TeamPlayOn = true;
	
	// Player Info
	for (i = 0; i < NumPlayers; i++)
	{
		IPS_Rs(a_Data, NULL, 0);	// Name
		IPS_Ri16(a_Data);
		IPS_Ri32(a_Data);
		IPS_Ru8(a_Data);
	}
	
	// WAD Hashes (ignore for now)
	for (i = 0; i < NumWADs; i++)
		IPS_Rs(a_Data, NULL, 0);
	
	IPS_Rs(a_Data, NULL, 0);	// ???
	
	// Teamplay
	if (TeamPlayOn)
	{
		IPS_Ri32(a_Data);
		
		for (i = 0; i < ODA_NUMTEAMS; i++)
			if (IPS_Ru8(a_Data) != 0)
				IPS_Ri32(a_Data);
	}
	
	// Version
	a_Data->SVVersion = IPS_Ru8(a_Data);
	
	// Cap version
	if (a_Data->SVVersion > ODA_VERSION)
		a_Data->SVVersion = ODA_VERSION;
	
	// TODO FIXME: Rest of packet
	
	/* Switch to WADs */
	// Lock OCCB
	WL_LockOCCB(true);
	
	// Pop all WADs
	while (WL_PopWAD())
		;
	
	// Load new WADs
	for (i = 0; i < WADp; i++)
	{
		WL_PushWAD(WADs[i]);
		
		// Push remood.wad
		if (i == 0)
			WL_PushWAD(WL_OpenWAD("remood.wad"));
	}
	
	// Close unneeded WADs
	WL_CloseNotStacked();
	
	// Unlock OCCB
	WL_LockOCCB(false);
	
	return true;
#undef BUFSIZE
}

/* IPS_ODA_GetUserInfo() -- Loads player info */
static void IPS_ODA_GetUserInfo(IP_OdaData_t* const a_Data)
{
	uint8_t PID;
	uint32_t UUID;
	D_XPlayer_t* XPlayer;
	
	/* Obtain player ID */
	PID = IPS_Ru8(a_Data);
	
	// Based on unique ID
	UUID = PID + 1;
	
	// TODO FIXME
}

/* IPS_ODA_MagicCVAR() -- Maps Odamex CVAR to ReMooD Variable */
static void IPS_ODA_MagicCVAR(IP_OdaData_t* const a_Data, const char* const a_Name, const char* const a_Value)
{
	CONL_OutputUT(CT_NETWORK, DSTR_IPC_ODAVIRTCVAR, "%s%s\n", a_Name, a_Value);
}

/* IPS_ODA_ParseSVSettings() -- Parses server settings */
static void IPS_ODA_ParseSVSettings(IP_OdaData_t* const a_Data)
{
#define BUFSIZE 128
	uint8_t Code;
	char Name[BUFSIZE];
	char Value[BUFSIZE];
	
	/* CVAR Read Loop */
	for (;;)
	{
		// Read Code
		Code = IPS_Ru8(a_Data);
		
		// End?
		if (Code == 2)
			break;
		
		// Read Name, read value
		IPS_Rs(a_Data, Name, BUFSIZE);
		IPS_Rs(a_Data, Value, BUFSIZE);
		
		// No string?
		if (!Name[0])
			break;
		
		// Map to ReMooD
		IPS_ODA_MagicCVAR(a_Data, Name, Value);
	}
#undef BUFSIZE
}

/* IPS_ODA_PrintMessage() -- Server sends message */
static void IPS_ODA_PrintMessage(IP_OdaData_t* const a_Data)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint8_t Level;
	
	/* Read Message Data */
	Level = IPS_Ru8(a_Data);
	IPS_Rs(a_Data, Buf, BUFSIZE);
	
	/* Print to screen */
	CONL_PrintF("%s\n", Buf);
#undef BUFSIZE
}

/* IPS_ODA_Decompress() -- Decompress packet */
static void IPS_ODA_Decompress(IP_OdaData_t* const a_Data)
{
	uint8_t Method;
	lzo_uint Len;
	
	/* Read Compression Method */
	Method = IPS_Ru8(a_Data);
	
	/* MiniLZO Compressed? */
	if (Method & 0x8)
	{
		// Remove from mask
		Method &= ~0x8;
		
		// Setup buffer
		memset(a_Data->LZOBuf, 0, sizeof(a_Data->LZOBuf));
		Len = MAXLZOLEN;
		
		// Decompress
		lzo1x_decompress_safe(
			a_Data->PackAt,
			a_Data->PackEnd - a_Data->PackAt,
			a_Data->LZOBuf, &Len, NULL);
		
		// Well this is cool
		a_Data->PackAt = a_Data->LZOBuf;
		a_Data->PackEnd = &a_Data->LZOBuf[Len];
	}
	
	/* Remaining compression masks? */
	if (Method)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_IPC_SVUNKENCODING, "\n");
		
		// Error Out!
		a_Data->PackAt = a_Data->PackEnd;
	}
}

/* IP_ODA_RunConnF() -- Runs connection */
void IP_ODA_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
{
	IP_OdaData_t* Data;
	int32_t MsgID, i;
	
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
	
	/* Get Data */
	Data = a_Conn->Data;
	
	/* Handle network packets */
	while (IPS_ODA_ReadPacket(Data))
		// Not Connected
		if (Data->NetMode == IPOCS_REQUESTINFO ||
			Data->NetMode == IPOCS_WAITINFO ||
			Data->NetMode == IPOCS_STARTSYNC ||
			Data->NetMode == IPOCS_WAITSYNC)
		{
			// Read Message Type
			MsgID = IPS_Ri32(Data);
			
			// Got launcher challenge
			if (MsgID == ODA_CHAL && (Data->NetMode == IPOCS_REQUESTINFO || Data->NetMode == IPOCS_WAITINFO))
				if (!IPS_ODA_ReadChal(Data))
				{
					// Disconnect from server
					D_XNetDisconnect(false);
				}
				else
				{
					// Start sync to game
					Data->NetMode = IPOCS_STARTSYNC;
					Data->TimeOut = 0;
				}
			
			// Connected to game
			else if (MsgID == 0)
			{
				// Acknowledge server
				IPS_Wm(Data, clc_ack);
				IPS_Wi32(Data, 0);
				IPS_ODA_WritePacket(Data);
				
				// Init
				Data->SVPacketNum = 0;
				
				// Set to connected mode
				Data->NetMode = IPOCS_CONNECTED;
			}
		}
		
		// Connected
		else
		{
			// Acknowledge
			i = IPS_Ri32(Data);
			IPS_Wm(Data, clc_ack);
			IPS_Wi32(Data, i);
			Data->SVPacketNum++;
			
			// Decompress
			if (IPS_Pu8(Data) == svc_compressed)
			{
				// Eat the byte
				IPS_Ru8(Data);
				
				// Decompress it
				IPS_ODA_Decompress(Data);
			}
			
			// Handle Commands
			while (Data->PackAt < Data->PackEnd)
			{
				// Read Message Type
				MsgID = IPS_Rm(Data);
				
				// End?
				if (MsgID == 0xFF)
					break;
			
				// Which message?
				switch (MsgID)
				{
						// Ignore
					case svc_connectclient:
						break;
						
						// Server Settings
					case svc_serversettings:
						IPS_ODA_ParseSVSettings(Data);
						break;
						
						// Print Message
					case svc_print:
						IPS_ODA_PrintMessage(Data);
						break;
						
						// Set a player's info
					case svc_userinfo:
						IPS_ODA_GetUserInfo(Data);
						break;
						
						// Unknown
					default:
						// Warning
						CONL_OutputUT(CT_NETWORK, DSTR_IPC_UNKODA, "%hhi\n", MsgID);
						
						// Oh well
						Data->PackAt = Data->PackEnd;
					
						// Disconnect from server
						//D_XNetDisconnect(false);
						return;
				}
			}
			
			// Send Packet
			IPS_ODA_WritePacket(Data);
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
			IPS_ODA_WritePacket(Data);
			
			// Change to wait on info
			Data->NetMode = IPOCS_WAITINFO;
			Data->TimeOut = g_ProgramTic + TICRATE;
			break;
		
			// Start game synchronization
		case IPOCS_STARTSYNC:
			// Information
			CONL_OutputUT(CT_NETWORK, DSTR_IPC_SVSERVERSYNC, "\n");
			
			// Send Message
			IPS_Wi32(Data, ODA_CHAL);
			IPS_Wi32(Data, Data->SVToken);
			IPS_Wi16(Data, Data->SVVersion);
			IPS_Wu8(Data, 0);	// We want to play
			IPS_Wi32(Data, ODA_GAMEVER);
			
			// Send current profile info
			IPS_ODA_SendProfile(Data);
			
			IPS_Wi32(Data, ODA_RATE);	// TODO FIXME: cvar
			
			IPS_Ws(Data, "");	// TODO FIXME: Password
			
			IPS_ODA_WritePacket(Data);
			
			// Change to wait on sync
			Data->NetMode = IPOCS_WAITSYNC;
			Data->TimeOut = g_ProgramTic + TICRATE;
			break;
		
			// Waiting for server info/sync
		case IPOCS_WAITINFO:
		case IPOCS_WAITSYNC:
			// Timed out?
			if (g_ProgramTic > Data->TimeOut)
			{
				if (Data->NetMode == IPOCS_WAITSYNC)
				{
					Data->NetMode = IPOCS_STARTSYNC;
					Data->SyncTries++;
				}
				else
					Data->NetMode = IPOCS_REQUESTINFO;
				Data->TimeOut = 0;
			}
			
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


