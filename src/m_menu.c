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

boolean localgame;

// -1 = no quicksave slot picked!
int quickSaveSlot;
boolean menuactive;

// we are going to be entering a savegame string
int saveStringEnter;
int saveSlot;					// which slot to save in
int saveCharIndex;				// which char we're editing
char saveOldString[SAVESTRINGSIZE]; // old save description before edit

char savegamestrings[10][SAVESTRINGSIZE];

// current menudef
menu_t *currentMenu;
short itemOn;					// menu item skull is on
short skullAnimCounter;			// skull animation counter
short whichSkull;				// which skull to draw
int SkullBaseLump;

// graphic name of skulls
char skullName[2][9] = { "M_SKULL1", "M_SKULL2" };

const char *ALLREADYPLAYING = "You are already playing\n\nLeave this game first\n";

menu_t
	MainDef,					// Main Menu
	OptionsDef,					// Options Menu
	DefaultKeyBindDef,			// Key Binds
	GameOptionsDef,				// Game Options
	NewGameDef,					// New Game
	NewGameClassicDef,			// New Game -> Classic
	SoundsDef,					// Sound Options
	VideoDef,					// Video Options
	CreateLocalGameDef,			// New Game -> Local
	NewGameOptionsDef,			// New Game -> Options
	NewGameCCSkillDef,
	NewGameCCEpiDef,
	
	ControlSettingsDef,
	GraphicalSettingsDef,
	
	LASTMENU
	;

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
	
	NULL,	// To determine the end
};

// =============================================================================
//                                MAIN MENU
// =============================================================================

menuitem_t MainItems[] =
{
	{ITX_MAINMENUITEM | IT_SUBMENU, NULL,		PTROFUNICODESTRING(MENU_MAIN_NEWGAME), &NewGameDef},
	{ITX_MAINMENUITEM | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_MAIN_ENDGAME), M_EndGame},
	{ITX_MAINMENUITEM | IT_DISABLED2, NULL,		PTROFUNICODESTRING(MENU_MAIN_LOADGAME), NULL},
	{ITX_MAINMENUITEM | IT_DISABLED2, NULL,		PTROFUNICODESTRING(MENU_MAIN_SAVEGAME), NULL},
	{ITX_MAINMENUITEM | IT_SUBMENU,	 NULL,		PTROFUNICODESTRING(MENU_MAIN_OPTIONS), &OptionsDef},
	{ITX_MAINMENUITEM | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_MAIN_PROFILES), M_StartProfiler},
	{ITX_MAINMENUITEM | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_MAIN_QUITGAME), M_QuitDOOM},
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
	//{ITX_SUBMENUTITLE, NULL,			"*** GAME ***", NULL},
	{IT_STRING | IT_SUBMENU, NULL,		PTROFUNICODESTRING(MENU_OPTIONS_GAMESETTINGS), &GameOptionsDef},
	{IT_STRING | IT_SUBMENU, NULL,		PTROFUNICODESTRING(MENU_OPTIONS_CONTROLSETTINGS), &ControlSettingsDef},
	{IT_STRING | IT_SUBMENU, NULL,		PTROFUNICODESTRING(MENU_OPTIONS_GRAPHICALSETTINGS), &GraphicalSettingsDef},
	{IT_STRING | IT_SUBMENU, NULL,		PTROFUNICODESTRING(MENU_OPTIONS_AUDIOSETTINGS), &SoundsDef},
	//{IT_STRING | IT_SUBMENU, NULL,	PTROFUNICODESTRING(MENU_OPTIONS_ADVANCEDSETTINGS), &AdvancedDef},	// Change if you dare
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(MENU_NULLSPACE), NULL},
	{IT_STRING | IT_CVAR, 0,			PTROFUNICODESTRING(MENU_OPTIONS_DISABLETITLESCREENDEMOS), &cv_disabledemos, 0},
	
	/*{ITX_SUBMENUTITLE, NULL,			"*** GAME ***", NULL},
	{IT_STRING | IT_SUBMENU, NULL,		"Options and Flags...", &GameOptionsDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Heads up Display...", &HeadsUpDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Graphics...", &GraphicsDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Console...", &ConsoleDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Mouse...", &MouseOptionsDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Joysticks...", &JoystickOptionsDef},
	{IT_STRING | IT_SUBMENU, NULL,		"Miscellaneous...", &MiscOptionsDef},
	{ITX_SUBMENUTITLE, NULL,			"*** SYSTEM ***", NULL},
	{IT_STRING | IT_CALL, NULL,			"Video...", &M_StartVideoOptions},
	{IT_STRING | IT_SUBMENU, NULL,		"Sound...", &SoundsDef},
	{ITX_SUBMENUTITLE, NULL,			"*** PLAYERS ***", NULL},
	{IT_STRING | IT_CALL, NULL,			"Default Key Binds (Player 1)...", M_ControlsDoPlayer1},
	{IT_STRING | IT_CALL, NULL,			"Default Key Binds (Player 2)...", M_ControlsDoPlayer2},
	{IT_STRING | IT_CALL, NULL,			"Default Key Binds (Player 3)...", M_ControlsDoPlayer3},
	{IT_STRING | IT_CALL, NULL,			"Default Key Binds (Player 4)...", M_ControlsDoPlayer4},*/
};

menu_t OptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTIONS",
	PTROFUNICODESTRING(MENU_OPTIONS_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_CONTROLSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0,			PTROFUNICODESTRING(MENU_CONTROLS_ACTIONSPERKEY), &cv_controlperkey, 0},
	
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_PLAYERONECONTROLS), M_ControlsDoPlayer1},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_PLAYERTWOCONTROLS), M_ControlsDoPlayer2},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_PLAYERTHREECONTROLS), M_ControlsDoPlayer3},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_PLAYERFOURCONTROLS), M_ControlsDoPlayer4},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_CONTROLS_MOUSESUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_ENABLEMOUSE), &cv_usemouse, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_ADVANCEDMOUSE), &cv_m_legacymouse, 0},
	
	{IT_STRING | IT_SPACE | IT_WHITESTRING, 0, PTROFUNICODESTRING(MENU_CONTROLS_BASICSETTINGSSUBSUBTITLE)},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_USEMOUSEFREELOOK), &cv_alwaysfreelook, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_USEMOUSEMOVE), &cv_mousemove, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_INVERTYAXIS), &cv_invertmouse, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(MENU_CONTROLS_MOVETURNSENS), &cv_m_xsensitivity, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(MENU_CONTROLS_LOOKUPDOWNSENS), &cv_mlooksens, 0},
	
	{IT_STRING | IT_SPACE | IT_WHITESTRING, 0, PTROFUNICODESTRING(MENU_CONTROLS_ADVANCEDSETTINGSUBSUBTITLE)},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(MENU_CONTROLS_XAXISSENS), &cv_m_xsensitivity, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, PTROFUNICODESTRING(MENU_CONTROLS_YAXISSENS), &cv_m_ysensitivity, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_XAXISMOVE), &cv_m_xaxismode, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_YAXISMOVE), &cv_m_yaxismode, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_XAXISMOVESECONDARY), &cv_m_xaxissecmode, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_YAXISMOVESECONDARY), &cv_m_yaxissecmode, 0},
	{IT_STRING | IT_CVAR, 0, PTROFUNICODESTRING(MENU_CONTROLS_STRAFEKEYUSESECONDARY), &cv_m_classicalt, 0},
};

menu_t ControlSettingsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_CONTRO",
	PTROFUNICODESTRING(MENU_CONTROLS_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GRAPHICS_SCREENSUBTITLE), NULL},
	{IT_STRING | IT_CALL, NULL,				PTROFUNICODESTRING(MENU_GRAPHICS_SETRESOLUTION), &M_StartVideoOptions},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_FULLSCREEN), &cv_fullscreen, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,	PTROFUNICODESTRING(MENU_GRAPHICS_BRIGHTNESS), &cv_usegamma, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,	PTROFUNICODESTRING(MENU_GRAPHICS_SCREENSIZE), &cv_viewsize, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_SCREENLINK), &cv_screenslink, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GRAPHICS_RENDERERSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_TRANSLUCENCY), &cv_translucency, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_ENABLEDECALS), &cv_splats, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_MAXDECALS), &cv_maxsplats, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GRAPHICS_CONSOLESUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_CONSOLESPEED), &cons_speed, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_CONSOLEHEIGHT), &cons_height, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_CONSOLEBACKGROUND), &cons_backpic, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_MESSAGEDURATION), &cons_msgtimeout, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_ECHOMESSAGES), &cv_showmessages, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GRAPHICS_MENUSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_CURSORBLINKDURATION), &cv_cons_blinkingrate, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GRAPHICS_HUDSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_SCALESTATUSBAR), &cv_scalestatusbar, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_TRANSPARENTSTATUSBAR), &cv_transparentstatusbar, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_STATUSBARTRANSPARENCYAMOUNT), &cv_transparentstatusbarmode, 0},
	{IT_STRING | IT_CVAR, 0, 				PTROFUNICODESTRING(MENU_GRAPHICS_CROSSHAIR), &cv_crosshair, 0},
};

menu_t GraphicalSettingsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(MENU_GRAPHICS_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_KEYBINDS_MOVEMENTSUBTITLE), NULL},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_FIRE), M_ChangeControl, gc_fire},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_ACTIVATE), M_ChangeControl, gc_use},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_MOVEFORWARDS), M_ChangeControl, gc_forward},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_MOVEBACKWARDS), M_ChangeControl, gc_backward},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_TURNLEFT), M_ChangeControl, gc_turnleft},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_TURNRIGHT), M_ChangeControl, gc_turnright},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_RUN), M_ChangeControl, gc_speed},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_STRAFEON), M_ChangeControl, gc_strafe},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_STRAFELEFT), M_ChangeControl, gc_strafeleft},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_STRAFERIGHT), M_ChangeControl, gc_straferight},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_LOOKUP), M_ChangeControl, gc_lookup},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_LOOKDOWN), M_ChangeControl, gc_lookdown},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_CENTERVIEW), M_ChangeControl, gc_centerview},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_MOUSELOOK), M_ChangeControl, gc_mouseaiming},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_JUMPFLYUP), M_ChangeControl, gc_jump},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_FLYDOWN), M_ChangeControl, gc_flydown},
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_KEYBINDS_WEAPONSANDITEMSSUBTITLE), NULL},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTONE), M_ChangeControl, gc_weapon1},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTTWO), M_ChangeControl, gc_weapon2},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTTHREE), M_ChangeControl, gc_weapon3},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTFOUR), M_ChangeControl, gc_weapon4},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTFIVE), M_ChangeControl, gc_weapon5},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTSIX), M_ChangeControl, gc_weapon6},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTSEVEN), M_ChangeControl, gc_weapon7},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_SLOTEIGHT), M_ChangeControl, gc_weapon8},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_PREVIOUSWEAPON), M_ChangeControl, gc_prevweapon},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_NEXTWEAPON), M_ChangeControl, gc_nextweapon},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_BESTWEAPON), M_ChangeControl, gc_bestweapon},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_INVENTORYLEFT), M_ChangeControl, gc_invprev},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_INVENTORYRIGHT), M_ChangeControl, gc_invnext},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_INVENTORYUSE), M_ChangeControl, gc_invuse},
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_KEYBINDS_MISCSUBTITLE), NULL},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_TALKKEY), M_ChangeControl, gc_talkkey},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_RANKINGSANDSCORES), M_ChangeControl, gc_scores},
	{IT_CALL | IT_STRING2, 0, PTROFUNICODESTRING(MENU_KEYBINDS_TOGGLECONSOLE), M_ChangeControl, gc_console},
};

menu_t DefaultKeyBindDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_CONTRO",
	PTROFUNICODESTRING(MENU_KEYBINDS_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,				PTROFUNICODESTRING(MENU_AUDIO_OUTPUTSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_SOUNDOUTPUT), &cv_snd_output, 0},
	{IT_STRING | IT_CVAR | IT_CV_STRING, 0,	PTROFUNICODESTRING(MENU_AUDIO_SOUNDDEVICE), &cv_snd_device, 0},
	//{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_MUSICOUTPUT), &cv_, 0},
	//{IT_STRING | IT_CVAR | IT_CV_STRING, 0,	PTROFUNICODESTRING(MENU_AUDIO_MUSICDEVICE), &cv_, 0},
	
	{ITX_SUBMENUTITLE, NULL,				PTROFUNICODESTRING(MENU_AUDIO_QUALITYSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_SPEAKERSETUP), &cv_snd_speakersetup, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_SAMPLESPERSECOND), &cv_snd_soundquality, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_BITSPERSAMPLE), &cv_snd_sounddensity, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_FAKEPCSPEAKERWAVEFORM), &cv_snd_pcspeakerwave, 0},
	
	{ITX_SUBMENUTITLE, NULL,				PTROFUNICODESTRING(MENU_AUDIO_VOLUMESUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,	PTROFUNICODESTRING(MENU_AUDIO_SOUNDVOLUME), &cv_soundvolume, 0},
	//{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,	PTROFUNICODESTRING(MENU_AUDIO_MUSICVOLUME), &cv_musicvolume, 0},
	
	{ITX_SUBMENUTITLE, NULL,				PTROFUNICODESTRING(MENU_AUDIO_MISCSUBTITLE), NULL},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_PRECACHESOUNDS), &precachesound},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_RANDOMSOUNDPITCH), &cv_rndsoundpitch, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_SOUNDCHANNELS), &cv_snd_channels, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_RESERVEDSOUNDCHANNELS), &cv_snd_reservedchannels, 0},
	{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_MULTITHREADEDSOUND), &cv_snd_multithreaded, 0},
	//{IT_STRING | IT_CVAR, 0,				PTROFUNICODESTRING(MENU_AUDIO_MULTITRHEADEDMUSIC), &cv_mus_multithreaded, 0},
	
	{ITX_SUBMENUTITLE, NULL,				PTROFUNICODESTRING(MENU_AUDIO_RESETSUBTITLE), NULL},
	{IT_STRING | IT_CALL, 0,				PTROFUNICODESTRING(MENU_AUDIO_RESETSOUND), M_ResetSound},
	//{IT_STRING | IT_CALL, 0,				PTROFUNICODESTRING(MENU_AUDIO_RESETMUSIC)},

	/*{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "Sound Volume", &cv_soundvolume, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "Music Volume", &cv_musicvolume, 0},
	{IT_STRING | IT_CVAR | IT_CV_SLIDER, 0, "CD Volume", &cd_volume, 0},
	{IT_STRING | IT_CVAR, 0, "Channel Count", &cv_numChannels, 0},
	{IT_STRING | IT_CVAR, 0, "Reverse Stereo", &stereoreverse, 0},*/
	
};

menu_t SoundsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(MENU_AUDIO_TITLE),
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
	{IT_WHITESTRING | IT_KEYHANDLER,	0,	PTROFUNICODESTRING(MENU_VIDEO_MODESELECT), &M_HandleVideoKey, 0},
};

menu_t VideoDef =
{
	MENUFLAG_OPTIMALSPACE/* | MENUFLAG_HIDECURSOR*/,
	"M_OPTION",
	PTROFUNICODESTRING(MENU_VIDEO_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_MULTIPLAYERSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_DEATHMATCHTYPE), &cv_deathmatch, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FRAGLIMIT), &cv_fraglimit, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_TIMELIMIT), &cv_timelimit, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_TEAMSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLETEAMPLAY), &cv_teamplay, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FRIENDLYFIRE), &cv_teamdamage, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_RESTRICTIONSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWJUMP), &cv_allowjump, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWROCKETJUMP), &cv_allowrocketjump, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWAUTOAIM), &cv_allowautoaim, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWTURBO), &cv_allowturbo, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWEXITLEVEL), &cv_allowexitlevel, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FORCEAUTOAIM), &cv_forceautoaim, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_WEAPONSANDITEMSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLEITEMRESPAWN), &cv_itemrespawn, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ITEMRESPAWNTIME), &cv_itemrespawntime, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_DROPWEAPONSWHENYOUDIE), &cv_fragsweaponfalling, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_INFINITEAMMO), &cv_infiniteammo, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_MONSTERSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_SPAWNMONSTERS), &cv_spawnmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLEMONSTERRESPAWN), &cv_respawnmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_MONSTERRESPAWNTIME), &cv_respawnmonsterstime, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FASTMONSTERS), &cv_fastmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_PREDICTINGMONSTERS), &cv_predictingmonsters, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_MISCSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_GRAVITY), &cv_gravity, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_SOLIDCORPSES), &cv_solidcorpse, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_BLOODTIME), &cv_bloodtime, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_COMPATIBILITYSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICBLOOD), &cv_classicblood, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICROCKETEXPLOSIONS), &cv_classicrocketblast, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICMONSTERMELEERANGE), &cv_classicmeleerange, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICMONSTERLOGIC), &cv_classicmonsterlogic, 0},
};

menu_t GameOptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(MENU_GAME_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_TEAMSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLETEAMPLAY), &cv_ng_teamplay, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FRIENDLYFIRE), &cv_ng_teamdamage, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_RESTRICTIONSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWJUMP), &cv_ng_allowjump, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWROCKETJUMP), &cv_ng_allowrocketjump, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWAUTOAIM), &cv_ng_allowautoaim, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWTURBO), &cv_ng_allowturbo, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ALLOWEXITLEVEL), &cv_ng_allowexitlevel, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FORCEAUTOAIM), &cv_ng_forceautoaim, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_WEAPONSANDITEMSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLEITEMRESPAWN), &cv_ng_itemrespawn, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ITEMRESPAWNTIME), &cv_ng_itemrespawntime, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_DROPWEAPONSWHENYOUDIE), &cv_ng_fragsweaponfalling, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_INFINITEAMMO), &cv_ng_infiniteammo, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_MONSTERSSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_ENABLEMONSTERRESPAWN), &cv_ng_respawnmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_MONSTERRESPAWNTIME), &cv_ng_respawnmonsterstime, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_FASTMONSTERS), &cv_ng_fastmonsters, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_PREDICTINGMONSTERS), &cv_ng_predictingmonsters, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_MISCSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_GRAVITY), &cv_ng_gravity, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_SOLIDCORPSES), &cv_ng_solidcorpse, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_BLOODTIME), &cv_ng_bloodtime, 0},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_GAME_COMPATIBILITYSUBTITLE), NULL},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICBLOOD), &cv_ng_classicblood, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICROCKETEXPLOSIONS), &cv_ng_classicrocketblast, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICMONSTERMELEERANGE), &cv_ng_classicmeleerange, 0},
	{IT_STRING | IT_CVAR | IT_CVARREADONLY, 0, PTROFUNICODESTRING(MENU_GAME_CLASSICMONSTERLOGIC), &cv_ng_classicmonsterlogic, 0}
};

menu_t NewGameOptionsDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(MENU_GAME_TITLE),
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
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_NEWGAME_SINGLEPLAYERSUBTITLE), NULL},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_NEWGAME_CLASSIC), M_DoNewGameClassicClassic},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_NEWGAME_CREATEGAME), M_DoNewGameClassic},
	{IT_STRING | IT_DISABLED2, NULL,	PTROFUNICODESTRING(MENU_NEWGAME_QUICKSTART), NULL},
	
	{ITX_SUBMENUTITLE, NULL,			PTROFUNICODESTRING(MENU_NEWGAME_MULTIPLAYERSUBTITLE), NULL},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_NEWGAME_SPLITSCREENGAME), M_DoNewGameLocal},
	{IT_STRING | IT_DISABLED2, NULL,	PTROFUNICODESTRING(MENU_NEWGAME_UDPLANINTERNETGAME), NULL},
	{IT_STRING | IT_DISABLED2, NULL,	PTROFUNICODESTRING(MENU_NEWGAME_FORKGAME), NULL},
};

menu_t NewGameDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(MENU_NEWGAME_TITLE),
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
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLA), M_SelectSkill},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLB), M_SelectSkill},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLC), M_SelectSkill},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLD), M_SelectSkill},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLE), M_SelectSkill},
};

menuitem_t NewGameCCEpisodeItems[] =
{
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEA), M_SelectEpisode},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEB), M_SelectEpisode},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEC), M_SelectEpisode},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODED), M_SelectEpisode},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEE), M_SelectEpisode},
	{IT_STRING | IT_CALL | IT_CENTERSTRING, NULL,	PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEF), M_SelectEpisode},
};

menu_t NewGameCCSkillDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(MENU_CLASSICGAME_TITLE),
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
	PTROFUNICODESTRING(MENU_CLASSICGAME_TITLE),
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
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_LEVEL), &cv_ng_map},
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_SKILL), &cv_ng_skill},
	{IT_STRING | IT_CVAR, 0, 			PTROFUNICODESTRING(MENU_CREATEGAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0},
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_OPTIONS), &cv_ng_options},
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(MENU_NULLSPACE), NULL},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_SETUPOPTIONS), M_ClassicGameOptions},
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(MENU_NULLSPACE), NULL},
	{IT_WHITESTRING | IT_CALL, NULL,	PTROFUNICODESTRING(MENU_CREATEGAME_STARTGAME), M_StartClassicGame},
};

menu_t NewGameClassicDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(MENU_CREATEGAME_SOLOTITLE),
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
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_NUMBEROFPLAYERS), &cv_ng_splitscreen},
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_LEVEL), &cv_ng_map},
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_SKILL), &cv_ng_skill},
	{IT_STRING | IT_CVAR, 0, 			PTROFUNICODESTRING(MENU_CREATEGAME_SPAWNMONSTERS), &cv_ng_spawnmonsters, 0},
	{IT_STRING | IT_CVAR, 0, 			PTROFUNICODESTRING(MENU_CREATEGAME_DEATHMATCHTYPE), &cv_ng_deathmatch, 0},
	{IT_STRING | IT_CVAR, 0,			PTROFUNICODESTRING(MENU_CREATEGAME_FRAGLIMIT), &cv_ng_fraglimit, 0},
	{IT_STRING | IT_CVAR, 0, 			PTROFUNICODESTRING(MENU_CREATEGAME_TIMELIMIT), &cv_ng_timelimit, 0},
	{IT_STRING | IT_CVAR, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_OPTIONS), &cv_ng_options},
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(MENU_NULLSPACE), NULL},
	{IT_STRING | IT_CALL, NULL,			PTROFUNICODESTRING(MENU_CREATEGAME_SETUPOPTIONS), M_LocalGameOptions},
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(MENU_NULLSPACE), NULL},
	{IT_WHITESTRING | IT_CALL, NULL,	PTROFUNICODESTRING(MENU_CREATEGAME_STARTGAME), M_StartLocalGame},
};

menu_t CreateLocalGameDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_NGAME",
	PTROFUNICODESTRING(MENU_CREATEGAME_LOCALTITLE),
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

