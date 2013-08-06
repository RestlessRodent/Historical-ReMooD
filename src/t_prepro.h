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
// Copyright(C) 2000 Simon Howard
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: FraggleScript files...
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

#ifndef __PREPRO_H__
#define __PREPRO_H__

typedef struct section_s section_t;

#if !defined(FREEBSD) && !defined(SOLARIS)
typedef struct label_s label_t;
#elif __FreeBSD__ > 4
typedef struct label_s label_t;
#endif
#define SECTIONSLOTS 17
#define LABELSLOTS 17

#include "t_parse.h"

void preprocess(script_t* script);

/***** {} sections **********/

section_t* find_section_start(char* brace);
section_t* find_section_end(char* brace);

struct section_s
{
	char* start;				// offset of starting brace {
	char* end;					// offset of ending brace   }
	int type;					// section type: for() loop, while() loop etc
	
	union
	{
		struct
		{
			char* loopstart;	// positioned before the while()
		} data_loop;
	} data;						// data for section
	
	section_t* next;			// for hashing
};

enum							// section types
{
	st_empty,					// none: empty {} braces
	st_if,						// if() statement
	st_elseif,					// elseif() statement
	st_else,					// else() statement
	st_loop,					// loop
};

/****** goto labels ***********/

label_t* labelforname(char* labelname);

#endif

//---------------------------------------------------------------------------
//
// $Log: t_prepro.h,v $
// Revision 1.4  2003/05/04 04:19:06  sburke
// Don't typedef label_t on Solaris.
//
// Revision 1.3  2003/01/19 21:24:26  bock
// Make sources buildable on FreeBSD 5-CURRENT.
//
// Revision 1.2  2001/05/16 22:33:34  bock
// Initial FreeBSD support.
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
// Revision 1.1.1.1  2000/04/30 19:12:09  fraggle
// initial import
//
//---------------------------------------------------------------------------
