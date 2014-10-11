// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Network transmission

/***************
*** INCLUDES ***
***************/

#include "sn.h"
#include "i_util.h"
#include "console.h"
#include "dstrings.h"
#include "p_saveg.h"
#include "p_demcmp.h"
#include "p_info.h"
#include "g_state.h"
#include "z_zone.h"
#include "d_player.h"
#include "w_wad.h"
#include "d_main.h"
#include "d_netcmd.h"
#include "i_system.h"

/****************
*** CONSTANTS ***
****************/

/* D_ClientStage_t -- Client connect stage */
typedef enum D_ClientStage_e
{
	DCS_LISTWADS,
	DCS_CHECKWADS,
	DCS_DOWNLOADWADS,
	DCS_SWITCHWADS,
	DCS_REQUESTSAVE,
	DCS_GETSAVE,
	DCS_CANPLAYNOW,
	DCS_PLAYING
} D_ClientStage_t;

#define GHOSTS (*g_HostsP)
#define GNUMHOSTS (*g_NumHostsP)

/*****************
*** STRUCTURES ***
*****************/

#define CHAINSIZE 128
#define ENCODESIZE 16384
#define MAXJOBHOSTS 64

#define JOBDEFAULTTARGET	3					// target at 3 tics retrans
#define JOBMINTICCAP		5					// Do not retransmit tics to far ahead

#define SERVERPINGOUTWARN (TICRATE * 5)
#define SERVERPINGOUTTIME (TICRATE * 30)

/* D_WADChain_t -- WAD Chain */
typedef struct D_WADChain_s
{
	char DOS[CHAINSIZE];
	char Full[CHAINSIZE];
	char Sum[CHAINSIZE];
	const WL_WADFile_t* WAD;
	struct D_WADChain_s* Prev;
	struct D_WADChain_s* Next;
	bool_t Want;								// Wants
} D_WADChain_t;

/* D_JobHost_t -- Remote host specifier in a job */
typedef struct D_JobHost_s
{
	SN_Host_t* Host;							// Host pointer
	int32_t ArrID;								// Array ID
	bool_t Clear;								// Clear host
	
	tic_t LastSend;								// Last Send
	tic_t NextSend;								// Time to send next
	tic_t Counter;								// Counter
	tic_t Target;								// Target delay
} D_JobHost_t;

/* D_XMitJob_t -- Transmission jobs, who needs what */
typedef struct D_XMitJob_s
{
	tic_t GameTic;								// Tic job is for
	uint8_t EncData[ENCODESIZE];				// Encoded data buffer
	uint32_t EncSize;							// Encode size
	D_JobHost_t Dests[MAXJOBHOSTS];				// Job hosts
	int32_t NumDests;							// Number of destinations
} D_XMitJob_t;

/**************
*** GLOBALS ***
**************/

extern SN_Host_t*** g_HostsP;
extern int32_t* g_NumHostsP;
extern int32_t g_IgnoreWipeTics;

/*************
*** LOCALS ***
*************/

static I_NetSocket_t* l_Sock;
static I_HostAddress_t l_HostAddr;
static D_BS_t* l_BS;
static D_WADChain_t* l_WADTail;
static D_ClientStage_t l_Stage;
static uint8_t l_IsIWADMap, l_IsFreeDoom, l_IWADMission;
static tic_t l_LastRanTime;

static D_XMitJob_t l_XStack[MAXNETXTICS];		// Transmission stack
static int32_t l_XAt;							// Current transmission at

static SN_PingWin_t l_SvPings[MAXPINGWINDOWS];	// Ping windows
static int8_t l_SvPingAt;						// Current window
static tic_t l_SvNextPing;						// Time of next ping
static tic_t l_SvLastPing;						// Last ping time
static int32_t l_SvPing;						// Ping of server

// net_lanbroadcast -- Broadcast to LAN
CONL_StaticVar_t l_NETLanBroadcast =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"net_lanbroadcast", DSTR_CVHINT_CONMONOSPACE, CLVVT_STRING, "true",
	NULL
};

/****************
*** FUNCTIONS ***
****************/

/* SN_DeleteJob() -- Delets job from list */
// Assumes for loop with i++ in for statement
static void SN_DeleteJob(D_XMitJob_t* const a_Job, int32_t* const a_iP)
{
	int32_t i;
	D_XMitJob_t* Job;
	
	/* Find job in buffer */
	for (i = 0; i < l_XAt; i++)
		if ((Job = &l_XStack[i]) == a_Job)
		{
			// Wipe any trace of the job
			memmove(&l_XStack[i], &l_XStack[i + 1], sizeof(l_XStack[0]) * (MAXNETXTICS - (i)));
			l_XAt--;
			
			// If i was passed, lower value
			if (a_iP)
				if (*a_iP >= i)
					*a_iP -= 1;
			
			// Job was deleted
			return;
		}
}

/* SN_DeleteIndexInJob() -- Deletes index in this job */
static void SN_DeleteIndexInJob(D_XMitJob_t* const a_Job, int32_t* const a_iP)
{
	int32_t i;
	
	/* Check */
	if (!a_Job || !a_iP)
		return;
	
	/* Move down */
	for (i = *a_iP; i < a_Job->NumDests; i++)
		a_Job->Dests[i] = a_Job->Dests[i + 1];
	
	// Decrement
	*a_iP -= 1;
	a_Job->NumDests -= 1;
}

/* SN_ClearJobs() -- Clears all jobs */
void SN_ClearJobs(void)
{
	/* Loop deletion */
	while (l_XAt > 0)
		SN_DeleteJob(&l_XStack[0], NULL);
}

/* SN_XMitTics() -- Transmit tics to local client */
void SN_XMitTics(const tic_t a_GameTic, SN_TicBuf_t* const a_Buffer)
{
	static D_JobHost_t* HostL;
	static int32_t NumHostL, MaxHostL;
	
	D_XMitJob_t* Job;
	int32_t i;
	SN_Host_t* Host;
	uint8_t* OutD;
	
	/* Wipe the host list */
	if (HostL)
	{
		memset(HostL, 0, sizeof(*HostL) * MaxHostL);
		NumHostL = 0;
	}
	
	/* Go through hosts and determine who needs this nice new tic */
	for (i = 0; i < GNUMHOSTS; i++)
		if ((Host = GHOSTS[i]))
		{
			// Ignore local hosts (they are the server!)
			if (Host->Local)
				continue;
			
			// Host disconnected and will be cleaned up
			if (Host->Cleanup)
				continue;
			
			// Host is in initial connect stage
			if (!Host->Save.Has)
			{
				// Does not want save game
				if (!Host->Save.Want)
					continue;
			
				// This tic is before their join window tic
				if (a_GameTic < Host->Save.TicTime)
					continue;
			}
			
			// Need to resize host list?
			if (NumHostL >= MaxHostL)
			{
				Z_ResizeArray((void**)&HostL, sizeof(*HostL),
					MaxHostL, MaxHostL + 1);
				MaxHostL++;
			}
			
			// At end of jobs?
			if (NumHostL >= MAXJOBHOSTS - 1)
			{
				SN_DisconnectHost(Host, "No more jobs available.");
				continue;
			}
			
			// Place host here
			HostL[NumHostL].Host = Host;
			HostL[NumHostL].Target = JOBDEFAULTTARGET;
			HostL[NumHostL++].ArrID = i;
		}
	
	/* No hosts want this tic */
	if (!NumHostL)
		return;
	
	/* Place job at end */
	Job = NULL;
	if (l_XAt < MAXNETXTICS)
		Job = &l_XStack[l_XAt++];
	
	// really bad!
	if (!Job)
		return;
	
	/* Initialize job */
	memset(Job, 0, sizeof(*Job));
	Job->GameTic = a_GameTic;
	
	// Encode tic buffer and clone the data into the job
	SN_EncodeTicBuf(a_Buffer, &OutD, &Job->EncSize, DXNTBV_LATEST);
	memmove(Job->EncData, OutD, (Job->EncSize < ENCODESIZE ? Job->EncSize : ENCODESIZE));
	
	// Put all players that should get this job
	memmove(Job->Dests, HostL, sizeof(*HostL) * NumHostL);
	Job->NumDests = NumHostL;
}

/* SN_OkTics() -- Tics that can be run by the game */
int32_t SN_OkTics(tic_t* const a_LocalP, tic_t* const a_LastP)
{
	int32_t i;
	SN_Host_t* Host;
	int32_t SaveID;
	bool_t Kick;
	D_XMitJob_t* Job;
	tic_t LowTic;
	
	/* Do not move forward if not connected */
	if (!SN_IsConnected())
		return 0;	
	
	/* Server */
	if (SN_IsServer())
	{
		// Job tranmission buffer is almost full!
			// A client is lagging
		if (l_XAt >= MAXNETXTICS - 2)
		{
			*a_LastP = *a_LocalP;
			return 0;
		}
			
		// Get the earliest gametic in the job buffer
		LowTic = (tic_t)-1;
		for (i = 0; i < l_XAt; i++)
		{
			Job = &l_XStack[i];
			
			if (Job->GameTic < LowTic)
				LowTic = Job->GameTic;
		}
		
		// No low tics?
		if (LowTic == (tic_t)-1)
			LowTic = gametic;
		
		// Did not make save
		SaveID = -1;
		Kick = false;
		
		// Go through hosts
		for (i = 0; i < (*g_NumHostsP); i++)
			if ((Host = (*g_HostsP)[i]))
			{
				// Remote has no save game?
				if (!Host->Cleanup && !Host->Local && !Host->Save.Has && Host->Save.Want && gametic >= Host->Save.TicTime)
				{
					// Not latched
					if (!Host->Save.Latched)
					{
						// Latch
						Host->Save.PTimer = g_ProgramTic;
						Host->Save.Latched = true;
						
						// Setup save slot
						if (!Kick && SaveID < 0)
						{
							SaveID = SN_PrepSave();
							
							// Failed to make save
							if (SaveID < 0)
								Kick = true;
						}
						
						// Send file
						if (SaveID >= 0)
							SN_SendFile(SaveID, Host);
					}
					
					// Kick off player
					if (Kick)
					{
						SN_DisconnectHost(Host, "Failed to save game");
						continue;
					}
					
					// Reset time and delay until transfer
					*a_LastP = *a_LocalP;
					return 0;
				}
			}
		
		// Single tics, screw networked players
		if (singletics)
			return 1;
		
		// Not enough time has passed
		if (*a_LocalP <= *a_LastP)
			return 0;
		
		// Ignore tics during wipe
		if (g_IgnoreWipeTics)
		{
			g_IgnoreWipeTics = false;
			*a_LastP = *a_LocalP;
			return 0;
		}
		
		// Keep running normal time
		i = *a_LocalP - *a_LastP;
		*a_LastP = *a_LocalP;
		
		// If the run count exceeds the oldest job count
		if (gametic + i > LowTic + (MAXNETXTICS - 1))
			return 0;
		
		// Return count
		return i;
	}
	
	/* Client */
	else
	{
		return SN_NumSeqTics();
	}
}

/* SN_NetCreate() -- Creates network connection */
// Either listener or remote
bool_t SN_NetCreate(const bool_t a_Listen, const char* const a_Addr, const uint16_t a_Port)
{
	I_HostAddress_t Host;
	uint32_t Flags, Fails;
	uint16_t Port;
	
	static bool_t Regged;
	
	/* Register Vars */
	if (!Regged)
	{
		CONL_VarRegister(&l_NETLanBroadcast);
		
		Regged = true;
	}
	
	/* Check */
	// Need address if not listening
	if (!a_Listen && !a_Addr)
		return false;
	
	/* If socket exists, terminate */
	if (l_Sock)
		SN_NetTerm("Creating new socket");
	
	/* Create socket */
	// Clear
	memset(&Host, 0, sizeof(Host));
	
	// Initial Flags
	Flags = 0;
	
	// Setup host
	if (a_Addr)
		I_NetNameToHost(NULL, &Host, a_Addr);
	
	// IPv6?
	if (Host.IPvX == INIPVN_IPV6)
		Flags |= INSF_V6;
	
	// Port
	Port = 0;
	if (a_Port != 0)
		Port = a_Port;
		
		// Do not use port obtained from address if connecting
		// Because it will fail on the same system
	else if (a_Listen && Host.IPvX && Host.Port != 0)
		Port = Host.Port;
	
	// Cap, port
	if (Port < 1 || Port >= 65536)
	{
		// Use default port if hosting server
		if (a_Listen)
			Port = __REMOOD_BASEPORT;
		
		// Otherwise as client, use some random port
		else
		{
			do
			{
				Port = D_CMakePureRandom() >> UINT32_C(16);
			} while (Port < 32767 || Port >= 65535);
		}
	}
	
	// Create socket
	for (Fails = 0; Fails < 10; Fails++)
		if ((l_Sock = I_NetOpenSocket(Flags, (a_Listen && Host.IPvX ? &Host : NULL), Port + Fails)))
			break;	// it worked!
	
	// Failed completely?
	if (!l_Sock)
		return false;
	
	/* Copy host and setup stream */
	if (a_Listen)	// Copy port for server
		Host.Port = Port + Fails;
	memmove(&l_HostAddr, &Host, sizeof(Host));
	l_BS = D_BSCreateNetStream(l_Sock);
	
	/* Socket created */
	return true;
}

/* SN_NetTerm() -- Terminates network connection */
void SN_NetTerm(const char* const a_Reason)
{
	const char* Reason;
	int32_t i;
	
	/* If there is no socket, do not terminate */
	if (!l_Sock)
		return;
	
	/* Reason */
	if (a_Reason)
		Reason = a_Reason;
	else
		Reason = "None given";
	
	/* Send Bye */	
	D_BSBaseBlock(l_BS, "QUIT");
	D_BSws(l_BS, Reason);
	
	for (i = 0; i < 5; i++)
		D_BSRecordNetBlock(l_BS, &l_HostAddr);
	
	/* Drop all clients */
	SN_DropAllClients(a_Reason);
	
	/* Clear file transfers */
	SN_ClearFiles();
	
	/* Close socket */
	D_BSCloseStream(l_BS);
	l_BS = NULL;
	I_NetCloseSocket(l_Sock);
	l_Sock = NULL;
	
	/* Clear host */
	memset(&l_HostAddr, 0, sizeof(l_HostAddr));
	
	/* Disconnect message */
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_DISCONNED, "%s\n", Reason);
}

/* SN_HasSocket() -- Socket exists */
bool_t SN_HasSocket(void)
{
	return !!l_Sock;
}

/* SN_DisconnectHost() -- Disconnects another host */
void SN_DisconnectHost(SN_Host_t* const a_Host, const char* const a_Reason)
{
	int i;
	
	/* Check */
	if (!l_BS || !a_Host || !SN_IsServer() || a_Host->Local)
		return;
	
	/* Already cleaning up */
	if (a_Host->Cleanup)
		return;
	
	/* Cleanup */
	a_Host->Cleanup = true;
	
	// Change reason
	if (a_Reason)
	{
		strncpy(a_Host->QuitReason, (a_Reason ? a_Reason : "No Reason"), MAXQUITREASON);
		a_Host->QuitReason[MAXQUITREASON - 1] = 0;
	}
	
	/* Send them a packet */
	D_BSBaseBlock(l_BS, "QUIT");
	D_BSws(l_BS, a_Host->QuitReason);
	for (i = 0; i < 5; i++)
		D_BSRecordNetBlock(l_BS, &a_Host->Addr);
}

/* SN_RequestPortNet() -- Request port from the network */
void SN_RequestPortNet(const uint32_t a_ProcessID)
{
	/* If there is no socket, do not ask */
	// Also do not ask if we are the server
	if (!l_BS || !l_Sock || SN_IsServer())
		return;
	
	/* Build packet */
	D_BSBaseBlock(l_BS, "WANT");
	
	D_BSwu32(l_BS, a_ProcessID);
	D_BSwcu64(l_BS, g_ProgramTic);
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
}

/* SN_UnplugPortNet() -- Request that the server unplug this port */
void SN_UnplugPortNet(SN_Port_t* const a_Port)
{
	/* Check */
	if (!a_Port)
		return;
}

/* SN_PortJoinGame() -- Request that port join the game */
void SN_PortJoinGame(SN_Port_t* const a_Port)
{
	/* Check */
	if (!l_BS || !l_Sock || SN_IsServer() || !a_Port)
		return;
		
	/* Build packet */
	D_BSBaseBlock(l_BS, "PJGG");
	
	D_BSwu32(l_BS, a_Port->ID);
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
}

/* SN_ReqSpectatePort() -- Request port spectate */
void SN_ReqSpectatePort(const uint32_t a_Player)
{
	/* Check */
	if (!l_BS || !l_Sock || SN_IsServer() || a_Player < 0 || a_Player >= MAXPLAYERS)
		return;
	
	/* Build packet */
	D_BSBaseBlock(l_BS, "RSPP");
	
	D_BSwi32(l_BS, a_Player);
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
}

/* SN_WaitingForSave() -- Waiting for save game */
bool_t SN_WaitingForSave(void)
{
	// Only if wanting save
	if (l_Sock && SN_IsConnected() && (l_Stage == DCS_REQUESTSAVE || l_Stage == DCS_GETSAVE))
		return true;
	return false;
}

/* SN_SendSyncCode() -- Sends sync code to client */
void SN_SendSyncCode(const tic_t a_GameTic, const uint32_t a_Code)
{
	/* Check */
	if (!l_BS || !l_Sock || SN_IsServer())
		return;
		
	/* Build packet */
	D_BSBaseBlock(l_BS, "SYNC");
	
	D_BSwcu64(l_BS, a_GameTic);
	D_BSwu32(l_BS, a_Code);
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
}

/* SN_SendChat() -- Sends chat command */
void SN_SendChat(SN_Port_t* const a_Port, const bool_t a_Team, const char* const a_Text)
{
	uint8_t Mode;
	const char* p;
	I_HostAddress_t* AddrP;
	SN_Port_t* Target;
	
	/* No origin */
	if (!a_Port || !a_Text)
		return;
	
	/* Base pointer */
	p = a_Text;
	Target = NULL;
	
	/* Who should recieve message? */
	if (a_Team)
		if (!a_Port->Player)
			Mode = 2;	// Spec
		else
			Mode = 1;	// Team
	else
		Mode = 0;		// Everyone
	
	/* Special commands */
	// Force team chat?
	if (!strncmp("/team ", p, 6))
	{
		Mode = 1;
		p += 6;
	}
	
	/* Limitize Value? */
	// Team mode but no teams
	if (Mode == 1 && !P_GMIsTeam())
		Mode = 0;
	
	/* Server can directly encode message */
	if (SN_IsServer())
	{
		SN_DirectChat(a_Port->Host->ID, a_Port->ID, Mode, 0, p);
	}
	
	/* Otherwise, send request to server */
	else if (l_Sock)
	{
		D_BSBaseBlock(l_BS, "CHAT");
	
		D_BSwu32(l_BS, a_Port->ID);
		D_BSwu8(l_BS, Mode);
		D_BSwu32(l_BS, (Target ? Target->ID : 0));
		D_BSwu32(l_BS, ++a_Port->ChatID);
		D_BSws(l_BS, p);
	
		D_BSRecordNetBlock(l_BS, &l_HostAddr);
	}
}

/* SN_SetLastTic() -- Sets the last tic running for */
void SN_SetLastTic(void)
{
	l_LastRanTime = g_ProgramTic;
}

/* SN_AppendLocalCmds() -- Append local commands */
void SN_AppendLocalCmds(D_BS_t* const a_BS)
{
	SN_Host_t* Host;
	SN_Port_t* Port;
	int32_t i, p;
	SN_TicBuf_t TicBuf;
	ticcmd_t* Cmd;
	uint8_t* OutD;
	uint32_t OutS;
	
	/* Check */
	if (!a_BS || !(Host = SN_MyHost()))
		return;
	
	/* Clear Buffer */
	memset(&TicBuf, 0, sizeof(TicBuf));
	
	/* Go through ports */
	for (i = 0; i < Host->NumPorts; i++)
		if ((Port = Host->Ports[i]))
		{
			// No commands to send or not playing
			if (!Port->LocalAt || !Port->Player)
				continue;
			
			// Get player number
			p = Port->Player - players;
			
			// Setup area to place commands
			Cmd = &TicBuf.Tics[p];
			TicBuf.PIGRevMask |= (1 << p);	// clear PIG mask
			
			// Merge commands into this
			D_XNetMergeTics(Cmd, Port->LocalBuf, Port->LocalAt);
			Port->LocalAt = 0;
			memset(Port->LocalBuf, 0, sizeof(Port->LocalBuf));
			
			// Do local turning and aiming
			SN_LocalTurn(Port, Cmd);
		}
	
	/* Check to see if anything was ever encoded */
	if (!TicBuf.PIGRevMask)
		return;
	
	// Reverse mask
	TicBuf.PIGRevMask = ~TicBuf.PIGRevMask;
	
	/* Encode and write */
	OutD = NULL;
	OutS = 0;
	SN_EncodeTicBuf(&TicBuf, &OutD, &OutS, DXNTBV_LATEST);
	
	// Write
	D_BSwu32(a_BS, OutS);
	D_BSWriteChunk(a_BS, OutD, OutS);
}

/* SN_SendSettings() -- Sends setting to server */
void SN_SendSettings(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen)
{
	const uint8_t* p;
	int32_t i;	
	
	/* Check */
	if (!l_BS || !l_Sock || SN_IsServer())
		return;
		
	/* Build packet */
	D_BSBaseBlock(l_BS, "SETT");
	
	// Write Data
	D_BSwu32(l_BS, a_Port->Host->ID);
	D_BSwu32(l_BS, a_Port->ID);
	D_BSwu8(l_BS, (a_Port->Player ? a_Port->Player - players : 255));
	
	D_BSwu16(l_BS, a_Setting);
	
	// String?
	if (a_StrVal)
	{
		// No length specified
		if (!a_StrLen)
			for (p = a_StrVal, i = 0; i < MAXTCSTRINGCAT; i++)
			{
				// Write
				D_BSwu8(l_BS, *p);
			
				// Increase p as long as it is not NULL
				if (*p)
					p++;
			}
		
		// Length Specified
		else
			for (i = 0; i < MAXTCSTRINGCAT; i++)
				if (i < a_StrLen)
					D_BSwu8(l_BS, ((uint8_t*)a_StrVal)[i]);
				else
					D_BSwu8(l_BS, 0);
	}
	
	// Integer
	else
	{
		D_BSwi32(l_BS, a_IntVal);
		
		// Padding
		for (i = 4; i < MAXTCSTRINGCAT; i++)
			D_BSwu8(l_BS, 0);
	}
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
}

/* SN_DoConnect() -- Do connection logic */
void SN_DoConnect(void)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	static tic_t Timeout;
	
	/* Not yet timed out */
	if (g_ProgramTic < Timeout)
		return;
	
	/* Time out in the future */
	Timeout = g_ProgramTic + TICRATE;
	I_NetHostToString(&l_HostAddr, Buf, BUFSIZE);
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_CONNECTING, "%s\n", Buf);
	
	/* Build connect info */
	D_BSBaseBlock(l_BS, "CONN");
	
	D_BSwu8(l_BS, SN_IsServer());
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
#undef BUFSIZE
}

/* SN_DoServer() -- Do server stuff */
void SN_DoServer(D_BS_t* const a_BS)
{
	D_XMitJob_t* Job;
	D_JobHost_t* JHost;
	SN_Host_t* Host;
	int32_t i, j;
	SN_PingWin_t* PWin;
	static int32_t LastSlot;
	tic_t PCap, GCap;
	static tic_t LastAdvert;
	I_HostAddress_t Addr;
	
	/* Transmit jobs to hosts that need them */
	for (i = 0; i < l_XAt; i++)
	{
		// Get current job
		Job = &l_XStack[i];
		
		// If job has no transmission sources remaining, delete
			// Or it is in the past!
		if (!Job->NumDests || (gametic > MAXNETXTICS && Job->GameTic < gametic - MAXNETXTICS))
		{
			SN_DeleteJob(Job, &i);
			continue;
		}
		
		// Go through destination hosts
		for (j = 0; j < Job->NumDests; j++)
		{
			// Get job host
			JHost = &Job->Dests[j];
			
			// Find host
			Host = NULL;
			if (JHost->Host == GHOSTS[JHost->ArrID])
				Host = JHost->Host;
			
			// Host is no longer a valid destination (or want to clear)
			if (JHost->Clear || !Host || (Host &&
					(Host->Cleanup || Host->MinTic > Job->GameTic)
				))
			{
				SN_DeleteIndexInJob(Job, &j);
				continue;
			}
			
			// Not ready to send yet
			if (JHost->NextSend > 0 && g_ProgramTic < JHost->NextSend)
				continue;
			
			// Calculate propogating delay for the next send request
			if (JHost->NextSend)
			{
				// Too far in the future (they do not need this tic yet)
				if (Job->GameTic > JHost->Host->MinTic + JOBMINTICCAP)
					continue;
				
				// Increase counter
				JHost->Counter++;
			
				// Counter exceeded target (increase target time)
				if (JHost->Counter >= JHost->Target)
				{
					JHost->Target += 2;
					JHost->Counter = 0;
				}
			}
			
			// Time to send the command again
			JHost->NextSend = g_ProgramTic + JHost->Target;
			JHost->LastSend = g_ProgramTic;
			
			// Build packet
			D_BSBaseBlock(a_BS, "JOBT");
			
			D_BSwcu64(a_BS, Job->GameTic);
			D_BSwu32(a_BS, Job->EncSize);
			D_BSWriteChunk(a_BS, Job->EncData, Job->EncSize);
			
			D_BSRecordNetBlock(a_BS, &Host->Addr);
		}
	}
	
	/* Determine if players get kicked, by timeouts */
	// The last seen time of the host is the time of the latest ack of a tic.
	// So if the game pauses because someone is dropping, then their timer will
	// incrase also.
	// Waiting for players after 3 seconds
	if (g_ProgramTic > l_LastRanTime + (TICRATE * 3))
		;	// TODO FIXME
	
	// Ping cap
		// TODO FIXME: make CVar
	if (g_ProgramTic < (TICRATE * 12))
		PCap = 0;
	else
		PCap = g_ProgramTic - (TICRATE * 12);
	
	// Gametic cap
	if (gametic < (MAXNETXTICS - 2))
		GCap = gametic;
	else
		GCap = gametic - (MAXNETXTICS - 2);
	
	// TODO FIXME: Make CVar rather than hardcoded (10 secs currently)
	if (g_ProgramTic > l_LastRanTime + (TICRATE * 10))
		for (i = 0; i < GNUMHOSTS; i++)
			if ((Host = GHOSTS[i]))
			{
				// Ignore locals and cleanups
				if (Host->Local || Host->Cleanup)
					continue;
				
				// Ping timeout
				if (Host->LastPing < PCap)
				{
					SN_DisconnectHost(Host, "Timed out");
					continue;
				}
				
				// This host appears to be frozen
					// Game too far in the past
				if (Host->MinTic <= GCap)
				{
					SN_DisconnectHost(Host, "Failed to acknowlegde");
					continue;
				}
			}
	
	/* Calculate Pings */
	// Lower calculating by slotting ping requests
	for (; LastSlot < GNUMHOSTS; LastSlot++)
		if ((Host = GHOSTS[LastSlot]))
		{
			// Ignore locals and cleanups
			if (Host->Local || Host->Cleanup)
				continue;
			
			// No ping yet
			if (g_ProgramTic < Host->NextPing)
				continue;
			
			// Ping again in two seconds (every second is not that important)
			Host->NextPing = g_ProgramTic + (TICRATE << 1);
			
			// Get the next ping window
			++Host->PingAt;
			Host->PingAt &= MAXPINGWINDOWS - 1;
			PWin = &Host->Pings[Host->PingAt];
			
			// Generate new random number
			PWin->Code = D_CMakePureRandom();
			PWin->SendTime = g_ProgramTic;
			PWin->Millis = I_GetTimeMS();
			
			// Build packet to send to them
			D_BSBaseBlock(l_BS, "PING");
			
			D_BSwu8(l_BS, Host->PingAt);
			D_BSwu32(l_BS, PWin->Code);
			
			D_BSRecordNetBlock(l_BS, &Host->Addr);
			
			// Do not ping anyone else
			LastSlot++;	// And go to next slot
			break;
		}
	
	// Ran out of slots
	if (LastSlot >= GNUMHOSTS)
		LastSlot = 0;
	
	/* Advertise server over LAN */
	if (l_NETLanBroadcast.Value->Int)
	{
		// Advertise to LAN
		if (g_ProgramTic < LastAdvert)
			return;
		
		// Advertise this
		LastAdvert = g_ProgramTic + (TICRATE * 10);
		
		// Currently just send to IPv4 multicast address
		memset(&Addr, 0, sizeof(Addr));
		if (I_NetNameToHost(l_Sock, &Addr, "224.0.0.167:29500"))
		{
			D_BSBaseBlock(l_BS, "ADVR");
			
			// Port because it is lost in a multicast
			D_BSwu16(l_BS, l_HostAddr.Port);
			
			D_BSRecordNetBlock(l_BS, &Addr);
		}
	}
}

/* SN_DoClient() -- Do client stuff */
void SN_DoClient(D_BS_t* const a_BS)
{
	static tic_t WADTimeout, SaveTimeout, PlayTimeout;
	D_WADChain_t* CWAD, *FirstCWAD;
	const WL_WADFile_t* WAD, *ExtraWAD;
	int32_t i, j, k, l;
	D_IWADInfoEx_t* Info;
	SN_PingWin_t* PWin;
	
	/* Send ping to server (to make sure it is alive) */
	if (g_ProgramTic >= l_SvNextPing)
	{
		// Ping again in two seconds (every second is not that important)
		l_SvNextPing = g_ProgramTic + (TICRATE << 1);
	
		// Get the next ping window
		++l_SvPingAt;
		l_SvPingAt &= MAXPINGWINDOWS - 1;
		PWin = &l_SvPings[l_SvPingAt];
	
		// Generate new random number
		PWin->Code = D_CMakePureRandom();
		PWin->SendTime = g_ProgramTic;
		PWin->Millis = I_GetTimeMS();
	
		// Build packet to send to them
		D_BSBaseBlock(l_BS, "PING");
	
		D_BSwu8(l_BS, l_SvPingAt);
		D_BSwu32(l_BS, PWin->Code);
	
		D_BSRecordNetBlock(l_BS, &l_HostAddr);
	}
	
	/* Server is going to ping out */
	if (g_ProgramTic > l_SvLastPing + SERVERPINGOUTWARN)
		SN_SetServerLagWarn(l_SvLastPing + SERVERPINGOUTTIME);
	else
		SN_SetServerLagWarn(0);

	/* Server is pinging out? */
	if (g_ProgramTic > l_SvLastPing + SERVERPINGOUTTIME)
		SN_PartialDisconnect("Connection to server lost");
	
	/* Which stage? */
	switch (l_Stage)
	{
			// Ask server for list of WADs
		case DCS_LISTWADS:
			if (g_ProgramTic < WADTimeout)
				return;
			
			WADTimeout = g_ProgramTic + TICRATE;
			D_BSBaseBlock(a_BS, "LIST");
			D_BSRecordNetBlock(a_BS, &l_HostAddr);
			return;
			
			// Check local WADs
		case DCS_CHECKWADS:
			// No WADs? Now we are feeling OK!
			if (!l_WADTail)
			{
				l_Stage = DCS_REQUESTSAVE;
				return;
			}
			
			// Go through all WADs
			FirstCWAD = 0;
			i = j = k = l = 0;
			for (CWAD = l_WADTail; CWAD; CWAD = CWAD->Prev)
			{
				// First?
				if (!CWAD->Prev && !FirstCWAD)
					FirstCWAD = CWAD;
				
				// Count of WADs
				i++;
				
				// If WAD is loaded, ignore
				if (CWAD->WAD)
					continue;
				
				// Otherwise, try to find the WAD file for this
				if (!(WAD = WL_OpenWAD(CWAD->Full, CWAD->Sum)))
					if (!(WAD = WL_OpenWAD(CWAD->DOS, CWAD->Sum)))
						if (!l_IsFreeDoom || (l_IsFreeDoom && !(WAD = WL_OpenWAD(CWAD->DOS, NULL))))	// Try by name if the server is using FreeDoom (maybe we have it too?)
						{
							// Need to download
							j++;
							CONL_OutputUT(CT_NETWORK, DSTR_DTRANSC_GETWAD, "%s\n",
								CWAD->Full);
						
							// Want this now, provided not a blacklisted sum
							if (!D_CheckWADBlacklist(CWAD->Sum))
								CWAD->Want = true;
							else
								CONL_OutputUT(CT_NETWORK, DSTR_DTRANSC_BLACKLIST, "%s\n",
									CWAD->Full);
						
							// Try other WADs
							continue;
						}
				
				// Got this WAD
				if (WAD)
				{
					CWAD->WAD = WAD;
					k++;
				}
			}
			
			// Need at least one WAD file
			if (j > 0)
			{
				// Client joins server, client gets...
				// v Client \ Server > | Doom | Doom II | Free1 | Free2 |
				// --------------------+------+---------+-------+-------+
				// Doom                | PLAY | KICK    | PLAY* | KICK  |
				// Doom II             | KICK | PLAY    | KICK  | PLAY* |
				// FreeDoom (Doom)     | PLAY*| KICK    | PLAY  | KICK  |
				// FreeDoom (Doom II)  | KICK | PLAY*   | KICK  | PLAY  |
				// * = Not playing IWAD Level
				
				// You AWLAYS need an IWAD (even if it is just shareware or FreeDoom)
				if (!FirstCWAD->WAD)
				{
					// Current IWAD selected
					WAD = WL_IterateVWAD(NULL, true);
					
					// Get Info
					if (!(Info = D_GetThisIWAD()))
					{
						SN_Disconnect(false, "No information on current IWAD being used.");
						return;
					}
					
					// Are we FreeDooming?
					i = 0;
					if (Info->Flags & CIF_FREEDOOM)
						i = 1;
					
					// Mode mismatch (Doom vs Doom 2 vs Heretic vs Hexen vs ...)
					if (Info->mission != l_IWADMission)
					{
						SN_Disconnect(false, "Playing different game (e.g. Doom vs Doom 2).");
						return;
					}
					
					// Server is FreeDooming
					if (l_IsFreeDoom)
					{
						// If server is on IWAD level and we are not FreeDooming
						if (l_IsIWADMap && !i)
						{
							SN_Disconnect(false, "Incompatible IWAD level.");
							return;
						}
						
						// Server is on some PWAD, so that is OK
						else
						{
							FirstCWAD->WAD = WAD;
							j--;
						}
					}
					
					// Server is not FreeDooming
					else
					{
						// If server is on an IWAD level, just die
						if (l_IsIWADMap)
						{
							SN_Disconnect(false, "Incompatible IWAD level.");
							return;
						}
						
						// Otherwise, use equiv. FreeDoom WAD (or Plutonia/TNT??)
						else
						{
							FirstCWAD->WAD = WAD;
							j--;
						}
					}
				}
				
				// Go to download stage (if still unresolved)
				if (j > 0)
					l_Stage = DCS_DOWNLOADWADS;
			}
			
			// No WADs needed
			else
				l_Stage = DCS_SWITCHWADS;
			break;
			
			// Download WADs
		case DCS_DOWNLOADWADS:
			SN_Disconnect(false, "Downloading WADs not implemented");
			break;
		
		case DCS_SWITCHWADS:
			// Do not need to redo
			j = 0;
			
			// Find first WAD
			FirstCWAD = NULL;
			for (CWAD = l_WADTail; CWAD; CWAD = CWAD->Prev)
			{
				// First?
				if (!CWAD->Prev && !FirstCWAD)
					FirstCWAD = CWAD;
			}
			
			// Go through WADs owned by server and see if they differ.
				// If there is any difference, then WADs need to be cycled
			for (WAD = NULL, i = 0, CWAD = FirstCWAD; CWAD; CWAD = CWAD->Next, i++)
			{
				// Get current WAD
				WAD = WL_IterateVWAD(WAD, true);
				
				// Link for ReMooD.WAD
				if (!i)
					ExtraWAD = WL_IterateVWAD(WAD, true);
				else
					ExtraWAD = WAD;
				
				// WAD is not matched
				if (CWAD->WAD != WAD || (!CWAD->Next != !WL_IterateVWAD(ExtraWAD, true)))
				{
					j = 1;
					break;
				}
				
				// Skip ReMooD.WAD
				if (!i)
					WAD = WL_IterateVWAD(WAD, true);
			}
			
			// Need to redo
			if (j)
			{
				// Lock OCCB
				WL_LockOCCB(true);
			
				// Remember ReMooD.WAD
				WAD = WL_IterateVWAD(WL_IterateVWAD(NULL, true), true);
			
				// Pop all WADs
				while (WL_PopWAD())
					;
				
				// Go through CWADs and repop
				for (i = 0, CWAD = FirstCWAD; CWAD; CWAD = CWAD->Next, i++)
				{
					WL_PushWAD(CWAD->WAD);
					
					// Push ReMooD.WAD back on the stack
					if (!i)
						WL_PushWAD(WAD);
				}
				
				// Unlock OCCB
				WL_LockOCCB(false);
			}
		
			// Go to next stage
			l_Stage++;
			break;
			
			// Request save game
		case DCS_REQUESTSAVE:
			if (g_ProgramTic < SaveTimeout)
				return;
			
			SaveTimeout = g_ProgramTic + TICRATE;
			D_BSBaseBlock(a_BS, "SAVE");
			D_BSRecordNetBlock(a_BS, &l_HostAddr);
			break;
		
			// Get savegame
		case DCS_GETSAVE:
			break;
			
			// Can play now
		case DCS_CANPLAYNOW:
			if (g_ProgramTic < PlayTimeout)
				return;
			
			PlayTimeout = g_ProgramTic + TICRATE;
			D_BSBaseBlock(a_BS, "PLAY");
			D_BSRecordNetBlock(a_BS, &l_HostAddr);
			break;
			
			// Now playing game
		case DCS_PLAYING:
			break;
			
			// Unknown Stage
		default:
			return;
	}
}

/*****************************************************************************/

/* DT_CONN() -- Connection request */
void DT_CONN(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint8_t IsServer, *Wp;
	SN_Host_t* New;
	
	/* If not allocated, build host for it */
	if (!a_Host)
	{
		// Read data bits
		IsServer = D_BSru8(a_BS);
		
		// Same side?
		if (!!IsServer == SN_IsServer())
			return;
		
		// Put trying to connect...
		I_NetHostToString(a_Addr, Buf, BUFSIZE);
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_CLCONNECT, "%s\n", Buf);
		
		// Create a host for them
		New = SN_CreateHost();
		
		// Create unique ID
		do
		{
			New->ID = D_CMakePureRandom();
		} while (!New->ID || SN_HostByID(New->ID) != New);
		
		// Fill info
		memmove(&New->Addr, a_Addr, sizeof(*a_Addr));
		New->BS = a_BS;
		
		// Create packet
		if (SN_ExtCmdInGlobal(DTCT_SNJOINHOST, &Wp))
		{
			LittleWriteUInt32((uint32_t**)&Wp, New->ID);
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			WriteUInt8(&Wp, 0);
		}
	}
	
	// Use host here
	else
		New = a_Host;
	
	/* Reply with host info */
	D_BSBaseBlock(a_BS, "HELO");
	
	// Write ID
	D_BSwu32(a_BS, New->ID);
	
	D_BSRecordNetBlock(a_BS, a_Addr);
#undef BUFSIZE
}

/* DT_HELO() -- Connection granted */
void DT_HELO(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	SN_Host_t* New;
	
	/* Ignore if already connected */
	if (SN_IsConnected())
		return;
	
	/* Set connected */
	SN_SetConnected(true);
	l_SvLastPing = g_ProgramTic;	// So there is no partial disconn
	
	/* Create host */
	New = SN_CreateHost();
	New->ID = D_BSru32(a_BS);
	New->Local = true;
	SN_SetMyHost(New);
	l_Stage = 0;
}

/* DT_LIST() -- List WADS */
void DT_LIST(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	const WL_WADFile_t* Rover, *First;
	int32_t i;
	D_IWADInfoEx_t* Info;
	
	/* Reply with wad list */
	D_BSBaseBlock(a_BS, "WADL");
	
	// If playing a level and that level is an IWAD map
		// This is for FreeDoom to prevent joining when the IWAD mismatches.
	First = WL_IterateVWAD(NULL, true);
	if (g_CurrentLevelInfo && g_CurrentLevelInfo->WAD == First)
		D_BSwu8(a_BS, 1);
	else
		D_BSwu8(a_BS, 0);
	
	// Doing a FreeDoom game (allow FreeDoomers with differing FreeDoom.WADs)
	Info = D_GetThisIWAD();
	if (Info && (Info->Flags & CIF_FREEDOOM))
		D_BSwu8(a_BS, 1);
	else
		D_BSwu8(a_BS, 0);
	
	// Mission of IWAD
	if (Info)
		D_BSwu8(a_BS, Info->mission);
	else
		D_BSwu8(a_BS, 0);
	
	// Send WAD Order
	for (Rover = First, i = 0; Rover; Rover = WL_IterateVWAD(Rover, true), i++)
	{
		// Ignore ReMooD.WAD
		if (i == 1)
			continue;
		
		// Names and MD5
		D_BSws(a_BS, WL_GetWADName(Rover, false));
		D_BSws(a_BS, WL_GetWADName(Rover, true));
		D_BSws(a_BS, Rover->CheckSumChars);
	}
	
	// NUL for end
	D_BSwu32(a_BS, 0);
	
	/* Send */
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_WADL() -- WAD List */
void DT_WADL(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_WADChain_t* Rover;
	D_WADChain_t Temp;
	const WL_WADFile_t* WAD;
	
	/* Only on first stage */
	if (l_Stage != DCS_LISTWADS || SN_IsServer())
		return;
	
	/* Store WADs from server */
	// Clear tail
	for (Rover = l_WADTail; Rover; Rover = l_WADTail)
	{
		// Set previous
		l_WADTail = Rover->Prev;
		
		// Free current
		Z_Free(Rover);
	}
	
	l_WADTail = NULL;
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADHEADER, "\n");
	
	// Is this an IWAD map? What about FreeDoom?
	l_IsIWADMap = D_BSru8(a_BS);
	l_IsFreeDoom = D_BSru8(a_BS);
	l_IWADMission = D_BSru8(a_BS);
	
	// Read in contents
	for (;;)
	{
		// Clear
		memset(&Temp, 0, sizeof(Temp));
		
		// DOS, Full, Sum
		D_BSrs(a_BS, Temp.DOS, CHAINSIZE);
		D_BSrs(a_BS, Temp.Full, CHAINSIZE);
		D_BSrs(a_BS, Temp.Sum, CHAINSIZE);
		
		// End
		if (!Temp.DOS[0])
			break;
		
		// Print
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADENTRY, "%s%s%s\n", Temp.DOS, Temp.Full, Temp.Sum);
		
		// Try to find WAD by sum (not by name, in case of renames??)
		for (WAD = WL_IterateVWAD(NULL, true); WAD; WAD = WL_IterateVWAD(WAD, true))
			if (!strcasecmp(Temp.Sum, WAD->CheckSumChars))
			{
				Temp.WAD = WAD;
				break;
			}
		
		// Allocate
		Rover = Z_Malloc(sizeof(*Rover), PU_STATIC, NULL);
		memmove(Rover, &Temp, sizeof(*Rover));
		
		// Chain in
		if (l_WADTail)
			l_WADTail->Next = Rover;
		Rover->Prev = l_WADTail;
		l_WADTail = Rover;
	}
	
	/* Increase Stage */
	l_Stage++;
}

/* DT_QUIT() -- Quit */
void DT_QUIT(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Read reason */
	D_BSrs(a_BS, Buf, BUFSIZE);
	
	/* If not connected, disconnect */
	// Could be in the middle of a connect request, or not playing yet
	if (!SN_IsConnected() || (!SN_IsServer() && l_Stage < DCS_PLAYING))
	{
		SN_Disconnect(false, Buf);
		return;
	}
	
	/* If we are the server, and this host is local do not disconnect */
	else if (SN_IsServer() && a_Host && a_Host->Local)
		return;
	
	/* Cleanup remote client if non-local (they quit) */
	else if (a_Host && !a_Host->Local)
		SN_DisconnectHost(a_Host, Buf);
	
	/* Otherwise, perform a partial disconnect */
	// As long as we are not the server!
	else if (!SN_IsServer())
		SN_PartialDisconnect(Buf);
#undef BUFSIZE
}

/* DT_SAVE() -- Requests save game */
void DT_SAVE(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Check */
	if (!SN_IsServer() || !a_Host || a_Host->Local)
		return;
	
	/* If already has save, ignore */
	if (a_Host->Save.Has)
		return;
	
	/* If save not wanted, give them it */
	if (!a_Host->Save.Want)
	{
		// Now wants
		a_Host->Save.Want = true;
		a_Host->Save.TicTime = gametic + (TICRATE - (gametic % TICRATE));
	}
	
	/* Inform them */
	D_BSBaseBlock(a_BS, "PSAV");
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_PSAV() -- Requests save game */
void DT_PSAV(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Client only */
	if (SN_IsServer() || l_Stage != DCS_REQUESTSAVE)
		return;
	
	/* Go to save OK */
	l_Stage = DCS_GETSAVE;
}

/* DT_PLAY() -- Client can play now! */
void DT_PLAY(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Server only */
	if (!SN_IsServer() || !a_Host || a_Host->Local)
		return;
	
	/* Mark them as in the game */
	a_Host->Save.Has = true;
	
	/* Inform them */
	D_BSBaseBlock(a_BS, "COOL");
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_COOL() -- Our play request was accepted! */
void DT_COOL(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Client Only */
	if (SN_IsServer() || l_Stage != DCS_CANPLAYNOW)
		return;
	
	/* Start playing */
	l_Stage = DCS_PLAYING;
}

/* DT_JOBT() -- Received a job tic */
void DT_JOBT(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	tic_t GameTic;
	uint32_t Size;
	SN_TicBuf_t* TicBuf;
	
	static uint8_t* Buf;
	static uint32_t BufSize;
	
	/* Client Only that is connected */
	// Accept can play now in case of some packets being missed.
	if (SN_IsServer() || !(l_Stage == DCS_CANPLAYNOW || l_Stage == DCS_PLAYING))
		return;
	
	/* Read packet data */
	GameTic = D_BSrcu64(a_BS);
	Size = D_BSru32(a_BS);
	
	// Size limitation
	if (Size > 32767)
		Size = 32767;
	
	// Allocate buffer, if needed
	if (!Buf || Size > BufSize)
	{
		// Clear old
		if (Buf)
		{
			Z_Free(Buf);
			Buf = NULL;
		}
		
		// Make new
		Buf = Z_Malloc(Size, PU_STATIC, NULL);
		BufSize = Size;
	}
	
	// Find tic buffer for this tic
	TicBuf = SN_BufForGameTic(GameTic);
	
	// Oops!
	if (!TicBuf)
		return;
	
	// Never got tic
	if (!TicBuf->GotTic)
	{
		// Read packet into buffer (this is here to reduce CPU cost)
		memset(Buf, 0, BufSize);
		D_BSReadChunk(a_BS, Buf, Size);
		
		// Decode
		if (!SN_DecodeTicBuf(TicBuf, Buf, Size))
		{
			SN_PartialDisconnect("Tic decode error");
			return;
		}
		
		// Got tic now yay!
		TicBuf->GotTic = true;
		
		// Set the last ping time, to prevent time out
		l_SvLastPing = g_ProgramTic;
	}
	
	/* Reply to server saying, the tic was recieved and buffered */
	D_BSBaseBlock(a_BS, "JOBA");
	
	D_BSwcu64(a_BS, GameTic);
	D_BSwcu64(a_BS, gametic);
	
	// Append local port tics to the server
	SN_AppendLocalCmds(a_BS);
	
	// Send
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_JOBA() -- Acknowledged job tic */
void DT_JOBA(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	int32_t i, j;
	D_XMitJob_t* Job;
	tic_t GameTic, ClientGT;
	
	static uint8_t* Buf;
	static uint32_t BufSize;
	uint32_t Size; 
	SN_TicBuf_t TicBuf;
	ticcmd_t* Cmd;
	SN_Port_t* Port;
	
	/* Server Only */
	if (!SN_IsServer() || !a_Host)
		return;
	
	/* Get gametic */
	GameTic = D_BSrcu64(a_BS);
	ClientGT = D_BSrcu64(a_BS);
	
	/* Set host to this minimum tic */
	if (ClientGT > a_Host->MinTic)
		a_Host->MinTic = ClientGT;
	
	/* Find job and verify */
	for (i = 0; i < l_XAt; i++)
		if ((Job = &l_XStack[i]))
		{
			// Wrong gametic
			if (Job->GameTic != GameTic)
				continue;
			
			// Find sub host
			for (j = 0; j < Job->NumDests; j++)
				if (a_Host == Job->Dests[j].Host)
				{
					Job->Dests[j].Clear = true;
					a_Host->LastSeen = g_ProgramTic;
					break;
				}
			
			// If found, would have been cleared
			break;
		}
	
	/* Read client tic commands */
	Size = D_BSru32(a_BS);
	
	// Nothing to read
	if (Size < 0)
		return;
	
	// Cap size
	if (Size > 16383)
		Size = 16383;
	
	// Need to resize buffer
	if (BufSize < Size)
	{
		if (Buf)
			Z_Free(Buf);
		BufSize = Size;
		Buf = Z_Malloc(BufSize, PU_STATIC, NULL);
	}
	
	// Read
	D_BSReadChunk(a_BS, Buf, Size);
	
	// Decode commands
	if (!SN_DecodeTicBuf(&TicBuf, Buf, Size))
		return;	// Who cares if it is bad
	
	// reverse the pig mask
	TicBuf.PIGRevMask = ~TicBuf.PIGRevMask;
	
	/* Copy decoded commands to player ports */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// No commands for this player
		if (!(TicBuf.PIGRevMask & (1 << i)))
			continue;
		
		// Find port for this player
		for (j = 0; j < a_Host->NumPorts; j++)
			if ((Port = a_Host->Ports[j]))
			{
				// Not playing or wrong player
				if (!Port->Player || Port->Player - players != i)
					continue;
				
				// Append command to this location
				if (Port->LocalAt < MAXLBTSIZE - 1)
					Cmd = &Port->LocalBuf[Port->LocalAt++];
				else
					Cmd = &Port->LocalBuf[0];
				
				// Copy
				memmove(Cmd, &TicBuf.Tics[i], sizeof(*Cmd));
			}
	}
}

/* DT_WANT() -- Client wants another port */
void DT_WANT(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	int32_t i, Total;
	SN_Port_t* Port, *PIDMatch;
	uint32_t ProcessID, ID;
	tic_t GameTic;
	uint8_t* Wp;
	
	/* Server Only */
	if (!SN_IsServer() || !a_Host)
		return;
	
	/* Read Packet */
	ProcessID = D_BSru32(a_BS);
	GameTic = D_BSrcu64(a_BS);
	
	/* Count the number of ports in this host */
	Total = 0;
	PIDMatch = NULL;
	for (i = 0; i < a_Host->NumPorts; i++)
		if ((Port = a_Host->Ports[i]))
		{
			// If process ID matches, remember that
			if (ProcessID && ProcessID == Port->ProcessID)	
				PIDMatch = Port;
			
			// Increase total
			Total++;
		}
	
	/* No match and total exceeds count */
	// They can only have 4 players connected at once
	if (!PIDMatch && Total >= MAXSPLITS)
	{
		D_BSBaseBlock(a_BS, "FULL");
		
		D_BSwu8(a_BS, Total);
		D_BSwu32(a_BS, ProcessID);
		D_BSwcu64(a_BS, GameTic);
		
		D_BSRecordNetBlock(a_BS, a_Addr);
		return;
	}
	
	/* Otherwise, use said (new) port */
	if (!PIDMatch)
	{
		// Create a new port for them
		PIDMatch = SN_AddPort(a_Host);
		
		// Setup local port info
		if (ProcessID)
			PIDMatch->ProcessID = ProcessID;
		
		// Need a unique process ID
		else
		{
			// Pure random may be random enough to prevent mishaps
			do
			{
				ProcessID = D_CMakePureRandom();
			} while (!ProcessID);
		}
		
		// Generate unique port ID
		do
		{
			ID = D_CMakePureRandom();
		} while (!ID || SN_PortByID(ID) || SN_HostByID(ID));
		PIDMatch->ID = ID;
		
		// Create packet
		if (SN_ExtCmdInGlobal(DTCT_SNJOINPORT, &Wp))
		{
			LittleWriteUInt32((uint32_t**)&Wp, a_Host->ID);
			LittleWriteUInt32((uint32_t**)&Wp, PIDMatch->ID);
			WriteUInt8(&Wp, 0);
			LittleWriteUInt32((uint32_t**)&Wp, PIDMatch->ProcessID);
		}
	}
	
	/* Send a basic packet */
	D_BSBaseBlock(a_BS, "GIVE");
		
	D_BSwu8(a_BS, Total);
	D_BSwu32(a_BS, PIDMatch->Host->ID);
	D_BSwu32(a_BS, PIDMatch->ID);
	D_BSwu32(a_BS, PIDMatch->ProcessID);
	D_BSwcu64(a_BS, GameTic);
	
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_GIVE() -- Client wants another port */
void DT_GIVE(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	uint8_t Total;
	uint32_t ID, HostID, ProcessID;
	tic_t GameTic;	
	SN_Host_t* Host, *MyHost;
	SN_Port_t* Port;
	
	/* Client Only */
	if (SN_IsServer() || !(l_Stage == DCS_CANPLAYNOW || l_Stage == DCS_PLAYING))
		return;
	
	/* Read Packet */
	Total = D_BSru8(a_BS);
	HostID = D_BSru32(a_BS);
	ID = D_BSru32(a_BS);
	ProcessID = D_BSru32(a_BS);
	GameTic = D_BSrcu64(a_BS);
	
	/* Check to see if it exists already */
	if (SN_PortByID(ID))
		return;
	
	/* Get current host */
	Host = SN_HostByID(HostID);
	MyHost = SN_MyHost();
	
	// If this is not our host, ignore
	if (!Host || Host != MyHost)
		return;
	
	/* Create a new port */
	Port = SN_AddPort(MyHost);
	
	// Set local port fields
	Port->ID = ID;
	Port->ProcessID = ProcessID;
}

/* DT_PING() -- Ping request */
void DT_PING(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	uint8_t At;
	uint32_t Code;
	
	/* Read data */
	At = D_BSru8(a_BS);
	Code = D_BSru32(a_BS);
	
	/* Reply with pong */
	D_BSBaseBlock(a_BS, "PONG");
	
	D_BSwu8(a_BS, At);
	D_BSwu32(a_BS, Code);
	D_BSwcu64(a_BS, g_ProgramTic);
	
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_PONG() -- Ping reply */
void DT_PONG(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	uint8_t At;
	uint32_t Code;
	SN_PingWin_t* Win;
	tic_t TimeDiff;
	int32_t MilliDiff;
	
	/* Read */
	At = D_BSru8(a_BS);
	Code = D_BSru32(a_BS);
	
	// Illegal at? Or code is zero
	if (!Code || At < 0 || At >= MAXPINGWINDOWS)
		return;
	
	/* Determine the window to use */
	Win = NULL;
	
	// Client
	if (!SN_IsServer())
		Win = &l_SvPings[At];
	
	// Server
	else if (a_Host)
		Win = &a_Host->Pings[At];
	
	// Unknown (die)
	else
		return;
	
	/* Make sure the code matches */
	if (!Win->Code || Win->Code != Code)
		return;
	
	/* Calculate time difference */
	TimeDiff = g_ProgramTic - Win->SendTime;
	MilliDiff = I_GetTimeMS() - Win->Millis;
	
	/* Set timers (average) */
	// Client
	if (!SN_IsServer())
	{
		l_SvPing = (l_SvPing + MilliDiff) >> 1;
		l_SvLastPing = g_ProgramTic;
	}
	
	// Server
	else if (a_Host)
	{
		a_Host->Ping = (a_Host->Ping + MilliDiff) >> 1;
		a_Host->LastPing = g_ProgramTic;
	}
	
	/* Clear window, so it cannot be used again */
	// This prevents ping reply cheating to hold a game in WFP forever.
	memset(Win, 0, sizeof(*Win));
}

/* DT_PJGG() -- Port join request */
void DT_PJGG(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	uint32_t ID;
	SN_Port_t* Port;
	
	/* Server Only */
	if (!a_Host || !SN_IsServer())
		return;
	
	/* Get Port */
	ID = D_BSru32(a_BS);
	Port = SN_PortByID(ID);
	
	// No port?
	if (!Port)
		return;
	
	// Host does not own this port (cannot join other ports)
	if (Port->Host != a_Host)
		return;
	
	/* Mark them to join */
	Port->WillJoin = true;
}

/* DT_SYNC() -- Client Game Synchronization Code */
void DT_SYNC(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	tic_t GameTic;
	uint32_t Code;	
	
	/* Check */
	if (!a_Host || !SN_IsServer())
		return;
	
	/* Read Data */
	GameTic = D_BSrcu64(a_BS);
	Code = D_BSru32(a_BS);
	
	/* Handle sync code from host */
	SN_CheckSyncCode(a_Host, GameTic, Code);
}

/* DT_CHAT() -- Client Chats */
void DT_CHAT(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 128
	uint32_t Source, Target;
	uint8_t Mode;	
	SN_Port_t* MyPort, *TargetPort;
	char Buf[BUFSIZE];
	int32_t Len;
	uint32_t ChatID;
	
	/* Check */
	if (!a_Host)
		return;
	
	/* Read Info */
	Source = D_BSru32(a_BS);
	Mode = D_BSru8(a_BS);
	Target = D_BSru32(a_BS);
	ChatID = D_BSru32(a_BS);
	
	memset(Buf, 0, sizeof(Buf));
	D_BSrs(a_BS, Buf, BUFSIZE);
	
	// Get source player
	MyPort = SN_PortByID(Source);
	TargetPort = SN_PortByID(Target);
	
	// Wrong host? indiv and no target?
	if (MyPort->Host != a_Host || (Mode == 3 && !TargetPort))
		return;
	
	// Already sent this or older message?
	if (ChatID <= MyPort->ChatID)
		return;
	
	// Length of current message
	Len = strlen(Buf);
	
	// Needs cooling down?
	if (g_ProgramTic + Len < MyPort->ChatCoolDown)
		return;
	
	// Set cooldown to length of string
	MyPort->ChatCoolDown = g_ProgramTic + Len;
	MyPort->ChatID = ChatID;	// to prevent same message spam due to lag
	
	/* Direct encode */
	SN_DirectChat(a_Host->ID, Source, Mode, Target, Buf);
#undef BUFSIZE
}

/* DT_SETT() -- Change Setting */
void DT_SETT(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	uint32_t HostID, PortID;
	uint16_t Setting;
	uint8_t Player;	
	SN_Port_t* Port;
	uint8_t Data[MAXTCSTRINGCAT];
	
	/* Check */
	if (!a_Host || !SN_IsServer())
		return;
	
	/* Read Data */
	HostID = D_BSru32(a_BS);
	PortID = D_BSru32(a_BS);
	Player = D_BSru8(a_BS);
	Setting = D_BSru16(a_BS);
	
	// Find port
	Port = SN_PortByID(PortID);
	
	// Invalid mismatch
	if (a_Host->ID != HostID || !Port || Port->Host != a_Host)
		return;
	
	// Invalid player
	if (Player != 255 && Port->Player != &players[Player])
		return;
	
	// Read giant data
	D_BSReadChunk(a_BS, Data, MAXTCSTRINGCAT);
	
	/* Send to setting writer */
	SN_PortSetting(Port, Setting, 0, Data, MAXTCSTRINGCAT);
}

/* DT_RSPP() -- Request Spectation of player */
void DT_RSPP(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	int32_t PlayerID, i;
	SN_Port_t* Port;
	
	/* Check */
	if (!SN_IsServer() || !a_Host)
		return;
	
	/* Read player ID */
	PlayerID = D_BSri32(a_BS);
	
	// Check bounds
	if (PlayerID < 0 || PlayerID >= MAXPLAYERS)
		return;
	
	/* Already not playing? */
	if (!playeringame[PlayerID])
		return;
	
	/* Security check, be sure host OWNS this player */
	for (i = 0; i < a_Host->NumPorts; i++)
		if ((Port = a_Host->Ports[i]))
			if (Port->Player && PlayerID == (Port->Player - players))
			{
				SN_RemovePlayer(PlayerID);
				break;
			}
}	

/* l_Packets -- Data packets */
static const struct
{
	union
	{
		uint8_t Char[4];
		uint32_t Int;
	} H;
	void (*Func)(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr);
	bool_t RemoteOnly;							// Only from remote end
} l_Packets[] =
{
	{{"PING"}, DT_PING, false},
	{{"JOBT"}, DT_JOBT, false},
	{{"PONG"}, DT_PONG, false},
	{{"JOBA"}, DT_JOBA, false},
	{{"SYNC"}, DT_SYNC, false},
	{{"CONN"}, DT_CONN, false},
	{{"HELO"}, DT_HELO, true},
	{{"LIST"}, DT_LIST, false},
	{{"WADL"}, DT_WADL, true},
	{{"QUIT"}, DT_QUIT, false},
	{{"SAVE"}, DT_SAVE, false},
	{{"PSAV"}, DT_PSAV, false},
	{{"PLAY"}, DT_PLAY, false},
	{{"COOL"}, DT_COOL, false},
	{{"WANT"}, DT_WANT, false},
	{{"GIVE"}, DT_GIVE, false},
	//{{"FULL"}, DT_FULL, false},	// TODO FIXME
	{{"PJGG"}, DT_PJGG, false},
	{{"CHAT"}, DT_CHAT, false},
	{{"SETT"}, DT_SETT, false},
	{{"RSPP"}, DT_RSPP, false},
	
	{{"FPUT"}, SN_FileRecv, false},
	{{"FOPN"}, SN_FileInit, false},
	{{"FRDY"}, SN_FileReady, false},
	
	{{{0}}}
};

/* SN_DoTrans() -- Do transmission */
void SN_DoTrans(void)
{
	union
	{
		char Char[4];
		uint32_t Int;
	} Header;
	bool_t Continue;
	I_HostAddress_t Addr;
	SN_Host_t* Host;
	int32_t i;
	
	/* No Socket */
	if (!l_Sock)
		return;
	
	/* Perform packet updates */
	for (Continue = true; Continue;)
	{
		// Clear
		memset(&Header, 0, sizeof(Header));
		memset(&Addr, 0, sizeof(Addr));
		
		// Play block
		if ((Continue = D_BSPlayNetBlock(l_BS, Header.Char, &Addr)))
		{
			// Find host, if any
			Host = SN_HostByAddr(&Addr);
			
			// Which type?
			for (i = 0; l_Packets[i].H.Char[0]; i++)
				if (l_Packets[i].H.Int == Header.Int)
				{
					// Needs to be from remote
					if (l_Packets[i].RemoteOnly)
						if (!I_NetCompareHost(&Addr, &l_HostAddr))
							break;
					
					l_Packets[i].Func(l_BS, Host, &Addr);
					break;
				}
		}
	}
	
	/* If not connected, connect to remote side */
	if (!SN_IsConnected())
	{
		SN_DoConnect();
		return;
	}
	
	/* Files */
	SN_FileLoop();
	
	/* Do client or server stuff */
	if (SN_IsServer())
		SN_DoServer(l_BS);
	else
		SN_DoClient(l_BS);
}

/* SN_GotFile() -- Received file */
bool_t SN_GotFile(const char* const a_PathName)
{
	char* Ext;	
	
	/* Check */
	if (!a_PathName)
		return false;
	
	/* Grab extension */
	Ext = strrchr(a_PathName, '.');
	
	/* Save game */
	if (!strcasecmp(Ext, ".rsv"))
	{
		// Load save game
		if (!P_LoadGameEx(NULL, a_PathName, strlen(a_PathName), NULL, NULL))
			SN_Disconnect(false, "Failed to load save game");
		
		// Tell server, we are playing
		else
			l_Stage = DCS_CANPLAYNOW;
		
		// Delete save games after they are used
		return true;
	}
	
	/* Assume WAD content data thing */
	else
	{
		// Do not delete WADs!
		return false;
	}
}

