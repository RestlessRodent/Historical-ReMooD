// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// DESCRIPTION: DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//              plus functions to determine game mode (shareware, registered),
//              parse command line parameters, configure game parameters (turbo),
//              and call the startup functions.

#ifdef LINUX
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef _WIN32
#include <unistd.h>				// for access
#else
#include <direct.h>
#endif
#include <fcntl.h>

#ifdef __OS2__
#include "I_os2.h"
#endif

#include "doomdef.h"

#include "command.h"
#include "console.h"

#include "doomstat.h"

#include "am_map.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "dehacked.h"
#include "dstrings.h"

#include "f_wipe.h"
#include "f_finale.h"

#include "g_game.h"
#include "g_input.h"

#include "hu_stuff.h"

#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"

#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"

#include "p_setup.h"
#include "p_fab.h"
#include "p_info.h"

#include "r_main.h"
#include "r_local.h"

#include "s_sound.h"
#include "st_stuff.h"

#include "t_script.h"

#include "v_video.h"

#include "wi_stuff.h"
#include "w_wad.h"

#include "z_zone.h"
#include "d_main.h"
#include "m_cheat.h"
#include "p_chex.h"

#include "d_prof.h"

//
//  DEMO LOOP
//
int demosequence;
int pagetic;
char *pagename = "TITLEPIC";
boolean novideo = false;

//  PROTOS
void D_PageDrawer(char *lumpname);
void D_AdvanceDemo(void);

#ifdef LINUX
void VID_PrepareModeList(void);	// FIXME: very dirty; will use a proper include file
#endif

char *startupwadfiles[MAX_WADFILES];

boolean devparm;				// started game with -devparm
boolean nomonsters;				// checkparm of -nomonsters
boolean infight = false;		//DarkWolf95:November 21, 2003: Monsters Infight!

boolean singletics = false;		// timedemo

boolean nomusic;
boolean nosound;
boolean digmusic;				// OGG/MP3 Music SSNTails 12-13-2002
boolean newnet_use = false;
boolean newnet_solo = false;

boolean advancedemo;

char wadfile[1024];				// primary wad file
char mapdir[1024];				// directory of development maps

//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
// referenced from i_system.c for I_GetKey()

event_t events[MAXEVENTS];
int eventhead = 0;
int eventtail;

boolean dedicated;

//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent(const event_t * ev)
{
	events[eventhead] = *ev;
	eventhead = (eventhead + 1) & (MAXEVENTS - 1);
}

// just for lock this function
#ifdef PC_DOS
void D_PostEvent_end(void)
{
};
#endif

boolean shiftdown = false;

//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents(void)
{
	event_t *ev;

	for (; eventtail != eventhead; eventtail = (++eventtail) & (MAXEVENTS - 1))
	{
		ev = &events[eventtail];
		
		if (ev->type == ev_keydown && ev->data1 == KEY_SHIFT)
			shiftdown = true;
		else if (ev->type == ev_keyup && ev->data1 == KEY_SHIFT)
			shiftdown = false;
		
		// GhostlyDeath <November 2, 2010> -- Only respond to console if menu is not active
		if (!menuactive)
			// GhostlyDeath <November 2, 2010> -- Extended console
			if (CONEx_Responder(ev))
				continue;
		
		// Menu input
		if (M_Responder(ev))
			continue;			// menu ate the event
		
		// console input
		if (CON_Responder(ev))
			continue;			// ate the event

		G_Responder(ev);
	}
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

/*#ifdef _WIN32
void I_DoStartupMouse(void);    //win_sys.c
#endif*/

// wipegamestate can be set to -1 to force a wipe on the next draw
// added comment : there is a wipe eatch change of the gamestate
gamestate_t wipegamestate = GS_DEMOSCREEN;
CV_PossibleValue_t screenslink_cons_t[] = { {0, "None"}
, {wipe_ColorXForm + 1, "Color"}
, {wipe_Melt + 1, "Melt"}
, {0, NULL}
};
consvar_t cv_screenslink = { "screenlink", "2", CV_SAVE, screenslink_cons_t };

void D_Display(void)
{
	static boolean menuactivestate = false;
	static gamestate_t oldgamestate = -1;
	static int borderdrawcount;
	tic_t nowtime;
	tic_t tics;
	tic_t wipestart;
	int i;
	int y;
	int a,b;
	int oldviewwidth;
	boolean done;
	boolean wipe;
	boolean redrawsbar;
	boolean viewactivestate = false;

	if (dedicated)
		return;

	if (nodrawers)
		return;					// for comparative timing / profiling

	redrawsbar = false;

	//added:21-01-98: check for change of screen size (video mode)
	if (setmodeneeded)
		SCR_SetMode();			// change video mode

	if (vid.recalc)
		//added:26-01-98: NOTE! setsizeneeded is set by SCR_Recalc()
		SCR_Recalc();

	// change the view size if needed
	if (setsizeneeded)
	{
		R_ExecuteSetViewSize();
		oldgamestate = -1;		// force background redraw
		borderdrawcount = 3;
		redrawsbar = true;
	}

	// save the current screen if about to wipe
	if (gamestate != wipegamestate)
	{
		wipe = true;
		wipe_StartScreen(0, 0, vid.width, vid.height);
	}
	else
		wipe = false;

	// draw buffered stuff to screen
	// BP: Used only by linux GGI version
	I_UpdateNoBlit();

	// do buffered drawing
	switch (gamestate)
	{
		case GS_LEVEL:
			if (!gametic)
				break;
			HU_Erase();
			if (automapactive && !automapoverlay)
				AM_Drawer();
			if (wipe || menuactivestate
				|| vid.recalc)
				redrawsbar = true;
			break;

		case GS_INTERMISSION:
			WI_Drawer();
			break;

		case GS_FINALE:
			F_Drawer();
			break;

		case GS_DEMOSCREEN:
			D_PageDrawer(pagename);
		case GS_NULL:
			break;
	}

	// clean up border stuff
	// see if the border needs to be initially drawn
	if (gamestate == GS_LEVEL)
	{
		if (oldgamestate != GS_LEVEL)
		{
			viewactivestate = false;	// view was not active
			R_FillBackScreen();	// draw the pattern into the back screen
		}

		// see if the border needs to be updated to the screen
		if ((!automapactive || automapoverlay) && (scaledviewwidth != vid.width))
		{
			// the menu may draw over parts out of the view window,
			// which are refreshed only when needed
			if (menuactive || menuactivestate || !viewactivestate)
				borderdrawcount = 3;

			if (borderdrawcount)
			{
				R_DrawViewBorder();	// erase old menu stuff
				borderdrawcount--;
			}
		}

		// draw the view directly
		if (!automapactive || automapoverlay)
		{
			// added 16-6-98: render the second screen
			switch (cv_splitscreen.value)
			{
				case 1:
					if (playeringame[displayplayer[1]] && players[displayplayer[1]].mo)
					{
						//faB: Boris hack :P !!
						viewwindowy = vid.height / 2;
						activeylookup = ylookup;
						memcpy(ylookup, ylookup2, viewheight * sizeof(ylookup[0]));

						R_RenderPlayerView(&players[displayplayer[1]]);

						viewwindowy = 0;
						activeylookup = ylookup;
						memcpy(ylookup, ylookup1, viewheight * sizeof(ylookup[0]));
					}
					else
						V_DrawScreenFill(0, vid.height >> 1, vid.width, vid.height >> 1, 0);
				case 0:
					if (players[displayplayer[0]].mo)
					{
						activeylookup = ylookup;
						R_RenderPlayerView(&players[displayplayer[0]]);
					}
					break;
				case 2:
				case 3:
				default:
					for (i = 0; i < 4; i++)
					{
						if (playeringame[displayplayer[i]] && players[displayplayer[i]].mo &&
							i < cv_splitscreen.value+1)
						{
							activeylookup = ylookup4[i];
							
							if (cv_splitscreen.value % 2 == 1)
								viewwindowx = vid.width / 2;
							if (cv_splitscreen.value > 1)
								viewwindowy = vid.height / 2;

							R_RenderPlayerView(&players[displayplayer[i]]);

							viewwindowx = 0;
							viewwindowy = 0;
						}
						else
							V_DrawScreenFill(
								((i == 1 || i == 3) ? vid.width >> 1 : 0),
								((i == 2 || i == 3) ? vid.height >> 1 : 0),
								vid.width >> 1, vid.height >> 1, 0);
					}
					break;
			}
		}
		
		if (automapactive && automapoverlay)
			AM_Drawer();

		HU_Drawer();

		ST_Drawer(redrawsbar);
	}

	// change gamma if needed
	if (gamestate != oldgamestate && gamestate != GS_LEVEL)
		V_SetPalette(0);

	menuactivestate = menuactive;
	oldgamestate = wipegamestate = gamestate;

	// draw pause pic
	if (paused && (!menuactive || netgame) && (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		patch_t *patch;
		if (automapactive)
			y = 4;
		else
			y = viewwindowy + 4;
		patch = W_CachePatchName("M_PAUSE", PU_CACHE);
		V_DrawScaledPatch(viewwindowx + (BASEVIDWIDTH - LittleSwapInt16(patch->width)) / 2, y, 0, patch);
	}

	//added:24-01-98:vid size change is now finished if it was on...
	vid.recalc = 0;
	
	//CON_Drawer();
	
	// GhostlyDeath <November 2, 2010> -- Draw either the console or the menu
	if (menuactive)
		M_Drawer();
	else
		CONEx_Drawer();
	
	NetUpdate();				// send out any new accumulation

//
// normal update
//
	if (!wipe)
	{
		{
			//I_BeginProfile();
			I_FinishUpdate();	// page flip or blit buffer
			//CONS_Printf ("last frame update took %d\n", I_EndProfile());
		}
		return;
	}

//
// wipe update
//
	if (!cv_screenslink.value)
		return;

	wipe_EndScreen(0, 0, vid.width, vid.height);

	wipestart = I_GetTime() - 1;
	y = wipestart + 2 * TICRATE;	// init a timeout
	do
	{
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
		}
		while (!tics);
		wipestart = nowtime;
		done = wipe_ScreenWipe(cv_screenslink.value - 1, 0, 0, vid.width, vid.height, tics);
		I_OsPolling();
		I_UpdateNoBlit();
		M_Drawer();				// menu is drawn even on top of wipes
		I_FinishUpdate();		// page flip or blit buffer
	}
	while (!done && I_GetTime() < (unsigned)y);

	ST_Invalidate();
}

// =========================================================================
//   D_DoomLoop
// =========================================================================

tic_t rendergametic, oldentertics;
boolean supdate;

//#define SAVECPU_EXPERIMENTAL

void D_DoomLoop(void)
{
	tic_t oldentertics, entertic, realtics, rendertimeout = -1;

	if (demorecording)
		G_BeginRecording();

	// user settings
	COM_BufAddText("exec autoexec.cfg\n");

	// end of loading screen: CONS_Printf() will no more call FinishUpdate()
	con_startup = false;

	CONS_Printf("I_StartupKeyboard...\n");
	I_StartupKeyboard();

/*#ifdef _WIN32
    CONS_Printf("I_StartupMouse...\n");
    I_DoStartupMouse();
#endif*/

	oldentertics = I_GetTime();

	// make sure to do a d_display to init mode _before_ load a level
	SCR_SetMode();				// change video mode
	SCR_Recalc();

	for (;;)
	{
		// get real tics
		entertic = I_GetTime();
		realtics = entertic - oldentertics;
		oldentertics = entertic;

#ifdef SAVECPU_EXPERIMENTAL
		if (realtics == 0)
		{
			usleep(10000);
			continue;
		}
#endif

		// frame syncronous IO operations
		// UNUSED for the moment (18/12/98)
		I_StartFrame();

		// process tics (but maybe not if realtic==0)
		TryRunTics(realtics);
		if (singletics || gametic > rendergametic)
		{
			rendergametic = gametic;
			rendertimeout = entertic + TICRATE / 17;

			//added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
			S_UpdateSounds();	// move positional sounds
			// Update display, next frame, with current state.
			D_Display();
			supdate = false;
		}
		else if (rendertimeout < entertic)	// in case the server hang or netsplit
			D_Display();

		// Win32 exe uses DirectSound..
#if !defined( _WIN32) && !defined( __OS2__)
		//
		//Other implementations might need to update the sound here.
		//
#ifndef SNDSERV
		// Sound mixing for the buffer is snychronous.
		I_UpdateSound();
#endif
		// Synchronous sound output is explicitly called.
#ifndef SNDINTR
		// Update sound output.
		I_SubmitSound();
#endif

#endif							//_WIN32
		// check for media change, loop music..
		I_UpdateCD();
	}
}

// =========================================================================
//   D_AdvanceDemo
// =========================================================================

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
	if (--pagetic < 0)
		D_AdvanceDemo();
}

//
// D_PageDrawer : draw a patch supposed to fill the screen,
//                fill the borders with a background pattern (a flat)
//                if the patch doesn't fit all the screen.
//
void D_PageDrawer(char *lumpname)
{
	byte *src;
	byte *dest;
	int x;
	int y;

	// software mode which uses generally lower resolutions doesn't look
	// good when the pic is scaled, so it fills space aorund with a pattern,
	// and the pic is only scaled to integer multiples (x2, x3...)
	if ((vid.width > BASEVIDWIDTH) || (vid.height > BASEVIDHEIGHT))
	{
		src = scr_borderpatch;
		dest = screens[0];

		for (y = 0; y < vid.height; y++)
		{
			for (x = 0; x < vid.width / 64; x++)
			{
				memcpy(dest, src + ((y & 63) << 6), 64);
				dest += 64;
			}
			if (vid.width & 63)
			{
				memcpy(dest, src + ((y & 63) << 6), vid.width & 63);
				dest += (vid.width & 63);
			}
		}
	}
	
	V_DrawScaledPatch(0, 0, 0, W_CachePatchName(lumpname, PU_CACHE));

	//added:08-01-98:if you wanna centre the pages it's here.
	//          I think it's not so beautiful to have the pic centered,
	//          so I leave it in the upper-left corner for now...
	//V_DrawPatch (0,0, 0, W_CachePatchName(pagename, PU_CACHE));
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo(void)
{
	advancedemo = true;
}

//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo(void)
{
	players[consoleplayer[0]].playerstate = PST_LIVE;	// not reborn
	advancedemo = false;
	gameaction = ga_nothing;

	if (cv_disabledemos.value)
		demosequence = (demosequence + 1) % 3;
	else
	{
		if (gamemode == retail)
			demosequence = (demosequence + 1) % 7;
		else
			demosequence = (demosequence + 1) % 6;
	}

	if (cv_disabledemos.value)
	{
		switch (demosequence)
		{
			case 0:
				switch (gamemode)
				{
					case commercial:
						pagename = "TITLEPIC";
						pagetic = TICRATE * 11;
						S_StartMusic(mus_dm2ttl);
						break;
					default:
						pagename = "TITLEPIC";
						pagetic = 170;
						S_StartMusic(mus_intro);
						break;
				}
				gamestate = GS_DEMOSCREEN;
				break;
			case 1:
				pagetic = 200;
				gamestate = GS_DEMOSCREEN;
				pagename = "RMCREDIT";
				break;
			case 2:
				gamestate = GS_DEMOSCREEN;
				if (gamemode == commercial)
				{
					pagetic = TICRATE * 11;
					pagename = "CREDIT";
					S_StartMusic(mus_dm2ttl);
				}
				else
				{
					pagetic = 200;
					pagename = "CREDIT";
				}
				break;
		}
	}
	else
	{
		switch (demosequence)
		{
			case 0:
				switch (gamemode)
				{
					case commercial:
						pagename = "TITLEPIC";
						pagetic = TICRATE * 11;
						S_StartMusic(mus_dm2ttl);
						break;
					default:
						pagename = "TITLEPIC";
						pagetic = 170;
						S_StartMusic(mus_intro);
						break;
				}
				gamestate = GS_DEMOSCREEN;
				break;
			case 1:
				pagetic = 9999999;
				G_DeferedPlayDemo("demo1");
				break;
			case 2:
				pagetic = 200;
				gamestate = GS_DEMOSCREEN;
				pagename = "RMCREDIT";
				break;
			case 3:
				pagetic = 9999999;
				G_DeferedPlayDemo("demo2");
				break;
			case 4:
				gamestate = GS_DEMOSCREEN;
				if (gamemode == commercial)
				{
					pagetic = TICRATE * 11;
					pagename = "CREDIT";
					S_StartMusic(mus_dm2ttl);
				}
				else
				{
					pagetic = 200;
					pagename = "CREDIT";
				}
				break;
			case 5:
				pagetic = 9999999;
				G_DeferedPlayDemo("demo3");
				break;
			case 6:	// THE DEFINITIVE DOOM Special Edition demo
				pagetic = 9999999;
				G_DeferedPlayDemo("demo4");
				break;
		}
	}
}

// =========================================================================
//   D_DoomMain
// =========================================================================

//
// D_StartTitle
//
void D_StartTitle(void)
{
	int i;
	
	// Clear Profiles
	for (i = 0; i < MAXPLAYERS; i++)
		players[i].profile = NULL;
	
	MainDef.menuitems[1].status |= IT_DISABLED2;
	
	gameaction = ga_nothing;
	playerdeadview = false;
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		displayplayer[i] = consoleplayer[i] = 0;
	statusbarplayer = 0;
	demosequence = -1;
	paused = false;
	D_AdvanceDemo();
	CON_ToggleOff();
}

//
// D_AddFile
//
void D_AddFile(char *file)
{
	int numwadfiles;
	char *newfile;

	for (numwadfiles = 0; startupwadfiles[numwadfiles]; numwadfiles++)
		;

	newfile = malloc(strlen(file) + 1);
	strcpy(newfile, file);

	startupwadfiles[numwadfiles] = newfile;
}

// ==========================================================================
// Identify the Doom version, and IWAD file to use.
// Sets 'gamemode' to determine whether registered/commmercial features are
// available (notable loading PWAD files).
// ==========================================================================

// return gamemode for Doom or Ultimate Doom, use size to detect which one
// GhostlyDeath <July 11, 2008> -- This should work for now...
gamemode_t GetDoomVersion(char *wadfile)
{
	uint32_t Magic, NumLumps, IndexOffset;
	FILE* IWAD;
	
	// Cheap but it works
	IWAD = fopen(wadfile, "rb");
	
	if (!IWAD)
		return registered;	// woops!
	else
	{
		fread(&Magic, sizeof(uint32_t), 1, IWAD);
		fread(&NumLumps, sizeof(uint32_t), 1, IWAD);
		fread(&IndexOffset, sizeof(uint32_t), 1, IWAD);
	
		fclose(IWAD);
	}
	
	// GhostlyDeath <July 11, 2008> -- Only have the retail one on me now...
	if ((LittleSwapInt32(NumLumps) == 2306) && (LittleSwapInt32(IndexOffset) == 12371396))
		return retail;
	else
		return registered;
}

typedef struct wadinformation_s
{
	char filename[13];
	int mission;
	int mode;
	gamemode_t (*check)(char*);
	gamemission_t (*checkmission)(char*);
} wadinformation_t;

/* GhostlyDeath <November 18, 2008> -- Rewritten and IMPROVED! (Smaller and more efficient) */
// Also supports DOOMWADDIR and DOOMWADPATH
wadinformation_t wadinfos[] =
{
	{"doom2.wad", doom2, commercial, NULL, NULL},
	{"doom2f.wad", doom2, commercial, NULL, NULL},
	{"doomu.wad", doom, retail, NULL, NULL},
	{"doom.wad", doom, 0, GetDoomVersion, NULL},
	{"plutonia.wad", pack_plut, commercial, NULL, NULL},
	{"tnt.wad", pack_tnt, commercial, NULL, NULL},
	{"chex1.wad", pack_chex, chexquest1, NULL, NULL},
	{"doom1.wad", doom, shareware, NULL, NULL},
	{"freedoom.wad", doom2, commercial, NULL, NULL},
	{"freedm.wad", doom2, commercial, NULL, NULL}
};

#ifdef _WIN32
#define PATHDELIM '\\'
#else
#define PATHDELIM '/'
#endif

/* D_AddPWADs() -- Add PWADs from -file */
// GhostlyDeath <October 24, 2010> -- Greatly improved
void D_AddPWADs(void)
{
	char* PWADArg = NULL;
	char WADPath[256];
	
	/* Load every -file */
	if (M_CheckParm("-file"))
		while (M_IsNextParm())
		{
			// Get it
			PWADArg = M_GetNextParm();
			
			// Find it
			if (PWADArg)
				if (W_FindWad(PWADArg, NULL, WADPath, 256))
				{
					// Add it
					D_AddFile(WADPath);
				
					// Modify Game
					modifiedgame = true;
				}
		}
}

/* IdentifyVersion() -- Find an IWAD and remood.wad */
// GhostlyDeath <October 24, 2010> -- Greatly improved
void IdentifyVersion(void)
{
	char WADPath[256];
	char* IWADArg = NULL;
	boolean IWADOk = false;
	char* RMDArg = NULL;
	boolean RMDOk = false;
	char* BaseName;
	int i;
	
	/* Clear for IWAD */
	memset(WADPath, 0, sizeof(WADPath));
	
	/* -iwad argument set? */
	if (M_CheckParm("-iwad"))
	{
		// Get IWAD
		IWADArg = M_GetNextParm();
		
		// Try finding IWAD
		if (W_FindWad(IWADArg, NULL, WADPath, 256))
			// Don't search for a random IWAD
			IWADOk = true;
	}
	
	/* Search for random IWAD */
	if (!IWADOk)
		for (i = 0; i < sizeof(wadinfos) / sizeof(wadinformation_t); i++)
			if (W_FindWad(wadinfos[i].filename, NULL, WADPath, 256))
			{
				IWADOk = true;
				break;	// found it so break out
			}
	
	/* Load the WAD if we found it */
	if (strlen(WADPath))
	{
		// Get the basename of the WAD to determine mission, etc.
		BaseName = W_BaseName(WADPath);
		
		// Now load it
		D_AddFile(WADPath);
		
		// Set mission
		for (i = 0; i < sizeof(wadinfos) / sizeof(wadinformation_t); i++)
			if (strcasecmp(BaseName, wadinfos[i].filename) == 0)
			{
				// Set
				gamemission = wadinfos[i].mission;

				if (wadinfos[i].check)
					gamemode = wadinfos[i].check(WADPath);
				else
					gamemode = wadinfos[i].mode;
					
				if (devparm)
					CONS_Printf("IdentifyVersion: \"%s\" identified as mission %i mode %i\n", BaseName, gamemission, gamemode);
				
				// Break
				break;
			}
	}
	
	/* -remoodwad argument set */
	if (M_CheckParm("-remoodwad"))
	{
		// Get IWAD
		RMDArg = M_GetNextParm();
		
		// Try finding IWAD
		if (W_FindWad(RMDArg, NULL, WADPath, 256))
			// Don't search for remood.wad
			RMDOk = true;
	}
	
	/* Search for remood.wad */
	if (!RMDOk)
		if (W_FindWad("remood.wad", NULL, WADPath, 256))
			RMDOk = true;
	
	/* Load remood.wad */
	if (strlen(WADPath))
	{
		// Just load it
		D_AddFile(WADPath);
	}
	
	/* Failure messages */
	if (!IWADOk)
		I_Error("ReMooD was unable to find an IWAD (doom.wad, doom2.wad, etc.). To fix this problem: Place the correct IWADs where ReMooD is located; pass -iwad <exact path to IWAD>; pass -waddir <location of WADs>; set the environment variable DOOMWADPATH to locations where WADs exist.");
	else if (!RMDOk)
		I_Error("ReMooD was unable to find remood.wad. To fix this problem: Place remood.wad where ReMooD is located; pass -file <exact path to remood.wad>; pass -waddir <location of remood.wad>; set the environment variable DOOMWADPATH to a location where remood.wad exist.");
}

//added:11-01-98:
//
//  Center the title string, then add the date and time of compilation.
//
void D_MakeTitleString(char *s)
{
	char temp[82];
	char *t;
	char *u;
	int i;

	for (i = 0, t = temp; i < 82; i++)
		*t++ = ' ';

	for (t = temp + (80 - strlen(s)) / 2, u = s; *u != '\0';)
		*t++ = *u++;

	u = __DATE__;
	for (t = temp + 1, i = 11; i--;)
		*t++ = *u++;
	u = __TIME__;
	for (t = temp + 71, i = 8; i--;)
		*t++ = *u++;

	temp[80] = '\0';
	strcpy(s, temp);
}

void D_CheckWadVersion()
{
	int wadversion = 0;
	WadIndex_t lump;
	char *ver = NULL;
	char *verx = NULL;

	// more to do - Demyx, GhostlyDeath -- fixed

	lump = W_CheckNumForNameFirst("version");
	if (lump == INVALIDLUMP)
	{
		I_Error
			("VERSION lump not found! Be sure remood.wad can be accessed or use -file to load it manually.\nYou can also use -nocheckwadversion to ignore this error but it IS NOT recommended!\n");
		return;
	}
}

extern boolean g_PaintBallMode;

//
// D_DoomMain
//
void D_DoomMain(void)
{
	int i;
	int p;
	char file[256];
	char legacy[82];			//added:18-02-98: legacy title banner
	char title[82];				//added:11-01-98:moved, doesn't need to be global

	int startepisode;
	int startmap;
	boolean autostart;
	
	// GhostlyDeath <November 18, 2008> -- Move devparm up here
	devparm = M_CheckParm("-devparm");
	g_QuietConsole = M_CheckParm("-quiet");
	g_PaintBallMode = M_CheckParm("-paintballmode");
	
	// GhostlyDeath <July 6, 2008> -- initialize fields
	memset(player_names, 0, sizeof(player_names));
	memset(team_names, 0, sizeof(team_names));
	for (i = 0; i < MAXPLAYERS; i++)
	{
		sprintf(player_names[i], "Player %i", i + 1);
		sprintf(team_names[i], "Team %i", i + 1);
	}

	if (M_CheckParm("-novideo"))
		novideo = true;
	
	//added:18-02-98:keep error messages until the final flush(stderr)
	//if (setvbuf(stderr, NULL, _IOFBF, 1000))
	//	CONS_Printf("setvbuf didnt work\n");

	// get parameters from a response file (eg: doom3 @parms.txt)
	M_FindResponseFile();

	// identify the main IWAD file to use
	IdentifyVersion();

	//setbuf(stdout, NULL);		// non-buffered output
	modifiedgame = false;

	nomonsters = M_CheckParm("-nomonsters");

	//added:11-01-98:removed the repeated spaces in title strings,
	//               because GCC doesn't expand the TABS from my text editor.
	//  Now the string is centered in a larger one just before output,
	//  and the date and time of compilation is added. (see below)
	switch (gamemode)
	{
		case retail:
			strcpy(title, "The Ultimate DOOM Startup");
			break;
		case shareware:
			strcpy(title, "DOOM Shareware Startup");
			break;
		case registered:
			strcpy(title, "DOOM Registered Startup");
			break;
		case commercial:
			switch (gamemission)
			{
				case pack_plut:
					strcpy (title,"DOOM 2: Plutonia Experiment");
					break;
				case pack_tnt:
					strcpy (title,"DOOM 2: TNT - Evilution");
					break;
				default:
					strcpy(title, "DOOM 2: Hell on Earth");
					break;
			}
			break;
		default:
			strcpy(title, "Public DOOM");
			break;
	}

	//added:11-01-98:center the string, add compilation time and date.
	sprintf(legacy, "ReMooD v%i.%i%c \"%s\"",
		REMOOD_MAJORVERSION,
		REMOOD_MINORVERSION,
		REMOOD_RELEASEVERSION,
		REMOOD_VERSIONCODESTRING);
	D_MakeTitleString(legacy);

	CONS_Printf("%s\n%s\n", legacy, title);

	if (devparm)
		CONS_Printf(D_DEVSTR);

	// default savegame
	strcpy(savegamename, text[NORM_SAVEI_NUM]);

	// add any files specified on the command line with -file wadfile
	// to the wad list
	//
	// convenience hack to allow -wart e m to add a wad file
	// prepend a tilde to the filename so wadfile will be reloadable
	p = M_CheckParm("-wart");
	if (p)
	{
		myargv[p][4] = 'p';		// big hack, change to -warp

		// Map name handling.
		switch (gamemode)
		{
			case shareware:
			case retail:
			case registered:
				sprintf(file, "~" DEVMAPS "E%cM%c.wad", myargv[p + 1][0], myargv[p + 2][0]);
				CONS_Printf("Warping to Episode %s, Map %s.\n", myargv[p + 1], myargv[p + 2]);
				break;

			case commercial:
			default:
				p = atoi(myargv[p + 1]);
				if (p < 10)
					sprintf(file, "~" DEVMAPS "cdata/map0%i.wad", p);
				else
					sprintf(file, "~" DEVMAPS "cdata/map%i.wad", p);
				break;
		}
		D_AddFile(file);
	}

	if (M_CheckParm("-file"))
	{
		// the parms after p are wadfile/lump names,
		// until end of parms or another - preceded parm
		D_AddPWADs();
	}

	// load dehacked file
	p = M_CheckParm("-dehacked");
	if (!p)
		p = M_CheckParm("-deh");	//Fab:02-08-98:like Boom & DosDoom
	if (p != 0)
	{
		while (M_IsNextParm())
			D_AddFile(M_GetNextParm());
	}

	// get skill / episode / map from parms
	gameskill = sk_medium;
	startepisode = 1;
	startmap = 1;
	autostart = false;

	p = M_CheckParm("-skill");
	if (p && p < myargc - 1)
	{
		gameskill = myargv[p + 1][0] - '1';
		autostart = true;
	}

	p = M_CheckParm("-episode");
	if (p && p < myargc - 1)
	{
		startepisode = myargv[p + 1][0] - '0';
		startmap = 1;
		autostart = true;
	}

	p = M_CheckParm("-warp");
	if (p && p < myargc - 1)
	{
		if (gamemode == commercial)
			startmap = atoi(myargv[p + 1]);
		else
		{
			startepisode = myargv[p + 1][0] - '0';
			if (p < myargc - 2 && myargv[p + 2][0] >= '0' && myargv[p + 2][0] <= '9')
				startmap = myargv[p + 2][0] - '0';
			else
				startmap = 1;
		}
		autostart = true;
	}

	CONS_Printf(text[Z_INIT_NUM]);
	Z_Init();
	
	G_InitKeys();

	// adapt tables to legacy needs
	P_PatchInfoTables();

	if (gamemode == chexquest1)
		Chex1PatchEngine();

	CONS_Printf(text[W_INIT_NUM]);
	// load wad, including the main wad file
	if (W_InitMultipleFiles(startupwadfiles) == 0)
		I_Error("A WAD file was not found\n");

	if (!M_CheckParm("-nocheckwadversion"))
		D_CheckWadVersion();
	
	// GhostlyDeath <October 24, 2010> -- Load WAD Data
	W_LoadData();

	//Hurdler: someone wants to keep those lines?
	//BP: i agree with you why should be registered to play someone wads ?
	//    unfotunately most addistional wad have more texture and monsters
	//    that sharware wad do, so there will miss resourse :(

	//added:28-02-98: check for Ultimate doom.
	//if ( (gamemode==registered) && (W_CheckNumForName("E4M1") > 0) )
	//    gamemode = retail;

	// Check for -file in shareware
	if (modifiedgame)
	{
		// These are the lumps that will be checked in IWAD,
		// if any one is not present, execution will be aborted.
		char name[23][8] = {
			"e2m1", "e2m2", "e2m3", "e2m4", "e2m5", "e2m6", "e2m7", "e2m8",
			"e2m9",
			"e3m1", "e3m3", "e3m3", "e3m4", "e3m5", "e3m6", "e3m7", "e3m8",
			"e3m9",
			"dphoof", "bfgga0", "heada1", "cybra1", "spida1d1"
		};
		int i;

		if (gamemode == shareware)
			CONS_Printf("\nYou shouldn't use -file with the shareware version. Register!");

		// Check for fake IWAD with right name,
		// but w/o all the lumps of the registered version.
		if (gamemode == registered)
			for (i = 0; i < 23; i++)
				if (W_CheckNumForName(name[i]) == INVALIDLUMP)
					CONS_Printf("\nThis is not the registered version.");
	}

	// If additonal PWAD files are used, print modified banner
	if (modifiedgame)
		CONS_Printf(text[MODIFIED_NUM]);

	// Check and print which version is executed.
	switch (gamemode)
	{
		case shareware:
		case indetermined:
			CONS_Printf(text[SHAREWARE_NUM]);
			break;
		case registered:
		case retail:
		case commercial:
			CONS_Printf(text[COMERCIAL_NUM]);
			break;
		default:
			// Ouch.
			break;
	}
	cht_Init();

	//---------------------------------------------------- READY SCREEN
	//printf("\nI_StartupComm...");

	CONS_Printf("I_StartupTimer...\n");
	I_StartupTimer();

	// now initted automatically by use_mouse var code
	//CONS_Printf("I_StartupMouse...\n");
	//I_StartupMouse ();

	//CONS_Printf ("I_StartupKeyboard...\n");
	//I_StartupKeyboard (); // FIXME: this is a dummy, we can remove it!

	// now initialised automatically by use_joystick var code
	//CONS_Printf (text[I_INIT_NUM]);
	//I_InitJoystick ();

	// we need to check for dedicated before initialization of some subsystems
	dedicated = M_CheckParm("-dedicated") != 0;

	CONS_Printf("I_StartupGraphics...\n");
	I_StartupGraphics();

	//--------------------------------------------------------- CONSOLE
	// setup loading screen
	SCR_Startup();
	SCR_ReclassBuffers();

	// we need the font of the console
	CONS_Printf(text[HU_INIT_NUM]);
	HU_Init();

	COM_Init();
	CON_Init();

	D_RegisterClientCommands();	//Hurdler: be sure that this is called before D_CheckNetGame
	D_AddDeathmatchCommands();
	ST_AddCommands();
	T_AddCommands();
	P_Info_AddCommands();
	R_RegisterEngineStuff();
	S_RegisterSoundStuff();
	PROF_Init();
	CV_RegisterVar(&cv_screenslink);
	
	if (devparm)
		M_DumpMenuXML();
	
	CONS_Printf(text[M_INIT_NUM]);
	M_Init();

	//Fab:29-04-98: do some dirty chatmacros strings initialisation
	HU_HackChatmacros();
	//--------------------------------------------------------- CONFIG.CFG
	M_FirstLoadConfig();		// WARNING : this do a "COM_BufExecute()"
	
	VID_PrepareModeList();		// Regenerate Modelist according to cv_fullscreen

	// set user default mode or mode set at cmdline
	SCR_CheckDefaultMode();

	wipegamestate = gamestate;
	//------------------------------------------------ COMMAND LINE PARAMS

	// Initialize CD-Audio
	if (!M_CheckParm("-nocd"))
		I_InitCD();
	if (M_CheckParm("-respawn"))
		COM_BufAddText("respawnmonsters 1\n");
	if (M_CheckParm("-teamplay"))
		COM_BufAddText("teamplay 1\n");
	if (M_CheckParm("-teamskin"))
		COM_BufAddText("teamplay 2\n");
	if (M_CheckParm("-splitscreen"))
		CV_SetValue(&cv_splitscreen, 1);
	if (M_CheckParm("-altdeath"))
		COM_BufAddText("deathmatch 2\n");
	else if (M_CheckParm("-deathmatch"))
		COM_BufAddText("deathmatch 1\n");
	if (M_CheckParm("-fast"))
		COM_BufAddText("fastmonsters 1\n");
	if (M_CheckParm("-predicting"))
		COM_BufAddText("predictingmonsters 1\n");	//added by AC

	if (M_CheckParm("-timer"))
	{
		char *s = M_GetNextParm();
		COM_BufAddText(va("timelimit %s\n", s));
	}

	if (M_CheckParm("-avg"))
	{
		COM_BufAddText("timelimit 20\n");
		CONS_Printf(text[AUSTIN_NUM]);
	}

	// turbo option, is not meant to be saved in config, still
	// supported at cmd-line for compatibility
	if (M_CheckParm("-turbo") && M_IsNextParm())
		COM_BufAddText(va("turbo %s\n", M_GetNextParm()));

	// push all "+" parameter at the command buffer
	M_PushSpecialParameters();

	CONS_Printf(text[R_INIT_NUM]);
	R_Init();

	//
	// setting up sound
	//
	CONS_Printf(text[S_SETSOUND_NUM]);
	nosound = M_CheckParm("-nosound");
	nomusic = M_CheckParm("-nomusic");	// WARNING: DOS version initmusic in I_StartupSound
	digmusic = M_CheckParm("-digmusic");	// SSNTails 12-13-2002
	I_StartupSound();
	I_InitMusic();				// setup music buffer for quick mus2mid
	S_Init(cv_soundvolume.value, cv_musicvolume.value);

	CONS_Printf(text[ST_INIT_NUM]);
	ST_Init();

	////////////////////////////////
	// SoM: Init FraggleScript
	////////////////////////////////
	T_Init();

	// init all NETWORK
	CONS_Printf(text[D_CHECKNET_NUM]);
	if (D_CheckNetGame())
		autostart = true;

	// check for a driver that wants intermission stats
	p = M_CheckParm("-statcopy");
	if (p && p < myargc - 1)
	{
		I_Error("Sorry but statcopy isn't supported at this time\n");
		/*
		   // for statistics driver
		   extern  void*   statcopy;

		   statcopy = (void*)atoi(myargv[p+1]);
		   CONS_Printf (text[STATREG_NUM]);
		 */
	}

	// start the apropriate game based on parms
	p = M_CheckParm("-record");
	if (p && p < myargc - 1)
	{
		G_RecordDemo(myargv[p + 1]);
		autostart = true;
	}

	// demo doesn't need anymore to be added with D_AddFile()
	p = M_CheckParm("-playdemo");
	if (!p)
		p = M_CheckParm("-timedemo");
	if (p && M_IsNextParm())
	{
		char tmp[MAX_WADPATH];
		// add .lmp to identify the EXTERNAL demo file

		strcpy(tmp, M_GetNextParm());
		// get spaced filename or directory
		while (M_IsNextParm())
		{
			strcat(tmp, " ");
			strcat(tmp, M_GetNextParm());
		}
		
		// GhostlyDeath <July 6, 20008> -- Enable playback of internal demos again
		if (W_CheckNumForName(tmp) == INVALIDLUMP)
			FIL_DefaultExtension(tmp, ".lmp");

		CONS_Printf("Playing demo %s.\n", tmp);

		if ((p = M_CheckParm("-playdemo")))
		{
			singledemo = true;	// quit after one demo
			G_DeferedPlayDemo(tmp);
		}
		else
			G_TimeDemo(tmp);
		gamestate = wipegamestate = GS_NULL;

		return;
	}

	p = M_CheckParm("-loadgame");
	if (p && p < myargc - 1)
	{
		G_LoadGame(atoi(myargv[p + 1]));
	}
	else
	{
		if (autostart || netgame || M_CheckParm("+connect") || M_CheckParm("-connect"))
		{
			//added:27-02-98: reset the current version number
			G_Downgrade(VERSION);
			gameaction = ga_nothing;
			COM_BufAddText(va("map \"%s\"\n", G_BuildMapName(startepisode, startmap)));
		}
		else
			D_StartTitle();		// start up intro loop

	}
}

