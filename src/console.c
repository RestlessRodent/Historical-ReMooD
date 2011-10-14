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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

/*************
*** LOCALS ***
*************/

static bool_t l_CONLActive = false;	// Console active?
static CONL_BasicBuffer_t l_CONLBuffers[2];	// Input/Output Buffer
static CONL_PlayerMessage_t l_CONLMessageQ[MAXSPLITSCREENPLAYERS][MAXCONLPLAYERMQ];	// Player message queue
static uint32_t l_CONLLineOff = 0;	// Line offset
static CONCTI_Inputter_t* l_CONLInputter = NULL;	// Console inputter

/****************
*** FUNCTIONS ***
****************/

/*** Common Text Input ***/

/* CONCTI_CreateInput() -- Creates common inputter */
CONCTI_Inputter_t* CONCTI_CreateInput(const size_t a_NumHistory, const CONCTI_OutBack_t a_OutBack, CONCTI_Inputter_t** const a_RefPtr)
{
	CONCTI_Inputter_t* New;
	
	/* Create new */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Set data
	New->OutFunc = a_OutBack;
	New->RefPtr = a_RefPtr;
	
	// History base
	New->NumHistory = a_NumHistory;
	if (New->NumHistory)
		New->History = Z_Malloc(sizeof(*New->History) * New->NumHistory, PU_STATIC, NULL);
		
	return New;
}

/* CONCTI_DestroyInput() -- Destroys common inputter */
void CONCTI_DestroyInput(CONCTI_Inputter_t* const a_Input)
{
	size_t i;
	
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
}

/* CONCTI_HandleEvent() -- Handles event for inputter */
bool_t CONCTI_HandleEvent(CONCTI_Inputter_t* const a_Input, const I_EventEx_t* const a_Event)
{
#define BUFSIZE 512
	uint8_t Code;
	uint16_t Char;
	int32_t i, j;
	char MB[5];
	char Buf[BUFSIZE];
	CONCTI_MBChain_t* MBRover, *Last;
	
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
			
			// Send buffer to handler
			if (a_Input->OutFunc)
				if (a_Input->OutFunc(a_Input, Buf))
					CONCTI_DestroyInput(a_Input);	// Done with this, so destroy
					
			// Unmark changed, cleared away
			a_Input->Changed = false;
			return true;
			
			// Delete Characters
		case IKBK_BACKSPACE:
		case IKBK_DELETE:
			// Find character to delete
			j = a_Input->CursorPos - (Code == IKBK_DELETE ? 0 : 1);
			
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
}

/* CONCTI_SetText() -- Sets text for inputter */
void CONCTI_SetText(CONCTI_Inputter_t* const a_Input, const char* const a_Text)
{
}

/* CONCTI_DrawInput() -- Draws input text */
int32_t CONCTI_DrawInput(CONCTI_Inputter_t* const a_Input, const uint32_t a_Options, const int32_t a_x, const int32_t a_y, const int32_t a_x2)
{
	CONCTI_MBChain_t* MBRover;
	uint32_t Options, DefaultOptions;
	int32_t bx, x, j;
	bool_t GotCur, CurVirtual;
	uint16_t ThisChar, NextChar;
	
	/* Check */
	if (!a_Input)
		return NULL;
	
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
		x += V_DrawCharacterMB(CONLCONSOLEFONT, Options, (MBRover->EnableVirtual ? MBRover->VirtualMB : MBRover->MB), x, a_y, NULL, &Options);
		
		// If a virtual character was drawn here, then replace it with a ?
		if (MBRover->EnableVirtual)
			x += V_DrawCharacterMB(CONLCONSOLEFONT, Options, "?", x, a_y, NULL, &Options);
		
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
		V_DrawCharacterA(CONLCONSOLEFONT, Options, (a_Input->Overwrite ? 0x7F : '_'), bx, a_y);
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
			S_StartSound(0, (commercial ? sfx_radio : sfx_tink));
			
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
	if (devparm || !con_started)
	{
		I_OutputText(Buf);
		I_OutputText("\n");
	}
	
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

/*** Console Commands ***/
#define MAXCONLCOMMANDNAME		128							// Max name for console command

/* CONL_ConCommand_t -- Console command */
typedef struct CONL_ConCommand_s
{
	char Name[MAXCONLCOMMANDNAME];							// Name of command
	CONL_ExitCode_t (*ComFunc)(const uint32_t, const char** const);	// Function to call
	uint32_t Hash;											// Hash table hash
} CONL_ConCommand_t;

static CONL_ExitCode_t g_CONLError = CLE_SUCCESS;			// '? command' Exit Code

static CONL_ConCommand_t* l_CONLCommands = NULL;			// Console commands
static size_t l_CONLNumCommands = 0;						// Number of commands
static Z_HashTable_t* l_CONLCommandHashes = NULL;			// Speed lookup

static CONL_ConVariable_t** l_CONLVariables = NULL;			// Console variables
static size_t l_CONLNumVariables = 0;						// Number of variables
static Z_HashTable_t* l_CONLVariableHashes = NULL;			// Speed lookup

/* CONL_CommandHashCompare() -- Compares entry hash */
// A = const char*
// B = uintptr_t
static bool_t CONL_CommandHashCompare(void* const a_A, void* const a_B)
{
	/* Check */
	if (!a_A || (((uintptr_t)a_B) - 1) >= l_CONLNumCommands)
		return false;
	
	/* Caseless compare */
	if (strcasecmp(a_A, l_CONLCommands[((uintptr_t)a_B) - 1].Name) == 0)
		return true;
	
	/* No match */
	return false;
}

/* CONL_AddCommand() -- Add console command */
bool_t CONL_AddCommand(const char* const a_Name, CONL_ExitCode_t (*a_ComFunc)(const uint32_t, const char** const))
{
	uintptr_t x;
	
	/* Check */
	if (!a_Name || !a_ComFunc)
		return false;
	
	/* Check if already registered */
	if (l_CONLCommands)
		for (x = 0; x < l_CONLNumCommands; x++)
			if (strcasecmp(l_CONLCommands[x].Name, a_Name) == 0)
				return false;	// Whoops!
	
	/* Resize command structure */
	Z_ResizeArray((void**)&l_CONLCommands, sizeof(*l_CONLCommands), l_CONLNumCommands, l_CONLNumCommands + 1);
	
	/* Set in command structure */
	strncpy(l_CONLCommands[l_CONLNumCommands].Name, a_Name, MAXCONLCOMMANDNAME);
	l_CONLCommands[l_CONLNumCommands].ComFunc = a_ComFunc;
	l_CONLCommands[l_CONLNumCommands].Hash = Z_Hash(l_CONLCommands[l_CONLNumCommands].Name);
	
	/* Create hash table */
	// No table yet created?
	if (!l_CONLCommandHashes)
		l_CONLCommandHashes = Z_HashCreateTable(CONL_CommandHashCompare);
	
	// Add to hash
	Z_HashAddEntry(l_CONLCommandHashes, l_CONLCommands[l_CONLNumCommands].Hash, (uintptr_t)(l_CONLNumCommands + 1));
	
	/* Increment */
	l_CONLNumCommands++;
	
	/* Success */
	return true;
}

/* CONL_Exec() -- Execute command */
CONL_ExitCode_t CONL_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	uint32_t Hash;
	uintptr_t ComNum;
	
	/* Check */
	if (!a_ArgC || !a_ArgV)
		return CLE_CRITICALFAILURE;
	
	/* Find hash for command */
	Hash = Z_Hash(a_ArgV[0]);
	
	/* Find command for this hash */
	ComNum = (uintptr_t)Z_HashFindEntry(l_CONLCommandHashes, Hash, a_ArgV[0], false);
	ComNum -= 1;
	
	// Check, if not found try variables
	if (ComNum >= l_CONLNumCommands)
	{
		return CLE_UNKNOWNCOMMAND;	// not found
	}
	
	/* Execute */
	return l_CONLCommands[ComNum].ComFunc(a_ArgC, a_ArgV);
}

/* CONL_ExitCodeToStr() -- Converts exit code to string */
const char* CONL_ExitCodeToStr(const CONL_ExitCode_t a_Code)
{
	static const char* const CodeStrs[NUMCONLEXITCODES] =
	{
		"Success",				// CLE_SUCCESS
		"Failure",				// CLE_FAILURE
		"Not An Error String",	// CLE_NOTANERRORSTRING
		"Critical Failure",		// CLE_CRITICALFAILURE
		"Unknown Command",		// CLE_UNKNOWNCOMMAND
		"Unknown Variable",		// CLE_UNKNOWNVARIABLE
		"Invalid Argument",		// CLE_INVALIDARGUMENT
		"Resource Not Found",	// CLE_RESOURCENOTFOUND
		"Connection Refused",	// CLE_CONNECTIONREFUSED
		"Read-only Medium",		// CLE_DISKREADONLY
		"Permission Denied",	// CLE_PERMISSIONDENIED
		"Unknown Sub-Command",	// CLE_UNKNOWNSUBCOMMAND
	};
	
	/* Return static char */
	if (a_Code >= NUMCONLEXITCODES || a_Code < 0)
		return CodeStrs[CLE_NOTANERRORSTRING];
	return CodeStrs[a_Code];
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
	CONL_AddCommand("!", CLC_Exclamation);
	CONL_AddCommand("?", CLC_Question);
	
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
	return (l_CONLActive && !menuactive) || con_startup;
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
	const static uint8_t Faders[TICRATE] =
	{
		VEX_TRANS90, VEX_TRANS90, VEX_TRANS90, VEX_TRANS80, VEX_TRANS80, VEX_TRANS80,
		VEX_TRANS80, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70, VEX_TRANS70, VEX_TRANS60,
		VEX_TRANS60, VEX_TRANS60, VEX_TRANS60, VEX_TRANS50, VEX_TRANS50, VEX_TRANS50,
		VEX_TRANS50, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS40, VEX_TRANS30,
		VEX_TRANS30, VEX_TRANS30, VEX_TRANS30, VEX_TRANS20, VEX_TRANS20, VEX_TRANS20,
		VEX_TRANS20, VEX_TRANS10, VEX_TRANS10, VEX_TRANS10, VEX_TRANSNONE
	};
	size_t i, j;
	tic_t CurrentTime, Left;
	uint8_t v;
	
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
					l_CONLMessageQ[i][j].Flags &= ~VFO_TRANSMASK;
					l_CONLMessageQ[i][j].Flags |= VFO_TRANS((uint32_t)Faders[Left]);
				}
			}
}

/* CONL_HandleEvent() -- Handles extended event for the console */
bool_t CONL_HandleEvent(const I_EventEx_t* const a_Event)
{
	uint8_t Code;
	uint16_t Char;
	uint32_t Old;
	static int32_t HistorySpot = -1;
	size_t i, j, k;
	const char* p;
	
	/* Check */
	if (!a_Event)
		return false;
		
	/* Don't handle events during startup */
	if (con_startup)
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
		else if (CONCTI_HandleEvent(l_CONLInputter, a_Event))
			return true;		// it ate the event
			
		// Always eat everything
		return true;
	}
	
	/* Fell through */
	return false;
}

/* CONL_DrawConsole() -- Draws the console */
void CONL_DrawConsole(void)
{
	bool_t FullCon;
	size_t i, n, j, k, l, BSkip;
	int32_t NumLines, Limit;
	uint32_t bx, x, by, y, bw, bh, Options, conX, conY, conW, conH, DrawCount, DefaultOptions;
	const char* p;
	char TempFill[6];
	static pic_t* BackPic;
	CONL_BasicBuffer_t* Out;
	
	/* Get output buffer */
	Out = &l_CONLBuffers[0];
	
	/* Console is active (draw the console) */
	if (CONL_IsActive())
	{
		//V_DrawStringA(VFONT_STATUSBARSMALL, 0, "The console is {1Active{z!", 100, 100);
		
		// Fullscreen console?
		FullCon = con_startup;
		
		// Console x and y
		conX = CONLPADDING;
		conY = CONLPADDING;
		conW = vid.width - (CONLPADDING * 2);
		
		// Get character dimensions
		bw = V_FontWidth(CONLCONSOLEFONT);
		bh = V_FontHeight(CONLCONSOLEFONT);
		
		// Draw back picture and determines lines to draw
		if (FullCon)
		{
			// Load background?
			if (!BackPic)
				BackPic = W_CacheLumpName("RMD_CB_D", PU_STATIC);
				
			// Blit to entire screen
			V_BlitScalePicExtern(0, 0, 0, BackPic);
			
			// Get height of console
			conH = vid.height;
		}
		
		else
		{
			// Height of console is...
			conH = (vid.height >> 1);
			
			// Draw box
			V_DrawFadeConsBackEx(VEX_COLORMAP(VEX_MAP_RED) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, 0, 0, vid.width, conH);
		}
		
		// Determine line count
		NumLines = conH / bh;
		
		// Draw every line
		DrawCount = 0;
		y = bh * (NumLines - (con_startup ? 0 : 2));
		for (l = 0, i = 0, j = ((Out->EndLine - 1) & Out->MaskLine); i < NumLines; j = ((j - 1) & Out->MaskLine))
		{
			// Lines out of range? Or this is the first line?
			if (!Out->Lines[j] || j == ((Out->StartLine) & Out->MaskLine))
				break;
				
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
			
			// Draw each character
			Options = VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES;
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
					bx = V_DrawCharacterMB(CONLCONSOLEFONT, Options, TempFill, x, y, &BSkip, &Options);
					
				// Normal character
				else
				{
					bx = 0;
					BSkip = 1;
				}
				
				if (bx)			// Some characters may return zero
					x += bw;	// Monospaced instead of variable
				n = (n + BSkip) & Out->MaskPos;
				p = &Out->Buffer[n];
			}
			
			// Go down
			y -= bh;
			DrawCount++;
		}
		
		// Draw console input line (as long as it is not startup)
		if (!con_startup)
		{
			y = conH - bh - CONLPADDING;
			x = conX;
			Options = VFO_COLOR(VEX_MAP_BLUE) | VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES;
			
			// Draw command prompt
			x += V_DrawStringA(CONLCONSOLEFONT, VFO_COLOR(VEX_MAP_BRIGHTWHITE) | VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES, "#>", x, y);
			// Draw input buffer
			x += CONCTI_DrawInput(l_CONLInputter, Options, x, y, vid.width);
		}
		// Draw scrollbar
		if (!con_startup)
		{
			// Pixels per line (of all the console lines)
			by = conH / Out->CountLine;
			bw = conH - 1;
			bh = bw - (by * l_CONLLineOff);
			
			// Draw skipped lines (from bottom)
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_BLACK) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, CONLSCROLLBACK, 1, bw, CONLPADDING - 1, bh);
			
			// Draw lines being seen (in current view)
			bw = bh;
			bh = bw - (by * (DrawCount < NumLines ? DrawCount : NumLines));
			
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_WHITE) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, CONLSCROLLFORE, 1, bw, CONLPADDING - 1, bh);
			
			// Draw lines you should see but you cant
			bw = bh;
			bh = bw - (by * (DrawCount < NumLines ? NumLines - DrawCount : DrawCount - NumLines));
			
			// Less than 1?
			if (bh < 1)
				bh = 1;
				
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_GRAY) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, CONLSCROLLMISS, 1, bw, CONLPADDING - 1, bh);
			
			// Draw remaining black
			bw = bh;
			bh = 1;
			
			V_DrawColorBoxEx(VEX_COLORMAP(VEX_MAP_BLACK) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, CONLSCROLLBACK, 1, bw, CONLPADDING - 1, bh);
		}
	}
	
	/* Not active, draw per player messages */
	else
	{
		n = cv_splitscreen.value + 1;
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
					bw = bw + (vid.width >> 1);
					bh = bh + (vid.height >> 1);
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
						if (x >= bw)
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
}

/* CLC_Version() -- ReMooD version info */
CONL_ExitCode_t CLC_Version(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_OutputF("ReMooD %i.%i%c \"%s\"\n", REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
	CONL_OutputF("  Please visit %s for more information.\n", REMOOD_URL);
	
	/* Return success always */
	return CLE_SUCCESS;
}

/* CLC_Dep() -- Access to deprecated console */
CONL_ExitCode_t CLC_Dep(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	uint32_t i, j, n;
	
	/* Clear */
	memset(Buf, 0, sizeof(Buf));
	
	/* Place command in buffer */
	// The old console code uses a single line
	// Also, ignore the actual "dep" part
	for (i = 1; i < a_ArgC; i++)
	{	
		// Always quote everything
		strncat(Buf, "\"", BUFSIZE);
		strncat(Buf, a_ArgV[i], BUFSIZE);
		strncat(Buf, "\"", BUFSIZE);
	}
	
	// Add newline
	strncat(Buf, "\n", BUFSIZE);
	
	/* Send command to be executed */
	COM_BufAddText(Buf);

	/* Flush command (to execute it) */
	COM_BufExecute();
	
	/* Always return success, no real way to get it to work */
	return CLE_SUCCESS;
#undef BUFSIZE
}

/* CLC_Exec() -- Execute command */
CONL_ExitCode_t CLC_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	return CONL_Exec(a_ArgC - 1, a_ArgV + 1);
}

/* CLC_Exclamation() -- Runs command and reverses error status */
CONL_ExitCode_t CLC_Exclamation(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_ExitCode_t Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Which do we return? */
	if (!Code)
		return CLE_FAILURE;
	else
		return CLE_SUCCESS;
}

/* CLC_Question() -- Runs command and sets error number */
CONL_ExitCode_t CLC_Question(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_ExitCode_t Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Set error */
	g_CONLError = Code;
	
	/* Always succeed */
	return CLE_SUCCESS;
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
	CONS_Printf("English keymap.\n");
}

//  Choose french keymap
//
void CONS_French_f(void)
{
	shiftxform = french_shiftxform;
	con_keymap = french;
	CONS_Printf("French keymap.\n");
}

char* bindtable[NUMINPUTS];

void CONS_Bind_f(void)
{
	int na, key;
	
	na = COM_Argc();
	
	if (na != 2 && na != 3)
	{
		CONS_Printf("bind <keyname> [<command>]\n");
		CONS_Printf("\2bind table :\n");
		na = 0;
		for (key = 0; key < NUMINPUTS; key++)
			if (bindtable[key])
			{
				CONS_Printf("%s : \"%s\"\n", G_KeynumToString(key), bindtable[key]);
				na = 1;
			}
		if (!na)
			CONS_Printf("Empty\n");
		return;
	}
	
	key = G_KeyStringtoNum(COM_Argv(1));
	if (!key)
	{
		CONS_Printf("Invalid key name\n");
		return;
	}
	
	if (bindtable[key] != NULL)
	{
		Z_Free(bindtable[key]);
		bindtable[key] = NULL;
	}
	
	if (na == 3)
		bindtable[key] = Z_StrDup(COM_Argv(2), PU_STATIC, NULL);
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
	V_InitializeColormaps();
	
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
		
	for (i = 0; i < NUMINPUTS; i++)
		bindtable[i] = NULL;
		
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
	COM_AddCommand("bind", CONS_Bind_f);
	// set console full screen for game startup MAKE SURE VID_Init() done !!!
	con_destlines = vid.height;
	con_curlines = vid.height;
	consoletoggle = false;
	
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

//  Console print! Wahooo! Lots o fun!
//
void CONS_Printf(char* fmt, ...)
{
	va_list argptr;
	char txt[512];
	static bool_t AlreadyDrawn;	// Draw once
	
	va_start(argptr, fmt);
#if _MSC_VER >= 1400
	vsprintf_s(txt, 512, fmt, argptr);
#elif defined(__GNUC__)
	vsnprintf(txt, 512, fmt, argptr);
#else
	vsprintf(txt, fmt, argptr);
#endif
	va_end(argptr);
	
	if (devparm || !con_started /* || !graphics_started */ )
	{
//#if !defined( _WIN32) && !defined( __OS2__)
		I_OutputText(txt);
//#endif
		if (!devparm)
			return;
	}
	// GhostlyDeath <September 10, 2011> -- Write to light console
	CONL_RawPrint(&l_CONLBuffers[0], txt);
	
	// write message in con text buffer
	CON_Print(txt);
	
	// make sure new text is visible
	con_scrollup = 0;
	
	// if not in display loop, force screen update
	if (con_startup)
	{
	
		/*#if defined( _WIN32) || defined( __OS2__)
		   // show startup screen and message using only 'software' graphics
		   // (rendermode may be hardware accelerated, but the video mode is not set yet)
		   CON_DrawBackpic (con_backpic, 0, vid.width);    // put console background
		   I_LoadingScreen ( txt );
		   #else */
		// here we display the console background and console text
		// (no hardware accelerated support for these versions)
		
		// GhostlyDeath <November 4, 2010> -- If we aren't devparming, draw once
		if ((devparm && !g_QuietConsole) || ((!devparm || g_QuietConsole) && !AlreadyDrawn))
		{
			CONL_DrawConsole();
			
			//CON_Drawer();
			I_FinishUpdate();	// page flip or blit buffer
			
			AlreadyDrawn = true;
		}
//#endif
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
//bool_t altdown;
	bool_t shiftdown;
	int i;
	
// sequential completions a la 4dos
	char completion[80];
	int comskips, varskips;
	
	char* cmd;
	int key;
	
	if (chat_on)
		return false;
		
	// special keys state
	if (ev->data1 == KEY_SHIFT && ev->type == ev_keyup)
	{
		shiftdown = false;
		return false;
	}
	//else if (ev->data1 == KEY_ALT)
	//{
	//    altdown = (ev->type == ev_keydown);
	//    return false;
	//}
	
	// let go keyup events, don't eat them
	if (ev->type != ev_keydown)
		return false;
		
	key = ev->data1;
	
//
//  check for console toggle key
//
	if (key == gamecontrol[0][gc_console][0] || key == gamecontrol[0][gc_console][1])
	{
		consoletoggle = true;
		return true;
	}
//
//  check other keys only if console prompt is active
//
	if (!consoleready && key < NUMINPUTS)	// metzgermeister: boundary check !!
	{
		if (bindtable[key])
		{
			COM_BufAddText(bindtable[key]);
			COM_BufAddText("\n");
			return true;
		}
		return false;
	}
	// eat shift only if console active
	if (key == KEY_SHIFT)
	{
		shiftdown = true;
		return true;
	}
	// escape key toggle off console
	if (key == KEY_ESCAPE)
	{
		consoletoggle = true;
		return true;
	}
	// command completion forward (tab) and backward (shift-tab)
	if (key == KEY_TAB)
	{
		// TOTALLY UTTERLY UGLY NIGHT CODING BY FAB!!! :-)
		//
		// sequential command completion forward and backward
		
		// remember typing for several completions (a-la-4dos)
		if (inputlines[inputline][input_cx - 1] != ' ')
		{
			if (strlen(inputlines[inputline] + 1) < 80)
				strcpy(completion, inputlines[inputline] + 1);
			else
				completion[0] = 0;
				
			comskips = varskips = 0;
		}
		else
		{
			if (shiftdown)
			{
				if (comskips < 0)
				{
					if (--varskips < 0)
						comskips = -(comskips + 2);
				}
				else if (comskips > 0)
					comskips--;
			}
			else
			{
				if (comskips < 0)
					varskips++;
				else
					comskips++;
			}
		}
		
		if (comskips >= 0)
		{
			cmd = COM_CompleteCommand(completion, comskips);
			if (!cmd)
				// dirty:make sure if comskips is zero, to have a neg value
				comskips = -(comskips + 1);
		}
		if (comskips < 0)
			cmd = CV_CompleteVar(completion, varskips);
			
		if (cmd)
		{
			memset(inputlines[inputline] + 1, 0, CON_MAXPROMPTCHARS - 1);
			strcpy(inputlines[inputline] + 1, cmd);
			input_cx = strlen(cmd) + 1;
			inputlines[inputline][input_cx] = ' ';
			input_cx++;
			inputlines[inputline][input_cx] = 0;
		}
		else
		{
			if (comskips > 0)
				comskips--;
			else if (varskips > 0)
				varskips--;
		}
		
		return true;
	}
	// move up (backward) in console textbuffer
	if (key == KEY_PGUP)
	{
		if (con_scrollup < (con_totallines - ((con_curlines - 16) >> 3)))
			con_scrollup++;
		return true;
	}
	else if (key == KEY_PGDN)
	{
		if (con_scrollup > 0)
			con_scrollup--;
		return true;
	}
	// oldset text in buffer
	if (key == KEY_HOME)
	{
		con_scrollup = (con_totallines - ((con_curlines - 16) >> 3));
		return true;
	}
	else
		// most recent text in buffer
		if (key == KEY_END)
		{
			con_scrollup = 0;
			return true;
		}
	// command enter
	if (key == KEY_ENTER)
	{
		if (input_cx < 2)
			return true;
			
		// push the command
		COM_BufAddText(inputlines[inputline] + 1);
		COM_BufAddText("\n");
		
		CONS_Printf("%s\n", inputlines[inputline]);
		
		inputline = (inputline + 1) & 31;
		inputhist = inputline;
		
		memset(inputlines[inputline], 0, CON_MAXPROMPTCHARS);
		inputlines[inputline][0] = CON_PROMPTCHAR;
		input_cx = 1;
		
		return true;
	}
	// backspace command prompt
	if (key == KEY_BACKSPACE)
	{
		if (input_cx > 1)
		{
			input_cx--;
			inputlines[inputline][input_cx] = 0;
		}
		return true;
	}
	// move back in input history
	if (key == KEY_UPARROW)
	{
		// copy one of the previous inputlines to the current
		do
		{
			inputhist = (inputhist - 1) & 31;	// cycle back
		}
		while (inputhist != inputline && !inputlines[inputhist][1]);
		
		// stop at the last history input line, which is the
		// current line + 1 because we cycle through the 32 input lines
		if (inputhist == inputline)
			inputhist = (inputline + 1) & 31;
			
		memcpy(inputlines[inputline], inputlines[inputhist], CON_MAXPROMPTCHARS);
		input_cx = strlen(inputlines[inputline]);
		
		return true;
	}
	// move forward in input history
	if (key == KEY_DOWNARROW)
	{
		if (inputhist == inputline)
			return true;
		do
		{
			inputhist = (inputhist + 1) & 31;
		}
		while (inputhist != inputline && !inputlines[inputhist][1]);
		
		memset(inputlines[inputline], 0, CON_MAXPROMPTCHARS);
		
		// back to currentline
		if (inputhist == inputline)
		{
			inputlines[inputline][0] = CON_PROMPTCHAR;
			input_cx = 1;
		}
		else
		{
			strcpy(inputlines[inputline], inputlines[inputhist]);
			input_cx = strlen(inputlines[inputline]);
		}
		return true;
	}
	// allow people to use keypad in console (good for typing IP addresses) - Calum
	if (key >= KEY_KEYPAD7 && key <= KEY_KPADDEL)
	{
		char keypad_translation[] = { '7', '8', '9', '-',
			'4', '5', '6', '+',
			'1', '2', '3',
			'0', '.'
		};
		
		key = keypad_translation[key - KEY_KEYPAD7];
	}
	else if (key == KEY_KPADSLASH)
		key = '/';
	else if (con_keymap == french)
		key = ForeignTranslation((uint8_t)key);
		
	if (shiftdown)
		key = shiftxform[key];
		
	// enter a char into the command prompt
	if (key < 32 || key > 127)
		return false;
		
	// add key to cmd line here
	if (input_cx < CON_MAXPROMPTCHARS)
	{
		// make sure letters are lowercase for commands & cvars
		if (key >= 'A' && key <= 'Z')
			key = key + 'a' - 'A';
			
		inputlines[inputline][input_cx] = key;
		inputlines[inputline][input_cx + 1] = 0;
		input_cx++;
	}
	
	return true;
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
	CONS_Printf("\2%s", msg);	// write error msg in different colour
	CONS_Printf("Press ENTER to continue\n");
	
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
