// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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
// DESCRIPTION:
//      console for Doom LEGACY

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

#ifdef GAMECLIENT
#include "hu_stuff.h"
#include "v_video.h"
#include "st_stuff.h"
#include "r_defs.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

boolean con_started = false;	// console has been initialised
boolean con_startup = false;	// true at game startup, screen need refreshing
boolean con_forcepic = true;	// at startup toggle console transulcency when
							 // first off
boolean con_recalc;				// set true when screen size has changed

int con_tick;					// console ticker for anim or blinking prompt cursor
						 // con_scrollup should use time (currenttime - lasttime)..

boolean consoletoggle;			// true when console key pushed, ticker will handle
boolean consoleready;			// console prompt is ready

int con_destlines;				// vid lines used by console at final position
int con_curlines;				// vid lines currently used by console

int con_clipviewtop;			// clip value for planes & sprites, so that the
						 // part of the view covered by the console is not
						 // drawn when not needed, this must be -1 when
						 // console is off

int con_hudlines;		// number of console heads up message lines
int con_hudtime[5];				// remaining time of display for hud msg lines

int con_clearlines;				// top screen lines to refresh when view reduced
boolean con_hudupdate;			// when messages scroll, we need a backgrnd refresh

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

#ifdef GAMECLIENT
// GhostlyDeath <April 26, 2009> -- 
struct pic_s *con_backpic;				// console background picture, loaded static
struct pic_s *con_bordleft;
struct pic_s *con_bordright;			// console borders in translucent mode
#endif

// protos.
void CON_InputInit(void);
void CON_RecalcSize(void);

void CONS_speed_Change(void);
void CON_DrawBackpic(struct pic_s * pic, int startx, int destwidth);

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
#ifdef GAMECLIENT
	shiftxform = english_shiftxform;
	con_keymap = english;
	CONS_Printf("English keymap.\n");
#endif
}

//  Choose french keymap
//
void CONS_French_f(void)
{
#ifdef GAMECLIENT
	shiftxform = french_shiftxform;
	con_keymap = french;
	CONS_Printf("French keymap.\n");
#endif
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
		bindtable[key] = Z_StrDup(COM_Argv(2));
}

//======================================================================
//                          CONSOLE SETUP
//======================================================================

// Prepare a colormap for GREEN ONLY translucency over background
// GhostlyDeath - Red in ReMooD
#ifdef GAMECLIENT
byte *whitemap;
byte *greenmap;
byte *graymap;
byte* orangemap;
void CON_SetupBackColormap(void)
{
	int i, j, k, l, m;
	byte *pal;
	double hicolor;
	double locolor;

//
//  setup the green translucent background colormap
//
	greenmap = (byte *) Z_Malloc(256, PU_STATIC, NULL);
	whitemap = (byte *) Z_Malloc(256, PU_STATIC, NULL);
	graymap = (byte *) Z_Malloc(256, PU_STATIC, NULL);
	orangemap = (byte *) Z_Malloc(256, PU_STATIC, NULL);

	pal = W_CacheLumpName("PLAYPAL", PU_CACHE);

	for (i = 0, k = 0; i < 768; i += 3, k++)
	{
		j = pal[i] + pal[i + 1] + pal[i + 2];

		// 191 is darker
		// 183 is the lightest
		if (gamemode == heretic)
			greenmap[k] = 145 + (j >> 6);
		else
			greenmap[k] = /*127 */ 183 - (j >> 6);
	}
	
	// GhostlyDeath <July 9, 2008> -- TRUE COLOR CHANGING STUFF (FINALLY!!)
	for (i = 0, k = 0; i < 768; i += 3, k++)
	{
		hicolor = pal[i] + pal[i + 1] + pal[i + 2];
		hicolor /= 3.0;
		
		// Convert to Percent
		hicolor *= 1.0 / 255.0;
		locolor = hicolor + (hicolor * 2);
		if (locolor > 1.0)
			locolor = 1.0;
		if (locolor < 0.0)
			locolor = 0.0;

		// 255 * 0.32 = 81.6, 255 / 32 = 7.96875
		if (gamemode == heretic)
			l = 145 + (int)(12 * locolor);//37 + (int)(7 * locolor);
		else
		{
			l = 111 - (int)(32 * locolor);
			if (l <= 79) l = 4;
		}
		whitemap[k] = l;

		// 255 * 0.32 = 81.6, 255 / 32 = 7.96875
		if (gamemode == heretic)
			l = 241 + (int)(4 * locolor);
		else
		{
			l = 223 - (int)(24 * locolor);
			if (l <= 207) l = 4;
			else if (l >= 224 && l <= 226) l = 232;
			else if (l >= 227 && l <= 228) l = 233;
			else if (l >= 229 && l <= 230) l = 234;
			else if (l >= 231 && l <= 232) l = 235;
		}
		orangemap[k] = l;
		
		if (gamemode == heretic)
			graymap[k] = 0 + (int)(16 * locolor);
		else
			graymap[k] = 111 - (int)(16 * hicolor);
	}
}
#endif

//  Setup the console text buffer
//
void CON_Init(void)
{
	int i;

#ifdef GAMECLIENT
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
	if (gamemode == heretic)
		con_backpic = (pic_t *) W_CacheLumpName("CONSBCKH", PU_STATIC);
	else
		con_backpic = (pic_t *) W_CacheLumpName("CONSBACK", PU_STATIC);

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
#endif
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
			if (*msg == '\2')	// set white color
				mask = 128;
			else if (*msg == '\3')
			{
				mask = 128;		// white text + sound
				if (gamemode == commercial)
					S_StartSound(0, sfx_radio);
				else
					S_StartSound(0, sfx_tink);
			}
			else if (*msg == '\4')	//Splitscreen: This message is for the second player
				second_player_message = 1;

		}

		while (*msg)
		{
			// skip non-printable characters and white spaces
			while (*msg && *msg <= ' ')
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
			for (l = 0; l < con_width && msg[l] > ' '; l++)
				;

			// word wrap
			if (con_cx + l > con_width)
				CON_Linefeed(second_player_message);

			// a word at a time
			for (; l > 0; l--)
				con_line[con_cx++] = *(msg++) | mask;

		}
	}
}

//  Console print! Wahooo! Lots o fun!
//
void CONS_Printf(char *fmt, ...)
{
	va_list argptr;
	char txt[512];

	va_start(argptr, fmt);
#if _MSC_VER >= 1400
	vsprintf_s(txt, 512, fmt, argptr);
#elif defined(__GNUC__)
	vsnprintf(txt, 512, fmt, argptr);
#else
	vsprintf(txt, fmt, argptr);
#endif
	va_end(argptr);

	// echo console prints to log file
#ifdef LOGMESSAGES
#ifndef _WIN32
	if (logstream != INVALID_HANDLE_VALUE)
		write(logstream, txt, strlen(txt));
#endif
#endif
	DEBFILE(txt);

#ifndef GAMESERVER
	if (devparm || !con_started /* || !graphics_started */ )
	{
#endif
//#if !defined( _WIN32) && !defined( __OS2__)
		I_OutputMsg("%s", txt);
//#endif
#ifndef GAMESERVER
		if (!devparm)
			return;
	}
#else
	return;
#endif

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
		CON_Drawer();
		I_FinishUpdate();		// page flip or blit buffer
//#endif
	}
}




