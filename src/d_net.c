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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
#include "p_demcmp.h"
#include "d_main.h"
#include "m_menu.h"
#include "r_main.h"
#include "doomstat.h"
#include "p_setup.h"
#include "b_bot.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_saveg.h"
#include "d_xpro.h"
#include "p_inter.h"

/*************
*** LOCALS ***
*************/

static tic_t l_MapTime = 0;						// Map local time
static tic_t l_BaseTime = 0;					// Base Game Time
static tic_t l_LocalTime = 0;					// Local Time
static bool_t l_ConsistencyFailed = false;		// Consistency failed
static bool_t l_SoloNet = true;					// Solo Network
static bool_t l_IsConnected = false;			// Connected to server
static bool_t l_IsDedicated = false;			// Dedicated Server

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/

/* D_NetSetPlayerName() -- Sets name of player */
bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name)
{
	uint32_t OldNameHash;
	uint32_t NewNameHash;
	char OldName[MAXPLAYERNAME + 1];
	
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS || !a_Name)
		return false;
	
	/* Hash old name */
	memset(OldName, 0, sizeof(OldName));
	strncpy(OldName, player_names[a_PlayerID], MAXPLAYERNAME);
	OldNameHash = Z_Hash(player_names[a_PlayerID]);
	
	/* Copy name over */
	strncpy(player_names[a_PlayerID], a_Name, MAXPLAYERNAME);
	player_names[a_PlayerID][MAXPLAYERNAME - 1] = 0;
	NewNameHash = Z_Hash(player_names[a_PlayerID]);
	
	/* Inform? */
	if (OldNameHash != NewNameHash)
		CONL_OutputU(DSTR_NETPLAYERRENAMED, "%s%s\n", OldName, player_names[a_PlayerID]);
	
	/* Success! */
	return true;
}

/* D_NetPlayerChangedPause() -- Player changed the pause state */
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID)
{
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return false;
	
	/* Paused or not paused? */
	CONL_OutputU((paused ? DSTR_GAMEPAUSED : DSTR_GAMEUNPAUSED), "%s\n", player_names[a_PlayerID]);
	
	/* Success! */
	return true;
}

/*****************************************************************************/

/*** STRUCTURES ***/

/*** GLOBALS ***/

uint32_t g_NetStat[4] = {0, 0, 0, 0};			// Network stats
tic_t g_LastServerTic = 0;						// Server's Last tic
extern int32_t g_IgnoreWipeTics;				// Demo playback, ignore this many wipe tics

/*** LOCALS ***/

// sv_name -- Name of Server
CONL_StaticVar_t l_SVName =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_name", DSTR_CVHINT_SVNAME, CLVVT_STRING, "Untitled ReMooD Server",
	NULL
};

// sv_email -- Administrator E-Mail of Server
CONL_StaticVar_t l_SVEMail =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_email", DSTR_CVHINT_SVEMAIL, CLVVT_STRING, "nobody@localhost",
	NULL
};

// sv_url -- Administrator URL of Server
CONL_StaticVar_t l_SVURL =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_url", DSTR_CVHINT_SVURL, CLVVT_STRING, "http://remood.org/",
	NULL
};

// sv_wadurl -- Where WADs should be downloaded from
CONL_StaticVar_t l_SVWADURL =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_wadurl", DSTR_CVHINT_SVURL, CLVVT_STRING, "http://remood.org/wads/",
	NULL
};

// sv_irc -- Administrator IRC Channel of Server
CONL_StaticVar_t l_SVIRC =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_irc", DSTR_CVHINT_SVIRC, CLVVT_STRING, "irc://irc.oftc.net/remood",
	NULL
};

// sv_motd -- Message Of The Day
CONL_StaticVar_t l_SVMOTD =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_motd", DSTR_CVHINT_SVMOTD, CLVVT_STRING, "Welcome to ReMooD! Enjoy your stay!",
	NULL
};

// sv_connectpassword -- Password needed to connect
CONL_StaticVar_t l_SVConnectPassword =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_connectpassword", DSTR_CVHINT_SVCONNECTPASSWORD, CLVVT_STRING, "",
	NULL
};

// sv_joinpassword -- Password needed to join
CONL_StaticVar_t l_SVJoinPassword =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_joinpassword", DSTR_CVHINT_SVJOINPASSWORD, CLVVT_STRING, "",
	NULL
};

// sv_maxclients -- Name of Server
CONL_StaticVar_t l_SVMaxClients =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxclients", DSTR_CVHINT_SVMAXCLIENTS, CLVVT_INTEGER, "48",
	NULL
};

// sv_joinwindow -- Join Window Interval
CONL_StaticVar_t l_SVJoinWindow =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_joinwindow", DSTR_CVHINT_SVJOINWINDOW, CLVVT_INTEGER, "10",
	NULL
};

// sv_lagstat -- Time in minutes before re-stat for lag
CONL_StaticVar_t l_SVLagStat =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_lagstat", DSTR_CVHINT_SVLAGSTAT, CLVVT_INTEGER, "5",
	NULL
};

// sv_readyby -- Time to be catchup from lag
CONL_StaticVar_t l_SVReadyBy =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_readyby", DSTR_CVHINT_SVREADYBY, CLVVT_INTEGER, "30",
	NULL
};

// sv_lagthreshexpire -- When the lag threshold expires
CONL_StaticVar_t l_SVLagThreshExpire =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_lagthreshexpire", DSTR_CVHINT_SVLAGTHRESHEXPIRE, CLVVT_INTEGER, "60",
	NULL
};

// sv_maxcatchup -- Limit to the amount of time the server can catchup
CONL_StaticVar_t l_SVMaxCatchup =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxcatchup", DSTR_CVHINT_SVMAXCATCHUP, CLVVT_INTEGER, "105",
	NULL
};

// sv_maxdemocatchup -- Similar to sv_maxcatchup, but for demos
CONL_StaticVar_t l_SVMaxDemoCatchup =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxdemocatchup", DSTR_CVHINT_SVMAXDEMOCATCHUP, CLVVT_INTEGER, "52",
	NULL
};

// cl_maxptries -- Maximum tries to obtain an XPlayer before failing
CONL_StaticVar_t l_CLMaxPTries =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"cl_maxptries", DSTR_CVHINT_CLMAXPTRIES, CLVVT_INTEGER, "5",
	NULL
};

// cl_maxptrytime -- Delay in seconds to retry
CONL_StaticVar_t l_CLMaxPTryTime =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"cl_maxptrytime", DSTR_CVHINT_CLMAXPTRYTIME, CLVVT_INTEGER, "3",
	NULL
};

// cl_reqticdelay -- When the lag threshold expires
CONL_StaticVar_t l_CLReqTicDelay =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"cl_reqticdelay", DSTR_CVHINT_CLREQTICDELAY, CLVVT_INTEGER, "7",
	NULL
};

extern CONL_StaticVar_t l_CONPauseGame;

/*** FUNCTIONS ***/

/* D_CheckNetGame() -- Checks whether the game was started on the network */
bool_t D_CheckNetGame(void)
{
	I_NetSocket_t* Socket;
	bool_t ret = false;
	uint16_t i, v, PortNum, ShowPort;
	
	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	
	multiplayer = false;
	
	// only dos version with external driver will return true
	netgame = false;
	if (netgame)
		netgame = false;
	
	/* Register server commands */
	// Control
	//CONL_AddCommand("startserver", DS_NetMultiComm);
	//CONL_AddCommand("connect", DS_NetMultiComm);
	//CONL_AddCommand("disconnect", DS_NetMultiComm);
	//CONL_AddCommand("reconnect", DS_NetMultiComm);
	
	//CONL_AddCommand("listclients", DS_NetMultiComm);
	
	/* Register variables */
	// Server
	CONL_VarRegister(&l_SVName);
	CONL_VarRegister(&l_SVEMail);
	CONL_VarRegister(&l_SVURL);
	CONL_VarRegister(&l_SVWADURL);
	CONL_VarRegister(&l_SVIRC);
	CONL_VarRegister(&l_SVMOTD);
	CONL_VarRegister(&l_SVConnectPassword);
	CONL_VarRegister(&l_SVJoinPassword);
	CONL_VarRegister(&l_SVMaxClients);
	CONL_VarRegister(&l_SVJoinWindow);
	CONL_VarRegister(&l_SVLagStat);
	CONL_VarRegister(&l_SVReadyBy);
	CONL_VarRegister(&l_SVLagThreshExpire);
	CONL_VarRegister(&l_SVMaxCatchup);
	CONL_VarRegister(&l_SVMaxDemoCatchup);
	
	// Client
	CONL_VarRegister(&l_CLMaxPTries);
	CONL_VarRegister(&l_CLMaxPTryTime);
	CONL_VarRegister(&l_CLReqTicDelay);
	
	/* Debug? */
	if (M_CheckParm("-netdev") || M_CheckParm("-devnet"))
		g_NetDev = true;
	
	/* Initial Disconnect */
	D_XNetInit();
	D_XNetDisconnect(false);
	
	return ret;
}

/*****************************************************************************/

/* D_NCLocalPlayerAdd() -- Adds a local player */
void D_NCLocalPlayerAdd(const char* const a_Name, const bool_t a_Bot, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy)
{
	uint32_t ProcessID, PlaceAt, LastScreen;
	D_ProfileEx_t* Profile;
	const B_BotTemplate_t* BotTemplate;
	bool_t BumpSplits;
	
	/* No bots */
	if (a_Bot)
		return;
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return;
	
	/* Find first free slot */
	// Find the last screened player
	for (LastScreen = 0; LastScreen < MAXSPLITSCREEN; LastScreen++)
		if (!D_ScrSplitHasPlayer(LastScreen))
			break;
	
	// Place at the first wanted spot, unless already bound
		// Assign joystick to player, can be in game
	if (a_UseJoy && D_ScrSplitHasPlayer(a_ScreenID) && !g_Splits[a_ScreenID].JoyBound)
		PlaceAt = a_ScreenID;
	
		// No joystick wanted
	else
	{
		// Cannot fit any more players
		if (LastScreen >= MAXSPLITSCREEN)
			return;	
		
		PlaceAt = LastScreen;
	}
	
	// If placement is after the last, cap to last
		// So there is no gap in the screens
	//if (PlaceAt > LastScreen)
	//	PlaceAt = LastScreen;
	
	/* Create Process ID */
	ProcessID = D_XNetMakeID(0);
	
	/* If not a bot, bind to a local screen */
	// Bump splits? Not if a screen has a player (controlling keyboarder)
	BumpSplits = true;
	if (D_ScrSplitHasPlayer(PlaceAt))
		BumpSplits = false;
	
	// Find Profile
	Profile = NULL;
	if (a_Name)
		Profile = D_FindProfileEx(a_Name);
	
	// Never redisplay
		// Also if a player is not active, then reset the display status
	if (!demoplayback)
		if (!g_Splits[PlaceAt].Active)
		{
			g_Splits[PlaceAt].Console = 0;
			g_Splits[PlaceAt].Display = -1;
		}
	
	g_Splits[PlaceAt].RequestSent = false;
	g_Splits[PlaceAt].GiveUpAt = 0;
	
	// Bind stuff here
	g_Splits[PlaceAt].Waiting = true;
	g_Splits[PlaceAt].Profile = Profile;
	g_Splits[PlaceAt].JoyBound = a_UseJoy;
	g_Splits[PlaceAt].JoyID = a_JoyID;
	g_Splits[PlaceAt].ProcessID = ProcessID;
	
	// Resize Splits
	if (BumpSplits)
		if (!demoplayback)
		{
			if (PlaceAt >= g_SplitScreen)
				g_SplitScreen = ((int)PlaceAt);// - 1;
			R_ExecuteSetViewSize();
		}
	
	/* Add resume menu time */
	// i.e. HOM when adding a player in the menu
	g_ResumeMenu++;
}

/*****************************************************************************/

/*** LOCALS ***/

#define MAXGLOBALBUFSIZE					32	// Size of global buffer

static tic_t l_GlobalTime[MAXGLOBALBUFSIZE];	// Time
static ticcmd_t l_GlobalBuf[MAXGLOBALBUFSIZE];	// Global buffer
static int32_t l_GlobalAt = -1;					// Position Global buf is at

/*** PACKET HANDLER FUNCTIONS ***/

/* DS_GrabGlobal() -- Grabs the next global command */
static ticcmd_t* DS_GrabGlobal(const uint8_t a_ID, const int32_t a_NeededSize, void** const a_Wp)
{
	ticcmd_t* Placement;
	void* Wp;
	
	/* Clear */
	Placement = NULL;
	
	/* Determine */
	if (l_GlobalAt < MAXGLOBALBUFSIZE - 1)
	{
		// Append to last global tic command, if possible
		if (l_GlobalAt >= 0)
		{
			if (l_GlobalBuf[l_GlobalAt].Ext.DataSize < MAXTCDATABUF - (a_NeededSize + 2))
				Placement = &l_GlobalBuf[l_GlobalAt];
			else
			{
				// Write Commands Here
				Placement = &l_GlobalBuf[++l_GlobalAt];
		
				// Clear it
				memset(Placement, 0, sizeof(*Placement));
		
				// Set as extended
				Placement->Ctrl.Type = 1;
			}
		}
		
		// Otherwise eat first spot
		else
		{
			Placement = &l_GlobalBuf[++l_GlobalAt];
			Placement->Ctrl.Type = 1;
		}
	}
	
	/* Worked? */
	if (Placement)
	{
		*a_Wp = &Placement->Ext.DataBuf[Placement->Ext.DataSize];
		WriteUInt8((uint8_t**)a_Wp, a_ID);
		l_GlobalBuf[l_GlobalAt].Ext.DataSize += a_NeededSize + 1;
	}
	
	/* Return it */
	return Placement;
}

/* D_XNetGlobalTic() -- Wrapper for DS_GrabGlobal() */
bool_t D_XNetGlobalTic(const uint8_t a_ID, void** const a_Wp)
{
	/* Check */
	if (!D_XNetIsServer() || a_ID < 0 || a_ID >= NUMDTCT)
		return false;
	
	/* Call it */
	return !!DS_GrabGlobal(a_ID, c_TCDataSize[a_ID], a_Wp);
}

/* D_XNetGetCommand() -- Grabs command from tic */
bool_t D_XNetGetCommand(const uint8_t a_ID, const uint32_t a_Size, void** const a_Wp, ticcmd_t* const a_TicCmd)
{
	uint16_t* dsP;
	uint8_t** dbP;
	
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_TicCmd)
		return false;
	
	/* Extended Tic */
	if (a_TicCmd->Ctrl.Type == 1)
	{
		dsP = &a_TicCmd->Ext.DataSize;
		dbP = &a_TicCmd->Ext.DataBuf;
	}
		
	/* Standard Tic */
	else if (a_TicCmd->Ctrl.Type == 0)
	{
		dsP = &a_TicCmd->Std.DataSize;
		dbP = &a_TicCmd->Std.DataBuf;
	}
	
	/* Bad Type */
	else
		return false;
	
	/* Not enough room to store extended command? */
	if (a_Size + 2 >= MAXTCDATABUF - *dsP)
		return false;
		
	/* Write Command at point */
	*a_Wp = &((*dbP)[*dsP]);
	WriteUInt8((uint8_t**)a_Wp, a_ID);
	*dsP += a_Size + 1;
	
	// Was written OK
	return true;
}

/*** FUNCTIONS ***/

/*****************************************************************************/

/******************************
*** NEW EXTENDED NETWORKING ***
******************************/

/*** CONSTANTS ***/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])
#define MAXLOCALJOYS	MAXJOYSTICKS			// Max joysticks handled

/*** STRUCTURES ***/

/*** GLOBALS ***/

D_XPlayer_t** g_XPlays = NULL;					// Extended Players
size_t g_NumXPlays = 0;							// Number of them

tic_t g_DemoFreezeTics = 0;						// Tics to freeze demo for
bool_t g_NetBoardDown = false;					// Scoreboard down in netdemo

/*** LOCALS ***/

static tic_t l_XNLastPTic;						// Last Program Tic

static bool_t l_PermitMouse = false;			// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static uint32_t l_MouseButtons[2];				// Mouse buttons down (sng/dbl)
static tic_t l_MouseLastTime[32];				// Last time pressed
static bool_t l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];

static uint32_t l_LocalHostID;					// ID of localhost
static bool_t l_PreppedSave;					// Prepped Savegame

static bool_t l_ForceLag;						// Forces Lagging
static tic_t l_LastJW;							// Last Join Window

static D_XNetTicBuf_t** l_TicStore;				// Tic Storage
static size_t l_NumTicStore;					// Tics in storage

static D_XPlayer_t* l_CacheSVXPlay;

static CONCTI_Inputter_t* l_ChatBox[MAXSPLITSCREEN];	// Splitscreen chat

//l_SVMaxCatchup

/*** FUNCTIONS ***/

extern int demosequence;
extern int pagetic;

/* D_XNetBufForTic() -- Get buffer for tic */
D_XNetTicBuf_t* D_XNetBufForTic(const tic_t a_GameTic, const bool_t a_Create)
{
	size_t i;
	
	/* Look in list */
	for (i = 0; i < l_NumTicStore; i++)
		if (l_TicStore[i])
			if (l_TicStore[i]->GameTic == a_GameTic)
				return l_TicStore[i];
	
	/* Not Worth it? */
	if (a_GameTic < gametic)
		return NULL;
	
	/* Do not create? */
	if (!a_Create)
		return NULL;
	
	/* Allocate new spot */
	for (i = 0; i < l_NumTicStore; i++)
		if (!l_TicStore[i])
			break;
	
	// End?
	if (i >= l_NumTicStore)
	{
		Z_ResizeArray((void**)&l_TicStore, sizeof(*l_TicStore),
			l_NumTicStore, l_NumTicStore + 1);
		i = l_NumTicStore++;
	}
	
	// Place here
	l_TicStore[i] = Z_Malloc(sizeof(*l_TicStore[i]), PU_STATIC, NULL);
	l_TicStore[i]->GameTic = a_GameTic;
	return l_TicStore[i];
}

/* D_XNetWipeBefores() -- Wipe tics all before this tic */
void D_XNetWipeBefores(const tic_t a_GameTic)
{	
	size_t i;
	
	/* Erase all in store */
	for (i = 0; i < l_NumTicStore; i++)
		if (l_TicStore[i])
			if (l_TicStore[i]->GameTic < a_GameTic)
			{
				Z_Free(l_TicStore[i]);
				l_TicStore[i] = NULL;
			}
}

/* D_XNetTicBufSum() -- Calculates checksum of Tic Buffer */
uint32_t D_XNetTicBufSum(D_XNetTicBuf_t* const a_TicBuf,  const D_XNetTicBufVersion_t a_VersionNum, const uint32_t a_Players)
{
	int32_t i;
	uint32_t RetVal = UINT32_C(0xDEADBEEF);
	ticcmd_t* TicCmd;
	
	/* Go through players */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Commands
		TicCmd = &a_TicBuf->Tics[i];
		
		// Not playing?
		if (!(a_Players & (1 << i)))
			continue;
		
		// XOR in buttons
		RetVal ^= TicCmd->Std.buttons;
	}
	
	/* Return calculated code */
	return RetVal;
}

/* D_XNetEncodeTicBuf() -- Encodes tic buffer into more compact method */
void D_XNetEncodeTicBuf(D_XNetTicBuf_t* const a_TicBuf, uint8_t** const a_OutD, uint32_t* const a_OutSz, const D_XNetTicBufVersion_t a_VersionNum)
{
	// Size for a single player
	// 384 + 2 + 2 + 4 + 1 + 2 + 2 + 32 + 1 + 4 + 2 + 2 + 1 + 1 + 1 + 8 = 449
	// For 32+global: 449 * 33 = 14817
#define BUFSIZE 16384
	static uint8_t* Buf;
	D_XNetTicBuf_t* TicBuf;
	uint8_t* p;
	uint64_t Left;
	uint16_t u16;
	uint32_t u32;
	int32_t i, j, z;
	ticcmd_t* Cmd;
	uint16_t* dsP;
	uint8_t* dbP, u8;
	
	/* Check */
	if (!a_TicBuf || !a_OutD || !a_OutSz)
		return;
	
	/* Init */
	TicBuf = a_TicBuf;
	
	// Allocate buffer and start there
	if (!Buf)
		Buf = Z_Malloc(sizeof(*Buf) * BUFSIZE, PU_STATIC, NULL);
	p = Buf;
	
	/* Write Version Number */
	WriteUInt8(&p, a_VersionNum);
	
	/* Write Data */
	// Encode gametic in multiple parts
	Left = TicBuf->GameTic;
	do
	{
		// Write lower bits
		u16 = Left & UINT16_C(0x7FFF);
		Left >>= UINT64_C(15);
		
		// More data?
		if (Left)
			u16 |= UINT16_C(0x8000);
		
		// Encode
		LittleWriteUInt16(&p, u16);
	} while (Left);
	
	/* Player Counts */
	for (u32 = 0, i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			u32 |= UINT32_C(1) << i;
	
	// Reverse mask
	u32 &= ~a_TicBuf->PIGRevMask;
	
	// Write counts
	LittleWriteUInt32(&p, u32);
	
	// Do not use u32 any longer
	
	/* Encode player tics */
	for (i = 0; i < MAXPLAYERS + 1; i++)
	{
		// Not in game?
		if (!(u32 & (1 << i)) && i != MAXPLAYERS)
			continue;
		
		// Get Command for this player
		Cmd = &TicBuf->Tics[i];
		
		// Encode Type
		WriteUInt8(&p, Cmd->Ctrl.Type);
		
		// Encode Ping
		LittleWriteUInt16(&p, Cmd->Ctrl.Ping);
		
		// Standard player stuff
		if (!Cmd->Ctrl.Type)
		{	
			// Set Data to encode
			u16 = 0;

#define __DIFFY(x,y) if (Cmd->Std.x) u16 |= y;
#define __WRITE(x,y) if (u16 & x) y
			
			__DIFFY(forwardmove, DDB_FORWARD);
			__DIFFY(sidemove, DDB_SIDE);
			__DIFFY(aiming, DDB_AIMING);
			__DIFFY(buttons, DDB_BUTTONS);
			__DIFFY(angleturn, DDB_ANGLE);
			__DIFFY(InventoryBits, DDB_INVENTORY);
			__DIFFY(StatFlags, DDB_STATFLAGS);
			__DIFFY(artifact, DDB_ARTIFACT);
			__DIFFY(FlySwim, DDB_FLYSWIM);
			__DIFFY(XSNewWeapon[0], DDB_WEAPON);
			
			// Save a byte by encoding the more important commands first
			Left = u16;
			u8 = u16 & UINT16_C(0x7F);
			Left >>= 7;
			if (Left)
				u8 |= UINT8_C(0x80);
			WriteUInt8(&p, u8);
			if (Left)
			{
				u8 = Left & UINT16_C(0xFF);
				WriteUInt8(&p, u8);
			}
			
			__WRITE(DDB_FORWARD, WriteInt8(&p, Cmd->Std.forwardmove));
			__WRITE(DDB_SIDE, WriteInt8(&p, Cmd->Std.sidemove));
			__WRITE(DDB_AIMING, LittleWriteUInt16((uint16_t**)&p, Cmd->Std.aiming));
			__WRITE(DDB_ANGLE, LittleWriteInt16((int16_t**)&p, Cmd->Std.angleturn));
			__WRITE(DDB_INVENTORY, WriteUInt8(&p, Cmd->Std.InventoryBits));
			__WRITE(DDB_ARTIFACT, WriteUInt8(&p, Cmd->Std.artifact));
			__WRITE(DDB_FLYSWIM, LittleWriteInt16((int16_t**)&p, Cmd->Std.FlySwim));
			
			if (u16 & DDB_WEAPON)
				WriteString((uint8_t**)&p, Cmd->Std.XSNewWeapon);
			
			// Variable encode buttons and status flags
			for (z = 0; z < 2; z++)
			{
				// Check if flag is not set, if not do not write anything
				if (!(u16 & (z ? DDB_BUTTONS : DDB_STATFLAGS)))
					continue;
				
				// Encode in variable length
				Left = (z ? Cmd->Std.buttons : Cmd->Std.StatFlags);
				do
				{
					u8 = Left & UINT32_C(0x7F);
					Left >>= 7;
					
					if (Left)
						u8 |= UINT8_C(0x80);
					
					WriteUInt8(&p, u8);
				} while (Left);
			}
			
			// Data pointers
			dsP = &Cmd->Std.DataSize;
			dbP = &Cmd->Std.DataBuf;
#undef __WRITE	
#undef __DIFFY
		}
		
		// Extended data
		else
		{
			// Data pointers
			dsP = &Cmd->Ext.DataSize;
			dbP = &Cmd->Ext.DataBuf;
		}
		
		// Write pointer data
		Left = *dsP;
		do
		{
			// Get value and shift down
			u16 = Left & UINT8_C(0x7F);
			Left >>= 7;
			
			// More Data?
			if (Left)
				u16 |= UINT8_C(0x80);
			
			// Write Value
			WriteUInt8(&p, u16);
		} while (Left);
		
		// Write buffer
		for (j = 0; j < *dsP; j++)
			WriteUInt8(&p, dbP[j]);
	}
	
	/* Checksum */
	LittleWriteUInt32(&p, D_XNetTicBufSum(a_TicBuf, a_VersionNum, u32));
	
	/* Done */
	*a_OutD = Buf;
	*a_OutSz = p - Buf;
#undef BUFSIZE
}

/* D_XNetDecodeTicBuf() -- Decodes tic buffer */
bool_t D_XNetDecodeTicBuf(D_XNetTicBuf_t* const a_TicBuf, const uint8_t* const a_InD, const uint32_t a_InSz)
{
#define BUFSIZE 16384
	static uint8_t* Buf;
	uint8_t* p, u8;
	uint16_t u16, Mask;
	uint32_t PIG, u32;
	ticcmd_t* Cmd;
	uint16_t* dsP;
	uint8_t* dbP, VersionNum;
	int32_t i, j, z, ShiftMul;
	
	/* Check */
	if (!a_TicBuf || !a_InD || !a_InSz)
		return;
	
	/* Init */
	if (!Buf)
		Buf = Z_Malloc(BUFSIZE, PU_STATIC, NULL);
	
	// Setup buffer
	memset(Buf, 0, BUFSIZE);
	memmove(Buf, a_InD, (a_InSz < BUFSIZE ? a_InSz : BUFSIZE));
	p = Buf;
	
	// Clear tic buffer
	memset(a_TicBuf, 0, sizeof(*a_TicBuf));
	
	/* Read Data */
	// Read Version
	VersionNum = ReadUInt8(&p);
	
	// Read Gametic
	ShiftMul = 0;
	do
	{
		u16 = LittleReadUInt16(&p);
		a_TicBuf->GameTic |= ((tic_t)(u16 & UINT16_C(0x7FFF))) << (15 * ShiftMul++);
	} while (u16 & UINT16_C(0x8000));
	
	// Player in game
	PIG = LittleReadUInt32(&p);
	
	// Player Commands
	for (i = 0; i < MAXPLAYERS + 1; i++)
	{
		// Get tic command
		Cmd = &a_TicBuf->Tics[i];
		
		// not in game?
		if (!(PIG & (1 << i)) && i != MAXPLAYERS)
			continue;
		
		// Read type
		Cmd->Ctrl.Type = ReadUInt8(&p);
		
		// Read Ping
		Cmd->Ctrl.Ping = LittleReadUInt16(&p);
		
		// Standard Player
		if (!Cmd->Ctrl.Type)
		{
			// Read config mask
			u8 = ReadUInt8(&p);
			Mask = u8 & UINT8_C(0x7F);
		
			if (u8 & UINT8_C(0x80))
			{
				u8 = ReadUInt8(&p);
				Mask |= ((uint16_t)u8) << 7;
			}
			
			if (Mask & DDB_FORWARD)
				Cmd->Std.forwardmove = ReadInt8(&p);
			if (Mask & DDB_SIDE)
				Cmd->Std.sidemove = ReadInt8(&p);
			if (Mask & DDB_AIMING)
				Cmd->Std.aiming = LittleReadUInt16(&p);
			if (Mask & DDB_ANGLE)
				Cmd->Std.angleturn = LittleReadInt16(&p);
			if (Mask & DDB_INVENTORY)
				Cmd->Std.InventoryBits = ReadUInt8(&p);
			if (Mask & DDB_ARTIFACT)
				Cmd->Std.artifact = ReadUInt8(&p);
			if (Mask & DDB_FLYSWIM)
				Cmd->Std.FlySwim = LittleReadInt16(&p);
			
			if (Mask & DDB_WEAPON)
			{
				j = 0;
				do
				{
					u8 = ReadUInt8(&p);
					
					if (j < MAXTCWEAPNAME)
						Cmd->Std.XSNewWeapon[j++] = u8;
				} while (u8);
			}
			
			// Variable decode buttons and status flags
			for (z = 0; z < 2; z++)
			{
				// Check if flag is set
				if (!(Mask & (z ? DDB_BUTTONS : DDB_STATFLAGS)))
					continue;
				
				// Read variable length
				u32 = ShiftMul = 0;
				do
				{
					u8 = ReadUInt8(&p);
					u32 |= ((uint32_t)(u8 & UINT8_C(0x7F))) << (7 * ShiftMul++);
				} while (u8 & UINT8_C(0x80));
				
				// Set value
				if (z)
					Cmd->Std.buttons = u32;
				else
					Cmd->Std.StatFlags = u32;
			}
			
			// Data pointers
			dsP = &Cmd->Std.DataSize;
			dbP = &Cmd->Std.DataBuf;
		}
		
		// Extended
		else
		{
			// Data pointers
			dsP = &Cmd->Ext.DataSize;
			dbP = &Cmd->Ext.DataBuf;
		}
		
		// Read Size
		*dsP = ShiftMul = 0;
		do
		{
			u8 = ReadUInt8(&p);
			*dsP |= (tic_t)(u8 & UINT16_C(0x7F)) << (7 * ShiftMul++);
		} while (u8 & UINT16_C(0x80));
		
		// Read data buffer
		for (j = 0; j < *dsP; j++)
		{
			u8 = ReadUInt8(&p);
			
			if (j < MAXTCDATABUF)
				dbP[j] = u8;
		}
	}
	
	/* Confirm checksum */
	u32 = LittleReadUInt32(&p);
	return (u32 == D_XNetTicBufSum(a_TicBuf, VersionNum, PIG));
#undef BUFSIZE
}

/* D_XNetPlaceTicCmd() -- Place tic command in store */
void D_XNetPlaceTicCmd(const tic_t a_GameTic, const int32_t a_Player, ticcmd_t* const a_Cmd)
{
	D_XNetTicBuf_t* Buf;
	int32_t Player;
	
	/* Check */
	if (!a_Cmd)
		return;
	
	/* Get Buffer */
	Buf = D_XNetBufForTic(a_GameTic, true);
	
	// Not there?
	if (!Buf)
		return;
	
	/* Correct player */
	if (a_Player < 0 || a_Player >= MAXPLAYERS)
		Player = MAXPLAYERS;
	else
		Player = a_Player;
	
	/* Place in specific location */
	memmove(&Buf->Tics[Player], a_Cmd, sizeof(Buf->Tics[Player]));
}

/* D_XNetSendTicToHost() -- Sends tic to XPlayer */
void D_XNetSendTicToHost(D_XNetTicBuf_t* const a_Buf, D_XPlayer_t* const a_Host)
{
	uint8_t* OutD;
	uint32_t OutSz;
	D_XPlayer_t* Host;
	int32_t i;
	D_XEndPoint_t* EP;
	D_BS_t* BS;
	uint32_t NowTime;
	
	/* Encode Commands */
	OutD = NULL;
	OutSz = 0;
	D_XNetEncodeTicBuf(a_Buf, &OutD, &OutSz, DXNTBV_LATEST);
	NowTime = I_GetTimeMS();
	
	/* If host specified */
	// Only send to them
	if (a_Host)
	{
		// Obtain real host
		Host = D_XNetPlayerByXPlayerHost(a_Host);
		
		// Not there?
		if (!Host)
			return;
			
		// Local, defunct or bot, ignore
		if (Host->Flags & (DXPF_LOCAL | DXPF_BOT | DXPF_DEFUNCT))
			return;
			
		// Get endpoint
		EP = Host->Socket.EndPoint;
	
		if (!EP)
			return;
		
		// Record block to them
		BS = EP->Desc->RelBS;
		D_BSBaseBlock(BS, "TICS");
		
		// Timing Codes
		D_BSwcu64(BS, Host->LastRanTic);
		D_BSwcu64(BS, Host->LastXMit);
		D_BSwcu64(BS, Host->LastAckTic);
		D_BSwcu64(BS, g_ProgramTic);
		D_BSwu32(BS, NowTime);
		
		D_BSWriteChunk(BS, OutD, OutSz);
		D_BSRecordNetBlock(BS, &EP->Addr);
	}
	
	// Otherwise send to all
	else
	{
		// Send to everyone
		for (i = 0; i < g_NumXPlays; i++)
		{
			Host = g_XPlays[i];
			
			// Not there?
			if (!Host)
				continue;
			
			// Local, defunct or bot, ignore
			if (Host->Flags & (DXPF_LOCAL | DXPF_BOT | DXPF_DEFUNCT))
				continue;
			
			// Obtain real Host
			Host = D_XNetPlayerByXPlayerHost(Host);
			
			// Not there?
			if (!Host)
				continue;
			
			// Get endpoint
			EP = Host->Socket.EndPoint;
			
			if (!EP)
				continue;
			
			// Already xmitted?
			if (a_Buf->GameTic <= Host->LastXMit)
				continue;
			
			// XMit now
			Host->LastXMit = a_Buf->GameTic;
			BS = EP->Desc->RelBS;
			
			// Record block to them
			D_BSBaseBlock(BS, "TICS");
			
			// Timing Codes
			D_BSwcu64(BS, Host->LastRanTic);
			D_BSwcu64(BS, Host->LastXMit);
			D_BSwcu64(BS, Host->LastAckTic);
			D_BSwcu64(BS, g_ProgramTic);
			D_BSwu32(BS, NowTime);
			
			D_BSWriteChunk(BS, OutD, OutSz);
			D_BSRecordNetBlock(BS, &EP->Addr);
		}
	}
}

/* D_XNetFinalCmds() -- Place final tic commands */
void D_XNetFinalCmds(const tic_t a_GameTic, const uint32_t a_SyncCode)
{
	D_XNetTicBuf_t* Buf;
	int32_t i, p, j;
	D_XPlayer_t* XPlay, *Host;
	D_BS_t* BS;
	I_HostAddress_t* AddrP;
	
	/* If server, store into buffer */
	if (D_XNetIsServer())
	{
		// Get Buffer
		Buf = D_XNetBufForTic(a_GameTic, false);
		
		// Not there?
		if (!Buf)
			return;
			
		// Place sync code there
		Buf->SyncCode = a_SyncCode;
		
		// Send to all
		D_XNetSendTicToHost(Buf, NULL);
	}
	
	/* If client, tell server the sync code for this gametic */
	// If the server feels otherwise, they kick us
	else
	{
		// Tell the server the sync code for this tic
		BS = D_XBRouteToServer(NULL, &AddrP);
		
		if (BS)
		{
			D_BSBaseBlock(BS, "TRUN");
			D_BSwcu64(BS, a_GameTic);
			D_BSwu32(BS, a_SyncCode);
			D_BSRecordNetBlock(BS, AddrP);
		}
	}
}

/* D_XNetDisconnect() -- Disconnect from server/self server */
// Replaces D_NCDisconnect()
void D_XNetDisconnect(const bool_t a_FromDemo)
{
	int32_t i;
	static bool_t DoingDiscon;
	
	/* Already disconnecting? */
	if (DoingDiscon)
		return;
	DoingDiscon = true;
	
	/* Send quit message to server */
	if (!a_FromDemo)
		D_XNetSendQuit();
	
	/* Disconnect all players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			D_XNetKickPlayer(g_XPlays[i], DS_GetString(DSTR_NET_SERVERDISCON), false);
	
	/* Demos? */
	if (!a_FromDemo)
		if (demoplayback)
		{
			// Don't quit when the demo stops
			singledemo = false;
		
			// Stop it
			G_StopDemoPlay();
		}
	
	/* Clear network stuff */
	// Server Cache
	l_CacheSVXPlay = NULL;
	
	// Tics
	
	// Global Commands
	memset(l_GlobalTime, 0, sizeof(l_GlobalTime));
	memset(l_GlobalBuf, 0, sizeof(l_GlobalBuf));
	l_GlobalAt = -1;
	
	// Consistency problems
	l_ConsistencyFailed = false;
	
	// If connected, reset splits
	if (l_IsConnected)
		D_NCResetSplits(a_FromDemo);
	
	// Revert back to solo networking
	if (!a_FromDemo)
	{
		l_SoloNet = true;
		l_IsConnected = false;	// Set disconnected
		l_LocalHostID = 0;
	}
	
	// Reset time to now
	l_XNLastPTic = g_ProgramTic;
	g_DemoFreezeTics = 0;
	
	// There are no bots
	g_GotBots = false;
	
	// Reset random numbers to nothing
	P_SetRandIndex(0);
	
	/* Clear all player information */
	// Reset all vars
	P_XGSSetAllDefaults();
	
	// Just wipe ALL of it!
	memset(players, 0, sizeof(players));
	memset(playeringame, 0, sizeof(playeringame));
	
	// Always reset demo splits though
	D_NCResetSplits(true);
	
	// Initialize some players some
	for (i = 0; i < MAXPLAYERS; i++)
		G_InitPlayer(&players[i]);
	
	/* Destroy the level */
	P_ExClearLevel();
	
	/* Destroy Socket */
	// But not when playing a demo
	//if (!a_FromDemo)
		D_XBSocketDestroy();
	
	/* Wipe all the old tics */
	D_XNetWipeBefores((tic_t)-1);
	
	/* Go back to the title screen */
	if (!a_FromDemo)
	{
		gamestate = GS_DEMOSCREEN;
		
		demosequence = -1;
		pagetic = -1;
	}
	
	/* Remove any remaining XPlayers */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
		{
			Z_Free(g_XPlays[i]);
			g_XPlays[i] = NULL;
		}
	
	/* Done disconnecting */
	DoingDiscon = false;
	g_NetBoardDown = false;
	P_SpecInit(-2);
}

/* DS_XNetMakeServPB() -- Callback for make player */
static void DS_XNetMakeServPB(D_XPlayer_t* const a_Player, void* const a_Data)
{
	/* Set initial information */
	a_Player->Flags |= DXPF_SERVER | DXPF_LOCAL;
	
	/* Process ID from local player? */
	//if (a_Data)
		a_Player->ClProcessID = *((uint32_t*)a_Data);
}

/* D_XNetMakeServer() -- Creates a server, possibly networked */
void D_XNetMakeServer(const bool_t a_Networked, I_HostAddress_t* const a_Addr, const uint32_t a_GameID, const bool_t a_NotHost)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	int32_t i;
	D_XPlayer_t* SPlay;
	player_t* FakeP;
	uint32_t ProcessID;
	bool_t XBOk;
	
	/* Disconnect First */
	D_XNetDisconnect(false);
	
	/* Address required if net and not hosted */
	if (a_Networked && a_NotHost)
		if (!a_Addr)
			return;
	
	/* Create process ID, or grab one */
	ProcessID = 0;
	
	// Try grabbing from first split
	if (D_ScrSplitHasPlayer(0))
		ProcessID = g_Splits[0].ProcessID;
	
	// Otherwise create a random one
	if (!ProcessID)
		ProcessID = D_XNetMakeID(0);
	
	/* Create a starting spectator (the host) */
	SPlay = D_XNetAddPlayer(DS_XNetMakeServPB, (void*)&ProcessID, false);
	
	// Set server infos
	l_IsConnected = true;	// Connected to self!
	
	/* Clear important flags */
	// not playing title screen demos
	g_TitleScreenDemo = false;
	
	/* Set the proper gamestate */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
	
	/* Set all game vars */
	if (!demoplayback)
		NG_ApplyVars();
	
	/* Setup Socket Connection */
	if (a_Networked)
	{
		// Not acting as network host
		if (a_NotHost)
			XBOk = D_XBCallHost(a_Addr, a_GameID);
		
		// We are network host
		else
			XBOk = D_XBWaitForCall(a_Addr);
		
		// Creation failed?
		if (!XBOk)
		{
			memset(Buf, 0, sizeof(Buf));
			I_NetHostToString(a_Addr, Buf, BUFSIZE);
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_BINDFAIL, "%s\n", Buf);
			D_XNetDisconnect(false);
			return;
		}
	}
	
	/* If dedicated, no need to continue */
	if (l_IsDedicated)
		return;
	
	/* Initialize screens */
	// Only give players to screens that have occupents
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitHasPlayer(i))
		{
			// Create initial player for server, but not for P1
				// P1 already has one
			if (i)
				SPlay = D_XNetAddPlayer(DS_XNetMakeServPB, (void*)g_Splits[i].ProcessID, false);
			
			// Assign player
			g_Splits[i].XPlayer = SPlay;
			SPlay->ScreenID = i;
			SPlay->Profile = g_Splits[i].Profile;	// if lucky
			
			// Get fake player
			FakeP = P_SpecGet(i);
			
			// Map to this one
			if (FakeP)
				FakeP->XPlayer = SPlay;
			
			// Up splitscreen
			g_SplitScreen = i;
		}
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
#undef BUFSIZE
}

/* D_XNetConnect() -- Connects to server */
void D_XNetConnect(I_HostAddress_t* const a_Addr, const uint32_t a_GameID, const bool_t a_NotClient)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	bool_t XBOk;
	
	/* Need address if we are a standard client */
	if (!a_NotClient)
		if (!a_Addr)
			return;
	
	/* Disconnect First */
	D_XNetDisconnect(false);
	D_XBSocketDestroy();
	
	/* Set standard connection state */
	// Set as connected, even though one might not be!
	l_IsConnected = true;
	gamestate = GS_WAITFORJOINWINDOW;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
	
	/* Connect to remote client with specified ID or wait */
	// Anti-Client
	if (a_NotClient)
		XBOk = D_XBWaitForCall(a_Addr);
	
	// Standard Client
	else
		XBOk = D_XBCallHost(a_Addr, a_GameID);
	
	/* Creation failed? */
	if (!XBOk)
	{
		memset(Buf, 0, sizeof(Buf));
		I_NetHostToString(a_Addr, Buf, BUFSIZE);
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_BINDFAIL, "%s\n", Buf);
		D_XNetDisconnect(false);
		return;
	}
#undef BUFSIZE
}

/* D_XNetHostnameToAddrGID() -- Hostname to IP and GameID */
bool_t D_XNetHostnameToAddrGID(const char* const a_Hostname, I_HostAddress_t* const a_Addr, uint32_t* const a_GameIDp)
{
	char* p, *Dup;
	uint32_t GameID;
	bool_t RetVal;	
	
	/* Check */
	if (!a_Hostname || !a_Addr)
		return false;
	
	/* Clear */
	memset(a_Addr, 0, sizeof(*a_Addr));
	if (a_GameIDp)
		*a_GameIDp = 0;
	
	/* Duplicate Host */
	Dup = Z_StrDup(a_Hostname, PU_STATIC, NULL);
		
	/* Find slash (Game ID) */
	GameID = 0;
	p = strchr(Dup, '/');
	
	// Found?
	if (p)
	{
		// Remove and move up
		*(p++) = 0;
		
		// Get the GameID
		GameID = C_strtou32(p, NULL, 0);
	}
	
	/* Resolve IP first */
	RetVal = true;
	if (!I_NetNameToHost(NULL, a_Addr, Dup))
		RetVal = false;
	
	// Set GameID
	if (a_GameIDp)
		*a_GameIDp = GameID;
	
	/* Cleanup */
	Z_Free(Dup);
	
	/* Return success or failure */
	return RetVal;
}

/* D_XNetIsServer() -- Returns true if we are the server */
bool_t D_XNetIsServer(void)
{
	int32_t i;
	
	static int32_t XNISNum, XNISCount;
	
	/* If Dedicated, return true */
	if (l_IsDedicated)
		return true;
	
	/* Cached? */
	if (l_CacheSVXPlay)
	{
		// Now illegal?
		if (XNISCount != g_NumXPlays || XNISNum >= g_NumXPlays || l_CacheSVXPlay != g_XPlays[XNISNum] || (l_CacheSVXPlay && (l_CacheSVXPlay->Flags & DXPF_DEFUNCT)))
		{
			XNISCount = -1;
			XNISNum = -1;
			l_CacheSVXPlay = NULL;
		}
		
		// Cached response
		else
			return true;
	}
	
	/* Grab server status from first local player */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				if (!!((g_XPlays[i]->Flags & (DXPF_SERVER | DXPF_DEFUNCT)) == DXPF_SERVER))
				{
					// Cache response for more "speed"
					XNISCount = g_NumXPlays;
					XNISNum = i;
					l_CacheSVXPlay = g_XPlays[i];
					return true;
				}
	
	/* Fell through */
	return false;
}

/* D_XNetIsConnected() -- Is connected to server */
bool_t D_XNetIsConnected(void)
{
	return l_IsConnected | D_XNetIsServer();
}

/* D_XNetGetHostID() -- Gets our host ID */
bool_t D_XNetGetHostID(void)
{
	int32_t i;
	
	/* Grab server status from first local player */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				return g_XPlays[i]->HostID;
	
	/* If we are connecting (loading the save) then return the mapped ID */
	return l_LocalHostID;
}

/* D_XNetSetHostID() -- Sets the local host ID */
void D_XNetSetHostID(const uint32_t a_NewID)
{
	l_LocalHostID = a_NewID;
}

/* D_XNetPlayerByXPlayerHost() -- Finds host specifier for this player */
D_XPlayer_t* D_XNetPlayerByXPlayerHost(D_XPlayer_t* const a_XPlayer)
{
	int32_t i;
	
	/* Check */
	if (!a_XPlayer)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->HostID == a_XPlayer->HostID)
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetPlayerByID() -- Finds player by ID */
D_XPlayer_t* D_XNetPlayerByID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->ID == a_ID)
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetPlayerByHostID() -- Finds player by host ID */
D_XPlayer_t* D_XNetPlayerByHostID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->HostID == a_ID)
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetLocalPlayerByPID() -- Finds local player by process ID */
D_XPlayer_t* D_XNetLocalPlayerByPID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				if (g_XPlays[i]->ClProcessID == a_ID)
					return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetPlayerByString() -- Finds player by string */
D_XPlayer_t* D_XNetPlayerByString(const char* const a_Str)
{
	uint32_t IntNum;
	D_XPlayer_t* XPlay;
	
	/* Check */
	if (!a_Str)
		return NULL;
	
	/* String empty */
	if (strlen(a_Str) <= 0)
		return NULL;
	
	/* Direct array slot */
	IntNum = C_strtou32(a_Str, NULL, 10);
	
	// Hit?
	if (IntNum >= 1 && IntNum <= g_NumXPlays)
		if (g_XPlays[IntNum - 1])
			return g_XPlays[IntNum - 1];
	
	/* By ID */
	IntNum = C_strtou32(a_Str, NULL, 16);
	
	// Find?
	XPlay = D_XNetPlayerByID(IntNum);
	
	// Found?
	if (XPlay)
		return XPlay;
	
	/* Try Account Match */
	// Loop
	for (IntNum = 0; IntNum < g_NumXPlays; IntNum++)
	{
		// Get
		XPlay = g_XPlays[IntNum];
		
		// Missing?
		if (!XPlay)
			continue;
			
		// Name match?
		if (strcasecmp(a_Str, XPlay->AccountName) == 0)
			return XPlay;
	}
	
	/* Not Found */
	return NULL;
}

/* D_XNetPlayerByAddr() -- Finds player by address */
D_XPlayer_t* D_XNetPlayerByAddr(const I_HostAddress_t* const a_Addr)
{
	int32_t i;
	
	/* Check */
	if (!a_Addr)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (I_NetCompareHost(a_Addr, &g_XPlays[i]->Socket.Address))
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetAddPlayer() -- Adds new player */
D_XPlayer_t* D_XNetAddPlayer(void (*a_PacketBack)(D_XPlayer_t* const a_Player, void* const a_Data), void* const a_Data, const bool_t a_FromGTicker)
{
	D_XPlayer_t* New, *Other;
	uint32_t ID, OurHID;
	void* Wp;
	ticcmd_t* Placement;
	int32_t i;
	player_t* FP;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Base initialization */
	New->InGameID = -1;
	New->ScreenID = -1;
	strncpy(New->AccountName, "I have no account!", MAXPLAYERNAME);
	strncpy(New->AccountServer, "remood.org", MAXXSOCKTEXTSIZE);
	
	// Correct account name
	D_ProfFixAccountName(New->AccountName);
	
	/* Call callback */
	if (a_PacketBack)
		a_PacketBack(New, a_Data);
	
	// ID already taken?
	Other = D_XNetPlayerByID(New->ID);
	if (New->ID && Other && Other != New)
	{
		Z_Free(New);
		return NULL;
	}
	
	/* Other already exists */
	// But is defunct
	if (Other)
		if (Other->Flags & DXPF_DEFUNCT)
			return NULL;
	
	/* Create ID for the player */
	// If it does not exist!
	if (!New->ID)
		// Set ID, is hopefully really random
		New->ID = D_XNetMakeID(0);
	
	/* Link into players list */
	// Find free spot
	for (i = 0; i < g_NumXPlays; i++)
		if (!g_XPlays[i])
			break;
	
	// No room?
	if (i >= g_NumXPlays)
	{
		Z_ResizeArray((void**)&g_XPlays, sizeof(*g_XPlays),
			g_NumXPlays, g_NumXPlays + 1);
		i = g_NumXPlays++;
	}
	
	// Set here
	g_XPlays[i] = New;
	
	/* Bind to screen */
	OurHID = D_XNetGetHostID();
	if (New->HostID == OurHID)
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (D_ScrSplitHasPlayer(i))
				if (g_Splits[i].ProcessID == New->ClProcessID)
				{
					// Setup XPlayer and split
					g_Splits[i].XPlayer = New;
					g_Splits[i].Console = 0;
					g_Splits[i].Display = -1;
					New->ScreenID = i;
					
					// Initialize fake player (spectator)
					P_SpecInit(i);
					
					// Set local angle, so it matches
					FP = P_SpecGetPOV(i);
					
					if (FP && FP->mo)
						localangle[i] = FP->mo->angle;
					break;
				}
	
	/* Send player creation packet (if server) */
	// But only if it was not created by a G_Ticker() call
	if (D_XNetIsServer() && !a_FromGTicker)
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XADDPLAYER, c_TCDataSize[DTCT_XADDPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Addition Info
			LittleWriteUInt32((uint32_t**)&Wp, New->ID);
			LittleWriteUInt32((uint32_t**)&Wp, New->HostID);
			LittleWriteUInt32((uint32_t**)&Wp, New->ClProcessID);
			WriteUInt8((uint8_t**)&Wp, New->ScreenID);
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			LittleWriteUInt32((uint32_t**)&Wp, New->Flags);
			
			// Write Names
			for (i = 0; i < MAXPLAYERNAME; i++)
			{
				WriteUInt8((uint8_t**)&Wp, New->AccountName[i]);
				WriteUInt8((uint8_t**)&Wp, New->AccountCookie[i]);
			}
		}
	}
	
	/* Write console message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTCONNECTED, "%s\n",
			New->AccountName
		);
	
	/* Update Scores */
	P_UpdateScores();
	
	/* Return fresh player */
	return New;
}

/* D_XNetKickPlayer() -- Kicks player for some reason */
void D_XNetKickPlayer(D_XPlayer_t* const a_Player, const char* const a_Reason, const bool_t a_FromGTicker)
{
	size_t Slot;
	int32_t i, j;
	void* Wp;
	ticcmd_t* Placement;
	uint32_t OurHID;
	
	/* Check */
	if (!a_Player)
		return;
	
	/* Already defunct? */
	if (a_Player->Flags & DXPF_DEFUNCT)
		return;
	
	/* Remove from split */
	P_SpecDelXPlayer(a_Player);
	
	/* Write console message */
	if (!(a_Player->Flags & DXPF_DEFUNCT))
		CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTGONE, "%s%s\n",
				a_Player->AccountName,
				(!a_Reason ? DS_GetString(DSTR_NET_NOREASON) : a_Reason)
			);
		
	/* Mark Defunct */
	a_Player->Flags |= DXPF_DEFUNCT;
	
	/* Get our host ID */
	OurHID = D_XNetGetHostID();
	
	/* Find slot player uses */
	for (Slot = 0; Slot < g_NumXPlays; Slot++)
		if (a_Player == g_XPlays[Slot])
			break;
	
	// Not in any slot? Must have been removed then
	if (Slot >= g_NumXPlays)
		return;
	
	/* Remove from slot */
	//g_XPlays[Slot] = NULL;
	
	/* Local Player */
	if (a_Player->Flags & DXPF_LOCAL)
	{
		// Disconnect from server, if no more local players remain
		for (j = 0, i = 0; i < g_NumXPlays; i++)
			if (g_XPlays[i])
				if (g_XPlays[i]->Flags & DXPF_LOCAL)
					j++;
		
		// No players, so disconnect ourself
		if (j <= 0)
			D_XNetDisconnect(false);
		
		// Remove from local screen
		if (a_Player->HostID == OurHID)
			for (i = 0; i < MAXSPLITSCREEN; i++)
				if (D_ScrSplitVisible(i))
					if (a_Player == g_Splits[i].XPlayer)
						D_NCRemoveSplit(i, demoplayback);
	}
	
	/* Remote Player */
	else
	{
		// Send disconnection packet to them, if we are the server, and they
		// have no more clients attached.
		if (D_XNetIsServer())
		{
			// See if the player is not shared by any more hosts (all gone)
			for (j = 0, i = 0; i < g_NumXPlays; i++)
				if (g_XPlays[i])
				{
					// Ignore the player being kicked
					if (g_XPlays[i] == a_Player)
						continue;
					
					// Ignore defunct players
					if (g_XPlays[i]->Flags & DXPF_DEFUNCT)
						continue;
					
					// Ignore non matching HostIDs
					if (g_XPlays[i]->HostID != a_Player->HostID)
						continue;
					
					// A player with the same host is still inside
					j++;
				}
			
			// None left, Send them a disconnect notice
			if (!j)
			{
				D_XPDropXPlay(a_Player, a_Reason);
				
				// Drop their endpoint
				D_XBDelEndPoint(a_Player->Socket.EndPoint, a_Reason);
			}
		}
	}
	
	/* Enqueue Special Command, if server */
	// This informs the other clients (w/ demo) to delete the player. This is
	// also read for the game state, to remove players who have no controller.
	// So if a player is dropping and everyone is stuck on a WFP screen and the
	// player is removed, they will still be "in" the game until the tic command
	// removes them.
	if (D_XNetIsServer() && !a_FromGTicker)
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XKICKPLAYER, c_TCDataSize[DTCT_XKICKPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Send player to remove
			LittleWriteUInt16((uint16_t**)&Wp, a_Player->InGameID);
			LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
			
			// Write reason
			for (i = 0, j = 0; i < MAXTCCBUFSIZE; i++)
				if (a_Reason && a_Reason[j] && j < MAXTCCBUFSIZE - 1)
					WriteUInt8((uint8_t**)&Wp, a_Reason[j++]);
				else
					WriteUInt8((uint8_t**)&Wp, 0);
		}
		
		// Remove player ref, if any
		if (a_Player->Player)
			a_Player->Player->XPlayer = NULL;
	}
	
	/* Delete bot, if any */
	if (a_Player->BotData)
	{
		B_XDestroyBot(a_Player->BotData);
		a_Player->BotData = NULL;
	}
	
	/* Free associated data */
	//Z_Free(a_Player);
	
	/* Update Scores */
	P_UpdateScores();
}

/* D_XNetClearDefunct() -- Deletes any defunct XPlayers */
void D_XNetClearDefunct(void)
{
	int32_t i;
	D_XPlayer_t* XPlay;
	
	/* Loop */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Nothing?
		if (!XPlay)
			continue;
		
		// Player is defunct
		if (XPlay->Flags & DXPF_DEFUNCT)
		{
			g_XPlays[i] = NULL;
			Z_Free(XPlay);
		}
	}
}

/* D_XNetSpectate() -- Forces player to spectate */
void D_XNetSpectate(const int32_t a_PlayerID)
{
	void* Wp;
	ticcmd_t* Placement;
	D_XPlayer_t* XPlayer;
	
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS || !playeringame[a_PlayerID])
		return;
	
	/* Get XPlayer */
	XPlayer = players[a_PlayerID].XPlayer;
	
	/* If server setup command */
	if (D_XNetIsServer())
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XSPECPLAYER, c_TCDataSize[DTCT_XSPECPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Send player to remove
			LittleWriteUInt16((uint16_t**)&Wp, a_PlayerID);
			LittleWriteUInt32((uint32_t**)&Wp, (XPlayer ? XPlayer->ID : 0));
		}
	}
	
	/* Otherwise, request spectate from server */
	else
	{
	}
}

/* D_XNetSendQuit() -- Informs the server we are quitting */
void D_XNetSendQuit(void)
{
	int i;
	D_BS_t* BS;
	I_HostAddress_t* AddrP;
	
	/* If we are not the server, tell the server */
	if (!D_XNetIsServer())
	{
		// Tell all of our local screens to leave */
		for (i = 0; i < MAXSPLITSCREEN; i++)
			D_XNetPartLocal(g_Splits[i].XPlayer);
		
		// Send final disconnect to server
		if ((BS = D_XBRouteToServer(NULL, &AddrP)))
			D_XPSendDisconnect(NULL, BS, AddrP, "Disconnecting.");
	}
	
	/* Disconnect from the server */
	D_XNetDisconnect(false);
}

/* D_XNetPartLocal() -- Local player wants disconnecting from server */
void D_XNetPartLocal(D_XPlayer_t* const a_Player)
{
	/* Check */
	if (!a_Player)
		return;
	
	/* Not local? */
	if (!(a_Player->Flags & DXPF_LOCAL))
		return;
	
	/* If we are the server, kick em */
	if (D_XNetIsServer())
	{
		// Standard kick takes care of everything
		D_XNetKickPlayer(a_Player, "Left the game.", false);
	}
	
	/* Otherwise, inform the server */
	else
	{
	}
}

/* D_XNetChangeVar() -- Change Variable */
void D_XNetChangeVar(const uint32_t a_Code, const int32_t a_Value)
{
	ticcmd_t* Placement;
	void* Wp;
	
	/* Server can always change variables */
	if (D_XNetIsServer())
	{
		// Encode in command
		Placement = DS_GrabGlobal(DTCT_GAMEVAR, c_TCDataSize[DTCT_GAMEVAR], &Wp);
	
		if (Placement)
		{
			// Fill in data
			LittleWriteUInt32((uint32_t**)&Wp, a_Code);
			LittleWriteInt32((int32_t**)&Wp, a_Value);
		}
	}
	
	/* Client must ask the server */
	else
	{
	}
}

/* D_XNetChangeMap() -- Changes the map */
void D_XNetChangeMap(const char* const a_Map, const bool_t a_Reset)
{
	P_LevelInfoEx_t* Info;
	size_t i, j;
	void* Wp;
	ticcmd_t* Placement;
	
	/* Find level */
	Info = P_FindLevelByNameEx(a_Map, NULL);
	
	// Check
	if (!Info)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_NET_LEVELNOTFOUND, "%s\n", a_Map);
		return;
	}
	
	/* Server can change the map no problem */
	if (D_XNetIsServer())
	{
		Wp = NULL;
		Placement = DS_GrabGlobal(DTCT_MAPCHANGE, c_TCDataSize[DTCT_MAPCHANGE], &Wp);
	
		if (Placement)
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
	
	/* Client must ask the server though */
	else
	{
	}
}

/* D_XNetDirectChatEncode() -- Direct message encoding */
// This creates commands from buffer
void D_XNetDirectChatEncode(const uint32_t a_ID, const uint8_t a_Mode, const uint32_t a_Target, const char* const a_Message)
{
	char* p;
	int32_t Room;
	uint8_t* Wp;
	
	/* Check */
	if (!a_ID || !a_Message || !strlen(a_Message))
		return;
	
	/* Encode message into multiple tics */
	for (p = a_Message, Room = 0; *p; p++)
	{
		// No room? create new global
		if (!Room)
		{
			if (!D_XNetGlobalTic(DTCT_XCHATFRAG, &Wp))
				return;
			LittleWriteUInt32((uint32_t**)&Wp, a_ID);
			WriteUInt8((uint8_t**)&Wp, a_Mode);
			LittleWriteUInt32((uint32_t**)&Wp, a_Target);
			
			Room = MAXTCCBUFSIZE;
		}
		
		// Write single character and reduce the room available
		WriteUInt8((uint8_t**)&Wp, *p);
		Room--;
	}
}

/* D_XNetSendChat() -- Send chat message */
void D_XNetSendChat(D_XPlayer_t* const a_Source, const bool_t a_Team, const char* const a_Message)
{
	uint8_t Mode;
	const char* p;
	D_BS_t* RelBS;
	I_HostAddress_t* AddrP;
	D_XPlayer_t* XTarget;
	
	/* No origin */
	if (!a_Source || !a_Message)
		return;
	
	/* Base pointer */
	p = a_Message;
	XTarget = NULL;
	
	/* Who should recieve message? */
	if (a_Team)
		if (!a_Source->Player)
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
	if (D_XNetIsServer())
	{
		D_XNetDirectChatEncode(a_Source->ID, Mode, 0, p);
	}
	
	/* Otherwise, send request to server */
	else
	{
		// Obtain route to server
		RelBS = D_XBRouteToServer(NULL, &AddrP);
		
		if (RelBS)
		{
			// Base block data
			D_BSBaseBlock(RelBS, "CHAT");
			
			// Write Chat Data
			D_BSwu32(RelBS, a_Source->ID);
			D_BSwu8(RelBS, Mode);
			D_BSwu32(RelBS, (XTarget ? XTarget->ID : 0));
			D_BSwu32(RelBS, ++a_Source->ChatID);
			D_BSws(RelBS, p);
			
			// Send
			D_BSRecordNetBlock(RelBS, AddrP);
		}
	}
}

/* D_XNetChangeLocalProf() -- Change local screen to use this profile */
void D_XNetChangeLocalProf(const int32_t a_ScreenID, struct D_ProfileEx_s* const a_Profile)
{
	D_XPlayer_t* XPlay;	
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN || !a_Profile)
		return;
	
	/* Get */
	XPlay = g_Splits[a_ScreenID].XPlayer;
	
	/* No player? */
	if (!XPlay)
		return;
	
	/* Set screen to use profile */
	XPlay->Profile = a_Profile;
	g_Splits[a_ScreenID].Profile = a_Profile;
	
	// If player is attached, switch profile
	if (XPlay->Player)
		XPlay->Player->ProfileEx = a_Profile;
	
	// Copy UUID to XPlayer
	strncpy(XPlay->ProfileUUID, a_Profile->UUID, MAXUUIDLENGTH);
	
	/* Auto-Grab Joystick? */
	// Only if it isn't grabbed by someone else
	if (a_Profile->AutoGrabJoy > 0)
		if (!D_JoyToPort(a_Profile->AutoGrabJoy))
		{
			g_Splits[a_ScreenID].JoyBound = true;
			g_Splits[a_ScreenID].JoyID = a_Profile->AutoGrabJoy;
		}
	
	/* Send information to server to change colors and stuff */
	D_XNetPlayerPref(XPlay, false, DXPP_ACCOUNTNAME, a_Profile->AccountName);
	D_XNetPlayerPref(XPlay, false, DXPP_PROFILEUUID, a_Profile->UUID);
	D_XNetPlayerPref(XPlay, false, DXPP_DISPLAYNAME, a_Profile->DisplayName);
	D_XNetPlayerPref(XPlay, false, DXPP_SKINCOLOR, a_Profile->Color);
	D_XNetPlayerPref(XPlay, false, DXPP_VTEAM, a_Profile->VTeam);
	D_XNetPlayerPref(XPlay, false, DXPP_HEXENCLASS, a_Profile->HexenClass);
	D_XNetPlayerPref(XPlay, false, DXPP_COUNTEROP, a_Profile->CounterOp);
}

/* D_XNetTryJoin() -- Player attempt to join game */
void D_XNetTryJoin(D_XPlayer_t* const a_Player)
{
	const B_BotTemplate_t* BotTemplate;
	D_ProfileEx_t* Profile;
	ticcmd_t* Placement;
	void* Wp;
	int32_t i;
	uint32_t Flags;
	D_BS_t* RelBS;
	I_HostAddress_t* AddrP;
	
	/* Already playing? */
	if (a_Player->Player)
		return;
	
	/* Already tried to join? */
	// Prevents over join abuse, or when someone goes to join during a save
	if (a_Player->TriedToJoin)
	{
		// Join has not yet expired
		if (gametic < a_Player->JoinExpire)
			return;
		
		// Clear
		a_Player->TriedToJoin = false;
		a_Player->JoinExpire = 0;
	}
	
	// Set tried to join
	a_Player->TriedToJoin = true;
	a_Player->JoinExpire = gametic + TICRATE;
	
	/* If we are the server and this is ourself, join ourself */
	if (D_XNetIsServer())
	{
		// Check Flags
		Flags = 0;
		
		if (a_Player->CounterOp)
			Flags |= DTCJF_MONSTERTEAM;
		
		// If non-local, check settings first
		if (!(a_Player->Flags & DXPF_LOCAL))
		{
		}
		
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XJOINPLAYER, c_TCDataSize[DTCT_XJOINPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Is a bot (must be local)
			if ((a_Player->Flags & (DXPF_LOCAL | DXPF_BOT)) == (DXPF_LOCAL | DXPF_BOT))
			{
				// Get Template
				BotTemplate = B_BotGetTemplateDataPtr(a_Player->BotData);
				
				// Write to network
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ClProcessID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->HostID);
				LittleWriteUInt32((uint32_t**)&Wp, DTCJF_ISBOT | Flags);
				WriteUInt8((uint8_t**)&Wp, BotTemplate->SkinColor);
				WriteUInt8((uint8_t**)&Wp, a_Player->VTeam);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, BotTemplate->DisplayName[i]);
					
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, BotTemplate->HexenClass[i]);
			}
			
			// Normal Player
			else
			{
				// Get player
				Profile = a_Player->Profile;
				
				// Write to network
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ClProcessID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->HostID);
				LittleWriteUInt32((uint32_t**)&Wp, Flags);
				
				if ((a_Player->Flags & DXPF_LOCAL) && Profile)
					WriteUInt8((uint8_t**)&Wp, Profile->Color);
				else
					WriteUInt8((uint8_t**)&Wp, a_Player->Color);
				
				WriteUInt8((uint8_t**)&Wp, a_Player->VTeam);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				
				// Display name of player (to others)
				if ((a_Player->Flags & DXPF_LOCAL) && Profile)
					for (i = 0; i < MAXPLAYERNAME; i++)
						WriteUInt8((uint8_t**)&Wp, Profile->DisplayName[i]);
				else
					for (i = 0; i < MAXPLAYERNAME; i++)
						WriteUInt8((uint8_t**)&Wp, (a_Player->DisplayName[0] ? a_Player->DisplayName[i] : "Unnamed Doomer"));
				
				// Hexen class to choose
				if ((a_Player->Flags & DXPF_LOCAL) && Profile)
					for (i = 0; i < MAXPLAYERNAME; i++)
						WriteUInt8((uint8_t**)&Wp, Profile->HexenClass[i]);
				else
					for (i = 0; i < MAXPLAYERNAME; i++)
						WriteUInt8((uint8_t**)&Wp, (a_Player->HexenClass[0] ? a_Player->HexenClass[i] : 0));
			}
		}
	}
	
	/* Otherwise, we must ask the server */
	else
	{
		// Obtain route to server
		RelBS = D_XBRouteToServer(NULL, &AddrP);
		
		// Get player
		Profile = a_Player->Profile;
		
		if (RelBS)
		{
			// Base block data
			D_BSBaseBlock(RelBS, "TRYJ");
			
			// Write required data
			D_BSwu32(RelBS, a_Player->ID);
			
			D_BSws(RelBS, ((Profile && Profile->DisplayName) ? Profile->DisplayName : a_Player->DisplayName));
			D_BSws(RelBS, ((Profile && Profile->HexenClass) ? Profile->HexenClass : a_Player->HexenClass));
			D_BSwu32(RelBS, (Profile ? Profile->Color : a_Player->Color));
			D_BSwu32(RelBS, (Profile ? Profile->VTeam : a_Player->VTeam));
			
			// Send
			D_BSRecordNetBlock(RelBS, AddrP);
		}
	}
}

/* D_XNetCreatePlayer() -- Create player joined in game */
void D_XNetCreatePlayer(D_XJoinPlayerData_t* const a_JoinData)
{
	int i, j, k;
	D_XPlayer_t* XPlay;
	player_t* Player;
	
	/* Check */
	if (!a_JoinData)
		return;
	
	/* Check player limits */
	k = -1;
	for (j = i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			j++;
		else
			if (k < 0)
				k = i;
	
	// Too many players?
	if (j >= MAXPLAYERS)
		return;
	
	/* Change variables */
	// Set multiplayer mode, if player already exists
	if (j > 0 && !P_XGSVal(PGS_COMULTIPLAYER))
	{
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
		
		// Make multiplayer stuff spawn too
		P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
	}
	
	/* Find XPlayer */
	XPlay = D_XNetPlayerByID(a_JoinData->ID);
	
	// Invalid? Oops!
	if (!XPlay)
		return;
	
	/* Setup XPlayer */
	XPlay->InGameID = k;
	XPlay->ClProcessID = a_JoinData->ProcessID;
	XPlay->TriedToJoin = false;	// can join again?
	
	/* Setup Game Player */
	XPlay->Player = Player = G_AddPlayer(k);
	
	Player->XPlayer = XPlay;
	Player->skincolor = a_JoinData->Color;
	
	P_ChangePlVTeam(Player, a_JoinData->CTFTeam);
	P_ChangeCounterOp(Player, !!(a_JoinData->Flags & DTCJF_MONSTERTEAM));
	
	if (a_JoinData->DisplayName[0])
		D_NetSetPlayerName(k, a_JoinData->DisplayName);
	else
		D_NetSetPlayerName(k, XPlay->AccountName);
	
	/* Setup Screen */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
			if (g_Splits[i].XPlayer == XPlay)
			{
				g_Splits[i].Active = true;
				g_Splits[i].Waiting = false;
				g_Splits[i].Console = g_Splits[i].Display = k;
				localaiming[i] = 0;
				break;
			}
	
	/* Print Message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_PLAYERJOINED, "%s\n",
			D_NCSGetPlayerName(k)
		);
	
	/* Update Scores */
	P_UpdateScores();
	
	/* Spawn player into game */
	// Control monster, initially
	if (P_GMIsCounter() && Player->CounterOpPlayer)
		P_ControlNewMonster(Player);
	
	// Spawn them normally
	else
		G_DoReborn(Player - players);
}

/* D_XNetSetServerName() -- Server changes name */
void D_XNetSetServerName(const char* const a_NewName)
{
	/* Check */
	if (!a_NewName)
		return;
	
	/* Print */
	CONL_OutputUT(CT_NETWORK, DSTR_DNETC_SERVERCHANGENAME, "%s\n", a_NewName);
}

/* D_XNetGetPlayerName() -- Get name of player */
const char* D_XNetGetPlayerName(D_XPlayer_t* const a_Player)
{
	/* Check */
	if (!a_Player)
		return NULL;
	
	/* Return correct name */
		// Name of them inside
	if (a_Player->InGameID >= 0 && a_Player->InGameID <= MAXPLAYERS && player_names[a_Player->InGameID][0])
		return player_names[a_Player->InGameID];
	
		// Display name as spec
	else if (a_Player->DisplayName[0])
		return a_Player->DisplayName;
	
		// Use account name if there is no display name
	else
		return a_Player->AccountName;
}

/* D_XNetPlayerPref() -- Change preference of player */
void D_XNetPlayerPref(D_XPlayer_t* const a_Player, const bool_t a_FromTic, const D_XPlayerPref_t a_Pref, const intptr_t a_Value)
{
#define BUFSIZE (MAXPLAYERNAME + MAXPLAYERNAME + 1)
	char Buf[BUFSIZE];
	bool_t StrValue;
	D_ProfileEx_t* Prof;
	int32_t i, ModVal;
	
	ticcmd_t* Place;
	uint8_t* Wp;
	
	D_BS_t* BS;
	I_HostAddress_t* AddrP;
	
	/* Check */
	if (!a_Player)
		return;
	
	/* Check argument type */
	StrValue = false;
	
	switch (a_Pref)
	{
			// Integer
		case DXPP_SKINCOLOR:
		case DXPP_VTEAM:
		case DXPP_COUNTEROP:
			StrValue = false;
			ModVal = a_Value;
			break;
			
			// String
		case DXPP_DISPLAYNAME:
		case DXPP_HEXENCLASS:
		case DXPP_PROFILEUUID:
		case DXPP_ACCOUNTNAME:
			// Safe String
			memset(Buf, 0, sizeof(Buf));
			for (i = 0; i < MAXPLAYERNAME + MAXPLAYERNAME; i++)
			{
				Buf[i] = ((const char*)a_Value)[i];
				
				if (!Buf[i])
					break;
			}
			
			StrValue = true;
			break;
		
			// Unknown
		default:
			return;
	}
	
	/* From Running Tics (server changed it) */
	if (a_FromTic)
	{
		// Which preference?
		switch (a_Pref)
		{
				// Skin Color
			case DXPP_SKINCOLOR:
				a_Player->Color = ModVal;
				
				// Change in game color
				if (a_Player->Player)
				{
					a_Player->Player->skincolor = ModVal % MAXSKINCOLORS;
					
					// Need to change translation
					if (a_Player->Player->mo)
					{
						a_Player->Player->mo->flags &= ~MF_TRANSLATION;
						a_Player->Player->mo->flags |= (a_Player->Player->skincolor) << MF_TRANSSHIFT;
					}
				}
				break;
			
				// Virtual Team
			case DXPP_VTEAM:
				a_Player->VTeam = ModVal;
				
				// Change in game team
				if (a_Player->Player)
					P_ChangePlVTeam(a_Player->Player, a_Player->VTeam);
				break;
			
				// Display Name
			case DXPP_DISPLAYNAME:
				// Change their in game name if they are playing
				if (a_Player->Player)
					D_NetSetPlayerName(a_Player->Player - players, Buf);
				
				// Otherwise set their name (no need to broadcast)
				else
					strncpy(a_Player->DisplayName, Buf, MAXPLAYERNAME - 1);
					
				// Update scores
				P_UpdateScores();
				break;
			
				// Account Name
			case DXPP_ACCOUNTNAME:
				strncpy(a_Player->AccountName, Buf, MAXPLAYERNAME - 1);
				
				// Update scores
				P_UpdateScores();
				break;
				
				// Profile UUID
			case DXPP_PROFILEUUID:
				// If local, try and relate change
				if (a_Player->Flags & DXPF_LOCAL)
				{
					Prof = D_FindProfileEx(Buf);
					
					// Found, so change it
					if (Prof)
					{
						// Use this instead
						strncpy(a_Player->ProfileUUID, Prof->UUID, MAXPLAYERNAME - 1);
						
						// Set profile references to that (so key binds take effect)
						a_Player->Profile = Prof;
						if (a_Player->Player)
							a_Player->Player->ProfileEx = Prof;
						if (a_Player->ScreenID >= 0 && a_Player->ScreenID < MAXSPLITSCREEN)
							if (g_Splits[a_Player->ScreenID].XPlayer == a_Player)
								g_Splits[a_Player->ScreenID].Profile = Prof;
					}
				}
				
				// Blind copy for remote/demo
				else
					strncpy(a_Player->ProfileUUID, Buf, MAXPLAYERNAME - 1);
				break;
		
				// Hexen Class
			case DXPP_HEXENCLASS:
				strncpy(a_Player->HexenClass, Buf, MAXPLAYERNAME - 1);
				break;
				
				// Counter-Op Player
			case DXPP_COUNTEROP:
				a_Player->CounterOp = !!ModVal;
				
				if (a_Player->Player)
					P_ChangeCounterOp(a_Player->Player, a_Player->CounterOp);
				break;
		}
	}
	
	/* Otherwise, place change or request server to change it */
	else
	{
		// If server, elicit change into tic structure
			// Inject into tic stream to inform clients of changes
		if (D_XNetIsServer())
		{
			// If a string, use long version
			if (StrValue)
			{
				if (D_XNetGlobalTic(DTCT_XPLAYERPREFSTR, &Wp))
				{
					LittleWriteUInt32(&Wp, a_Player->ID);
					LittleWriteUInt16(&Wp, a_Pref);
					
					for (i = 0; i < MAXPLAYERNAME + MAXPLAYERNAME; i++)
						WriteUInt8(&Wp, Buf[i]);
				}
			}
			
			// Otherwise, if an int, just use one
			else
			{
				if (D_XNetGlobalTic(DTCT_XPLAYERPREFINT, &Wp))
				{
					LittleWriteUInt32(&Wp, a_Player->ID);
					LittleWriteUInt16(&Wp, a_Pref);
					LittleWriteInt32(&Wp, ModVal);
				}
			}
		}
		
		// Otherwise, if client, request change
		else
		{
			BS = D_XBRouteToServer(NULL, &AddrP);
					
			if (BS)
			{
				D_BSBaseBlock(BS, "PREF");
				
				D_BSwu32(BS, a_Player->ID);
				D_BSwu16(BS, a_Pref);
				
				// String
				if (StrValue)
				{
					D_BSwu8(BS, 1);
					D_BSws(BS, Buf);
				}
				
				// Int
				else
				{
					D_BSwu8(BS, 0);
					D_BSwi32(BS, ModVal);
				}
				
				D_BSRecordNetBlock(BS, AddrP);
			}
		}
	}
#undef BUFSIZE
}

/* D_XNetCalcPing() -- Calculates Player Ping */
uint16_t D_XNetCalcPing(D_XPlayer_t* const a_Player)
{
	D_XEndPoint_t* EP;
	uint16_t PVal, SVal;
	int32_t TicDiff;
	uint32_t NowTime;
	
	/* Invalid Player? */
	if (!a_Player)
		return 0;
	
	/* Calculate */
	EP = a_Player->Socket.EndPoint;
	PVal = SVal = 0;
	TicDiff = 0;
	
	// No endpoint?
	if (!EP)
		return 0;
	
	// Programtic difference
	NowTime = I_GetTimeMS();
	//NowTime = g_ProgramTic * TICSPERMS;
	
	TicDiff = NowTime - EP->PongMS;
	
	// Program value
	if (TicDiff < 0)
		PVal = 0;
	else if (TicDiff < 65535)
		PVal = TicDiff;
	else
		PVal = TICPINGAMOUNTMASK;
	
	/* Limit */
	if (SVal > 3)
		SVal = 3;
	if (PVal >= TICPINGAMOUNTMASK)
		PVal = TICPINGAMOUNTMASK - 1;
	
	/* Return */
	a_Player->Ping = PVal | (SVal << TICPINGSIGNALSHIFT);
	return a_Player->Ping;
}

/* D_XNetMultiTics() -- Read/Write Tics all in one */
void D_XNetMultiTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	int32_t i;
	D_XPlayer_t* XPlay;
	D_XNetTicBuf_t* Buf;
	
	/* We are the server */
	if (D_XNetIsServer())
	{
		// Write commands to clients
		if (a_Write)
		{
			D_XNetPlaceTicCmd(gametic, a_Player, a_TicCmd);
		}
	
		// Read commands
		else
		{
			// Clear tic command first
			memset(a_TicCmd, 0, sizeof(a_TicCmd));
			
			// Global
			if (a_Player < 0)
			{
				// Something is in the buffer
				if (l_GlobalAt >= 0)
				{
					// Move the first item inside
					memmove(a_TicCmd, &l_GlobalBuf[0], sizeof(*a_TicCmd));
					
					// Move everything down
					memmove(&l_GlobalBuf[0], &l_GlobalBuf[1], sizeof(ticcmd_t) * (MAXGLOBALBUFSIZE - 1));
					memset(&l_GlobalBuf[MAXGLOBALBUFSIZE - 1], 0, sizeof(l_GlobalBuf[MAXGLOBALBUFSIZE - 1]));
					l_GlobalAt--;
				}
				
				// Nothing
				else
					memset(a_TicCmd, 0, sizeof(*a_TicCmd));
			}
			
			// Individual player
			else
			{
				// Get player
				XPlay = players[a_Player].XPlayer;
				
				// No player?
				if (!XPlay)
					return;
				
				// No commands available in queue? (lag)
				if (!XPlay->LocalAt && XPlay->GotBackup)
				{
					// Ouch =(
					memmove(a_TicCmd, &XPlay->BackupTicCmd, sizeof(ticcmd_t));
					
					// Set as missing commands
					XPlay->StatusBits |= DXPSB_MISSINGTICS;
					
					// Set local angles (probably lost)
					a_TicCmd->Std.angleturn = XPlay->LookAngle;
					a_TicCmd->Std.aiming = XPlay->AimAngle;
				}
				
				// Merge local tic commands
				else if (XPlay->LocalAt)
				{
					D_XNetMergeTics(a_TicCmd, XPlay->LocalBuf, XPlay->LocalAt);
					XPlay->LocalAt = 0;
					memset(XPlay->LocalBuf, 0, sizeof(XPlay->LocalBuf));
				
					// If player is local, modify angle set, and spectating
					if (XPlay->Flags & DXPF_LOCAL)
						if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
						{
							// Absolute Angles
							if (P_XGSVal(PGS_COABSOLUTEANGLE))
							{
								localangle[XPlay->ScreenID] += a_TicCmd->Std.BaseAngleTurn << 16;
								a_TicCmd->Std.angleturn = localangle[XPlay->ScreenID] >> 16;
							}
			
							// Doom Angles
							else
								a_TicCmd->Std.angleturn = a_TicCmd->Std.BaseAngleTurn;
						
							// Aiming Angle
							if (a_TicCmd->Std.buttons & BT_RESETAIM)
								localaiming[XPlay->ScreenID] = 0;
							else
							{
								// Panning Look
								if (a_TicCmd->Std.buttons & BT_PANLOOK)
									localaiming[XPlay->ScreenID] = a_TicCmd->Std.BaseAiming << 16;
								
								// Standard Look
								else
									localaiming[XPlay->ScreenID] += a_TicCmd->Std.BaseAiming << 16;
							}
							
							// Clip aiming pitch to not exceed bounds
							a_TicCmd->Std.aiming = G_ClipAimingPitch(&localaiming[XPlay->ScreenID]);
						}
					
					// Set angles from tic command
					XPlay->LookAngle = a_TicCmd->Std.angleturn;
					XPlay->AimAngle = a_TicCmd->Std.aiming;
					
					// Copy command to backup command
					memmove(&XPlay->BackupTicCmd, a_TicCmd, sizeof(*a_TicCmd));
					XPlay->GotBackup = true;
					
					// Clear missing commands
					XPlay->StatusBits &= ~DXPSB_MISSINGTICS;
				}
				
				// No backup tic, this really sucks
				else if (!XPlay->GotBackup)
					XPlay->StatusBits |= DXPSB_MISSINGTICS;
				
				// Always move over status bits
				a_TicCmd->Std.StatFlags = XPlay->StatusBits;
				a_TicCmd->Ctrl.Ping = XPlay->Ping;
			}
		}
	}
	
	/* We are the client */
	else
	{
		// Writing
		if (a_Write)
		{
		}
		
		// Reading
		else
		{
			// Get tic that we want
			Buf = D_XNetBufForTic(gametic, false);
		
			// That tic better be found!
			if (Buf)
			{
				// Copy specific player
				memmove(a_TicCmd, &Buf->Tics[(a_Player < 0 ? MAXPLAYERS : a_Player)], sizeof(*a_TicCmd));
			}
			
			// If not, disconnect
			else
			{
				CONL_OutputUT(CT_NETWORK, DSTR_DNETC_INVALIDTIC, "%u\n", (unsigned int)gametic);
				D_XNetDisconnect(false);
			}
		}
	}
}

/* D_XNetTicsToRun() -- Amount of tics to run */
tic_t D_XNetTicsToRun(void)
{
#define BUFSIZE 48
	char ErrBuf[BUFSIZE];
	tic_t Diff, ThisTic;
	bool_t Lagging, NonLocal;
	int32_t i;
	D_XPlayer_t* XPlay, *Host;
	static tic_t LastSpecTic;
	D_XNetTicBuf_t* Buf;
	tic_t OldestTic, BehindCount;
	
	bool_t ItWorked;
	static tic_t ReqThresh;
	D_BS_t* BS;
	I_HostAddress_t* AddrP;
	
	/* Initialize */
	// Get current tic
	ThisTic = I_GetTime();
	
	// The oldest tic visible
	OldestTic = ((tic_t)-1);
	
	/* Playing a demo back */
	if (demoplayback)
	{
		// Timing Demo
		if (singletics)
			return 1;
		
		// Get time difference
		Diff = ThisTic - l_XNLastPTic;
		l_XNLastPTic = ThisTic;
		
		// Frozen Demo
		if (Diff)
			if (g_DemoFreezeTics)
			{
				P_SpecTicker();	// Tic fake camera for effects
				g_DemoFreezeTics--;
				return 0;
			}
		
		// If the difference is too great, cap it
			// i.e. debug break, moving window, etc. doesn't cause the game to
			// catchup for all the time that was lost
		if (Diff > l_SVMaxDemoCatchup.Value->Int)
			return l_SVMaxDemoCatchup.Value->Int + 1;
		return Diff;
	}
	
	/* We are the server */
	else if (D_XNetIsServer())
	{
		// Timing Server
		if (singletics)
			return 1;
		
		// See if everyone is not lagging behind
			// If not, freeze the game until everyone catches up some
		Lagging = NonLocal = false;
		
		for (i = 0; i < g_NumXPlays; i++)
		{
			// Get current
			XPlay = g_XPlays[i];
			
			// Not here?
			if (!XPlay)
				continue;
			
			// Ignore defunct players
			if (XPlay->Flags & DXPF_DEFUNCT)
				continue;
			
			// Ignore Bots
			if (XPlay->Flags & DXPF_BOT)
				continue;
			
			// Clear not cause of lag
			XPlay->StatusBits &= ~DXPSB_CAUSEOFLAG;
			
			// Non-Local
			if ((XPlay->Flags & DXPF_LOCAL) == 0)
			{
				// One savegame transmit per host
				NonLocal = true;
				Host = D_XNetPlayerByXPlayerHost(XPlay);
				
				// Time behind
				if (Host->LastRanTic >= gametic)
					BehindCount	= 0;
				else
					BehindCount = gametic - Host->LastRanTic;
				
				// Reasons for lag:
					// Save game not transmitted?
				if ((!Host->TransSave) ||
					(BehindCount >= l_SVMaxCatchup.Value->Int))
				{
					Lagging = true;
					XPlay->StatusBits |= DXPSB_CAUSEOFLAG;
				}
				
				// Not lagging
				else
					XPlay->StatusBits &= ~DXPSB_CAUSEOFLAG;
			}
			
			// Check lag stop/start
			if (XPlay->StatusBits & DXPSB_CAUSEOFLAG)
			{
				// Set initial timer
				if (!XPlay->LagStart)
				{
					XPlay->LagStart = g_ProgramTic;
					
					// Threshold prevents lag abuse
					if (XPlay->LagThreshold)
						XPlay->LagKill = XPlay->LagThreshold;
					
					else
						XPlay->LagKill = XPlay->LagStart + (l_SVReadyBy.Value->Int * TICRATE);
				}
				
				// If lag timer is past the threshold, kick
				else if (g_ProgramTic >= XPlay->LagKill)
				{
					snprintf(ErrBuf, BUFSIZE - 1, "Timed out. ( @ %u - %u)", (unsigned)Host->LastRanTic, (unsigned)gametic);
					D_XNetKickPlayer(XPlay, ErrBuf, false);
				}
			}
			
			// If not lagging, clear indicators
			else
			{
				// If we were lagging, then set the threshold
				if (XPlay->LagStart)
				{
					XPlay->LagThreshExpire = g_ProgramTic + (l_SVLagThreshExpire.Value->Int * TICRATE);
					XPlay->LagThreshold = XPlay->LagKill;
					XPlay->LagKill = 0;
				}
				
				XPlay->LagStart = 0;
				XPlay->LagKill = 0;
				
				// Threshold expires?
				if (g_ProgramTic >= XPlay->LagThreshExpire)
				{
					XPlay->LagThreshExpire = 0;
					XPlay->LagThreshold = 0;
				}
			}
		}
		
		// If dedicated, always NonLocal
		if (l_IsDedicated)
			NonLocal = true;
		
		// See if we are acting as a internet server
		if (D_XBHasConnection())
			NonLocal = true;
		
		// No other clients in game?
		if (!NonLocal)
			// If a menu or console is open
			if (!demoplayback && (
					M_SMFreezeGame()
					|| (g_IgnoreWipeTics > 0)
					|| (M_ExAllUIActive() && g_ResumeMenu <= 0)
					|| (l_CONPauseGame.Value->Int && CONL_IsActive())))
				Lagging = true;
		
		// Force Lagging?
		if (l_ForceLag)
		{
			Lagging = true;
			l_ForceLag = false;
		}
		
		// Clients need to catchup
		if (Lagging)
		{
			// Move join window ahead
			if (D_XNetIsServer())
				D_XNetPushJW();
			
			// Tic spectators, so you can move when lagging
			if (g_ProgramTic != LastSpecTic)
			{
				P_SpecTicker();
				LastSpecTic = g_ProgramTic;
			}
			
			l_XNLastPTic = ThisTic;
			return 0;
		}
		
		// Wipe old gametics we don't care about
		if (gametic > l_SVMaxCatchup.Value->Int + 1)
			D_XNetWipeBefores(gametic - l_SVMaxCatchup.Value->Int);
			
		// Get time difference
		Diff = ThisTic - l_XNLastPTic;
		l_XNLastPTic = ThisTic;
		
		// If the difference is too great, cap it
			// i.e. debug break, moving window, etc. doesn't cause the game to
			// catchup for all the time that was lost
		if (Diff > l_SVMaxCatchup.Value->Int)
			return l_SVMaxCatchup.Value->Int + 1;
		return Diff;
	}
	
	/* We are the client */
	else
	{
		// If time smoothing is enabled...
			// If running to far behind, execute more tics
			// If running within threshold, execute tic at certain time
			// so that the client game is not jerky and is consistent.
		if (false)
		{
		}
		
		// Otherwise, return the amount of tics that are in queue
			// This is the as fast as possible spec, super fast
			
		else
		{
			// Count tics in buffer
			Diff = 0;
			while ((Buf = D_XNetBufForTic(gametic + Diff, false)))
				Diff++;
			
			// Remove all tics before gametic
			if (gametic > 1)
				D_XNetWipeBefores(gametic - 1);
			
			// Return available count
			if (Diff > 0)
			{
				ReqThresh = 0;
				return Diff;
			}
			
			// Otherwise, ask server for those missing tics
			else
			{
				// Request Threshold once
				if (!ReqThresh)
					// Set threshold
					ReqThresh = g_ProgramTic + l_CLReqTicDelay.Value->Int + 1;
				
				// Wait until obtained threshold
				else if (g_ProgramTic >= ReqThresh)
				{
					// Request tic from server
					BS = D_XBRouteToServer(NULL, &AddrP);
					
					if (BS)
					{
						D_BSBaseBlock(BS, "TREQ");
						D_BSwcu64(BS, gametic);
						D_BSRecordNetBlock(BS, AddrP);
					}
					
					// Remove threshold to wait again
					ReqThresh = 0;
				}
			}
		}
	}
	
	/* Fell through? */
	return 0;
#undef BUFSIZE
}

/* D_XNetForceLag() -- Forces lag to the server */
void D_XNetForceLag(void)
{
	l_ForceLag = true;
}

/* DS_PBABOpt_t -- Options for bot addition */
typedef struct DS_PBABOpt_s
{
	B_BotTemplate_t* BotTemp;
	bool_t CounterOp;
	int32_t VTeam;
} DS_PBABOpt_t;

/* DS_PBAddBot() -- Adds a bot */
static void DS_PBAddBot(D_XPlayer_t* const a_Player, void* const a_Data)
{
	DS_PBABOpt_t* Opts = a_Data;
	
	/* Set Initial Data */
	// Flags
	a_Player->Flags |= DXPF_BOT | DXPF_NOLOGIN | DXPF_LOCAL;
	
	/* Init Bot */
	a_Player->BotData = B_InitBot(Opts->BotTemp);
	a_Player->CounterOp = Opts->CounterOp;
	a_Player->VTeam = Opts->VTeam;
	
	// If template exists
	if (Opts->BotTemp)
	{
		strncpy(a_Player->AccountName, Opts->BotTemp->AccountName, MAXPLAYERNAME);
		strncpy(a_Player->DisplayName, Opts->BotTemp->DisplayName, MAXPLAYERNAME);
	}
	
	// Missing Stuff?
		// Account Name
	if (!strlen(a_Player->AccountName))
		strncpy(a_Player->AccountName, "bot", MAXPLAYERNAME);
		
	// Correct account name
	D_ProfFixAccountName(a_Player->AccountName);
		
		// Display Name
	if (!strlen(a_Player->DisplayName))
		strncpy(a_Player->DisplayName, a_Player->AccountName, MAXPLAYERNAME);
	
	/* Cheat and say we got bots now */
	g_GotBots = true;
}


/* DS_XNetCon() -- Command */
static int DS_XNetCon(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	D_XPlayer_t* XPlay;
	int32_t i, j;
	bool_t Flag;
	struct IP_Conn_s* Conn;
	I_HostAddress_t Addr;
	DS_PBABOpt_t BotOpt;
	const char* Name;
	
	/* Not enough args? */
	if (a_ArgC < 2)
		return 1;
	
	/* Listing Players */
	if (strcasecmp(a_ArgV[1], "list") == 0)
	{
		// Print Header
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
				"%3s%-8s%s\n",
				"Cl#",
				"ID/Host#",
				"Name/Account^Server"
			);
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
				"%3s%-8s%s\n",
				"---",
				"--------",
				"-------------------------"
			);
		
		// Go through list
		for (i = 0; i < g_NumXPlays; i++)
		{
			XPlay = g_XPlays[i];
		
			// No player?
			if (!XPlay)
				continue;
		
			// List details
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
					"%3u%-08x%s\n",
					i + 1,
					XPlay->ID,
					XPlay->DisplayName
				);
		
			// Account w/ Server
			snprintf(Buf, BUFSIZE, "%s^%s", XPlay->AccountName, XPlay->AccountServer);
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
					"%3s%-08x%s\n",
					"",
					XPlay->HostID,
					Buf
				);
		}
	}
	
	/* Kicking Player */
	else if (strcasecmp(a_ArgV[1], "kick") == 0)
	{
		// Not server?
		if (!D_XNetIsServer())
			return 1;
		
		// Not enough args?
		if (a_ArgC < 3)
			return 1;
		
		// Find player
		XPlay = D_XNetPlayerByString(a_ArgV[2]);
		
		// Not found?
		if (!XPlay)
			return 1;
		
		// Kick player
		D_XNetKickPlayer(XPlay, (a_ArgC > 3 ? a_ArgV[4] : DS_GetString(DSTR_NET_KICKED)), false);
		return 0;
	}
	
	/* Adding Bot */
	else if (strcasecmp(a_ArgV[1], "addbot") == 0)
	{
		// Not server?
		if (!D_XNetIsServer())
			return 1;
		
		// Clear
		memset(&BotOpt, 0, sizeof(BotOpt));
		
		// Use Bot Template?
		if (a_ArgC >= 3)
			BotOpt.BotTemp = B_GHOST_FindTemplate(a_ArgV[2]);
		
		// Random?
		if (!BotOpt.BotTemp)
			BotOpt.BotTemp = B_GHOST_RandomTemplate();
		
		// Put Bot on random team
		BotOpt.VTeam = M_Random() % MAXSKINCOLORS;
		
		// Extra arguments?
		for (i = 3; i < a_ArgC; i++)
		{
			// Counter-Op Player?
			if (!strcasecmp(a_ArgV[i], "counter"))
				BotOpt.CounterOp = true;
			
			// Matches Team Name
			if (!strncasecmp(a_ArgV[i], "team", 4))
				for (j = 0; j < MAXSKINCOLORS; j++)
				{
					Name = NULL;
					P_GetTeamInfo(j, NULL, &Name);
					
					if (Name)
						if (!strcasecmp(a_ArgV[i] + 4, Name))
						{
							BotOpt.VTeam = j;
							break;
						}
				}
		}
		
		// Add Player
		D_XNetAddPlayer(DS_PBAddBot, &BotOpt, false);
		
		// Success?
		return 0;
	}
	
	/* Bind Connection */
	else if (strcasecmp(a_ArgV[1], "bind") == 0)
	{
#if 0
		// Requires two arguments
		if (a_ArgC < 4)
			return 1;
		
		// First argument is a true/false word
		Flag = INFO_BoolFromString(a_ArgV[2]);
		
		// Possibly create a socket to bind
		Conn = IP_Create(a_ArgV[3], (Flag ? IPF_INPUT : 0));
		
		// Failed to create
		if (!Conn)
			return 0;
		
		// Bind it
		return !D_XNetBindConn(Conn);
#endif
		return 0;
	}
	
	/* Connect */
	else if (strcasecmp(a_ArgV[1], "connect") == 0)
	{
#if 0
		// Requires one argument
		if (a_ArgC < 3)
			return 1;
		
		// Start connecting
		D_XNetConnect(a_ArgV[2]);
#endif
		return 0;
	}
	
	/* Start Server */
	else if (strcasecmp(a_ArgV[1], "create") == 0)
	{
		// This is easy
		D_XNetMakeServer(false, NULL, 0, false);
		return 0;
	}
	
	/* Disconnect */
	else if (strcasecmp(a_ArgV[1], "disconnect") == 0)
	{
		// Start connecting
		D_XNetDisconnect(false);
		return 0;
	}
	
	/* Resolve hostname */
	else if (strcasecmp(a_ArgV[1], "resolve") == 0)
	{
		// Requires one argument
		if (a_ArgC < 3)
			return 1;
			
		memset(&Addr, 0, sizeof(Addr));
		
		// Use net resolve
		if (!I_NetNameToHost(NULL, &Addr, a_ArgV[2]))
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_HOSTNORESOLVE, "%s\n", a_ArgV[2]);
		else
			if (Addr.IPvX == INIPVN_IPV6)
				CONL_OutputUT(CT_NETWORK, DSTR_DNETC_HOSTRESSIX, "%s%08x%08x%04x%04x%04x%04x%u\n",
						a_ArgV[2],
						Addr.Host.v6.Addr.u[0],
						Addr.Host.v6.Addr.u[1],
						Addr.Host.v6.Addr.s[4],
						Addr.Host.v6.Addr.s[5],
						Addr.Host.v6.Addr.s[6],
						Addr.Host.v6.Addr.s[7],
						Addr.Host.v6.Scope
					);
			else
				CONL_OutputUT(CT_NETWORK, DSTR_DNETC_HOSTRESFOUR, "%s%i%i%i%i\n",
						a_ArgV[2],
						Addr.Host.v4.b[0],
						Addr.Host.v4.b[1],
						Addr.Host.v4.b[2],
						Addr.Host.v4.b[3]
					);
		
		return 0;
	}
	
	/* Failure */
	return 1;
#undef BUFSIZE
}

int DS_XNetRootCon(const uint32_t a_ArgC, const char** const a_ArgV);

/* DS_XNetCmdConnect() -- Connect to server */
static int DS_XNetCmdConnect(const uint32_t a_ArgC, const char** const a_ArgV)
{
	const char* Host, *Pass;
	I_HostAddress_t Addr;
	uint32_t GameID;
	
	/* Requires at least 2 arguments */
	if (a_ArgC < 2)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_CONNECTUSAGE, "%s\n", a_ArgV[0]);
		return 1;
	}
	
	/* Extract host and password */
	Host = a_ArgV[1];
	Pass = NULL;
	
	if (a_ArgC >= 3)
		Pass = a_ArgV[2];
	
	/* Resolve hostname for connect */
	if (!D_XNetHostnameToAddrGID(Host, &Addr, &GameID))
		return 1;
	
	// Disconnect
	D_XNetDisconnect(false);
	
	// Now connect
	D_XNetConnect(&Addr, GameID, false);
	
	/* Success? */
	return 0;
}

/* DS_XNetCmdAddPlayer() -- Adds local player */
static int DS_XNetCmdAddPlayer(const uint32_t a_ArgC, const char** const a_ArgV)
{
	const char* Profile;
	int32_t Screen, Joy;
	bool_t DoJoy;
	
	/* Need at least 1 arg */
	if (a_ArgC < 2)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_ADDPLAYERUSAGE, "%s\n", a_ArgV[0]);
		return 1;
	}
	
	/* Read Arguments */
	Screen = Joy = 0;
	DoJoy = false;
	Profile = a_ArgV[1];
	
	if (a_ArgC >= 3)
		Screen = C_strtoi32(a_ArgV[2], NULL, 10);
	
	if (a_ArgC >= 4)
	{
		DoJoy = true;
		Joy = C_strtoi32(a_ArgV[3], NULL, 10);
	}
	
	/* Do a standard add player */
	D_NCLocalPlayerAdd(Profile, false, Joy, Screen, DoJoy);
	return 0;
}

/* D_XNetInit() -- Initializes the Extended Network Code */
void D_XNetInit(void)
{
	CONL_AddCommand("xnet", DS_XNetCon);
	CONL_AddCommand("root", DS_XNetRootCon);
	
	CONL_AddCommand("connect", DS_XNetCmdConnect);
	CONL_AddCommand("addplayer", DS_XNetCmdAddPlayer);
}

/* DS_XNetCONCTIChatLine() -- For when enter is pressed */
static bool_t DS_XNetCONCTIChatLine(struct CONCTI_Inputter_s* a_Input, const char* const a_Text)
{
	/* Handle chat string and send to server (or local) */
	D_XNetSendChat(g_Splits[a_Input->Screen].XPlayer, (g_Splits[a_Input->Screen].ChatMode == 2 || g_Splits[a_Input->Screen].ChatMode == 3), a_Text);
	
	/* Leave Chat Mode */
	// Done with it, no use
	D_XNetClearChat(a_Input->Screen);
	
	/* always keep box */
	return true;
}

/* D_XNetChatDrawer() -- Draws player chat */
void D_XNetChatDrawer(const int32_t a_Screen, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
{
	uint32_t Flags;
	
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return;
	
	/* Not chatting? */
	if (!g_Splits[a_Screen].ChatMode)
		return;
	
	/* Draw box */
	if (l_ChatBox[a_Screen])
	{	
		// Base flags
		Flags = 0;
		
		// If team chat, draw colorized!
		if (g_Splits[a_Screen].ChatMode == 2 || g_Splits[a_Screen].ChatMode == 3)
			if (!g_Splits[a_Screen].XPlayer || !g_Splits[a_Screen].XPlayer->Player)		// Spec
				Flags = VFO_COLOR(VEX_MAP_WHITE);
			else		// Team
			{
				Flags = P_GetPlayerTeam(&players[g_Splits[a_Screen].Console]);
				P_GetTeamInfo(Flags, &Flags, NULL);
				Flags = VFO_PCOL(Flags);
			}
		
		// Draw it
		CONCTI_DrawInput(l_ChatBox[a_Screen], Flags, a_X, a_H - V_FontHeight(l_ChatBox[a_Screen]->Font), a_W);
	}
}

/* D_XNetClearChat() -- Clears player chat */
void D_XNetClearChat(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return;
	
	/* Get out of chat mode */
	g_Splits[a_Screen].ChatMode = 0;
	g_Splits[a_Screen].ChatTargetID = 0;
	g_Splits[a_Screen].ChatTimeOut = g_ProgramTic + (TICRATE >> 1);
	
	/* Remove line */
	if (l_ChatBox[a_Screen])
		CONCTI_SetText(l_ChatBox[a_Screen], "");
}

/* D_XNetHandleEvent() -- Handle advanced events */
bool_t D_XNetHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t ButtonNum, LocalJoy;
	uint32_t Bit;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Clear events if not playing */
	// Only on a title screen (walk around in demos)
	if (gamestate == GS_DEMOSCREEN || (demoplayback && g_TitleScreenDemo))
	{
		memset(l_MouseMove, 0, sizeof(l_MouseMove));
		memset(l_KeyDown, 0, sizeof(l_KeyDown));
		memset(l_JoyButtons, 0, sizeof(l_JoyButtons));
		memset(l_JoyAxis, 0, sizeof(l_JoyAxis));
		return false;
	}
	
	/* Handle chatting for players */
	// Keyboard for P1 only
	if (a_Event->Type == IET_KEYBOARD)
		Bit = 1;
	
	// Synth OSK always work
	else if (a_Event->Type == IET_SYNTHOSK)
		Bit = a_Event->Data.SynthOSK.PNum + 1;
	
	// No player specified (mouse, joy, etc.)
	else
		Bit = 0;	// Non-chat stuff
	
	// Doing chat?
	if (Bit && !demoplayback)
	{
		// Bit is off by one
		Bit -= 1;
		
		// If they are chatting
		if (g_Splits[Bit].ChatMode && D_ScrSplitHasPlayer(Bit))
		{
			// Cancel chat?
			if ((a_Event->Type == IET_KEYBOARD && a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE && a_Event->Data.Keyboard.Down) || (a_Event->Type == IET_SYNTHOSK && a_Event->Data.SynthOSK.Cancel))
			{
				D_XNetClearChat(Bit);
				return true;
			}
			
			// Need to create chat box?
			if (!l_ChatBox[Bit])
			{
				l_ChatBox[Bit] = CONCTI_CreateInput(16, DS_XNetCONCTIChatLine, &l_ChatBox[Bit]);
				l_ChatBox[Bit]->Screen = Bit;
				l_ChatBox[Bit]->Font = VFONT_SMALL;
			}
			
			// Handle event for inputter
			return CONCTI_HandleEvent(l_ChatBox[Bit], a_Event);
		}
	}
	
	/* Which kind of event? */
	switch (a_Event->Type)
	{
			// Mouse
		case IET_MOUSE:
			// Add position to movement
			l_MouseMove[0] += a_Event->Data.Mouse.Move[0];
			l_MouseMove[1] += a_Event->Data.Mouse.Move[1];
			
			// Handling of buttons (with double click)
			if (a_Event->Data.Mouse.Button > 0 && a_Event->Data.Mouse.Button < 32)
			{
				// Determine bit
				ButtonNum = a_Event->Data.Mouse.Button - 1U;
				Bit = 1U << (ButtonNum);
				
				// Unpressed?
				if (!a_Event->Data.Mouse.Down)
				{
					l_MouseButtons[0] &= ~Bit;
					l_MouseButtons[1] &= ~Bit;
				}
				else
				{
					// Always set single bit
					l_MouseButtons[0] |= Bit;
					
					// Double Click?
						// TODO make this a CVAR of sorts
					if (g_ProgramTic - l_MouseLastTime[ButtonNum] < 17)
					{
						l_MouseButtons[1] |= Bit;
						l_MouseLastTime[ButtonNum] = 0;
					}
				
					// Single Click (set last time for double)
					else
						l_MouseLastTime[ButtonNum] = g_ProgramTic;
				}
			}
			return true;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			return true;
			
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
				// Not bound? Then remove anything remembered (prevents stuckness)
				if (!D_JoyToPort(LocalJoy + 1))
				{
					l_JoyButtons[LocalJoy] = 0;
					memset(&l_JoyAxis[LocalJoy], 0, sizeof(l_JoyAxis[LocalJoy]));
					break;
				}
				
				// Button Pressed Down
				if (a_Event->Data.Joystick.Button)
				{
					// Get Number
					ButtonNum = a_Event->Data.Joystick.Button;
					ButtonNum--;
					
					// Limited to 32 buttons =(
					if (ButtonNum >= 0 && ButtonNum < 32)
					{
						// Was it pressed?
						if (a_Event->Data.Joystick.Down)
							l_JoyButtons[LocalJoy] |= (1 << ButtonNum);
						else
							l_JoyButtons[LocalJoy] &= ~(1 << ButtonNum);
					}
				}
				
				// Axis Moved
				else if (a_Event->Data.Joystick.Axis)
				{
					ButtonNum = a_Event->Data.Joystick.Axis;
					ButtonNum--;
					
					if (ButtonNum >= 0 && ButtonNum < MAXJOYAXIS)
						l_JoyAxis[LocalJoy][ButtonNum] = a_Event->Data.Joystick.Value;
				}
			}
			return true;
		
			// Unknown
		default:
			break;
	}
	
	/* Un-Handled */
	return false;
}

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in PI_wep_t for ordering info
static uint8_t DS_XNetNextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	PI_wep_t** weapons;
	
	/* No player? */
	if (!player)
		return 0;
	
	/* Get current weapon info */
	weapons = player->weaponinfo;
	
	/* Get the weapon with the lowest and highest order */
	// Find first gun the player has (so order is correct)
	MostOrder = LeastOrder = 0;
	for (w = 0; w < NUMWEAPONS; w++)
		if (P_CanUseWeapon(player, w))
		{
			// Got the first available gun
			MostOrder = LeastOrder = w;
			break;
		}
	
	// Now go through
	for (w = 0; w < NUMWEAPONS; w++)
	{
		// Can't use this gun?
		if (!P_CanUseWeapon(player, w))
			continue;
		
		// Least
		if (weapons[w]->SwitchOrder < weapons[LeastOrder]->SwitchOrder)
			LeastOrder = w;
		
		// Most
		if (weapons[w]->SwitchOrder > weapons[MostOrder]->SwitchOrder)
			MostOrder = w;
	}
	
	/* Look for the current weapon in the weapon list */
	// Well that was easy
	fw = s = g = player->readyweapon;
	
	/* Constantly change the weapon */
	// Prepare variables
	Neg = (step < 0 ? true : false);
	StepsAdd = (Neg ? -1 : 1);
	StepsLeft = step * StepsAdd;
	
	// Go through the weapon list, step times
	while (StepsLeft > 0)
	{
		// Clear variables
		BestDiff = 9999999;		// The worst weapon difference ever
		BestNum = NUMWEAPONS;
		
		// Go through every weapon and find the next in the order
		for (w = 0; w < NUMWEAPONS; w++)
		{
			// Ignore the current weapon (don't want to switch back to it)
			if (w == fw)		// Otherwise BestDiff is zero!
				continue;
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, w))
				continue;
			
			// Only consider worse/better weapons?
			if ((Neg && weapons[w]->SwitchOrder > weapons[fw]->SwitchOrder) || (!Neg && weapons[w]->SwitchOrder < weapons[fw]->SwitchOrder))
				continue;
			
			// Get current diff
			ThisDiff = abs(weapons[fw]->SwitchOrder - weapons[w]->SwitchOrder);
			
			// Closer weapon?
			if (ThisDiff < BestDiff)
			{
				BestDiff = ThisDiff;
				BestNum = w;
			}
		}
		
		// Found no weapon? Then "loop" around
		if (BestNum == NUMWEAPONS)
		{
			// Switch to the highest gun if going down
			if (Neg)
				fw = MostOrder;
			
			// And if going up, go to the lowest
			else
				fw = LeastOrder;
		}
		
		// Found a weapon
		else
		{
			// Switch to this gun
			fw = BestNum;
		}
		
		// Next step
		StepsLeft--;
	}
	
	/* Return the weapon we want */
	return fw;
}

/* GAMEKEYDOWN() -- Checks if a key is down */
static bool_t GAMEKEYDOWN(D_ProfileEx_t* const a_Profile, const uint8_t a_SID, const uint8_t a_Key)
{
	static bool_t Recoursed;
	int8_t MoreDown;
	size_t i;
	uint32_t CurrentButton;
	uint32_t KeyBit, JoyBit, MouseBit, DMouseBit;
	
	/* Determine if more key is down */
	// But do not infinite loop here
	MoreDown = 0;
	if (!Recoursed)
	{
		// More Down?
		Recoursed = true;
		if (GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MORESTUFF))
			MoreDown = 1;
		Recoursed = false;
		
		// Even more down?
		if (MoreDown == 1)
		{
			Recoursed = true;
			if (GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MOREMORESTUFF))
				MoreDown = 2;
			Recoursed = false;
		}
	}
	
	// If key is more more stuff, check more stuff bind
		// This is for multi-key combo buttons
	if (a_Key == DPEXIC_MOREMORESTUFF)
		MoreDown = 1;
	
	/* Determine check shifts */
	// Standard
	if (MoreDown <= 0 || MoreDown > 2)
	{
		KeyBit = PRFKBIT_KEY;
		JoyBit = PRFKBIT_JOY;
		MouseBit = PRFKBIT_MOUSE;
		DMouseBit = PRFKBIT_DMOUSE;
	}
	
	// More
	else if (MoreDown == 1)
	{
		KeyBit = PRFKBIT_KEYP;
		JoyBit = PRFKBIT_JOYP;
		MouseBit = PRFKBIT_MOUSEP;
		DMouseBit = PRFKBIT_DMOUSEP;
	}
	
	// Extra
	else if (MoreDown == 2)
	{
		KeyBit = PRFKBIT_KEYX;
		JoyBit = PRFKBIT_JOYX;
		MouseBit = PRFKBIT_MOUSEX;
		DMouseBit = PRFKBIT_DMOUSEX;
	}
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if ((a_Profile->Ctrls[a_Key][i] & PRFKBIT_MASK) == KeyBit)
		{
			// Get current key
			CurrentButton = (a_Profile->Ctrls[a_Key][i] & PRFKBIT_VMASK);
			
			// Check if key is down
			if (CurrentButton >= 0 && CurrentButton < NUMIKEYBOARDKEYS)
				if (l_KeyDown[CurrentButton])
					return true;
		}
	
	/* Check Joysticks */
	//if (a_Profile->Flags & DPEXF_GOTJOY)
		//if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
	if (a_SID >= 0 && a_SID < MAXSPLITSCREEN && g_Splits[a_SID].JoyBound)
		if (g_Splits[a_SID].JoyID >= 1 && g_Splits[a_SID].JoyID <= MAXLOCALJOYS)
			if (l_JoyButtons[g_Splits[a_SID].JoyID - 1])
				for (i = 0; i < 4; i++)
					if ((a_Profile->Ctrls[a_Key][i] & PRFKBIT_MASK) == JoyBit)
					{
						// Get current button
						CurrentButton = (a_Profile->Ctrls[a_Key][i] & PRFKBIT_VMASK);
				
						// Button pressed?
						if (CurrentButton >= 0 && CurrentButton < 32)
							if (l_JoyButtons[g_Splits[a_SID].JoyID - 1] & (1 << CurrentButton))
								return true;
					}
				
	/* Check Mice */
	if (a_Profile->Flags & DPEXF_GOTMOUSE)
		if (l_MouseButtons[0] || l_MouseButtons[1])
			for (i = 0; i < 4; i++)
			{
				// Single
				if ((a_Profile->Ctrls[a_Key][i] & PRFKBIT_MASK) == MouseBit)
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & PRFKBIT_VMASK);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[0] & (1 << CurrentButton))
							return true;
				}
		
				// Double
				if ((a_Profile->Ctrls[a_Key][i] & PRFKBIT_MASK) == DMouseBit)
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & PRFKBIT_VMASK);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[1] & (1 << CurrentButton))
							return true;
				}
			}
	
	/* Not pressed */
	return false;
}

int16_t G_ClipAimingPitch(int32_t* aiming);

/* D_XNetBuildTicCmd() -- Builds tic command for player */
void D_XNetBuildTicCmd(D_XPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd)
{
#define MAXWEAPONSLOTS 12
	D_ProfileEx_t* Profile;
	player_t* Player, *SpyCon, *SpyPOV, *SpyFake;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT, BaseAM, NegMod, FlyMove;
	bool_t IsTurning, GunInSlot, ResetAim;
	int slot, j, l, k;
	PI_wepid_t newweapon;
	PI_wepid_t SlotList[MAXWEAPONSLOTS];
	D_SplitInfo_t* SSplit;
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Obtain profile */
	Profile = a_NPp->Profile;
	Player = a_NPp->Player;
	
	// No profile?
	if (!Profile)
		return;
	
	/* Find Player ID */
	PID = a_NPp->Player - players;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (D_ScrSplitVisible(SID))
			if (g_Splits[SID].XPlayer == a_NPp)
				break;
	
	// Not found?
	if (SID >= MAXSPLITSCREEN)
	{
		// Force first screen in demo?
		if (demoplayback)
			SID = 0;
		
		// Otherwise don't make any commands
		else
			return;
	}
	
	// Quick ref
	SSplit = &g_Splits[SID];
	
	/* Chatting? */
	// Not in chat mode
	if (!SSplit->ChatMode)
	{
		i = 0;

		// To All
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_CHAT))
			i = 1;

		// To Team
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_TEAMCHAT))
			i = 2;

		// Starting chat?
		if (i)
		{
			// Initiate Chat Mode
			SSplit->ChatMode = i;
			SSplit->ChatTargetID = 0;
			
			// If player 1 is chatting, let go of all keyboard keys
				// Otherwise, stuck keys and re-chatting when chat is done!
			if (SID == 0)
				for (i = 0; i < NUMIKEYBOARDKEYS; i++)
					l_KeyDown[i] = false;
			
			return;		// Do not continue processing events
		}
	}
	
	// In Chat mode
	else
	{
		// Do not handle player events, make them not move as if all keys are down
		return;
	}
	
	/* Reset Some Things */
	SideMove = ForwardMove = BaseAT = BaseAM = FlyMove = 0;
	IsTurning = ResetAim = false;
	
	/* Modifiers */
	// Mouse Sensitivity
	SensMod = 0;
	
	// Movement Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((Profile->SlowTurn) &&
			gametic < (a_NPp->TurnHeld + Profile->SlowTurnTime))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	// Auto-run? If so, invert speeds
	if (Profile->AutoRun)
	{
		MoveSpeed = !MoveSpeed;
		
		if (TurnSpeed != 2)
			TurnSpeed = !TurnSpeed;
	}
	
	/* Player has joystick input? */
	// Read input for all axis
	if (SID >= 0 && SID < MAXSPLITSCREEN && SSplit->JoyBound)
		if (SSplit->JoyID >= 1 && SSplit->JoyID <= MAXLOCALJOYS)
			for (i = 0; i < MAXJOYAXIS; i++)
			{
				// Modify with sensitivity
				TargetMove = ((float)l_JoyAxis[SSplit->JoyID - 1][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
				// Which movement to perform?
				NegMod = 1;
				switch (Profile->JoyAxis[MouseMod][i])
				{
						// Movement
					case DPEXCMA_NEGMOVEX:
					case DPEXCMA_NEGMOVEY:
						NegMod = -1;
						
					case DPEXCMA_MOVEX:
					case DPEXCMA_MOVEY:
						// Movement is fractionally based
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_forwardmove[(Profile->JoyAutoRun ? 1 : MoveSpeed)]);
						TargetMove *= NegMod;
					
						// Now which action really?
						if (Profile->JoyAxis[MouseMod][i] == DPEXCMA_MOVEX ||
								Profile->JoyAxis[MouseMod][i] == DPEXCMA_NEGMOVEX)
							SideMove += TargetMove;
						else
							ForwardMove -= TargetMove;
						break;
					
						// Looking Left/Right
					case DPEXCMA_NEGLOOKX:
						NegMod = -1;
						
					case DPEXCMA_LOOKX:
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_angleturn[(Profile->JoyAutoRun ? 1 : TurnSpeed)]);
						TargetMove *= NegMod;
						IsTurning = true;
						BaseAT -= TargetMove;
						break;
					
						// Looking Up/Down
					case DPEXCMA_NEGLOOKY:
						NegMod = -1;
						
					case DPEXCMA_LOOKY:
#if 0
						TargetMove = (((double)TargetMove) / ((double)32767.0)) *
						
						((double)Profile->LookUpDownSpeed) / (double;
						TargetMove *= NegMod;
						BaseAM += (TargetMove * NegMod);
#endif
						break;
						
						// Panning Up/Down (Linear)
					case DPEXCMA_NEGPANY:
						NegMod = -1;
						
					case DPEXCMA_PANY:
						BaseAM = (((double)l_JoyAxis[SSplit->JoyID - 1][i]) / ((double)32767.0)) * ((double)5856.0);
						BaseAM *= -1 * NegMod;
						
						// Make sure panning look is set
						a_TicCmd->Std.buttons |= BT_PANLOOK;
						break;
						
						// Panning Up/Down (Angular)
					case DPEXCMA_NEGANGPANY:
						NegMod = -1;
						
					case DPEXCMA_ANGPANY:
						// Get angle to extract
						TargetMove = abs(l_JoyAxis[SSplit->JoyID - 1][i]) >> 2;
						
						// Cap to valid precision in LUT
						if (TargetMove >= 8192)
							TargetMove = 8191;
						else if (TargetMove < 0)
							TargetMove = 0;
						
						// Extract from LUT
						BaseAM = (((double)c_AngLUT[TargetMove]) / ((double)32767.0)) * ((double)5856.0);
						BaseAM *= -1 * NegMod;
						
						// Negative?
						if (l_JoyAxis[SSplit->JoyID - 1][i] < 0)
							BaseAM *= -1;
						
						// Make sure panning look is set
						a_TicCmd->Std.buttons |= BT_PANLOOK;
						break;
				
					default:
						break;
				}
			}
	
	/* Player has mouse input? */
	if (l_PermitMouse && (Profile->Flags & DPEXF_GOTMOUSE))
	{
		// Read mouse input for both axis
		for (i = 0; i < 2; i++)
		{
			// Modify with sensitivity
			TargetMove = l_MouseMove[i] * ((((float)(Profile->MouseSens[SensMod] * Profile->MouseSens[SensMod])) / 110.0) + 0.1);
			
			// Do action for which movement type?
			NegMod = 1;
			switch (Profile->MouseAxis[MouseMod][i])
			{
					// Strafe Left/Right
				case DPEXCMA_NEGMOVEX:
					NegMod = -1;
						
				case DPEXCMA_MOVEX:
					SideMove += TargetMove * NegMod;
					break;
					
					// Move Forward/Back
				case DPEXCMA_NEGMOVEY:
					NegMod = -1;
					
				case DPEXCMA_MOVEY:
					ForwardMove += TargetMove * NegMod;
					break;
					
					// Left/Right Look
				case DPEXCMA_NEGLOOKX:
					NegMod = -1;
				
				case DPEXCMA_LOOKX:
					BaseAT -= TargetMove * 8 * NegMod;
					break;
					
					// Up/Down Look
				case DPEXCMA_NEGLOOKY:
					NegMod = -1;
					
				case DPEXCMA_LOOKY:
					BaseAM += (TargetMove * NegMod) << 3;
					//localaiming[SID] += TargetMove << 19;
					break;
				
					// Unknown
				default:
					break;
			}
		}
		
		// Clear mouse permission
		l_PermitMouse = false;
		
		// Clear mouse input
		l_MouseMove[0] = l_MouseMove[1] = 0;
	}
	
	/* Handle Player Control Keyboard Stuff */
	// Weapon Attacks
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_ATTACK))
		a_TicCmd->Std.buttons |= BT_ATTACK;
	
	// Use
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_USE))
		a_TicCmd->Std.buttons |= BT_USE;
	
	// Jump
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_JUMP))
		a_TicCmd->Std.buttons |= BT_JUMP;
	
	// Suicide
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SUICIDE))
		a_TicCmd->Std.buttons |= BT_SUICIDE;
	
	// Keyboard Turning
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNLEFT))
	{
		// Strafe
		if (MoveMod)
			SideMove -= c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT += c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNRIGHT))
	{
		// Strafe
		if (MoveMod)
			SideMove += c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT -= c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	
	// 180 Degree Turn (don't allow repeat on it, otherwise it is useless)
	slot = GAMEKEYDOWN(Profile, SID, DPEXIC_TURNSEMICIRCLE);
	
	if (!a_NPp->Turned180 && slot)
	{
		BaseAT = 0x7FFF;
		IsTurning = true;
		a_NPp->Turned180 = true;
	}
	else if (a_NPp->Turned180 && !slot)
		a_NPp->Turned180 = false;
	
	// Keyboard Moving
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKCENTER))
		ResetAim = true;
		//localaiming[SID] = 0;
	else
	{
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKUP))
			BaseAM += Profile->LookUpDownSpeed >> 16;
		
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKDOWN))
			BaseAM -= Profile->LookUpDownSpeed >> 16;
	}
	
	// Flying
		// Up
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_FLYUP))
		FlyMove += 5;
		
		// Down
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_FLYDOWN))
		FlyMove -= 5;
		
		// Land
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LAND))
		a_TicCmd->Std.buttons |= BT_FLYLAND;
	
	// Weapons
	if (Player)
	{
		// Next
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, 1));
		}
		
		// Prev
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, -1));
		}
		
		// Best Gun
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BESTWEAPON))
		{
			newweapon = P_PlayerBestWeapon(Player, true);
		
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
		
		// Worst Gun
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_WORSTWEAPON))
		{
			newweapon = P_PlayerBestWeapon(Player, false);
		
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
		
		// Slots
		else
		{
			// Which slot?
			slot = -1;
		
			// Look for keys
			for (i = DPEXIC_SLOT1; i <= DPEXIC_SLOT10; i++)
				if (GAMEKEYDOWN(Profile, SID, i))
				{
					slot = (i - DPEXIC_SLOT1) + 1;
					break;
				}
		
			// Hit slot?
			if (slot != -1)
			{
				// Clear flag
				GunInSlot = false;
				l = 0;
		
				// Figure out weapons that belong in this slot
				for (j = 0, i = 0; i < NUMWEAPONS; i++)
					if (P_CanUseWeapon(Player, i))
					{
						// Weapon not in this slot?
						if (Player->weaponinfo[i]->SlotNum != slot)
							continue;
				
						// Place in slot list before the highest
						if (j < (MAXWEAPONSLOTS - 1))
						{
							// Just place here
							if (j == 0)
							{
								// Current weapon is in this slot?
								if (Player->readyweapon == i)
								{
									GunInSlot = true;
									l = j;
								}
						
								// Place in last spot
								SlotList[j++] = i;
							}
					
							// Otherwise more work is needed
							else
							{
								// Start from high to low
									// When the order is lower, we know to insert now
								for (k = 0; k < j; k++)
									if (Player->weaponinfo[i]->SwitchOrder < Player->weaponinfo[SlotList[k]]->SwitchOrder)
									{
										// Current gun may need shifting
										if (!GunInSlot)
										{
											// Current weapon is in this slot?
											if (Player->readyweapon == i)
											{
												GunInSlot = true;
												l = k;
											}
										}
								
										// Possibly shift gun
										else
										{
											// If the current gun is higher then this gun
											// then it will be off by whatever is more
											if (Player->weaponinfo[SlotList[l]]->SwitchOrder > Player->weaponinfo[i]->SwitchOrder)
												l++;
										}
								
										// move up
										memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
										// Place in slightly upper spot
										SlotList[k] = i;
										j++;
								
										// Don't add it anymore
										break;
									}
						
								// Can't put it anywhere? Goes at end then
								if (k == j)
								{
									// Current weapon is in this slot?
									if (Player->readyweapon == i)
									{
										GunInSlot = true;
										l = k;
									}
							
									// Put
									SlotList[j++] = i;
								}
							}
						}
					}
		
				// No guns in this slot? Then don't switch to anything
				if (j == 0)
					newweapon = Player->readyweapon;
		
				// If the current gun is in this slot, go to the next in the slot
				else if (GunInSlot)		// from [best - worst]
					newweapon = SlotList[((l - 1) + j) % j];
		
				// Otherwise, switch to the best gun there
				else
					// Set it to the highest valued gun
					newweapon = SlotList[j - 1];
				
				// Did it work?
				if (newweapon != Player->readyweapon)
				{
					a_TicCmd->Std.buttons |= BT_CHANGE;
					D_TicCmdFillWeapon(a_TicCmd, newweapon);
				}
			}
		}
	}
	
	// Inventory
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVRIGHT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVLEFT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_USEINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVUSE;
	
	/* Handle special functions */
	// Show Scores
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TOPSCORES))
		a_NPp->Scores = 1;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BOTTOMSCORES))
		a_NPp->Scores = -1;
	else
		a_NPp->Scores = 0;
	
	// Automap
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_AUTOMAP))
	{
		// Don't flash the automap like crazy
		if (!SSplit->MapKeyStillDown)
		{
			// Map not active, activate
			if (!SSplit->AutomapActive)
			{
				SSplit->AutomapActive = true;
				SSplit->OverlayMap = false;
			}
			
			// Is active
			else
			{
				// Overlay now active, activate
				if (!SSplit->OverlayMap)
					SSplit->OverlayMap = true;
				
				// Otherwise, stop the map
				else
					SSplit->AutomapActive = false;
			}
			
			// Place key down to prevent massive flashing
			SSplit->MapKeyStillDown = true;
		}
	}
	else
		SSplit->MapKeyStillDown = false;
	
	// Coop Spy
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_COOPSPY))
	{
		// Only every half second
		if (gametic > (a_NPp->CoopSpyTime + (TICRATE >> 1)))
		{
			// Get current POV
			SpyPOV = P_SpecGetPOV(SID);
			SpyFake = P_SpecGet(SID);
			
			// Get current player
			SpyCon = a_NPp->Player;
			
			if (!SpyCon)
				SpyCon = SpyFake;
			
			// In spectator mode
			if (!a_NPp->Player)
			{
				// Go through all players
					// If watching self, find first player
					// If watching someone, find next player
				for (j = ((SpyPOV == SpyFake) ? 0 : SSplit->Display + 1); j < MAXPLAYERS; j++)
					if (playeringame[j])
					{
						SSplit->Display = j;
						SpyPOV = &players[SSplit->Display];
						break;
					}
				
				// Nobody?
				if (j >= MAXPLAYERS)
				{
					SSplit->Display = -1;
					SpyPOV = SpyFake;
				}
				
				else
					SpyPOV = &players[SSplit->Display];
			}
			
			// Normal Game Mode
			else
			{
				j = 0;
				do
				{
					SSplit->Display = (SSplit->Display + 1) % MAXPLAYERS;
					j++;
				} while (j < MAXPLAYERS && (!playeringame[SSplit->Display] || (!ST_SameTeam(&players[SSplit->Console], &players[SSplit->Display]))));
				
				// Change POV
				SpyPOV = &players[SSplit->Display];
			}
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(SpyCon == SpyPOV ? "Yourself" : D_NCSGetPlayerName(SSplit->Display))
				);
			
			// Reset timeout
			a_NPp->CoopSpyTime = gametic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		a_NPp->CoopSpyTime = 0;
	
	/* Set Movement Now */
	// Cap
	if (SideMove > MAXPLMOVE)
		SideMove = MAXPLMOVE;
	else if (SideMove < -MAXPLMOVE)
		SideMove = -MAXPLMOVE;
	
	if (ForwardMove > MAXPLMOVE)
		ForwardMove = MAXPLMOVE;
	else if (ForwardMove < -MAXPLMOVE)
		ForwardMove = -MAXPLMOVE;
	
	// Set
	a_TicCmd->Std.sidemove = SideMove;
	a_TicCmd->Std.forwardmove = ForwardMove;
	a_TicCmd->Std.FlySwim = FlyMove;
	
	/* Slow turning? */
	if (!IsTurning)
		a_NPp->TurnHeld = gametic;
	
	/* Turning */
	a_TicCmd->Std.BaseAngleTurn = BaseAT;
	a_TicCmd->Std.BaseAiming = BaseAM;
	
	if (ResetAim)
		a_TicCmd->Std.buttons |= BT_RESETAIM;
	
	/* Handle Look Spring */
	// This resets aim to center once you move
	if (Profile->LookSpring)
		if (!BaseAM && (abs(ForwardMove) >= c_forwardmove[0] || abs(SideMove) >= c_sidemove[0]))
		{
			a_TicCmd->Std.BaseAiming = 0;
			a_TicCmd->Std.buttons |= BT_RESETAIM;
		}
	
#undef MAXWEAPONSLOTS
}

/* D_XNetPushJW() -- Push join window time */
void D_XNetPushJW(void)
{
	l_LastJW = g_ProgramTic + TICRATE;
}

/* D_XNetUpdate() -- Updates Extended Network */
void D_XNetUpdate(void)
{
	int32_t i, a, j, ScrID;
	D_XPlayer_t* XPlay, *XOrig;
	ticcmd_t* TicCmdP;
	M_UIMenu_t* ProfMenu;
	static tic_t LastSpecTic, LastPT;
	ticcmd_t MergeTrunk;
	bool_t NeedsSomething;
	
	static D_XPlayer_t DemoXPlay;
	
	/* From NetUpdate() */
	if (singletics)
		g_ProgramTic = gametic;
	else
		g_ProgramTic = I_GetTimeMS() / TICRATE;
	
	I_OsPolling();				// i_getevent
	D_ProcessEvents();			// menu responder ???!!!
	
	// This stuff is only important once a program tic
	if (LastPT != g_ProgramTic)
	{
		CONL_Ticker();
		D_JoySpecialTicker();
		M_SMTicker();				// Simple Menu Ticker
		
		LastPT = g_ProgramTic;
	}
	
	/* Handle Networking */
	D_XPRunConnection();
	
	/* Not playing? */
	if (gamestate == GS_DEMOSCREEN || demoplayback)
	{
		// If not on a title screen demo, handle local tic commands
		if (!g_TitleScreenDemo)
		{
			// If the screen has an XPlayer set, use that instead
			if (g_Splits[0].XPlayer)
				XPlay = g_Splits[0].XPlayer;
			
			// Otherwise use a fake one with the default profile
			else
			{
				// Needs init?
				if (DemoXPlay.ID != 1)
				{
					// Set screen and such
					DemoXPlay.Flags = DXPF_LOCAL;
					DemoXPlay.ScreenID = 0;
					DemoXPlay.Profile = g_KeyDefaultProfile;
					
					// Done
					DemoXPlay.ID = 1;
				}
				
				// Set
				XPlay = &DemoXPlay;
			}
			
			// Place tic command at last spot, when possible
			TicCmdP = NULL;
			if (XPlay->LocalAt < MAXLBTSIZE - 1)
				TicCmdP = &XPlay->LocalBuf[XPlay->LocalAt++];
			else
				TicCmdP = &XPlay->LocalBuf[0];
			
			// Virtual tic command creation
			D_XNetBuildTicCmd(XPlay, TicCmdP);
			
			// Move spec only if tic changed
			if (LastSpecTic != g_ProgramTic)
			{
				// Merge tics
				memset(&MergeTrunk, 0, sizeof(MergeTrunk));
				D_XNetMergeTics(&MergeTrunk, XPlay->LocalBuf, XPlay->LocalAt);
				memset(XPlay->LocalBuf, 0, sizeof(XPlay->LocalBuf));
				XPlay->LocalAt = 0;
			
				// Execute Command
				P_SpecRunTics(0, &MergeTrunk);
				
				// Scores
				g_NetBoardDown = XPlay->Scores;
			
				// Tics
				LastSpecTic = g_ProgramTic;
			}
		}
		
		return;
	}
	
	/* Handle local players and splits */ 
	if (!demoplayback && gamestate != GS_DEMOSCREEN)
	{
		// Players in split, but not mapped player?
		for (a = 0, i = 0; i < MAXSPLITSCREEN; i++)
			if (D_ScrSplitHasPlayer(i))
			{
				a++;
				
				if (!g_Splits[i].XPlayer)
				{
					g_Splits[i].XPlayer = D_XNetLocalPlayerByPID(g_Splits[i].ProcessID);
					
					// Found it!? Elicit profile change
					if (g_Splits[i].XPlayer)
					{
						if (g_Splits[i].Profile)
							D_XNetChangeLocalProf(i, g_Splits[i].Profile);
						
						if (i >= g_SplitScreen)
						{
							g_SplitScreen = i;
							R_ExecuteSetViewSize();
						}
					}
				}
			}
		
		// No local players? Give a local player, if possible
		if (!a)
		{
			for (i = 0; i < g_NumXPlays; i++)
			{
				// Get player
				XPlay = g_XPlays[i];
				
				// Missing?
				if (!XPlay)
					continue;
				
				// Local and not a bot?
				if ((XPlay->Flags & (DXPF_LOCAL | DXPF_BOT)) == DXPF_LOCAL)
				{
					XPlay->ScreenID = 0;
					g_Splits[0].Display = -1;
					g_Splits[0].Waiting = true;
					g_Splits[0].XPlayer = XPlay;
					if (XPlay->ClProcessID)
						g_Splits[0].ProcessID = XPlay->ClProcessID;
					else
						g_Splits[0].ProcessID = XPlay->ID;
					g_SplitScreen = 0;
				}
			}
		}
	}
	
	/* Enable Mouse Input */
	l_PermitMouse = true;
	
	/* Request Screens to Join */
	// They need XPlayers too
	for (i = 0; i < MAXSPLITSCREEN; i++)
		// Screen not active and waiting, and there is no net player
		if (!g_Splits[i].XPlayer && !g_Splits[i].Active && g_Splits[i].Waiting)
			// If we are the local server, we can just add players for fun
			if (D_XNetIsServer())
			{
				// Send with process ID
				g_Splits[i].XPlayer = D_XNetAddPlayer(DS_XNetMakeServPB, &g_Splits[i].ProcessID, false);
				
				// Set screen
				g_Splits[i].XPlayer->ScreenID = i;
			}
			
			// Otherwise, we need to tell them we want to join
			else
			{
				// No requests sent or ran out of time
				if (!g_Splits[i].RequestSent || g_ProgramTic > g_Splits[i].GiveUpAt)
					// Reached request limit
					if (g_Splits[i].RequestSent > l_CLMaxPTries.Value->Int)
					{
						// Remove the split
						D_NCRemoveSplit(i, false);
					}
				
					// Limit not yet reached
					else
					{
						// Send Message
						D_XPRequestScreen(i);
						
						// Set timeout
						g_Splits[i].RequestSent++;
						g_Splits[i].GiveUpAt = g_ProgramTic + (l_CLMaxPTryTime.Value->Int * TICRATE);
					}
			}
	
	/* Build Local Tic Commands (possibly) */
	for (i = 0; i < g_NumXPlays; i++)
	{
		// Get current player
		XPlay = g_XPlays[i];
		
		// Missing?
		if (!XPlay)
			continue;
		
		// Non-local player?
		if (!(XPlay->Flags & DXPF_LOCAL))
			continue;
		
		// Map to screen
		for (ScrID = 0; ScrID < MAXSPLITSCREEN; ScrID++)
			if (XPlay == g_Splits[ScrID].XPlayer)
				break;
		
		// Illegal?
		if (ScrID >= MAXSPLITSCREEN)
			ScrID = -1;
		
		
		// No profile loaded?
		if (ScrID >= 0)
		{
			// Inactive?
			if (!D_ScrSplitHasPlayer(ScrID))
				continue;
				
			// No profile set? And not a bot
			NeedsSomething = false;
			if (!XPlay->Profile && !(XPlay->Flags & DXPF_BOT))
				NeedsSomething = true;
			
			// See if split screen has a profile
			if (NeedsSomething)
				if (g_Splits[ScrID].Profile)
				{
					// Use this profile
					if (!XPlay->DidPFromS)
						D_XNetChangeLocalProf(ScrID, g_Splits[ScrID].Profile);
					
					// Did from screen
					XPlay->DidPFromS = true;
					
					// Do not need request
					NeedsSomething = false;
				}
			
			// Even split player has no profile =(, use menu
			if (NeedsSomething)
			{
				// Not in level
				if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
					continue;
				
				// Set as needing a profile
				XPlay->StatusBits |= DXPSB_NEEDSPROFILE;
				
				// Check players menus for profile prompt
				ProfMenu = M_ExMakeMenu(M_ExMenuIDByName("profileselect"), NULL);
				
				// Failed?
				if (!ProfMenu)
					continue;
				
				// See where the menu is
				a = M_ExFirstMenuSpot(ScrID, ProfMenu);
			
				// Not in first spot?
				if (a != 0)
				{
					// Clear all menus and place there
					M_ExPopAllMenus(ScrID);
					M_ExPushMenu(ScrID, ProfMenu);
					g_ResumeMenu++;
				}
			}
			
			// Do not build commands?
			if (NeedsSomething)
				continue;
		}
		
		// Does not need profile
		XPlay->StatusBits &= ~DXPSB_NEEDSPROFILE;
		
		// Place tic command at last spot, when possible
		TicCmdP = NULL;
		if (XPlay->LocalAt < MAXLBTSIZE - 1)
			TicCmdP = &XPlay->LocalBuf[XPlay->LocalAt++];
		else
		{
			TicCmdP = &XPlay->LocalBuf[0];
			XPlay->StatusBits |= DXPSB_LBOVERFLOW;
		}
		
		// Clear Tic command before rebuilding
		memset(TicCmdP, 0, sizeof(*TicCmdP));
		
		// Clear overflow bit
		XPlay->StatusBits &= ~DXPSB_LBOVERFLOW;
		
		// Bot Player?
		if (XPlay->Flags & DXPF_BOT)
			B_BuildBotTicCmd(XPlay, XPlay->BotData, TicCmdP);
		
		// Human player
		else
			D_XNetBuildTicCmd(XPlay, TicCmdP);
		
		// No player for this one?
		if (!XPlay->Player)
		{
			// Use is down? Attempt joining
			if (g_ProgramTic >= XPlay->LastJoinAttempt && (TicCmdP->Std.buttons & BT_USE))
			{
				XPlay->LastJoinAttempt = g_ProgramTic + TICRATE;
				D_XNetTryJoin(XPlay);
			}
			
			// Move the spectator camera around
			else if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
			{
				// Modify the fake spectator camera
				if (g_ProgramTic/*gametic*/ != LastSpecTic)
				{
					// Merge tics
					memset(&MergeTrunk, 0, sizeof(MergeTrunk));
					D_XNetMergeTics(&MergeTrunk, XPlay->LocalBuf, XPlay->LocalAt);
					memset(XPlay->LocalBuf, 0, sizeof(XPlay->LocalBuf));
					XPlay->LocalAt = 0;
					
					// Execute Command
					P_SpecRunTics(XPlay->ScreenID, &MergeTrunk);
				}
			}
		}
	}
	
	/* Modify spectators */
	LastSpecTic = gametic;

	/* Handle Networking, again */
	D_XPRunConnection();
}

/* D_XNetInitialServer() -- Create Initial Server */
void D_XNetInitialServer(void)
{
#define BUFSIZE 128
#define SMALLBUF 16
	char Buf[BUFSIZE], *p;
	bool_t DoServer, DoClient, GotIP, Anti;
	uint16_t Port;
	I_HostAddress_t Addr;
	uint32_t GameID;
	
	/* Actually make the server? */
	// Reset
	DoServer = DoClient = GotIP = Anti = false;
	GameID = 0;
	Port = 0;
	memset(Buf, 0, sizeof(Buf));
	memset(&Addr, 0, sizeof(Addr));
	
	/* Connection Type? */
	// Master: Hosting server
	if (M_CheckParm("-server") || M_CheckParm("-host"))
	{
		// Set as server
		DoServer = true;
		
		// Binding to certain address
		if (M_IsNextParm())
			strncat(Buf, M_GetNextParm(), BUFSIZE - 1);
		
		// Dedicated Server?
		if (M_CheckParm("-dedicated"))
			l_IsDedicated = true;
	}
	
	// Master: Connecting to server
	else if ((M_CheckParm("-antiserver") || M_CheckParm("-antihost")) && M_IsNextParm())
	{
		// We are a server, but anti
		DoServer = true;
		Anti = true;
		
		// Read hostname
		strncat(Buf, M_GetNextParm(), BUFSIZE - 1);
	}
	
	// Slave: Connecting to server
	else if (M_CheckParm("-connect") && M_IsNextParm())
	{
		// Set as client
		DoClient = true;
		
		// Copy entire buffer
		strncat(Buf, M_GetNextParm(), BUFSIZE - 1);
	}
	
	// Slave: Hosting server
	else if (M_CheckParm("-anticonnect"))
	{
		// Set as anti-client
		DoClient = true;
		Anti = true;
		
		// Binding to certain address
		if (M_IsNextParm())
			strncat(Buf, M_GetNextParm(), BUFSIZE - 1);
	}
	
	/* Determine hostname */
	if (Buf[0])
	{
		// Get host and gameid
		GotIP = D_XNetHostnameToAddrGID(Buf, &Addr, &GameID);		
		
		// Failed to get IP?
		if (!GotIP)
			memset(Buf, 0, sizeof(Buf));
	}
	
	/* If there is an address, or we are serving */
	// Always auto start, so we don't get stuck a the title screen
	if (GotIP || DoServer || DoClient)//(DoServer && !Anti) || (DoClient && Anti))
		NG_SetAutoStart(true);
	
	/* Starting Server? */
	if (DoServer)
	{
		// No address specified?
		if (Anti)
			if (!Buf[0] || !GotIP)
				return;
		
		// Make server as usual
		D_XNetMakeServer(true, (GotIP ? &Addr : NULL), GameID, Anti);
	}
	
	/* Starting Client */
	else if (DoClient)
	{
		// No address specified?
		if (!Anti)
			if (!Buf[0] || !GotIP)
				return;
		
		// Use connection pathway
		D_XNetConnect((GotIP ? &Addr : NULL), GameID, Anti);
	}
	
	/* Standard Command Line Game */
	else if (NG_IsAutoStart())
	{
		D_XNetMakeServer(false, NULL, 0, false);
	}
#undef BUFSIZE
#undef SMALLBUF
}

/* D_XNetBecomeServer() -- Become the server */
void D_XNetBecomeServer(void)
{
	size_t i;
	D_XPlayer_t* XPlay;
	
	/* Go through all XPlayers */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Not here?
		if (!XPlay)
			continue;
		
		// Local Player
		if (XPlay->Flags & DXPF_LOCAL)
		{
			// Become server!
			XPlay->Flags |= DXPF_SERVER;
		}
		
		// Bot player
		else if (XPlay->Flags & DXPF_BOT)
		{
			// Remove server and make local
			XPlay->Flags &= ~DXPF_SERVER;
			XPlay->Flags |= DXPF_LOCAL;
		}
		
		// Other, remove server status if any
		else
		{
			XPlay->Flags &= DXPF_SERVER;
		}
	}
	
	/* Kick any non local players */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Not here?
		if (!XPlay)
			continue;
		
		// Kick them?
		if (!(XPlay->Flags & DXPF_LOCAL))
			D_XNetKickPlayer(XPlay, "Became server player.", false);
	}
}

/* D_XNetMakeHostID() -- Makes a random ID */
uint32_t D_XNetMakeID(const uint32_t a_ID)
{
	uint32_t ID;
	
	/* ID Creation Loop */
	ID = a_ID;
	while (!ID || D_XNetPlayerByID(ID) || D_NCSFindSplitByProcess(ID) >= 0 || D_XNetPlayerByHostID(ID) || D_XBEndPointForID(ID))
	{
		ID = D_CMakePureRandom();
	}
	
	/* Return specified ID, which is unique */
	return ID;
}

