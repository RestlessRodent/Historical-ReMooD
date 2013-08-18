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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
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
// DESCRIPTION: Game Controllers

/***************
*** INCLUDES ***
***************/

#include "sn.h"
#include "p_demcmp.h"
#include "d_player.h"
#include "p_inter.h"
#include "p_mobj.h"
#include "console.h"
#include "dstrings.h"
#include "d_netcmd.h"
#include "p_info.h"
#include "s_sound.h"
#include "g_state.h"
#include "g_game.h"
#include "p_local.h"
#include "p_spec.h"

/****************
*** CONSTANTS ***
****************/

// c_ChatPrefix -- Chat prefix for player screens
static const char* const c_ChatPrefix[4] =
{
	"", "\x4", "\x5", "\x6"
};

/****************
*** FUNCTIONS ***
****************/

/* SN_ChangeVar() -- Change value of variable */
void SN_ChangeVar(const uint32_t a_Code, const int32_t a_Value)
{
	uint8_t* Wp;
	
	/* Server Only */
	if (!SN_IsServer())
		return;
	
	/* Grab global command */
	if (!SN_ExtCmdInGlobal(DTCT_GAMEVAR, &Wp))
		return;
	
	/* Write Data */
	LittleWriteUInt32((uint32_t**)&Wp, a_Code);
	LittleWriteInt32((int32_t**)&Wp, a_Value);
}

/* SN_DirectChat() -- Direct chat message */
void SN_DirectChat(const uint32_t a_HostID, const uint32_t a_ID, const uint8_t a_Mode, const uint32_t a_Target, const char* const a_Message)
{
	const char* p;
	int32_t Room;
	uint8_t* Wp;
	SN_Host_t* Host;
	SN_Port_t* Port;
	
	/* Check */
	if (!a_HostID || !a_ID || !a_Message || !strlen(a_Message))
		return;
	
	/* Get host and port */
	Host = SN_HostByID(a_HostID);
	Port = SN_PortByID(a_ID);
	
	// Bad host and/or port
	if (!Host || !Port || Port->Host != Host)
		return;
	
	/* Encode message into multiple tics */
	for (p = a_Message, Room = 0; *p; p++)
	{
		// No room? create new global
		if (!Room)
		{
			// Get new packet
			if (!SN_ExtCmdInGlobal(DTCT_SNCHATFRAG, &Wp))
				return;
			
			LittleWriteUInt32((uint32_t**)&Wp, a_HostID);
			LittleWriteUInt32((uint32_t**)&Wp, a_ID);
			WriteUInt8((uint8_t**)&Wp, 0);
			
			LittleWriteUInt32((uint32_t**)&Wp, a_Target);
			WriteUInt8((uint8_t**)&Wp, a_Mode);
			
			Room = MAXTCSTRINGCAT;
		}
		
		// Write single character and reduce the room available
		WriteUInt8((uint8_t**)&Wp, *p);
		Room--;
	}
}

/* SN_RemovePlayer() -- Remove player from game */
void SN_RemovePlayer(const int32_t a_PlayerID)
{
	uint8_t* Wp;
	
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return;
	
	/* Encode directly on server */
	if (SN_IsServer())
	{
		if (SN_ExtCmdInGlobal(DTCT_SNPARTPLAYER, &Wp))
		{
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			WriteUInt8((uint8_t**)&Wp, a_PlayerID);
		}
	}
	
	/* Otherwise, ask the server to quit */
	else
		SN_ReqSpectatePort(a_PlayerID);
}

/* SN_ChangeMap() -- Changes the map */
void SN_ChangeMap(const char* const a_NewMap, const bool_t a_Reset)
{
	uint8_t* Wp;
	P_LevelInfoEx_t* Info;
	int32_t i, j;	
	
	/* Check */
	if (!a_NewMap)
		return;
	
	/* See if the map exists first */
	if (!(Info = P_FindLevelByNameEx(a_NewMap, NULL)))
		return;
	
	/* Server */
	if (SN_IsServer())
	{
		// Attempt global grab
		if (SN_ExtCmdInGlobal(DTCT_MAPCHANGE, &Wp))
		{
			// Resetting players?
			WriteUInt8((uint8_t**)&Wp, a_Reset);
			
			// Map name
			for (i = 0, j = 0; i < 8; i++)
				if (!j)
				{
					WriteUInt8((uint8_t**)&Wp, Info->LumpName[i]);
				
					if (!Info->LumpName[i])
						j = 1;
				}
				else
					WriteUInt8((uint8_t**)&Wp, 0);
		}
	}
	
	/* Client */
	else
	{
	}
}

/* SN_HGTJoinPlayer() -- Handle join player */
static void SN_HGTJoinPlayer(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	int32_t i, n;
	SN_Host_t* Host;
	SN_Port_t* Port;
	player_t* Player;
	D_SplitInfo_t* Split;
	uint8_t VTeam, Color;
	uint32_t Flags;
	
	/* Out of bounds? */
	if (a_PID < 0 || a_PID >= MAXPLAYERS)
		return;
	
	/* Already taken */
	if (playeringame[a_PID])
		return;
	
	/* Read settings */
	VTeam = ReadUInt8(a_PP);
	Color = ReadUInt8(a_PP);
	Flags = LittleReadUInt32((const uint32_t**)a_PP);
	
	// Cap values
	VTeam = VTeam % P_XGSVal(PGS_PLMAXTEAMS);
	Color = Color % MAXSKINCOLORS;
	
	/* Count existing players in game */
	for (i = n = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			n++;
	
	// Set multiplayer mode and spawn multiplayer junk
	if (n > 0 && !P_XGSVal(PGS_COMULTIPLAYER))
	{
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
		P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
	}
	
	/* If no host exists, create one */
	if (!(Host = a_Host))
	{
		// Base
		Host = SN_CreateHost();
		
		// Set pointers and such
		Host->ID = a_HID;
		Host->Local = false;
	}
	
	/* If no port exists, create one */
	if (!(Port = a_Port))
	{
		// Base
		Port = SN_AddPort(Host);
		
		// Fields
		Port->ID = a_UID;
		Port->Bot = false;
	}
	
	/* Mark in game */
	playeringame[a_PID] = true;
	
	/* Create player in the game */
	Port->Player = Player = G_AddPlayer(a_PID);
	
	// Set local fields
	Player->Port = Port;
	Player->skincolor = Color;
	
	P_ChangePlVTeam(Player, VTeam);
	P_ChangeCounterOp(Player, !!(Flags & DTCJF_MONSTERTEAM));
	
	/* Update Scores */
	P_UpdateScores();
	
	/* Spawn player into game */
	// Control monster, initially
	if (P_GMIsCounter() && Player->CounterOpPlayer)
		P_ControlNewMonster(Player);
	
	// Spawn them normally
	else
		G_DoReborn(a_PID);
	
	/* Bind to split screens, if any on the local side */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
		{
			// Current split
			Split = &g_Splits[i];
			
			// Matches port
			if (Split->Port == Port)
			{
				Split->Active = true;
				Split->Waiting = false;
				Split->Console = Split->Display = a_PID;
				
				localaiming[i] = 0;
				if (Player->mo)
					localangle[i] = Player->mo->angle;
				else
					localangle[i] = 0;
				
				// Set profile to broadcast data changes
				SN_SetPortProfile(Port, Split->Profile);
			}
		}
	
	/* Tell bot to leave statis mode */
	BOT_LeaveStasis(Port);
	
	/* If this is a local port, transmit settings to the server */
	// The server does not know the full extent of our own port settings.
	// Although it is transmitted already possibly (maybe packet loss?).
	// So generally, this is here because join player does not fully contain
	// information on the player's settings. Nobody can use the port settings
	// because it could cause a desync.
	if (Host->Local)
	{
		// Integer Based Settings
		SN_PortSetting(Port, DSNPS_VTEAM, a_Port->VTeam, NULL, 0);
		SN_PortSetting(Port, DSNPS_COLOR, a_Port->Color, NULL, 0);
		SN_PortSetting(Port, DSNPS_COUNTEROP, a_Port->CounterOp, NULL, 0);
		
		// String Based Settings
		SN_PortSetting(Port, DSNPS_NAME, 0, a_Port->Name, 0);
	}
}

/* SN_HGTPartPlayer() -- Handle leaving player */
static void SN_HGTPartPlayer(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	player_t* Player;
	int32_t i;
	mobj_t* Mo;
	
	/* Out of bounds? */
	if (a_PID < 0 || a_PID >= MAXPLAYERS)
		return;
	
	/* Get player */
	Player = &players[a_PID];
	
	/* Count as suicide against self */
	// This is for any scripting, held flags, etc.
	Mo = Player->mo;
	
	if (Mo)	// Do not kill a counter-op controlled monster however
		if (!P_GMIsCounter() || (P_GMIsCounter() && !Player->CounterOpPlayer))
			P_KillMobj(Mo, Mo, Mo);
	
	// Player is no longer in game
	if (Mo)
		Mo->player = NULL;
	Player->mo = NULL;
	playeringame[a_PID] = false;
	
	/* Make local split a spectator */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
			if (g_Splits[i].Console == a_PID)
			{
				// Remove console view
				g_Splits[i].Console = -1;
				
				// If F12ing other player, keep F12
				if (g_Splits[i].Display != g_Splits[i].Console)
					g_Splits[i].Display = -1;
				
				// Make inactive
				g_Splits[i].Active = false;
				
				if (!demoplayback)
					g_Splits[i].Waiting = true;
			}
	
	/* Unassign port */
	Player->Port = NULL;
	
	if (a_Port)
		a_Port->Player = NULL;
	
	/* Tell bot to enter statis mode */
	BOT_EnterStatis(a_Port);
}

/* SN_HGTCleanupHost() -- Handle delete host */
static void SN_HGTQuitMsg(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	char Buf[MAXTCSTRINGCAT + 1];
	int32_t Cat, i;
	
	/* Need Host */
	if (!a_Host)
		return;
	
	/* Concat? */
	Cat = ReadUInt8(a_PP);
	
	/* Read Message */
	for (i = 0; i < MAXTCSTRINGCAT; i++)
		Buf[i] = ReadUInt8(a_PP);
	Buf[MAXTCSTRINGCAT] = 0;
	
	/* Add to reason */
	// Clear reason if not cat
	if (!Cat)
		memset(a_Host->QuitReason, 0, sizeof(a_Host->QuitReason));
	
	// Cat always
	strncat(a_Host->QuitReason, Buf, MAXQUITREASON - 1);
}

/* SN_HGTCleanupHost() -- Handle delete host */
static void SN_HGTCleanupHost(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	/* Check */
	if (!a_Host)
		return;
	
	/* Call cleanup */
	if (!SN_IsServer())
		SN_CleanupHost(a_Host);
}

/* SN_HGTJoinHost() -- Host joins the game */
static void SN_HGTJoinHost(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	SN_Host_t* New;	
	
	/* Create host if it does not exist */
	if (!a_Host)
	{
		New = SN_CreateHost();
		New->ID = a_HID;
	}
	
	/* Display message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTCONNECTED, "%s\n", "Client");
}

/* SN_HGTJoinPort() -- Handles joining of a new port */
static void SN_HGTJoinPort(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	SN_Host_t* Host;
	SN_Port_t* New;
	
	/* Create host if it does not exist */
	if (!(Host = a_Host))
	{
		Host = SN_CreateHost();
		Host->ID = a_HID;
	}
	
	/* If port does not exist, create it */
	// Give packet may have reached client already
	if (!a_Port)
	{
		/* Create new port belonging to this host */
		New = SN_AddPort(Host);
	
		// Set fields
		New->ID = a_UID;
		New->ProcessID = LittleReadUInt32((const uint32_t**)a_PP);
	}
	
	// Does not exist
	else
		New = a_Port;
	
	/* Display message */
	// As long as one was never displayed...
	if (!New->AttachMsg)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_NET_PORTCONNECTED, "%s\n", "Client");
		New->AttachMsg = true;
	}
}

/* SN_HGTChatFrag() -- Chat fragment */
static void SN_HGTChatFrag(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	D_SplitInfo_t* Split;
	SN_Port_t* PortTarget, *Port;
	uint32_t Target;
	uint8_t Mode, Char;
	int32_t i, k, l, Sound, RealTeam;
	bool_t Ended;
	
	/* Check */
	if (!a_Host || !a_Port)
		return;	
	
	/* Read values */
	Target = LittleReadUInt32((const uint32_t**)a_PP);
	Mode = ReadUInt8(a_PP);
	
	/* Append to their chat line */
	for (Ended = false, i = 0; i < MAXTCSTRINGCAT; i++)
	{
		// Read character
		Char = ReadUInt8(a_PP);
		
		// Append
		if (!Ended && Char)
		{
			if (a_Port->ChatAt < MAXCHATLINE - 1)
				a_Port->ChatBuf[a_Port->ChatAt++] = Char;
			continue;
		}
		
		// Ended (passed /0)
		Ended = true;
	}
	
	/* Did not speak */
	if (!Ended)
		return;
		
	/* Get target */
	PortTarget = SN_PortByID(Target);
	
	/* Speak for all local screens */
	Sound = 0;
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		// Split inactive?
		if (!D_ScrSplitVisible(i))
			continue;
		
		// Split port
		Split = &g_Splits[i];
		Port = Split->Port;
		
		// Sending to team
		if (Mode == 1 || (a_Port->Player && Mode == 2))
		{
			// Get team of player
			k = P_GetPlayerTeam(a_Port->Player);
			l = P_GetPlayerTeam(Port->Player);
		
			// Spectating or on different team
			if (a_Port != Port)
				if (!Port->Player || (k == -1 || l == -1) || k != l)
					continue;
		
			// Recolor team
			RealTeam = k;
			P_GetTeamInfo(k, &RealTeam, NULL);
		
			// More than 10, use a
			if (RealTeam >= 10)
				RealTeam += 'a';
			else
				RealTeam += '0';
		
			// Colorized
			CONL_OutputUT(CT_CHAT, DSTR_GGAMEC_CHATTEAM,
					"%s%s%c",
					c_ChatPrefix[i],
					SN_GetPortName(a_Port),
					RealTeam
				);
		}
		
		// Send to spectators
		else if (Mode == 2 || (!a_Port->Player && Mode == 1))
		{
			// Port is playing the game
			if (Port->Player)
				continue;
			
			// Send message
			CONL_OutputUT(CT_CHAT, DSTR_GGAMEC_CHATSPEC,
					"%s%s",
					c_ChatPrefix[i],
					SN_GetPortName(a_Port)
				);
		}
		
		// Send to one person
		else if (Mode == 3)
		{
			// Not the target port
			if (Port != PortTarget)
				continue;
			
			// Send message
			CONL_OutputUT(CT_CHAT, DSTR_GGAMEC_CHATPRIV,
					"%s%s",
					c_ChatPrefix[i],
					SN_GetPortName(a_Port)
				);
		}
		
		// Send to all
		else
		{
			CONL_OutputUT(CT_CHAT, DSTR_GGAMEC_CHATALL,
					"%s%s",
					c_ChatPrefix[i],
					SN_GetPortName(a_Port)
				);
		}
		
		// Append actual message
		CONL_PrintF("%s\n", a_Port->ChatBuf);
		
		// Emit generic sound
		if (!Sound)
			Sound = sfx_generic_chat;
	}
	
	/* Clear chat buffer */
	a_Port->ChatAt = 0;
	memset(a_Port->ChatBuf, 0, sizeof(a_Port->ChatBuf));
	
	/* Emit Sound */
	if (Sound)
		S_StartSound(NULL, Sound);
}

/* SN_HGTPortSetting() -- Change setting of a port */
static void SN_HGTPortSetting(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
#define BUFSIZE (MAXTCSTRINGCAT + 1)
	uint16_t Setting;
	const uint8_t* OldPPd;
	uint8_t Buf[BUFSIZE];
	player_t* PlayerP;
	
	int32_t IntVal, i;
	
	/* Read Setting and values */
	Setting = LittleReadUInt16((const uint16_t**)a_PP);
	PlayerP = &players[a_PID];
	
	// Illegal Setting
	if (Setting < 0 || Setting >= NUMDSNPS)
		return;
	
	// Value is either a string or an integer, but takes both places at once
	OldPPd = *a_PP;
	IntVal = LittleReadInt32((const int32_t**)a_PP);
	*a_PP = OldPPd;
	
	memset(Buf, 0, sizeof(Buf));
	for (i = 0; i < MAXTCSTRINGCAT; i++)
		Buf[i] = ReadUInt8(a_PP);
	
	/* Change setting of port */
	// But do not change settings for a local port, because it is always up to
	// date. However, if there is packet loss, the server might not see a
	// setting we see locally. May be problem, but gamestate wise the player
	// will know exactly which settings were transmitted.
	if (a_Port && !a_Port->Host->Local)
		SN_PortSettingOnPort(a_Port, Setting, IntVal, Buf, MAXTCSTRINGCAT);
	
	/* Change setting of player */
	if (a_PID >= 0 && a_PID < MAXPLAYERS)
		switch (Setting)
		{
			case DSNPS_VTEAM:
				P_ChangePlVTeam(PlayerP, IntVal);
				break;
			
			case DSNPS_COLOR:
				PlayerP->skincolor = IntVal % MAXSKINCOLORS;
				
				// Change color of thing
				if (!PlayerP->CounterOpPlayer &&
					PlayerP->mo)
				{
					PlayerP->mo->flags &= ~MF_TRANSLATION;
					PlayerP->mo->flags |= (PlayerP->skincolor) << MF_TRANSSHIFT;
				}
				break;
			
			case DSNPS_NAME:
				D_NetSetPlayerName(a_PID, Buf);
				break;
			
			case DSNPS_COUNTEROP:
				P_ChangeCounterOp(PlayerP, !!IntVal);
				break;
		}
#undef BUFSIZE
}

/* SN_HGTUnPlugPort() -- Unplugs port from game */
static void SN_HGTUnPlugPort(const uint8_t a_ID, const uint8_t** const a_PP, SN_Host_t* const a_Host, SN_Port_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	/* If host exists, show message */
	if (a_Host)
		CONL_OutputUT(CT_NETWORK, DSTR_NET_PORTDISCONNECTED, "%s\n", "Client");
	
	/* Only if this port is valid, is it to be removed */
	// Quite possible that we are the server and it was already removed!
	if (!a_Port)
		return;
	
	/* Remove it now */
	SN_RemovePort(a_Port);
}

/* SN_HandleGT() -- Handles game command IDs */
void SN_HandleGT(const uint8_t a_ID, const uint8_t** const a_PP)
{
	uint32_t HID, ID;
	uint8_t PID;
	SN_Port_t* Port;
	SN_Host_t* Host;
	
	/* Check */
	if (!a_ID || !a_PP)
		return;
	
	/* All start with ID and PID */
	HID = LittleReadUInt32((const uint32_t**)a_PP);
	ID = LittleReadUInt32((const uint32_t**)a_PP);
	PID = ReadUInt8((const uint8_t**)a_PP);
	
	// Find port and host
	Port = SN_PortByID(ID);
	
	Host = NULL;
	if (Port)
		Host = Port->Host;
	else
		Host = SN_HostByID(HID);
	
	/* Which Command? */
	switch (a_ID)
	{
			// Player Joins Game
		case DTCT_SNJOINPLAYER:
			SN_HGTJoinPlayer(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Disconnect reason
		case DTCT_SNQUITREASON:
			SN_HGTQuitMsg(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Delete Host
		case DTCT_SNCLEANUPHOST:
			SN_HGTCleanupHost(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Create Host
		case DTCT_SNJOINHOST:
			SN_HGTJoinHost(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Player leaves game
		case DTCT_SNPARTPLAYER:
			SN_HGTPartPlayer(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Port is added to host
		case DTCT_SNJOINPORT:
			SN_HGTJoinPort(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Chat Fragment
		case DTCT_SNCHATFRAG:
			SN_HGTChatFrag(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Setting
		case DTCT_SNPORTSETTING:
			SN_HGTPortSetting(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Unplug Port
		case DTCT_SNUNPLUGPORT:
			SN_HGTUnPlugPort(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Unknown!?!
		default:
			break;
	}
}

