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
// DESCRIPTION: DOOM strings, by language.

#ifndef __DSTRINGS_H__
#define __DSTRINGS_H__

#include "doomtype.h"

// All important printed strings.
// Language selection (message strings).
// Use -DFRENCH etc.

#ifdef FRENCH
#include "d_french.h"
#else
#include "d_englsh.h"
#endif

// Misc. other strings.
#define SAVEGAMENAME    "remoodsv"

extern char savegamename[256];

//
// File locations,
//  relative to current position.
// Path names are OS-sensitive.
//
#define DEVMAPS "devmaps"
#define DEVDATA "devdata"

// Not done in french?

// QuitDOOM messages
//added:02-01-98: "22 messages - 7 fucking messages = 15 cool messages" !
#define NUM_QUITMESSAGES   15

extern char* endmsg[];

/******************************************************************************/

/******************************************************************************/

/******************************************************************************/

typedef enum
{
	/* Menus */
	DSTR_MENUNULLSPACE,
	DSTR_MENUMAIN_NEWGAME,
	DSTR_MENUMAIN_ENDGAME,
	DSTR_MENUMAIN_LOADGAME,
	DSTR_MENUMAIN_SAVEGAME,
	DSTR_MENUMAIN_OPTIONS,
	DSTR_MENUMAIN_PROFILES,
	DSTR_MENUMAIN_QUITGAME,
	DSTR_MENUOPTIONS_TITLE,
	DSTR_MENUOPTIONS_GAMESETTINGS,
	DSTR_MENUOPTIONS_CONTROLSETTINGS,
	DSTR_MENUOPTIONS_GRAPHICALSETTINGS,
	DSTR_MENUOPTIONS_AUDIOSETTINGS,
	DSTR_MENUOPTIONS_ADVANCEDSETTINGS,
	DSTR_MENUOPTIONS_DISABLETITLESCREENDEMOS,
	DSTR_MENUCONTROLS_TITLE,
	DSTR_MENUCONTROLS_CONTROLSUBTITLE,
	DSTR_MENUCONTROLS_ACTIONSPERKEY,
	DSTR_MENUCONTROLS_PLAYERONECONTROLS,
	DSTR_MENUCONTROLS_PLAYERTWOCONTROLS,
	DSTR_MENUCONTROLS_PLAYERTHREECONTROLS,
	DSTR_MENUCONTROLS_PLAYERFOURCONTROLS,
	DSTR_MENUCONTROLS_MOUSESUBTITLE,
	DSTR_MENUCONTROLS_ENABLEMOUSE,
	DSTR_MENUCONTROLS_ADVANCEDMOUSE,
	DSTR_MENUCONTROLS_BASICSETTINGSSUBSUBTITLE,
	DSTR_MENUCONTROLS_USEMOUSEFREELOOK,
	DSTR_MENUCONTROLS_USEMOUSEMOVE,
	DSTR_MENUCONTROLS_INVERTYAXIS,
	DSTR_MENUCONTROLS_MOVETURNSENS,
	DSTR_MENUCONTROLS_LOOKUPDOWNSENS,
	DSTR_MENUCONTROLS_ADVANCEDSETTINGSUBSUBTITLE,
	DSTR_MENUCONTROLS_XAXISSENS,
	DSTR_MENUCONTROLS_YAXISSENS,
	DSTR_MENUCONTROLS_XAXISMOVE,
	DSTR_MENUCONTROLS_YAXISMOVE,
	DSTR_MENUCONTROLS_XAXISMOVESECONDARY,
	DSTR_MENUCONTROLS_YAXISMOVESECONDARY,
	DSTR_MENUCONTROLS_STRAFEKEYUSESECONDARY,
	DSTR_MENUGRAPHICS_TITLE,
	DSTR_MENUGRAPHICS_SCREENSUBTITLE,
	DSTR_MENUGRAPHICS_SETRESOLUTION,
	DSTR_MENUGRAPHICS_FULLSCREEN,
	DSTR_MENUGRAPHICS_BRIGHTNESS,
	DSTR_MENUGRAPHICS_SCREENSIZE,
	DSTR_MENUGRAPHICS_SCREENLINK,
	DSTR_MENUGRAPHICS_RENDERERSUBTITLE,
	DSTR_MENUGRAPHICS_TRANSLUCENCY,
	DSTR_MENUGRAPHICS_ENABLEDECALS,
	DSTR_MENUGRAPHICS_MAXDECALS,
	DSTR_MENUGRAPHICS_CONSOLESUBTITLE,
	DSTR_MENUGRAPHICS_CONSOLESPEED,
	DSTR_MENUGRAPHICS_CONSOLEHEIGHT,
	DSTR_MENUGRAPHICS_CONSOLEBACKGROUND,
	DSTR_MENUGRAPHICS_MESSAGEDURATION,
	DSTR_MENUGRAPHICS_ECHOMESSAGES,
	DSTR_MENUGRAPHICS_MENUSUBTITLE,
	DSTR_MENUGRAPHICS_CURSORBLINKDURATION,
	DSTR_MENUGRAPHICS_HUDSUBTITLE,
	DSTR_MENUGRAPHICS_SCALESTATUSBAR,
	DSTR_MENUGRAPHICS_TRANSPARENTSTATUSBAR,
	DSTR_MENUGRAPHICS_STATUSBARTRANSPARENCYAMOUNT,
	DSTR_MENUGRAPHICS_CROSSHAIR,
	DSTR_MENUKEYBINDS_TITLE,
	DSTR_MENUKEYBINDS_MOVEMENTSUBTITLE,
	DSTR_MENUKEYBINDS_FIRE,
	DSTR_MENUKEYBINDS_ACTIVATE,
	DSTR_MENUKEYBINDS_MOVEFORWARDS,
	DSTR_MENUKEYBINDS_MOVEBACKWARDS,
	DSTR_MENUKEYBINDS_TURNLEFT,
	DSTR_MENUKEYBINDS_TURNRIGHT,
	DSTR_MENUKEYBINDS_RUN,
	DSTR_MENUKEYBINDS_STRAFEON,
	DSTR_MENUKEYBINDS_STRAFELEFT,
	DSTR_MENUKEYBINDS_STRAFERIGHT,
	DSTR_MENUKEYBINDS_LOOKUP,
	DSTR_MENUKEYBINDS_LOOKDOWN,
	DSTR_MENUKEYBINDS_CENTERVIEW,
	DSTR_MENUKEYBINDS_MOUSELOOK,
	DSTR_MENUKEYBINDS_JUMPFLYUP,
	DSTR_MENUKEYBINDS_FLYDOWN,
	DSTR_MENUKEYBINDS_WEAPONSANDITEMSSUBTITLE,
	DSTR_MENUKEYBINDS_SLOTONE,
	DSTR_MENUKEYBINDS_SLOTTWO,
	DSTR_MENUKEYBINDS_SLOTTHREE,
	DSTR_MENUKEYBINDS_SLOTFOUR,
	DSTR_MENUKEYBINDS_SLOTFIVE,
	DSTR_MENUKEYBINDS_SLOTSIX,
	DSTR_MENUKEYBINDS_SLOTSEVEN,
	DSTR_MENUKEYBINDS_SLOTEIGHT,
	DSTR_MENUKEYBINDS_PREVIOUSWEAPON,
	DSTR_MENUKEYBINDS_NEXTWEAPON,
	DSTR_MENUKEYBINDS_BESTWEAPON,
	DSTR_MENUKEYBINDS_INVENTORYLEFT,
	DSTR_MENUKEYBINDS_INVENTORYRIGHT,
	DSTR_MENUKEYBINDS_INVENTORYUSE,
	DSTR_MENUKEYBINDS_MISCSUBTITLE,
	DSTR_MENUKEYBINDS_TALKKEY,
	DSTR_MENUKEYBINDS_RANKINGSANDSCORES,
	DSTR_MENUKEYBINDS_TOGGLECONSOLE,
	DSTR_MENUAUDIO_TITLE,
	DSTR_MENUAUDIO_OUTPUTSUBTITLE,
	DSTR_MENUAUDIO_SOUNDOUTPUT,
	DSTR_MENUAUDIO_SOUNDDEVICE,
	DSTR_MENUAUDIO_MUSICOUTPUT,
	DSTR_MENUAUDIO_MUSICDEVICE,
	DSTR_MENUAUDIO_QUALITYSUBTITLE,
	DSTR_MENUAUDIO_SPEAKERSETUP,
	DSTR_MENUAUDIO_SAMPLESPERSECOND,
	DSTR_MENUAUDIO_BITSPERSAMPLE,
	DSTR_MENUAUDIO_FAKEPCSPEAKERWAVEFORM,
	DSTR_MENUAUDIO_VOLUMESUBTITLE,
	DSTR_MENUAUDIO_SOUNDVOLUME,
	DSTR_MENUAUDIO_MUSICVOLUME,
	DSTR_MENUAUDIO_MISCSUBTITLE,
	DSTR_MENUAUDIO_PRECACHESOUNDS,
	DSTR_MENUAUDIO_RANDOMSOUNDPITCH,
	DSTR_MENUAUDIO_SOUNDCHANNELS,
	DSTR_MENUAUDIO_RESERVEDSOUNDCHANNELS,
	DSTR_MENUAUDIO_MULTITHREADEDSOUND,
	DSTR_MENUAUDIO_MULTITHREADEDMUSIC,
	DSTR_MENUAUDIO_RESETSUBTITLE,
	DSTR_MENUAUDIO_RESETSOUND,
	DSTR_MENUAUDIO_RESETMUSIC,
	DSTR_MENUVIDEO_TITLE,
	DSTR_MENUVIDEO_MODESELECT,
	DSTR_MENUGAME_TITLE,
	DSTR_MENUGAME_MULTIPLAYERSUBTITLE,
	DSTR_MENUGAME_DEATHMATCHTYPE,
	DSTR_MENUGAME_FRAGLIMIT,
	DSTR_MENUGAME_TIMELIMIT,
	DSTR_MENUGAME_TEAMSUBTITLE,
	DSTR_MENUGAME_ENABLETEAMPLAY,
	DSTR_MENUGAME_FRIENDLYFIRE,
	DSTR_MENUGAME_RESTRICTIONSSUBTITLE,
	DSTR_MENUGAME_ALLOWJUMP,
	DSTR_MENUGAME_ALLOWROCKETJUMP,
	DSTR_MENUGAME_ALLOWAUTOAIM,
	DSTR_MENUGAME_ALLOWTURBO,
	DSTR_MENUGAME_ALLOWEXITLEVEL,
	DSTR_MENUGAME_FORCEAUTOAIM,
	DSTR_MENUGAME_WEAPONSANDITEMSSUBTITLE,
	DSTR_MENUGAME_ENABLEITEMRESPAWN,
	DSTR_MENUGAME_ITEMRESPAWNTIME,
	DSTR_MENUGAME_DROPWEAPONSWHENYOUDIE,
	DSTR_MENUGAME_INFINITEAMMO,
	DSTR_MENUGAME_MONSTERSSUBTITLE,
	DSTR_MENUGAME_SPAWNMONSTERS,
	DSTR_MENUGAME_ENABLEMONSTERRESPAWN,
	DSTR_MENUGAME_MONSTERRESPAWNTIME,
	DSTR_MENUGAME_FASTMONSTERS,
	DSTR_MENUGAME_PREDICTINGMONSTERS,
	DSTR_MENUGAME_MISCSUBTITLE,
	DSTR_MENUGAME_GRAVITY,
	DSTR_MENUGAME_SOLIDCORPSES,
	DSTR_MENUGAME_BLOODTIME,
	DSTR_MENUGAME_COMPATIBILITYSUBTITLE,
	DSTR_MENUGAME_CLASSICBLOOD,
	DSTR_MENUGAME_CLASSICROCKETEXPLOSIONS,
	DSTR_MENUGAME_CLASSICMONSTERMELEERANGE,
	DSTR_MENUGAME_CLASSICMONSTERLOGIC,
	DSTR_MENUNEWGAME_TITLE,
	DSTR_MENUNEWGAME_SINGLEPLAYERSUBTITLE,
	DSTR_MENUNEWGAME_CLASSIC,
	DSTR_MENUNEWGAME_CREATEGAME,
	DSTR_MENUNEWGAME_QUICKSTART,
	DSTR_MENUNEWGAME_MULTIPLAYERSUBTITLE,
	DSTR_MENUNEWGAME_SPLITSCREENGAME,
	DSTR_MENUNEWGAME_UDPLANINTERNETGAME,
	DSTR_MENUNEWGAME_TCPLANINTERNETGAME,
	DSTR_MENUNEWGAME_MODEMGAME,
	DSTR_MENUNEWGAME_SERIALNULLMODEMGAME,
	DSTR_MENUNEWGAME_FORKGAME,
	DSTR_MENUCLASSICGAME_TITLE,
	DSTR_MENUCLASSICGAME_DOOMSKILLA,
	DSTR_MENUCLASSICGAME_DOOMSKILLB,
	DSTR_MENUCLASSICGAME_DOOMSKILLC,
	DSTR_MENUCLASSICGAME_DOOMSKILLD,
	DSTR_MENUCLASSICGAME_DOOMSKILLE,
	DSTR_MENUCLASSICGAME_HERETICSKILLA,
	DSTR_MENUCLASSICGAME_HERETICSKILLB,
	DSTR_MENUCLASSICGAME_HERETICSKILLC,
	DSTR_MENUCLASSICGAME_HERETICSKILLD,
	DSTR_MENUCLASSICGAME_HERETICSKILLE,
	DSTR_MENUCLASSICGAME_DOOMEPISODEA,
	DSTR_MENUCLASSICGAME_DOOMEPISODEB,
	DSTR_MENUCLASSICGAME_DOOMEPISODEC,
	DSTR_MENUCLASSICGAME_DOOMEPISODED,
	DSTR_MENUCLASSICGAME_DOOMEPISODEE,
	DSTR_MENUCLASSICGAME_DOOMEPISODEF,
	DSTR_MENUCLASSICGAME_HERETICEPISODEA,
	DSTR_MENUCLASSICGAME_HERETICEPISODEB,
	DSTR_MENUCLASSICGAME_HERETICEPISODEC,
	DSTR_MENUCLASSICGAME_HERETICEPISODED,
	DSTR_MENUCLASSICGAME_HERETICEPISODEE,
	DSTR_MENUCLASSICGAME_HERETICEPISODEF,
	DSTR_MENUCREATEGAME_SOLOTITLE,
	DSTR_MENUCREATEGAME_LOCALTITLE,
	DSTR_MENUCREATEGAME_LEVEL,
	DSTR_MENUCREATEGAME_SKILL,
	DSTR_MENUCREATEGAME_SPAWNMONSTERS,
	DSTR_MENUCREATEGAME_OPTIONS,
	DSTR_MENUCREATEGAME_SETUPOPTIONS,
	DSTR_MENUCREATEGAME_STARTGAME,
	DSTR_MENUCREATEGAME_NUMBEROFPLAYERS,
	DSTR_MENUCREATEGAME_DEATHMATCHTYPE,
	DSTR_MENUCREATEGAME_FRAGLIMIT,
	DSTR_MENUCREATEGAME_TIMELIMIT,
	DSTR_MENUPROFILES_TITLE,
	DSTR_MENUPROFILES_CREATEPROFILE,
	DSTR_MENUPROFILES_CURRENTPROFILE,
	DSTR_MENUPROFILES_NAME,
	DSTR_MENUPROFILES_COLOR,
	DSTR_MENUPROFILES_SKIN,
	DSTR_MENUPROFILES_AUTOAIM,
	DSTR_MENUCREATEPROFILE_TITLE,
	DSTR_MENUCREATEPROFILE_PLEASENAME,
	DSTR_MENUCREATEPROFILE_NAME,
	DSTR_MENUCREATEPROFILE_ACCEPT,
	DSTR_MENUSELECTPROFILE_TITLE,
	DSTR_MENUSELECTPROFILE_PLEASESELECT,
	DSTR_MENUSELECTPROFILE_PLACEHOLDER,
	DSTR_MENUSELECTPROFILE_FORYOU,
	DSTR_MENUSELECTPROFILE_TWOSPLITA,
	DSTR_MENUSELECTPROFILE_TWOSPLITB,
	DSTR_MENUSELECTPROFILE_FOURSPLITA,
	DSTR_MENUSELECTPROFILE_FOURSPLITB,
	DSTR_MENUSELECTPROFILE_FOURSPLITC,
	DSTR_MENUSELECTPROFILE_FOURSPLITD,
	DSTR_MENUSELECTPROFILE_PROFILE,
	DSTR_MENUSELECTPROFILE_ACCEPT,
	DSTR_MENUOTHER_RANDOM,
	DSTR_MENUOTHER_RANDOMEPISODEA,
	DSTR_MENUOTHER_RANDOMEPISODEB,
	DSTR_MENUOTHER_RANDOMEPISODEC,
	DSTR_MENUOTHER_RANDOMEPISODED,
	DSTR_MENUOTHER_RANDOMEPISODEE,
	DSTR_MENUOTHER_RANDOMEPISODEF,
	DSTR_MENUOTHER_CHANGECONTROL,
	DSTR_MENUOTHER_PLAYERACONTROLS,
	DSTR_MENUOTHER_PLAYERBCONTROLS,
	DSTR_MENUOTHER_PLAYERCCONTROLS,
	DSTR_MENUOTHER_PLAYERDCONTROLS,
	DSTR_MENUOPTIONS_BINARYSAVES,
	
	/* Intermission Screen */
	DSTR_INTERMISSION_FINISHED,
	DSTR_INTERMISSION_ENTERING,
	DSTR_INTERMISSION_KILLS,
	DSTR_INTERMISSION_ITEMS,
	DSTR_INTERMISSION_SECRETS,
	DSTR_INTERMISSION_FRAGS,
	DSTR_INTERMISSION_NETKILLS,
	DSTR_INTERMISSION_NETITEMS,
	DSTR_INTERMISSION_NETSECRETS,
	DSTR_INTERMISSION_NETFRAGS,
	DSTR_INTERMISSION_NOWENTERING,
	DSTR_INTERMISSION_TIME,
	DSTR_INTERMISSION_PAR,
	DSTR_INTERMISSION_NETTIME,
	DSTR_INTERMISSION_NETPAR,
	
	/* Console Variable Hints */
	DSTR_CVHINT_CONSCREENHEIGHT,
	DSTR_CVHINT_CONBACKCOLOR,
	DSTR_CVHINT_CONFONT,
	DSTR_CVHINT_CONMONOSPACE,
	DSTR_CVHINT_CONSCALE,
	
	NUMUNICODESTRINGS
} UnicodeStringID_t;

typedef struct StringGroupEX_s
{
	const char* const id;
	char* wcharstr;
	uint32_t Hash;
} StringGroupEX_t;

StringGroupEX_t UnicodeStrings[NUMUNICODESTRINGS];

#define PTROFUNICODESTRING(n) (&(UnicodeStrings[n].wcharstr))
#define PTRTOUNICODESTRING(n) (UnicodeStrings[n].wcharstr)
#define DS_GetString(n) ((const char*)(UnicodeStrings[(UnicodeStringID_t)(n)].wcharstr))

const char* DS_NameOfString(char** const WCharStr);
const char** DS_FindStringRef(const char* const a_StrID);

#endif							/* __DSTRINGS_H__ */
