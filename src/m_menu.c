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
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
// NOTE:
//      All V_DrawPatchDirect () has been replaced by V_DrawScaledPatch ()
//      so that the menu is scaled to the screen size. The scaling is always
//      an integer multiple of the original size, so that the graphics look
//      good.

#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>

#include "am_map.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_sound.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"
#include "t_script.h"

#include "d_net.h"
#include "p_inter.h"
#include "d_prof.h"

bool_t localgame;

// -1 = no quicksave slot picked!
int quickSaveSlot;
bool_t menuactive;

// we are going to be entering a savegame string
int saveStringEnter;
int saveSlot;					// which slot to save in
int saveCharIndex;				// which char we're editing
char saveOldString[SAVESTRINGSIZE];	// old save description before edit

char savegamestrings[10][SAVESTRINGSIZE];

// current menudef
menu_t* currentMenu;
short itemOn;					// menu item skull is on
short skullAnimCounter;			// skull animation counter
short whichSkull;				// which skull to draw
int SkullBaseLump;

// graphic name of skulls
char skullName[2][9] = { "M_SKULL1", "M_SKULL2" };

const char* ALLREADYPLAYING = "You are already playing\n\nLeave this game first\n";

menu_t MainDef,					// Main Menu
       OptionsDef,					// Options Menu
       DefaultKeyBindDef,				// Key Binds
       GameOptionsDef,				// Game Options
       NewGameDef,					// New Game
       NewGameClassicDef,				// New Game -> Classic
       SoundsDef,						// Sound Options
       VideoDef,						// Video Options
       CreateLocalGameDef,			// New Game -> Local
       NewGameOptionsDef,				// New Game -> Options
       NewGameCCSkillDef, NewGameCCEpiDef, ControlSettingsDef, GraphicalSettingsDef, LASTMENU;

menu_t* MenuPtrList[] =
{
	&MainDef,
	&OptionsDef,
	&DefaultKeyBindDef,
	&GameOptionsDef,
	&NewGameDef,
	&NewGameClassicDef,
	&ProfileDef,
	&SoundsDef,
	&VideoDef,
	&CreateLocalGameDef,
	&NewGameOptionsDef,
	&NewGameCCSkillDef,
	&NewGameCCEpiDef,
	
	&ControlSettingsDef,
	&GraphicalSettingsDef,
	
	NULL,						// To determine the end
};

// =============================================================================
//                                MAIN MENU
// =============================================================================

menuitem_t MainItems[] =
{
	{ITX_MAINMENUITEM | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_NEWGAME), &NewGameDef},
	{ITX_MAINMENUITEM | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_ENDGAME), M_EndGame},
	{ITX_MAINMENUITEM | IT_DISABLED2, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_LOADGAME), NULL},
	{ITX_MAINMENUITEM | IT_DISABLED2, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_SAVEGAME), NULL},
	{ITX_MAINMENUITEM | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_OPTIONS), &OptionsDef},
	{ITX_MAINMENUITEM | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_PROFILES), M_StartProfiler},
	{ITX_MAINMENUITEM | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUMAIN_QUITGAME), M_QuitDOOM},
};

menu_t MainDef =
{
	MENUFLAG_OPTIMALSPACE,
	"*",
	NULL,
	sizeof(MainItems) / sizeof(menuitem_t),
	MainItems,
	NULL,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1,
};

// =============================================================================
//                              OPTIONS MENU
// =============================================================================

menuitem_t OptionsItems[] =
{
	//{ITX_SUBMENUTITLE, NULL,          "*** GAME ***", NULL},
	{IT_STRING | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUOPTIONS_GAMESETTINGS), &GameOptionsDef}
	,
	{IT_STRING | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUOPTIONS_CONTROLSETTINGS), &ControlSettingsDef}
	,
	{IT_STRING | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUOPTIONS_GRAPHICALSETTINGS), &GraphicalSettingsDef}
	,
	{IT_STRING | IT_SUBMENU, NULL, PTROFUNICODESTRING(DSTR_MENUOPTIONS_AUDIOSETTINGS), &SoundsDef}
	,
	//{IT_STRING | IT_SUBMENU, NULL,    PTROFUNICODESTRING(DSTR_MENUOPTIONS_ADVANCEDSETTINGS), &AdvancedDef},   // Change if you dare
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUOPTIONS_DISABLETITLESCREENDEMOS), &cv_disabledemos, 0}
	,
	
	/*{ITX_SUBMENUTITLE, NULL,          "*** GAME ***", NULL},
	   {IT_STRING | IT_SUBMENU, NULL,       "Options and Flags...", &GameOptionsDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Heads up Display...", &HeadsUpDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Graphics...", &GraphicsDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Console...", &ConsoleDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Mouse...", &MouseOptionsDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Joysticks...", &JoystickOptionsDef},
	   {IT_STRING | IT_SUBMENU, NULL,       "Miscellaneous...", &MiscOptionsDef},
	   {ITX_SUBMENUTITLE, NULL,         "*** SYSTEM ***", NULL},
	   {IT_STRING | IT_CALL, NULL,          "Video...", &M_StartVideoOptions},
	   {IT_STRING | IT_SUBMENU, NULL,       "Sound...", &SoundsDef},
	   {ITX_SUBMENUTITLE, NULL,         "*** PLAYERS ***", NULL},
	   {IT_STRING | IT_CALL, NULL,          "Default Key Binds (Player 1)...", M_ControlsDoPlayer1},
	   {IT_STRING | IT_CALL, NULL,          "Default Key Binds (Player 2)...", M_ControlsDoPlayer2},
	   {IT_STRING | IT_CALL, NULL,          "Default Key Binds (Player 3)...", M_ControlsDoPlayer3},
	   {IT_STRING | IT_CALL, NULL,          "Default Key Binds (Player 4)...", M_ControlsDoPlayer4}, */
};

menu_t OptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTIONS",
	PTROFUNICODESTRING(DSTR_MENUOPTIONS_TITLE),
	sizeof(OptionsItems) / sizeof(menuitem_t),
	OptionsItems,
	&MainDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1,
};

// =============================================================================
//                               CONTROL SETTINGS
// =============================================================================

menuitem_t ControlSettingsItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_CONTROLSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_ACTIONSPERKEY), &cv_controlperkey, 0}
	,
	
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_PLAYERONECONTROLS), M_ControlsDoPlayer1}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_PLAYERTWOCONTROLS), M_ControlsDoPlayer2}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_PLAYERTHREECONTROLS), M_ControlsDoPlayer3}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_PLAYERFOURCONTROLS), M_ControlsDoPlayer4}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUCONTROLS_MOUSESUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_ENABLEMOUSE), &cv_use_mouse, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_ADVANCEDMOUSE), &cv_m_legacymouse, 0}
	,
	
	{IT_STRING | IT_SPACE | IT_WHITESTRING, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_BASICSETTINGSSUBSUBTITLE)}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_USEMOUSEFREELOOK), &cv_alwaysfreelook, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_USEMOUSEMOVE), &cv_mousemove, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_INVERTYAXIS), &cv_invertmouse, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_MOVETURNSENS), &cv_m_xsensitivity, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_LOOKUPDOWNSENS), &cv_mlooksens, 0}
	,
	
	{IT_STRING | IT_SPACE | IT_WHITESTRING, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_ADVANCEDSETTINGSUBSUBTITLE)}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_XAXISSENS), &cv_m_xsensitivity, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_YAXISSENS), &cv_m_ysensitivity, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_XAXISMOVE), &cv_m_xaxismode, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_YAXISMOVE), &cv_m_yaxismode, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_XAXISMOVESECONDARY), &cv_m_xaxissecmode, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_YAXISMOVESECONDARY), &cv_m_yaxissecmode, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCONTROLS_STRAFEKEYUSESECONDARY), &cv_m_classicalt, 0}
	,
};

menu_t ControlSettingsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_CONTRO",
	PTROFUNICODESTRING(DSTR_MENUCONTROLS_TITLE),
	sizeof(ControlSettingsItems) / sizeof(menuitem_t),
	ControlSettingsItems,
	&OptionsDef,
	M_DrawControl,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                             GRAPHICAL SETTINGS
// =============================================================================

menuitem_t GraphicalSettingsItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_SCREENSUBTITLE), NULL}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_SETRESOLUTION), &M_StartVideoOptions}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_FULLSCREEN), &cv_fullscreen, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_BRIGHTNESS), &cv_usegamma, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_SCREENSIZE), &cv_viewsize, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_SCREENLINK), &cv_screenslink, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_RENDERERSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_TRANSLUCENCY), &cv_translucency, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_ENABLEDECALS), &cv_splats, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_MAXDECALS), &cv_maxsplats, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_CONSOLESUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_CONSOLESPEED), &cons_speed, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_CONSOLEHEIGHT), &cons_height, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_CONSOLEBACKGROUND), &cons_backpic, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_MESSAGEDURATION), &cons_msgtimeout, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_ECHOMESSAGES), &cv_showmessages, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_MENUSUBTITLE), NULL}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_HUDSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_SCALESTATUSBAR), &cv_scalestatusbar, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_TRANSPARENTSTATUSBAR), &cv_transparentstatusbar, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_STATUSBARTRANSPARENCYAMOUNT), &cv_transparentstatusbarmode, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUGRAPHICS_CROSSHAIR), &cv_crosshair, 0}
	,
};

menu_t GraphicalSettingsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUGRAPHICS_TITLE),
	sizeof(GraphicalSettingsItems) / sizeof(menuitem_t),
	GraphicalSettingsItems,
	&OptionsDef,
	M_DrawControl,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                           DEFAULT KEY BINDS MENU
// =============================================================================

menuitem_t DefaultKeyBindItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_MOVEMENTSUBTITLE), NULL}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_FIRE), M_ChangeControl, gc_fire}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_ACTIVATE), M_ChangeControl, gc_use}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_MOVEFORWARDS), M_ChangeControl, gc_forward}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_MOVEBACKWARDS), M_ChangeControl, gc_backward}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_TURNLEFT), M_ChangeControl, gc_turnleft}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_TURNRIGHT), M_ChangeControl, gc_turnright}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_RUN), M_ChangeControl, gc_speed}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_STRAFEON), M_ChangeControl, gc_strafe}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_STRAFELEFT), M_ChangeControl, gc_strafeleft}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_STRAFERIGHT), M_ChangeControl, gc_straferight}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_LOOKUP), M_ChangeControl, gc_lookup}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_LOOKDOWN), M_ChangeControl, gc_lookdown}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_CENTERVIEW), M_ChangeControl, gc_centerview}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_MOUSELOOK), M_ChangeControl, gc_mouseaiming}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_JUMPFLYUP), M_ChangeControl, gc_jump}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_FLYDOWN), M_ChangeControl, gc_flydown}
	,
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_WEAPONSANDITEMSSUBTITLE), NULL}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTONE), M_ChangeControl, gc_weapon1}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTTWO), M_ChangeControl, gc_weapon2}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTTHREE), M_ChangeControl, gc_weapon3}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTFOUR), M_ChangeControl, gc_weapon4}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTFIVE), M_ChangeControl, gc_weapon5}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTSIX), M_ChangeControl, gc_weapon6}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTSEVEN), M_ChangeControl, gc_weapon7}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_SLOTEIGHT), M_ChangeControl, gc_weapon8}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_PREVIOUSWEAPON), M_ChangeControl, gc_prevweapon}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_NEXTWEAPON), M_ChangeControl, gc_nextweapon}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_BESTWEAPON), M_ChangeControl, gc_bestweapon}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_INVENTORYLEFT), M_ChangeControl, gc_invprev}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_INVENTORYRIGHT), M_ChangeControl, gc_invnext}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_INVENTORYUSE), M_ChangeControl, gc_invuse}
	,
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_MISCSUBTITLE), NULL}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_TALKKEY), M_ChangeControl, gc_talkkey}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_RANKINGSANDSCORES), M_ChangeControl, gc_scores}
	,
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(DSTR_MENUKEYBINDS_TOGGLECONSOLE), M_ChangeControl, gc_console}
	,
};

menu_t DefaultKeyBindDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_CONTRO",
	PTROFUNICODESTRING(DSTR_MENUKEYBINDS_TITLE),
	sizeof(DefaultKeyBindItems) / sizeof(menuitem_t),
	DefaultKeyBindItems,
	&ControlSettingsDef,
	M_DrawControl,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                               SOUND OPTIONS MENU
// =============================================================================

menuitem_t SoundOptionsItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUAUDIO_OUTPUTSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SOUNDOUTPUT), &cv_snd_output, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_STRING, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SOUNDDEVICE), &cv_snd_device, 0}
	,
	//{IT_STRING | IT_CVAR, 0,              PTROFUNICODESTRING(DSTR_MENUAUDIO_MUSICOUTPUT), &cv_, 0},
	//{IT_STRING | IT_CVAR | IT_CV_STRING, 0,   PTROFUNICODESTRING(DSTR_MENUAUDIO_MUSICDEVICE), &cv_, 0},
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUAUDIO_QUALITYSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SPEAKERSETUP), &cv_snd_speakersetup, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SAMPLESPERSECOND), &cv_snd_soundquality, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_BITSPERSAMPLE), &cv_snd_sounddensity, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_FAKEPCSPEAKERWAVEFORM), &cv_snd_pcspeakerwave, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUAUDIO_VOLUMESUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SOUNDVOLUME), &cv_soundvolume, 0}
	,
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_MUSICVOLUME), &cv_musicvolume, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUAUDIO_MISCSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_PRECACHESOUNDS), &precachesound}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_RANDOMSOUNDPITCH), &cv_rndsoundpitch, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_SOUNDCHANNELS), &cv_snd_channels, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_RESERVEDSOUNDCHANNELS), &cv_snd_reservedchannels, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_MULTITHREADEDSOUND), &cv_snd_multithreaded, 0}
	,
	//{IT_STRING | IT_CVAR, 0,              PTROFUNICODESTRING(DSTR_MENUAUDIO_MULTITRHEADEDMUSIC), &cv_mus_multithreaded, 0},
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUAUDIO_RESETSUBTITLE), NULL}
	,
	{IT_STRING | IT_CALL, 0, PTROFUNICODESTRING(DSTR_MENUAUDIO_RESETSOUND), M_ResetSound}
	,
	//{IT_STRING | IT_CALL, 0,              PTROFUNICODESTRING(DSTR_MENUAUDIO_RESETMUSIC)},
	
	/*{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "Sound Volume", &cv_soundvolume, 0},
	   {IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "Music Volume", &cv_musicvolume, 0},
	   {IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "CD Volume", &cd_volume, 0},
	   {IT_STRING | IT_CVAR, 0, "Channel Count", &cv_numChannels, 0},
	   {IT_STRING | IT_CVAR, 0, "Reverse Stereo", &stereoreverse, 0}, */
	
};

menu_t SoundsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUAUDIO_TITLE),
	sizeof(SoundOptionsItems) / sizeof(menuitem_t),
	SoundOptionsItems,
	&OptionsDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

menuitem_t VideoOptionsItems[] =
{
	{IT_WHITESTRING | IT_KEYHANDLER, 0, PTROFUNICODESTRING(DSTR_MENUVIDEO_MODESELECT), &M_HandleVideoKey, 0}
	,
};

menu_t VideoDef =
{
	MENUFLAG_OPTIMALSPACE /* | MENUFLAG_HIDECURSOR */ ,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUVIDEO_TITLE),
	sizeof(VideoOptionsItems) / sizeof(menuitem_t),
	VideoOptionsItems,
	&GraphicalSettingsDef,
	M_DrawVideoOptions,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                              GAME OPTIONS MENU
// =============================================================================

menuitem_t GameOptionsItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_MULTIPLAYERSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_DEATHMATCHTYPE), &cv_deathmatch, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FRAGLIMIT), &cv_fraglimit, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_TIMELIMIT), &cv_timelimit, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_TEAMSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLETEAMPLAY), &cv_teamplay, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FRIENDLYFIRE), &cv_teamdamage, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_RESTRICTIONSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWJUMP), &cv_allowjump, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWROCKETJUMP), &cv_allowrocketjump, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWAUTOAIM), &cv_allowautoaim, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWTURBO), &cv_allowturbo, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWEXITLEVEL), &cv_allowexitlevel, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FORCEAUTOAIM), &cv_forceautoaim, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_WEAPONSANDITEMSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLEITEMRESPAWN), &cv_itemrespawn, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ITEMRESPAWNTIME), &cv_itemrespawntime, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_DROPWEAPONSWHENYOUDIE), &cv_fragsweaponfalling, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_INFINITEAMMO), &cv_infiniteammo, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_MONSTERSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_SPAWNMONSTERS), &cv_spawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLEMONSTERRESPAWN), &cv_respawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_MONSTERRESPAWNTIME), &cv_respawnmonsterstime, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FASTMONSTERS), &cv_fastmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_PREDICTINGMONSTERS), &cv_predictingmonsters, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_MISCSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_GRAVITY), &cv_gravity, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_SOLIDCORPSES), &cv_solidcorpse, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_BLOODTIME), &cv_bloodtime, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_COMPATIBILITYSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICBLOOD), &cv_classicblood, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICROCKETEXPLOSIONS), &cv_classicrocketblast, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICMONSTERMELEERANGE), &cv_classicmeleerange, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICMONSTERLOGIC), &cv_classicmonsterlogic, 0}
	,
};

menu_t GameOptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUGAME_TITLE),
	sizeof(GameOptionsItems) / sizeof(menuitem_t),
	GameOptionsItems,
	&OptionsDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                                NEW GAME MENU
// =============================================================================

menuitem_t NewGameOptionsItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_TEAMSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLETEAMPLAY), &cv_ng_teamplay, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FRIENDLYFIRE), &cv_ng_teamdamage, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_RESTRICTIONSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWJUMP), &cv_ng_allowjump, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWROCKETJUMP), &cv_ng_allowrocketjump, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWAUTOAIM), &cv_ng_allowautoaim, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWTURBO), &cv_ng_allowturbo, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ALLOWEXITLEVEL), &cv_ng_allowexitlevel, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FORCEAUTOAIM), &cv_ng_forceautoaim, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_WEAPONSANDITEMSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLEITEMRESPAWN), &cv_ng_itemrespawn, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ITEMRESPAWNTIME), &cv_ng_itemrespawntime, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_DROPWEAPONSWHENYOUDIE), &cv_ng_fragsweaponfalling, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_INFINITEAMMO), &cv_ng_infiniteammo, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_MONSTERSSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_ENABLEMONSTERRESPAWN), &cv_ng_respawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_MONSTERRESPAWNTIME), &cv_ng_respawnmonsterstime, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_FASTMONSTERS), &cv_ng_fastmonsters, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_PREDICTINGMONSTERS), &cv_ng_predictingmonsters, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_MISCSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_GRAVITY), &cv_ng_gravity, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_SOLIDCORPSES), &cv_ng_solidcorpse, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_BLOODTIME), &cv_ng_bloodtime, 0}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUGAME_COMPATIBILITYSUBTITLE), NULL}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICBLOOD), &cv_ng_classicblood, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICROCKETEXPLOSIONS), &cv_ng_classicrocketblast, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICMONSTERMELEERANGE), &cv_ng_classicmeleerange, 0}
	,
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(DSTR_MENUGAME_CLASSICMONSTERLOGIC), &cv_ng_classicmonsterlogic, 0}
};

menu_t NewGameOptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUGAME_TITLE),
	sizeof(NewGameOptionsItems) / sizeof(menuitem_t),
	NewGameOptionsItems,
	&NewGameDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

menuitem_t NewGameItems[] =
{
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_SINGLEPLAYERSUBTITLE), NULL}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_CLASSIC), M_DoNewGameClassicClassic}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_CREATEGAME), M_DoNewGameClassic}
	,
	{IT_STRING | IT_DISABLED2, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_QUICKSTART), NULL}
	,
	
	{ITX_SUBMENUTITLE, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_MULTIPLAYERSUBTITLE), NULL}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_SPLITSCREENGAME), M_DoNewGameLocal}
	,
	{IT_STRING | IT_DISABLED2, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_UDPLANINTERNETGAME), NULL}
	,
	{IT_STRING | IT_DISABLED2, NULL, PTROFUNICODESTRING(DSTR_MENUNEWGAME_FORKGAME), NULL}
	,
};

menu_t NewGameDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(DSTR_MENUNEWGAME_TITLE),
	sizeof(NewGameItems) / sizeof(menuitem_t),
	NewGameItems,
	&MainDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                         CREATE GAME (Single) MENU
// =============================================================================

menuitem_t NewGameCCSkillItems[] =
{
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMSKILLA), M_SelectSkill}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMSKILLB), M_SelectSkill}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMSKILLC), M_SelectSkill}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMSKILLD), M_SelectSkill}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMSKILLE), M_SelectSkill}
	,
};

menuitem_t NewGameCCEpisodeItems[] =
{
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODEA), M_SelectEpisode}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODEB), M_SelectEpisode}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODEC), M_SelectEpisode}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODED), M_SelectEpisode}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODEE), M_SelectEpisode}
	,
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_DOOMEPISODEF), M_SelectEpisode}
	,
};

menu_t NewGameCCSkillDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_TITLE),
	sizeof(NewGameCCSkillItems) / sizeof(menuitem_t),
	NewGameCCSkillItems,
	&NewGameDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

menu_t NewGameCCEpiDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(DSTR_MENUCLASSICGAME_TITLE),
	sizeof(NewGameCCEpisodeItems) / sizeof(menuitem_t),
	NewGameCCEpisodeItems,
	&NewGameDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

menuitem_t NewGameClassicItems[] =
{
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_LEVEL), &cv_ng_map}
	,
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SKILL), &cv_ng_skill}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_OPTIONS), &cv_ng_options}
	,
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SETUPOPTIONS), M_ClassicGameOptions}
	,
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL}
	,
	{IT_WHITESTRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_STARTGAME), M_StartClassicGame}
	,
};

menu_t NewGameClassicDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SOLOTITLE),
	sizeof(NewGameClassicItems) / sizeof(menuitem_t),
	NewGameClassicItems,
	&NewGameDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

// =============================================================================
//                         CREATE GAME (Local) MENU
// =============================================================================

menuitem_t CreatLocalGameItems[] =
{
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_NUMBEROFPLAYERS), &cv_ng_splitscreen}
	,
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_LEVEL), &cv_ng_map}
	,
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SKILL), &cv_ng_skill}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_DEATHMATCHTYPE), &cv_ng_deathmatch, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_FRAGLIMIT), &cv_ng_fraglimit, 0}
	,
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_TIMELIMIT), &cv_ng_timelimit, 0}
	,
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_OPTIONS), &cv_ng_options}
	,
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL}
	,
	{IT_STRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_SETUPOPTIONS), M_LocalGameOptions}
	,
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL}
	,
	{IT_WHITESTRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEGAME_STARTGAME), M_StartLocalGame}
	,
};

menu_t CreateLocalGameDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(DSTR_MENUCREATEGAME_LOCALTITLE),
	sizeof(CreatLocalGameItems) / sizeof(menuitem_t),
	CreatLocalGameItems,
	&NewGameDef,
	M_DrawGenericMenu,
	0,
	0,
	0,
	0,
	1
};

/* M_CreateID() -- Creates a menu ID */
char* M_CreateID(char* TempBuf, size_t TempSize, menu_t* MenuPtr)
{
	/* Check */
	if (!TempBuf || !TempSize || !MenuPtr)
		return NULL;
		
	if (MenuPtr == &MainDef)
		return "main";
	else if (MenuPtr == &OptionsDef)
		return "options";
	else if (MenuPtr == &DefaultKeyBindDef)
		return "keybinds";
	else if (MenuPtr == &GameOptionsDef)
		return "gameoptions";
	else if (MenuPtr == &NewGameDef)
		return "newgame";
	else if (MenuPtr == &NewGameClassicDef)
		return "newgameclassic";
	else if (MenuPtr == &ProfileDef)
		return "profiles";
	else if (MenuPtr == &SoundsDef)
		return "sounds";
	else if (MenuPtr == &VideoDef)
		return "video";
	else if (MenuPtr == &CreateLocalGameDef)
		return "createlocalgame";
	else if (MenuPtr == &NewGameOptionsDef)
		return "newgameoptions";
	else if (MenuPtr == &NewGameCCSkillDef)
		return "newgameskill";
	else if (MenuPtr == &NewGameCCEpiDef)
		return "newgameepisode";
	else if (MenuPtr == &ControlSettingsDef)
		return "controls";
	else if (MenuPtr == &GraphicalSettingsDef)
		return "graphics";
	else
		return NULL;
}

/* M_DumpMenuXML() -- Dumps the XML data of the menus */
void M_DumpMenuXML(void)
{
#define BUFSIZE 512
	size_t i, j, n;
	menu_t* MenuPtr;
	menuitem_t* MenuItem;
	menuitem_t** PtrPtr;
	char TempBuf[BUFSIZE];
	
	return;
	
	/* Go through every single menu */
	for (i = 0, MenuPtr = NULL; MenuPtrList[i]; i++)
	{
		// Copy pointer
		MenuPtr = MenuPtrList[i];
		PtrPtr = &MenuPtrList[i];
		
		// Check
		if (!MenuPtr)
			break;
			
		// Menu opening
		CONS_Printf("<SubMenu>\n");
		
		// Menu Number
		CONS_Printf("\t<ID>%s</ID>\n", M_CreateID(TempBuf, BUFSIZE, MenuPtr));
		
		// Menu Title
		if (MenuPtr->WMenuTitlePtr)
			CONS_Printf("\t<Title>%s</Title>\n", *(MenuPtr->WMenuTitlePtr));
			
		// Menu Unicode String
		CONS_Printf("\t<Unicode>%s</Unicode>\n", DS_NameOfString(MenuPtr->WMenuTitlePtr));
		
		// Previous Menu
		if (MenuPtr->prevMenu)
			CONS_Printf("\t<PrevID>%s</PrevID>\n", M_CreateID(TempBuf, BUFSIZE, MenuPtr->prevMenu));
			
		// Column Count
		if (MenuPtr->numcolumns)
			CONS_Printf("\t<Columns>%i</Columns>\n", MenuPtr->numcolumns);
			
		// Menu Flags
		CONS_Printf("\t<Flags>");
		
		if (MenuPtr == &MainDef)
			CONS_Printf("root ");
		if (MenuPtr->extraflags & MENUFLAG_OPTIMALSPACE)
			CONS_Printf("optimalspace ");
		if (MenuPtr->extraflags & MENUFLAG_HIDECURSOR)
			CONS_Printf("hidecursor ");
			
		CONS_Printf("</Flags>\n");
		
		// Print Items
		CONS_Printf("\t<Items>\n");
		
		// Go through every item
		for (j = 0; j < MenuPtr->numitems; j++)
		{
			// Get item
			MenuItem = &MenuPtr->menuitems[j];
			
			// Intro
			CONS_Printf("\t\t<Item>\n");
			
			// Item text
			if (MenuItem->WItemTextPtr)
				CONS_Printf("\t\t\t<Text>%s</Text>\n", *(MenuItem->WItemTextPtr));
				
			// Item unicode string
			CONS_Printf("\t\t\t<Unicode>%s</Unicode>\n", DS_NameOfString(MenuItem->WItemTextPtr));
			
			// Hotkey
			if (MenuItem->alphaKey > 0x32 && MenuItem->alphaKey < 0x7F)
				CONS_Printf("\t\t\t<Hotkey>%c</Hotkey>\n", MenuItem->alphaKey);
				
			// Item Type
			if ((MenuItem->status & IT_TYPE) == IT_CVAR)
				CONS_Printf("\t\t\t<ConsoleVar>%s</ConsoleVar>\n", ((consvar_t*) MenuItem->itemaction)->name);
				
			// Item Flags
			CONS_Printf("\t\t\t<Flags>");
			
			//if (MenuPtr->extraflags & MENUFLAG_OPTIMALSPACE)
			//  CONS_Printf("optimalspace ");
			
			CONS_Printf("</Flags>\n");
			
			// Outro
			CONS_Printf("\t\t</Item>\n");
		}
		
		// Close items
		CONS_Printf("\t</Items>\n");
		
		// Menu Closing
		CONS_Printf("</SubMenu>\n");
	}
#undef BUFSIZE
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/* M_MenuExRMODHandle() -- Handle RMOD Menu Data */
bool_t M_MenuExRMODHandle(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private)
{
	/* Check */
	if (!a_Table || !a_WAD || !a_ID || !a_Private)
		return false;
	
	/* Success! */
	return false;
}

/* M_MenuExRMODOrder() -- WAD order changed */
bool_t M_MenuExRMODOrder(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID)
{
	/* Success! */
	return false;
}

