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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
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

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_clisrv.h"
#include "z_zone.h"
#include "i_util.h"
#include "d_block.h"
#include "console.h"
#include "p_info.h"

/*************
*** LOCALS ***
*************/

static tic_t l_MapTime = 0;		// Map local time

/****************
*** FUNCTIONS ***
****************/

/* D_SyncNetDebugMessage() -- Debug message for syncrhonized networking */
void D_SyncNetDebugMessage(const char* const a_Format, ...)
{
#define BUFSIZE 512
	va_list ArgPtr;
	char Text[BUFSIZE];
	
	/* Check */
	if (!M_CheckParm("-devnet"))
		return;
		
	// Make
	va_start(ArgPtr, a_Format);
	vsnprintf(Text, BUFSIZE, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	// Print
	fprintf(stderr, "%s\n", Text);
#undef BUFSIZE
}

/* D_SyncNetIsArbiter() -- Do we control the game? */
bool_t D_SyncNetIsArbiter(void)
{
	return true;
}

/* D_SyncNetSetMapTime() -- Sets the new map time */
void D_SyncNetSetMapTime(const tic_t a_Time)
{
	l_MapTime = a_Time;
}

/* D_SyncNetMapTime() -- Returns the current map time */
tic_t D_SyncNetMapTime(void)
{
	return l_MapTime;
}

/* D_SyncNetRealTime() -- Returns the real game time */
tic_t D_SyncNetRealTime(void)
{
	/* Just return the number of tics that has passed */
	return I_GetTimeMS() / (tic_t)TICSPERMS;
}

extern consvar_t cv_g_gamespeed;

/* D_SyncNetIsPaused() -- Returns true if the game is paused */
bool_t D_SyncNetIsPaused(void)
{
	if (paused || (!netgame && menuactive && !demoplayback))
		return true;
	return false;
}

/* D_SyncNetIsSolo() -- Is solo game (non-networked) */
bool_t D_SyncNetIsSolo(void)
{
	return true;
}

/* D_SyncNetAllReady() -- Inidicates that all parties are ready to move to the next tic */
// It returns the tics in the future that everyone is ready to move to
tic_t D_SyncNetAllReady(void)
{
	static tic_t LocalTime = 0;
	tic_t ThisTime, DiffTime;
	
	/*** START BIG HACK AREA ***/
	static fixed_t CurVal, ModVal, TicsPerMS = TICSPERMS;
	
	if (!TicsPerMS)
		TicsPerMS = TICSPERMS;
	
	/* Slow */
#if 0
	if (CurVal != cv_g_gamespeed.value)
	{
		ModVal = CurVal = cv_g_gamespeed.value;
		if (ModVal < 16384)		// limit to 0.25 speed
			ModVal = 16384;
			
		ModVal = FixedDiv(1 << FRACBITS, cv_g_gamespeed.value);
		
		// Calculate new speed
		TicsPerMS = FixedMul((TICSPERMS << FRACBITS), ModVal) >> FRACBITS;
		
		if (TicsPerMS < 1)
			TicsPerMS = 1;
	}
#endif
	
	/*** END BIG HACK AREA ***/
	
	/* If we are the server, we dictate time */
	if (D_SyncNetIsArbiter())
	{
		// The map time is determined by the framerate
		ThisTime = I_GetTimeMS() / TICSPERMS;
		DiffTime = ThisTime - LocalTime;
		
		if (DiffTime > 0)
		{
			// Return the time difference
			LocalTime = ThisTime;
			return l_MapTime + DiffTime;
		}
		else
			return l_MapTime;
	}
	
	/* Otherwise time gets dictated to us */
	else
	{
		return l_MapTime;
	}
	
	/* Fell through? */
	return (tic_t)-1;
}

/* D_SyncNetUpdate() -- Update synchronized networking */
bool_t D_SyncNetUpdate(void)
{
	/* Old Update Code */
	NetUpdate();
	
	/* Update all networked players */
	D_NCSNetUpdateAll();
	
	/* Update network code */
	D_NCUpdate();
	
	/* Success */
	return true;
}

/*****************************************************************************/

/*** STRUCTURES ***/

/* D_NetQueueCommand_t -- Net queue */
typedef struct D_NetQueueCommand_s
{
	D_NCQCFunc_t Func;							// Function to execute
	void* Data;									// Data to pass
} D_NetQueueCommand_t;

/*** GLOBALS ***/

uint32_t g_NetStat[4] = {0, 0, 0, 0};			// Network stats

/*** LOCALS ***/

static D_NetQueueCommand_t** l_ComQueue = NULL;	// Command Queue
static size_t l_NumComQueue = 0;				// Number of commands

static uint32_t l_LocalStat[4];					// Local Stats

static D_NetClient_t** l_Clients = NULL;		// Networked Clients
static size_t l_NumClients = 0;					// Number of net clients

/*** FUNCTIONS ***/

/* D_NCAllocClient() -- Creates a new network client */
D_NetClient_t* D_NCAllocClient(void)
{
	size_t i;
	D_NetClient_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_NETWORK, NULL);
	
	/* Find free spot in list */
	for (i = 0; i < l_NumClients; i++)
		if (!l_Clients[i])
			return (l_Clients[i] = New);
	
	/* Append to end */
	Z_ResizeArray((void**)&l_Clients, sizeof(*l_Clients), l_NumClients, l_NumClients + 1);
	return (l_Clients[l_NumClients++] = New);
}

/* D_NCFindClientByNetPlayer() -- Finds client by net player */
D_NetClient_t* D_NCFindClientByNetPlayer(D_NetPlayer_t* const a_NetPlayer)
{
	/* Check */
	if (!a_NetPlayer)
		return NULL;
}

/* D_NCFindClientByHost() -- Finds client by host name */
D_NetClient_t* D_NCFindClientByHost(I_HostAddress_t* const a_Host)
{
	/* Check */
	if (!a_Host)
		return NULL;
}

/* D_NCFindClientByPlayer() -- Find client by player */
D_NetClient_t* D_NCFindClientByPlayer(struct player_s* const a_Player)
{
	/* Check */
	if (!a_Player)
		return NULL;
}

/* D_CheckNetGame() -- Checks whether the game was started on the network */
bool_t D_CheckNetGame(void)
{
	I_NetSocket_t* Socket;
	D_NetClient_t* Client;
	bool_t ret = false;
	size_t i;
	
	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	
	multiplayer = false;
	
	// only dos version with external driver will return true
	netgame = false;
	if (netgame)
		netgame = false;
		
	/* Create LoopBack Client */
	Client = D_NCAllocClient();
	Client->CoreStream = D_RBSCreateLoopBackStream();
	
	// Create perfection Wrapper
	Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
	// Set read/writes for all streams
	Client->Streams[DNCSP_READ] = Client->CoreStream;
	Client->Streams[DNCSP_WRITE] = Client->CoreStream;
	Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
	Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
	
	/* Create Local Network Client */
	// Attempt creating a UDP Server
	Socket = NULL;
	for (i = 0; i < 20 && !Socket; i++)
		Socket = I_NetOpenSocket(true, NULL, __REMOOD_BASEPORT + i);
	
	// Initial input/output of stream
	if (Socket)
	{
		// Allocate
		Client = D_NCAllocClient();
		
		// Copy socket
		Client->NetSock = Socket;
		
		// Create stream from it
		Client->CoreStream = D_RBSCreateNetStream(Client->NetSock);
		
		// Create encapsulated perfect stream
		Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
		// Set read/writes for all streams
		Client->Streams[DNCSP_READ] = Client->CoreStream;
		Client->Streams[DNCSP_WRITE] = Client->CoreStream;
		Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
		Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
	}
	
	return ret;
}

/* D_NCAddQueueCommand() -- Add command */
void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data)
{
	size_t i;
	D_NetQueueCommand_t* NQC;	
	
	/* Add somewhere in an empty spot */
	NQC = NULL;
	for (i = 0; i < l_NumComQueue; i++)
		if (!l_ComQueue[i])
		{
			NQC = l_ComQueue[i] = Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
			break;
		}
	
	// No Spot?
	if (!NQC)
	{
		Z_ResizeArray((void**)&l_ComQueue, sizeof(*l_ComQueue), l_NumComQueue, l_NumComQueue + 1);
		NQC = l_ComQueue[l_NumComQueue++] = Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
	}
	
	/* Slap data in */
	NQC->Func = a_Func;
	NQC->Data = a_Data;
}

/* D_NCRunCommands() -- Run all commands in Queue */
void D_NCRunCommands(void)
{
	size_t i;
	D_NetQueueCommand_t* NQC;
	
	/* Go through each one */
	while (l_NumComQueue && l_ComQueue[0])
	{
		// Get
		NQC = l_ComQueue[0];
		
		// Check
		if (!NQC)
			break;
		
		// Execute
		NQC->Func(NQC->Data);
		
		// Wipe away
		Z_Free(NQC);
		
		// Move all down
		l_ComQueue[0] = NULL;
		memmove(l_ComQueue, l_ComQueue + 1, sizeof(*l_ComQueue) * (l_NumComQueue - 1));
		l_ComQueue[l_NumComQueue - 1] = NULL;
	}
}

/* D_NCUpdate() -- Update all networking stuff */
void D_NCUpdate(void)
{
	char Header[5];
	D_RBlockStream_t* Stream, *OutStream, *GenOut;
	D_NetClient_t* NetClient;
	size_t nc, snum, i;
	I_HostAddress_t FromAddress;
	
	bool_t SendPing;
	uint32_t ThisTime, DiffTime;
	static uint32_t LastTime;
	
	uint32_t u32a, u32b, u32c, u32d;
	
	/* Send Ping Request? */
	ThisTime = I_GetTimeMS();
	
	// Send pings?
	SendPing = false;
	if (ThisTime > LastTime + 1000)
	{
		DiffTime = ThisTime - LastTime;
		LastTime = ThisTime;
		SendPing = true;
		
		// Set global stat count to current local stats
		for (i = 0; i < 4; i++)
			g_NetStat[i] = l_LocalStat[i];
	}
	
	/* Clear local stats */
	// This is for traffic monitoring
	for (i = 0; i < 4; i++)
		l_LocalStat[i] = 0;
	
	/* Go through each client and read/write commands */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Failed?
		if (!NetClient)
			continue;
		
		// Initialize some things
		memset(&FromAddress, 0, sizeof(FromAddress));
		
		// Use base stream initially
		Stream = NetClient->CoreStream;
		
		// Sending a ping?
			// If sending, send command and unstat stream
		if (SendPing)
		{
			// Unstat the stream
			D_RBSUnStatStream(Stream);
			
			// Build PING command
			D_RBSBaseBlock(Stream, "PING");
			D_RBSWriteUInt32(Stream, ThisTime);
			D_RBSWriteUInt32(Stream, DiffTime);
			D_RBSRecordBlock(Stream);	// Send to default destination
		}
			// Otherwise, Stat the stream and add to local counts
		else
		{
			// Stat it
			D_RBSStatStream(Stream, &u32a, &u32b, &u32c, &u32d);
			
			// Add to local
			l_LocalStat[0] += u32a;
			l_LocalStat[1] += u32b;
			l_LocalStat[2] += u32c;
			l_LocalStat[3] += u32d;
		}
		
		// Read from the "Perfect" Stream
			// The perfect stream knows whether a packet is perfect or not and
			// if a perfect packet is not yet ready it won't return any of them
			// Also, a read stream might not exist, a client could be using
			// another clients stream for reading, this would be the case for
			// network games over UDP. Why? Becuase all network players write
			// to the server (the local client) for commands.
		Stream = NetClient->Streams[DNCSP_PERFECTREAD];
		OutStream = NetClient->Streams[DNCSP_PERFECTWRITE];
		
		GenOut = NetClient->Streams[DNCSP_WRITE];
		
		// Constantly read command packets
		memset(Header, 0, sizeof(Header));
		if (Stream)
		{
			// Constantly Read
			while (D_RBSPlayNetBlock(Stream, Header, &FromAddress))
			{
				// Debug?
				if (devparm)
					D_SyncNetDebugMessage("%i Got \"%c%c%c%c\" (From %08x:%i)...",
						(int)nc, Header[0], Header[1], Header[2], Header[3],
							SwapUInt32(FromAddress.Host.v4.u),
							FromAddress.Port
						);
				
				// Everything -- Ping request
				if (D_RBSCompareHeader("PING", Header))
				{
					// Send PONG back to the from address (using generic stream)
					u32a = D_RBSReadUInt32(Stream);		// Rem: ThisTime
					u32b = D_RBSReadUInt32(Stream);		// Rem: DiffTime
					u32c = ThisTime;					// Loc: ThisTime
					
					// Create response and send away
					D_RBSBaseBlock(GenOut, "PONG");
					D_RBSWriteUInt32(GenOut, u32a);
					D_RBSWriteUInt32(GenOut, u32b);
					D_RBSWriteUInt32(GenOut, u32c);
					D_RBSRecordNetBlock(GenOut, &FromAddress);
				}
				
				// Everything -- Pong reply
				else if (D_RBSCompareHeader("PONG", Header))
				{
				}
				
				// Master Server -- Request List
				else if (D_RBSCompareHeader("MSRQ", Header))
				{
					// Read Cookie (Basic Security)
					u32a = D_RBSReadUInt32(Stream);
					u32b = D_RBSReadUInt32(Stream);
					
					// Setup Base Info
					D_RBSBaseBlock(GenOut, "MSLS");
					D_RBSWriteUInt32(GenOut, u32a);
					D_RBSWriteUInt32(GenOut, u32b);
					
					// Send Server Info
					D_RBSWriteUInt8(GenOut, 'R');	// Auto-remote end
					D_NSZZ_SendINFO(GenOut);
					
					// Send away
					D_RBSRecordNetBlock(GenOut, &FromAddress);
				}
				
				// Master Server -- List
				else if (D_RBSCompareHeader("MSLS", Header))
				{
					// Read Cookie (Basic Security)
					u32a = D_RBSReadUInt32(Stream);
					u32b = D_RBSReadUInt32(Stream);
				}
				
				// Server -- Request Game Info
				else if (D_RBSCompareHeader("RINF", Header))
				{
					// Read Cookie (Basic Security)
					u32a = D_RBSReadUInt32(Stream);
					u32b = D_RBSReadUInt32(Stream);
					
					// Write INFO
					D_RBSBaseBlock(GenOut, "INFO");
					D_RBSWriteUInt32(GenOut, u32a);
					D_RBSWriteUInt32(GenOut, u32b);
					
					// Send Server Info
					D_NSZZ_SendINFO(GenOut);
					
					// Send away
					D_RBSRecordNetBlock(GenOut, &FromAddress);
				}
				
				// Client -- Recieve Game Info
				else if (D_RBSCompareHeader("INFO", Header))
				{
				}
				
				// Clear from address
				memset(&FromAddress, 0, sizeof(FromAddress));
				memset(Header, 0, sizeof(Header));
			}
			
			// Flush write streams
			D_RBSFlushStream(OutStream);
			D_RBSFlushStream(GenOut);
		}
	}
	
#if 0
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char Header[5];
	size_t nc, i, j, p;
	D_NetController_t* CurCtrl, *OtherCtrl, *HostCtrl;
	D_RBlockStream_t* Stream, *OtherStream;
	
	D_NetPlayer_t* NetPlayer;
	D_ProfileEx_t* Profile;
	player_t* DoomPlayer;
	
	uint32_t u32, u32b, u32c, u32d;
	uint8_t u8;
	
	bool_t SendPing, AnythingWritten;
	uint32_t ThisTime, DiffTime;
	static uint32_t LastTime;
	
	/* Get Host */
	HostCtrl = l_LocalController;
	
	/* Init */
	memset(Header, 0, sizeof(Header));
	
	/* Get Current Time */
	ThisTime = I_GetTimeMS();
	
	// Send pings?
	SendPing = false;
	if (ThisTime > LastTime + 1000)
	{
		DiffTime = ThisTime - LastTime;
		LastTime = ThisTime;
		SendPing = true;
	}
	
	/* Go through every controller */
	for (nc = 0; nc < l_NumControllers; nc++)
	{
		// Get current
		CurCtrl = l_Controllers[nc];
		
		// Nothing here?
		if (!CurCtrl)
			continue;
		
		// Init some things
		Stream = CurCtrl->BlockStream;
		
		// Send ping command
		if (SendPing)
		{
			// Clear stream stats
			D_RBSUnStatStream(Stream);
			for (i = 0; i < 4; i++)
			{
				g_NetStat[i] = l_LocalStat[i];
				l_LocalStat[i] = 0;
			}
			
			// Create ping
			D_RBSBaseBlock(Stream, "PING");
			D_RBSWriteUInt32(Stream, ThisTime);
			D_RBSWriteUInt32(Stream, DiffTime);
			
			// Record it
			D_RBSRecordBlock(Stream);
		}
		
		// Collect some infos
		else
		{
			// Stats
			D_RBSStatStream(Stream, &u32, &u32b, &u32c, &u32d);
			
			// Add to local
			l_LocalStat[0] = u32;
			l_LocalStat[1] = u32b;
			l_LocalStat[2] = u32c;
			l_LocalStat[3] = u32d;
		}
		
		// Constantly read blocks (packets)
		while (D_RBSPlayBlock(Stream, Header))
		{
			// PING -- Ping Request
			if (strcasecmp("PING", Header) == 0)
			{
				// Send a PONG back to it
				D_RBSRenameHeader(Stream, "PONG");
				D_RBSWriteUInt32(Stream, ThisTime);
				D_RBSRecordBlock(Stream);
			}
			
			// PONG -- Ping Reply
			else if (strcasecmp("PONG", Header) == 0)
			{
				CurCtrl->Ping = ThisTime - D_RBSReadUInt32(Stream);
			}
			
			// VERR -- Version Request
			else if (strcasecmp("VERR", Header) == 0)
			{
				// Create version reply
				D_RBSBaseBlock(Stream, "VERI");
				
				// Put in info
				D_RBSWriteUInt8(Stream, VERSION);
				D_RBSWriteUInt8(Stream, REMOOD_MAJORVERSION);
				D_RBSWriteUInt8(Stream, REMOOD_MINORVERSION);
				D_RBSWriteUInt8(Stream, REMOOD_RELEASEVERSION);
				D_RBSWriteString(Stream, REMOOD_FULLVERSIONSTRING);
				D_RBSWriteString(Stream, REMOOD_URL);
				
				// Send it away
				D_RBSRecordBlock(Stream);
			}
			
			// VERI -- Version Information
			else if (strcasecmp("VERI", Header) == 0)
			{
				// Read version info
				CurCtrl->VerLeg = D_RBSReadUInt8(Stream);
				CurCtrl->VerMaj = D_RBSReadUInt8(Stream);
				CurCtrl->VerMin = D_RBSReadUInt8(Stream);
				CurCtrl->VerRel = D_RBSReadUInt8(Stream);
			}
			
			// MESG -- Generic Message
			else if (strcasecmp("MESG", Header) == 0)
			{
				// Get Message
				memset(Buf, 0, sizeof(Buf));
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				// Print
				CONL_PrintF("%s\n", Buf);
			}
			
			// MAPC -- Map Change
			else if (strcasecmp("MAPC", Header) == 0)
			{
				// Only accept if from a server
				if (CurCtrl->IsServer)
				{
					// Read map name
					memset(Buf, 0, sizeof(Buf));
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					
					// Add to command queue
					D_NCAddQueueCommand(D_NCQC_MapChange, Z_StrDup(Buf, PU_NETWORK, NULL));
				}
			}
			
			// LPRJ -- Local Player, Request Join (Split screen)
			else if (strcasecmp("LPRJ", Header) == 0)
			{
				// Only accept if we are the server
				if (HostCtrl->IsServer && HostCtrl->IsLocal)
				{
					// Get Player UUID/AccountName
					memset(Buf, 0, sizeof(Buf));
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					
					// Find player
					NetPlayer = D_NCSFindNetPlayer(Buf);
					
					// Find free player spot
					for (p = 0; p < MAXPLAYERS; p++)
						if (!playeringame[p])
							break;
					
					// Only resume request joining when there is no player
						// And the current local player count fits local players
						// And there is a free player spot
					if (!NetPlayer && CurCtrl->NumArbs < MAXSPLITSCREEN && p < MAXPLAYERS)
					{
						// Create player
						NetPlayer = D_NCSAllocNetPlayer();
						
						// Copy UUID Over
						strncpy(NetPlayer->UUID, Buf, MAXPLAYERNAME * 2);
						
						// Add to arbitrating players
						Z_ResizeArray((void**)&CurCtrl->Arbs, sizeof(*CurCtrl->Arbs),
								CurCtrl->NumArbs, CurCtrl->NumArbs + 1);
						CurCtrl->Arbs[CurCtrl->NumArbs++] = NetPlayer;
						
						// Create Profile
						D_RBSReadString(Stream, NetPlayer->AccountName, MAXPLAYERNAME);
						
						// If server split, use local profile
						if (CurCtrl->IsLocal)
						{
							// Try finding it
							Profile = D_FindProfileEx(NetPlayer->AccountName);
							
							// If not found, make fresh then
							if (!Profile)
								Profile = D_CreateProfileEx(NetPlayer->AccountName);
						}
						
						// Otherwise use remote profile
						else
							Profile = D_CreateProfileEx(NetPlayer->AccountName);
						
						// Fill info
						if (CurCtrl->IsLocal)	// Server Split
							Profile->Flags |= DPEXT_LOCAL;
						else					// Client split
							Profile->Flags |= DPEXT_NETWORK;
						D_RBSReadString(Stream, Profile->DisplayName, MAXPLAYERNAME);
						Profile->Color = D_RBSReadUInt8(Stream);
						
						// Inform everyone that a player has joined
						for (j = 0; j < l_NumControllers; j++)
						{
							// Get Other
							OtherCtrl = l_Controllers[j];
		
							// Nothing here?
							if (!OtherCtrl)
								continue;
		
							// Init some things
							OtherStream = OtherCtrl->BlockStream;
							
							// Write player join OK
							D_RBSBaseBlock(OtherStream, "PJOK");
							D_RBSWriteString(OtherStream, NetPlayer->UUID);
							D_RBSWriteUInt8(OtherStream, p);
							if (OtherStream == CurCtrl)
							{
								D_RBSWriteUInt8(OtherStream, CurCtrl->NumArbs);
								D_RBSWriteString(OtherStream, Profile->UUID);
							}
							else
							{
								D_RBSWriteUInt8(OtherStream, 0);
								D_RBSWriteString(OtherStream, "");
							}
							D_RBSWriteString(OtherStream, NetPlayer->AccountName);
							D_RBSWriteString(OtherStream, Profile->AccountName);
							D_RBSWriteString(OtherStream, Profile->DisplayName);
							D_RBSWriteUInt8(OtherStream, Profile->Color);
							D_RBSRecordBlock(OtherStream);
						}
						
						// Create Player In Local Server
						DoomPlayer = G_AddPlayer(p);
						DoomPlayer->NetPlayer = NetPlayer;
						DoomPlayer->ProfileEx = Profile;
						Profile->NetPlayer = NetPlayer;
						NetPlayer->Player = DoomPlayer;
						NetPlayer->Profile = Profile;
						G_InitPlayer(DoomPlayer);
						
						// Check split screen
						if (CurCtrl->IsLocal)
							for (j = 0; j < MAXSPLITSCREEN; j++)
								if (!g_PlayerInSplit[j])
								{
									g_PlayerInSplit[j] = true;
									consoleplayer[j] = displayplayer[j] = p;
									
									g_SplitScreen = j;
									break;
								}
					}
				}
			}
			
			// PJOK -- Player Join OK
			else if (strcasecmp("PJOK", Header) == 0)
			{
				// Only accept if from a server and non-local
					// Non-local because if it is local then the player would
					// already be in the structures.
				if (CurCtrl->IsServer && !CurCtrl->IsLocal && CurCtrl != HostCtrl)
				{
					// Create network player
					NetPlayer = D_NCSAllocNetPlayer();
					
					// Add to arbitrating players
					Z_ResizeArray((void**)&CurCtrl->Arbs, sizeof(*CurCtrl->Arbs),
							CurCtrl->NumArbs, CurCtrl->NumArbs + 1);
					CurCtrl->Arbs[CurCtrl->NumArbs++] = NetPlayer;
					
					// Read NetPlayer UUID and the local player number
					D_RBSReadString(Stream, NetPlayer->UUID, MAXPLAYERNAME * 2);
					p = D_RBSReadUInt8(Stream);
					
					// Determine if the player is our own screen player
					u8 = D_RBSReadUInt8(Stream);
					D_RBSReadString(Stream, Buf, BUFSIZE);
					
					// See if it is worth looking for a profile
					if (u8)
						Profile = D_FindProfileEx(NetPlayer->UUID);
					
					// No profile found or remote profile
					if (!Profile)
					{
						// Create blank slate
						u8 = 0;
						Profile = D_CreateProfileEx(NetPlayer->UUID);
						
						// Fill with guessed info
						Profile->Type = DPEXT_NETWORK;
						D_RBSReadString(Stream, NetPlayer->AccountName, MAXPLAYERNAME);
						D_RBSReadString(Stream, Profile->AccountName, MAXPLAYERNAME);
						D_RBSReadString(Stream, Profile->DisplayName, MAXPLAYERNAME);
						Profile->Color = D_RBSReadUInt8(Stream);
					}
					else
						Profile->Type = DPEXT_LOCAL;
					
					// Create Player In Local Game
					DoomPlayer = G_AddPlayer(p);
					DoomPlayer->NetPlayer = NetPlayer;
					DoomPlayer->ProfileEx = Profile;
					Profile->NetPlayer = NetPlayer;
					NetPlayer->Player = DoomPlayer;
					NetPlayer->Profile = Profile;
					G_InitPlayer(DoomPlayer);
					
					// Add player to split screen
					if (u8)
					{
						// Find free screen
						for (j = 0; j < MAXSPLITSCREEN; j++)
							if (!g_PlayerInSplit[j])
							{
								g_PlayerInSplit[j] = true;
								consoleplayer[j] = displayplayer[j] = p;
								
								g_SplitScreen = j;
								break;
							}
					}
				}
			}
		}
		
		// Flush commands (Send them together, if possible)
		D_RBSFlushStream(Stream);
	}
#undef BUFSIZE
#endif
}

/*** NCSR FUNCTIONS ***/

/* D_NCSR_RequestMap() -- Requests that the map changes */
void D_NCSR_RequestMap(const char* const a_Map)
{
#if 0
	D_NetController_t* Ctrl;
	size_t i;
	
	/* Check */
	if (!a_Map)
		return;
	
	/* Find Server Player */
	Ctrl = D_NCGetServer();
	
	// Not found?
	if (!Ctrl)
		return;
	
	/* Check if the server is also local */
	// This means we don't own the game
	if (!Ctrl->IsLocal)
		return;
	
	/* Send map change to server */
	for (i = 0; i < l_NumControllers; i++)
	{
		// Get controller
		Ctrl = l_Controllers[i];
		
		// Bad?
		if (!Ctrl)
			continue;
		
		// Not the server?
		if (!Ctrl->IsServer)
			continue;
		
		// Write map message
		D_RBSBaseBlock(Ctrl->BlockStream, "MAPC");
		D_RBSWriteString(Ctrl->BlockStream, a_Map);
		D_RBSRecordBlock(Ctrl->BlockStream);
	}
#endif
}

/* D_NCSR_RequestNewPlayer() -- Requests that a local profile join remote server */
void D_NCSR_RequestNewPlayer(struct D_ProfileEx_s* a_Profile)
{
#if 0
	D_NetController_t* Ctrl;
	D_RBlockStream_t* Stream;
	size_t i;
	
	/* Check */
	if (!a_Profile)
		return;
	
	/* Find Server Player */
	Ctrl = D_NCGetServer();
	
	// Not found?
	if (!Ctrl)
		return;
		
	// Use server stream
	Stream = Ctrl->BlockStream;
	
	/* Tell server to add player */
	D_RBSBaseBlock(Stream, "LPRJ");
	D_RBSWriteString(Stream, a_Profile->UUID);
	D_RBSWriteString(Stream, a_Profile->AccountName);
	D_RBSWriteString(Stream, a_Profile->DisplayName);
	D_RBSWriteUInt8(Stream, a_Profile->Color);
	D_RBSRecordBlock(Stream);
#endif
}

/*** NSZZ FUNCTIONS ***/

/* D_NSZZ_SendINFO() -- Send server info */
void D_NSZZ_SendINFO(struct D_RBlockStream_s* a_Stream)
{
	/* Wfrite Version */
	D_RBSWriteUInt8(a_Stream, VERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_MAJORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_MINORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_RELEASEVERSION);
}

/*** NCQC FUNCTIONS ***/

/* D_NCQC_MapChange() -- Change Map */
void D_NCQC_MapChange(void* const a_Data)
{
	char* MapName;
	P_LevelInfoEx_t* Info;
	
	/* Check */
	if (!a_Data)
		return;
	
	/* Get Map */
	MapName = (char*)a_Data;
	
	/* Switch to that level */
	// Try finding the level
	Info = P_FindLevelByNameEx(MapName, NULL);
	
	// Load the level
	if (!Info)
		CONL_PrintF("NET: Could not find level.\n");
	else
		if (!P_ExLoadLevel(Info, 0))
			CONL_PrintF("NET: Could not load level.\n");
	
	/* Free string (was Z_StrDup) */
	Z_Free(a_Data);
}


