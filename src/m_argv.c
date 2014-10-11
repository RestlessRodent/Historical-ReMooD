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
#include "console.h"

int myargc;
char** myargv;
static int found;

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
int M_CheckParm(char* check)
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
	if (found > 0 && found + 1 < myargc && myargv[found + 1][0] != '-' && myargv[found + 1][0] != '+')
		return true;
	return false;
}

// return the next parameter after a M_CheckParm
// NULL if not found use M_IsNext to find if there is a parameter
char* M_GetNextParm(void)
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
		if ((!plusplus && (myargv[i][0] == '+' && myargv[i][1] != '+')) || (plusplus && (myargv[i][0] == '+' && myargv[i][1] == '+')))
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
				strncat(s, "\"", BUFSIZE);
				strncat(s, myargv[i], BUFSIZE);
				strncat(s, "\"", BUFSIZE);
			}
			
			strncat(s, "\n", BUFSIZE);
			
			// push it
#if 0
			if (devparm)
			{
				if (plusplus)
					CONL_PrintFUL(SRCSTR__M_ARGV_C__EXECPP, L"%s", s);
				else
					CONL_PrintFUL(SRCSTR__M_ARGV_C__EXECP, L"%s", s);
			}
#endif
			
			// GhostlyDeath <May 9, 2012> -- Execute on new console instead
			CONL_InputF("%s", s);
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
			FILE* handle;
			int size;
			int k;
			int index;
			int indexinfile;
			bool_t inquote = false;
			uint8_t* infile;
			char* file;
			char* moreargs[20];
			char* firstargv;
			
			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen(&myargv[i][1], "rb");
			if (!handle)
			{
				I_Error("\nResponse file %s not found !", &myargv[i][1]);
				exit(1);
			}
			CONL_PrintF("Found response file %s!\n", &myargv[i][1]);
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
			myargv = malloc(sizeof(char*) * MAXARGVS);
			if (!myargv)
				I_Error("no enought memory");
			memset(myargv, 0, sizeof(char*) * MAXARGVS);
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
			CONL_PrintF("%d command-line args:\n", myargc);
			for (k = 1; k < myargc; k++)
				CONL_PrintF("%s\n", myargv[k]);
				
			break;
		}
}
