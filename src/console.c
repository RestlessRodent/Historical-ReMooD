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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
//      console for ReMooD

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "g_input.h"
#include "keys.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_video.h"
#include "z_zone.h"
#include "i_system.h"
#include "d_main.h"
#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "g_input.h"
#include "keys.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_video.h"
#include "z_zone.h"
#include "i_system.h"
#include "d_main.h"
#include "hu_stuff.h"
#include "v_video.h"
#include "st_stuff.h"
#include "r_defs.h"

#include "hu_stuff.h"
#include "v_video.h"
#include "st_stuff.h"
#include "r_defs.h"

#ifndef _WIN32
#include <unistd.h>
#endif

/****************
*** CONSTANTS ***
****************/

#define MAXCONLPLAYERMQ		5	// Max messages in player queue
#define DEFPRCONLPLAYERMQ	127	// Default priority in queue
#define DEFTOCONPLAYERMQ	(TICRATE * 4)	// Default timeout in queue
#define MAXCONLPMQBUFSIZE	128	// Length of the message buffer
#define CONLMAXCOMBUFSIZE	160	// Length of input command

#include "bootdata.h"

/*****************
*** STRUCTURES ***
*****************/

typedef void (*CONL_FlushFunc_t) (const char* const a_Buf);

/* CONL_BasicBuffer_t -- A basic buffer for the console */
typedef struct CONL_BasicBuffer_s
{
	char* Buffer;				// Actual buffer
	char** Lines;				// Lines in buffer
	size_t Size;				// Size of the buffer
	size_t NumLines;			// Number of lines in buffer
	
	uint32_t MaskPos;			// Position mask
	uint32_t StartPos;			// Start position of buffer
	uint32_t EndPos;			// End position of buffer
	uint32_t MaskLine;			// Line mask
	uint32_t StartLine;			// First line in buffer
	uint32_t EndLine;			// Last line in buffer
	uint32_t CountLine;			// Line count
	
	CONL_FlushFunc_t FlushFunc;	// Function to call on '\n'
	char ExtraNewLine;			// Extra newline
} CONL_BasicBuffer_t;

/* CONL_PlayerMessage_t -- Message for player */
typedef struct CONL_PlayerMessage_s
{
	uint32_t Flags;				// Flags for this message
	char Text[MAXCONLPMQBUFSIZE];	// Message Text
	tic_t Timeout;				// When the message times out
	uint8_t Priority;			// Priority of the message
} CONL_PlayerMessage_t;

/**************
*** GLOBALS ***
**************/

bool_t g_EarlyBootConsole = false;				// Early Boot Console
int32_t g_MousePos[2] = {0, 0};					// Mouse Position
bool_t g_MouseDown = false;						// Mouse is down

/*************
*** LOCALS ***
*************/

static bool_t l_CONLActive = false;	// Console active?
static CONL_BasicBuffer_t l_CONLBuffers[2];	// Input/Output Buffer
static CONL_PlayerMessage_t l_CONLMessageQ[MAXSPLITSCREENPLAYERS][MAXCONLPLAYERMQ];	// Player message queue
static uint32_t l_CONLLineOff = 0;	// Line offset
static CONCTI_Inputter_t* l_CONLInputter = NULL;	// Console inputter

static char l_ConfigDir[PATH_MAX];				// Configuration directory
static char l_DataDir[PATH_MAX];				// Data directory
static char l_DefaultConfig[PATH_MAX];			// The default config

// Variables
#if !defined(__REMOOD_DEDICATED)

// con_screenheight -- Height of the console
CONL_StaticVar_t l_CONScreenHeight =
{
	CLVT_FIXED, c_CVPVClamp, CLVF_SAVE,
	"con_screenheight", DSTR_CVHINT_CONSCREENHEIGHT, CLVVT_PERCENT, "0.5",
	NULL
};

// con_backcolor -- Background color of the console
CONL_StaticVar_t l_CONBackColor =
{
	CLVT_INTEGER, c_CVPVVexColor, CLVF_SAVE,
	"con_backcolor", DSTR_CVHINT_CONBACKCOLOR, CLVVT_STRING, "Red",
	NULL
};

// con_font -- Console Font
CONL_StaticVar_t l_CONFont =
{
	CLVT_INTEGER, c_CVPVFont, CLVF_SAVE,
	"con_font", DSTR_CVHINT_CONFONT, CLVVT_STRING, "OEM",
	NULL
};

// con_monospace -- Draw as monospaced
CONL_StaticVar_t l_CONMonoSpace =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"con_monospace", DSTR_CVHINT_CONMONOSPACE, CLVVT_STRING, "true",
	NULL
};

// con_scale -- Scale the console text
CONL_StaticVar_t l_CONScale =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"con_scale", DSTR_CVHINT_CONSCALE, CLVVT_STRING, "false",
	NULL
};

// con_teststring -- A test string
CONL_StaticVar_t l_CONTestString =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"con_teststring", DSTR_CVHINT_CONTESTSTRING, CLVVT_STRING, "Hello World!",
	NULL
};

#endif

/****************
*** FUNCTIONS ***
****************/

/*** Common Text Input ***/

/* CONCTI_CreateInput() -- Creates common inputter */
CONCTI_Inputter_t* CONCTI_CreateInput(const size_t a_NumHistory, const CONCTI_OutBack_t a_OutBack, CONCTI_Inputter_t** const a_RefPtr)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	CONCTI_Inputter_t* New;
	
	/* Not in dedicated */
	if (g_DedicatedServer)
		return NULL;
	
	/* Create new */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Set data
	New->OutFunc = a_OutBack;
	New->RefPtr = a_RefPtr;
	
	// History base
	New->NumHistory = a_NumHistory;
	if (New->NumHistory)
		New->History = Z_Malloc(sizeof(*New->History) * New->NumHistory, PU_STATIC, NULL);
	
	// Default font is OEM
	New->Font = VFONT_OEM;
	
	return New;
#endif /* __REMOOD_DEDICATED */
}

/* CONCTI_DestroyInput() -- Destroys common inputter */
void CONCTI_DestroyInput(CONCTI_Inputter_t* const a_Input)
{

	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	size_t i;
	
	/* Not in dedicated */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_Input)
		return;
		
	/* Clear reference */
	if (a_Input->RefPtr)
		*a_Input->RefPtr = NULL;
		
	/* Clear history */
	if (a_Input->History)
	{
		// Clear individual lines
		for (i = 0; i < a_Input->HistoryCount; i++)
			if (a_Input->History[i])
				Z_Free(a_Input->History[i]);
				
		// Clear entire buffer
		Z_Free(a_Input->History);
	}
	
	/* Delete self */
	Z_Free(a_Input);
#endif /* __REMOOD_DEDICATED */
}

/* CONCTI_HandleEvent() -- Handles event for inputter */
bool_t CONCTI_HandleEvent(CONCTI_Inputter_t* const a_Input, const I_EventEx_t* const a_Event)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
#define BUFSIZE 512
	uint8_t Code;
	uint16_t Char;
	int32_t i, j, OldSpot;
	char MB[5];
	char Buf[BUFSIZE];
	CONCTI_MBChain_t* MBRover, *Last;
	
	/* Not in dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Check */
	if (!a_Input || !a_Event)
		return false;
		
	/* Only handle keyboard events */
	if (a_Event->Type != IET_KEYBOARD)
		return false;
		
	// Ignore release/up events
	if (!a_Event->Data.Keyboard.Down)
		return false;
		
	// Remember key code and character
	Code = a_Event->Data.Keyboard.KeyCode;
	Char = a_Event->Data.Keyboard.Character;
	
	/* Which event occured? */
	switch (Code)
	{
			// Browse through history (if it exists)
		case IKBK_UP:
		case IKBK_DOWN:
			// Move input around
			if (Code == IKBK_UP)
				a_Input->HistorySpot++;
			else
				a_Input->HistorySpot--;
			
			// Cap
			if (a_Input->HistorySpot < -1)
				a_Input->HistorySpot = -1;
			else if (a_Input->HistorySpot >= (int)a_Input->NumHistory)
				a_Input->HistorySpot = a_Input->NumHistory - 1;
			
			// Set history?
			if (a_Input->HistorySpot >= 0 && a_Input->HistorySpot < a_Input->NumHistory)
				if (a_Input->History[a_Input->HistorySpot])
					CONCTI_SetText(a_Input, a_Input->History[a_Input->HistorySpot]);
			return true;
			
			// Move cursor left
		case IKBK_LEFT:
			if (a_Input->CursorPos > 0)
				a_Input->CursorPos--;
			return true;
			
			// Move cursor right
		case IKBK_RIGHT:
			if (a_Input->CursorPos < a_Input->NumMBs)
				a_Input->CursorPos++;
			return true;
			
			// Go to start of line
		case IKBK_HOME:
			a_Input->CursorPos = 0;
			return true;
			
			// Go to end of line
		case IKBK_END:
			a_Input->CursorPos = a_Input->NumMBs;
			return true;
			
			// Enter is pressed (send characters to handler)
		case IKBK_RETURN:
			// Clear output
			memset(Buf, 0, sizeof(Buf));
			
			// Go through, adding to buffer, then deleting
			MBRover = a_Input->ChainRoot;
			
			while (MBRover)
			{
				// Remember next
				Last = MBRover->Next;
				
				// Append
				strncat(Buf, MBRover->MB, BUFSIZE);
				
				// Delete
				Z_Free(MBRover);
				
				// Go to next
				MBRover = Last;
			}
			
			// Clear
			a_Input->ChainRoot = NULL;
			a_Input->CursorPos = a_Input->NumMBs = 0;
			
			// Reset history mark
			a_Input->HistorySpot = -1;
			
			// Free last history, if it exists
			if (a_Input->History[a_Input->NumHistory - 1])
			{
				Z_Free(a_Input->History[a_Input->NumHistory - 1]);
				a_Input->History[a_Input->NumHistory - 1] = NULL;
			}
			
			// Push histories down
			for (j = a_Input->NumHistory - 2; j > 0 ; j--)
				a_Input->History[j] = a_Input->History[j - 1];
			
			// Place buffer in current spot
			a_Input->History[0] = Z_StrDup(Buf, PU_STATIC, NULL);
			
			// Send buffer to handler
			if (a_Input->OutFunc)
				if (a_Input->OutFunc(a_Input, Buf))
					CONCTI_DestroyInput(a_Input);	// Done with this, so destroy
					
			// Unmark changed, cleared away
			a_Input->Changed = false;
			return true;
			
			// Delete Characters
		case IKBK_BACKSPACE:
		case IKBK_KDELETE:
			// Reset history mark
			a_Input->HistorySpot = -1;
			
			// Find character to delete
			j = a_Input->CursorPos - (Code == IKBK_KDELETE ? 0 : 1);
			
			for (MBRover = a_Input->ChainRoot, i = 0; MBRover; i++)
			{
				// Is this it?
				if (i == j)
					break;
					
				// Next
				MBRover = MBRover->Next;
			}
			
			// Found it, so unlink it
			if (MBRover)
			{
				// First character?
				if (MBRover == a_Input->ChainRoot)
					a_Input->ChainRoot = MBRover->Next;
					
				// Unlink from chain
				if (MBRover->Next)
					MBRover->Next->Prev = MBRover->Prev;
					
				if (MBRover->Prev)
					MBRover->Prev->Next = MBRover->Next;
					
				// Delete rover
				Z_Free(MBRover);
				
				// Change input stuff
				a_Input->NumMBs--;	// Deleted char
				a_Input->CursorPos = j;	// Set cursor pos to deleted spot
			}
			
			// Changed
			a_Input->Changed = true;
			return true;
			
			// Toggle Insertion/Overwrite mode
		case IKBK_INSERT:
			a_Input->Overwrite = !a_Input->Overwrite;
			return true;
			
			// Normal character or otherwise
		default:
			// Control or non-character key (fall through)
			if (!Char || (Char < 0x20 && Char != '\t'))
				return false;
				
			// Reset history mark
			a_Input->HistorySpot = -1;
				
			// Set to first in chain
			MBRover = a_Input->ChainRoot;
			
			// If there are no characters in the buffer, create one
			if (!MBRover)
			{
				MBRover = a_Input->ChainRoot = Z_Malloc(sizeof(*MBRover), PU_STATIC, NULL);
				a_Input->NumMBs++;	// Added more
			}
			// Append somewhere, somehow
			else
			{
				// Keep running until we hit the spot
				for (Last = NULL, i = 0; MBRover; i++)
				{
					// Did we hit it?
					if (i == a_Input->CursorPos)
						break;
						
					// Next
					Last = MBRover;
					MBRover = MBRover->Next;
				}
				
				// Attach to end?
				if (!MBRover)
				{
					MBRover = Last->Next = Z_Malloc(sizeof(*MBRover), PU_STATIC, NULL);
					MBRover->Prev = Last;	// link back
					a_Input->NumMBs++;	// Added
				}
					
				// Not overwriting
				else if (!a_Input->Overwrite)
				{
					Last = Z_Malloc(sizeof(*Last), PU_STATIC, NULL);
					
					// Shove current to the next spot
					Last->Next = MBRover;
					Last->Prev = MBRover->Prev;
					
					// Relink current
					MBRover->Prev = Last;
					
					// Relink before
					if (Last->Prev)
						Last->Prev->Next = Last;
						
					// This is the first!
					if (MBRover == a_Input->ChainRoot)
						a_Input->ChainRoot = Last;
						
					// Set to last
					MBRover = Last;
					
					a_Input->NumMBs++;	// Added
				}
			}
			
			// Set character info
			i = V_ExtWCharToMB(Char, MB);
			memset(MBRover->MB, 0, sizeof(MBRover->MB));
			strncpy(MBRover->MB, MB, i);
			a_Input->CursorPos++;	// Always increment
			
			// Changed
			a_Input->Changed = true;
			
			// Eat this event
			return true;
	}
	
	/* Fell through, do not eat */
	return false;
#undef BUFSIZE
#endif /* __REMOOD_DEDICATED */
}

/* CONCTI_SetText() -- Sets text for inputter */
void CONCTI_SetText(CONCTI_Inputter_t* const a_Input, const char* const a_Text)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	CONCTI_MBChain_t* MBRover, *Last;
	const char* p;
	size_t i, j;
	char MB[5];
	
	/* Not in dedicated server */
	if (g_DedicatedServer)
		return;
		
	/* Erase */
	while (a_Input->ChainRoot)
	{
		// Find character to delete
		j = 0;
	
		for (MBRover = a_Input->ChainRoot, i = 0; MBRover; i++)
		{
			// Is this it?
			if (i == j)
				break;
			
			// Next
			MBRover = MBRover->Next;
		}
	
		// Found it, so unlink it
		if (MBRover)
		{
			// First character?
			if (MBRover == a_Input->ChainRoot)
				a_Input->ChainRoot = MBRover->Next;
			
			// Unlink from chain
			if (MBRover->Next)
				MBRover->Next->Prev = MBRover->Prev;
			
			if (MBRover->Prev)
				MBRover->Prev->Next = MBRover->Next;
			
			// Delete rover
			Z_Free(MBRover);
		
			// Change input stuff
			a_Input->NumMBs--;	// Deleted char
			a_Input->CursorPos = j;	// Set cursor pos to deleted spot
		}
	}
	
	/* Create */
	for (p = a_Text; *p; p++)
	{
		// Set to first in chain
		MBRover = a_Input->ChainRoot;
	
		// If there are no characters in the buffer, create one
		if (!MBRover)
		{
			MBRover = a_Input->ChainRoot = Z_Malloc(sizeof(*MBRover), PU_STATIC, NULL);
			a_Input->NumMBs++;	// Added more
		}
		// Append somewhere, somehow
		else
		{
			// Keep running until we hit the spot
			for (Last = NULL, i = 0; MBRover; i++)
			{
				// Did we hit it?
				if (i == a_Input->CursorPos)
					break;
				
				// Next
				Last = MBRover;
				MBRover = MBRover->Next;
			}
		
			// Attach to end?
			if (!MBRover)
			{
				MBRover = Last->Next = Z_Malloc(sizeof(*MBRover), PU_STATIC, NULL);
				MBRover->Prev = Last;	// link back
				a_Input->NumMBs++;	// Added
			}
			
			// Not overwriting
			else if (!a_Input->Overwrite)
			{
				Last = Z_Malloc(sizeof(*Last), PU_STATIC, NULL);
			
				// Shove current to the next spot
				Last->Next = MBRover;
				Last->Prev = MBRover->Prev;
			
				// Relink current
				MBRover->Prev = Last;
			
				// Relink before
				if (Last->Prev)
					Last->Prev->Next = Last;
				
				// This is the first!
				if (MBRover == a_Input->ChainRoot)
					a_Input->ChainRoot = Last;
				
				// Set to last
				MBRover = Last;
			
				a_Input->NumMBs++;	// Added
			}
		}
	
		// Set character info
		i = V_ExtWCharToMB(*p, MB);
		memset(MBRover->MB, 0, sizeof(MBRover->MB));
		strncpy(MBRover->MB, MB, i);
		a_Input->CursorPos++;	// Always increment
	}
	
	// Changed
	a_Input->Changed = true;
#endif /* __REMOOD_DEDICATED */
}

/* CONCTI_DrawInput() -- Draws input text */
int32_t CONCTI_DrawInput(CONCTI_Inputter_t* const a_Input, const uint32_t a_Options, const int32_t a_x, const int32_t a_y, const int32_t a_x2)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	CONCTI_MBChain_t* MBRover;
	uint32_t Options, DefaultOptions;
	int32_t bx, x, j;
	bool_t GotCur, CurVirtual;
	uint16_t ThisChar, NextChar;
	
	/* Not in dedicated server */
	if (g_DedicatedServer)
		return 0;
	
	/* Check */
	if (!a_Input)
		return 0;
	
	/* Default check */
	DefaultOptions = a_Options & (VFO_COLORMASK | VFO_TRANSMASK);
	
	/* Do virtualization of character representations */
	// This is for displaying actual colorization and effects, etc.
	if (a_Input->Changed)
	{
		MBRover = a_Input->ChainRoot;
		
		// While going through it
		while (MBRover)
		{
			// Clear virtual
			MBRover->EnableVirtual = false;
			
			// No character next to it?
			if (!MBRover->Next)
				break;
			
			// Convert current and next to wchar
			ThisChar = V_ExtMBToWChar(MBRover->MB, NULL);
			NextChar = tolower(V_ExtMBToWChar(MBRover->Next->MB, NULL));
			
			// Is this character { and the next character legal sequence?
			if (ThisChar == '{' && ((NextChar >= '0' && NextChar <= '9') || (NextChar >= 'a' && NextChar <= 'z')))
			{
				// Convert to code
				if (NextChar >= '0' && NextChar <= '9')
					NextChar = 0xF100U | (NextChar - '0');
				else
					NextChar = 0xF100U | (10 + (NextChar - 'a'));
				
				// Set virtuals
				MBRover->EnableVirtual = MBRover->Next->EnableVirtual = true;
				V_ExtWCharToMB(NextChar, MBRover->VirtualMB);
				V_ExtWCharToMB(0, MBRover->Next->VirtualMB);
				
				// Skip next
				MBRover = MBRover->Next;
			}
			
			// Go to next
			MBRover = MBRover->Next;
		}
		
		// Unmark changed
		a_Input->Changed = false;
	}
		
	/* Draw command text */
	MBRover = a_Input->ChainRoot;
	Options = a_Options;
	x = bx = a_x;
	j = 0;
	GotCur = false;
	CurVirtual = false;
	
	// While there is a multibyte char
	while (MBRover)
	{
		// If this is the cursor position, remember it
		if (a_Input->CursorPos == j)
		{
			bx = x;
			GotCur = true;
			CurVirtual = MBRover->EnableVirtual;
		}
		
		// Draw it
		x += V_DrawCharacterMB(a_Input->Font, Options, (MBRover->EnableVirtual ? MBRover->VirtualMB : MBRover->MB), x, a_y, NULL, &Options);
		
		// If a virtual character was drawn here, then replace it with a ?
		if (MBRover->EnableVirtual)
			x += V_DrawCharacterMB(a_Input->Font, Options, "?", x, a_y, NULL, &Options);
		
		// If no color/trans is set, set default
		if (DefaultOptions)
			if (!(Options & (VFO_COLORMASK | VFO_TRANSMASK)))
				Options |= DefaultOptions;
		
		// Go to next character in chain
		MBRover = MBRover->Next;
		j++;
	}
	
	// Cursor never set?
	if (!GotCur)
		bx = x;
		
	// Draw box/underscore here (blinked)
	Options = a_Options;
	Options &= ~VFO_COLORMASK;
	Options |= VFO_COLOR(VEX_MAP_BRIGHTWHITE);
	
	if ((gametic >> 4) & 1)
		V_DrawCharacterA(a_Input->Font, Options, (a_Input->Overwrite ? 0x7F : '_'), bx, a_y);
		
#endif /* __REMOOD_DEDICATED */
}

/*** Static Stuff ***/

/* CONLS_InitConsole() -- Initializes a buffer */
static bool_t CONLS_InitConsole(CONL_BasicBuffer_t* const a_Buffer, const uint32_t a_Size, const CONL_FlushFunc_t a_FlushFunc)
{
	uint32_t Size, i;
	
	/* Check */
	if (!a_Buffer || !a_Size)
		return false;
		
	/* Clear */
	memset(a_Buffer, 0, sizeof(*a_Buffer));
	
	/* Limit size to power of 2 */
	for (i = (sizeof(a_Size) * CHAR_BIT) - 1; i > 0 && !(a_Size & (1 << i)); i--)
		;
	Size = 1 << i;
	
	/* Allocate */
	// Buffer
	a_Buffer->Size = Size;
	a_Buffer->MaskPos = a_Buffer->Size - 1;
	a_Buffer->Buffer = Z_Malloc(sizeof(*a_Buffer->Buffer) * a_Buffer->Size, PU_STATIC, NULL);
	
	// Lines
	a_Buffer->NumLines = 128;
	a_Buffer->MaskLine = a_Buffer->NumLines - 1;
	a_Buffer->Lines = Z_Malloc(sizeof(*a_Buffer->Lines) * a_Buffer->NumLines, PU_STATIC, NULL);
	
	// Extra
	a_Buffer->FlushFunc = a_FlushFunc;
	
	// Create initial line
	a_Buffer->Lines[a_Buffer->EndLine++] = a_Buffer->Buffer;
	
	/* Success */
	return true;
}

/* CONLS_DestroyConsole() -- Destroys a console */
static void CONLS_DestroyConsole(CONL_BasicBuffer_t* const a_Buffer)
{
	/* Check */
	if (!a_Buffer)
		return;
		
	/* Free Everything */
	// Buffer
	if (a_Buffer->Buffer)
		Z_Free(a_Buffer->Buffer);
		
	// Lines
	if (a_Buffer->Lines)
		Z_Free(a_Buffer->Lines);
		
	/* Final memset */
	memset(a_Buffer, 0, sizeof(*a_Buffer));
}

/*** Flush Functions ***/

/* CONLFF_OutputFF() -- Line is flushed from the output buffer */
// Parsing of all the lines should be done here, that is, if it starts with
// the following:
//   \1 = Beep
//   \2 = White
//   \3 = White + Beep
//   \4 = Player 2 Screen
//   \5 = Player 3 Screen (ReMooD)
//   \6 = Player 4 Screen (ReMooD)
//   \7 = All Screens (ReMooD) (\a)
// So if there are any screen modifiers, they are sent to screen specific
// buffers
extern bool_t con_started;
static void CONLFF_OutputFF(const char* const a_Buf)
{
	const char* p;
	char* d;
	char Buf[MAXCONLPMQBUFSIZE];
	size_t i, j, pNum, n;
	tic_t CurrentTime;
	
	/* Check */
	if (!a_Buf)
		return;

	/* Default destination is player 1's buffer */
	pNum = 1;
	memset(Buf, 0, sizeof(Buf));
	n = 0;
	
	/* Parse string */
	for (p = a_Buf; *p; p++)
	{
		// Annoying beep?
		if (*p == 1 || *p == 3)
		{
#if !defined(__REMOOD_DEDICATED)
			if (!g_DedicatedServer)
				S_StartSound(0, (commercial ? sfx_radio : sfx_tink));
#endif /* __REMOOD_DEDICATED */
			
			// Beep only
			if (*p == 1)
				continue;
		}
		
		// Adding white?
		if (*p == 2 || *p == 3)
		{
			strncat(Buf, "{9", MAXCONLPMQBUFSIZE);
			n += 2;
			continue;
		}
		
		// Only player x's screen
		if (*p >= 4 && *p <= 6)
		{
			pNum = 1 << ((*p - 4) + 1);
			continue;
		}
		
		// Every player's screen
		if (*p == 7)
		{
			pNum = 0xF;
			continue;
		}
		
		// Output normal text
		if (n < MAXCONLPMQBUFSIZE)
			Buf[n++] = *p;
	}
	
	// Always add \0 at end (just in case!)
	Buf[MAXCONLPMQBUFSIZE - 1] = 0;
	
	/* Print text to console */
	// Early Boot
	if (g_EarlyBootConsole)
		CONL_EarlyBootTic(Buf, true);
	
#if !defined(_DEBUG)
	if (devparm || !con_started || g_DedicatedServer)
	{
		I_OutputText(Buf);
		I_OutputText("\n");
	}
#endif

#if !defined(__REMOOD_DEDICATED)
	/* Not in dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Add messages to queues */
	CurrentTime = I_GetTime();
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (pNum & (1 << i))	// Only if selected
		{
			// Find blank spot
			for (j = 0; j < MAXCONLPLAYERMQ; j++)
				if (!l_CONLMessageQ[i][j].Text[0])
					break;
					
			// No more room?
			if (j == MAXCONLPLAYERMQ)
			{
				// Shove everything over, clear last, take last spot
				memmove(&l_CONLMessageQ[i][0], &l_CONLMessageQ[i][1], sizeof(CONL_PlayerMessage_t) * (MAXCONLPLAYERMQ - 1));
				memset(&l_CONLMessageQ[i][MAXCONLPLAYERMQ - 1], 0, sizeof(CONL_PlayerMessage_t));
				j = MAXCONLPLAYERMQ - 1;
			}
			// Set timeout to default (4 secs) and priority to default
			l_CONLMessageQ[i][j].Timeout = CurrentTime + DEFTOCONPLAYERMQ;
			l_CONLMessageQ[i][j].Priority = DEFPRCONLPLAYERMQ;
			
			// Append text
			l_CONLMessageQ[i][j].Flags = VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES;
			strncat(l_CONLMessageQ[i][j].Text, Buf, MAXCONLPMQBUFSIZE);
		}
#endif /* __REMOOD_DEDICATED */
}

/* CONLFF_InputFF() -- Line is flushed from the input buffer */
static void CONLFF_InputFF(const char* const a_Buf)
{
#define MAXARGV 128
	const char* p;
	const char* m;
	const char* n;
	int ArgC, Current, i, j;
	char** ArgV;
	char Quote;
	CONL_ExitCode_t ec;
	
	/* Check */
	if (!a_Buf)
		return;
		
	/* Create command line like stuff here */
	// Tokenize between space and single/double quotes
	// This makes command processing hell of alot easier
	// So calling console functions is like main(argc, argv)
	// Handle ; in inputs
	for (n = a_Buf; *n;)
	{
		ArgC = Current = 0;
		ArgV = NULL;
		m = n;
		
		// Determine ; location (but not inside quotes)
		for (Quote = 0; *n && (Quote || (!Quote && *n != ';')); n++)
			if (*n == '\"' || *n == '\'')
				if (Quote)
					Quote = 0;
				else
					Quote = *n;
	
		// Run loop
		for (Quote = 0, p = m; p != n; p++)
		{
			// Check if a new thing needs to be created
			if (Current == ArgC)
			{
				// Resize
				Z_ResizeArray((void**)&ArgV, sizeof(*ArgV), ArgC, ArgC + 1);
				ArgC++;
			
				// Create text here
				j = 0;
				ArgV[Current] = Z_Malloc(sizeof(*ArgV[Current]) * MAXARGV, PU_STATIC, NULL);
				Quote = 0;
			}
			
			// Whitespace adds to current
			if (!Quote && (*p == 0 || *p == ' ' || *p == '\t'))
			{
				// But only if there is j
				if (j > 0)
					Current++;
			}
			
			// Quote?
			else if ((!Quote && (*p == '\"' || *p == '\'')) || (Quote && *p == Quote))
			{
				// Toggle quote
				if (Quote)
					Quote = 0;
				else
					Quote = *p;
			}
			
			// Normal character?
			else
			{
				if (j < MAXARGV - 1)
					ArgV[Current][j++] = *p;
			}
		}
	
		/* Send to execution handler */
		if (ArgC)
		{
			ec = CONL_Exec(ArgC, ArgV);
			
			if (ec)
				CONL_OutputF("{b%s{z\n", CONL_ExitCodeToStr(ec));
		}
	
		/* Clear */
		if (ArgV)
		{
			for (i = 0; i < ArgC; i++)
				Z_Free(ArgV[i]);
			Z_Free(ArgV);
			ArgV = NULL;
		}
		ArgC = 0;
		
		// Increment to prevent being stuck
		if (*n == ';')
			n++;
	}
#undef MAXARGV
}

/*** Base Console ***/

size_t CONL_RawPrint(CONL_BasicBuffer_t* const a_Buffer, const char* const a_Text);

/* CONL_OutBack() -- \n is pressed from inputter */
static bool_t CONL_OutBack(struct CONCTI_Inputter_s* a_Input, const char* const a_Buf)
{
	/* Append to execution buffer */
	CONL_RawPrint(&l_CONLBuffers[1], a_Buf);
	CONL_RawPrint(&l_CONLBuffers[1], "\n");	// \n is always missing
	
	/* Never destroy */
	return false;
}

void P_InitSGConsole(void);

/* CONLS_ExitFunc() -- Exit function */
static void CONLS_ExitFunc(void)
{
	/* Save default config */
	CONL_SaveConfigFile(l_DefaultConfig);
}

/* CONL_Init() -- Initializes the light console */
bool_t CONL_Init(const uint32_t a_OutBS, const uint32_t a_InBS)
{
	/* Initialize output */
	if (!CONLS_InitConsole(&l_CONLBuffers[0], a_OutBS, CONLFF_OutputFF))
		return false;
		
	/* Initialize input */
	if (!CONLS_InitConsole(&l_CONLBuffers[1], a_InBS, CONLFF_InputFF))
	{
		CONLS_DestroyConsole(&l_CONLBuffers[0]);
		return false;
	}
	
	// Set ; to extra newline
	l_CONLBuffers[1].ExtraNewLine = ';';
	
	/* Create input */
	l_CONLInputter = CONCTI_CreateInput(128, CONL_OutBack, &l_CONLInputter);
	
	/* Add Commands */
	CONL_AddCommand("version", CLC_Version);
	CONL_AddCommand("dep", CLC_Dep);
	CONL_AddCommand("exec", CLC_Exec);
	CONL_AddCommand("execfile", CLC_ExecFile);
	CONL_AddCommand("!", CLC_Exclamation);
	CONL_AddCommand("?", CLC_Question);
	
	/* Initialize the variable system */
	CONL_VarLocate("theconsolesystemwasjustbooted");
	
	/* Get directories for files */
	I_GetStorageDir(l_ConfigDir, PATH_MAX, DST_CONFIG);
	I_GetStorageDir(l_DataDir, PATH_MAX, DST_DATA);
	
	// Print the info about them
	CONL_PrintF("CONL_Init: Config directory is \"%s\".\n", l_ConfigDir);
	CONL_PrintF("CONL_Init: Data directory is \"%s\".\n", l_DataDir);
	
	/* Load configuration file */
	if (CONL_FindDefaultConfig())
		CONL_LoadConfigFile(l_DefaultConfig);
	
	/* Add exit function */
	// To save the config file when the game exits
	I_AddExitFunc(CONLS_ExitFunc);
	
	/* Initialize variables for drawing */
#if !defined(__REMOOD_DEDICATED)
	if (!g_DedicatedServer)
	{
		CONL_VarRegister(&l_CONScreenHeight);
		CONL_VarRegister(&l_CONBackColor);
		CONL_VarRegister(&l_CONFont);
		CONL_VarRegister(&l_CONMonoSpace);
		CONL_VarRegister(&l_CONScale);
		CONL_VarRegister(&l_CONTestString);
	}
#endif

	/* Add other things */
	Z_RegisterCommands();						// Memory Manager
	
	/* Base init */
	P_InitSGConsole();
	P_InitSetupEx();
	
	/* Success! */
	return true;
}

/* CONL_Stop() -- Stops the console */
void CONL_Stop(void)
{
	/* Destroy console input */
	CONCTI_DestroyInput(l_CONLInputter);
	l_CONLInputter = NULL;
}

/* CONL_RawPrint() -- Prints text directly to buffer */
size_t CONL_RawPrint(CONL_BasicBuffer_t* const a_Buffer, const char* const a_Text)
{
#define POSMASK(x) ((x) & a_Buffer->MaskPos)
#define LINEMASK(x) ((x) & a_Buffer->MaskLine)
	char** Que;
	size_t RetVal, i, j, k, Q, NumQ;
	const char* p;
	char* z, *w;
	
	/* Check */
	if (!a_Buffer || !a_Text || !a_Buffer->Buffer || !a_Buffer->Lines)
		return 0;
		
	/* Splatter into buffer */
	Q = NumQ = 0;
	Que = NULL;
	for (RetVal = 0, p = a_Text; *p; p++, RetVal++)
	{
		// If the character is a carraige return, go back to start of line
		if (*p == '\r')
		{
			// Go to the start of the line, if possible
			if (a_Buffer->Lines[LINEMASK(a_Buffer->EndLine - 1)])
				a_Buffer->EndLine = a_Buffer->Lines[LINEMASK(a_Buffer->EndLine - 1)] - a_Buffer->Buffer;
				
			// Don't add it to the buffer
			continue;
		}
		// Put character in last spot
		a_Buffer->Buffer[POSMASK(a_Buffer->EndPos)] = *p;
		a_Buffer->EndPos = POSMASK(a_Buffer->EndPos + 1);
		
		// Unset character in current spot
		a_Buffer->Buffer[POSMASK(a_Buffer->EndPos)] = 0;
		
		// Start collision?
		if (POSMASK(a_Buffer->EndPos) == POSMASK(a_Buffer->StartPos))
		{
			// Check for line overflow
			if (a_Buffer->Lines[LINEMASK(a_Buffer->StartLine)] == &a_Buffer->Buffer[POSMASK(a_Buffer->StartPos)])
			{
				// Set current line to NULL and move up
				a_Buffer->Lines[LINEMASK(a_Buffer->StartLine)] = NULL;
				a_Buffer->StartLine = LINEMASK(a_Buffer->StartLine + 1);
			}
			
			a_Buffer->StartPos = POSMASK(a_Buffer->StartPos + 1);
			//&a_Buffer->Buffer[a_Buffer->StartPos] >= a_Buffer->Lines[a_Buffer->StartLine];
		}
		// Newline?
		if (*p == '\n'/* || (a_Buffer->ExtraNewLine && *p == a_Buffer->ExtraNewLine)*/)
		{
			// Add to Q (Formerly, sending to the flush was done later but the
			// lines sent to it must be remembered now. This prevents any
			// overflows (deliberate or not) in the console code that would
			// disallow early commands to be executed or when mass console
			// is printed)
			if (a_Buffer->FlushFunc)
			{
				// Resize BigQ
				Z_ResizeArray((void**)&Que, sizeof(Que), NumQ, NumQ + 1);
				NumQ = Q + 1;
				
				// Find size of line to write
				z = &a_Buffer->Buffer[POSMASK(a_Buffer->EndPos)];	// EOL
				w = a_Buffer->Lines[LINEMASK(a_Buffer->EndLine - 1)];	// SOL
				
				if (!w)			// Just use first character (ouch)
					w = &a_Buffer->Buffer[0];
					
				// Wrapped
				if (z < w)
					j = (z + a_Buffer->Size) - w;
				// Not wrapped
				else
					j = z - w;
					
				// Only queue strings if they have length
				if (j > 1)
				{
					// Allocate buffer
					Que[Q] = Z_Malloc(j + 2, PU_STATIC, NULL);
					
					// Copy chars into buffer
					i = POSMASK(w - a_Buffer->Buffer);
					for (k = 0; k < j - 1; i = POSMASK(i + 1))
						Que[Q][k++] = a_Buffer->Buffer[POSMASK(i)];
					Que[Q][k++] = 0;
					
					// Increase Q
					Q++;
				}
			}
			// Write current line to buffer
			a_Buffer->Lines[LINEMASK(a_Buffer->EndLine)] = &a_Buffer->Buffer[POSMASK(a_Buffer->EndPos)];
			a_Buffer->EndLine = LINEMASK(a_Buffer->EndLine + 1);
			
			// Unset current line to prevent old corruption
			a_Buffer->Lines[LINEMASK(a_Buffer->EndLine)] = NULL;
			
			// Start collision?
			if (LINEMASK(a_Buffer->EndLine) == LINEMASK(a_Buffer->StartLine))
				a_Buffer->StartLine = LINEMASK(a_Buffer->StartLine + 1);
		}
	}
	
	/* Normalize line count */
	a_Buffer->StartLine = LINEMASK(a_Buffer->StartLine);
	a_Buffer->EndLine = LINEMASK(a_Buffer->EndLine);
	if (a_Buffer->EndLine < a_Buffer->StartLine)
		a_Buffer->CountLine = (a_Buffer->EndLine + a_Buffer->NumLines) - a_Buffer->StartLine;
	else
		a_Buffer->CountLine = a_Buffer->EndLine - a_Buffer->StartLine;
		
	/* print Q */
	if (a_Buffer->FlushFunc)
		if (Que)
		{
			for (i = 0; i < Q; i++)
			{
				// Check
				if (!Que[i])
					continue;
					
				a_Buffer->FlushFunc(Que[i]);
				
				// Free character buffer
				Z_Free(Que[i]);
			}
			
			// Free ques
			Z_Free(Que);
		}
		
	return RetVal;
#undef POSMASK
#undef LINEMASK
}

/* CONL_PrintF() -- Prints formatted text to console */
size_t CONL_PrintF(const char* const a_Format, ...)
{
	va_list ArgPtr;
	size_t RetVal;
	static bool_t AlreadyDrawn = false;
	bool_t Drew;
	
	/* Check */
	if (!a_Format)
		return 0;
	
	/* Obtain format, print to first console */
	va_start(ArgPtr, a_Format);
	RetVal = CONL_PrintV(false, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	/* Console just starting up?  */
	if (con_startup || g_EarlyBootConsole)
	{
		// GhostlyDeath <November 4, 2010> -- If we aren't devparming, draw once
		if ((devparm && !g_QuietConsole) || ((!devparm || g_QuietConsole) && !AlreadyDrawn))
		{
			Drew = CONL_DrawConsole();
			
			if ((!g_EarlyBootConsole && Drew) || g_EarlyBootConsole)
				I_FinishUpdate();	// page flip or blit buffer
			
			if (!g_EarlyBootConsole)
				AlreadyDrawn = true;
		}
	}
	
	/* Return value */
	return RetVal;
}

/* CONL_PrintV() -- Prints formatted text to buffer */
size_t CONL_PrintV(const bool_t a_InBuf, const char* const a_Format, va_list a_ArgPtr)
{
#define BUFSIZE 512
	size_t RetVal;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Format)
		return 0;
		
	/* Print to buffer */
	memset(Buf, 0, sizeof(Buf));
	vsnprintf(Buf, BUFSIZE, a_Format, a_ArgPtr);
	RetVal = CONL_RawPrint(&l_CONLBuffers[(a_InBuf ? 1 : 0)], Buf);
	
	/* Debug print here */
#if defined(_DEBUG)
	if (devparm || !con_started || g_DedicatedServer)
	{
		I_OutputText(Buf);
	}
#endif
	
	/* Return */
	return RetVal;
#undef BUFSIZE
}

/* CONL_UnicodePrintV() -- Prints localized text to a buffer */
size_t CONL_UnicodePrintV(const bool_t a_InBuf, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr)
{
	return 0;
}

/* CONL_OutputF() -- Prints formatted text to the output buffer */
size_t CONL_OutputF(const char* const a_Format, ...)
{
	size_t RetVal;
	va_list ArgPtr;
	
	/* Send to PrintV() */
	va_start(ArgPtr, a_Format);
	RetVal = CONL_PrintV(false, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	/* Return */
	return RetVal;
}

/* CONL_InputF() -- Prints formatted text to the input buffer */
size_t CONL_InputF(const char* const a_Format, ...)
{
	size_t RetVal;
	va_list ArgPtr;
	
	/* Send to PrintV() */
	va_start(ArgPtr, a_Format);
	RetVal = CONL_PrintV(true, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	/* Return */
	return RetVal;
}

/* CONL_OutputU() -- Prints localized text to the output buffer */
size_t CONL_OutputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...)
{
	size_t RetVal;
	va_list ArgPtr;
	
	/* Send to PrintV() */
	va_start(ArgPtr, a_Format);
	RetVal = CONL_UnicodePrintV(false, a_StrID, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	/* Return */
	return RetVal;
}

/* CONL_InputU() -- Prints localized text to the input buffer */
size_t CONL_InputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...)
{
	size_t RetVal;
	va_list ArgPtr;
	
	/* Send to PrintV() */
	va_start(ArgPtr, a_Format);
	RetVal = CONL_UnicodePrintV(true, a_StrID, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	/* Return */
	return RetVal;
}

/*** Client Drawing ***/

/* CONL_IsActive() -- Returns true if the console is active */
bool_t CONL_IsActive(void)
{
	return (l_CONLActive && !M_ExUIActive()) || con_startup;
}

/* CONL_SetActive() -- Sets whether the console is active or not */
// Returns the old flag
bool_t CONL_SetActive(const bool_t a_Set)
{
	bool_t Ret;
	
	Ret = l_CONLActive;
	l_CONLActive = a_Set;
	return Ret;
}

/* CONL_Ticker() -- Tick the console */
void CONL_Ticker(void)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
#if 0
	const static uint8_t Faders[TICRATE] =
	{
		VEX_TRANSFULL, VEX_TRANSFULL, VEX_TRANSFULL, VEX_TRANS90, VEX_TRANS90, VEX_TRANS90,
		VEX_TRANS80, VEX_TRANS80, VEX_TRANS80, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70,
		VEX_TRANS60, VEX_TRANS60, VEX_TRANS60, VEX_TRANS60, VEX_TRANS50, VEX_TRANS50,
		VEX_TRANS50, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS30, VEX_TRANS30,
		VEX_TRANS30, VEX_TRANS30, VEX_TRANS20, VEX_TRANS20, VEX_TRANS20, VEX_TRANS10,
		VEX_TRANS10, VEX_TRANS10, VEX_TRANSNONE, VEX_TRANSNONE, VEX_TRANSNONE
	};
#elif 1
	const static uint8_t Faders[TICRATE] =
	{
		VEX_TRANS90, VEX_TRANS90, VEX_TRANS90,
		VEX_TRANS80, VEX_TRANS80, VEX_TRANS80, VEX_TRANS80,
		VEX_TRANS70, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70,
		VEX_TRANS60, VEX_TRANS60, VEX_TRANS60, VEX_TRANS60,
		VEX_TRANS50, VEX_TRANS50, VEX_TRANS50, VEX_TRANS50,
		VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40,
		VEX_TRANS30, VEX_TRANS30, VEX_TRANS30, VEX_TRANS30,
		VEX_TRANS20, VEX_TRANS20, VEX_TRANS20, VEX_TRANS20,
		VEX_TRANS10, VEX_TRANS10, VEX_TRANS10, VEX_TRANS10,
	};
#else
	const static uint8_t Faders[TICRATE] =
	{
		VEX_TRANS90, VEX_TRANS90, VEX_TRANS90, VEX_TRANS80, VEX_TRANS80, VEX_TRANS80,
		VEX_TRANS80, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70, VEX_TRANS60,
		VEX_TRANS60, VEX_TRANS60, VEX_TRANS60, VEX_TRANS50, VEX_TRANS50, VEX_TRANS50,
		VEX_TRANS50, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS30,
		VEX_TRANS30, VEX_TRANS30, VEX_TRANS30, VEX_TRANS20, VEX_TRANS20, VEX_TRANS20,
		VEX_TRANS20, VEX_TRANS10, VEX_TRANS10, VEX_TRANS10, VEX_TRANSNONE
	};
#endif
	size_t i, j;
	tic_t CurrentTime, Left;
	uint8_t v;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Remove old messages */
	CurrentTime = I_GetTime();
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		for (j = 0; j < MAXCONLPLAYERMQ; j++)
			if (l_CONLMessageQ[i][j].Text[0])
			{
				// Expired?
				if (CurrentTime >= l_CONLMessageQ[i][j].Timeout)
				{
					// Wipe ahead back
					memmove(&l_CONLMessageQ[i][j], &l_CONLMessageQ[i][j + 1], sizeof(CONL_PlayerMessage_t) * (MAXCONLPLAYERMQ - (j + 1)));
					
					// Clear back
					memset(&l_CONLMessageQ[i][MAXCONLPLAYERMQ - 1], 0, sizeof(CONL_PlayerMessage_t));
				}
				
				// Soon to expire? Transparent fade away
				else if ((Left = (l_CONLMessageQ[i][j].Timeout - CurrentTime)) <= TICRATE)
				{
					// Cap left?
					if (Left >= TICRATE)
						Left = TICRATE - 1;
					
					l_CONLMessageQ[i][j].Flags &= ~VFO_TRANSMASK;
					l_CONLMessageQ[i][j].Flags |= VFO_TRANS((uint32_t)Faders[Left]);
				}
			}
#endif /* __REMOOD_DEDICATED */
}

/* CONL_DrawMouse() -- Draws the mouse */
void CONL_DrawMouse(void)
{
	V_Image_t* CurPic;
	
	CurPic = V_ImageFindA("MOUSECUR");
	
	if (CurPic)
		V_ImageDraw(0, CurPic, g_MousePos[0], g_MousePos[1], NULL);
}

/* CONL_HandleEvent() -- Handles extended event for the console */
bool_t CONL_HandleEvent(const I_EventEx_t* const a_Event)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	uint8_t Code;
	uint16_t Char;
	uint32_t Old;
	static int32_t HistorySpot = -1;
	size_t i, j, k;
	const char* p;

	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Check */
	if (!a_Event)
		return false;
		
	/* Don't handle events during startup */
	if (con_startup || g_EarlyBootConsole)
		return false;
	
	/* Update Mouse Position */
	if (a_Event->Type == IET_MOUSE)
	{
		g_MouseDown = (a_Event->Data.Mouse.Down && a_Event->Data.Mouse.Button == 1);
		g_MousePos[0] = FixedMul(a_Event->Data.Mouse.Pos[0] << FRACBITS, vid.fxdivx) >> FRACBITS;
		g_MousePos[1] = FixedMul(a_Event->Data.Mouse.Pos[1] << FRACBITS, vid.fxdivy) >> FRACBITS;
	}
	
	/* Handle Virtual OSK Events? */
	if (CONL_IsActive())
	{
		CONL_OSKSetVisible(0, true);
		if (CONL_OSKHandleEvent(a_Event, 0))
			return true;
	}
	else
		CONL_OSKSetVisible(0, false);
		
	/* Only handle keyboard events */
	if (a_Event->Type != IET_KEYBOARD)
		return false;
		
	// Ignore release/up events
	if (!a_Event->Data.Keyboard.Down)
		return false;
		
	// Remember key code and character
	Code = a_Event->Data.Keyboard.KeyCode;
	Char = a_Event->Data.Keyboard.Character;
	
	/* Console is not active */
	if (!CONL_IsActive() && (Code == IKBK_TILDE || Code == IKBK_GRAVE))
	{
		CONL_SetActive(true);
		return true;
	}
	
	/* Console is active */
	else if (CONL_IsActive())
	{
		// Deactivate
		if (Code == IKBK_ESCAPE)
		{
			CONL_SetActive(false);
			return true;
		}
		
		// Scroll console up
		else if (Code == IKBK_PAGEUP)
		{
			if (l_CONLLineOff < l_CONLBuffers[0].NumLines)
				l_CONLLineOff++;
		}
		
		// Scroll console down
		else if (Code == IKBK_PAGEDOWN)
		{
			if (l_CONLLineOff > 0)
				l_CONLLineOff--;
		}
		
		// Handle input line
		else
		{
			if (CONCTI_HandleEvent(l_CONLInputter, a_Event))
				return true;		// it ate the event
		}
			
		// Always eat everything
		return true;
	}
	
	/* Fell through */
	return false;
#endif /* __REMOOD_DEDICATED */
}

static uint32_t l_EarlyBootTicNum = 0;			// Early tic number

/* CONL_EarlyBootTic() -- Tic Early Boot */
void CONL_EarlyBootTic(const char* const a_Message, const bool_t a_DoTic)
{
#define BUFSIZE 40
	static char EarlyBuf[BUFSIZE];
	static bool_t DrewLogo;
	char* c;
	size_t i, j, x, y, r, yB;
	uint8_t BMPc, Char;
	uint32_t BarBits;
	fixed_t Frac;
	
	/* Not in early boot console? */
	if (!g_EarlyBootConsole)
		return;
	
	/* Tick it */
	if (a_DoTic)
	{
		// Tic Counter
		l_EarlyBootTicNum++;
	
		// Set Message to draw
		if (a_Message)
		{
			memset(EarlyBuf, 0, sizeof(EarlyBuf));
			strncpy(EarlyBuf, a_Message, BUFSIZE - 1);
		}
	}
	
	/* Copy background */
	if (!DrewLogo)
		for (x = 62, y = 0, i = 0; i < CBOOTLOGOSIZE; i++)
		{
			// Get Bit Count
			BMPc = c_BootLogo[i++];
		
			// Draw to screen
			while (BMPc--)
			{
				screens[0][(vid.rowbytes * y) + (x++)] = ((c_BootLogo[i] & 0x0F)) + 1;
				screens[0][(vid.rowbytes * y) + (x++)] = ((c_BootLogo[i] & 0xF0) >> 4) + 1;
			
				if (x >= 198 + 62)
				{
					x = 62;
					y++;
				}
			}
		}
	
	// Logo is now drawn
	DrewLogo = true;
	
	/* Black behind text */
	if (a_Message)
	{
		// Draw bar across entire screen (white for incomplete)
		V_DrawColorBoxEx(0, 0, 0, 160, 320, 168);
	
		/* Draw Message Text */
		yB = 160;	// Base near the bottom somewhere
		for (x = 0, c = EarlyBuf; *c; c++, x += 8)
		{
			// Get better character
			Char = toupper(*c) - '!';
		
			// Illegal?
			if (Char < 0 || Char >= 64)
				continue;
		
			// Draw character
			for (r = 0, j = 0; j < 8; j++)
				for (y = yB, r = 0; r < 8; r++)
					if (c_OEMFont[((int)Char) + (64 * r)] & (1 << j))
						screens[0][(vid.rowbytes * (y++)) + (x + j)] = 5;
					else
						y++;
		}
	}
	
	/* Draw Bar */
#define SIDESPACE 5
#define BASEBARY (170)
#define BASEBARSIZE 10
	// Draw bar across entire screen (white for incomplete)
	V_DrawColorBoxEx(0, 5, SIDESPACE, BASEBARY, SIDESPACE + (320 - (SIDESPACE * 2)), BASEBARY + BASEBARSIZE);
	
	// Draw bar
	V_DrawColorBoxEx(0, 6,
			SIDESPACE + ((20 * ((int32_t)l_EarlyBootTicNum)) % 300),
			BASEBARY,
			SIDESPACE + ((20 * ((int32_t)l_EarlyBootTicNum)) % 300) + 20,
			BASEBARY + BASEBARSIZE
		);
#undef BASEBARSIZE
#undef BASEBARY
#undef SIDESPACE
	
	/* Update Screen */
	I_FinishUpdate();
#undef BUFSIZE
}

/* l_OSKLayout -- On Screen Keyboard */
// --- STANDARD BOARD ---        -- EXTRA --
// ` 1 2 3 4 5 6 7 8 9 0 - =  BP IN HM PU 14/3 keys 
// T  q w e r t y u i o p [ ] \  DL EN PD 14/3 keys
// CL  a s d f g h j k l ; ' RET          13/0 keys
// SH   z x c v b n m , . /   SH    UA    12/1 keys
// CT AL      space        AL CT LA DA RA 5/3 keys
#define LOSKROWS 5
#define LOSKCOLS 17
static const struct
{
	// 0 = Lower, 1 = Caps, 2 = International
		// Remember key codes are used for input, UTF is for character input!
	int8_t KeySize;
	int8_t VirtOnly;
	I_KeyBoardKey_t IkbkKey[3];
	char* UTFKey[3];
} l_OSKLayout[LOSKROWS][LOSKCOLS] =	//
{
	// ` 1 2 3 4 5 6 7 8 9 0 - =  BP IN HM PU 14/3 keys 
	{
		{1, 0, {IKBK_GRAVE, IKBK_TILDE, IKBK_NULL}, {"`", "~", ""}},
		{1, 0, {IKBK_1, IKBK_EXCLAIM, IKBK_NULL}, {"1", "!", ""}},
		{1, 0, {IKBK_2, IKBK_AT, IKBK_NULL}, {"2", "@", ""}},
		{1, 0, {IKBK_3, IKBK_HASH, IKBK_NULL}, {"3", "#", ""}},
		{1, 0, {IKBK_4, IKBK_DOLLAR, IKBK_NULL}, {"4", "$", ""}},
		{1, 0, {IKBK_5, IKBK_PERCENT, IKBK_NULL}, {"5", "%", ""}},
		{1, 0, {IKBK_6, IKBK_CARET, IKBK_NULL}, {"6", "^", ""}},
		{1, 0, {IKBK_7, IKBK_AMPERSAND, IKBK_NULL}, {"7", "&", ""}},
		{1, 0, {IKBK_8, IKBK_ASTERISK, IKBK_NULL}, {"8", "*", ""}},
		{1, 0, {IKBK_9, IKBK_LEFTPARENTHESES, IKBK_NULL}, {"9", "(", ""}},
		{1, 0, {IKBK_0, IKBK_RIGHTPARENTHESES, IKBK_NULL}, {"0", ")", ""}},
		{1, 0, {IKBK_MINUS, IKBK_UNDERSCORE, IKBK_NULL}, {"-", "_", ""}},
		{1, 0, {IKBK_EQUALS, IKBK_PLUS, IKBK_NULL}, {"=", "+", ""}},
		{1, 1, {IKBK_BACKSPACE, 0, 0}, {"\xE2\x8C\xAB", NULL, NULL}},
		
		{1, 1, {IKBK_INSERT, 0, 0}, {"#", NULL, NULL}},
		{1, 1, {IKBK_HOME, 0, 0}, {"#", NULL, NULL}},
		{1, 1, {IKBK_PAGEUP, 0, 0}, {"#", NULL, NULL}},
	},
	
	// T  q w e r t y u i o p [ ] \  DL EN PD 14/3 keys
	{
		{1, 1, {IKBK_TAB, 0, 0}, {"#", NULL, NULL}},
		{1, 0, {IKBK_Q, IKBK_Q, IKBK_NULL}, {"q", "Q", ""}},
		{1, 0, {IKBK_W, IKBK_W, IKBK_NULL}, {"w", "W", ""}},
		{1, 0, {IKBK_E, IKBK_E, IKBK_NULL}, {"e", "E", ""}},
		{1, 0, {IKBK_R, IKBK_R, IKBK_NULL}, {"r", "R", ""}},
		{1, 0, {IKBK_T, IKBK_T, IKBK_NULL}, {"t", "T", ""}},
		{1, 0, {IKBK_Y, IKBK_Y, IKBK_NULL}, {"y", "Y", ""}},
		{1, 0, {IKBK_U, IKBK_U, IKBK_NULL}, {"u", "U", ""}},
		{1, 0, {IKBK_I, IKBK_I, IKBK_NULL}, {"i", "I", ""}},
		{1, 0, {IKBK_O, IKBK_O, IKBK_NULL}, {"o", "O", ""}},
		{1, 0, {IKBK_P, IKBK_P, IKBK_NULL}, {"p", "P", ""}},
		{1, 0, {IKBK_LEFTBRACKET, IKBK_LEFTBRACE, IKBK_NULL}, {"[", "{", ""}},
		{1, 0, {IKBK_RIGHTBRACKET, IKBK_RIGHTBRACE, IKBK_NULL}, {"]", "}", ""}},
		{1, 0, {IKBK_BACKSLASH, IKBK_PIPE, IKBK_NULL}, {"\\", "|", ""}},
		
		{1, 1, {IKBK_KDELETE, 0, 0}, {"#", NULL, NULL}},
		{1, 1, {IKBK_END, 0, 0}, {"#N", NULL, NULL}},
		{1, 1, {IKBK_PAGEDOWN, 0, 0}, {"#", NULL, NULL}},
	},
	
	// CL  a s d f g h j k l ; ' RET  13 keys
	{
		{1, 1, {IKBK_CAPSLOCK, 0, 0}, {"#", NULL, NULL}},
		{1, 0, {IKBK_A, IKBK_A, IKBK_NULL}, {"a", "A", ""}},
		{1, 0, {IKBK_S, IKBK_S, IKBK_NULL}, {"s", "S", ""}},
		{1, 0, {IKBK_D, IKBK_D, IKBK_NULL}, {"d", "D", ""}},
		{1, 0, {IKBK_F, IKBK_F, IKBK_NULL}, {"f", "F", ""}},
		{1, 0, {IKBK_G, IKBK_G, IKBK_NULL}, {"g", "G", ""}},
		{1, 0, {IKBK_H, IKBK_H, IKBK_NULL}, {"h", "H", ""}},
		{1, 0, {IKBK_J, IKBK_J, IKBK_NULL}, {"j", "J", ""}},
		{1, 0, {IKBK_K, IKBK_K, IKBK_NULL}, {"k", "K", ""}},
		{1, 0, {IKBK_L, IKBK_L, IKBK_NULL}, {"l", "L", ""}},
		{1, 0, {IKBK_SEMICOLON, IKBK_COLON, IKBK_NULL}, {";", ":", ""}},
		{1, 0, {IKBK_APOSTROPHE, IKBK_QUOTE, IKBK_NULL}, {"\'", "\"", ""}},
		{2, 1, {IKBK_RETURN, 0, 0}, {"RT", NULL, NULL}},
		{0},
	},
	
	// SH   z x c v b n m , . /   SH    UA    12/1 keys
	{
		{2, 1, {IKBK_SHIFT, 0, 0}, {"SH", NULL, NULL}},
		{0},
		{1, 0, {IKBK_Z, IKBK_Z, IKBK_NULL}, {"z", "Z", ""}},
		{1, 0, {IKBK_X, IKBK_X, IKBK_NULL}, {"x", "X", ""}},
		{1, 0, {IKBK_C, IKBK_C, IKBK_NULL}, {"c", "C", ""}},
		{1, 0, {IKBK_V, IKBK_V, IKBK_NULL}, {"v", "V", ""}},
		{1, 0, {IKBK_B, IKBK_B, IKBK_NULL}, {"b", "B", ""}},
		{1, 0, {IKBK_N, IKBK_N, IKBK_NULL}, {"n", "N", ""}},
		{1, 0, {IKBK_M, IKBK_M, IKBK_NULL}, {"m", "M", ""}},
		{1, 0, {IKBK_COMMA, IKBK_LESSTHAN, IKBK_NULL}, {",", "<", ""}},
		{1, 0, {IKBK_PERIOD, IKBK_GREATERTHAN, IKBK_NULL}, {".", ">", ""}},
		{1, 0, {IKBK_FORWARDSLASH, IKBK_QUESTION, IKBK_NULL}, {"/", "?", ""}},
		{2, 1, {IKBK_SHIFT, 0, 0}, {"SH", NULL, NULL}},
		{0},
		
		{0},
		{1, 1, {IKBK_UP, 0, 0}, {"#", NULL, NULL}},
		{0},
	},
	
	// CT AL      space        AL CT  5 keys
	{
		{2, 1, {IKBK_CTRL, 0, 0}, {"CT", NULL, NULL}},
		{0},
		{2, 1, {IKBK_ALT, 0, 0}, {"AL", NULL, NULL}},
		{0},
		{6, 1, {IKBK_SPACE, 0, 0}, {" ", " ", " "}},
		{0},
		{0},
		{0},
		{0},
		{0},
		{2, 1, {IKBK_ALT, 0, 0}, {"AL", NULL, NULL}},
		{0},
		{2, 1, {IKBK_CTRL, 0, 0}, {"CT", NULL, NULL}},
		{0},
		
		{1, 1, {IKBK_LEFT, 0, 0}, {"#", NULL, NULL}},
		{1, 1, {IKBK_DOWN, 0, 0}, {"#", NULL, NULL}},
		{1, 1, {IKBK_RIGHT, 0, 0}, {"#", NULL, NULL}},
	},
};

static int8_t l_OSKSel[MAXSPLITSCREEN][2];
static uint32_t l_OSKLast[MAXSPLITSCREEN][2];
static bool_t l_OSKVis[MAXSPLITSCREEN];
static int32_t l_OSKShift[MAXSPLITSCREEN];

/* CONL_OSKSetVisible() -- Sets OSK Visibility */
bool_t CONL_OSKSetVisible(const size_t a_PlayerNum, const bool_t a_IsVis)
{
	/* Check */
	if (a_PlayerNum < 0 || a_PlayerNum >= MAXSPLITSCREEN)
		return false;
	
	/* Set and return */
	return (l_OSKVis[a_PlayerNum] = a_IsVis);
}

/* CONLS_DrawOSK() -- Draws On-Screen-Keyboard */
void CONLS_DrawOSK(const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, const uint32_t a_SplitP)
{
	size_t i, j, Shift;
	bool_t IsSelected;
	uint32_t DrawFlags;
	I_EventEx_t VirtEvent;
	int32_t x, y, ex, ey;
	
	/* Check */
	if (a_SplitP >= MAXSPLITSCREEN)
		return;
	
	/* Draw The Layout */
	Shift = l_OSKShift[a_SplitP];
	for (i = 0; i < LOSKROWS; i++)
		for (j = 0; j < LOSKCOLS; j += (l_OSKLayout[i][j].KeySize ? l_OSKLayout[i][j].KeySize : 1))
		{
			// Determine if it is selected
			IsSelected = false;
			if (l_OSKSel[a_SplitP][0] == i && l_OSKSel[a_SplitP][1] == j)
				IsSelected = true;
			
			// Flags to draw with
			if (!IsSelected)
				DrawFlags = VFO_COLOR(VEX_MAP_WHITE);
			else
			{
				if (gametic & 0x8)
					DrawFlags = VFO_COLOR(VEX_MAP_YELLOW);
				else
					DrawFlags = VFO_COLOR(VEX_MAP_RED);
			}
			
			// Draw it
			x = a_X + (9 * j);
			y = a_Y + (9 * i);
			ex = x + 8;
			ey = y + 8;
			V_DrawStringA(
					VFONT_OEM,
					DrawFlags,
					l_OSKLayout[i][j].UTFKey[(l_OSKLayout[i][j].VirtOnly ? 0 : Shift)],
					x,
					y
				);
			
			// Mouse in spot?
			if (g_MousePos[0] >= x && g_MousePos[0] <= ex)
				if (g_MousePos[1] >= y && g_MousePos[1] <= ey)
				{
					// Clear
					memset(&VirtEvent, 0, sizeof(VirtEvent));
					
					// Set
					VirtEvent.Type = IET_SYNTHOSK;
					VirtEvent.Data.SynthOSK.PNum = a_SplitP;
					VirtEvent.Data.SynthOSK.Direct |= 0x8000 | (i & 0xF) | ((j & 0xFF) << 8);
					I_EventExPush(&VirtEvent);
				}
		}
	
	/* Draw Mouse */
	CONL_DrawMouse();
}

/* CONL_OSKHandleEvent() -- Handle on screen keyboard event */
// OSK Events are virtually synthetic, really!
bool_t CONL_OSKHandleEvent(const I_EventEx_t* const a_Event, const size_t a_PlayerNum)
{
	bool_t VirtOnly;
	int8_t Shift;
	uint32_t ThisTime;
	I_EventEx_t FakeEvent;
	uint8_t r, c;
	
	/* Check */
	if (!a_Event || a_PlayerNum < 0 || a_PlayerNum >= MAXSPLITSCREEN)
		return false;
	
	/* Only Handle OSK Events */
	if (a_Event->Type != IET_SYNTHOSK)
		return false;
	
	/* Not visible? Ignore */
	if (!l_OSKVis[a_Event->Data.SynthOSK.PNum])
		return false;
	
	/* Limit movement */
	ThisTime = I_GetTimeMS();
	
	/* Direct? */
	if (a_Event->Data.SynthOSK.Direct & 0x8000)
	{
		r = (a_Event->Data.SynthOSK.Direct & 0xF);
		c = (a_Event->Data.SynthOSK.Direct & 0xFF0) >> 8;
		
		// Valid Move?
		if (r >= 0 && r < LOSKROWS && c >= 0 && c < LOSKCOLS)
		{
			// No key here?
			while (!l_OSKLayout[r][c].KeySize)
				c--;
			
			// And not already there?
			if (l_OSKSel[a_Event->Data.SynthOSK.PNum][0] != r ||
				l_OSKSel[a_Event->Data.SynthOSK.PNum][1] != c)
			{
				// Emit sound to inform user
				S_StartSound(NULL, sfx_oskmov);
			
				// Place selected here
				l_OSKSel[a_Event->Data.SynthOSK.PNum][0] = r;
				l_OSKSel[a_Event->Data.SynthOSK.PNum][1] = c;
			}
		}
	}
	
	/* Move Around Board */
	else if (ThisTime >= (l_OSKLast[a_Event->Data.SynthOSK.PNum][0] + 250))
	{
		l_OSKLast[a_Event->Data.SynthOSK.PNum][0] = ThisTime;
		
		// Emit sound to inform user
		S_StartSound(NULL, sfx_oskmov);
		
		// Move, but keep doing it if on a bad key 
		do
		{
			// Move Around
			l_OSKSel[a_Event->Data.SynthOSK.PNum][0] += a_Event->Data.SynthOSK.Down;
			l_OSKSel[a_Event->Data.SynthOSK.PNum][1] += a_Event->Data.SynthOSK.Right;
		
			// Move around rows
			if (l_OSKSel[a_Event->Data.SynthOSK.PNum][0] < 0)
				l_OSKSel[a_Event->Data.SynthOSK.PNum][0] = LOSKROWS - 1;
			else if (l_OSKSel[a_Event->Data.SynthOSK.PNum][0] >= LOSKROWS)
				l_OSKSel[a_Event->Data.SynthOSK.PNum][0] = 0;
	
			// Move around columns
			if (l_OSKSel[a_Event->Data.SynthOSK.PNum][1] < 0)
				l_OSKSel[a_Event->Data.SynthOSK.PNum][1] = LOSKCOLS - 1;
			else if (l_OSKSel[a_Event->Data.SynthOSK.PNum][1] >= LOSKCOLS)
				l_OSKSel[a_Event->Data.SynthOSK.PNum][1] = 0;
		} while (!l_OSKLayout[l_OSKSel[a_Event->Data.SynthOSK.PNum][0]][l_OSKSel[a_Event->Data.SynthOSK.PNum][1]].KeySize);
	}
	
	/* Type On Board */
	if (a_Event->Data.SynthOSK.Press || g_MouseDown)
		if (ThisTime >= (l_OSKLast[a_Event->Data.SynthOSK.PNum][1] + 250))
		{
			l_OSKLast[a_Event->Data.SynthOSK.PNum][1] = ThisTime;
			
			// Get pos
			r = l_OSKSel[a_Event->Data.SynthOSK.PNum][0];
			c = l_OSKSel[a_Event->Data.SynthOSK.PNum][1];
		
			// Emit sound to inform user
			S_StartSound(NULL, sfx_osktyp);
		
			// Typing into console
			if (!a_PlayerNum)
			{
				bool_t VirtOnly;
				int8_t Shift;
			
				// Virtual Only?
				VirtOnly = l_OSKLayout[r][c].VirtOnly;
			
				// Determine Shift
				Shift = 0;
				if (!VirtOnly)
					Shift = l_OSKShift[a_Event->Data.SynthOSK.PNum];
			
				// Create fake keyboard event
				memset(&FakeEvent, 0, sizeof(FakeEvent));
				FakeEvent.Type = IET_KEYBOARD;
				FakeEvent.Data.Keyboard.Down = true;
				FakeEvent.Data.Keyboard.KeyCode = l_OSKLayout[r][c].IkbkKey[Shift];
			
				if (!VirtOnly || FakeEvent.Data.Keyboard.KeyCode == IKBK_SPACE)
					FakeEvent.Data.Keyboard.Character = V_ExtMBToWChar(l_OSKLayout[r][c].UTFKey[Shift], NULL);
				
				// Push it
				I_EventExPush(&FakeEvent);
				
				// Let go of key
				FakeEvent.Data.Keyboard.Down = false;
				I_EventExPush(&FakeEvent);
				
				// If it was shift, toggle shift
				if (FakeEvent.Data.Keyboard.KeyCode == IKBK_SHIFT)
					l_OSKShift[a_Event->Data.SynthOSK.PNum] ^= 1;
			}
		
			// Typing into player chat string
			else
			{
			}
		}
	
	/* Un-handled */
	return false;
}

/* CONL_DrawConsole() -- Draws the console */
bool_t CONL_DrawConsole(void)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	bool_t FullCon;
	size_t i, n, j, k, l, BSkip;
	int32_t NumLines, Limit;
	uint32_t bx, x, by, y, bw, bh, Options, conX, conY, conW, conH, DrawCount, DefaultOptions, BackFlags, ScaleConH, ScaleBH;
	const char* p;
	char TempFill[6];
	CONL_BasicBuffer_t* Out;
	
	static int32_t BootLines = -1;
	static int32_t BootCount = 0;
	static V_Image_t* BackPic;
	
	static uint8_t SColor;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Get output buffer */
	Out = &l_CONLBuffers[0];
	
	/* Early Boot Console */
	if (g_EarlyBootConsole)
	{
		CONL_EarlyBootTic(NULL, false);
		BootLines = -1;
		return true;
	}
	
	/* Console is active (draw the console) */
	else if (CONL_IsActive())
	{
		// Fullscreen console?
		FullCon = con_startup;
		
		// Console x and y
		conX = CONLPADDING;
		conY = CONLPADDING;
		
		// Scale the console?
		if (l_CONScale.Value[0].Int)
			conW = BASEVIDWIDTH;
		
		// Not scaled
		else
			conW = vid.width - (CONLPADDING * 2);
		
		// Get character dimensions
		bw = V_FontWidth(l_CONFont.Value[0].Int);
		bh = V_FontHeight(l_CONFont.Value[0].Int);
		
		// Default background draw flags
		BackFlags = VEX_COLORMAP(l_CONBackColor.Value[0].Int);
		
		if (!l_CONScale.Value[0].Int)
			BackFlags |= VEX_NOSCALESTART | VEX_NOSCALESCREEN;
		
		// Draw back picture and determines lines to draw
		if (FullCon)
		{
			// Load background?
			if (!BackPic)
				BackPic = V_ImageFindA("RMD_CB_D");
				
			// Blit to entire screen
			if (!con_startup || BootLines == -1)	// Draw only once here
			{
				V_ImageDraw(0, BackPic, 0, 0, NULL);
				
				// And initialize BootLines
				BootLines = ((Out->EndLine - 1) & Out->MaskLine);
			}
			
			// Get height of console
			conH = vid.height;
		}
		
		else
		{
			// Scaled
			if (l_CONScale.Value[0].Int)
			{
				// Height of console is...
				if (l_CONScreenHeight.Value)
					conH = FixedMul(l_CONScreenHeight.Value->Fixed, BASEVIDHEIGHT << FRACBITS) >> FRACBITS;
				else
					conH = (BASEVIDHEIGHT >> 1);
			
				// Too small?
				if (conH <= (BASEVIDHEIGHT >> 3))
					conH = BASEVIDHEIGHT >> 3;
			}
			
			// Un-Scaled
			else
			{
				// Height of console is...
				if (l_CONScreenHeight.Value)
					conH = FixedMul(l_CONScreenHeight.Value->Fixed, vid.height << FRACBITS) >> FRACBITS;
				else
					conH = (vid.height >> 1);
			
				// Too small?
				if (conH <= (vid.height >> 3))
					conH = vid.height >> 3;
			}
			
			// Draw box
			V_DrawFadeConsBackEx(BackFlags, 0, 0, vid.width, conH);
		}
		
		// Determine line count
		NumLines = conH / bh;
		
		// No lines to draw?
		if (NumLines < 1)
			NumLines = 1;
		
		// Draw every line
		DrawCount = 0;
		
		// Start from the bottom if normal, top if boot
		if (con_startup && BootCount < NumLines)
			y = 0;
		else
			y = bh * (NumLines - (con_startup ? 0 : 2));
		
		// No lines needed?
		if (con_startup)
			if (BootLines == ((Out->EndLine - 1) & Out->MaskLine))
				return false;
		
		// Draw each line
		for (l = 0, i = 0, j = ((Out->EndLine - 1) & Out->MaskLine); i < NumLines; j = ((j - 1) & Out->MaskLine))
		{
			// Lines out of range? Or this is the first line?
			if (!Out->Lines[j] || j == ((Out->StartLine) & Out->MaskLine))
				break;
			
			// Boot console
			if (con_startup)
			{
				// Only draw this line
				if (BootLines != j)
				{
					//if (BootCount < NumLines)
					//	y += bh;
					continue;
				}
				
				// Pull the screen up (if no more room)
				if (BootCount >= NumLines)
				{
					memmove(screens[0], screens[0] + (vid.rowbytes * bh), (vid.rowbytes * (bh * (NumLines - 1))));
				
					// Draw black in the bottom portion
					memset(screens[0] + (vid.rowbytes * (bh * (NumLines - 1))), 0, (vid.rowbytes * bh));
					
					y -= bh;
				}
				
				// Draw first
				else
					y = bh * BootCount;
			}
				
			// Skip drawing this line
			if (l < l_CONLLineOff)
			{
				l++;
				continue;
			}
			else
				i++;
				
			// Get line to draw
			x = conX;
			p = Out->Lines[j];
			
			// Scale the console?
			if (!l_CONScale.Value[0].Int)
				Options = VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES;
			else
				Options = 0;
			
			// Draw each character
			while (*p && *p != '\n' && x < conW && y > conY)
			{
				// Reget n
				n = p - Out->Buffer;
				
				// Fill into temporary (for MB overchunking)
				for (k = 0; k < 5; k++)
					TempFill[k] = Out->Buffer[(n + k) & Out->MaskPos];
				TempFill[k] = 0;	// is 6
				
				// Make tab special (keep output even)
				if (*p == '\t')
				{
					bx = 0;
					x += bw * 4;
					x -= (x % (bw * 4));
					BSkip = 1;
				}
				
				// Make \1 and \3 special (white)
				else if (*p == '\1' || *p == '\2')
				{
					Options &= ~VFO_COLORMASK;
					Options |= VFO_COLOR(VEX_MAP_BRIGHTWHITE);
					bx = 0;
					BSkip = 1;
				}
				
				// Non console special control (Legacy)
				else if (*p > 7)
					bx = V_DrawCharacterMB(l_CONFont.Value[0].Int, Options, TempFill, x, y, &BSkip, &Options);
					
				// Normal character
				else
				{
					bx = 0;
					BSkip = 1;
				}
				
				// Draw using monospace text
				if (l_CONMonoSpace.Value[0].Int)
				{
					if (bx)			// Some characters may return zero
						x += bw;	// Monospaced instead of variable
				}
				
				// Otherwise, variable width
				else
					x += bx;
				
				n = (n + BSkip) & Out->MaskPos;
				p = &Out->Buffer[n];
			}
			
			// Go down
			if (!(con_startup && BootCount < NumLines))
				y -= bh;
			DrawCount++;
			
			// If this is the boot console...
			if (con_startup)
			{
				// Set last boot lines
				BootLines = ((j - 1) & Out->MaskLine);
				BootCount++;
				
				// Break out, don't draw any more
				break;
			}
		}
		
		// Draw console input line (as long as it is not startup)
		if (!con_startup)
		{
			y = conH - bh - CONLPADDING;
			x = conX;
			Options = 0;
			
			// Scale the console?
			if (!l_CONScale.Value[0].Int)
				Options |= VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES;
			
			// Draw command prompt
			x += V_DrawStringA(l_CONFont.Value[0].Int, VFO_COLOR(VEX_MAP_BRIGHTWHITE) | Options, "#>", x, y);
			
			// Draw input buffer
			Options |= VFO_COLOR(VEX_MAP_GREEN);
			l_CONLInputter->Font = l_CONFont.Value[0].Int;
			x += CONCTI_DrawInput(l_CONLInputter, Options, x, y, vid.width);
		}
		
		// Draw scrollbar
		if (!con_startup && Out->CountLine)
		{
			// Pixels per line (of all the console lines)
			by = conH / Out->CountLine;
			bw = conH - 1;
			bh = bw - (by * l_CONLLineOff);
			Options = 0;
			
			// Scale the console?
			if (!l_CONScale.Value[0].Int)
				Options |= VEX_NOSCALESTART | VEX_NOSCALESCREEN;
			
			// Draw skipped lines (from bottom)
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_BLACK) | Options, CONLSCROLLBACK, 1, bw, CONLPADDING - 1, bh);
			
			// Draw lines being seen (in current view)
			bw = bh;
			bh = bw - (by * (DrawCount < NumLines ? DrawCount : NumLines));
			
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_WHITE) | Options, CONLSCROLLFORE, 1, bw, CONLPADDING - 1, bh);
			
			// Draw lines you should see but you cant
			bw = bh;
			bh = bw - (by * (DrawCount < NumLines ? NumLines - DrawCount : DrawCount - NumLines));
			
			// Less than 1?
			if (bh < 1)
				bh = 1;
				
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_GRAY) | Options, CONLSCROLLMISS, 1, bw, CONLPADDING - 1, bh);
			
			// Draw remaining black
			bw = bh;
			bh = 1;
			
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_BLACK) | Options, CONLSCROLLBACK, 1, bw, CONLPADDING - 1, bh);
		}
		
		// Draw OSK
		if (!con_startup)
			CONLS_DrawOSK(80, 120, 320, 100, 0);
	}
	
	/* Not active, draw per player messages */
	else
	{
		// On intermission, only draw player 1's stuff
		if (gamestate == GS_INTERMISSION)
			n = 1;
		
		// Otherwise for each player
		else
			n = g_SplitScreen + 1;
		
		// Limit, just in case
		if (n > MAXSPLITSCREEN)
			n = MAXSPLITSCREEN;
		
		// Run through each player
		for (i = 0; i < n; i++)
		{
			// Obtain bounding box
			switch (n)
			{
				case 2:
					bx = 0;
					by = ((i & 1)) * (vid.height >> 1);
					bw = vid.width;
					bh = by + (vid.height >> 1);
					break;
					
				case 3:
				case 4:
					bx = ((i & 1)) * (vid.width >> 1);
					by = (((i & 2) >> 1)) * (vid.height >> 1);
					bw = bx + (vid.width >> 1);
					bh = by + (vid.height >> 1);
					break;
					
				case 1:
				default:
					bx = 0;
					by = 0;
					bw = vid.width;
					bh = vid.height;
					break;
			}
			
			// Reset y
			y = by;
			
			// Draw messages from first to last
			for (j = 0; j < MAXCONLPLAYERMQ; j++)
			{
				// If there is a message here, draw it
				if (l_CONLMessageQ[i][j].Text[0])
				{
					// Reset everything
					x = bx;
					p = &l_CONLMessageQ[i][j].Text[0];
					DefaultOptions = Options = l_CONLMessageQ[i][j].Flags;
					DefaultOptions &= (VFO_COLORMASK | VFO_TRANSMASK);
					
					// Draw single character
					while (*p)
					{
						// Draw
						x += V_DrawCharacterMB(VFONT_SMALL, Options, p, x, y, &BSkip, &Options);
						
						// Reset attributes?
						if (DefaultOptions)
							if (!(Options & (VFO_COLORMASK | VFO_TRANSMASK)))
								Options |= DefaultOptions;
						
						// Never over trans
						if ((Options & VFO_TRANSMASK) < (DefaultOptions & VFO_TRANSMASK))
						{
							Options &= ~VFO_TRANSMASK;
							Options |= (DefaultOptions & VFO_TRANSMASK);
						}
						
						// Skip char
						p += BSkip;
						
						// Over edge?
						if (x >= bx + bw)
						{
							// Reset x and increase y some
							x = bx;
							y += V_FontHeight(VFONT_SMALL);
						}
					}
					
					// Done drawing line
					y += V_FontHeight(VFONT_SMALL);
				}
			}
		}
	}
	
	/* Update screen */
	return true;
#endif /* __REMOOD_DEDICATED */
}

/*** Configuration Files ***/

#if 0
static char l_ConfigDir[PATH_MAX];				// Configuration directory
static char l_DataDir[PATH_MAX];				// Data directory
static char l_DefaultConfig[PATH_MAX];			// The default config
#endif

// l_EscapeChars -- Escape sequences
static const struct
{
	char From;
	const char* To;
} l_EscapeChars[] =
{
	{'\t', "\\t"},
	{'\n', "\\n"},
	{'\r', "\\r"},
	{'\"', "\\\""},
	{'\'', "\\\'"},
	{'\\', "\\\\"},
	{0, NULL},
};

/* CONL_EscapeString() -- Escapes a string */
size_t CONL_EscapeString(char* const a_Dest, const size_t a_Size, const char* const a_Src)
{
#define BUFSIZE 32
	char* d;
	char* p;
	const char* s;
	uint16_t WChar;
	size_t MBSkip, i, j;
	char Buf[BUFSIZE];
	bool_t IgnoreEscape;
	
	/* Check */
	if (!a_Dest || !a_Size || !a_Src)
		return 0;
	
	/* Conversion Loop */
	for (IgnoreEscape = false, i = 0, d = a_Dest, s = a_Src; *s;)
	{
		// Overflow?
		if (((size_t)(d - a_Dest)) >= (a_Size - 1))
			return (d - a_Dest);
		
		// Get unicode character
		WChar = V_ExtMBToWChar(s, &MBSkip);
		
		// No skip?
		if (!MBSkip)
			MBSkip = 1;
		
		// Unicode Character?
		if (WChar > 127)
		{
			// Place in \uHHHH
			snprintf(Buf, BUFSIZE - 1, "\\u%04X", WChar);
			
			for (p = Buf; *p; p++)
				if (((size_t)(d - a_Dest)) < (a_Size - 1))
					*(d++) = *p;
		}
		
		// Standard ASCII
		else
		{
			// See if it is already escaped
			if (!IgnoreEscape && *s != '\\')
				for (j = 0; l_EscapeChars[j].From; j++)
					if (*s == l_EscapeChars[j].From)
						break;
			
			// Found one
			if (!IgnoreEscape && *s != '\\' && l_EscapeChars[j].From)
			{
				for (p = l_EscapeChars[j].To; *p; p++)
					if (((size_t)(d - a_Dest)) < (a_Size - 1))
						*(d++) = *p;
			}
			
			// Not found, copy as it
			else
			{
				*(d++) = *s;
				
				// Unignore escape?
				if (IgnoreEscape)
					IgnoreEscape = false;
				
				// If the next character is a '\', ignore that
				if (*(s + 1) == '\\')
					IgnoreEscape = true;
			}
		}
		
		// Skip
		s += MBSkip;
	}
	
	/* Return converted length */
	return (d - a_Dest);
#undef BUFSIZE
}

/* CONL_UnEscapeString() -- Unescapes a string */
size_t CONL_UnEscapeString(char* const a_Dest, const size_t a_Size, const char* const a_Src)
{
	size_t i, j, k;
	char* d;
	char* p;
	const char* s;
	uint16_t WChar;
	char MB[5];
	
	/* Check */
	if (!a_Dest || !a_Size || !a_Src)
		return 0;
	
	/* Conversion Loop */
	for (i = 0, d = a_Dest, s = a_Src; *s;)
	{
		// Overflow?
		if (((size_t)(d - a_Dest)) >= (a_Size - 1))
			return (d - a_Dest);
		
		// Determine if string is an escape string
		if (*s == '\\')
			for (j = 0; l_EscapeChars[j].From; j++)
				if (strncasecmp(s, l_EscapeChars[j].To, strlen(l_EscapeChars[j].To)) == 0)
					break;
		
		// Found
		if (*s == '\\' && l_EscapeChars[j].From)
		{
			// Use literal character
			*(d++) = l_EscapeChars[j].From;
			s += strlen(l_EscapeChars[j].To);	// Skip length of conversion
		}
		
		// Not found
		else
		{
			// Check for unicode character
			if (*s == '\\' && (*(s + 1) == 'u' || *(s + 1) == 'U'))
			{
				// Skip ahead
				s += 2;
				
				// Read hex characters
				for (WChar = 0, j = 0; *s && j < 4; s++, j++)
				{
					// Multiply wchar by 16
					WChar *= 16;
					
					if (*s >= '0' && *s <= '9')
						WChar += (uint16_t)(*s - '0');
					else if (*s >= 'A' && *s <= 'F')
						WChar += (uint16_t)((*s - 'A') + 10);
					else if (*s >= 'a' && *s <= 'f')
						WChar += (uint16_t)((*s - 'a') + 10);
				}
				
				// Convert to multibyte
				j = V_ExtWCharToMB(WChar, MB);
				
				// Slap to buffer
				for (k = 0; k < j; k++)
					if (((size_t)(d - a_Dest)) < (a_Size - 1))
						*(d++) = MB[k];
			}
			
			// A normal character
			else
				*(d++) = *(s++);
		}
	}
	
	/* Return converted length */
	return (d - a_Dest);
}

/* CONL_FindDefaultConfig() -- Finds the default config file */
bool_t CONL_FindDefaultConfig(void)
{
	const char* Arg;
	bool_t ConfigOK;
	
	/* Clear OK */
	ConfigOK = false;
	
	/* Clear */
	memset(l_DefaultConfig, 0, sizeof(l_DefaultConfig));
	
	/* Check for -config */
	if (M_CheckParm("-config"))
	{
		// Get argument
		Arg = M_GetNextParm();
		
		// Copy to config location
		strncat(l_DefaultConfig, Arg, PATH_MAX);
		
		// OK
		ConfigOK = true;
	}
	
	/* Assume default location */
	if (!ConfigOK)
	{
		// Concat config dir
		strncat(l_DefaultConfig, l_ConfigDir, PATH_MAX);
		strncat(l_DefaultConfig, "/remoodex.cfg", PATH_MAX);
	}
	
	/* Success! */
	return true;
}

/* CONL_LoadConfigFile() -- Loads the configuration file specified by path */
bool_t CONL_LoadConfigFile(const char* const a_Path)
{
	const char* p;
	char Buf[PATH_MAX];
	
	/* Check */
	if (!a_Path)
		return false;
		
	/* Message */
	CONL_PrintF("CONL_LoadConfigFile: Loading from \"%s\"...\n", a_Path);
	
	/* Escape the string */
	// Clear
	memset(Buf, 0, sizeof(Buf));
	
	// Escape
	CONL_EscapeString(Buf, PATH_MAX, a_Path);
	
	/* Execute it */
	CONL_VarSetLoaded(true);	// Set loaded from config
	CONL_InputF("execfile \"%s\"\n\n", Buf);
	CONL_VarSetLoaded(false);	// Unset
	
	/* Success! */
	return true;
}

/* CONL_SaveConfigFile() -- Save configuration file */
bool_t CONL_SaveConfigFile(const char* const a_Path)
{
#define BUFSIZE 256
	size_t i;
	CONL_StaticVar_t* SVar;
	FILE* File;
	const char* p;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Path)
		return false;
	
	/* Message */
	CONL_PrintF("CONL_SaveConfigFile: Saving to \"%s\"...\n", a_Path);
	
	/* Attempt config file open */
	File = fopen(a_Path, "w");
	
	// Failed?
	if (!File)
		return false;
	
	/* Print Header */
	fprintf(File, "// ReMooD %i.%i%c \"%s\" Configuration File\n", REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
	fprintf(File, "// Visit %s\n", REMOOD_URL);
	
	/* Get every var */
	for (i = 0;; i++)
	{
		// Clear
		SVar = NULL;
		
		// Get variable
		if (!CONL_StaticVarByNum(i, &SVar))
			break;	// No more
		
		// Variable here?
		if (!SVar)
			continue;
		
		// Variable not saved to config
		if (!(SVar->Flags & CLVF_SAVE))
			continue;
		
		// Print command to set variables
		fprintf(File, "cvset \"%s\" \"", SVar->VarName);
		
		// Escape string before it is printed (to not mess up syntax)
		memset(Buf, 0, sizeof(Buf));
		CONL_EscapeString(Buf, BUFSIZE, SVar->Value[0].String);
		
		// And print the escaped string
		fprintf(File, "%s", Buf);
		
		// Print end
		fprintf(File, "\"\n");
	}
	
	/* Close file */
	fclose(File);
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/*** LOADING SCREENS ***/

/** LOCALS **/

#define CCLSLOADINGSCREENWAIT				2000	// Time before screen effect

static bool_t l_CLSDoBase = false;
static bool_t l_CLSBaseDrawn = false;
static const char* l_CLSMessage = NULL;
static int32_t l_CLSProgress[2][2] = {{0, 0}, {0, 0}};
static uint32_t l_CLSBaseTic = 0;					// Base loading screen time

/** FUNCTIONS **/

/* CONLS_DrawLoadingScreen() -- Draw loading screen */
static void CONLS_DrawLoadingScreen(const bool_t a_QuickDraw)
{
	size_t i;
	uint32_t BarBits;
	fixed_t Frac;
	V_Image_t* BGImage;
	
	/* Clear screen if not quick drawing */
	if (!a_QuickDraw || (l_CLSDoBase && !l_CLSBaseDrawn))
	{
		// Set as drawn
		l_CLSBaseDrawn = true;
		
		// Wipe away
		memset(screens[0], 0, vid.rowbytes * vid.height);
		
		// Draw ReMooD Background
		BGImage = V_ImageFindA("RMD_LLOA");
		V_ImageDraw(0, BGImage, 0, 0, NULL);
		V_ImageDestroy(BGImage);
		
		// Draw stuff onto screen
		//V_DrawStringA(VFONT_LARGE, VFO_CENTERED, "Loading...", vid.width / 4, 100);
		
		// Draw current action
		if (l_CLSMessage)
			V_DrawStringA(VFONT_OEM, 0, l_CLSMessage, 0, 160);
	}
	
	/* Draw completion bars */
	for (i = 0; i < 2; i++)
	{
#define SIDESPACE 5
#define BASEBARY (170 + (12 * i))
#define BASEBARSIZE 10
		// Draw bar across entire screen (white for incomplete)
		V_DrawColorBoxEx(0, 4, SIDESPACE, BASEBARY, SIDESPACE + (320 - (SIDESPACE * 2)), BASEBARY + BASEBARSIZE);
		
		// No right side?
		if (!l_CLSProgress[i][1])
			continue;
			
		// Draw green bar ontop (complete)
		if (l_CLSProgress[i][0] >= 0)
		{
			Frac = FixedDiv(l_CLSProgress[i][0] << FRACBITS, l_CLSProgress[i][1] << FRACBITS);
			
			// Limit to 0.0-1.0
			if (Frac < 0)
				Frac = 0;
			else if (Frac > (1 << FRACBITS))
				Frac = 1 << FRACBITS;
			
			// Cap Bar
			BarBits = (FixedMul(Frac, (320 - (SIDESPACE * 2)) << FRACBITS) >> FRACBITS);
			
			if (BarBits >= 320 - (SIDESPACE * 2))
				BarBits = 320 - (SIDESPACE * 2);
			
			// Draw bar
			V_DrawColorBoxEx(0, 112, SIDESPACE, BASEBARY, SIDESPACE + BarBits, BASEBARY + BASEBARSIZE);
		}
#undef BASEBARSIZE
#undef BASEBARY
#undef SIDESPACE
	}
	
	/* Update Screen */
	I_FinishUpdate();
}

/* CONL_LoadingScreenSet() -- Set loading screen */
bool_t CONL_LoadingScreenSet(const int32_t a_NumSteps)
{
	/* Reset */
	l_CLSMessage = NULL;
	memset(l_CLSProgress, 0, sizeof(l_CLSProgress));
	
	/* Set count */
	l_CLSProgress[0][0] = -1;
	l_CLSProgress[0][1] = a_NumSteps;
	
	/* Reset Time */
	l_CLSBaseTic = I_GetTimeMS();
	
	/* Draw and return */
	l_CLSDoBase = true;
	l_CLSBaseDrawn = false;
	if (I_GetTimeMS() - l_CLSBaseTic >= CCLSLOADINGSCREENWAIT)
		CONLS_DrawLoadingScreen(false);
	return true;
}

/* CONL_LoadingScreenIncrMaj() -- Increment major */
bool_t CONL_LoadingScreenIncrMaj(const char* const a_Message, const int32_t a_NumSteps)
{
	/* Set */
	// Major
	l_CLSMessage = a_Message;
	l_CLSProgress[0][0]++;
	
	// Sub
	l_CLSProgress[1][0] = -1;
	l_CLSProgress[1][1] = a_NumSteps;
	
	/* Draw and return */
	l_CLSDoBase = true;
	l_CLSBaseDrawn = false;
	if (I_GetTimeMS() - l_CLSBaseTic >= CCLSLOADINGSCREENWAIT)
		CONLS_DrawLoadingScreen(false);
	return true;
}

/* CONL_LoadingScreenIncrSub() -- Increment Sub */
bool_t CONL_LoadingScreenIncrSub(void)
{
	/* Increment */
	l_CLSProgress[1][0]++;
	
	/* Draw and return */
	if (I_GetTimeMS() - l_CLSBaseTic >= CCLSLOADINGSCREENWAIT)
		CONLS_DrawLoadingScreen(true);
	return true;
}

/* CONL_LoadingScreenSetSubEnd() -- Set sub steps */
bool_t CONL_LoadingScreenSetSubEnd(const int32_t a_NumSteps)
{
	/* Increment */
	l_CLSProgress[1][1] = a_NumSteps;
	
	/* Draw and return */
	if (I_GetTimeMS() - l_CLSBaseTic >= 2000)
		CONLS_DrawLoadingScreen(true);
	return true;
}

/*****************************************************************************/

/**************
*** GLOBALS ***
**************/

bool_t g_QuietConsole = false;	// Mute startup console

/***************************
*** DEPRECATED FUNCTIONS ***
***************************/

/*******************************************************************************
********************************************************************************
*******************************************************************************/

bool_t con_started = false;		// console has been initialised
bool_t con_startup = false;		// true at game startup, screen need refreshing
bool_t con_forcepic = true;		// at startup toggle console transulcency when

// first off
bool_t con_recalc;				// set true when screen size has changed

int con_tick;					// console ticker for anim or blinking prompt cursor

// con_scrollup should use time (currenttime - lasttime)..

bool_t consoletoggle;			// true when console key pushed, ticker will handle
bool_t consoleready;			// console prompt is ready

int con_destlines;				// vid lines used by console at final position
int con_curlines;				// vid lines currently used by console

int con_clipviewtop;			// clip value for planes & sprites, so that the

// part of the view covered by the console is not
// drawn when not needed, this must be -1 when
// console is off

int con_hudlines;				// number of console heads up message lines
int con_hudtime[5];				// remaining time of display for hud msg lines

int con_clearlines;				// top screen lines to refresh when view reduced
bool_t con_hudupdate;			// when messages scroll, we need a backgrnd refresh

// console text output
char* con_line;					// console text output current line
int con_cx;						// cursor position in current line
int con_cy;						// cursor line number in con_buffer, is always

//  increasing, and wrapped around in the text
//  buffer using modulo.

int con_totallines;				// lines of console text into the console buffer
int con_width;					// columns of chars, depend on vid mode width

int con_scrollup;				// how many rows of text to scroll up (pgup/pgdn)

int con_lineowner[CON_MAXHUDLINES];	//In splitscreen, which player gets this line of text

//0 or 1 is player 1, 2 is player 2

char inputlines[32][CON_MAXPROMPTCHARS];	// hold last 32 prompt lines

int inputline;					// current input line number
int inputhist;					// line number of history input line to restore
int input_cx;					// position in current input line

// GhostlyDeath <April 26, 2009> --
struct pic_s* con_backpic;		// console background picture, loaded static
struct pic_s* con_bordleft;
struct pic_s* con_bordright;	// console borders in translucent mode

// protos.
void CON_InputInit(void);
void CON_RecalcSize(void);

void CONS_speed_Change(void);
void CON_DrawBackpic(pic_t* pic, int startx, int destwidth);

//======================================================================
//                   CONSOLE VARS AND COMMANDS
//======================================================================

char con_buffer[CON_BUFFERSIZE];

// how many seconds the hud messages lasts on the screen
consvar_t cons_msgtimeout = { "con_hudtime", "5", CV_SAVE, CV_Unsigned };

// number of lines console move per frame
consvar_t cons_speed = { "con_speed", "8", CV_CALL | CV_SAVE, CV_Unsigned, &CONS_speed_Change };

// percentage of screen height to use for console
consvar_t cons_height = { "con_height", "50", CV_SAVE, CV_Unsigned };

CV_PossibleValue_t backpic_cons_t[] = { {0, "translucent"}, {1, "picture"}, {2, "Nothing"}, {0, NULL} };

// whether to use console background picture, or translucent mode
consvar_t cons_backpic = { "con_backpic", "0", CV_SAVE, backpic_cons_t };

void CON_Print(char* msg);

//  Check CONS_speed value (must be positive and >0)
//
void CONS_speed_Change(void)
{
	if (cons_speed.value < 1)
		CV_SetValue(&cons_speed, 1);
}

//  Clear console text buffer
//
void CONS_Clear_f(void)
{
	if (con_buffer)
		memset(con_buffer, 0, CON_BUFFERSIZE);
		
	con_cx = 0;
	con_cy = con_totallines - 1;
	con_line = &con_buffer[con_cy * con_width];
	con_scrollup = 0;
}

int con_keymap;					//0 english, 1 french

//  Choose english keymap
//
void CONS_English_f(void)
{
	shiftxform = english_shiftxform;
	con_keymap = english;
	CONL_PrintF("English keymap.\n");
}

//  Choose french keymap
//
void CONS_French_f(void)
{
	shiftxform = french_shiftxform;
	con_keymap = french;
	CONL_PrintF("French keymap.\n");
}

//======================================================================
//                          CONSOLE SETUP
//======================================================================

// Prepare a colormap for GREEN ONLY translucency over background
// GhostlyDeath - Red in ReMooD
uint8_t* whitemap;
uint8_t* greenmap;
uint8_t* graymap;
uint8_t* orangemap;

/* CON_SetupBackColormap() -- Deprecated handler for old colormaps */
void CON_SetupBackColormap(void)
{
	/* Call new function */
	// GhostlyDeath <November 5, 2010> -- Initialize colormaps
	//V_InitializeColormaps();
	
	/* Wrap old arrays */
	// Allocate
	greenmap = (uint8_t*)Z_Malloc(256, PU_STATIC, NULL);
	whitemap = (uint8_t*)Z_Malloc(256, PU_STATIC, NULL);
	graymap = (uint8_t*)Z_Malloc(256, PU_STATIC, NULL);
	orangemap = (uint8_t*)Z_Malloc(256, PU_STATIC, NULL);
	
	// Copy in
	memmove(greenmap, V_ReturnColormapPtr(VEX_MAP_RED), sizeof(uint8_t) * 256);
	memmove(whitemap, V_ReturnColormapPtr(VEX_MAP_BRIGHTWHITE), sizeof(uint8_t) * 256);
	memmove(graymap, V_ReturnColormapPtr(VEX_MAP_GRAY), sizeof(uint8_t) * 256);
	memmove(orangemap, V_ReturnColormapPtr(VEX_MAP_ORANGE), sizeof(uint8_t) * 256);
}

//  Setup the console text buffer
//
void CON_Init(void)
{
	int i;
	
	if (dedicated)
		return;
		
	// clear all lines
	memset(con_buffer, 0, CON_BUFFERSIZE);
	
	// make sure it is ready for the loading screen
	con_width = 0;
	CON_RecalcSize();
	
	CON_SetupBackColormap();
	
	//note: CON_Ticker should always execute at least once before D_Display()
	con_clipviewtop = -1;		// -1 does not clip
	
	con_hudlines = CON_MAXHUDLINES;
	
	// setup console input filtering
	CON_InputInit();
	
	// load console background pic
	con_backpic = (pic_t*)W_CacheLumpName("RMD_CB_D", PU_STATIC);
	
	// borders MUST be there
	//con_bordleft  = (pic_t*) W_CacheLumpName ("CBLEFT",PU_STATIC);
	//con_bordright = (pic_t*) W_CacheLumpName ("CBRIGHT",PU_STATIC);
	
	// register our commands
	//
	CV_RegisterVar(&cons_msgtimeout);
	CV_RegisterVar(&cons_speed);
	CV_RegisterVar(&cons_height);
	CV_RegisterVar(&cons_backpic);
	COM_AddCommand("cls", CONS_Clear_f);
	COM_AddCommand("english", CONS_English_f);
	COM_AddCommand("french", CONS_French_f);
	// set console full screen for game startup MAKE SURE VID_Init() done !!!
	con_destlines = vid.height;
	con_curlines = vid.height;
	consoletoggle = false;
	
	g_EarlyBootConsole = false;
	con_started = true;
	con_startup = true;			// need explicit screen refresh
	// until we are in Doomloop
	
	//COM_AddCommand("conextended", CONS_ConExtended_f);
}

//  Console input initialization
//
void CON_InputInit(void)
{
	int i;
	
	// prepare the first prompt line
	memset(inputlines, 0, sizeof(inputlines));
	for (i = 0; i < 32; i++)
		inputlines[i][0] = CON_PROMPTCHAR;
	inputline = 0;
	input_cx = 1;
	
}

//  Insert a new line in the console text buffer
//
void CON_Linefeed(int second_player_message)
{
	// set time for heads up messages
	con_hudtime[con_cy % con_hudlines] = cons_msgtimeout.value * TICRATE;
	
	if (second_player_message == 1)
		con_lineowner[con_cy % con_hudlines] = 2;	//Msg for second player
	else
		con_lineowner[con_cy % con_hudlines] = 1;
		
	con_cy++;
	con_cx = 0;
	
	con_line = &con_buffer[(con_cy % con_totallines) * con_width];
	memset(con_line, ' ', con_width);
	
	// make sure the view borders are refreshed if hud messages scroll
	con_hudupdate = true;		// see HU_Erase()
}

//  Outputs text into the console text buffer
//
//TODO: fix this mess!!
void CON_Print(char* msg)
{
	int l;
	int mask = 0;
	int second_player_message = 0;
	
	if (con_started)
	{
	
		//TODO: finish text colors
		if (*msg < 5)
		{
			if (*msg == 0x02)	// set white color
				mask = 128;
			else if (*msg == 0x03)
			{
				mask = 128;		// white text + sound
				if (gamemode == commercial)
					S_StartSound(0, sfx_radio);
				else
					S_StartSound(0, sfx_tink);
			}
			else if (*msg == 0x04)	//Splitscreen: This message is for the second player
				second_player_message = 1;
				
		}
		
		while (*msg)
		{
			// skip non-printable characters and white spaces
			while (*msg && *msg >= 0x00 && *msg <= ' ')
			{
			
				// carriage return
				if (*msg == '\r')
				{
					con_cy--;
					CON_Linefeed(second_player_message);
				}
				else
					// linefeed
					if (*msg == '\n')
						CON_Linefeed(second_player_message);
					else if (*msg == ' ')
					{
						con_line[con_cx++] = ' ';
						if (con_cx >= con_width)
							CON_Linefeed(second_player_message);
					}
					else if (*msg == '\t')
					{
						//adds tab spaces for nice layout in console
						
						do
						{
							con_line[con_cx++] = ' ';
						}
						while (con_cx % 4 != 0);
						
						if (con_cx >= con_width)
							CON_Linefeed(second_player_message);
					}
				msg++;
			}
			
			if (*msg == 0)
				return;
				
			// printable character
			for (l = 0; l < con_width && !(msg[l] >= 0x00 && msg[l] <= ' '); l++)
			
				// word wrap
				if (con_cx + l > con_width)
					CON_Linefeed(second_player_message);
					
			// a word at a time
			for (; l > 0; l--)
				con_line[con_cx++] = *(msg++) /* | mask */ ;
				
		}
	}
}

//======================================================================
//                        CONSOLE EXECUTION
//======================================================================

//  Called at screen size change to set the rows and line size of the
//  console text buffer.
//
void CON_RecalcSize(void)
{
	int conw, oldcon_width, oldnumlines, i, oldcon_cy;
	char tmp_buffer[CON_BUFFERSIZE];
	char string[CON_BUFFERSIZE];	// BP: it is a line but who know
	
	con_recalc = false;
	
	conw = (vid.width >> 3) - 2;
	
	if (con_curlines == 200)	// first init
	{
		con_curlines = vid.height;
		con_destlines = vid.height;
	}
	// check for change of video width
	if (conw == con_width)
		return;					// didnt change
		
	oldcon_width = con_width;
	oldnumlines = con_totallines;
	oldcon_cy = con_cy;
	memcpy(tmp_buffer, con_buffer, CON_BUFFERSIZE);
	
	if (conw < 1)
		con_width = (BASEVIDWIDTH >> 3) - 2;
	else
		con_width = conw;
		
	con_totallines = CON_BUFFERSIZE / con_width;
	memset(con_buffer, ' ', CON_BUFFERSIZE);
	
	con_cx = 0;
	con_cy = con_totallines - 1;
	con_line = &con_buffer[con_cy * con_width];
	con_scrollup = 0;
	
	// re-arrange console text buffer to keep text
	if (oldcon_width)			// not the first time
	{
		for (i = oldcon_cy + 1; i < oldcon_cy + oldnumlines; i++)
		{
			if (tmp_buffer[(i % oldnumlines) * oldcon_width])
			{
				memcpy(string, &tmp_buffer[(i % oldnumlines) * oldcon_width], oldcon_width);
				conw = oldcon_width - 1;
				while (string[conw] == ' ' && conw)
					conw--;
				string[conw + 1] = '\n';
				string[conw + 2] = '\0';
				CON_Print(string);
			}
		}
	}
}

// Handles Console moves in/out of screen (per frame)
//
void CON_MoveConsole(void)
{
	// up/down move to dest
	if (con_curlines < con_destlines)
	{
		con_curlines += cons_speed.value;
		if (con_curlines > con_destlines)
			con_curlines = con_destlines;
	}
	else if (con_curlines > con_destlines)
	{
		con_curlines -= cons_speed.value;
		if (con_curlines < con_destlines)
			con_curlines = con_destlines;
	}
	
}

//  Clear time of console heads up messages
//
void CON_ClearHUD(void)
{
	int i;
	
	for (i = 0; i < con_hudlines; i++)
		con_hudtime[i] = 0;
}

// Force console to move out immediately
// note: con_ticker will set consoleready false
void CON_ToggleOff(void)
{
	if (!con_destlines)
		return;
		
	con_destlines = 0;
	con_curlines = 0;
	CON_ClearHUD();
	con_forcepic = 0;
	con_clipviewtop = -1;		//remove console clipping of view
}

//  Console ticker : handles console move in/out, cursor blinking
//
void CON_Ticker(void)
{
	int i;
	
	if (dedicated)
		return;
		
	// cursor blinking
	con_tick++;
	con_tick &= 7;
	
	// console key was pushed
	if (consoletoggle)
	{
		consoletoggle = false;
		
		// toggle off console
		if (con_destlines > 0)
		{
			con_destlines = 0;
			CON_ClearHUD();
		}
		else
		{
			// toggle console in
			con_destlines = (cons_height.value * vid.height) / 100;
			if (con_destlines < 20)
				con_destlines = 20;
			else if (con_destlines > vid.height - stbarheight)
				con_destlines = vid.height - stbarheight;
				
			con_destlines &= ~0x3;	// multiple of text row height
		}
	}
	// console movement
	if (con_destlines != con_curlines)
		CON_MoveConsole();
		
	// clip the view, so that the part under the console is not drawn
	con_clipviewtop = -1;
	if (cons_backpic.value == 1)	// clip only when using an opaque background
	{
		if (con_curlines > 0)
			con_clipviewtop = con_curlines - viewwindowy - 1 - 10;
//NOTE: BIG HACK::SUBTRACT 10, SO THAT WATER DON'T COPY LINES OF THE CONSOLE
//      WINDOW!!! (draw some more lines behind the bottom of the console)
		if (con_clipviewtop < 0)
			con_clipviewtop = -1;	//maybe not necessary, provided it's <0
	}
	// check if console ready for prompt
	if ( /*(con_curlines==con_destlines) && */ (con_destlines >= 20))
		consoleready = true;
	else
		consoleready = false;
		
	// make overlay messages disappear after a while
	for (i = 0; i < con_hudlines; i++)
	{
		con_hudtime[i]--;
		if (con_hudtime[i] < 0)
			con_hudtime[i] = 0;
	}
}

//  Handles console key input
//
bool_t CON_Responder(event_t* ev)
{
	return false;
}

//  Print an error message, and wait for ENTER key to continue.
//  To make sure the user has seen the message
//
void CONS_Error(char* msg)
{
#ifdef _WIN32
	if (!graphics_started)
	{
		MessageBox(NULL, msg, "Doom Legacy Warning", MB_OK);
		return;
	}
#endif
	CONL_PrintF("\2%s", msg);	// write error msg in different colour
	CONL_PrintF("Press ENTER to continue\n");
	
	// dirty quick hack, but for the good cause
	while (I_GetKey() != KEY_ENTER)
		;
}

//======================================================================
//                          CONSOLE DRAW
//======================================================================

// draw console prompt line
//
void CON_DrawInput(void)
{
}

// draw the last lines of console text to the top of the screen
//
void CON_DrawHudlines(void)
{
	con_clearlines = 0;
}

//  Scale a pic_t at 'startx' pos, to 'destwidth' columns.
//                startx,destwidth is resolution dependent
//  Used to draw console borders, console background.
//  The pic must be sized BASEVIDHEIGHT height.
//
//  TODO: ASM routine!!! lazy Fab!!
//
void CON_DrawBackpic(pic_t* pic, int startx, int destwidth)
{
	int x, y;
	int v;
	uint8_t* src, *dest;
	int frac, fracstep;
	
	dest = vid.buffer + startx;
	
	for (y = 0; y < con_curlines; y++, dest += vid.width)
	{
		// scale the picture to the resolution
		v = LittleSwapInt16(pic->height) - ((con_curlines - y) * (BASEVIDHEIGHT - 1) / vid.height) - 1;
		
		src = pic->data + v * LittleSwapInt16(pic->width);
		
		// in case of the console backpic, simplify
		if (LittleSwapInt16(pic->width) == destwidth)
			memcpy(dest, src, destwidth);
		else
		{
			// scale pic to screen width
			frac = 0;
			fracstep = (LittleSwapInt16(pic->width) << 16) / destwidth;
			for (x = 0; x < destwidth; x += 4)
			{
				dest[x] = src[frac >> 16];
				frac += fracstep;
				dest[x + 1] = src[frac >> 16];
				frac += fracstep;
				dest[x + 2] = src[frac >> 16];
				frac += fracstep;
				dest[x + 3] = src[frac >> 16];
				frac += fracstep;
			}
		}
	}
	
}

// draw the console background, text, and prompt if enough place
//
void CON_DrawConsole(void)
{
	char* p;
	int i, x, y, lx;
	int w = 0, x2 = 0;
	size_t MBSkip;
	
	if (con_curlines <= 0)
		return;
		
	//FIXME: refresh borders only when console bg is translucent
	con_clearlines = con_curlines;	// clear console draw from view borders
	con_hudupdate = true;		// always refresh while console is on
	
	// draw console background
	if (cons_backpic.value == 1 || con_forcepic)
		CON_DrawBackpic(con_backpic, 0, vid.width);	// picture as background
	else if (!cons_backpic.value)
	{
		w = 8 * vid.dupx;
		x2 = vid.width - w;
		//Hurdler: what's the correct value of w and x2 in hardware mode ???
		V_DrawFadeConsBack(0, 0, vid.width, con_curlines);	// translucent background
	}
	// draw console text lines from bottom to top
	// (going backward in console buffer text)
	//
	if (con_curlines < 20)		//8+8+4
		return;
		
	i = con_cy - con_scrollup;
	
	// skip the last empty line due to the cursor being at the start
	// of a new line
	if (!con_scrollup && !con_cx)
		i--;
		
	for (y = con_curlines - 20; y >= 0; y -= 8, i--)
	{
		if (i < 0)
			i = 0;
			
		p = &con_buffer[(i % con_totallines) * con_width];
		
		// GhostlyDeath <November 2, 2010> -- UTF-8 Console
		for (MBSkip = 1, x = 0, lx = 0; x < con_width; x += MBSkip, lx++)
		{
			V_DrawCharacterMB(VFONT_SMALL, VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES, &p[x], (lx + 1) << 3,	// require logical x
			y, &MBSkip, NULL);
			
			// Just in case
			if (!MBSkip)
				MBSkip = 1;
		}
	}
	
	// draw prompt if enough place (not while game startup)
	//
	if ((con_curlines == con_destlines) && (con_curlines >= 20) && !con_startup)
		CON_DrawInput();
}

//  Console refresh drawer, call each frame
//
void CON_Drawer(void)
{
	if (!con_started)
		return;
		
	if (con_recalc)
		CON_RecalcSize();
		
	//Fab: bighack: patch 'I' letter leftoffset so it centers
	hu_font['I' - HU_FONTSTART]->leftoffset = -2;
	
	if (con_curlines > 0)
		CON_DrawConsole();
	else if (gamestate == GS_LEVEL)
		CON_DrawHudlines();
		
	hu_font['I' - HU_FONTSTART]->leftoffset = 0;
}

