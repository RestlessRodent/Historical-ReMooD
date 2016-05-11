// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "doomtype.h"
#include "d_ticcmd.h"
#include "d_netcmd.h"
#include "g_state.h"
#include "sn.h"
#include "d_player.h"
#include "d_prof.h"
#include "r_main.h"
#include "i_video.h"
#include "m_random.h"

#include "info.h"	// TODO FIXME: Only used by D_TicCmdFillWeapon()


/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

const int32_t c_TCDataSize[NUMDTCT] =
{
	// NULL
	0,
	
	// DTCT_SNJOINPLAYER
	4 + 4 + 1 + 1 + 1 + 4,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
		// uint8	Team Number
		// uint8	Color
		// uint32	Flags
	
	// MAP CHANGE
	1 + 8,
		// uint8  Flags
		// uint8* Lump Name
	
	// DTCT_GAMEVAR, GAME VARIABLE
	4 + 4,
		// uint32 Code
		// int32  New Value
	
	// DTCT_XCHANGEMONSTERTEAM, Change Monster Team
	4 + 1,
		// uint32 Player ID
		// uint8  Set Flag
	
	// DTCT_XMORPHPLAYER, Morphs base class of player
	4 + MAXPLAYERNAME,
		// uint32 Player ID
		// uint8* Class to morph to
		
	// DTCT_SNQUITREASON, Reason for quitting
	4 + 4 + 1 + 1 + MAXTCSTRINGCAT,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
		// uint8  0 == set, 1 == append
		// uint8* String to set or append
	
	// DTCT_SNCLEANUPHOST, Tell clients to cleanup host
	4 + 4 + 1,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
	
	// DTCT_SNJOINHOST, Host connects
	4 + 4 + 1,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
	
	// DTCT_SNPARTPLAYER, Player leaves
	4 + 4 + 1,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
	
	// DTCT_SNJOINPORT, Port is joined
	4 + 4 + 1 + 4,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
		// uint32	Client Port's ProcessID
	
	// DTCT_SNCHATFRAG, Chat Fragment
	4 + 4 + 1 + 4 + 1 + MAXTCSTRINGCAT,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
		// uint32	Target
		// uint8	Mode
		// uint8*	Text
	
	// DTCT_SNPORTSETTING, Port Setting
	4 + 4 + 1 + 2 + MAXTCSTRINGCAT,
		// uint32	HID
		// uint32	ID
		// uint8	PlayerID
		// uint16	Setting
		// uint8*	Value
};

/*** GLOBALS ***/

bool_t g_NetDev = false;						// Network Debug
int g_SplitScreen = -1;							// Split screen players (-1 based)
D_SplitInfo_t g_Splits[MAXSPLITS];			// Split Information

/*** LOCALS ***/

/*** FUNCTIONS ***/

/* D_ScrSplitHasPlayer() -- Split-screen has player */
bool_t D_ScrSplitHasPlayer(const int8_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITS)
		return false;
	
	/* Active (Non-Demo Only) */
	if (!demoplayback && g_Splits[a_Player].Active)
		return true;
	
	/* Waiting for player */
	if (g_Splits[a_Player].Waiting)
		return true;
	
	/* No player */
	return false;
}

/* D_ScrSplitVisible() -- Screen can be seen */
bool_t D_ScrSplitVisible(const int8_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITS)
		return false;
	
	/* Visible if has a player */
	if (a_Player == 0 || D_ScrSplitHasPlayer(a_Player))
		return true;
	
	/* Not visible */
	return false;
}

/* D_XNetMergeTics() -- Merges all tic commands */
void D_XNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc)
{
//#define __REMOOD_SWIRVYANGLE
	size_t i, j;
	int32_t FM, SM, AT, AM, RANG, FLY;
	fixed_t xDiv;
	
	/* Check */
	if (!a_DestCmd || !a_SrcList || !a_NumSrc)
		return;
	
	/* Merge Variadic Stuff */
	// Super merging
	FM = SM = AT = AM = RANG = FLY = 0;
	for (j = 0; j < a_NumSrc; j++)
	{
		FM += a_SrcList[j].Std.forwardmove;
		SM += a_SrcList[j].Std.sidemove;
		RANG += a_SrcList[j].Std.angleturn;
		FLY += a_SrcList[j].Std.FlySwim;

#if defined(__REMOOD_SWIRVYANGLE)
		AT += a_SrcList[j].Std.BaseAngleTurn;
		AM += a_SrcList[j].Std.BaseAiming;
#else
		// Use the furthest aiming angle
		if (abs(a_SrcList[j].Std.BaseAngleTurn) > abs(AT))
			AT = a_SrcList[j].Std.BaseAngleTurn;
		if (abs(a_SrcList[j].Std.BaseAiming) > abs(AM))
			AM = a_SrcList[j].Std.BaseAiming;
#endif
		
		// Merge weapon here
		if (!a_DestCmd->Std.XSNewWeapon[0])
			strncpy(a_DestCmd->Std.XSNewWeapon, a_SrcList[j].Std.XSNewWeapon, MAXTCWEAPNAME - 1);
		
		// Clear slot and weapon masks (they OR badly)
		a_DestCmd->Std.buttons &= ~(BT_WEAPONMASK | BT_SLOTMASK);
		
		// Merge Buttons
		a_DestCmd->Std.buttons |= a_SrcList[j].Std.buttons;
		
		a_DestCmd->Std.aiming = a_SrcList[j].Std.aiming;
		
		// Use higher ping
		if ((a_SrcList[j].Ctrl.Ping & TICPINGAMOUNTMASK) > (a_DestCmd->Ctrl.Ping & TICPINGAMOUNTMASK))
			a_DestCmd->Ctrl.Ping = a_SrcList[j].Ctrl.Ping; 
	}

	// Do some math
	xDiv = ((fixed_t)a_NumSrc) << FRACBITS;
	a_DestCmd->Std.forwardmove = FixedDiv(FM << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.sidemove = FixedDiv(SM << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.FlySwim = FixedDiv(FLY << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.angleturn = a_SrcList[0].Std.angleturn;//FixedDiv(RANG << FRACBITS, xDiv) >> FRACBITS;
	
	/* Aiming is slightly different */
#if defined(__REMOOD_SWIRVYANGLE)
	// Divide some
	//AT /= ((int32_t)(a_NumSrc));
	
	// Cap
	if (AT > 32000)
		AT = 32000;
	else if (AT < -32000)
		AT = -32000;
	
	// Cap
	if (AM > 32000)
		AM = 32000;
	else if (AM < -32000)
		AM = -32000;
	
	// Try now
	a_DestCmd->Std.BaseAngleTurn = FixedDiv(AT << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.BaseAiming = FixedDiv(AM << FRACBITS, xDiv) >> FRACBITS;
#else
	// Use furthest angle
	//AT /= ((int32_t)(a_NumSrc));
	a_DestCmd->Std.BaseAngleTurn = AT;
	a_DestCmd->Std.BaseAiming = AM;
#endif
}

/* D_NCSFindSplitByProcess() -- Finds split screen by process */
int8_t D_NCSFindSplitByProcess(const uint32_t a_ID)
{
	int i;
	
	/* Check */
	if (!a_ID)
		return -1;
	
	/* Loop */
	for (i = 0; i < MAXSPLITS; i++)
		//if (D_ScrSplitHasPlayer(i))
			if (g_Splits[i].ProcessID == a_ID)
				return i;
	
	/* Not found */
	return -1;
}

/* D_NCRemoveSplit() -- Removes Split */
void D_NCRemoveSplit(const int32_t a_Split, const bool_t a_Demo)
{
	int i;
	
	/* Check */
	if (a_Split < 0 || a_Split >= MAXSPLITS)
		return;	
	
	/* Not in demo */
	if (!a_Demo)
	{
		// Tell the server that the player is no longer going to be around
		//if (g_Splits[a_Split].XPlayer)
		//	D_XNetPartLocal(g_Splits[a_Split].XPlayer);
		
		// Remove chat
		//D_XNetClearChat(a_Split);
		
		// Move splits down, to replace this split
		for (i = a_Split; i < MAXSPLITS; i++)
			// Last spot?
			if (i == MAXSPLITS - 1)
			{
				memset(&g_Splits[i], 0, sizeof(g_Splits[i]));
				g_Splits[i].Display = -1;
			}
			
			// Move the stuff from the next spot over this one
			else
			{
				memmove(&g_Splits[i], &g_Splits[i + 1], sizeof(g_Splits[i]));
				
				if (g_Splits[i].Port)
					g_Splits[i].Port->Screen = i;
			}
	}
	
	/* In demo */
	else
	{
		// Remove current info
#if 0
		g_Splits[a_Split].ProcessID = 0;
		g_Splits[a_Split].Profile = NULL;
		g_Splits[a_Split].XPlayer = NULL;
		g_Splits[a_Split].JoyBound = false;
		g_Splits[a_Split].JoyID = 0;
		g_Splits[a_Split].RequestSent = false;
		g_Splits[a_Split].GiveUpAt = 0;
#endif
		
		// Move splits down, to replace this split
		for (i = a_Split; i < MAXSPLITS; i++)
			// Last spot?
			if (i == MAXSPLITS - 1)
			{
				// Make inactive
				g_Splits[i].Active = false;
			}
			
			// Move the stuff from the next spot over this one
			else
			{
				// Grab non breaking stuff from demos over
				g_Splits[i].Active = g_Splits[i + 1].Active;
				g_Splits[i].Console = g_Splits[i + 1].Console;
				g_Splits[i].Display = g_Splits[i + 1].Display;
				g_Splits[i].Port = g_Splits[i + 1].Port;
			}
	}
	
	/* Correctsplit screen */
	// Subtract the removed player
	if (g_SplitScreen >= 0)
		g_SplitScreen--;
	
	// Correct visual display
	R_ExecuteSetViewSize();
}

/* D_NCResetSplits() -- Resets all splits */
void D_NCResetSplits(const bool_t a_Demo)
{
	int i;
	
	/* Wipe all splits */
	for (i = MAXSPLITS; i > 0; i--)
		D_NCRemoveSplit(i - 1, a_Demo);
}

/* D_NCSGetPlayerName() -- Get player name */
const char* D_NCSGetPlayerName(const uint32_t a_PlayerID)
{
	const char* dss;
	
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return NULL;
	
	/* Player is in game */
	if (playeringame[a_PlayerID])
	{
		// Try from profiles
		if (players[a_PlayerID].ProfileEx)
		{
			dss = D_ProfileDisplayName(players[a_PlayerID].ProfileEx);
			if (dss != NULL)
				return dss;
		}
	}
	
	/* Return default */
	if (player_names[a_PlayerID][0])
		return player_names[a_PlayerID];
	
	/* Return Unknown */
	return "Unnamed Player";
}

/* D_TicCmdFillWeapon() -- Fills weapon for tic command */
void D_TicCmdFillWeapon(ticcmd_t* const a_Target, const int32_t a_ID)
{
	strncpy(a_Target->Std.XSNewWeapon, wpnlev1info[a_ID]->ClassName, MAXTCWEAPNAME - 1);
}

/* D_CMakePureRandom() -- Create a pure random number */
uint32_t D_CMakePureRandom(void)
{
//#define __REMOOD_RANDRAWBITS
	uint32_t Garbage, i;
	uint32_t* RawBits;
	void* StackP;
	
	/* Allocate Raw Bits */
#ifdef __REMOOD_RANDRAWBITS
	RawBits = (uint32_t*)I_SysAlloc(sizeof(*RawBits) * 16);
#endif
	
	/* Attempt number generation */
	// Init
	Garbage = M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	
	// Current Time
	Garbage ^= ((int)I_GetTime() * (int)I_GetTime());
	
	// Address of this function
	Garbage ^= (uint32_t)(((uintptr_t)D_CMakePureRandom) * ((uintptr_t)D_CMakePureRandom));
	
	// Address of garbage
	Garbage ^= (uint32_t)(((uintptr_t)&Garbage) * ((uintptr_t)&Garbage));
	
	// Current PID
	Garbage ^= ((uint32_t)I_GetCurrentPID() * (uint32_t)I_GetCurrentPID());
	
	// Stack Pointer
	StackP = alloca(1);
	Garbage ^= (uint32_t)((uintptr_t)StackP ^ ((uintptr_t)StackP >> UINT64_C(32)));
	
#ifdef __REMOOD_RANDRAWBITS
	// Allocated Data
	if (RawBits)
	{
		// Raw bits address
		Garbage ^= (uint32_t)(((uintptr_t)RawBits) * ((uintptr_t)RawBits));
	
		// Raw bits data (unitialized memory)
		for (i = 0; i < 16; i++)
			Garbage ^= RawBits[i];
	
		// Cleanup
		I_SysFree(RawBits);
	}
#endif
	
	/* Return the garbage number */
	return Garbage;
}

/* D_CMakeUUID() -- Makes a UUID */
void D_CMakeUUID(char* const a_Buf)
{
	size_t i, FailCount;
	uint8_t Char;
	uint32_t Garbage;
	
	/* Generate a hopefully random ID */
	for (i = 0; i < MAXUUIDLENGTH - 1; i++)
	{
		// Hopefully random enough
		Garbage = D_CMakePureRandom();
		Char = (((int)(M_Random())) + Garbage);
		FailCount = 0;
		
		// Limit Char
		while (!((Char >= '0' && Char <= '9') || (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')))
		{
			if (Char <= 'A')
				Char += 15;
			else if (Char >= 'z')
				Char -= 15;
			else
				Char ^= D_CMakePureRandom();
			
			if (++FailCount >= 10)
				if (M_Random() & 1)
					Char = 'A' + (M_Random() % ('Y' - 'A'));
				else
					Char = 'a' + (M_Random() % ('y' - 'a'));
		}
		
		// Last character is the same as this?
		if (i > 0 && Char == a_Buf[i - 1])
		{
			i--;
			continue;
		}
		
		// Set as
		a_Buf[i] = Char;
		
		// Sleep for some unknown time
		I_WaitVBL(M_Random() & 1);
	}
}


