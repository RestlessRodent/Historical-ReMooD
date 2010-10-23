// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Console

#include "d_event.h"
#include "command.h"
#include "g_input.h"

// for debugging shopuld be replaced by nothing later.. so debug is inactive
#define LOG(x) CONS_Printf(x)

void CON_Init(void);

boolean CON_Responder(event_t * ev);

#ifdef __MACOS__
#define  CON_BUFFERSIZE   4096	//my compiler cant handle local vars >32k
#else
#define  CON_BUFFERSIZE   16384
#endif

// TODO: choose max hud msg lines
#define  CON_MAXHUDLINES      5

// hold 32 last lines of input for history
#define  CON_MAXPROMPTCHARS    256
#define  CON_PROMPTCHAR        '>'

#ifdef GAMECLIENT
extern boolean con_started;
extern boolean con_startup;
extern boolean con_forcepic;
extern boolean con_recalc;
extern int con_tick;
extern boolean consoletoggle;
extern boolean consoleready;
extern int con_destlines;
extern int con_curlines;
extern int con_clipviewtop;
extern int con_hudlines;
extern int con_hudtime[5];
extern int con_clearlines;
extern boolean con_hudupdate;
extern char *con_line;
extern int con_cx;
extern int con_cy;
extern int con_totallines;
extern int con_width;
extern int con_scrollup;
extern int con_lineowner[CON_MAXHUDLINES];
extern char inputlines[32][CON_MAXPROMPTCHARS];
extern int inputline;
extern int inputhist;
extern int input_cx;
extern struct pic_s *con_backpic;
extern struct pic_s *con_bordleft;
extern struct pic_s *con_bordright;
extern char con_buffer[CON_BUFFERSIZE];
extern char *bindtable[NUMINPUTS];
// set true when screen size has changed, to adapt console
extern boolean con_recalc;
extern boolean con_startup;

extern boolean consoleready;	// GhostlyDeath -- extern this here
// top clip value for view render: do not draw part of view hidden by console
extern int con_clipviewtop;

// 0 means console is off, or moving out
extern int con_destlines;

extern int con_clearlines;		// lines of top of screen to refresh
extern boolean con_hudupdate;	// hud messages have changed, need refresh
extern int con_keymap;			//0 english, 1 french

extern byte* redmap;
extern byte *whitemap;
extern byte *greenmap;
extern byte *graymap;
extern byte* orangemap;

extern consvar_t cons_msgtimeout;
extern consvar_t cons_speed;
extern consvar_t cons_height;
extern consvar_t cons_backpic;

#endif

void CON_ClearHUD(void);		// clear heads up messages

void CON_Ticker(void);
void CON_Drawer(void);
void CONS_Error(char *msg);		// print out error msg, and wait a key

// force console to move out
void CON_ToggleOff(void);

