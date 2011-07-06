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
#include "byteptr.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"
#include "v_video.h"
#include "d_main.h"
#include "p_demcmp.h"
#include "i_sound.h"

// ------
// protos
// ------
void Command_Color_f(void);
void Command_Name_f(void);

void Command_WeaponPref(void);

void Got_NameAndcolor(char **cp, int playernum);
void Got_WeaponPref(char **cp, int playernum);
void Got_Mapcmd(char **cp, int playernum);
void Got_ExitLevelcmd(char **cp, int playernum);
void Got_LoadGamecmd(char **cp, int playernum);
void Got_SaveGamecmd(char **cp, int playernum);
void Got_Pause(char **cp, int playernum);
void Got_UseArtefact(char **cp, int playernum);

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
consvar_t cv_playercolor3 = { "cl3_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t, CL_Player3Change};
consvar_t cv_skin3 = { "cl3_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT, NULL /*skin_cons_t */ , CL_Player3Change};
consvar_t cv_weaponpref3 = { "cl3_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player3Change};
consvar_t cv_autoaim3 = { "cl3_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player3Change };

/*** PLAYER 1 ***/
consvar_t cv_playername4 = { "cl4_name", NULL, CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player4Change };
consvar_t cv_playercolor4 = { "cl4_color", "0", CV_SAVE | CV_CALL | CV_NOINIT, Color_cons_t, CL_Player4Change};
consvar_t cv_skin4 = { "cl4_skin", DEFAULTSKIN, CV_SAVE | CV_CALL | CV_NOINIT, NULL /*skin_cons_t */ , CL_Player4Change};
consvar_t cv_weaponpref4 = { "cl4_weaponpref", "014576328", CV_SAVE | CV_CALL | CV_NOINIT, NULL, CL_Player4Change};
consvar_t cv_autoaim4 = { "cl4_autoaim", "1", CV_SAVE | CV_CALL | CV_NOINIT, CV_OnOff, CL_Player4Change };
/****************************************/

consvar_t cv_originalweaponswitch =
	{ "originalweaponswitch", "0", CV_SAVE, CV_OnOff,
	NULL
};

CV_PossibleValue_t usemouse_cons_t[] = { {0, "Off"}, {1, "On"}, {2, "Force"}, {0, NULL} };

#ifdef LMOUSE2
CV_PossibleValue_t mouse2port_cons_t[] =
	{ {0, "/dev/gpmdata"}, {1, "/dev/ttyS0"}, {2, "/dev/ttyS1"}, {3,
																  "/dev/ttyS2"},
{4, "/dev/ttyS3"}, {0, NULL}
};
#else
CV_PossibleValue_t mouse2port_cons_t[] =
	{ {1, "COM1"}, {2, "COM2"}, {3, "COM3"}, {4, "COM4"}, {0, NULL} };
#endif

#ifdef LJOYSTICK
CV_PossibleValue_t joyport_cons_t[] =
	{ {1, "/dev/js0"}, {2, "/dev/js1"}, {3, "/dev/js2"}, {4, "/dev/js3"}, {0,
																		   NULL}
};
#endif

#ifdef _WIN32
#define usejoystick_cons_t  NULL	// accept whatever value
										// it is in fact the joystick device number
#else
#define usejoystick_cons_t  NULL
//#error "cv_usejoystick don't have possible value for this OS !"
#endif

consvar_t cv_usemouse = { "use_mouse", "1", CV_SAVE | CV_CALL, usemouse_cons_t, I_StartupMouse };
consvar_t cv_usemouse2 = { "use_mouse2", "0", CV_SAVE | CV_CALL, usemouse_cons_t, I_StartupMouse2 };
consvar_t cv_usejoystick = { "use_joystick", "0", CV_SAVE | CV_CALL, usejoystick_cons_t,
	I_InitJoystick
};

#ifdef LJOYSTICK
extern void I_JoyScale();
consvar_t cv_joyport = { "joyport", "/dev/js0", CV_SAVE, joyport_cons_t };
consvar_t cv_joyscale = { "joyscale", "0", CV_SAVE | CV_CALL, NULL, I_JoyScale };
#endif
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
consvar_t cv_allowexitlevel = { "allowexitlevel", "1", CV_NETVAR, CV_YesNo, NULL };

consvar_t cv_netstat = { "netstat", "0", 0, CV_OnOff };

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

	COM_AddCommand("turbo", Command_Turbo_f);	// turbo speed
	COM_AddCommand("version", Command_Version_f);
	COM_AddCommand("quit", Command_Quit_f);

	COM_AddCommand("chatmacro", Command_Chatmacro_f);	// hu_stuff.c
	
	COM_AddCommand("setcontrol1", Command_Setcontrol_f);
	COM_AddCommand("setcontrol2", Command_Setcontrol_f);
	COM_AddCommand("setcontrol3", Command_Setcontrol_f);
	COM_AddCommand("setcontrol4", Command_Setcontrol_f);

	COM_AddCommand("frags", Command_Frags_f);
	COM_AddCommand("teamfrags", Command_TeamFrags_f);

	COM_AddCommand("saveconfig", Command_SaveConfig_f);
	COM_AddCommand("loadconfig", Command_LoadConfig_f);
	COM_AddCommand("changeconfig", Command_ChangeConfig_f);
	COM_AddCommand("screenshot", M_ScreenShot);

	COM_AddCommand("kill", Command_Kill);
	
	/* GhostlyDeath <June 6, 2008> -- Register demo compatibility variables */
	DC_RegisterDemoCompVars();

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

	//FIXME: not to be here.. but needs be done for config loading
	CV_RegisterVar(&cv_usegamma);

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
	CV_RegisterVar(&cv_usemouse2);
	CV_RegisterVar(&cv_invertmouse2);
	CV_RegisterVar(&cv_alwaysfreelook2);
	CV_RegisterVar(&cv_mousemove2);
	CV_RegisterVar(&cv_mousesens2);
	CV_RegisterVar(&cv_mlooksens2);
	CV_RegisterVar(&cv_joystickfreelook);

	// WARNING : the order is important when inititing mouse2 
	//           we need the mouse2port
	CV_RegisterVar(&cv_mouse2port);
#ifdef LMOUSE2
	CV_RegisterVar(&cv_mouse2opt);
#endif
	CV_RegisterVar(&cv_mousesens);
	CV_RegisterVar(&cv_mousesensy);
	CV_RegisterVar(&cv_mlooksens);
	CV_RegisterVar(&cv_controlperkey);
	CV_RegisterVar(&cv_legacymouse);
	CV_RegisterVar(&cv_m_legacymouse);
	CV_RegisterVar(&cv_m_classicalt);
	CV_RegisterVar(&cv_m_xsensitivity);
	CV_RegisterVar(&cv_m_ysensitivity);
	CV_RegisterVar(&cv_m_xaxismode);
	CV_RegisterVar(&cv_m_yaxismode);
	CV_RegisterVar(&cv_m_xaxissecmode);
	CV_RegisterVar(&cv_m_yaxissecmode);

	CV_RegisterVar(&cv_usemouse);
	CV_RegisterVar(&cv_usejoystick);
#ifdef LJOYSTICK
	CV_RegisterVar(&cv_joyport);
	CV_RegisterVar(&cv_joyscale);
#endif
	CV_RegisterVar(&cv_allowjump);
	CV_RegisterVar(&cv_allowrocketjump);
	CV_RegisterVar(&cv_allowautoaim);
	CV_RegisterVar(&cv_forceautoaim);
	CV_RegisterVar(&cv_allowturbo);
	CV_RegisterVar(&cv_allowexitlevel);

	//s_sound.c
	CV_RegisterVar(&cv_soundvolume);
	CV_RegisterVar(&cv_musicvolume);
	CV_RegisterVar(&cv_numChannels);
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
#if defined (LINUX) && !defined (SDL)
	CV_RegisterVar(&cv_jigglecdvol);
#endif

	// screen.c ?
	CV_RegisterVar(&cv_fullscreen);	// only for opengl so use differant name please and move it to differant place
	CV_RegisterVar(&cv_scr_depth);
	CV_RegisterVar(&cv_scr_width);
	CV_RegisterVar(&cv_scr_height);
	CV_RegisterVar(&cv_fragsweaponfalling);
	CV_RegisterVar(&cv_classicblood);
	CV_RegisterVar(&cv_classicrocketblast);
	CV_RegisterVar(&cv_classicmeleerange);
	CV_RegisterVar(&cv_classicmonsterlogic);

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

void CL_DynPlayerChange(player_t* Player,
	consvar_t* name,
	consvar_t* color,
	consvar_t* skin,
	consvar_t* weaponpref,
	consvar_t* autoaim);

void CL_Player1Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[0]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[0]],
		&cv_playername1,
		&cv_playercolor1,
		&cv_skin1,
		&cv_weaponpref1,
		&cv_autoaim1);
}

void CL_Player2Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[1]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[1]],
		&cv_playername2,
		&cv_playercolor2,
		&cv_skin2,
		&cv_weaponpref2,
		&cv_autoaim2);
}

void CL_Player3Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[2]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[2]],
		&cv_playername3,
		&cv_playercolor3,
		&cv_skin3,
		&cv_weaponpref3,
		&cv_autoaim3);
}

void CL_Player4Change(void)
{
	if (demoplayback)
		return;
		
	if (!playeringame[consoleplayer[3]])
		return;
		
	CL_DynPlayerChange(&players[consoleplayer[3]],
		&cv_playername4,
		&cv_playercolor4,
		&cv_skin4,
		&cv_weaponpref4,
		&cv_autoaim4);
}

void CL_DynPlayerChange(player_t* Player,
	consvar_t* name,
	consvar_t* color,
	consvar_t* skin,
	consvar_t* weaponpref,
	consvar_t* autoaim)
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

void Got_NameAndcolor(char **cp, int playernum)
{
	player_t *p = &players[playernum];

	// color
	p->skincolor = READBYTE(*cp) % MAXSKINCOLORS;

	// a copy of color
	if (p->mo)
		p->mo->flags = (p->mo->flags & ~MF_TRANSLATION) | ((p->skincolor) << MF_TRANSSHIFT);

	// name
	if (demoversion >= 128)
	{
		if (stricmp(player_names[playernum], *cp))
			CONS_Printf("%s renamed to %s\n", player_names[playernum], *cp);
		READSTRING(*cp, player_names[playernum]);
	}
	else
	{
		memcpy(player_names[playernum], *cp, MAXPLAYERNAME);
		*cp += MAXPLAYERNAME;
	}

	// skin
	if (demoversion < 120 || demoversion >= 125)
	{
		if (demoversion >= 128)
		{
			SetPlayerSkin(playernum, *cp);
			SKIPSTRING(*cp);
		}
		else
		{
			SetPlayerSkin(playernum, *cp);
			*cp += (SKINNAMESIZE + 1);
		}
	}
}

void SendWeaponPref(void)
{
}

void Got_WeaponPref(char **cp, int playernum)
{
	players[playernum].originalweaponswitch = *(*cp)++;
	memcpy(players[playernum].favoritweapon, *cp, NUMWEAPONS);
	*cp += NUMWEAPONS;
	players[playernum].autoaim_toggle = *(*cp)++;
}

void D_SendPlayerConfig(void)
{
}

// ========================================================================

//  play a demo, add .lmp for external demos
//  eg: playdemo demo1 plays the internal game demo
//
// byte*   demofile;       //demo file buffer

void Command_Playdemo_f(void)
{
	char name[256];

	if (COM_Argc() != 2)
	{
		CONS_Printf("playdemo <demoname> : playback a demo\n");
		return;
	}

	// disconnect from server here ?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONS_Printf("\nYou can't play a demo while in net game\n");
		return;
	}

	// open the demo file
	strcpy(name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played
	//FIL_DefaultExtension (name, ".lmp");

	CONS_Printf("Playing back demo '%s'.\n", name);

	G_DoPlayDemo(name);
}

void Command_Timedemo_f(void)
{
	char name[256];

	if (COM_Argc() != 2)
	{
		CONS_Printf("timedemo <demoname> : time a demo\n");
		return;
	}

	// disconnect from server here ?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONS_Printf("\nYou can't play a demo while in net game\n");
		return;
	}

	// open the demo file
	strcpy(name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played
	//FIL_DefaultExtension (name, ".lmp");

	CONS_Printf("Timing demo '%s'.\n", name);

	G_TimeDemo(name);
}

//  stop current demo
//
void Command_Stopdemo_f(void)
{
	G_CheckDemoStatus();
	CONS_Printf("Stopped demo.\n");
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
		CONS_Printf
			("map <mapname> [-skill <1..5>] [-monsters <0/1>] [-noresetplayers]: warp to map\n");
		return;
	}

	strncpy(MAPNAME, COM_Argv(1), MAX_WADPATH);

	// internal wad lump
	if (W_CheckNumForName(MAPNAME) == -1)
	{
		CONS_Printf("\2Internal game map '%s' not found\n"
					"(use .wad extension for external maps)\n", MAPNAME);
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
	if (!cv_splitscreen.value)
		multiplayer = false;
	else
		multiplayer = true;
	localgame = true;
	G_StopDemo();
	
	//M_UnLockGameCVARS();
	DC_SetMenuGameOptions(0);

	gamestate = wipegamestate = GS_NULL;
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
	{
		consoleplayer[i] = i;
		displayplayer[i] = i;
	}
	for (i = 0; i < MAXPLAYERS; i++)
		playeringame[i] = 0;
		
	for (i = 0; i < cv_splitscreen.value + 1; i++)
		playeringame[i] = 1;
	

	precache = false;
	G_InitNew(skill, MAPNAME, 1);
	precache = false;
}

void Got_Mapcmd(char **cp, int playernum)
{
	char mapname[MAX_WADPATH];
	int skill, resetplayer = 1;

	skill = READBYTE(*cp);
	if (demoversion >= 128)
		nomonsters = READBYTE(*cp);

	if (demoversion >= 129)
	{
		resetplayer = ((nomonsters & 2) == 0);
		nomonsters &= 1;
	}
	strcpy(mapname, *cp);
	*cp += strlen(mapname) + 1;

	CONS_Printf("Warping to map...\n");
	if (demoplayback && !timingdemo)
		precache = false;
	else
		precache = false;
	G_InitNew(skill, mapname, resetplayer);
	if (demoplayback && !timingdemo)
		precache = false;		// GhostlyDeath -- was true
	else
		precache = false;		// GhostlyDeath -- was true
	CON_ToggleOff();
	if (timingdemo)
		G_DoneLevelLoad();
}

void Command_Restart_f(void)
{
	if (netgame)
	{
		CONS_Printf("Restartlevel don't work in network\n");
		return;
	}

	/* Demyx -- Don't allow restarting in demo's */
	if (!demoplayback && gamestate == GS_LEVEL)
		G_DoLoadLevel(true);
	else
		CONS_Printf("You should be in a level to restart it !\n");
}

void Command_Pause(void)
{
	paused = ~paused;
	
	/* Demyx -- Don't allow pausing anywhere but the level and intermission.*/
	if (paused && (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
		I_PauseSong(0);
	else
		I_ResumeSong(0);
}

void Got_Pause(char **cp, int playernum)
{
	if (demoversion < 131)
		paused ^= 1;
	else
		paused = READBYTE(*cp);
	if (!demoplayback)
	{
		if (netgame)
		{
			if (paused)
				CONS_Printf("Game paused by %s\n", player_names[playernum]);
			else
				CONS_Printf("Game unpaused by %s\n", player_names[playernum]);
		}

		if (paused)
		{
			if (!M_ActiveMenu() || netgame)
				S_PauseSound();
		}
		else
			S_ResumeSound();
	}
}

//  Add a pwad at run-time
//  Search for sounds, maps, musics, etc..
//
void Command_Addfile(void)
{
	if (COM_Argc() != 2)
	{
		CONS_Printf("addfile <wadfile.wad> : load wad file\n");
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
		CONS_Printf("Frags : show the frag table\n");
		CONS_Printf("Only for deathmatch games\n");
		return;
	}

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			CONS_Printf("%-16s", player_names[i]);
			for (j = 0; j < MAXPLAYERS; j++)
				if (playeringame[j])
					CONS_Printf(" %3d", players[i].frags[j]);
			CONS_Printf("\n");
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
		CONS_Printf("teamfrags : show the frag table for teams\n");
		CONS_Printf("Only for deathmatch teamplay games\n");
		return;
	}

	HU_CreateTeamFragTbl(unused, frags, fragtbl);

	for (i = 0; i < 11; i++)
		if (teamingame(i))
		{
			CONS_Printf("%-8s", team_names[i]);
			for (j = 0; j < 11; j++)
				if (teamingame(j))
					CONS_Printf(" %3d", fragtbl[i][j]);
			CONS_Printf("\n");
		}
}

//  Returns program version.
//
void Command_Version_f(void)
{
	CONS_Printf("ReMooD %s \"%s\" (Compiled: %s %s)\n", 
		REMOOD_VERSIONSTRING, REMOOD_VERSIONCODESTRING, __DATE__, __TIME__);
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

ULONG timelimitintics = 0;

void TimeLimit_OnChange(void)
{
	if (cv_timelimit.value)
	{
		CONS_Printf("Levels will end after %d minute(s).\n", cv_timelimit.value);
		timelimitintics = cv_timelimit.value * 60 * TICRATE;
	}
	else
		CONS_Printf("Time limit disabled\n");
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
		CONS_Printf("You should be in a level to exit it !\n");
	else
		G_ExitLevel();
}

void Got_ExitLevelcmd(char **cp, int playernum)
{
	G_ExitLevel();
}

void Command_Load_f(void)
{
}

void Got_LoadGamecmd(char **cp, int playernum)
{
	byte slot = *(*cp)++;
	G_DoLoadGame(slot);
}

void Command_Save_f(void)
{
	 if (COM_Argc() != 3)
	 {
		 CONS_Printf("save <slot> <desciption>: save game\n");
		 return;
	 }
	 
	 G_DoSaveGame(atoi(COM_Argv(1)), COM_Argv(2));
}

void Got_SaveGamecmd(char **cp, int playernum)
{
	byte slot;
	char description[SAVESTRINGSIZE];

	slot = *(*cp)++;
	strcpy(description, *cp);
	*cp += strlen(description) + 1;

	G_DoSaveGame(slot, description);
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
		CONS_Printf("You can't exit a game if you aren't even in one !\n");
}

void Command_Kill(void)
{
	if (!demoplayback && gamestate == GS_LEVEL)
		P_KillMobj(players[consoleplayer[0]].mo, NULL, players[consoleplayer[0]].mo);
	else
		CONS_Printf("The Kill command cannot be used outside a game.\n");
}
