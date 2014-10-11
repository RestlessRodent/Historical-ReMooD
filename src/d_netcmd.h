// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: host/client network commands

#ifndef __D_NETCMD_H__
#define __D_NETCMD_H__







// add game commands, needs cleanup
void D_RegisterClientCommands(void);
void D_SendPlayerConfig(void);
void Command_ExitGame_f(void);

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

#define MAXDNETTICCMDCOUNT					64	// Max allowed buffered tics

/*** STRUCTURES ***/

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* Define ticcmd_t */
#if !defined(__REMOOD_TICCMDT_DEFINED)
	typedef union ticcmd_u ticcmd_t;
	#define __REMOOD_TICCMDT_DEFINED
#endif

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/* D_SplitInfo_t -- Split Screen Info */
typedef struct D_SplitInfo_s
{
	bool_t Active;								// Is Active
	bool_t Waiting;								// Waiting for fill
	int32_t Console;							// The console player
	int32_t Display;							// Display Player
	uint32_t ProcessID;							// Local Processing ID
	D_Prof_t* Profile;							// Player Profile
	
	SN_Port_t* Port;							// Control Port
	bool_t DoNotSteal;							// Do not steal port
	tic_t PortTimeOut;							// Timeout for port
	
	bool_t JoyBound;							// Joystick Bound
	uint32_t JoyID;								// Joystick ID
	
	uint8_t RequestSent;						// Sent join request
	tic_t GiveUpAt;								// Give up joining at this time
	bool_t OverlayMap;							// Overlay automap
	bool_t MapKeyStillDown;						// Automap key still down
	
	int8_t ChatMode;							// All? Spec? Team?
	uint32_t ChatTargetID;						// Player to talk to
	tic_t ChatTimeOut;							// Chat timeout
	
	tic_t CoopSpyTime;							// Time to wait to respy
	tic_t TurnHeld;								// Time turning is held
	int32_t Scores;								// Scoreboard showing
	bool_t Turned180;							// Did 180 degre turn
	
	// Automap Stuff
	bool_t AutomapActive;						// Activated Automap
	fixed_t MapZoom;							// Zoom in the map
	bool_t MapFreeMode;							// Free movement mode
	fixed_t MapPos[2];							// Map position
	
	// Profile Select
	bool_t SelProfile;							// Selecting profile
	D_Prof_t* AtProf;							// At this profile
} D_SplitInfo_t;

/*** GLOBALS ***/

extern int g_SplitScreen;						// Players in splits
extern D_SplitInfo_t g_Splits[MAXSPLITS];	// Split Information

extern bool_t g_NetDev;

extern angle_t localangle[MAXSPLITS];
extern int localaiming[MAXSPLITS];	// should be a angle_t but signed

/*** FUNCTIONS ***/

bool_t D_ScrSplitHasPlayer(const int8_t a_Player);
bool_t D_ScrSplitVisible(const int8_t a_Player);

void D_NCRemoveSplit(const int32_t a_Split, const bool_t a_Demo);
void D_NCResetSplits(const bool_t a_Demo);
int8_t D_NCSFindSplitByProcess(const uint32_t a_ID);

void D_XNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc);

const char* D_NCSGetPlayerName(const uint32_t a_PlayerID);

uint32_t D_CMakePureRandom(void);
void D_CMakeUUID(char* const a_Buf);

#endif

