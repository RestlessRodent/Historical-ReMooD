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
// Copyright(C) 2000 Simon Howard
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
// #############################################################################
// ##  THIS SOURCE FILE HAS BEEN DEPRECATED AND WILL BE REMOVED IN THE FUTURE ##
// #############################################################################
// # There should be no futher changes unless necessary. Future dependencies   #
// # on this code will be changed, replaced and/or removed.                    #
// #############################################################################
// # NOTE: Deprecated and will be replaced by the new ReMooD Virtual Machine   #
// #       Which will be the heart of ReMooD Script. Of course there is a      #
// #       Legacy compatibility layer that will be maintained so all those     #
// #       awesome Legacy mods can be played.                                  #
// #############################################################################
//
// Preprocessor.
//
// The preprocessor must be called when the script is first loaded.
// It performs 2 functions:
//      1: blank out comments (which could be misinterpreted)
//      2: makes a list of all the sections held within {} braces
//      3: 'dry' runs the script: goes thru each statement and
//         sets the types of all the section_t's in the script
//      4: Saves locations of all goto() labels
//
// the system of section_t's is pretty horrible really, but it works
// and its probably the only way i can think of of saving scripts
// half-way thru running

/* includes ************************/

#include "command.h"
#include "doomstat.h"
#include "w_wad.h"
#include "z_zone.h"

#include "t_parse.h"
#include "t_spec.h"
#include "t_oper.h"
#include "t_vari.h"
#include "t_func.h"
#include "w_wad.h"

// clear the script: section and variable slots

void clear_script()
{
	int i;
	
	for (i = 0; i < SECTIONSLOTS; i++)
		current_script->sections[i] = NULL;
		
	for (i = 0; i < VARIABLESLOTS; i++)
		current_script->variables[i] = NULL;
		
	// clear child scripts
	
	for (i = 0; i < MAXSCRIPTS; i++)
		current_script->children[i] = NULL;
}

/*********** {} sections *************/

// during preprocessing all of the {} sections
// are found. these are stored in a hash table
// according to their offset in the script.
// functions here deal with creating new section_t's
// and finding them from a given offset.

#define section_hash(b)           \
       ( (int) ( (b) - current_script->data) % SECTIONSLOTS)

section_t* new_section(char* brace)
{
	int n;
	section_t* newsec;
	
	// create section
	// make level so its cleared at start of new level
	
	newsec = Z_Malloc(sizeof(section_t), PU_LEVEL, 0);
	newsec->start = brace;
	
	// hook it into the hashchain
	
	n = section_hash(brace);
	newsec->next = current_script->sections[n];
	current_script->sections[n] = newsec;
	
	return newsec;
}

// find a section_t from the location of the starting { brace
section_t* find_section_start(char* brace)
{
	int n = section_hash(brace);
	section_t* current;
	
	current = current_script->sections[n];
	
	// use the hash table: check the appropriate hash chain
	
	while (current)
	{
		if (current->start == brace)
			return current;
		current = current->next;
	}
	
	return NULL;				// not found
}

// find a section_t from the location of the ending } brace
section_t* find_section_end(char* brace)
{
	int n;
	
	// hash table is no use, they are hashed according to
	// the offset of the starting brace
	
	// we have to go through every entry to find from the
	// ending brace
	
	for (n = 0; n < SECTIONSLOTS; n++)	// check all sections in all chains
	{
		section_t* current = current_script->sections[n];
		
		while (current)
		{
			if (current->end == brace)
				return current;	// found it
			current = current->next;
		}
	}
	
	return NULL;				// not found
}

/********** labels ****************/

// labels are also found during the
// preprocessing. these are of the form
//
//      label_name:
//
// and are used for the goto function.
// goto labels are stored as variables.

// from parse.c
#define isop(c)   !( ( (c)<='Z' && (c)>='A') || ( (c)<='z' && (c)>='a') || \
                     ( (c)<='9' && (c)>='0') || ( (c)=='_') )

// create a new label. pass the location inside the script
svariable_t* new_label(char* labelptr)
{
	svariable_t* newlabel;		// labels are stored as variables
	char labelname[256];
	char* temp, *temp2;
	
	// copy the label name from the script up to ':'
	for (temp = labelptr, temp2 = labelname; *temp != ':'; temp++, temp2++)
		*temp2 = *temp;
	*temp2 = '\0';				// end string
	
	newlabel = new_variable(current_script, labelname, svt_label);
	
	// put neccesary data in the label
	
	newlabel->value.labelptr = labelptr;
	
	return newlabel;
}

/*********** main loop **************/

// This works by recursion. when a { opening
// brace is found, another instance of the
// function is called for the data inside
// the {} section.
// At the same time, the sections are noted
// down and hashed. Goto() labels are noted
// down, and comments are blanked out

char* process_find_char(char* data, char find)
{
	while (*data)
	{
		if (*data == find)
			return data;
		if (*data == '\"')		// found a quote: ignore stuff in it
		{
			data++;
			while (*data && *data != '\"')
			{
				// escape sequence ?
				if (*data == '\\')
					data++;
				data++;
			}
			// error: end of script in a constant
			if (!*data)
				return NULL;
		}
		// comments: blank out
		
		if (*data == '/' && *(data + 1) == '*')	// /* -- */ comment
		{
			while (*data && (*data != '*' || *(data + 1) != '/'))
			{
				*data = ' ';
				data++;
			}
			if (*data)
				*data = *(data + 1) = ' ';	// blank the last bit
			else
			{
				rover = data;
				// script terminated in comment
				script_error("script terminated inside comment\n");
			}
		}
		if (*data == '/' && *(data + 1) == '/')	// // -- comment
			while (*data != '\n')
			{
				*data = ' ';
				data++;			// blank out
			}
		// labels
		
		if (*data == ':'		// ':' -- a label
		        && current_script->scriptnum != -1)	// not levelscript
		{
			char* labelptr = data - 1;
			
			while (!isop(*labelptr))
				labelptr--;
			new_label(labelptr + 1);
		}
		
		if (*data == '{')		// { -- } sections: add 'em
		{
			section_t* newsec = new_section(data);
			
			newsec->type = st_empty;
			// find the ending } and save
			newsec->end = process_find_char(data + 1, '}');
			if (!newsec->end)
			{
				// brace not found
				rover = data;
				script_error("section error: no ending brace\n");
				return NULL;
			}
			// continue from the end of the section
			data = newsec->end;
		}
		data++;
	}
	
	return NULL;
}

/*********** second stage parsing ************/

// second stage preprocessing considers the script
// in terms of tokens rather than as plain data.
//
// we 'dry' run the script: go thru each statement and
// collect types for section_t
//
// this is an important thing to do, it cannot be done
// at runtime for 2 reasons:
//      1. gotos() jumping inside loops will pass thru
//         the end of the loop
//      2. savegames. loading a script saved inside a
//         loop will let it pass thru the loop
//
// this is basically a cut-down version of the normal
// parsing loop.

void get_tokens(char*);		// t_parse.c

void dry_run_script()
{
	// save some stuff
	char* old_rover = rover;
	section_t* old_current_section = current_section;
	
	char* end = current_script->data + current_script->len;
	char* token_alloc;
	
	killscript = false;
	
	// allocate space for the tokens
	token_alloc = Z_Malloc(current_script->len + T_MAXTOKENS, PU_STATIC, 0);
	
	rover = current_script->data;
	
	while (rover < end && *rover)
	{
		tokens[0] = token_alloc;
		get_tokens(rover);
		
		if (killscript)
			break;
		if (!num_tokens)
			continue;
			
		if (current_section && tokentype[0] == function)
		{
			if (!strcmp(tokens[0], "if"))
			{
				current_section->type = st_if;
				continue;
			}
			else if (!strcmp(tokens[0], "elseif"))
			{
				current_section->type = st_elseif;
				continue;
			}
			else if (!strcmp(tokens[0], "else"))
			{
				current_section->type = st_else;
				continue;
			}
			else if (!strcmp(tokens[0], "while") || !strcmp(tokens[0], "for"))
			{
				current_section->type = st_loop;
				current_section->data.data_loop.loopstart = linestart;
				continue;
			}
		}
	}
	
	Z_Free(token_alloc);
	
	// restore stuff
	current_section = old_current_section;
	rover = old_rover;
}

/***************** main preprocess function ******************/

// set up current_script, script->len
// just call all the other functions

void preprocess(script_t* script)
{
	current_script = script;
	script->len = strlen(script->data);
	
	clear_script();
	
	process_find_char(script->data, 0);	// fill in everything
	
	dry_run_script();
}

/************ includes ******************/

// FraggleScript allows 'including' of other lumps.
// we divert input from the current_script (normally
// levelscript) to a seperate lump. This of course
// first needs to be preprocessed to remove comments
// etc.

void parse_data(char* data, char* end);	// t_parse.c

// parse an 'include' lump

void parse_include(char* lumpname)
{
#if 0
	int lumpnum;
	char* temp;
	char* lump, *end;
	char* saved_rover;
	char* x;
	
#if 0
	if (-1 == (lumpnum = W_CheckNumForName(lumpname)))
	{
		x = lumpname;
		while (*x)
		{
			if (*x == '.')
				*x = '_';
			x++;
		}
		
		if (-1 == (lumpnum = W_CheckNumForName(lumpname)))
		{
			script_error("include lump '%s' not found!\n", lumpname);
			return;
		}
	}
	
	lump = W_CacheLumpNum(lumpnum, PU_STATIC);
#endif
	script_error("include lump '%s' not found!\n", lumpname);
	return;
	
	// realloc bigger for NULL at end
	// SoM: REALLOC does not seem to work! So we alloc here and copy....
	//lump = Z_Realloc(lump, W_LumpLength(lumpnum)+10, PU_STATIC, NULL);
	temp = lump;
	lump = Z_Malloc(W_LumpLength(lumpnum) + 10, PU_STATIC, NULL);
	memcpy(lump, temp, W_LumpLength(lumpnum));
	
	saved_rover = rover;		// save rover during include
	rover = lump;
	end = lump + W_LumpLength(lumpnum);
	*end = 0;
	
	// preprocess the include
	// we assume that it does not include sections or labels or
	// other nasty things
	process_find_char(lump, 0);
	
	// now parse the lump
	parse_data(lump, end);
	
	// restore rover
	rover = saved_rover;
	
	// free the lump
	Z_Free(lump);
	Z_Free(temp);
#endif
}

//---------------------------------------------------------------------------
//
// $Log: t_prepro.c,v $
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
// Revision 1.1.1.1  2000/04/30 19:12:08  fraggle
// initial import
//
//---------------------------------------------------------------------------
