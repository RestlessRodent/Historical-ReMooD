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
//      host/client network commands
//      commands are executed through the command buffer
//      like console commands
//      other miscellaneous commands (at the end)

#include "doomdef.h"
#include "doomstat.h"

#include "console.h"
#include "command.h"

#include "d_netcmd.h"
#include "i_system.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "g_input.h"
#include "m_menu.h"
#include "r_local.h"
#include "r_things.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "m_misc.h"
#include "am_map.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"
#include "v_video.h"
#include "d_main.h"
#include "p_demcmp.h"
#include "i_sound.h"
#include "i_util.h"

#include "b_bot.h"

// ------
// protos
// ------
void Command_Color_f(void);
void Command_Name_f(void);

void Command_WeaponPref(void);

void TeamPlay_OnChange(void);
void FragLimit_OnChange(void);
void Deahtmatch_OnChange(void);
void TimeLimit_OnChange(void);

void Command_Playdemo_f(void);
void Command_Timedemo_f(void);
void Command_Stopdemo_f(void);
void Command_Map_f(void);
void Command_Restart_f(void);

void Command_Addfile(void);
void Command_Pause(void);

void Command_Frags_f(void);
void Command_TeamFrags_f(void);
void Command_Version_f(void);
void Command_Quit_f(void);

void Command_Water_f(void);
void Command_ExitLevel_f(void);
void Command_Load_f(void);
void Command_Save_f(void);
void Command_ExitGame_f(void);

void Command_Kill(void);

// =========================================================================
//                           CLIENT VARIABLES
// =========================================================================

void CL_Player1Change(void);
void CL_Player2Change(void);
void CL_Player3Change(void);
void CL_Player4Change(void);

/****** MULTIPLAYER CONFIGURATIONS ******/

/*** PLAYER 1 ***/
// these two are just meant to be saved to the config
consvar_t cv_playername1 = { "cl1_name", NULL, CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player1Change };

consvar_t cv_playercolor1 = { "cl1_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t,
                              CL_Player1Change
                            };

// player's skin, saved for commodity, when using a favorite skins wad..
consvar_t cv_skin1 = { "cl1_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT,
                       NULL /*skin_cons_t */ , CL_Player1Change
                     };

consvar_t cv_weaponpref1 = { "cl1_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL,
                             CL_Player1Change
                           };
consvar_t cv_autoaim1 = { "cl1_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player1Change };

/*** PLAYER 2 ***/
// these two are just meant to be saved to the config
consvar_t cv_playername2 = { "cl2_name", NULL, CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player2Change };

consvar_t cv_playercolor2 = { "cl2_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t,
                              CL_Player2Change
                            };

// player's skin, saved for commodity, when using a favorite skins wad..
consvar_t cv_skin2 = { "cl2_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT,
                       NULL /*skin_cons_t */ , CL_Player2Change
                     };

consvar_t cv_weaponpref2 = { "cl2_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL,
                             CL_Player2Change
                           };
consvar_t cv_autoaim2 = { "cl2_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player2Change };

/*** PLAYER 3 ***/
consvar_t cv_playername3 = { "cl3_name", NULL, CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player3Change };
consvar_t cv_playercolor3 = { "cl3_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t, CL_Player3Change };
consvar_t cv_skin3 = { "cl3_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT, NULL /*skin_cons_t */ , CL_Player3Change };
consvar_t cv_weaponpref3 = { "cl3_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player3Change };
consvar_t cv_autoaim3 = { "cl3_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player3Change };

/*** PLAYER 1 ***/
consvar_t cv_playername4 = { "cl4_name", NULL, CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player4Change };
consvar_t cv_playercolor4 = { "cl4_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t, CL_Player4Change };
consvar_t cv_skin4 = { "cl4_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT, NULL /*skin_cons_t */ , CL_Player4Change };
consvar_t cv_weaponpref4 = { "cl4_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player4Change };
consvar_t cv_autoaim4 = { "cl4_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player4Change };

/****************************************/

consvar_t cv_originalweaponswitch = { "originalweaponswitch", "0", CV_SAVE, CV_OnOff,
                                      NULL
                                    };

CV_PossibleValue_t usemouse_cons_t[] = { {0, "Off"}, {1, "On"}, {2, "Force"}, {0, NULL} };

#ifdef LMOUSE2
CV_PossibleValue_t mouse2port_cons_t[] = { {0, "/dev/gpmdata"}, {1, "/dev/ttyS0"}, {2, "/dev/ttyS1"}, {
		3,
		"/dev/ttyS2"
	},
	{4, "/dev/ttyS3"}, {0, NULL}
};
#else
CV_PossibleValue_t mouse2port_cons_t[] = { {1, "COM1"}, {2, "COM2"}, {3, "COM3"}, {4, "COM4"}, {0, NULL} };
#endif

consvar_t cv_use_mouse = { "use_mouse", "1", CV_SAVE | CV_CALL, usemouse_cons_t, I_StartupMouse };
consvar_t cv_use_mouse2 = { "use_mouse2", "0", CV_SAVE | CV_CALL, usemouse_cons_t, I_StartupMouse2 };
consvar_t cv_use_joystick = { "use_joystick", "1", CV_SAVE | CV_CALL, usemouse_cons_t, I_InitJoystick };
consvar_t cv_use_joyids = { "use_joyids", "", CV_SAVE, NULL, NULL };

#ifdef LMOUSE2
consvar_t cv_mouse2port = { "mouse2port", "/dev/gpmdata", CV_SAVE, mouse2port_cons_t };
consvar_t cv_mouse2opt = { "mouse2opt", "0", CV_SAVE, NULL };
#else
consvar_t cv_mouse2port = { "mouse2port", "COM2", CV_SAVE, mouse2port_cons_t };
#endif
CV_PossibleValue_t teamplay_cons_t[] = { {0, "Off"}, {1, "Color"}, {2, "Skin"}, {3, NULL} };

CV_PossibleValue_t deathmatch_cons_t[] =
{
	{0, "Cooperative"},
	{1, "Deathmatch"},
	{2, "Alt. Deathmatch"},
	{3, "New Deathmatch"},
	// {4, "CTF"},
	
	{0, NULL}
};
CV_PossibleValue_t fraglimit_cons_t[] = { {0, "MIN"}, {1000, "MAX"}, {0, NULL} };

consvar_t cv_teamplay = { "teamplay", "0", CV_NETVAR | CV_CALL, teamplay_cons_t,
	TeamPlay_OnChange
};
consvar_t cv_teamdamage = { "teamdamage", "0", CV_NETVAR, CV_OnOff };

consvar_t cv_fraglimit = { "fraglimit", "0", CV_NETVAR | CV_CALL | CV_NOINIT, fraglimit_cons_t,
	FragLimit_OnChange
};

consvar_t cv_timelimit = { "timelimit", "0", CV_NETVAR | CV_CALL | CV_NOINIT, CV_Unsigned,
	TimeLimit_OnChange
};

consvar_t cv_deathmatch = { "deathmatch", "0", CV_NETVAR | CV_CALL, deathmatch_cons_t,
	Deahtmatch_OnChange
};

consvar_t cv_netstat = { "netstat", "0", 0, CV_OnOff };

consvar_t cv_g_gamespeed = { "g_gamespeed", "1", CV_NETVAR | CV_FLOAT };

// =========================================================================
//                           CLIENT STARTUP
// =========================================================================

// register client and server commands
//
void D_RegisterClientCommands(void)
{
	int i;
	char buf[20];
	
	for (i = 0; i < MAXSKINCOLORS; i++)
		Color_cons_t[i].strvalue = Color_Names[i];
		
	//
	// register commands
	//
	COM_AddCommand("playdemo", Command_Playdemo_f);
	COM_AddCommand("timedemo", Command_Timedemo_f);
	COM_AddCommand("stopdemo", Command_Stopdemo_f);
	COM_AddCommand("map", Command_Map_f);
	COM_AddCommand("restartlevel", Command_Restart_f);
	COM_AddCommand("exitgame", Command_ExitGame_f);
	COM_AddCommand("exitlevel", Command_ExitLevel_f);
	
	COM_AddCommand("addfile", Command_Addfile);
	COM_AddCommand("pause", Command_Pause);
	
	COM_AddCommand("version", Command_Version_f);
	COM_AddCommand("quit", Command_Quit_f);
	
	COM_AddCommand("chatmacro", Command_Chatmacro_f);	// hu_stuff.c
	
	COM_AddCommand("frags", Command_Frags_f);
	COM_AddCommand("teamfrags", Command_TeamFrags_f);
	
	COM_AddCommand("saveconfig", Command_SaveConfig_f);
	COM_AddCommand("loadconfig", Command_LoadConfig_f);
	COM_AddCommand("changeconfig", Command_ChangeConfig_f);
	COM_AddCommand("screenshot", M_ScreenShot);
	
	COM_AddCommand("kill", Command_Kill);
	
	// Slow mo
	CV_RegisterVar(&cv_g_gamespeed);
	
	// p_mobj.c
	CV_RegisterVar(&cv_itemrespawntime);
	CV_RegisterVar(&cv_itemrespawn);
	CV_RegisterVar(&cv_spawnmonsters);
	CV_RegisterVar(&cv_respawnmonsters);
	CV_RegisterVar(&cv_respawnmonsterstime);
	CV_RegisterVar(&cv_fastmonsters);
	CV_RegisterVar(&cv_predictingmonsters);	//added by AC for predmonsters
	CV_RegisterVar(&cv_splats);
	CV_RegisterVar(&cv_maxsplats);
	CV_RegisterVar(&cv_infiniteammo);
	CV_RegisterVar(&cv_g_gibrules);
	
	//
	// register main variables
	//
	//register these so it is saved to config
	cv_playername1.defaultvalue = I_GetUserName();
	if (cv_playername1.defaultvalue == NULL)
		cv_playername1.defaultvalue = "gi joe";
	if (cv_playername2.defaultvalue == NULL)
		cv_playername2.defaultvalue = "dough boy";
	if (cv_playername3.defaultvalue == NULL)
		cv_playername3.defaultvalue = "devil dog";
	if (cv_playername4.defaultvalue == NULL)
		cv_playername4.defaultvalue = "grunt";
		
		
	/****** SPLIT SCREEN ******/
	
	/*** PLAYER 1 ***/
	CV_RegisterVar(&cv_playername1);
	CV_RegisterVar(&cv_playercolor1);
	CV_RegisterVar(&cv_weaponpref1);
	CV_RegisterVar(&cv_autoaim1);
	CV_RegisterVar(&cv_skin1);
	
	/*** PLAYER 2 ***/
	CV_RegisterVar(&cv_playername2);
	CV_RegisterVar(&cv_playercolor2);
	CV_RegisterVar(&cv_weaponpref2);
	CV_RegisterVar(&cv_autoaim2);
	CV_RegisterVar(&cv_skin2);
	
	/*** PLAYER 3 ***/
	CV_RegisterVar(&cv_playername3);
	CV_RegisterVar(&cv_playercolor3);
	CV_RegisterVar(&cv_weaponpref3);
	CV_RegisterVar(&cv_autoaim3);
	CV_RegisterVar(&cv_skin3);
	
	/*** PLAYER 4 ***/
	CV_RegisterVar(&cv_playername4);
	CV_RegisterVar(&cv_playercolor4);
	CV_RegisterVar(&cv_weaponpref4);
	CV_RegisterVar(&cv_autoaim4);
	CV_RegisterVar(&cv_skin4);
	
	CV_RegisterVar(&cv_originalweaponswitch);
	
	// WATER HACK TEST UNTIL FULLY FINISHED
	COM_AddCommand("dev_water", Command_Water_f);
	
	//misc
	CV_RegisterVar(&cv_teamplay);
	CV_RegisterVar(&cv_teamdamage);
	CV_RegisterVar(&cv_fraglimit);
	CV_RegisterVar(&cv_deathmatch);
	CV_RegisterVar(&cv_timelimit);
	CV_RegisterVar(&cv_playdemospeed);
	CV_RegisterVar(&cv_netstat);
	
	COM_AddCommand("load", Command_Load_f);
	COM_AddCommand("save", Command_Save_f);
	
	//m_menu.c
	CV_RegisterVar(&cv_crosshair);
	//CV_RegisterVar (&cv_crosshairscale); // doesn't work for now
	CV_RegisterVar(&cv_autorun);
	CV_RegisterVar(&cv_invertmouse);
	CV_RegisterVar(&cv_alwaysfreelook);
	CV_RegisterVar(&cv_mousemove);
	CV_RegisterVar(&cv_showmessages);
	CV_RegisterVar(&cv_disabledemos);
	
	//g_input.c
	CV_RegisterVar(&cv_use_mouse2);
	CV_RegisterVar(&cv_invertmouse2);
	CV_RegisterVar(&cv_alwaysfreelook2);
	CV_RegisterVar(&cv_joystickfreelook);
	
	// WARNING : the order is important when inititing mouse2
	//           we need the mouse2port
	CV_RegisterVar(&cv_mouse2port);
#ifdef LMOUSE2
	CV_RegisterVar(&cv_mouse2opt);
#endif
	
	CV_RegisterVar(&cv_use_mouse);
	CV_RegisterVar(&cv_use_joystick);
	CV_RegisterVar(&cv_use_joyids);
	
	//s_sound.c
	CV_RegisterVar(&cv_soundvolume);
	CV_RegisterVar(&cv_musicvolume);
	CV_RegisterVar(&cv_rndsoundpitch);
	
	CV_RegisterVar(&cv_snd_speakersetup);
	CV_RegisterVar(&cv_snd_soundquality);
	CV_RegisterVar(&cv_snd_pcspeakerwave);
	CV_RegisterVar(&cv_snd_output);
	CV_RegisterVar(&cv_snd_device);
	CV_RegisterVar(&cv_snd_channels);
	CV_RegisterVar(&cv_snd_reservedchannels);
	CV_RegisterVar(&cv_snd_multithreaded);
	CV_RegisterVar(&cv_snd_sounddensity);
	
	//i_cdmus.c
	CV_RegisterVar(&cd_volume);
	CV_RegisterVar(&cdUpdate);
	
	// screen.c ?
	CV_RegisterVar(&cv_fullscreen);	// only for opengl so use differant name please and move it to differant place
	CV_RegisterVar(&cv_scr_depth);
	CV_RegisterVar(&cv_scr_width);
	CV_RegisterVar(&cv_scr_height);
	CV_RegisterVar(&cv_fragsweaponfalling);
	CV_RegisterVar(&cv_classicblood);
	CV_RegisterVar(&cv_classicmeleerange);
	CV_RegisterVar(&cv_classicmonsterlogic);
	
	// GhostlyDeath <July 8, 2009> -- Add FPS Counter
	CV_RegisterVar(&cv_vid_drawfps);
	
	// add cheat commands, I'm bored of deh patches renaming the idclev ! :-)
	COM_AddCommand("noclip", Command_CheatNoClip_f);
	COM_AddCommand("god", Command_CheatGod_f);
	COM_AddCommand("gimme", Command_CheatGimme_f);
	COM_AddCommand("give", Command_CheatGimme_f);	// GhostlyDeath -- Alias to gimme
	COM_AddCommand("summon", Command_CheatSummon_f);
	COM_AddCommand("summonfriend", Command_CheatSummonFriend_f);
	
	COM_AddCommand("snd_reset", Command_SoundReset_f);
	
	/* ideas of commands names from Quake
	   "status"
	   "notarget"
	   "fly"
	   "changelevel"
	   "reconnect"
	   "tell"
	   "kill"
	   "spawn"
	   "begin"
	   "prespawn"
	   "ping"
	
	   "startdemos"
	   "demos"
	   "stopdemo"
	 */
	
}

// =========================================================================
//                            CLIENT STUFF
// =========================================================================

void CL_DynPlayerChange(player_t* Player, consvar_t* name, consvar_t* color, consvar_t* skin, consvar_t* weaponpref, consvar_t* autoaim);

void CL_Player1Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[0]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[0]], &cv_playername1, &cv_playercolor1, &cv_skin1, &cv_weaponpref1, &cv_autoaim1);
}

void CL_Player2Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[1]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[1]], &cv_playername2, &cv_playercolor2, &cv_skin2, &cv_weaponpref2, &cv_autoaim2);
}

void CL_Player3Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[2]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[2]], &cv_playername3, &cv_playercolor3, &cv_skin3, &cv_weaponpref3, &cv_autoaim3);
}

void CL_Player4Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[3]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[3]], &cv_playername4, &cv_playercolor4, &cv_skin4, &cv_weaponpref4, &cv_autoaim4);
}

void CL_DynPlayerChange(player_t* Player, consvar_t* name, consvar_t* color, consvar_t* skin, consvar_t* weaponpref, consvar_t* autoaim)
{
	int i;
	
	// Change Name
	for (i = 0; i < MAXPLAYERS; i++)
		if (&players[i] == Player)
		{
			strncpy(player_names[i], name->string, MAXPLAYERNAME);
			player_names[i][MAXPLAYERNAME - 1] = 0;
			break;
		}
	// Change Color
	Player->skincolor = color->value % MAXSKINCOLORS;
	if (Player->mo)
	{
		Player->mo->flags &= ~MF_TRANSLATION;
		Player->mo->flags |= (Player->skincolor) << MF_TRANSSHIFT;
	}
	// Change Skin
	SetPlayerSkin(i, skin->string);
	
	// Change Weapon Pref
	// TODO: figure out later
	
	// Change Autoaim
	players[consoleplayer[i]].autoaim_toggle = autoaim->value;
}

void SendWeaponPref(void)
{
}

void D_SendPlayerConfig(void)
{
}

// ========================================================================

//  play a demo, add .lmp for external demos
//  eg: playdemo demo1 plays the internal game demo
//
// uint8_t*   demofile;       //demo file buffer

void Command_Playdemo_f(void)
{
	char name[256];
	
	if (COM_Argc() != 2)
	{
		CONL_PrintF("playdemo <demoname> : playback a demo\n");
		return;
	}
	// disconnect from server here ?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONL_PrintF("\nYou can't play a demo while in net game\n");
		return;
	}
	// open the demo file
	strcpy(name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played
	//FIL_DefaultExtension (name, ".lmp");
	
	CONL_PrintF("Playing back demo '%s'.\n", name);
	
	G_DoPlayDemo(name);
}

void Command_Timedemo_f(void)
{
	char name[256];
	
	if (COM_Argc() != 2)
	{
		CONL_PrintF("timedemo <demoname> : time a demo\n");
		return;
	}
	// disconnect from server here ?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONL_PrintF("\nYou can't play a demo while in net game\n");
		return;
	}
	// open the demo file
	strcpy(name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played
	//FIL_DefaultExtension (name, ".lmp");
	
	CONL_PrintF("Timing demo '%s'.\n", name);
	
	G_TimeDemo(name);
}

//  stop current demo
//
void Command_Stopdemo_f(void)
{
	G_CheckDemoStatus();
	CONL_PrintF("Stopped demo.\n");
}

//  Warp to map code.
//  Called either from map <mapname> console command, or idclev cheat.
//
void Command_Map_f(void)
{
	char buf[MAX_WADPATH + 3];
	
#define MAPNAME &buf[2]
	int i;
	int skill;
	
	if (COM_Argc() < 2 || COM_Argc() > 7)
	{
		CONL_PrintF("map <mapname> [-skill <1..5>] [-monsters <0/1>] [-noresetplayers]: warp to map\n");
		return;
	}
	
	strncpy(MAPNAME, COM_Argv(1), MAX_WADPATH);
	
	// internal wad lump
	if (W_CheckNumForName(MAPNAME) == -1)
	{
		CONL_PrintF("\2Internal game map '%s' not found\n" "(use .wad extension for external maps)\n", MAPNAME);
		return;
	}
	
	if ((i = COM_CheckParm("-skill")) != 0)
		skill = atoi(COM_Argv(i + 1)) - 1;
	else
		skill = gameskill;
		
	if ((i = COM_CheckParm("-monsters")) != 0)
		nomonsters = (atoi(COM_Argv(i + 1)) == 0);
	else
		nomonsters = (nomonsters != 0);
		
	if (demoplayback)
		COM_BufAddText("stopdemo\n");
		
	// this leave the actual game if needed
	server = false;
	netgame = false;
	if (!g_SplitScreen)
		multiplayer = false;
	else
		multiplayer = true;
	localgame = true;
	G_StopDemo();
	
	gamestate = wipegamestate = GS_NULL;
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
	{
		consoleplayer[i] = i;
		displayplayer[i] = i;
	}
	for (i = 0; i < MAXPLAYERS; i++)
		playeringame[i] = 0;
		
	for (i = 0; i < g_SplitScreen + 1; i++)
		playeringame[i] = 1;
		
		
	precache = false;
	G_InitNew(skill, MAPNAME, 1);
	precache = false;
}

void Command_Restart_f(void)
{
	if (netgame)
	{
		CONL_PrintF("Restartlevel don't work in network\n");
		return;
	}
	
	/* Demyx -- Don't allow restarting in demo's */
	if (!demoplayback && gamestate == GS_LEVEL)
		G_DoLoadLevel(true);
	else
		CONL_PrintF("You should be in a level to restart it !\n");
}

void Command_Pause(void)
{
	paused = ~paused;
	
	/* Demyx -- Don't allow pausing anywhere but the level and intermission. */
	if (paused && (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
		I_PauseSong(0);
	else
		I_ResumeSong(0);
}

//  Add a pwad at run-time
//  Search for sounds, maps, musics, etc..
//
void Command_Addfile(void)
{
	if (COM_Argc() != 2)
	{
		CONL_PrintF("addfile <wadfile.wad> : load wad file\n");
		return;
	}
	
	P_AddWadFile(COM_Argv(1), NULL);
}

// =========================================================================
//                            MISC. COMMANDS
// =========================================================================

void Command_Frags_f(void)
{
	int i, j;
	
	if (!cv_deathmatch.value)
	{
		CONL_PrintF("Frags : show the frag table\n");
		CONL_PrintF("Only for deathmatch games\n");
		return;
	}
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			CONL_PrintF("%-16s", player_names[i]);
			for (j = 0; j < MAXPLAYERS; j++)
				if (playeringame[j])
					CONL_PrintF(" %3d", players[i].frags[j]);
			CONL_PrintF("\n");
		}
}

void Command_TeamFrags_f(void)
{
	int i, j;
	fragsort_t unused[MAXPLAYERS];
	int frags[MAXPLAYERS];
	int fragtbl[MAXPLAYERS][MAXPLAYERS];
	
	if (!cv_deathmatch.value && !cv_teamplay.value)
	{
		CONL_PrintF("teamfrags : show the frag table for teams\n");
		CONL_PrintF("Only for deathmatch teamplay games\n");
		return;
	}
	
	HU_CreateTeamFragTbl(unused, frags, fragtbl);
	
	for (i = 0; i < 11; i++)
		if (teamingame(i))
		{
			CONL_PrintF("%-8s", team_names[i]);
			for (j = 0; j < 11; j++)
				if (teamingame(j))
					CONL_PrintF(" %3d", fragtbl[i][j]);
			CONL_PrintF("\n");
		}
}

//  Returns program version.
//
void Command_Version_f(void)
{
	CONL_PrintF("ReMooD %s \"%s\" (Compiled: %s %s)\n", REMOOD_VERSIONSTRING, REMOOD_VERSIONCODESTRING, __DATE__, __TIME__);
}

//  Quit the game immediately
//
void Command_Quit_f(void)
{
	I_Quit();
}

void FragLimit_OnChange(void)
{
	int i;
	
	if (cv_fraglimit.value > 0)
	{
		for (i = 0; i < MAXPLAYERS; i++)
			P_CheckFragLimit(&players[i]);
	}
}

uint32_t timelimitintics = 0;

void TimeLimit_OnChange(void)
{
	if (cv_timelimit.value)
	{
		CONL_PrintF("Levels will end after %d minute(s).\n", cv_timelimit.value);
		timelimitintics = cv_timelimit.value * 60 * TICRATE;
	}
	else
		CONL_PrintF("Time limit disabled\n");
}

void P_RespawnWeapons(void);
void Deahtmatch_OnChange(void)
{
	if (cv_deathmatch.value >= 2)
		CV_SetValue(&cv_itemrespawn, 1);
	else
		CV_SetValue(&cv_itemrespawn, 0);
	if (cv_deathmatch.value == 1 || cv_deathmatch.value == 3)
		P_RespawnWeapons();
		
	// give all key to the players
	if (cv_deathmatch.value)
	{
		int j;
		
		for (j = 0; j < MAXPLAYERS; j++)
			if (playeringame[j])
				players[j].cards = it_allkeys;
	}
}

void Command_ExitLevel_f(void)
{
	if (gamestate != GS_LEVEL || demoplayback)
		CONL_PrintF("You should be in a level to exit it !\n");
	else
		G_ExitLevel();
}

void Command_Load_f(void)
{
}

void Command_Save_f(void)
{
	if (COM_Argc() != 3)
	{
		CONL_PrintF("save <slot> <desciption>: save game\n");
		return;
	}
	
	G_DoSaveGame(atoi(COM_Argv(1)), COM_Argv(2));
}

void Command_ExitGame_f(void)
{
	/* Demyx -- Precaution: Don't allow Exitgame when you arent playing. */
	if (!demoplayback && gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_FINALE)
	{
		D_QuitNetGame();
		D_StartTitle();
	}
	else
		CONL_PrintF("You can't exit a game if you aren't even in one !\n");
}

void Command_Kill(void)
{
	if (!demoplayback && gamestate == GS_LEVEL)
		P_KillMobj(players[consoleplayer[0]].mo, NULL, players[consoleplayer[0]].mo);
	else
		CONL_PrintF("The Kill command cannot be used outside a game.\n");
}

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])

#define MAXLOCALJOYS	4

/*** GLOBALS ***/

bool_t g_NetDev = false;						// Network Debug
int g_SplitScreen = -1;							// Split screen players (-1 based)
bool_t g_PlayerInSplit[MAXSPLITSCREEN] = {false, false, false, false};

/*** LOCALS ***/

static bool_t l_PermitMouse = false;			// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static bool_t l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];
static D_NetPlayer_t* l_FirstNetPlayer = NULL;	// First Net Player

/*** FUNCTIONS ***/

/* D_NCSAddLocalPlayer() -- Adds local player (splitscreen) */
// Takes: Profile ID to put local player as
// Returns the added player
struct player_s* D_NCSAddLocalPlayer(const char* const a_ProfileID)
{
	size_t p, s;
	D_NetPlayer_t* NPp;
	
	/* Check */
	if (!a_ProfileID)
		return NULL;
	
	/* Not enough screens? */
	if (g_SplitScreen >= 3)
		return NULL;
	
	/* Find Free screen */
	for (s = 0; s < MAXSPLITSCREEN; s++)
		if (!g_PlayerInSplit[s])
			break;
	
	// No free screens (too many local players?)
	if (s >= MAXSPLITSCREEN)
		return NULL;
	
	/* Find player that is not in game */
	for (p = 0; p < MAXPLAYERS; p++)
		if (!playeringame[p])
			break;
	
	// No free slots (too many inside the game)
	if (p >= MAXPLAYERS)
		return NULL;
	
	/* Add player to game */
	playeringame[p] = true;
	g_PlayerInSplit[s] = true;
	consoleplayer[s] = p;
	displayplayer[s] = p;
	g_SplitScreen++;
	
	/* Initialize Player */
	// Clear everything
	memset(&players[p], 0, sizeof(players[p]));
	
	// Set as reborn
	players[p].playerstate = PST_REBORN;
	
	/* Setup network player */
	// Allocate
	NPp = players[p].NetPlayer = D_NCSAllocNetPlayer();
	
	// Set base info
	NPp->Type = DNPT_LOCAL;
	NPp->Player = &players[p];
	players[p].ProfileEx = NPp->Profile = D_FindProfileEx(a_ProfileID);
	
	/* Functions for player screens */
	am_recalc = true;
	R_ExecuteSetViewSize();
	
	// Fix automap
	if (automapactive)
		AM_Start();
	AM_LevelInit();
	
	/* Return the new player */
	return &players[p];
}

/* D_NCSAddBotPlayer() -- Add bot player */
struct player_s* D_NCSAddBotPlayer(const char* const a_ProfileID)
{
	size_t p;
	D_NetPlayer_t* NPp;
	
	/* Check */
	if (!a_ProfileID)
		return NULL;
	
	/* Find player that is not in game */
	for (p = 0; p < MAXPLAYERS; p++)
		if (!playeringame[p])
			break;
	
	// No free slots (too many inside the game)
	if (p >= MAXPLAYERS)
		return NULL;
	
	/* Add player to game */
	playeringame[p] = true;
	
	/* Initialize Player */
	// Clear everything
	memset(&players[p], 0, sizeof(players[p]));
	
	// Init
	G_AddPlayer(p);
	
	// Set as reborn	
	players[p].playerstate = PST_REBORN;
	
	// Set color
	players[p].skincolor = p % MAXSKINCOLORS;
	
	/* Setup network player */
	// Allocate
	NPp = players[p].NetPlayer = D_NCSAllocNetPlayer();
	
	// Set base info
	NPp->Type = DNPT_BOT;
	NPp->Player = &players[p];
	NPp->NetColor = p % MAXSKINCOLORS;
	snprintf(NPp->DisplayName, MAXPLAYERNAME - 1, "Bot %i", p);
	
	/* Return the new player */
	return &players[p];
}

/* DS_NCSNetCommand() -- Network commands */
static CONL_ExitCode_t DS_NCSNetCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
	struct player_s* p;
	int i;
	
	/* Check */
	if (a_ArgC < 2)
		return CLE_INVALIDARGUMENT;
	
	/* Which Sub Command? */
	// Add Player to Game
	if (strcasecmp(a_ArgV[1], "addplayer") == 0)
	{
		CONL_PrintF("NET: Requesting the server add local player.\n");
		D_NCSR_RequestNewPlayer(D_FindProfileEx((a_ArgC >= 3 ? a_ArgV[2] : "guest")));
		return CLE_SUCCESS;
	}
	
	// Add Bot to game
	else if (strcasecmp(a_ArgV[1], "addbot") == 0)
	{
		p = D_NCSAddBotPlayer((a_ArgC > 2 ? a_ArgV[2] : "guest"));
		
		if (p)
		{
			i = p - players;
		
			CONL_PrintF("Net: Added bot %i.\n", i);
			
			// Debugging? Split screen the bot
			if (g_BotDebug || M_CheckParm("-devbots"))
			{
				if (g_SplitScreen < MAXSPLITSCREEN)
				{
					consoleplayer[g_SplitScreen + 1] = displayplayer[g_SplitScreen + 1] = i;
					g_PlayerInSplit[g_SplitScreen + 1] = true;
					g_SplitScreen++;
					R_ExecuteSetViewSize();
				}
			}
		}
		else
		{
			CONL_PrintF("Net: Failed to add bot.\n");
			return CLE_FAILURE;
		}
	} 
	
	// Add as many bots as possible
	else if (strcasecmp(a_ArgV[1], "addmaxbots") == 0)
	{
		while ((p = D_NCSAddBotPlayer((a_ArgC > 2 ? a_ArgV[2] : "default"))))
		{
			i = p - players;
		
			CONL_PrintF("Net: Added bot %i.\n", i);
		}
	} 
	
	/* Success */
	return CLE_SUCCESS;
}

/* D_NCSInit() -- Initialize network client/server */
void D_NCSInit(void)
{
	/* Debug? */
	if (M_CheckParm("-netdev"))
		g_NetDev = true;
	
	/* Register "net" command */
	CONL_AddCommand("net", DS_NCSNetCommand);
}

/* D_NCSHandleEvent() -- Handle advanced events */
bool_t D_NCSHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t ButtonNum, LocalJoy;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Which kind of event? */
	switch (a_Event->Type)
	{
			// Mouse
		case IET_MOUSE:
			// Add position to movement
			l_MouseMove[0] += a_Event->Data.Mouse.Move[0];
			l_MouseMove[1] += a_Event->Data.Mouse.Move[1];
			break;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			break;
			
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
				// Button Pressed Down
				if (a_Event->Data.Joystick.Button)
				{
					// Get Number
					ButtonNum = a_Event->Data.Joystick.Button;
					ButtonNum--;
					
					// Limited to 32 buttons =(
					if (ButtonNum >= 0 && ButtonNum < 32)
					{
						// Was it pressed?
						if (a_Event->Data.Joystick.Down)
							l_JoyButtons[LocalJoy] |= (1 << ButtonNum);
						else
							l_JoyButtons[LocalJoy] &= ~(1 << ButtonNum);
					}
				}
				
				// Axis Moved
				else if (a_Event->Data.Joystick.Axis)
				{
					ButtonNum = a_Event->Data.Joystick.Axis;
					ButtonNum--;
					
					if (ButtonNum >= 0 && ButtonNum < MAXJOYAXIS)
						l_JoyAxis[LocalJoy][ButtonNum] = a_Event->Data.Joystick.Value;
				}
			}
			break;
		
			// Unknown
		default:
			break;
	}
	
	/* Un-Handled */
	return false;
}

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in weaponinfo_t for ordering info
static uint8_t DS_NCSNextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	weaponinfo_t** weapons;
	
	/* Get current weapon info */
	weapons = player->weaponinfo;
	
	/* Get the weapon with the lowest and highest order */
	// Find first gun the player has (so order is correct)
	MostOrder = LeastOrder = 0;
	for (w = 0; w < NUMWEAPONS; w++)
		if (P_CanUseWeapon(player, w))
		{
			// Got the first available gun
			MostOrder = LeastOrder = w;
			break;
		}
	
	// Now go through
	for (w = 0; w < NUMWEAPONS; w++)
	{
		// Can't use this gun?
		if (!P_CanUseWeapon(player, w))
			continue;
		
		// Least
		if (weapons[w]->SwitchOrder < weapons[LeastOrder]->SwitchOrder)
			LeastOrder = w;
		
		// Most
		if (weapons[w]->SwitchOrder > weapons[MostOrder]->SwitchOrder)
			MostOrder = w;
	}
	
	/* Look for the current weapon in the weapon list */
	// Well that was easy
	fw = s = g = player->readyweapon;
	
	/* Constantly change the weapon */
	// Prepare variables
	Neg = (step < 0 ? true : false);
	StepsAdd = (Neg ? -1 : 1);
	StepsLeft = step * StepsAdd;
	
	// Go through the weapon list, step times
	while (StepsLeft > 0)
	{
		// Clear variables
		BestDiff = 9999999;		// The worst weapon difference ever
		BestNum = NUMWEAPONS;
		
		// Go through every weapon and find the next in the order
		for (w = 0; w < NUMWEAPONS; w++)
		{
			// Ignore the current weapon (don't want to switch back to it)
			if (w == fw)		// Otherwise BestDiff is zero!
				continue;
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, w))
				continue;
			
			// Only consider worse/better weapons?
			if ((Neg && weapons[w]->SwitchOrder > weapons[fw]->SwitchOrder) || (!Neg && weapons[w]->SwitchOrder < weapons[fw]->SwitchOrder))
				continue;
			
			// Get current diff
			ThisDiff = abs(weapons[fw]->SwitchOrder - weapons[w]->SwitchOrder);
			
			// Closer weapon?
			if (ThisDiff < BestDiff)
			{
				BestDiff = ThisDiff;
				BestNum = w;
			}
		}
		
		// Found no weapon? Then "loop" around
		if (BestNum == NUMWEAPONS)
		{
			// Switch to the highest gun if going down
			if (Neg)
				fw = MostOrder;
			
			// And if going up, go to the lowest
			else
				fw = LeastOrder;
		}
		
		// Found a weapon
		else
		{
			// Switch to this gun
			fw = BestNum;
		}
		
		// Next step
		StepsLeft--;
	}
	
	/* Return the weapon we want */
	return fw;
}

/* GAMEKEYDOWN() -- Checks if a key is down */
static bool_t GAMEKEYDOWN(D_ProfileEx_t* const a_Profile, const uint8_t a_Key)
{
	size_t i;
	uint32_t CurrentButton;
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if (a_Profile->Ctrls[a_Key][i] >= 0 && a_Profile->Ctrls[a_Key][i] < NUMIKEYBOARDKEYS)
			if (l_KeyDown[a_Profile->Ctrls[a_Key][i]])
				return true;
	
	/* Check Joysticks */
	if (a_Profile->Flags & DPEXF_GOTJOY)
		if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
			for (i = 0; i < 4; i++)
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == 0x1000)
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
				
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_JoyButtons[a_Profile->JoyControl] & (1 << CurrentButton))
							return true;
				}
	
	/* Not pressed */
	return false;
}

/* D_NCSLocalBuildTicCmd() -- Build local tic command */
static void D_NCSLocalBuildTicCmd(D_NetPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd, size_t* const a_SIDp)
{
#define MAXWEAPONSLOTS 12
	D_ProfileEx_t* Profile;
	player_t* Player;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT;
	bool_t IsTurning, GunInSlot;
	int slot, j, l, k;
	weapontype_t newweapon;
	weapontype_t SlotList[MAXWEAPONSLOTS];
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Obtain profile */
	Profile = a_NPp->Profile;
	Player = a_NPp->Player;
	
	// No profile?
	if (!Profile)
		return;
	
	/* Find Player ID */
	PID = a_NPp->Player - players;
	
	// Illegal player?
	if (PID < 0 || PID >= MAXPLAYERS)
		return;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_PlayerInSplit[SID])
			if (consoleplayer[SID] == PID)
				break;
	
	// Not found?
	if (SID >= MAXSPLITSCREEN)
		return;
	
	// Return pointer
	if (a_SIDp)
		*a_SIDp = SID;
	
	/* Reset Some Things */
	SideMove = ForwardMove = BaseAT = 0;
	IsTurning = false;
	
	/* Modifiers */
	// Mouse Sensitivity
	SensMod = 0;
	
	// Movement Modifier
	if (GAMEKEYDOWN(Profile, DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GAMEKEYDOWN(Profile, DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GAMEKEYDOWN(Profile, DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((Profile->Flags & DPEXF_SLOWTURNING) &&
			gametic < (Profile->TurnHeld + Profile->SlowTurnTime))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	/* Player has joystick input? */
	if (Profile->Flags & DPEXF_GOTJOY)
	{
		// Read input for all axis
		for (i = 0; i < MAXJOYAXIS; i++)
		{
			// Modify with sensitivity
			TargetMove = ((float)l_JoyAxis[Profile->JoyControl][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
			// Which movement to perform?
			switch (Profile->JoyAxis[MouseMod][i])
			{
					// Movement
				case DPEXCMA_MOVEX:
				case DPEXCMA_MOVEY:
					// Movement is fractionally based
					TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_forwardmove[MoveSpeed]);
					
					// Now which action really?
					if (Profile->JoyAxis[MouseMod][i] == DPEXCMA_MOVEX)
						SideMove += TargetMove;
					else
						ForwardMove -= TargetMove;
					break;
					
					// Looking Left/Right
				case DPEXCMA_LOOKX:
					TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_angleturn[TurnSpeed]);
					IsTurning = true;
					BaseAT -= TargetMove;
					break;
					
					// Looking Up/Down
				case DPEXCMA_LOOKY:
					break;
				
				default:
					break;
			}
		}
	}
	
	/* Player has mouse input? */
	if (l_PermitMouse && (Profile->Flags & DPEXF_GOTMOUSE))
	{
		// Read mouse input for both axis
		for (i = 0; i < 2; i++)
		{
			// Modify with sensitivity
			TargetMove = l_MouseMove[i] * ((((float)(Profile->MouseSens[SensMod] * Profile->MouseSens[SensMod])) / 110.0) + 0.1);
			
			// Do action for which movement type?
			switch (Profile->MouseAxis[MouseMod][i])
			{
					// Strafe Left/Right
				case DPEXCMA_MOVEX:
					SideMove += TargetMove;
					break;
					
					// Move Forward/Back
				case DPEXCMA_MOVEY:
					ForwardMove += TargetMove;
					break;
					
					// Left/Right Look
				case DPEXCMA_LOOKX:
					BaseAT -= TargetMove * 8;
					break;
					
					// Up/Down Look
				case DPEXCMA_LOOKY:
					localaiming[SID] += TargetMove << 19;
					break;
				
					// Unknown
				default:
					break;
			}
		}
		
		// Clear mouse permission
		l_PermitMouse = false;
		
		// Clear mouse input
		l_MouseMove[0] = l_MouseMove[1] = 0;
	}
	
	/* Handle Player Control Keyboard Stuff */
	// Weapon Attacks
	if (GAMEKEYDOWN(Profile, DPEXIC_ATTACK))
		a_TicCmd->buttons |= BT_ATTACK;
	
	// Use
	if (GAMEKEYDOWN(Profile, DPEXIC_USE))
		a_TicCmd->buttons |= BT_USE;
	
	// Jump
	if (GAMEKEYDOWN(Profile, DPEXIC_JUMP))
		a_TicCmd->buttons |= BT_JUMP;
	
	// Keyboard Turning
	if (GAMEKEYDOWN(Profile, DPEXIC_TURNLEFT))
	{
		// Strafe
		if (MoveMod)
			SideMove -= c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT += c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	if (GAMEKEYDOWN(Profile, DPEXIC_TURNRIGHT))
	{
		// Strafe
		if (MoveMod)
			SideMove += c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT -= c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	
	// Keyboard Moving
	if (GAMEKEYDOWN(Profile, DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GAMEKEYDOWN(Profile, DPEXIC_LOOKCENTER))
		localaiming[SID] = 0;
	else
	{
		if (GAMEKEYDOWN(Profile, DPEXIC_LOOKUP))
			localaiming[SID] += Profile->LookUpDownSpeed;
		if (GAMEKEYDOWN(Profile, DPEXIC_LOOKDOWN))
			localaiming[SID] -= Profile->LookUpDownSpeed;
	}
	
	// Weapons
		// Next
	if (GAMEKEYDOWN(Profile, DPEXIC_NEXTWEAPON))
	{
		// Set switch
		a_TicCmd->buttons |= BT_CHANGE;
		a_TicCmd->XNewWeapon = DS_NCSNextWeapon(Player, 1);
	}
	
		// Prev
	else if (GAMEKEYDOWN(Profile, DPEXIC_PREVWEAPON))
	{
		// Set switch
		a_TicCmd->buttons |= BT_CHANGE;
		a_TicCmd->XNewWeapon = DS_NCSNextWeapon(Player, -1);
	}
	
		// Slots
	else
	{
		// Which slot?
		slot = -1;
		
		// Look for keys
		for (i = DPEXIC_SLOT1; i <= DPEXIC_SLOT10; i++)
			if (GAMEKEYDOWN(Profile, i))
			{
				slot = (i - DPEXIC_SLOT1) + 1;
				break;
			}
		
		// Hit slot?
		if (slot != -1)
		{
			// Clear flag
			GunInSlot = false;
			l = 0;
		
			// Figure out weapons that belong in this slot
			for (j = 0, i = 0; i < NUMWEAPONS; i++)
				if (P_CanUseWeapon(Player, i))
				{
					// Weapon not in this slot?
					if (Player->weaponinfo[i]->SlotNum != slot)
						continue;
				
					// Place in slot list before the highest
					if (j < (MAXWEAPONSLOTS - 1))
					{
						// Just place here
						if (j == 0)
						{
							// Current weapon is in this slot?
							if (Player->readyweapon == i)
							{
								GunInSlot = true;
								l = j;
							}
						
							// Place in last spot
							SlotList[j++] = i;
						}
					
						// Otherwise more work is needed
						else
						{
							// Start from high to low
								// When the order is lower, we know to insert now
							for (k = 0; k < j; k++)
								if (Player->weaponinfo[i]->SwitchOrder < Player->weaponinfo[SlotList[k]]->SwitchOrder)
								{
									// Current gun may need shifting
									if (!GunInSlot)
									{
										// Current weapon is in this slot?
										if (Player->readyweapon == i)
										{
											GunInSlot = true;
											l = k;
										}
									}
								
									// Possibly shift gun
									else
									{
										// If the current gun is higher then this gun
										// then it will be off by whatever is more
										if (Player->weaponinfo[SlotList[l]]->SwitchOrder > Player->weaponinfo[i]->SwitchOrder)
											l++;
									}
								
									// move up
									memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
									// Place in slightly upper spot
									SlotList[k] = i;
									j++;
								
									// Don't add it anymore
									break;
								}
						
							// Can't put it anywhere? Goes at end then
							if (k == j)
							{
								// Current weapon is in this slot?
								if (Player->readyweapon == i)
								{
									GunInSlot = true;
									l = k;
								}
							
								// Put
								SlotList[j++] = i;
							}
						}
					}
				}
		
			// No guns in this slot? Then don't switch to anything
			if (j == 0)
				newweapon = Player->readyweapon;
		
			// If the current gun is in this slot, go to the next in the slot
			else if (GunInSlot)		// from [best - worst]
				newweapon = SlotList[((l - 1) + j) % j];
		
			// Otherwise, switch to the best gun there
			else
				// Set it to the highest valued gun
				newweapon = SlotList[j - 1];
		
			// Did it work?
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->buttons |= BT_CHANGE;
				a_TicCmd->XNewWeapon = newweapon;
			}
		}
	}
	
	/* Handle special functions */
	// Coop Spy
	if (GAMEKEYDOWN(Profile, DPEXIC_COOPSPY))
	{
		// Only every half second
		if (gametic > (Profile->CoopSpyTime + (TICRATE >> 1)))
		{
			do
			{
				displayplayer[SID] = (displayplayer[SID] + 1) % MAXPLAYERS;
			} while (!playeringame[displayplayer[SID]] || !P_PlayerOnSameTeam(&players[consoleplayer[SID]], &players[displayplayer[SID]]));
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(displayplayer[SID] == consoleplayer[SID] ? "Yourself" : D_NCSGetPlayerName(displayplayer[SID]))
				);
			
			// Reset timeout
			Profile->CoopSpyTime = gametic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		Profile->CoopSpyTime = 0;
	
	/* Set Movement Now */
	// Cap
	if (SideMove > MAXPLMOVE)
		SideMove = MAXPLMOVE;
	else if (SideMove < -MAXPLMOVE)
		SideMove = -MAXPLMOVE;
	
	if (ForwardMove > MAXPLMOVE)
		ForwardMove = MAXPLMOVE;
	else if (ForwardMove < -MAXPLMOVE)
		ForwardMove = -MAXPLMOVE;
	
	// Set
	a_TicCmd->sidemove = SideMove;
	a_TicCmd->forwardmove = ForwardMove;
	
	/* Slow turning? */
	if (!IsTurning)
		Profile->TurnHeld = gametic;
	
	/* Turning */
	a_TicCmd->BaseAngleTurn = BaseAT;
	
	/* Set from localaiming and such */
	// Don't change angle when teleporting
#if 0
	if ((!Player->mo) || (Player->mo && Player->mo->reactiontime <= 0))
	{
		// Local angle (x look)
		localangle[SID] += (a_TicCmd->angleturn << 16);
		a_TicCmd->angleturn = localangle[SID] >> 16;
	}
#endif
	
	// Local aiming (y look)
	a_TicCmd->aiming = G_ClipAimingPitch(&localaiming[SID]);
#undef MAXWEAPONSLOTS
}

/* D_NCSNetUpdateSingle() -- Update single player */
void D_NCSNetUpdateSingle(struct player_s* a_Player)
{
	size_t PID, SID, i;
	D_NetPlayer_t* NPp;
	ticcmd_t* TicCmd, *LocalTicCmd;
	
	/* Check */
	if (!a_Player)
		return;
	
	// Get player ID
	PID = a_Player - players;
	
	// Get Screen ID
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_PlayerInSplit[SID])
			if (PID == consoleplayer[SID])
				break;
	
	// More checks
	if (PID < 0 || PID >= MAXPLAYERS || !playeringame[PID])
		return;
	
	/* Get player's netplayer */
	NPp = players[PID].NetPlayer;
	
	// No net player?
	if (!NPp)
		return;
	
	/* Generate Commands */
	// Use last free spot
	if (NPp->LocalTicTotal < MAXDNETTICCMDCOUNT - 1)
		i = NPp->LocalTicTotal++;
	else
		i = MAXDNETTICCMDCOUNT - 1;
	TicCmd = &NPp->LocalTicCmd[i];
	
	// Set Time
	NPp->LastLocalTic = g_ProgramTic;
	
	// Clear command
	memset(TicCmd, 0, sizeof(*TicCmd));
	
	// Now what to do with this?
	switch (NPp->Type)
	{
			// Local player on this computer
		case DNPT_LOCAL:
			// Fill command based on controller input
			D_NCSLocalBuildTicCmd(NPp, TicCmd, NULL);
			break;
		
			// Networked player on another system
		case DNPT_NETWORK:
			break;
			
			// Bot, a simulated player
		case DNPT_BOT:
			// No bot data?
			if (!NPp->BotData)
				NPp->BotData = B_InitBot(NPp);
			
			// Build bot tic command
			TicCmd = &NPp->TicCmd[i];
			B_BuildBotTicCmd(NPp->BotData, TicCmd);
			break;
			
			// Unknown
		default:
			break;
	}
}

/* D_NCSNetUpdateAll() -- Update all players */
void D_NCSNetUpdateAll(void)
{
	size_t i, j, SID;
	D_NetPlayer_t* NetPlayer;
	ticcmd_t TicMerge;
	
	// Extended tic command stuff
	uint8_t XNewWeapon;							// New weapon to switch to
	
	/* Enable Mouse Input */
	l_PermitMouse = true;
	
	/* Update All Players */
	for (i = 0; i < MAXPLAYERS; i++)
		D_NCSNetUpdateSingle(&players[i]);
	
	D_NCUpdate();
}

/* D_NCSNetUpdateSingleTic() -- Single tic update */
void D_NCSNetUpdateSingleTic(void)
{
	size_t i, j, SID;
	D_NetPlayer_t* NetPlayer;
	ticcmd_t TicMerge;
	
	/* Transmit local tic command */
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// Get and check netplayer
			NetPlayer = players[i].NetPlayer;
		
			if (!NetPlayer)
				continue;
		
			// Non-local player?
			if (NetPlayer->Type != DNPT_LOCAL && NetPlayer->Type != DNPT_BOT)
				continue;
		
			// Merge all local tics into a single source
			memset(&TicMerge, 0, sizeof(TicMerge));
			
			// Merge
			D_NCSNetMergeTics(&TicMerge, NetPlayer->LocalTicCmd, NetPlayer->LocalTicTotal);
			
			// Clear away
			NetPlayer->LocalTicTotal = 0;
			
			// Transmit this tic command
			D_NCSNetTicTransmit(NetPlayer, &TicMerge);
		}
}

/* D_NCSNetSetState() -- Set state of local players */
void D_NCSNetSetState(const D_NetState_t a_State)
{
	size_t i;
	
	/* Check */
	if (a_State < 0 || a_State >= NUMDNETSTATES)
		return;
	
	/* Set it for all local players */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (g_PlayerInSplit[i] && playeringame[consoleplayer[i]])
			if (players[consoleplayer[i]].NetPlayer)
				players[consoleplayer[i]].NetPlayer->NetState = a_State;
}

/* D_NCSNetTicTransmit() -- Transmit tic command to server */
void D_NCSNetTicTransmit(D_NetPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd)
{
	size_t i, SID;
	ticcmd_t* DestTic;
	ticcmd_t Merge;
	I_EventEx_t OSKEvent;
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Determine Local Screen */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_PlayerInSplit[SID] && (a_NPp->Player - players) == consoleplayer[SID])
			break;
		
	/* Create Synthetic OSK Events */
	// These are player movement based
	// Right/Left Movement
	memset(&OSKEvent, 0, sizeof(OSKEvent));
	
	// Set type
	OSKEvent.Type = IET_SYNTHOSK;
	OSKEvent.Data.SynthOSK.PNum = SID;
	
	// Right/Left
	if ((a_TicCmd->sidemove) >= (c_sidemove[0] >> 1) || (a_TicCmd->BaseAngleTurn) <= -(c_angleturn[2] >> 1))
		OSKEvent.Data.SynthOSK.Right = 1;
	else if ((a_TicCmd->sidemove) <= -(c_sidemove[0] >> 1) || (a_TicCmd->BaseAngleTurn) >= (c_angleturn[2] >> 1))
		OSKEvent.Data.SynthOSK.Right = -1;
	
	// Up/Down
	if ((a_TicCmd->forwardmove) <= -(c_forwardmove[0] >> 1))
		OSKEvent.Data.SynthOSK.Down = 1;
	else if ((a_TicCmd->forwardmove) >= (c_forwardmove[0] >> 1))
		OSKEvent.Data.SynthOSK.Down = -1;
	
	// Press
	if (a_TicCmd->buttons & BT_ATTACK)
		OSKEvent.Data.SynthOSK.Press = 1;
	
	// Push Event
	if (OSKEvent.Data.SynthOSK.Right || OSKEvent.Data.SynthOSK.Down || OSKEvent.Data.SynthOSK.Press)	
		I_EventExPush(&OSKEvent);
	
	// if the OSK is visible do not transmit
		// TODO FIXME
	
	/* Remote Game */
	if (!D_SyncNetIsSolo())
	{
	}
	
	/* Local Game */
	else
	{
		// Add local command to end
		a_NPp->TicCmd[a_NPp->TicTotal++] = *a_TicCmd;
		
		// Merge it All
		memset(&Merge, 0, sizeof(Merge));
		D_NCSNetMergeTics(&Merge, a_NPp->TicCmd, a_NPp->TicTotal);
		
		// Set local view angle
		if (SID < MAXSPLITSCREEN)
		{
			localangle[SID] += Merge.BaseAngleTurn << 16;
			Merge.angleturn = localangle[SID] >> 16;
		}
		
		// Only use this tic (single player game)
		a_NPp->TicCmd[0] = Merge;
		a_NPp->TicTotal = 1;
	}
}

/* D_NCSNetMergeTics() -- Merges all tic commands */
void D_NCSNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc)
{
#define __REMOOD_SWIRVYANGLE
	size_t i, j;
	int32_t FM, SM, AT;
	fixed_t xDiv;
	
	/* Check */
	if (!a_DestCmd || !a_SrcList || !a_NumSrc)
		return;
	
	/* Merge Variadic Stuff */
	// Super merging
	FM = SM = AT = 0;
	for (j = 0; j < a_NumSrc; j++)
	{
		FM += a_SrcList[j].forwardmove;
		SM += a_SrcList[j].sidemove;

#if defined(__REMOOD_SWIRVYANGLE)
		AT += a_SrcList[j].BaseAngleTurn;
#else
		// Use the furthest aiming angle
		if (abs(a_SrcList[j].BaseAngleTurn) > abs(AT))
			AT = a_SrcList[j].BaseAngleTurn;
#endif
		
		// Merge weapon here
		if (!a_DestCmd->XNewWeapon)
			a_DestCmd->XNewWeapon = a_SrcList[j].XNewWeapon;
		
		// Clear slot and weapon masks (they OR badly)
		a_DestCmd->buttons &= ~(BT_WEAPONMASK | BT_SLOTMASK);
		
		// Merge Buttons
		a_DestCmd->buttons |= a_SrcList[j].buttons;
	}

	// Do some math
	xDiv = ((fixed_t)a_NumSrc) << FRACBITS;
	a_DestCmd->forwardmove = FixedDiv(FM << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->sidemove = FixedDiv(SM << FRACBITS, xDiv) >> FRACBITS;
	
	/* Aiming is slightly different */
#if defined(__REMOOD_SWIRVYANGLE)
	// Divide some
	//AT /= ((int32_t)(a_NumSrc));
	
	// Cap
	if (AT > 32000)
		AT = 32000;
	else if (AT < -32000)
		AT = -32000;
	
	// Try now
	a_DestCmd->BaseAngleTurn = FixedDiv(AT << FRACBITS, xDiv) >> FRACBITS;
#else
	// Use furthest angle
	//AT /= ((int32_t)(a_NumSrc));
	a_DestCmd->BaseAngleTurn = AT;
#endif
}

/* D_NCSAllocNetPlayer() -- Allocates a network player */
D_NetPlayer_t* D_NCSAllocNetPlayer(void)
{
	size_t i;
	uint8_t Char;
	D_NetPlayer_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(D_NetPlayer_t), PU_STATIC, NULL);
	
	/* Link */
	if (!l_FirstNetPlayer)
		l_FirstNetPlayer = New;
	else
	{
		l_FirstNetPlayer->ChainPrev = l_FirstNetPlayer;
		New->ChainNext = l_FirstNetPlayer;
		l_FirstNetPlayer = New;
	}
	
	/* Set properties */
	// UUID (hopefully random)
	for (i = 0; i < (MAXPLAYERNAME * 2) - 1; i++)
	{
		// Hopefully random enough
		Char = (((int)(M_Random())) + ((int)I_GetTime() * (int)I_GetTime()));
		
		// Limit Char
		if (!((Char >= '0' && Char <= '9') || (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')))
		{
			i--;
			continue;
		}
		
		// Set as
		New->UUID[i] = Char;
		
		// Sleep for some unknown time
		I_WaitVBL(M_Random() & 1);
	}
	
	/* Return New */
	return New;
}

/* D_NCSFindNetPlayer() -- Finds a net player */
D_NetPlayer_t* D_NCSFindNetPlayer(const char* const a_Name)
{
	D_NetPlayer_t* Rover;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Rove */
	for (Rover = l_FirstNetPlayer; Rover; Rover = Rover->ChainPrev)
	{
		// Match?
		if (strcmp(a_Name, Rover->UUID) == 0)
			return Rover;
			
		// Match?
		if (strcmp(a_Name, Rover->AccountName) == 0)
			return Rover;
	}
	
	/* Not Found */
	return NULL;
}

/* D_NCSGetPlayerName() -- Get player name */
const char* D_NCSGetPlayerName(const uint32_t a_PlayerID)
{
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return NULL;
	
	/* Player is in game */
	if (playeringame[a_PlayerID])
	{
		// Network Player
		if (players[a_PlayerID].NetPlayer)
			if (players[a_PlayerID].NetPlayer->DisplayName[0])
				return players[a_PlayerID].NetPlayer->DisplayName;
		
		// Try from profiles
		if (players[a_PlayerID].ProfileEx)
			if (players[a_PlayerID].ProfileEx->DisplayName[0])
				return players[a_PlayerID].ProfileEx->DisplayName;
	}
	
	/* Return default */
	if (player_names[a_PlayerID][0])
		return player_names[a_PlayerID];
	
	/* Return Unknown */
	return "Unnamed Player";
}

