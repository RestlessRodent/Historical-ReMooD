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
// DESCRIPTION: DOOM strings, by language.

#ifndef __DSTRINGS_H__
#define __DSTRINGS_H__

#include "doomtype.h"

/* UnicodeStringID_t -- String IDs */
typedef enum UnicodeStringID_e
{
	/* Menus */
	DSTR_MENU_NULLSPACE,
	DSTR_MENUMAIN_TITLE,
	DSTR_MENUMAIN_NEWGAME,
	DSTR_MENUMAIN_ENDGAME,
	DSTR_MENUMAIN_LOADGAME,
	DSTR_MENUMAIN_SAVEGAME,
	DSTR_MENUMAIN_OPTIONS,
	DSTR_MENUMAIN_PROFILES,
	DSTR_MENUMAIN_QUITGAME,
	DSTR_MENU_MAIN_PWADMENU,
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
	
	DSTR_MENUGAMEVAR_CATNONE,
	DSTR_MENUGAMEVAR_CATGAME,
	DSTR_MENUGAMEVAR_CATITEMS,
	DSTR_MENUGAMEVAR_CATPLAYERS,
	DSTR_MENUGAMEVAR_CATMONSTERS,
	DSTR_MENUGAMEVAR_CATMISC,
	DSTR_MENUGAMEVAR_CATFUN,
	DSTR_MENUGAMEVAR_CATHERETIC,
	DSTR_MENUGAMEVAR_CATCOMPAT,
	
	DSTR_MENUGENERAL_HELLOWORLD,
	DSTR_MENUGENERAL_NEWGAMETYPE,
	DSTR_MENUGENERAL_NEWGAMEISLOCAL,
	DSTR_MENUGENERAL_NEWGAMEISSERVER,
	DSTR_MENUGENERAL_NEWGAMELEVEL,
	DSTR_MENUGENERAL_SOLOGAME,
	DSTR_MENUGENERAL_MAINMENU,
	DSTR_MENUGENERAL_QUIT,
	DSTR_MENUGENERAL_CLASSICSTART,
	DSTR_MENUGENERAL_SKILLLEVEL,
	DSTR_MENUGENERAL_SELECTEPIS,
	DSTR_MENUGENERAL_SELECTPROFILE,
	DSTR_MENUGENERAL_SELECTTHISPROF,
	DSTR_MENUGENERAL_CREATEPROF,
	DSTR_MENUGENERAL_CURRENTPROF,
	DSTR_MENUGENERAL_OPTIONS,
	DSTR_MENUGENERAL_SERVERVARS,
	DSTR_MENUGENERAL_CONSOLEVARS,
	DSTR_MENUGENERAL_INTERFACEVARS,
	DSTR_MENUGENERAL_MENUVARS,
	DSTR_MENUGENERAL_RENDERERVARS,
	DSTR_MENUGENERAL_SCREENVARS,
	DSTR_MENUGENERAL_SOUNDVARS,
	
	DSTR_MENUGENERAL_CONBACKCOLOR,
	DSTR_MENUGENERAL_CONFONT,
	DSTR_MENUGENERAL_CONFORECOLOR,
	DSTR_MENUGENERAL_CONMONOSPACE,
	DSTR_MENUGENERAL_CONPAUSEGAME,
	DSTR_MENUGENERAL_CONSCALE,
	DSTR_MENUGENERAL_CONSCREENHEIGHT,
	DSTR_MENUGENERAL_CONTESTSTRING,
	DSTR_MENUGENERAL_IENABLEJOYSTICK,
	DSTR_MENUGENERAL_IENABLEMOUSE,
	DSTR_MENUGENERAL_IOSSMIDIDEV,
	DSTR_MENUGENERAL_MENUCOMPACT,
	DSTR_MENUGENERAL_MENUFONT,
	DSTR_MENUGENERAL_MENUHEADERCOLOR,
	DSTR_MENUGENERAL_MENUITEMCOLOR,
	DSTR_MENUGENERAL_MENUVALCOLOR,
	DSTR_MENUGENERAL_RCUTWALLDETAIL,
	DSTR_MENUGENERAL_RDRAWSPLATS,
	DSTR_MENUGENERAL_RFAKESSPAL,
	DSTR_MENUGENERAL_RMAXSPLATS,
	DSTR_MENUGENERAL_RRENDERER,
	DSTR_MENUGENERAL_RTRANSPARENCY,
	DSTR_MENUGENERAL_RVIEWSIZE,
	DSTR_MENUGENERAL_SCRFULLSCREEN,
	DSTR_MENUGENERAL_SCRHEIGHT,
	DSTR_MENUGENERAL_SCRWIDTH,
	DSTR_MENUGENERAL_SNDBUFFERSIZE,
	DSTR_MENUGENERAL_SNDCHANNELS,
	DSTR_MENUGENERAL_SNDDENSITY,
	DSTR_MENUGENERAL_SNDMUSICVOLUME,
	DSTR_MENUGENERAL_SNDQUALITY,
	DSTR_MENUGENERAL_SNDRANDOMPITCH,
	DSTR_MENUGENERAL_SNDRESERVEDCHANNELS,
	DSTR_MENUGENERAL_SNDSOUNDVOLUME,
	DSTR_MENUGENERAL_SNDSPEAKERSETUP,
	DSTR_MENUGENERAL_SVCONNECTPASSWORD,
	DSTR_MENUGENERAL_SVEMAIL,
	DSTR_MENUGENERAL_SVIRC,
	DSTR_MENUGENERAL_SVJOINPASSWORD,
	DSTR_MENUGENERAL_SVMAXCATCHUP,
	DSTR_MENUGENERAL_SVMAXCLIENTS,
	DSTR_MENUGENERAL_SVMAXDEMOCATCHUP,
	DSTR_MENUGENERAL_SVMOTD,
	DSTR_MENUGENERAL_SVNAME,
	DSTR_MENUGENERAL_SVREADYBY,
	DSTR_MENUGENERAL_SVURL,
	DSTR_MENUGENERAL_SVWADURL,
	DSTR_MENUGENERAL_VIDDRAWFPS,
	DSTR_MENUGENERAL_VIDSCREENLINK,
	
	DSTR_UIGENERAL_DONTCARE,
	DSTR_UIGENERAL_YES,
	DSTR_UIGENERAL_NO,
	
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
	DSTR_CVHINT_CONFORECOLOR,
	DSTR_CVHINT_CONFONT,
	DSTR_CVHINT_CONMONOSPACE,
	DSTR_CVHINT_CONSCALE,
	DSTR_CVHINT_CONTESTSTRING,
	DSTR_CVHINT_CONPAUSEGAME,
	
	DSTR_CVHINT_MENUFONT,
	DSTR_CVHINT_MENUITEMCOLOR,
	DSTR_CVHINT_MENUVALSCOLOR,
	DSTR_CVHINT_MENUHEADERCOLOR,
	
	DSTR_CVHINT_SVNAME,
	DSTR_CVHINT_SVEMAIL,
	DSTR_CVHINT_SVURL,
	DSTR_CVHINT_SVWADURL,
	DSTR_CVHINT_SVIRC,
	DSTR_CVHINT_SVMOTD,
	DSTR_CVHINT_SVCONNECTPASSWORD,
	DSTR_CVHINT_SVJOINPASSWORD,
	DSTR_CVHINT_SVMAXCLIENTS,
	DSTR_CVHINT_SVREADYBY,
	DSTR_CVHINT_SVMAXCATCHUP,
	DSTR_CVHINT_SVMAXDEMOCATCHUP,
	DSTR_CVHINT_SVLAGTHRESHEXPIRE,
	
	DSTR_CVHINT_CLMAXPTRIES,
	DSTR_CVHINT_CLMAXPTRYTIME,
	
	DSTR_CVHINT_RCUTWALLDETAIL,
	DSTR_CVHINT_RFAKESSPAL,
	DSTR_CVHINT_RRENDERER,
	DSTR_CVHINT_RVIEWSIZE,
	DSTR_CVHINT_RTRANSPARENCY,
	DSTR_CVHINT_RDRAWSPLATS,
	DSTR_CVHINT_RMAXSPLATS,
	
	DSTR_CVHINT_SCRFULLSCREEN,
	DSTR_CVHINT_SCRWIDTH,
	DSTR_CVHINT_SCRHEIGHT,
	
	DSTR_CVHINT_VIDSCREENLINK,
	DSTR_CVHINT_VIDDRAWFPS,
	
	DSTR_CVHINT_IENABLEMOUSE,
	DSTR_CVHINT_IENABLEJOYSTICK,
	DSTR_CVHINT_IOSSMIDIDEV,
	
	DSTR_CVHINT_SNDSPEAKERSETUP,
	DSTR_CVHINT_SNDQUALITY,
	DSTR_CVHINT_SNDDENSITY,
	DSTR_CVHINT_SNDBUFFERSIZE,
	DSTR_CVHINT_SNDRANDOMPITCH,
	DSTR_CVHINT_SNDCHANNELS,
	DSTR_CVHINT_SNDRESERVEDCHANNELS,
	DSTR_CVHINT_SNDSOUNDVOLUME,
	DSTR_CVHINT_SNDMUSICVOLUME,
	
	/* Network Stuff */
	DSTR_NET_YOUARENOTTHESERVER,
	DSTR_NET_LEVELNOTFOUND,
	DSTR_NET_EXCEEDEDSPLIT,
	DSTR_NET_ATMAXPLAYERS,
	DSTR_NET_BADCLIENT,
	DSTR_NET_CONNECTINGTOSAMESERVER,
	DSTR_NET_CONNECTNOSOCKET,
	DSTR_NET_CONNECTINGTOSERVER,
	DSTR_NET_RECONNECTYOUARESERVER,
	DSTR_NET_BADHOSTRESOLVE,
	DSTR_NET_CLIENTCONNECTED,
	DSTR_NET_PLAYERJOINED,
	DSTR_NET_CLIENTGONE,
	DSTR_NET_NOREASON,
	
	DSTR_WFGS_TITLE,
	DSTR_WFGS_PLAYERNAME,
	DSTR_WFGS_PING,
	DSTR_WFGS_DEMOPLAYER,
	DSTR_WFGS_HOST,
	DSTR_WFGS_BOT,
	
	DSTR_WFJW_TITLE,
	DSTR_NET_SERVERDISCON,
	DSTR_NET_KICKED,
	
	/* Deprecated Strings */
	DSTR_DEP_D_DEVSTR,
	DSTR_DEP_D_CDROM,
	DSTR_DEP_PRESSKEY,
	DSTR_DEP_PRESSYN,
	DSTR_DEP_LOADNET,
	DSTR_DEP_QLOADNET,
	DSTR_DEP_QSAVESPOT,
	DSTR_DEP_SAVEDEAD,
	DSTR_DEP_QSPROMPT,
	DSTR_DEP_QLPROMPT,
	DSTR_DEP_NEWGAME,
	DSTR_DEP_NIGHTMARE,
	DSTR_DEP_SWSTRING,
	DSTR_DEP_MSGOFF,
	DSTR_DEP_MSGON,
	DSTR_DEP_NETEND,
	DSTR_DEP_ENDGAME,
	DSTR_DEP_NOENDGAME,
	DSTR_DEP_DOSY,
	DSTR_DEP_DETAILHI,
	DSTR_DEP_DETAILLO,
	DSTR_DEP_GAMMALVL0,
	DSTR_DEP_GAMMALVL1,
	DSTR_DEP_GAMMALVL2,
	DSTR_DEP_GAMMALVL3,
	DSTR_DEP_GAMMALVL4,
	DSTR_DEP_EMPTYSTRING,
	DSTR_DEP_GOTARMOR,
	DSTR_DEP_GOTMEGA,
	DSTR_DEP_GOTHTHBONUS,
	DSTR_DEP_GOTARMBONUS,
	DSTR_DEP_GOTSTIM,
	DSTR_DEP_GOTMEDINEED,
	DSTR_DEP_GOTMEDIKIT,
	DSTR_DEP_GOTSUPER,
	DSTR_DEP_GOTBLUECARD,
	DSTR_DEP_GOTYELWCARD,
	DSTR_DEP_GOTREDCARD,
	DSTR_DEP_GOTBLUESKUL,
	DSTR_DEP_GOTYELWSKUL,
	DSTR_DEP_GOTREDSKULL,
	DSTR_DEP_GOTINVUL,
	DSTR_DEP_GOTBERSERK,
	DSTR_DEP_GOTINVIS,
	DSTR_DEP_GOTSUIT,
	DSTR_DEP_GOTMAP,
	DSTR_DEP_GOTVISOR,
	DSTR_DEP_GOTMSPHERE,
	DSTR_DEP_GOTCLIP,
	DSTR_DEP_GOTCLIPBOX,
	DSTR_DEP_GOTROCKET,
	DSTR_DEP_GOTROCKBOX,
	DSTR_DEP_GOTCELL,
	DSTR_DEP_GOTCELLBOX,
	DSTR_DEP_GOTSHELLS,
	DSTR_DEP_GOTSHELLBOX,
	DSTR_DEP_GOTBACKPACK,
	DSTR_DEP_GOTBFG9000,
	DSTR_DEP_GOTCHAINGUN,
	DSTR_DEP_GOTCHAINSAW,
	DSTR_DEP_GOTLAUNCHER,
	DSTR_DEP_GOTPLASMA,
	DSTR_DEP_GOTSHOTGUN,
	DSTR_DEP_GOTSHOTGUN2,
	DSTR_DEP_PD_BLUEO,
	DSTR_DEP_PD_REDO,
	DSTR_DEP_PD_YELLOWO,
	DSTR_DEP_PD_BLUEK,
	DSTR_DEP_PD_REDK,
	DSTR_DEP_PD_YELLOWK,
	DSTR_DEP_GGSAVED,
	DSTR_DEP_HUSTR_MSGU,
	DSTR_DEP_HUSTR_E1M1,
	DSTR_DEP_HUSTR_E1M2,
	DSTR_DEP_HUSTR_E1M3,
	DSTR_DEP_HUSTR_E1M4,
	DSTR_DEP_HUSTR_E1M5,
	DSTR_DEP_HUSTR_E1M6,
	DSTR_DEP_HUSTR_E1M7,
	DSTR_DEP_HUSTR_E1M8,
	DSTR_DEP_HUSTR_E1M9,
	DSTR_DEP_HUSTR_E2M1,
	DSTR_DEP_HUSTR_E2M2,
	DSTR_DEP_HUSTR_E2M3,
	DSTR_DEP_HUSTR_E2M4,
	DSTR_DEP_HUSTR_E2M5,
	DSTR_DEP_HUSTR_E2M6,
	DSTR_DEP_HUSTR_E2M7,
	DSTR_DEP_HUSTR_E2M8,
	DSTR_DEP_HUSTR_E2M9,
	DSTR_DEP_HUSTR_E3M1,
	DSTR_DEP_HUSTR_E3M2,
	DSTR_DEP_HUSTR_E3M3,
	DSTR_DEP_HUSTR_E3M4,
	DSTR_DEP_HUSTR_E3M5,
	DSTR_DEP_HUSTR_E3M6,
	DSTR_DEP_HUSTR_E3M7,
	DSTR_DEP_HUSTR_E3M8,
	DSTR_DEP_HUSTR_E3M9,
	DSTR_DEP_HUSTR_E4M1,
	DSTR_DEP_HUSTR_E4M2,
	DSTR_DEP_HUSTR_E4M3,
	DSTR_DEP_HUSTR_E4M4,
	DSTR_DEP_HUSTR_E4M5,
	DSTR_DEP_HUSTR_E4M6,
	DSTR_DEP_HUSTR_E4M7,
	DSTR_DEP_HUSTR_E4M8,
	DSTR_DEP_HUSTR_E4M9,
	DSTR_DEP_HUSTR_1,
	DSTR_DEP_HUSTR_2,
	DSTR_DEP_HUSTR_3,
	DSTR_DEP_HUSTR_4,
	DSTR_DEP_HUSTR_5,
	DSTR_DEP_HUSTR_6,
	DSTR_DEP_HUSTR_7,
	DSTR_DEP_HUSTR_8,
	DSTR_DEP_HUSTR_9,
	DSTR_DEP_HUSTR_10,
	DSTR_DEP_HUSTR_11,
	DSTR_DEP_HUSTR_12,
	DSTR_DEP_HUSTR_13,
	DSTR_DEP_HUSTR_14,
	DSTR_DEP_HUSTR_15,
	DSTR_DEP_HUSTR_16,
	DSTR_DEP_HUSTR_17,
	DSTR_DEP_HUSTR_18,
	DSTR_DEP_HUSTR_19,
	DSTR_DEP_HUSTR_20,
	DSTR_DEP_HUSTR_21,
	DSTR_DEP_HUSTR_22,
	DSTR_DEP_HUSTR_23,
	DSTR_DEP_HUSTR_24,
	DSTR_DEP_HUSTR_25,
	DSTR_DEP_HUSTR_26,
	DSTR_DEP_HUSTR_27,
	DSTR_DEP_HUSTR_28,
	DSTR_DEP_HUSTR_29,
	DSTR_DEP_HUSTR_30,
	DSTR_DEP_HUSTR_31,
	DSTR_DEP_HUSTR_32,
	DSTR_DEP_PHUSTR_1,
	DSTR_DEP_PHUSTR_2,
	DSTR_DEP_PHUSTR_3,
	DSTR_DEP_PHUSTR_4,
	DSTR_DEP_PHUSTR_5,
	DSTR_DEP_PHUSTR_6,
	DSTR_DEP_PHUSTR_7,
	DSTR_DEP_PHUSTR_8,
	DSTR_DEP_PHUSTR_9,
	DSTR_DEP_PHUSTR_10,
	DSTR_DEP_PHUSTR_11,
	DSTR_DEP_PHUSTR_12,
	DSTR_DEP_PHUSTR_13,
	DSTR_DEP_PHUSTR_14,
	DSTR_DEP_PHUSTR_15,
	DSTR_DEP_PHUSTR_16,
	DSTR_DEP_PHUSTR_17,
	DSTR_DEP_PHUSTR_18,
	DSTR_DEP_PHUSTR_19,
	DSTR_DEP_PHUSTR_20,
	DSTR_DEP_PHUSTR_21,
	DSTR_DEP_PHUSTR_22,
	DSTR_DEP_PHUSTR_23,
	DSTR_DEP_PHUSTR_24,
	DSTR_DEP_PHUSTR_25,
	DSTR_DEP_PHUSTR_26,
	DSTR_DEP_PHUSTR_27,
	DSTR_DEP_PHUSTR_28,
	DSTR_DEP_PHUSTR_29,
	DSTR_DEP_PHUSTR_30,
	DSTR_DEP_PHUSTR_31,
	DSTR_DEP_PHUSTR_32,
	DSTR_DEP_THUSTR_1,
	DSTR_DEP_THUSTR_2,
	DSTR_DEP_THUSTR_3,
	DSTR_DEP_THUSTR_4,
	DSTR_DEP_THUSTR_5,
	DSTR_DEP_THUSTR_6,
	DSTR_DEP_THUSTR_7,
	DSTR_DEP_THUSTR_8,
	DSTR_DEP_THUSTR_9,
	DSTR_DEP_THUSTR_10,
	DSTR_DEP_THUSTR_11,
	DSTR_DEP_THUSTR_12,
	DSTR_DEP_THUSTR_13,
	DSTR_DEP_THUSTR_14,
	DSTR_DEP_THUSTR_15,
	DSTR_DEP_THUSTR_16,
	DSTR_DEP_THUSTR_17,
	DSTR_DEP_THUSTR_18,
	DSTR_DEP_THUSTR_19,
	DSTR_DEP_THUSTR_20,
	DSTR_DEP_THUSTR_21,
	DSTR_DEP_THUSTR_22,
	DSTR_DEP_THUSTR_23,
	DSTR_DEP_THUSTR_24,
	DSTR_DEP_THUSTR_25,
	DSTR_DEP_THUSTR_26,
	DSTR_DEP_THUSTR_27,
	DSTR_DEP_THUSTR_28,
	DSTR_DEP_THUSTR_29,
	DSTR_DEP_THUSTR_30,
	DSTR_DEP_THUSTR_31,
	DSTR_DEP_THUSTR_32,
	DSTR_DEP_HUSTR_CHATMACRO1,
	DSTR_DEP_HUSTR_CHATMACRO2,
	DSTR_DEP_HUSTR_CHATMACRO3,
	DSTR_DEP_HUSTR_CHATMACRO4,
	DSTR_DEP_HUSTR_CHATMACRO5,
	DSTR_DEP_HUSTR_CHATMACRO6,
	DSTR_DEP_HUSTR_CHATMACRO7,
	DSTR_DEP_HUSTR_CHATMACRO8,
	DSTR_DEP_HUSTR_CHATMACRO9,
	DSTR_DEP_HUSTR_CHATMACRO0,
	DSTR_DEP_HUSTR_TALKTOSELF1,
	DSTR_DEP_HUSTR_TALKTOSELF2,
	DSTR_DEP_HUSTR_TALKTOSELF3,
	DSTR_DEP_HUSTR_TALKTOSELF4,
	DSTR_DEP_HUSTR_TALKTOSELF5,
	DSTR_DEP_HUSTR_MESSAGESENT,
	DSTR_DEP_AMSTR_FOLLOWON,
	DSTR_DEP_AMSTR_FOLLOWOFF,
	DSTR_DEP_AMSTR_GRIDON,
	DSTR_DEP_AMSTR_GRIDOFF,
	DSTR_DEP_AMSTR_MARKEDSPOT,
	DSTR_DEP_AMSTR_MARKSCLEARED,
	DSTR_DEP_STSTR_MUS,
	DSTR_DEP_STSTR_NOMUS,
	DSTR_DEP_STSTR_DQDON,
	DSTR_DEP_STSTR_DQDOFF,
	DSTR_DEP_STSTR_KFAADDED,
	DSTR_DEP_STSTR_FAADDED,
	DSTR_DEP_STSTR_NCON,
	DSTR_DEP_STSTR_NCOFF,
	DSTR_DEP_STSTR_BEHOLD,
	DSTR_DEP_STSTR_BEHOLDX,
	DSTR_DEP_STSTR_CHOPPERS,
	DSTR_DEP_STSTR_CLEV,
	DSTR_DEP_E1TEXT,
	DSTR_DEP_E2TEXT,
	DSTR_DEP_E3TEXT,
	DSTR_DEP_E4TEXT,
	DSTR_DEP_C1TEXT,
	DSTR_DEP_C2TEXT,
	DSTR_DEP_C3TEXT,
	DSTR_DEP_C4TEXT,
	DSTR_DEP_C5TEXT,
	DSTR_DEP_C6TEXT,
	DSTR_DEP_T1TEXT,
	DSTR_DEP_T2TEXT,
	DSTR_DEP_T3TEXT,
	DSTR_DEP_T4TEXT,
	DSTR_DEP_T5TEXT,
	DSTR_DEP_T6TEXT,
	DSTR_DEP_CC_ZOMBIE,
	DSTR_DEP_CC_SHOTGUN,
	DSTR_DEP_CC_HEAVY,
	DSTR_DEP_CC_IMP,
	DSTR_DEP_CC_DEMON,
	DSTR_DEP_CC_LOST,
	DSTR_DEP_CC_CACO,
	DSTR_DEP_CC_HELL,
	DSTR_DEP_CC_BARON,
	DSTR_DEP_CC_ARACH,
	DSTR_DEP_CC_PAIN,
	DSTR_DEP_CC_REVEN,
	DSTR_DEP_CC_MANCU,
	DSTR_DEP_CC_ARCH,
	DSTR_DEP_CC_SPIDER,
	DSTR_DEP_CC_CYBER,
	DSTR_DEP_CC_HERO,
	DSTR_DEP_QUITMSG,
	DSTR_DEP_QUITMSG1,
	DSTR_DEP_QUITMSG2,
	DSTR_DEP_QUITMSG3,
	DSTR_DEP_QUITMSG4,
	DSTR_DEP_QUITMSG5,
	DSTR_DEP_QUITMSG6,
	DSTR_DEP_QUITMSG7,
	DSTR_DEP_QUIT2MSG,
	DSTR_DEP_QUIT2MSG1,
	DSTR_DEP_QUIT2MSG2,
	DSTR_DEP_QUIT2MSG3,
	DSTR_DEP_QUIT2MSG4,
	DSTR_DEP_QUIT2MSG5,
	DSTR_DEP_QUIT2MSG6,
	DSTR_DEP_FLOOR4_8,
	DSTR_DEP_SFLR6_1,
	DSTR_DEP_MFLR8_4,
	DSTR_DEP_MFLR8_3,
	DSTR_DEP_SLIME16,
	DSTR_DEP_RROCK14,
	DSTR_DEP_RROCK07,
	DSTR_DEP_RROCK17,
	DSTR_DEP_RROCK13,
	DSTR_DEP_RROCK19,
	DSTR_DEP_CREDIT,
	DSTR_DEP_HELP2,
	DSTR_DEP_VICTORY2,
	DSTR_DEP_ENDPIC,
	DSTR_DEP_MODIFIED,
	DSTR_DEP_SHAREWARE,
	DSTR_DEP_COMERCIAL,
	DSTR_DEP_AUSTIN,
	DSTR_DEP_M_LOAD,
	DSTR_DEP_Z_INIT,
	DSTR_DEP_W_INIT,
	DSTR_DEP_M_INIT,
	DSTR_DEP_R_INIT,
	DSTR_DEP_P_INIT,
	DSTR_DEP_I_INIT,
	DSTR_DEP_D_CHECKNET,
	DSTR_DEP_S_SETSOUND,
	DSTR_DEP_HU_INIT,
	DSTR_DEP_ST_INIT,
	DSTR_DEP_STATREG,
	DSTR_DEP_DOOM2WAD,
	DSTR_DEP_DOOMUWAD,
	DSTR_DEP_DOOMWAD,
	DSTR_DEP_DOOM1WAD,
	DSTR_DEP_CDROM_DIR,
	DSTR_DEP_CDROM_DEF,
	DSTR_DEP_CDROM_SAVE,
	DSTR_DEP_NORM_SAVE,
	DSTR_DEP_CDROM_SAVEI,
	DSTR_DEP_NORM_SAVEI,
	DSTR_DEP_PD_BLUEC,
	DSTR_DEP_PD_REDC,
	DSTR_DEP_PD_YELLOWC,
	DSTR_DEP_PD_BLUES,
	DSTR_DEP_PD_REDS,
	DSTR_DEP_PD_YELLOWS,
	DSTR_DEP_PD_ANY,
	DSTR_DEP_PD_ALL3,
	DSTR_DEP_PD_ALL6,
	DSTR_DEP_TXT_ARTIHEALTH,
	DSTR_DEP_TXT_ARTIFLY,
	DSTR_DEP_TXT_ARTIINVULNERABILITY,
	DSTR_DEP_TXT_ARTITOMEOFPOWER,
	DSTR_DEP_TXT_ARTIINVISIBILITY,
	DSTR_DEP_TXT_ARTIEGG,
	DSTR_DEP_TXT_ARTISUPERHEALTH,
	DSTR_DEP_TXT_ARTITORCH,
	DSTR_DEP_TXT_ARTIFIREBOMB,
	DSTR_DEP_TXT_ARTITELEPORT,
	DSTR_DEP_TXT_AMMOGOLDWAND1,
	DSTR_DEP_TXT_AMMOGOLDWAND2,
	DSTR_DEP_TXT_AMMOMACE1,
	DSTR_DEP_TXT_AMMOMACE2,
	DSTR_DEP_TXT_AMMOCROSSBOW1,
	DSTR_DEP_TXT_AMMOCROSSBOW2,
	DSTR_DEP_TXT_AMMOBLASTER1,
	DSTR_DEP_TXT_AMMOBLASTER2,
	DSTR_DEP_TXT_AMMOSKULLROD1,
	DSTR_DEP_TXT_AMMOSKULLROD2,
	DSTR_DEP_TXT_AMMOPHOENIXROD1,
	DSTR_DEP_TXT_AMMOPHOENIXROD2,
	DSTR_DEP_TXT_WPNMACE,
	DSTR_DEP_TXT_WPNCROSSBOW,
	DSTR_DEP_TXT_WPNBLASTER,
	DSTR_DEP_TXT_WPNSKULLROD,
	DSTR_DEP_TXT_WPNPHOENIXROD,
	DSTR_DEP_TXT_WPNGAUNTLETS,
	DSTR_DEP_TXT_ITEMBAGOFHOLDING,
	DSTR_DEP_TXT_CHEATGODON,
	DSTR_DEP_TXT_CHEATGODOFF,
	DSTR_DEP_TXT_CHEATNOCLIPON,
	DSTR_DEP_TXT_CHEATNOCLIPOFF,
	DSTR_DEP_TXT_CHEATWEAPONS,
	DSTR_DEP_TXT_CHEATFLIGHTON,
	DSTR_DEP_TXT_CHEATFLIGHTOFF,
	DSTR_DEP_TXT_CHEATPOWERON,
	DSTR_DEP_TXT_CHEATPOWEROFF,
	DSTR_DEP_TXT_CHEATHEALTH,
	DSTR_DEP_TXT_CHEATKEYS,
	DSTR_DEP_TXT_CHEATSOUNDON,
	DSTR_DEP_TXT_CHEATSOUNDOFF,
	DSTR_DEP_TXT_CHEATTICKERON,
	DSTR_DEP_TXT_CHEATTICKEROFF,
	DSTR_DEP_TXT_CHEATARTIFACTS1,
	DSTR_DEP_TXT_CHEATARTIFACTS2,
	DSTR_DEP_TXT_CHEATARTIFACTS3,
	DSTR_DEP_TXT_CHEATARTIFACTSFAIL,
	DSTR_DEP_TXT_CHEATWARP,
	DSTR_DEP_TXT_CHEATSCREENSHOT,
	DSTR_DEP_TXT_CHEATCHICKENON,
	DSTR_DEP_TXT_CHEATCHICKENOFF,
	DSTR_DEP_TXT_CHEATMASSACRE,
	DSTR_DEP_TXT_CHEATIDDQD,
	DSTR_DEP_TXT_CHEATIDKFA,
	DSTR_DEP_HERETIC_E1M1,
	DSTR_DEP_HERETIC_E1M2,
	DSTR_DEP_HERETIC_E1M3,
	DSTR_DEP_HERETIC_E1M4,
	DSTR_DEP_HERETIC_E1M5,
	DSTR_DEP_HERETIC_E1M6,
	DSTR_DEP_HERETIC_E1M7,
	DSTR_DEP_HERETIC_E1M8,
	DSTR_DEP_HERETIC_E1M9,
	DSTR_DEP_HERETIC_E2M1,
	DSTR_DEP_HERETIC_E2M2,
	DSTR_DEP_HERETIC_E2M3,
	DSTR_DEP_HERETIC_E2M4,
	DSTR_DEP_HERETIC_E2M5,
	DSTR_DEP_HERETIC_E2M6,
	DSTR_DEP_HERETIC_E2M7,
	DSTR_DEP_HERETIC_E2M8,
	DSTR_DEP_HERETIC_E2M9,
	DSTR_DEP_HERETIC_E3M1,
	DSTR_DEP_HERETIC_E3M2,
	DSTR_DEP_HERETIC_E3M3,
	DSTR_DEP_HERETIC_E3M4,
	DSTR_DEP_HERETIC_E3M5,
	DSTR_DEP_HERETIC_E3M6,
	DSTR_DEP_HERETIC_E3M7,
	DSTR_DEP_HERETIC_E3M8,
	DSTR_DEP_HERETIC_E3M9,
	DSTR_DEP_HERETIC_E4M1,
	DSTR_DEP_HERETIC_E4M2,
	DSTR_DEP_HERETIC_E4M3,
	DSTR_DEP_HERETIC_E4M4,
	DSTR_DEP_HERETIC_E4M5,
	DSTR_DEP_HERETIC_E4M6,
	DSTR_DEP_HERETIC_E4M7,
	DSTR_DEP_HERETIC_E4M8,
	DSTR_DEP_HERETIC_E4M9,
	DSTR_DEP_HERETIC_E5M1,
	DSTR_DEP_HERETIC_E5M2,
	DSTR_DEP_HERETIC_E5M3,
	DSTR_DEP_HERETIC_E5M4,
	DSTR_DEP_HERETIC_E5M5,
	DSTR_DEP_HERETIC_E5M6,
	DSTR_DEP_HERETIC_E5M7,
	DSTR_DEP_HERETIC_E5M8,
	DSTR_DEP_HERETIC_E5M9,
	DSTR_DEP_HERETIC_E6M1,
	DSTR_DEP_HERETIC_E6M2,
	DSTR_DEP_HERETIC_E6M3,
	DSTR_DEP_HERETIC_E1TEXT,
	DSTR_DEP_HERETIC_E2TEXT,
	DSTR_DEP_HERETIC_E3TEXT,
	DSTR_DEP_HERETIC_E4TEXT,
	DSTR_DEP_HERETIC_E5TEXT,
	DSTR_DEP_DOOM2TITLE,
	DSTR_DEP_DOOMUTITLE,
	DSTR_DEP_DOOMTITLE,
	DSTR_DEP_DOOM1TITLE,
	
	/* Other Game Stuff */
	DSTR_FOUNDSECRET,
	DSTR_NETPLAYERRENAMED,
	DSTR_GAMEPAUSED,
	DSTR_GAMEUNPAUSED,
	
	/* Items */
	DSTR_ITEM_GOTPRESSRELEASEBFG,
	
	/* Demo Stuff */
	DSTR_BADDEMO_LEVELNOTFOUND,
	DSTR_BADDEMO_LOADGAMENOTSUPPORTED,
	DSTR_BADDEMO_SAVEGAMENOTSUPPORTED,
	DSTR_BADDEMO_NETVARNOTSUPPORTED,
	DSTR_BADDEMO_UNKNOWNXDCMD,
	DSTR_BADDEMO_UNKNOWNFACTORY,
	DSTR_BADDEMO_NONHOSTDEMO,
	DSTR_BADDEMO_ILLEGALHEADER,
	DSTR_BADDEMO_UNHANDLEDDATA,
	DSTR_BADDEMO_SKIPPEDTIC,
	DSTR_BADDEMO_DESYNC,
	
	/* d_prof.c */
	DSTR_DPROFC_CREATEUSAGE,
	DSTR_DPROFC_ALREADYEXIST,
	DSTR_DPROFC_FAILEDCREATE,
	DSTR_DPROFC_CONTROLUSAGE,
	DSTR_DPROFC_MAXISUSAGE,
	DSTR_DPROFC_JAXISUSAGE,
	DSTR_DPROFC_NOTFOUND,
	DSTR_DPROFC_NOTCONTROLNAME,
	DSTR_DPROFC_VALUEUSAGE,
	DSTR_DPROFC_INDEXOUTOFRANGE,
	
	/* d_net.c */
	DSTR_DNETC_SOCKFAILEDTOOPEN,
	DSTR_DNETC_BOUNDTOPORT,
	DSTR_DNETC_CONNECTFROM,
	DSTR_DNETC_CLIENTNOWREADY,
	DSTR_DNETC_CONSISTFAIL,
	DSTR_DNETC_PLEASERECON,
	DSTR_DNETC_JOININGPLAYER,
	DSTR_DNETC_PLAYERLISTENT,
	DSTR_DNETC_BADURI,
	DSTR_DNETC_BINDFAIL,
	DSTR_DNETC_SERVERCHANGENAME,
	DSTR_DNETC_CONNECTTO,
	DSTR_DNETC_SERVERWAD,
	DSTR_DNETC_HOSTNORESOLVE,
	DSTR_DNETC_HOSTRESFOUR,
	DSTR_DNETC_HOSTRESSIX,
	
	/* d_rmod.c */
	DSTR_DRMOD_NAMESPACENOTINWAD,
	DSTR_DRMOD_DATASTREAMERR,
	DSTR_DRMOD_PARSEERROR,
	
	/* command.c */
	DSTR_COMMANDC_WOULDHASHCOLLIDE,
	
	/* st_stuff.c */
	DSTR_STSTUFFC_SCOREBOARD,
	
	/* t_vm.c */
	DSTR_TVMC_COMPILING,
	DSTR_TVMC_ERROR,
	DSTR_TVMC_SUCCESS,
	DSTR_TVMC_GLOBALSCRIPTNUM,
	
	/* b_ghost.c */
	DSTR_BGHOSTC_BASEINIT,
	
	/* d_main.c */
	DSTR_DMAINC_JOYINSTRUCT,
	DSTR_DMAINC_PLAYER1,
	DSTR_DMAINC_PLAYER2,
	DSTR_DMAINC_PLAYER3,
	DSTR_DMAINC_PLAYER4,
	
	/* ip_*.c */
	DSTR_IPC_CREATECONN,
	DSTR_IPC_NOHOSTCANNOTBIND,
	DSTR_IPC_SVREQUESTINFO,
	DSTR_IPC_UNKODA,
	DSTR_IPC_SVGOTINFO,
	DSTR_IPC_SVSERVERSYNC,
	DSTR_IPC_SVUNKENCODING,
	DSTR_IPC_ODAVIRTCVAR,
	DSTR_IPC_CONNECTING,
	DSTR_IPC_NOWFORJOINWINDOW,
	DSTR_IPC_CLCONNECT,
	
	/* w_wad.c */
	DSTR_WWADC_WADSTILLLINKED,
	DSTR_WWADC_CHECKINGTHESUM,
	
	/* info.c */
	DSTR_INFOC_NODEH,
	DSTR_INFOC_NOTADEH,
	DSTR_INFOC_BINARYDEH,
	DSTR_INFOC_OLDPFDEH,
	DSTR_INFOC_DEHNOSPRMAP,
	
	/* d_block.c */
	DSTR_DBLOCKC_ZLIBINFLATEERR,
	
	/* d_nwline.c */
	DSTR_DNWLINE_LOCKEDDOOR,
	
	/* p_spec.c */
	DSTR_PSPECC_TIMELIMITREACHED,
	DSTR_PSPECC_FIVEMINLEFT,
	DSTR_PSPECC_ONEMINLEFT,
	DSTR_PSPECC_THIRTYSECLEFT,
	DSTR_PSPECC_TENSECLEFT,
	DSTR_PSPECC_FIVESECLEFT,
	DSTR_PSPECC_FOURSECLEFT,
	DSTR_PSPECC_THREESECLEFT,
	DSTR_PSPECC_TWOSECLEFT,
	DSTR_PSPECC_ONESECLEFT,
	
	/* p_inter.c */
	DSTR_PINTERC_FRAGLIMITREACHED,
	
	/* p_saveg.c */
	DSTR_PSAVEGC_ENDOFSTREAM,
	DSTR_PSAVEGC_WRONGHEADER,
	DSTR_PSAVEGC_UNKNOWNLEVEL,
	DSTR_PSAVEGC_LEVELLOADFAIL,
	DSTR_PSAVEGC_ILLEGALTHINKER,
	DSTR_PSAVEGC_UNHANDLEDTHINKER,
	
	/* p_demcmp.c */
	DSTR_M_PGS_NOTHINGHERE,
	DSTR_D_PGS_NOTHINGHERE,
	DSTR_M_PGS_COENABLEBLOODSPLATS,
	DSTR_D_PGS_COENABLEBLOODSPLATS,
	DSTR_M_PGS_CORANDOMLASTLOOK,
	DSTR_D_PGS_CORANDOMLASTLOOK,
	DSTR_M_PGS_COUNSHIFTVILERAISE,
	DSTR_D_PGS_COUNSHIFTVILERAISE,
	DSTR_M_PGS_COMODIFYCORPSE,
	DSTR_D_PGS_COMODIFYCORPSE,
	DSTR_M_PGS_CONOSMOKETRAILS,
	DSTR_D_PGS_CONOSMOKETRAILS,
	DSTR_M_PGS_COUSEREALSMOKE,
	DSTR_D_PGS_COUSEREALSMOKE,
	DSTR_M_PGS_COOLDCUTCORPSERADIUS,
	DSTR_D_PGS_COOLDCUTCORPSERADIUS,
	DSTR_M_PGS_COSPAWNDROPSONMOFLOORZ,
	DSTR_D_PGS_COSPAWNDROPSONMOFLOORZ,
	DSTR_M_PGS_CODISABLETEAMPLAY,
	DSTR_D_PGS_CODISABLETEAMPLAY,
	DSTR_M_PGS_COSLOWINWATER,
	DSTR_D_PGS_COSLOWINWATER,
	DSTR_M_PGS_COSLIDEOFFMOFLOOR,
	DSTR_D_PGS_COSLIDEOFFMOFLOOR,
	DSTR_M_PGS_COOLDFRICTIONMOVE,
	DSTR_D_PGS_COOLDFRICTIONMOVE,
	DSTR_M_PGS_COOUCHONCEILING,
	DSTR_D_PGS_COOUCHONCEILING,
	DSTR_M_PGS_COENABLESPLASHES,
	DSTR_D_PGS_COENABLESPLASHES,
	DSTR_M_PGS_COENABLEFLOORSMOKE,
	DSTR_D_PGS_COENABLEFLOORSMOKE,
	DSTR_M_PGS_COENABLESMOKE,
	DSTR_D_PGS_COENABLESMOKE,
	DSTR_M_PGS_CODAMAGEONLAND,
	DSTR_D_PGS_CODAMAGEONLAND,
	DSTR_M_PGS_COABSOLUTEANGLE,
	DSTR_D_PGS_COABSOLUTEANGLE,
	DSTR_M_PGS_COOLDJUMPOVER,
	DSTR_D_PGS_COOLDJUMPOVER,
	DSTR_M_PGS_COENABLESPLATS,
	DSTR_D_PGS_COENABLESPLATS,
	DSTR_M_PGS_COOLDFLATPUSHERCODE,
	DSTR_D_PGS_COOLDFLATPUSHERCODE,
	DSTR_M_PGS_COSPAWNPLAYERSEARLY,
	DSTR_D_PGS_COSPAWNPLAYERSEARLY,
	DSTR_M_PGS_COENABLEUPDOWNSHOOT,
	DSTR_D_PGS_COENABLEUPDOWNSHOOT,
	DSTR_M_PGS_CONOUNDERWATERCHECK,
	DSTR_D_PGS_CONOUNDERWATERCHECK,
	DSTR_M_PGS_COSPLASHTRANSWATER,
	DSTR_D_PGS_COSPLASHTRANSWATER,
	DSTR_M_PGS_COUSEOLDZCHECK,
	DSTR_D_PGS_COUSEOLDZCHECK,
	DSTR_M_PGS_COCHECKXYMOVE,
	DSTR_D_PGS_COCHECKXYMOVE,
	DSTR_M_PGS_COWATERZFRICTION,
	DSTR_D_PGS_COWATERZFRICTION,
	DSTR_M_PGS_CORANOMLASTLOOKSPAWN,
	DSTR_D_PGS_CORANOMLASTLOOKSPAWN,
	DSTR_M_PGS_COALWAYSRETURNDEADSPMISSILE,
	DSTR_D_PGS_COALWAYSRETURNDEADSPMISSILE,
	DSTR_M_PGS_COUSEMOUSEAIMING,
	DSTR_D_PGS_COUSEMOUSEAIMING,
	DSTR_M_PGS_COFIXPLAYERMISSILEANGLE,
	DSTR_D_PGS_COFIXPLAYERMISSILEANGLE,
	DSTR_M_PGS_COREMOVEMOINSKYZ,
	DSTR_D_PGS_COREMOVEMOINSKYZ,
	DSTR_M_PGS_COFORCEAUTOAIM,
	DSTR_D_PGS_COFORCEAUTOAIM,
	DSTR_M_PGS_COFORCEBERSERKSWITCH,
	DSTR_D_PGS_COFORCEBERSERKSWITCH,
	DSTR_M_PGS_CODOUBLEPICKUPCHECK,
	DSTR_D_PGS_CODOUBLEPICKUPCHECK,
	DSTR_M_PGS_CODISABLEMISSILEIMPACTCHECK,
	DSTR_D_PGS_CODISABLEMISSILEIMPACTCHECK,
	DSTR_M_PGS_COMISSILESPLATONWALL,
	DSTR_D_PGS_COMISSILESPLATONWALL,
	DSTR_M_PGS_CONEWBLOODHITSCANCODE,
	DSTR_D_PGS_CONEWBLOODHITSCANCODE,
	DSTR_M_PGS_CONEWAIMINGCODE,
	DSTR_D_PGS_CONEWAIMINGCODE,
	DSTR_M_PGS_COMISSILESPECHIT,
	DSTR_D_PGS_COMISSILESPECHIT,
	DSTR_M_PGS_COHITSCANSSLIDEONFLATS,
	DSTR_D_PGS_COHITSCANSSLIDEONFLATS,
	DSTR_M_PGS_CONONSOLIDPASSTHRUOLD,
	DSTR_D_PGS_CONONSOLIDPASSTHRUOLD,
	DSTR_M_PGS_CONONSOLIDPASSTHRUNEW,
	DSTR_D_PGS_CONONSOLIDPASSTHRUNEW,
	DSTR_M_PGS_COJUMPCHECK,
	DSTR_D_PGS_COJUMPCHECK,
	DSTR_M_PGS_COLINEARMAPTRAVERSE,
	DSTR_D_PGS_COLINEARMAPTRAVERSE,
	DSTR_M_PGS_COONLYTWENTYDMSPOTS,
	DSTR_D_PGS_COONLYTWENTYDMSPOTS,
	DSTR_M_PGS_COALLOWSTUCKSPAWNS,
	DSTR_D_PGS_COALLOWSTUCKSPAWNS,
	DSTR_M_PGS_COUSEOLDBLOOD,
	DSTR_D_PGS_COUSEOLDBLOOD,
	DSTR_M_PGS_FUNMONSTERFFA,
	DSTR_D_PGS_FUNMONSTERFFA,
	DSTR_M_PGS_FUNINFIGHTING,
	DSTR_D_PGS_FUNINFIGHTING,
	DSTR_M_PGS_COCORRECTVILETARGET,
	DSTR_D_PGS_COCORRECTVILETARGET,
	DSTR_M_PGS_FUNMONSTERSMISSMORE,
	DSTR_D_PGS_FUNMONSTERSMISSMORE,
	DSTR_M_PGS_COMORECRUSHERBLOOD,
	DSTR_D_PGS_COMORECRUSHERBLOOD,
	DSTR_M_PGS_CORANDOMBLOODDIR,
	DSTR_D_PGS_CORANDOMBLOODDIR,
	DSTR_M_PGS_COINFINITEROCKETZ,
	DSTR_D_PGS_COINFINITEROCKETZ,
	DSTR_M_PGS_COALLOWROCKETJUMPING,
	DSTR_D_PGS_COALLOWROCKETJUMPING,
	DSTR_M_PGS_COROCKETZTHRUST,
	DSTR_D_PGS_COROCKETZTHRUST,
	DSTR_M_PGS_COLIMITMONSTERZMATTACK,
	DSTR_D_PGS_COLIMITMONSTERZMATTACK,
	DSTR_M_PGS_HEREMONSTERTHRESH,
	DSTR_D_PGS_HEREMONSTERTHRESH,
	DSTR_M_PGS_COVOODOODOLLS,
	DSTR_D_PGS_COVOODOODOLLS,
	DSTR_M_PGS_COEXTRATRAILPUFF,
	DSTR_D_PGS_COEXTRATRAILPUFF,
	DSTR_M_PGS_COLOSTSOULTRAILS,
	DSTR_D_PGS_COLOSTSOULTRAILS,
	DSTR_M_PGS_COTRANSTWOSIDED,
	DSTR_D_PGS_COTRANSTWOSIDED,
	DSTR_M_PGS_COENABLEBLOODTIME,
	DSTR_D_PGS_COENABLEBLOODTIME,
	DSTR_M_PGS_PLENABLEJUMPING,
	DSTR_D_PGS_PLENABLEJUMPING,
	DSTR_M_PGS_COMOUSEAIM,
	DSTR_D_PGS_COMOUSEAIM,
	DSTR_M_PGS_MONRESPAWNMONSTERS,
	DSTR_D_PGS_MONRESPAWNMONSTERS,
	DSTR_M_PGS_FUNNOTARGETPLAYER,
	DSTR_D_PGS_FUNNOTARGETPLAYER,
	DSTR_M_PGS_MONARCHVILEANYRESPAWN,
	DSTR_D_PGS_MONARCHVILEANYRESPAWN,
	DSTR_M_PGS_COOLDCHECKPOSITION,
	DSTR_D_PGS_COOLDCHECKPOSITION,
	DSTR_M_PGS_COLESSSPAWNSTICKING,
	DSTR_D_PGS_COLESSSPAWNSTICKING,
	DSTR_M_PGS_PLSPAWNTELEFRAG,
	DSTR_D_PGS_PLSPAWNTELEFRAG,
	DSTR_M_PGS_GAMEONEHITKILLS,
	DSTR_D_PGS_GAMEONEHITKILLS,
	DSTR_M_PGS_COBETTERPLCORPSEREMOVAL,
	DSTR_D_PGS_COBETTERPLCORPSEREMOVAL,
	DSTR_M_PGS_PLSPAWNCLUSTERING,
	DSTR_D_PGS_PLSPAWNCLUSTERING,
	DSTR_M_PGS_COIMPROVEDMOBJONMOBJ,
	DSTR_D_PGS_COIMPROVEDMOBJONMOBJ,
	DSTR_M_PGS_COIMPROVEPATHTRAVERSE,
	DSTR_D_PGS_COIMPROVEPATHTRAVERSE,
	DSTR_M_PGS_PLJUMPGRAVITY,
	DSTR_D_PGS_PLJUMPGRAVITY,
	DSTR_M_PGS_FUNNOLOCKEDDOORS,
	DSTR_D_PGS_FUNNOLOCKEDDOORS,
	DSTR_M_PGS_GAMEAIRFRICTION,
	DSTR_D_PGS_GAMEAIRFRICTION,
	DSTR_M_PGS_GAMEWATERFRICTION,
	DSTR_D_PGS_GAMEWATERFRICTION,
	DSTR_M_PGS_GAMEMIDWATERFRICTION,
	DSTR_D_PGS_GAMEMIDWATERFRICTION,
	DSTR_M_PGS_GAMEALLOWLEVELEXIT,
	DSTR_D_PGS_GAMEALLOWLEVELEXIT,
	DSTR_M_PGS_GAMEALLOWROCKETJUMP,
	DSTR_D_PGS_GAMEALLOWROCKETJUMP,
	DSTR_M_PGS_PLALLOWAUTOAIM,
	DSTR_D_PGS_PLALLOWAUTOAIM,
	DSTR_M_PGS_PLFORCEWEAPONSWITCH,
	DSTR_D_PGS_PLFORCEWEAPONSWITCH,
	DSTR_M_PGS_PLDROPWEAPONS,
	DSTR_D_PGS_PLDROPWEAPONS,
	DSTR_M_PGS_PLINFINITEAMMO,
	DSTR_D_PGS_PLINFINITEAMMO,
	DSTR_M_PGS_GAMEHERETICGIBBING,
	DSTR_D_PGS_GAMEHERETICGIBBING,
	DSTR_M_PGS_MONPREDICTMISSILES,
	DSTR_D_PGS_MONPREDICTMISSILES,
	DSTR_M_PGS_MONRESPAWNMONSTERSTIME,
	DSTR_D_PGS_MONRESPAWNMONSTERSTIME,
	DSTR_M_PGS_PLSPAWNWITHMAXGUNS,
	DSTR_D_PGS_PLSPAWNWITHMAXGUNS,
	DSTR_M_PGS_PLSPAWNWITHSUPERGUNS,
	DSTR_D_PGS_PLSPAWNWITHSUPERGUNS,
	DSTR_M_PGS_PLSPAWNWITHMAXSTATS,
	DSTR_D_PGS_PLSPAWNWITHMAXSTATS,
	DSTR_M_PGS_ITEMSSPAWNPICKUPS,
	DSTR_D_PGS_ITEMSSPAWNPICKUPS,
	DSTR_M_PGS_COHERETICFRICTION,
	DSTR_D_PGS_COHERETICFRICTION,
	DSTR_M_PGS_GAMEDEATHMATCH,
	DSTR_D_PGS_GAMEDEATHMATCH,
	DSTR_M_PGS_PLSPAWNWITHALLKEYS,
	DSTR_D_PGS_PLSPAWNWITHALLKEYS,
	DSTR_M_PGS_ITEMSKEEPWEAPONS,
	DSTR_D_PGS_ITEMSKEEPWEAPONS,
	DSTR_M_PGS_GAMETEAMPLAY,
	DSTR_D_PGS_GAMETEAMPLAY,
	DSTR_M_PGS_GAMETEAMDAMAGE,
	DSTR_D_PGS_GAMETEAMDAMAGE,
	DSTR_M_PGS_GAMEFRAGLIMIT,
	DSTR_D_PGS_GAMEFRAGLIMIT,
	DSTR_M_PGS_GAMETIMELIMIT,
	DSTR_D_PGS_GAMETIMELIMIT,
	DSTR_M_PGS_MONSTATICRESPAWNTIME,
	DSTR_D_PGS_MONSTATICRESPAWNTIME,
	DSTR_M_PGS_PLFASTERWEAPONS,
	DSTR_D_PGS_PLFASTERWEAPONS,
	DSTR_M_PGS_MONSPAWNMONSTERS,
	DSTR_D_PGS_MONSPAWNMONSTERS,
	DSTR_M_PGS_GAMESPAWNMULTIPLAYER,
	DSTR_D_PGS_GAMESPAWNMULTIPLAYER,
	DSTR_M_PGS_ITEMRESPAWNITEMS,
	DSTR_D_PGS_ITEMRESPAWNITEMS,
	DSTR_M_PGS_ITEMRESPAWNITEMSTIME,
	DSTR_D_PGS_ITEMRESPAWNITEMSTIME,
	DSTR_M_PGS_MONFASTMONSTERS,
	DSTR_D_PGS_MONFASTMONSTERS,
	DSTR_M_PGS_GAMESOLIDCORPSES,
	DSTR_D_PGS_GAMESOLIDCORPSES,
	DSTR_M_PGS_GAMEBLOODTIME,
	DSTR_D_PGS_GAMEBLOODTIME,
	DSTR_M_PGS_GAMEGRAVITY,
	DSTR_D_PGS_GAMEGRAVITY,
	DSTR_M_PGS_MONENABLECLEANUP,
	DSTR_D_PGS_MONENABLECLEANUP,
	DSTR_M_PGS_MONCLEANUPRESPTIME,
	DSTR_D_PGS_MONCLEANUPRESPTIME,
	DSTR_M_PGS_MONCLEANUPNONRTIME,
	DSTR_D_PGS_MONCLEANUPNONRTIME,
	DSTR_M_PGS_GAMESKILL,
	DSTR_D_PGS_GAMESKILL,
	DSTR_M_PGS_PLHALFDAMAGE,
	DSTR_D_PGS_PLHALFDAMAGE,
	DSTR_M_PGS_PLDOUBLEAMMO,
	DSTR_D_PGS_PLDOUBLEAMMO,
	DSTR_M_PGS_MONKILLCOUNTMODE,
	DSTR_D_PGS_MONKILLCOUNTMODE,
	DSTR_M_PGS_COOLDBFGSPRAY,
	DSTR_D_PGS_COOLDBFGSPRAY,
	DSTR_M_PGS_COEXPLODEHITFLOOR,
	DSTR_D_PGS_COEXPLODEHITFLOOR,
	DSTR_M_PGS_COBOMBTHRUFLOOR,
	DSTR_D_PGS_COBOMBTHRUFLOOR,
	DSTR_M_PGS_COOLDEXPLOSIONS,
	DSTR_D_PGS_COOLDEXPLOSIONS,
	DSTR_M_PGS_COAIMCHECKFAKEFLOOR,
	DSTR_D_PGS_COAIMCHECKFAKEFLOOR,
	DSTR_M_PGS_CONEWGUNSHOTCODE,
	DSTR_D_PGS_CONEWGUNSHOTCODE,
	DSTR_M_PGS_COSHOOTCHECKFAKEFLOOR,
	DSTR_D_PGS_COSHOOTCHECKFAKEFLOOR,
	DSTR_M_PGS_COSHOOTFLOORCLIPPING,
	DSTR_D_PGS_COSHOOTFLOORCLIPPING,
	DSTR_M_PGS_CONEWSSGSPREAD,
	DSTR_D_PGS_CONEWSSGSPREAD,
	DSTR_M_PGS_COMONSTERLOOKFORMONSTER,
	DSTR_D_PGS_COMONSTERLOOKFORMONSTER,
	DSTR_M_PGS_COOLDTHINGHEIGHTS,
	DSTR_D_PGS_COOLDTHINGHEIGHTS,
	DSTR_M_PGS_COLASTLOOKMAXPLAYERS,
	DSTR_D_PGS_COLASTLOOKMAXPLAYERS,
	DSTR_M_PGS_COMOVECHECKFAKEFLOOR,
	DSTR_D_PGS_COMOVECHECKFAKEFLOOR,
	DSTR_M_PGS_COMULTIPLAYER,
	DSTR_D_PGS_COMULTIPLAYER,
	DSTR_M_PGS_COBOOMSUPPORT,
	DSTR_D_PGS_COBOOMSUPPORT,
	DSTR_M_PGS_PLSPAWNWITHFAVGUN,
	DSTR_D_PGS_PLSPAWNWITHFAVGUN,
	DSTR_M_PGS_CONOSAWFACING,
	DSTR_D_PGS_CONOSAWFACING,
	DSTR_M_PGS_COENABLETEAMMONSTERS,
	DSTR_D_PGS_COENABLETEAMMONSTERS,
	DSTR_M_PGS_COMONSTERDEADTARGET,
	DSTR_D_PGS_COMONSTERDEADTARGET,
	DSTR_M_PGS_COJUMPREGARDLESS,
	DSTR_D_PGS_COJUMPREGARDLESS,
	DSTR_M_PGS_COOLDLASTLOOKLOGIC,
	DSTR_D_PGS_COOLDLASTLOOKLOGIC,
	DSTR_M_PGS_CORADIALSPAWNCHECK,
	DSTR_D_PGS_CORADIALSPAWNCHECK,
	DSTR_M_PGS_MONENABLEPLAYASMONSTER,
	DSTR_D_PGS_MONENABLEPLAYASMONSTER,
	DSTR_M_PGS_COKILLSTOPLAYERONE,
	DSTR_D_PGS_COKILLSTOPLAYERONE,
	DSTR_M_PGS_PLALLOWSUICIDE,
	DSTR_D_PGS_PLALLOWSUICIDE,
	DSTR_M_PGS_PLSUICIDEDELAY,
	DSTR_D_PGS_PLSUICIDEDELAY,
	DSTR_M_PGS_PLSPAWNWITHMELEEONLY,
	DSTR_D_PGS_PLSPAWNWITHMELEEONLY,
	DSTR_M_PGS_PLSPAWNWITHRANDOMGUN,
	DSTR_D_PGS_PLSPAWNWITHRANDOMGUN,
	DSTR_M_PGS_COENABLESLOPES,
	DSTR_D_PGS_COENABLESLOPES,
	DSTR_M_PGS_FUNFLIPLEVELS,
	DSTR_D_PGS_FUNFLIPLEVELS,
	DSTR_M_PGS_PLREDUCEINVENTORY,
	DSTR_D_PGS_PLREDUCEINVENTORY,
	DSTR_M_PGS_CODISPLACESPAWN,
	DSTR_D_PGS_CODISPLACESPAWN,
	DSTR_M_PGS_CORESPAWNCORPSESONLY,
	DSTR_D_PGS_CORESPAWNCORPSESONLY,
	DSTR_M_PGS_CONEWGAMEMODES,
	DSTR_D_PGS_CONEWGAMEMODES,
	DSTR_M_PGS_GAMEMODE,
	DSTR_D_PGS_GAMEMODE,
	DSTR_M_PGS_CTFNEEDFLAGATHOME,
	DSTR_D_PGS_CTFNEEDFLAGATHOME,
	
	NUMUNICODESTRINGS
} UnicodeStringID_t;

/* StringGroupEX_t -- String data holder */
typedef struct StringGroupEX_s
{
	const char* const id;
	char* wcharstr;
	uint32_t Hash;
} StringGroupEX_t;

StringGroupEX_t UnicodeStrings[NUMUNICODESTRINGS];

#define PTROFUNICODESTRING(n) (&(UnicodeStrings[(n)].wcharstr))
#define PTRTOUNICODESTRING(n) (UnicodeStrings[(n)].wcharstr)
#define DS_GetString(n) ((const char*)(UnicodeStrings[(UnicodeStringID_t)(n)].wcharstr))
#define DS_GetStringRef(n) ((const char**)(&UnicodeStrings[(UnicodeStringID_t)(n)].wcharstr))

const char* DS_NameOfString(char** const WCharStr);
const char** DS_FindStringRef(const char* const a_StrID);

size_t D_USPrint(char* const a_OutBuf, const size_t a_OutSize, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr);

#endif							/* __DSTRINGS_H__ */

