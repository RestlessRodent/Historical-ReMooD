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

/*****************
*** STRUCTURES ***
*****************/

typedef void (*CONL_FlushFunc_t)(const char* const a_Buf);

/* CONL_BasicBuffer_t -- A basic buffer for the console */
typedef struct CONL_BasicBuffer_s
{
	char* Buffer;									// Actual buffer
	char** Lines;									// Lines in buffer
	size_t Size;									// Size of the buffer
	size_t NumLines;								// Number of lines in buffer
	
	uintmax_t MaskPos;								// Position mask
	uintmax_t StartPos;								// Start position of buffer
	uintmax_t EndPos;								// End position of buffer
	uintmax_t MaskLine;								// Line mask
	uintmax_t StartLine;							// First line in buffer
	uintmax_t EndLine;								// Last line in buffer
	uintmax_t DataLine;								// Line for the flush function
	
	CONL_FlushFunc_t FlushFunc;						// Function to call on '\n'
} CONL_BasicBuffer_t;

/*************
*** LOCALS ***
*************/

static CONL_BasicBuffer_t l_CONLBuffers[2];			// Input/Output Buffer

/****************
*** FUNCTIONS ***
****************/

/*** Static Stuff ***/
/* CONLS_InitConsole() -- Initializes a buffer */
static bool_t CONLS_InitConsole(CONL_BasicBuffer_t* const a_Buffer, const uintmax_t a_Size, const CONL_FlushFunc_t a_FlushFunc)
{
	uintmax_t Size, i;
	
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
static void CONLFF_OutputFF(const char* const a_Buf)
{
	const char* const p;
	
	/* Check */
	if (!a_Buf)
		return;
	
	/* Print text */
	extern bool_t con_started;
	if (devparm || !con_started)
		I_OutputMsg("%s\n", a_Buf);
}

/* CONLFF_InputFF() -- Line is flushed from the input buffer */
static void CONLFF_InputFF(const char* const a_Buf)
{
	/* Check */
	if (!a_Buf)
		return;
	
	/* Create command line like stuff here */
	// Tokenize between space and single/double quotes
	// This makes command processing hell of alot easier
	// So calling console functions is like main(argc, argv)
}

/*** Base Console ***/
/* CONL_Init() -- Initializes the light console */
bool_t CONL_Init(const uintmax_t a_OutBS, const uintmax_t a_InBS)
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
	
	/* Success! */
	return true;
}

/* CONL_Stop() -- Stops the console */
void CONL_Stop(void)
{
}

/* CONL_RawPrint() -- Prints text directly to buffer */
const size_t CONL_RawPrint(CONL_BasicBuffer_t* const a_Buffer, const char* const a_Text)
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
			a_Buffer->StartPos = POSMASK(a_Buffer->StartPos + 1);
			
			// Does this new position surpass the first line in the buffer?
			if (a_Buffer->Lines[LINEMASK(a_Buffer->StartLine)] && &a_Buffer->Buffer[POSMASK(a_Buffer->StartPos)] >= a_Buffer->Lines[LINEMASK(a_Buffer->StartLine)])
			{
				// Ignore data line on collision
				if (LINEMASK(a_Buffer->DataLine) == LINEMASK(a_Buffer->StartLine))
					a_Buffer->DataLine = LINEMASK(a_Buffer->DataLine + 1);
					
				// Set current line to NULL and move up
				a_Buffer->Lines[LINEMASK(a_Buffer->StartLine++)] = NULL;
			}
			
			&a_Buffer->Buffer[a_Buffer->StartPos] >= a_Buffer->Lines[a_Buffer->StartLine];
		}
		
		// Newline?
		if (*p == '\n')
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
				
				if (!w)	// Just use first character (ouch)
					w = &a_Buffer->Buffer[0];
				
					// Wrapped
				if (z < w)
					j = (z + a_Buffer->Size) - w;
					// Not wrapped
				else
					j = z - w;
				
				// Allocate buffer
				Que[Q] = Z_Malloc(j + 2, PU_STATIC, NULL);
				
				// Copy chars into buffer
				i = POSMASK(w - a_Buffer->Buffer);
				for (k = 0; k < j - 1; i = POSMASK(i + 1))
					Que[Q][k++] = a_Buffer->Buffer[POSMASK(i)];
				
				// Increase Q
				Q++;
			}
			
			// Write current line to buffer
			a_Buffer->Lines[LINEMASK(a_Buffer->EndLine)] = &a_Buffer->Buffer[POSMASK(a_Buffer->EndPos)];
			a_Buffer->EndLine = LINEMASK(a_Buffer->EndLine + 1);
			
			// Unset current line to prevent old corruption
			a_Buffer->Lines[LINEMASK(a_Buffer->EndLine)] = NULL;
			
			// Start collision?
			if (LINEMASK(a_Buffer->EndLine) == LINEMASK(a_Buffer->StartLine))
			{
				if (LINEMASK(a_Buffer->DataLine) == LINEMASK(a_Buffer->StartLine))
					a_Buffer->DataLine = LINEMASK(a_Buffer->DataLine + 1);
				
				a_Buffer->StartLine = LINEMASK(a_Buffer->StartLine + 1);
			}
		}
	}
	
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
const size_t CONL_PrintV(const bool_t a_InBuf, const char* const a_Format, va_list a_ArgPtr)
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
const size_t CONL_UnicodePrintV(const bool_t a_InBuf, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr)
{
	return 0;
}

/* CONL_OutputF() -- Prints formatted text to the output buffer */
const size_t CONL_OutputF(const char* const a_Format, ...)
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
const size_t CONL_InputF(const char* const a_Format, ...)
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
const size_t CONL_OutputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...)
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
const size_t CONL_InputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...)
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
const bool_t CONL_IsActive(void)
{
	return false;
}

/* CONL_SetActive() -- Sets whether the console is active or not */
// Returns the old flag
const bool_t CONL_SetActive(const bool_t a_Set)
{
	return false;
}

/* CONL_DrawConsole() -- Draws the console */
void CONL_DrawConsole(void)
{
}

/*****************************************************************************/

/**************
*** GLOBALS ***
**************/

bool_t g_QuietConsole = false;								// Mute startup console

/*************
*** LOCALS ***
*************/

/* Root Console */
CONEx_Console_t* l_RootConsole = NULL;

/* Drawing parms */
CONEx_Console_t* l_ActiveConsole = NULL;
bool_t l_ExConsoleOn = false;
int8_t l_ExConsoleLines = 0;
void* l_BackPic = NULL;
bool_t l_BackPicIsPicT = false;

/* Command Control */;
char l_ExInputBuffer[CON_MAXPROMPTCHARS] = "";
size_t l_ExInputCursor = 0;

/****************
*** FUNCTIONS ***
****************/

/* CONEx_CreateBuffer() -- Creates a buffer of size */
CONEx_Buffer_t* CONEx_CreateBuffer(const size_t Size)
{
	CONEx_Buffer_t* Temp;
	
	/* Check */
	if (Size < 1024)
		return CONEx_CreateBuffer(1024);	// Recursive fix
		
	/* Allocate */
	Temp = Z_Malloc(sizeof(CONEx_Buffer_t), PU_STATIC, NULL);
	
	if (!Temp)
		return NULL;
	
	/* Clear */
	memset(Temp, 0, sizeof(CONEx_Buffer_t));
	
	/* Create mini buffers */
	// Sizes
	Temp->BufferSize = Size;
	Temp->LineSize = (((Size >> 4) < 500) ? (Size >> 4) : 500);
	
	// Allocate
	Temp->Buffer = Z_Malloc(sizeof(char) * Temp->BufferSize, PU_STATIC, NULL);
	Temp->Lines = Z_Malloc(sizeof(char*) * Temp->LineSize, PU_STATIC, NULL);
	
	// Check
	if (!Temp->Buffer || !Temp->Lines)
	{
		if (Temp->Buffer)
			Z_Free(Temp->Buffer);
		if (Temp->Lines)
			Z_Free(Temp->Lines);
		Z_Free(Temp);
		return NULL;
	}
	
	/* Return */
	return Temp;
}

/* CONEx_DestroyBuffer() -- Destroys a buffer */
void CONEx_DestroyBuffer(CONEx_Buffer_t* const Buffer)
{
	char* Former;
	char** Former2;
	
	/* Check */
	if (!Buffer)
		return;
	
	/* Free data */
	// Remember former (in case Z_Free uses this buffer!)
	Former = Buffer->Buffer;
	Buffer->Buffer = NULL;
	if (Former)
		Z_Free(Former);
		
	Former2 = Buffer->Lines;
	Buffer->Lines = NULL;
	if (Former2)
		Z_Free(Former2);
	
	/* Clear */
	memset(Buffer, 0, sizeof(CONEx_Buffer_t));
	
	/* Free self */
	Z_Free(Buffer);
}

/* CONEx_BufferWrite() -- Write to a buffer */
void CONEx_BufferWrite(CONEx_Buffer_t* const Buffer, const char* const Text)
{
	const char* p;
	size_t LastLine;
	
	/* Check */
	if (!Buffer || !Text)
		return;
	
	/* Set */
	p = Text;
	
	/* Loop write into buffer */
	while (*p)
	{
		// Current line is empty
		if (!Buffer->Lines[Buffer->LineWrite])
			Buffer->Lines[Buffer->LineWrite] = &Buffer->Buffer[Buffer->BufferWrite];
		
		// Write to current buffer position
		Buffer->Buffer[Buffer->BufferWrite++] = *p;
		
		// Reached end of buffer?
		if (Buffer->BufferWrite == Buffer->BufferSize)
			Buffer->BufferWrite = 0;
		
		// Ensure zero
		Buffer->Buffer[Buffer->BufferWrite] = 0;
		
		// Write position hit start position
		if (Buffer->BufferWrite == Buffer->BufferStart)
			Buffer->BufferStart++;
		
		// Start is at end of buffer
		if (Buffer->BufferStart == Buffer->BufferSize)
			Buffer->BufferStart = 0;
		
		// Newline?
		if (*p == '\n')
		{
			// Increase written line
			LastLine = Buffer->LineWrite++;
			
			// Write hit end?
			if (Buffer->LineWrite == Buffer->LineSize)
				Buffer->LineWrite = 0;
			
			// Write hit start?
			if (Buffer->LineWrite == Buffer->LineStart)
				Buffer->LineStart++;
			
			// Start hit end?
			if (Buffer->LineStart == Buffer->LineSize)
				Buffer->LineStart = 0;
			
			// Ensure NULL
			Buffer->Lines[Buffer->LineWrite] = NULL;
			
			// Execute line
			if (Buffer->WroteLineFunc)
				Buffer->WroteLineFunc(Buffer->Parent, Buffer, Buffer->Lines[LastLine]);
		}
		
		// Carriage return?
		else if (*p == '\r')
		{
			// If the current line is non-null (that is a line with text)
			if (Buffer->Lines[Buffer->LineWrite])
				// Move write position to start
				Buffer->BufferWrite = Buffer->Lines[Buffer->LineWrite] - Buffer->Buffer;
		}
		
		// Increment p
		p++;
	}
}

/* CONEx_CreateConsole() -- Create a new console */
CONEx_Console_t* CONEx_CreateConsole(void)
{
	static uint32_t BaseUUID;
	CONEx_Console_t* Temp;
	
	/* Allocate */
	Temp = Z_Malloc(sizeof(CONEx_Console_t), PU_STATIC, NULL);
	
	if (!Temp)
		return NULL;
	
	/* Clear */
	memset(Temp, 0, sizeof(CONEx_Console_t));
	
	/* Create buffers */
	Temp->Output = CONEx_CreateBuffer(CON_BUFFERSIZE);
	Temp->Command = CONEx_CreateBuffer(CON_BUFFERSIZE);
	
	// Set parent
	if (Temp->Output)
		Temp->Output->Parent = Temp;
	
	// Set parent
	if (Temp->Command)
		Temp->Command->Parent = Temp;
	
	/* Settings */
	Temp->UUID = ++BaseUUID;
	
	return Temp;
}

/* CONEx_DestroyConsole() -- Destroys a console */
void CONEx_DestroyConsole(CONEx_Console_t* const Console)
{
	size_t i;
	
	/* Check */
	if (!Console)
		return;
	
	/* First detach all child consoles */
	while (Console->NumKids)
		CONEx_DetachConsole(Console, Console->Kids[0]);
	
	/* Destroy buffers */
	// Output
	if (Console->Output)
		CONEx_DestroyBuffer(Console->Output);
	Console->Output = NULL;
	
	// Commands
	if (Console->Command)
		CONEx_DestroyBuffer(Console->Command);
	Console->Command = NULL;
	
	/* Free self */
	Z_Free(Console);
}

/* CONEx_GetRootConsole() -- Return root console */
CONEx_Console_t* CONEx_GetRootConsole(void)
{
	return l_RootConsole;
}

/* CONEx_GetActiveConsole() -- Get the current active console */
CONEx_Console_t* CONEx_GetActiveConsole(void)
{
	return l_ActiveConsole;
}

/* CONEx_FindComVar() -- Find variable by name */
CONEx_VarTypeList_t* CONEx_FindComVar(CONEx_Console_t* const Console, const char* const String)
{
	/* Check */
	// Basic
	if (!Console || !String)
		return;
	
	// Sanity
	if (strlen(String) >= CONEX_MAXVARIABLENAME)
		return;
	
	/* Return */
	return CONEx_FindComVarHash(Console, CONEx_HashString(String));
}

/* CONEx_FindComVarHash() -- Find command or variable by hash */
CONEx_VarTypeList_t* CONEx_FindComVarHash(CONEx_Console_t* const Console, const uint32_t Hash)
{
	CONEx_VarTypeList_t* Rover;
	
	/* Check */
	if (!Console)
		return;
	
	/* Search Hash List */
	
	/* Long search since the hash list failed us? */
	for (Rover = Console->ComVarList; Rover; Rover = Rover->Next)
	{
		// Command?
		if (!Rover->IsVariable)
			if (Rover->Data.Command.Hash == Hash)
				return Rover;
			else
				continue;
		
		// Variable?
		else
			if (Rover->Data.Variable.Hash == Hash)
				return Rover;
			else
				continue;
	}
	
	/* Failed */
	return NULL;
}

/* CONEx_HashString() -- Converts string to a hashcode */
uint32_t CONEx_HashString(const char* const Name)
{
	size_t i, n;
	uint32_t Ret = 0;
	
	/* Check */
	if (!Name)
		return 0;
	
	/* Hash */
	n = strlen(Name);
	for (i = 0; i < n; i++)
		Ret ^= (uint32_t)((toupper(Name[i]) - 32) & 0x3F) << (6 * (i % 5));
	
	// Don't make zero hashes
	if (!Ret)
		Ret++;
	
	/* Return */
	return Ret;
}

/* CONEx_AddCommand() -- Add command to the console */
void CONEx_AddCommand(CONEx_Console_t* const Console, const CONEx_Command_t* const Command)
{
	uint32_t Hash;
	uint8_t HashBit;
	CONEx_VarTypeList_t* New;
	
	/* Check */
	// Basic
	if (!Console || !Command)
		return;
	
	// Sanity
	if (!strlen(Command->Name) || strlen(Command->Name) >= CONEX_MAXVARIABLENAME || !Command->Func)
		return;
		
	/* Search console for existing command or variable */
	if (CONEx_FindComVar(Console, Command->Name))
		return;
	
	/* Allocate */
	New = Z_Malloc(sizeof(CONEx_VarTypeList_t), PU_STATIC, NULL);
	
	/* Copy Data */
	strncpy(New->Data.Command.Name, Command->Name, CONEX_MAXVARIABLENAME);
	New->Data.Command.Hash = CONEx_HashString(New->Data.Command.Name);
	New->Data.Command.Flags = Command->Flags;
	New->Data.Command.Func = Command->Func;
	New->Data.Command.DepFunc = Command->DepFunc;
	
	/* Link to console */
	if (!Console->ComVarList)
		Console->ComVarList = New;
	else
	{
		// Link to start
		New->Next = Console->ComVarList;
		New->Next->Prev = New;
		Console->ComVarList = New;
		
		// Add to hash list
		HashBit = New->Data.Command.Hash & 0xFF;
	}
}

/* CONEx_AddVariable() -- Add variable to the console */
void CONEx_AddVariable(CONEx_Console_t* const Console, const CONEx_Variable_t* const Variable)
{
}

/* CONEx_AttachConsole() -- Attach sub console to another console */
void CONEx_AttachConsole(CONEx_Console_t* const ToThis, CONEx_Console_t* const Attacher)
{
	/* Check */
	if (!ToThis || !Attacher)
		return;
}

/* CONEx_DetachConsole() -- Detach sub console from another console */
void CONEx_DetachConsole(CONEx_Console_t* const FromThis, CONEx_Console_t* const Detacher)
{
	/* Check */
	if (!FromThis || !Detacher)
		return;
}

/* CONEx_CorrectChar() -- Correct Character */
static char CONEx_CorrectChar(int EventKey)
{
	switch (EventKey)
	{
		case KEY_EQUALS:
			return '=';
		
		case KEY_MINUS:
			return '-';
			
			// Fallback
		default:
			if (EventKey >= 0x20 && EventKey <= 0x7F)
				return EventKey;
			return 0;
	}
};

/* CONEx_Responder() -- Respond to active console */
bool_t CONEx_Responder(event_t* const Event)
{
	static bool_t ShiftDown;
	char MBIn[5];
	char c;
	size_t len;
	
	/* Check */
	if (!Event || !l_RootConsole)
		return false;
	
	/* Check shift */
	if (Event->type == ev_keydown && Event->data1 == KEY_SHIFT)
		ShiftDown = 1;
	else if (Event->type == ev_keyup && Event->data1 == KEY_SHIFT)
		ShiftDown = 0;
	
	/* Disabled console */
	if (!l_ExConsoleOn)
	{
		if (Event->type == ev_keydown)
		{
			// Console key shuts down console
			if (Event->data1 == gamecontrol[0][gc_console][0] ||
				Event->data1 == gamecontrol[0][gc_console][1])
			{
				l_ExConsoleOn = true;
				return true;
			}
		}
	}
	
	/* Enabled console */
	else
	{
		if (Event->type == ev_keydown)
		{
			// Console key shuts down console
			if (Event->data1 == gamecontrol[0][gc_console][0] ||
				Event->data1 == gamecontrol[0][gc_console][1])
			{
				l_ExConsoleOn = false;
				return true;
			}
		
			// Based on key
			switch (Event->data1)
			{
					// Backspace multibyte characters
				case KEY_BACKSPACE:
					len = strlen(l_ExInputBuffer);
					
					if (!(l_ExInputBuffer[len - 1] & 0x80))		// Single
						l_ExInputBuffer[len - 1] = 0;
					else							// Multi
						do
						{
							c = l_ExInputBuffer[len - 1];
							l_ExInputBuffer[len - 1] = 0;
							len--;
						} while (len > 1 && (c & 0xC0) != 0xC0);
					return true;
					
					// Enter command
				case KEY_ENTER:
					// Only if it has length
					if (!strlen(l_ExInputBuffer))
						return true;
					
					// Send to write of active console
					if (l_ActiveConsole->Command)
					{
						// Append \n at end
						strncat(l_ExInputBuffer, "\n", CON_MAXPROMPTCHARS);
						
						// Dirty hack \n at end
						l_ExInputBuffer[CON_MAXPROMPTCHARS - 1] = 0;
						l_ExInputBuffer[CON_MAXPROMPTCHARS - 2] = '\n';
						
						// Write to command buffer
						CONEx_BufferWrite(l_ActiveConsole->Command, l_ExInputBuffer);
					}
					
					// Clear
					memset(l_ExInputBuffer, 0, sizeof(l_ExInputBuffer));
					return true;
					
					// Close console
				case KEY_ESCAPE:
					l_ExConsoleOn = false;
					return true;
					
				default:
					break;
			}
		
			// Unicode handled input (recommended)
			if (Event->typekey)
			{
				// Convert to multibyte
				V_ExtWCharToMB(Event->typekey, MBIn);
		
				// Add to buffer
				strncat(l_ExInputBuffer, MBIn, CON_MAXPROMPTCHARS);
				
				return true;
			}
			
			// ASCII handled input (not recommended for foreigners)
			else if (CONEx_CorrectChar(Event->data1) >= 0x20 && CONEx_CorrectChar(Event->data1) <= 0x7E)
			{
				if (ShiftDown)
					MBIn[0] = shiftxform[CONEx_CorrectChar(Event->data1)];
				else
					MBIn[0] = CONEx_CorrectChar(Event->data1);
				MBIn[1] = 0;
				
				strncat(l_ExInputBuffer, MBIn, CON_MAXPROMPTCHARS);
				
				return true;
			}
		}
	}
	
	/* Not handled so return */
	return false;
}

/* CONEx_Ticker() -- Tick primary console */
void CONEx_Ticker(void)
{
	/* Check */
	if (!l_RootConsole)
		return;
	
	/* No active console? */
	if (!l_ActiveConsole)
		l_ActiveConsole = l_RootConsole;
	
	/* Console off? */
	if (!l_ExConsoleOn)
	{
		// Draw console up
		if (l_ExConsoleLines > 0)
			l_ExConsoleLines--;
	}
	
	/* Console on? */
	else
	{
		// Draw console down
		if (l_ExConsoleLines < 25)
			l_ExConsoleLines++;
	}
}

/* CONEx_Drawer() -- Draw current console */
void CONEx_Drawer(void)
{
#define BUFSIZE 512
	int32_t BottomCon, w, h;
	char Buf[BUFSIZE];
	char* b;
	char* Line;
	size_t MBSkip;
	int n, m, k, i, j, z;
	uint32_t ColorBits;
	int8_t ConsoleLines;
	bool_t DoPrompt;
	bool_t FadeBack;
	
	/* Check */
	if (!l_RootConsole)
		return;
		
	/* No active console? */
	if (!l_ActiveConsole)
		l_ActiveConsole = l_RootConsole;
	
	/* Console off? -- draw last few lines as hud text */
	if (!l_ExConsoleOn && !con_startup)
	{
		// Get height of small font
		h = V_FontHeight(VFONT_SMALL);
		w = V_FontWidth(VFONT_SMALL);
		
		// Draw last lines in buffer
		if (l_ActiveConsole && l_ActiveConsole->Output && l_ActiveConsole->Output->Lines)
			//for (i = 1, n = 5; n >= 0; n--, i++)
			for (i = 1, n = 0; n < 5; n++, i++)
			{
				// Get line
				j = l_ActiveConsole->Output->LineWrite - i;
				
				while (j < 0)	// Don't allow below zero
					j += l_ActiveConsole->Output->LineSize;
				while (j >= l_ActiveConsole->Output->LineSize)	// Don't allow above size
					j -= l_ActiveConsole->Output->LineSize;
				
				Line = l_ActiveConsole->Output->Lines[j];
				ColorBits = 0;
				
				// No line?
				if (!Line)
					break;
				
				b = Line;
				for (MBSkip = 1, m = 0, b = Line; *b && *b != '\n'; b += MBSkip, m++)
				{
					// Standard White Value
					if (*b == 0x02)
					{
						ColorBits = VEX_MAP_WHITE;
						MBSkip = 1;
					}
					
					// Beep (but don't beep, just blink)
					else if (*b == 0x03)
					{
						ColorBits = ((gametic & 0x08) ? VEX_MAP_WHITE : 0);
						MBSkip = 1;
					}
					
					// Draw character
					else
						V_DrawCharacterMB(
								VFONT_SMALL,
								VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
								b,
								4 + (w * m),
								(-(i - 5)) * h,
								&MBSkip
							);
				}
			}
	}
	
	/* Console on? -- draw background */
	else
	{
		// Force full lines when starting
		if (con_startup)
		{
			ConsoleLines = (vid.height / V_FontHeight(VFONT_SMALL)) + 1;
			DoPrompt = false;
			FadeBack = false;
		}
		
		// Normal operation
		else
		{
			ConsoleLines = l_ExConsoleLines;
			DoPrompt = true;
			FadeBack = true;
		}
		
		// Find bottom of console
		BottomCon = ConsoleLines * V_FontHeight(VFONT_SMALL);
		
		// Draw background (either fade or image)
		if (FadeBack)
			V_DrawFadeConsBackEx(
					(VEX_MAP_RED << VEX_COLORMAPSHIFT) | VEX_NOSCALESTART | VEX_NOSCALESCREEN,
					0,
					0,
					vid.width, BottomCon
				);
		// Draw as pic_t
		else if (l_BackPicIsPicT)
			V_BlitScalePicExtern(0, 0, 0, l_BackPic);
			
		// Draw as patch_t
		else
		{
			// TODO
		}
		
		// Draw ReMooD version
		V_StringDimensionsA(VFONT_OEM, VFO_NOSCALESTART | VFO_NOSCALEPATCH, REMOOD_FULLVERSIONSTRING, &w, &h);
		
		if (BottomCon - h - 4 > 0)
			V_DrawStringA(
					VFONT_OEM,
					VEX_MAP_ORANGE | VFO_NOSCALESTART | VFO_NOSCALEPATCH,
					REMOOD_FULLVERSIONSTRING,
					vid.width - w - 4,
					BottomCon - h - 4
				);
		
		// If console startup and we aren't devparming, don't draw text at all
		if (con_startup && (!devparm || g_QuietConsole))
		{
			// Just say "LOADING..."
			V_DrawStringA(VFONT_LARGE, VFO_CENTERED, "Loading...", 160, 100);
			return;
		}
			
		// Get height of small font
		h = V_FontHeight(VFONT_SMALL);
		w = V_FontWidth(VFONT_SMALL);
		
		// Draw last lines in buffer
		if (l_ActiveConsole && l_ActiveConsole->Output && l_ActiveConsole->Output->Lines)
			for (i = 1, n = ConsoleLines - 4; n >= 0; n--, i++)
			{
				// Get line
				j = l_ActiveConsole->Output->LineWrite - i;
				
				while (j < 0)	// Don't allow below zero
					j += l_ActiveConsole->Output->LineSize;
				while (j >= l_ActiveConsole->Output->LineSize)	// Don't allow above size
					j -= l_ActiveConsole->Output->LineSize;
				
				Line = l_ActiveConsole->Output->Lines[j];
				ColorBits = 0;
				
				// No line?
				if (!Line)
					break;
				
				b = Line;
				for (MBSkip = 1, m = 0, b = Line; *b && *b != '\n'; b += MBSkip, m++)
				{
					// Standard White Value
					if (*b == 0x02)
					{
						ColorBits = VEX_MAP_WHITE;
						MBSkip = 1;
					}
					
					// Beep (but don't beep, just blink)
					else if (*b == 0x03)
					{
						ColorBits = ((gametic & 0x08) ? VEX_MAP_WHITE : 0);
						MBSkip = 1;
					}
					
					// Draw character
					else
						V_DrawCharacterMB(
								VFONT_SMALL,
								VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
								b,
								4 + (w * m),
								(n + 1) * h,
								&MBSkip
							);
				}
			}
		
		// Everything after is prompt
		if (!DoPrompt)
			return;
		
		// Draw current command buffer
		if (BottomCon - h - 4 > 0)
		{
			// Draw dollar prompt
			n = V_DrawStringA(
						VFONT_SMALL,
						VEX_MAP_ORANGE | VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
						"$ ",
						0,
						BottomCon - h - 4
					);
			
			// Draw input buffer
			for (MBSkip = 1, m = 0, b = l_ExInputBuffer; *b; b += MBSkip, m++)
			{
				// Draw input character
				k = V_DrawCharacterMB(
						VFONT_SMALL,
						VEX_MAP_ORANGE | VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
						b,
						n,
						BottomCon - h - 4,
						&MBSkip
					);
				
				// Increment now
				n += k;
			
				// Just in case
				if (!MBSkip)
					MBSkip = 1;
			}
		
			// Draw cursor
			if ((gametic & 0x10))
				V_DrawCharacterMB(
						VFONT_SMALL,
						VEX_MAP_WHITE | VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
						"_",
						n,
						BottomCon - h - 4,
						NULL
					);
		}
	}
#undef BUFSIZE
}

/* CONS_ConExtended_f() -- Old interface to access new console */
void CONS_ConExtended_f(void)
{
	/* Shutdown old console */
	consoletoggle = true;
	
	/* Startup extended console */
	l_ExConsoleOn = true;
}

/* CONEx_CommandWriteLine() -- Default action when line is written to execution buffer */
void CONEx_CommandWriteLine(struct CONEx_Console_s* const Parent, struct CONEx_Buffer_s* const This, const char* const Line)
{
#define BUFSIZE 512
	size_t i, j, k;
	int Quoted = 0;
	char* q;
	int ArgC = 0;
	char** ArgV = NULL;
	CONEx_VarTypeList_t* Found = NULL;
	
	/* Check */
	if (!Parent || !This || !Line)
		return;
	
	/* Tokenize command */
	// Get ArgC and ArgV
	ArgC = j = k = 0;
	ArgV = NULL;
	Quoted = 0;
	
	// Create ArgV
	ArgV = Z_Malloc(sizeof(char*) * BUFSIZE, PU_STATIC, NULL);
	
	q = Line;
	while (*q && *q != '\n')
	{
		// If we hit a whitespace character in non-quoted mode
		// then we make some more args.
		if (!Quoted && (*q == ' ' || *q == '\t'))
		{
			// Only bother if the current argv has something
			if (ArgV[j])
				j++;
			
			// Hit arg limit?
			if (j == BUFSIZE - 1)
				break;
		}
		
		// normal
		else
		{
			// Start quoting
			if (!Quoted && *q == '\"')
				Quoted = *q;
			
			// End quoting
			else if (Quoted == *q)
				Quoted = 0;
			
			// Write into ArgV
			else
			{
				// If no ArgV exists, create it
				if (!ArgV[j])
				{
					ArgV[j] = Z_Malloc(sizeof(char) * BUFSIZE, PU_STATIC, NULL);
					k = 0;
				}
				
				// Fill in buffer
				if (k < BUFSIZE)
					ArgV[j][k++] = *q;
			}
		}
		
		// Increment
		q++;
	}
	
	ArgC = j + (ArgV[j] != NULL ? 1 : 0);
	
	/* Find command */
	Found = CONEx_FindComVar(Parent, ArgV[0]);
	
	// Execute if found, there is no unterminated quote, arguments exist, and first argument exists
	if (Found && !Quoted && ArgV && ArgV[0])
	{
		// Command?
		if (!Found->IsVariable)
			Found->Data.Command.Func(Parent, &Found->Data.Command, ArgC, (const char* const* const)ArgV);
		
		// Variable?
		else
		{
		}
	}
	
	/* Cleanup */
	if (ArgV)
	{
		// Free members
		for (i = 0; i < ArgC; i++)
		{
			if (ArgV[i])
				Z_Free(ArgV[i]);
			ArgV[i] = 0;
		}
		
		// Free holder
		Z_Free(ArgV);
	}
	ArgV = NULL;

#undef BUFSIZE
}

/* CONEx_Init() -- Initialize extended console */
void CONEx_Init(void)
{
	/* Create root console */
	l_RootConsole = CONEx_CreateConsole();
	l_RootConsole->Command->WroteLineFunc = CONEx_CommandWriteLine;
	
	/* Make Pictures */
	if (!l_BackPic)
	{
		// Cache
		l_BackPic = W_CacheLumpName("RMD_CB_D", PU_STATIC);
		
		// Determine if it's pic_t or not...
		l_BackPicIsPicT = true;	// TODO
	}
}

/***************************
*** DEPRECATED FUNCTIONS ***
***************************/

/*******************************************************************************
********************************************************************************
*******************************************************************************/

bool_t con_started = false;	// console has been initialised
bool_t con_startup = false;	// true at game startup, screen need refreshing
bool_t con_forcepic = true;	// at startup toggle console transulcency when
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

int con_hudlines;		// number of console heads up message lines
int con_hudtime[5];				// remaining time of display for hud msg lines

int con_clearlines;				// top screen lines to refresh when view reduced
bool_t con_hudupdate;			// when messages scroll, we need a backgrnd refresh

// console text output
char *con_line;					// console text output current line
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
struct pic_s *con_backpic;				// console background picture, loaded static
struct pic_s *con_bordleft;
struct pic_s *con_bordright;			// console borders in translucent mode

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

void CON_Print(char *msg);

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

char *bindtable[NUMINPUTS];

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
uint8_t *whitemap;
uint8_t *greenmap;
uint8_t *graymap;
uint8_t* orangemap;

/* CON_SetupBackColormap() -- Deprecated handler for old colormaps */
void CON_SetupBackColormap(void)
{
	/* Call new function */
	// GhostlyDeath <November 5, 2010> -- Initialize colormaps
	V_InitializeColormaps();
	
	/* Wrap old arrays */
	// Allocate
	greenmap = (uint8_t *) Z_Malloc(256, PU_STATIC, NULL);
	whitemap = (uint8_t *) Z_Malloc(256, PU_STATIC, NULL);
	graymap = (uint8_t *) Z_Malloc(256, PU_STATIC, NULL);
	orangemap = (uint8_t *) Z_Malloc(256, PU_STATIC, NULL);
	
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
	
	// GhostlyDeath <November 2, 2010> -- Extended console
	CONEx_Init();

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
	con_backpic = (pic_t *) W_CacheLumpName("RMD_CB_D", PU_STATIC);

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
	
	COM_AddCommand("conextended", CONS_ConExtended_f);
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
void CON_Print(char *msg)
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
				con_line[con_cx++] = *(msg++)/* | mask*/;

		}
	}
}

//  Console print! Wahooo! Lots o fun!
//
void CONS_Printf(char *fmt, ...)
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
		I_OutputMsg("%s", txt);
//#endif
		if (!devparm)
			return;
	}

	// GhostlyDeath <November 2, 2010> -- CONS_Printf is global to extended console
	if (l_RootConsole)
		CONEx_BufferWrite(l_RootConsole->Output, txt);
	
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
#else*/
		// here we display the console background and console text
		// (no hardware accelerated support for these versions)
		
		// GhostlyDeath <November 4, 2010> -- If we aren't devparming, draw once
		if ((devparm && !g_QuietConsole) || ((!devparm || g_QuietConsole) && !AlreadyDrawn))
		{
			CONEx_Drawer();
			CONL_DrawConsole();
		
			//CON_Drawer();
			I_FinishUpdate();		// page flip or blit buffer
			
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
	if (cons_backpic.value == 1)		// clip only when using an opaque background
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
bool_t CON_Responder(event_t * ev)
{
//bool_t altdown;
	bool_t shiftdown;
	int i;

// sequential completions a la 4dos
	char completion[80];
	int comskips, varskips;

	char *cmd;
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
		key = ForeignTranslation((uint8_t) key);

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
void CONS_Error(char *msg)
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
	char *p;
	int x, y;

	// input line scrolls left if it gets too long
	//
	p = inputlines[inputline];
	if (input_cx >= con_width)
		p += input_cx - con_width + 1;

	y = con_curlines - 12;

	for (x = 0; x < con_width; x++)
		V_DrawCharacterA(VFONT_SMALL, VFO_NOSCALESTART | VFO_NOSCALEPATCH | VFO_NOSCALELORES,
			p[x], (x + 1) << 3, y);

	// draw the blinking cursor
	//
	x = (input_cx >= con_width) ? con_width - 1 : input_cx;
	if (con_tick < 4)
		V_DrawCharacterA(VFONT_SMALL, VFO_NOSCALESTART | VFO_NOSCALEPATCH | VFO_NOSCALELORES,
			'_', (x + 1) << 3, y);
}

// draw the last lines of console text to the top of the screen
//
void CON_DrawHudlines(void)
{
	char *p;
	int i, x, y, y2;

	if (con_hudlines <= 0)
		return;

	if (chat_on)
		y = 8;					// leave place for chat input in the first row of text
	else
		y = 0;
	y2 = 0;						//player 2's message y in splitscreen

	for (i = con_cy - con_hudlines + 1; i <= con_cy; i++)
	{
		if (i < 0)
			continue;
		if (con_hudtime[i % con_hudlines] == 0)
			continue;

		p = &con_buffer[(i % con_totallines) * con_width];

		for (x = 0; x < con_width; x++)
			V_DrawCharacterA(VFONT_SMALL, VFO_NOSCALESTART | VFO_NOSCALEPATCH | VFO_NOSCALELORES,
			(p[x] & 0xff), x << 3, y);

		if (con_lineowner[i % con_hudlines] == 2)
			y2 += 8;
		else
			y += 8;
	}

	// top screen lines that might need clearing when view is reduced
	con_clearlines = y;			// this is handled by HU_Erase ();
}

//  Scale a pic_t at 'startx' pos, to 'destwidth' columns.
//                startx,destwidth is resolution dependent
//  Used to draw console borders, console background.
//  The pic must be sized BASEVIDHEIGHT height.
//
//  TODO: ASM routine!!! lazy Fab!!
//
void CON_DrawBackpic(pic_t * pic, int startx, int destwidth)
{
	int x, y;
	int v;
	uint8_t *src, *dest;
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
	char *p;
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
			V_DrawCharacterMB(
					VFONT_SMALL,
					VFO_NOSCALEPATCH | VFO_NOSCALESTART | VFO_NOSCALELORES,
					&p[x],
					(lx + 1) << 3,	// require logical x
					y,
					&MBSkip
				);
			
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

