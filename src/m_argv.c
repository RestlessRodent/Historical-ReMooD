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
// Copyright (C) 1993-1996 by id Software, Inc.
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

#include <string.h>

#include "doomdef.h"
#include "command.h"

int myargc;
char **myargv;
static int found;

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
int M_CheckParm(char *check)
{
	int i;

	for (i = 1; i < myargc; i++)
	{
		if (!strcasecmp(check, myargv[i]))
		{
			found = i;
			return i;
		}
	}
	found = 0;
	return 0;
}

// return true if there is available parameters
bool_t M_IsNextParm(void)
{
	if (found > 0 && found + 1 < myargc && myargv[found + 1][0] != '-' &&
		myargv[found + 1][0] != '+')
		return true;
	return false;
}

// return the next parameter after a M_CheckParm
// NULL if not found use M_IsNext to find if there is a parameter
char *M_GetNextParm(void)
{
	if (M_IsNextParm())
	{
		found++;
		return myargv[found];
	}
	return NULL;
}

/* M_PushSpecialParametersAsOne() -- To prevent code repeation between M_PushSpecialParameters and M_PushSpecialPlusParameters */
void M_PushSpecialParametersAsOne(const bool_t plusplus)
{
#define BUFSIZE 256
	int i;
	char s[BUFSIZE];
	bool_t onetime = false;

	for (i = 1; i < myargc; i++)
	{
		if ((!plusplus && (myargv[i][0] == '+' && myargv[i][1] != '+')) ||
			(plusplus && (myargv[i][0] == '+' && myargv[i][1] == '+')))
		{
			
			if (plusplus)
				strncpy(s, &myargv[i][2], BUFSIZE);
			else
				strncpy(s, &myargv[i][1], BUFSIZE);
			i++;

			// get the parameter of the command too
			for (; i < myargc && myargv[i][0] != '+' && myargv[i][0] != '-'; i++)
			{
				strncat(s, " ", BUFSIZE);
				if (!onetime)
				{
					strncat(s, "\"", BUFSIZE);
					onetime = true;
				}
				strncat(s, myargv[i], BUFSIZE);
			}
			if (onetime)
			{
				strncat(s, "\"", BUFSIZE);
				onetime = false;
			}
			strncat(s, "\n", BUFSIZE);

			// push it
#if 0
			if (devparm)
			{
				if (plusplus)
					CONS_PrintfUL(SRCSTR__M_ARGV_C__EXECPP, L"%s", s);
				else
					CONS_PrintfUL(SRCSTR__M_ARGV_C__EXECP, L"%s", s);
			}
#endif
			COM_BufAddText(s);
			i--;
		}
	}
#undef BUFSIZE
}

/* M_PushSpecialParameters() -- Push all + parameters */
void M_PushSpecialParameters(void)
{
	M_PushSpecialParametersAsOne(false);
}

/* M_PushSpecialPlusParameters() -- Push all ++ parameters */
void M_PushSpecialPlusParameters(void)
{
	M_PushSpecialParametersAsOne(true);
}

//
// Find a Response File
//
void M_FindResponseFile(void)
{
	int i;
#define MAXARGVS        256

	for (i = 1; i < myargc; i++)
		if (myargv[i][0] == '@')
		{
			FILE *handle;
			int size;
			int k;
			int index;
			int indexinfile;
			bool_t inquote = false;
			uint8_t *infile;
			char *file;
			char *moreargs[20];
			char *firstargv;

			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen(&myargv[i][1], "rb");
			if (!handle)
			{
				I_Error("\nResponse file %s not found !", &myargv[i][1]);
				exit(1);
			}
			CONS_Printf("Found response file %s!\n", &myargv[i][1]);
			fseek(handle, 0, SEEK_END);
			size = ftell(handle);
			fseek(handle, 0, SEEK_SET);
			file = malloc(size);
			fread(file, size, 1, handle);
			fclose(handle);

			// KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
			for (index = 0, k = i + 1; k < myargc; k++)
				moreargs[index++] = myargv[k];

			firstargv = myargv[0];
			myargv = malloc(sizeof(char *) * MAXARGVS);
			if (!myargv)
				I_Error("no enought memory");
			memset(myargv, 0, sizeof(char *) * MAXARGVS);
			myargv[0] = firstargv;

			infile = file;
			indexinfile = k = 0;
			indexinfile++;		// SKIP PAST ARGV[0] (KEEP IT)
			do
			{
				inquote = infile[k] == '"';
				if (inquote)	// strip encllosing double-quote
					k++;
				myargv[indexinfile++] = &infile[k];
				while (k < size && ((inquote && infile[k] != '"') || (!inquote && infile[k] > ' ')))
					k++;
				infile[k] = 0;
				while (k < size && (infile[k] <= ' '))
					k++;
			}
			while (k < size);

			for (k = 0; k < index; k++)
				myargv[indexinfile++] = moreargs[k];
			myargc = indexinfile;

			// DISPLAY ARGS
			CONS_Printf("%d command-line args:\n", myargc);
			for (k = 1; k < myargc; k++)
				CONS_Printf("%s\n", myargv[k]);

			break;
		}
}
