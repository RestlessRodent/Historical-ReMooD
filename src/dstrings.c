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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// DESCRIPTION:
//    Globally defined strings.

#include "dstrings.h"
#include "z_zone.h"

/* UnicodeStrings -- Game strings */
StringGroupEX_t UnicodeStrings[NUMUNICODESTRINGS] =
{
	/****** MENUS ******/
	{                           "MENU_NULLSPACE", " "},
	{                           "MENUMAIN_TITLE", "Main Menu"},
	{                        "MENU_MAIN_NEWGAME", "New Game"},
	{                        "MENU_MAIN_ENDGAME", "End Game"},
	{                       "MENU_MAIN_LOADGAME", "Load Game"},
	{                       "MENU_MAIN_SAVEGAME", "Save Game"},
	{                        "MENU_MAIN_OPTIONS", "Options"},
	{                       "MENU_MAIN_PROFILES", "Profiles"},
	{                       "MENU_MAIN_QUITGAME", "Quit Game"},
	{                       "MENU_MAIN_PWADMENU", "PWAD Menu"},
	{"MENU_OPTIONS_TITLE", "Options"},
	{"MENU_OPTIONS_GAMESETTINGS", "Game Settings..."},
	{"MENU_OPTIONS_CONTROLSETTINGS", "Control Settings..."},
	{"MENU_OPTIONS_GRAPHICALSETTINGS", "Graphical Settings..."},
	{"MENU_OPTIONS_AUDIOSETTINGS", "Audio Settings..."},
	{"MENU_OPTIONS_ADVANCEDSETTINGS", "Advanced Settings..."},
	{"MENU_OPTIONS_DISABLETITLESCREENDEMOS", "Disable Title Screen Demos"},
	{"MENU_CONTROLS_TITLE", "Control Settings"},
	{"MENU_CONTROLS_CONTROLSUBTITLE", "*** CONTROLS ***"},
	{"MENU_CONTROLS_ACTIONSPERKEY", "Max actions per key"},
	{"MENU_CONTROLS_PLAYERONECONTROLS", "Default Controls (Player 1)..."},
	{"MENU_CONTROLS_PLAYERTWOCONTROLS", "Default Controls (Player 2)..."},
	{"MENU_CONTROLS_PLAYERTHREECONTROLS", "Default Controls (Player 3)..."},
	{"MENU_CONTROLS_PLAYERFOURCONTROLS", "Default Controls (Player 4)..."},
	{"MENU_CONTROLS_MOUSESUBTITLE", "*** MOUSE ***"},
	{"MENU_CONTROLS_ENABLEMOUSE", "Enable Mouse"},
	{"MENU_CONTROLS_ADVANCEDMOUSE", "Use advanced mouse settings"},
	{"MENU_CONTROLS_BASICSETTINGSSUBSUBTITLE", ">> Basic Settings"},
	{"MENU_CONTROLS_USEMOUSEFREELOOK", "Use mouse to freelook"},
	{"MENU_CONTROLS_USEMOUSEMOVE", "Use mouse to move"},
	{"MENU_CONTROLS_INVERTYAXIS", "Invert Y-Axis"},
	{"MENU_CONTROLS_MOVETURNSENS", "Move/Turn sensitivity"},
	{"MENU_CONTROLS_LOOKUPDOWNSENS", "Look Up/Down sensitivity"},
	{"MENU_CONTROLS_ADVANCEDSETTINGSUBSUBTITLE", ">> Advanced Settings"},
	{"MENU_CONTROLS_XAXISSENS", "X-Axis sensitivity"},
	{"MENU_CONTROLS_YAXISSENS", "Y-Axis sensitivity"},
	{"MENU_CONTROLS_XAXISMOVE", "X-Axis Movement"},
	{"MENU_CONTROLS_YAXISMOVE", "Y-Axis Movement"},
	{"MENU_CONTROLS_XAXISMOVESECONDARY", "X-Axis Movement (Secondary)"},
	{"MENU_CONTROLS_YAXISMOVESECONDARY", "Y-Axis Movement (Secondary)"},
	{"MENU_CONTROLS_STRAFEKEYUSESECONDARY", "Strafe key uses secondary movement"},
	{"MENU_GRAPHICS_TITLE", "Graphical Settings"},
	{"MENU_GRAPHICS_SCREENSUBTITLE", "*** SCREEN ***"},
	{"MENU_GRAPHICS_SETRESOLUTION", "Set Resolution..."},
	{"MENU_GRAPHICS_FULLSCREEN", "Fullscreen"},
	{"MENU_GRAPHICS_BRIGHTNESS", "Brightness"},
	{"MENU_GRAPHICS_SCREENSIZE", "Screen Size"},
	{"MENU_GRAPHICS_SCREENLINK", "Screen Link"},
	{"MENU_GRAPHICS_RENDERERSUBTITLE", "*** RENDERER ***"},
	{"MENU_GRAPHICS_TRANSLUCENCY", "Translucency"},
	{"MENU_GRAPHICS_ENABLEDECALS", "Enable decals"},
	{"MENU_GRAPHICS_MAXDECALS", "Max decales"},
	{"MENU_GRAPHICS_CONSOLESUBTITLE", "*** CONSOLE ***"},
	{"MENU_GRAPHICS_CONSOLESPEED", "Console Speed"},
	{"MENU_GRAPHICS_CONSOLEHEIGHT", "Console Height"},
	{"MENU_GRAPHICS_CONSOLEBACKGROUND", "Console Background"},
	{"MENU_GRAPHICS_MESSAGEDURATION", "Message Duration"},
	{"MENU_GRAPHICS_ECHOMESSAGES", "Echo Messages"},
	{"MENU_GRAPHICS_MENUSUBTITLE", "*** MENU ***"},
	{"MENU_GRAPHICS_CURSORBLINKDURATION", "Cursor Blink Duration"},
	{"MENU_GRAPHICS_HUDSUBTITLE", "*** HUD ***"},
	{"MENU_GRAPHICS_SCALESTATUSBAR", "Scale Status Bar"},
	{"MENU_GRAPHICS_TRANSPARENTSTATUSBAR", "Transparent Status Bar"},
	{"MENU_GRAPHICS_STATUSBARTRANSPARENCYAMOUNT", "Status Bar Transparency Level"},
	{"MENU_GRAPHICS_CROSSHAIR", "Cross-hair"},
	{"MENU_KEYBINDS_TITLE", "Setup Controls"},
	{"MENU_KEYBINDS_MOVEMENTSUBTITLE", "*** MOVEMENT ***"},
	{"MENU_KEYBINDS_FIRE", "Fire"},
	{"MENU_KEYBINDS_ACTIVATE", "Activate"},
	{"MENU_KEYBINDS_MOVEFORWARDS", "Move Forwards"},
	{"MENU_KEYBINDS_MOVEBACKWARDS", "Move Backwards"},
	{"MENU_KEYBINDS_TURNLEFT", "Turn Left"},
	{"MENU_KEYBINDS_TURNRIGHT", "Turn Right"},
	{"MENU_KEYBINDS_RUN", "Run"},
	{"MENU_KEYBINDS_STRAFEON", "Strafe On"},
	{"MENU_KEYBINDS_STRAFELEFT", "Strafe Left"},
	{"MENU_KEYBINDS_STRAFERIGHT", "Strafe Right"},
	{"MENU_KEYBINDS_LOOKUP", "Look Up"},
	{"MENU_KEYBINDS_LOOKDOWN", "Look Down"},
	{"MENU_KEYBINDS_CENTERVIEW", "Center View"},
	{"MENU_KEYBINDS_MOUSELOOK", "Mouselook"},
	{"MENU_KEYBINDS_JUMPFLYUP", "Jump/Fly Up"},
	{"MENU_KEYBINDS_FLYDOWN", "Fly down"},
	{"MENU_KEYBINDS_WEAPONSANDITEMSSUBTITLE", "*** WEAPONS & ITEMS ***"},
	{"MENU_KEYBINDS_SLOTONE", "Fist/Chainsaw/Staff/Gauntlets"},
	{"MENU_KEYBINDS_SLOTTWO", "Pistol/Wand"},
	{"MENU_KEYBINDS_SLOTTHREE", "Shotgun/Super Shotgun/Crossbow"},
	{"MENU_KEYBINDS_SLOTFOUR", "Chaingun/Dragon Claw"},
	{"MENU_KEYBINDS_SLOTFIVE", "Rocket Launcher/Hellstaff"},
	{"MENU_KEYBINDS_SLOTSIX", "Plasma rifle/Phoenix Rod"},
	{"MENU_KEYBINDS_SLOTSEVEN", "BFG/Firemace"},
	{"MENU_KEYBINDS_SLOTEIGHT", "Chainsaw"},
	{"MENU_KEYBINDS_PREVIOUSWEAPON", "Previous Weapon"},
	{"MENU_KEYBINDS_NEXTWEAPON", "Next Weapon"},
	{"MENU_KEYBINDS_BESTWEAPON", "Best Weapon"},
	{"MENU_KEYBINDS_INVENTORYLEFT", "Inventory Left"},
	{"MENU_KEYBINDS_INVENTORYRIGHT", "Inventory Right"},
	{"MENU_KEYBINDS_INVENTORYUSE", "Inventory Use"},
	{"MENU_KEYBINDS_MISCSUBTITLE", "*** MISC ***"},
	{"MENU_KEYBINDS_TALKKEY", "Talk key"},
	{"MENU_KEYBINDS_RANKINGSANDSCORES", "Rankings/Scores"},
	{"MENU_KEYBINDS_TOGGLECONSOLE", "Toggle Console"},
	{"MENU_AUDIO_TITLE", "Audio Settings"},
	{"MENU_AUDIO_OUTPUTSUBTITLE", "*** OUTPUT ***"},
	{"MENU_AUDIO_SOUNDOUTPUT", "Sound Output"},
	{"MENU_AUDIO_SOUNDDEVICE", "Sound Device"},
	{"MENU_AUDIO_MUSICOUTPUT", "Music Output"},
	{"MENU_AUDIO_MUSICDEVICE", "Music Device"},
	{"MENU_AUDIO_QUALITYSUBTITLE", "*** QUALITY ***"},
	{"MENU_AUDIO_SPEAKERSETUP", "Speaker Setup"},
	{"MENU_AUDIO_SAMPLESPERSECOND", "Samples per second"},
	{"MENU_AUDIO_BITSPERSAMPLE", "Bits per sample"},
	{"MENU_AUDIO_FAKEPCSPEAKERWAVEFORM", "Simulated PC Speaker Waveform"},
	{"MENU_AUDIO_VOLUMESUBTITLE", "*** VOLUME ***"},
	{"MENU_AUDIO_SOUNDVOLUME", "Sound Volume"},
	{"MENU_AUDIO_MUSICVOLUME", "Music Volume"},
	{"MENU_AUDIO_MISCSUBTITLE", "*** MISC SETTINGS ***"},
	{"MENU_AUDIO_PRECACHESOUNDS", "Precache Sounds"},
	{"MENU_AUDIO_RANDOMSOUNDPITCH", "Random Sound Pitch"},
	{"MENU_AUDIO_SOUNDCHANNELS", "Sound Channels"},
	{"MENU_AUDIO_RESERVEDSOUNDCHANNELS", "Reserved Sound Channels"},
	{"MENU_AUDIO_MULTITHREADEDSOUND", "Multi-Threaded Sound"},
	{"MENU_AUDIO_MULTITHREADEDMUSIC", "Multi-Threaded Music"},
	{"MENU_AUDIO_RESETSUBTITLE", "*** RESET ***"},
	{"MENU_AUDIO_RESETSOUND", "Reset Sound"},
	{"MENU_AUDIO_RESETMUSIC", "Reset Music"},
	{"MENU_VIDEO_TITLE", "Video Resolution"},
	{"MENU_VIDEO_MODESELECT", "Select Mode (Press 'd' to set as default)"},
	{"MENU_GAME_TITLE", "Game Options"},
	{"MENU_GAME_MULTIPLAYERSUBTITLE", "*** MULTIPLAYER ***"},
	{"MENU_GAME_DEATHMATCHTYPE", "Deathmatch Type"},
	{"MENU_GAME_FRAGLIMIT", "Fraglimit"},
	{"MENU_GAME_TIMELIMIT", "Timelimit"},
	{"MENU_GAME_TEAMSUBTITLE", "*** TEAM ***"},
	{"MENU_GAME_ENABLETEAMPLAY", "Enable Teamplay"},
	{"MENU_GAME_FRIENDLYFIRE", "Friendly Fire"},
	{"MENU_GAME_RESTRICTIONSSUBTITLE", "*** RESTRICTIONS ***"},
	{"MENU_GAME_ALLOWJUMP", "Allow Jump"},
	{"MENU_GAME_ALLOWROCKETJUMP", "Allow Rocket Jump"},
	{"MENU_GAME_ALLOWAUTOAIM", "Allow autoaim"},
	{"MENU_GAME_ALLOWTURBO", "Allow turbo"},
	{"MENU_GAME_ALLOWEXITLEVEL", "Allow exitlevel"},
	{"MENU_GAME_FORCEAUTOAIM", "Force Autoaim"},
	{"MENU_GAME_WEAPONSANDITEMSSUBTITLE", "*** WEAPONS & ITEMS ***"},
	{"MENU_GAME_ENABLEITEMRESPAWN", "Enable Item Respawn"},
	{"MENU_GAME_ITEMRESPAWNTIME", "Item Respawn time"},
	{"MENU_GAME_DROPWEAPONSWHENYOUDIE", "Drop Weapons when you die"},
	{"MENU_GAME_INFINITEAMMO", "Infinite Ammo"},
	{"MENU_GAME_MONSTERSSUBTITLE", "*** MONSTERS ***"},
	{"MENU_GAME_SPAWNMONSTERS", "Spawn Monsters"},
	{"MENU_GAME_ENABLEMONSTERRESPAWN", "Enable Monster Respawn"},
	{"MENU_GAME_MONSTERRESPAWNTIME", "Monster Respawn time"},
	{"MENU_GAME_FASTMONSTERS", "Fast Monsters"},
	{"MENU_GAME_PREDICTINGMONSTERS", "Predicting Monsters"},
	{"MENU_GAME_MISCSUBTITLE", "*** MISC ***"},
	{"MENU_GAME_GRAVITY", "Gravity"},
	{"MENU_GAME_SOLIDCORPSES", "Solid corpses"},
	{"MENU_GAME_BLOODTIME", "BloodTime"},
	{"MENU_GAME_COMPATIBILITYSUBTITLE", "*** COMPATIBILITY ***"},
	{"MENU_GAME_CLASSICBLOOD", "Classic Blood"},
	{"MENU_GAME_CLASSICROCKETEXPLOSIONS", "Classic Rocket Explosions"},
	{"MENU_GAME_CLASSICMONSTERMELEERANGE", "Classic Monster Melee Range"},
	{"MENU_GAME_CLASSICMONSTERLOGIC", "Classic Monster Logic"},
	{"MENU_NEWGAME_TITLE", "New Game"},
	{"MENU_NEWGAME_SINGLEPLAYERSUBTITLE", "*** SINGLE PLAYER ***"},
	{"MENU_NEWGAME_CLASSIC", "Classic..."},
	{"MENU_NEWGAME_CREATEGAME", "Create Game..."},
	{"MENU_NEWGAME_QUICKSTART", "Quick Start..."},
	{"MENU_NEWGAME_MULTIPLAYERSUBTITLE", "*** MULTI PLAYER ***"},
	{"MENU_NEWGAME_SPLITSCREENGAME", "Split Screen Game..."},
	{"MENU_NEWGAME_UDPLANINTERNETGAME", "UDP LAN/Internet Game..."},
	{"MENU_NEWGAME_TCPLANINTERNETGAME", "TCP LAN/Internet Game..."},
	{"MENU_NEWGAME_MODEMGAME", "Modem Game..."},
	{"MENU_NEWGAME_SERIALNULLMODEMGAME", "Serial/Null-Modem Game..."},
	{"MENU_NEWGAME_FORKGAME", "Fork Game..."},
	{"MENU_CLASSICGAME_TITLE", "Classic Game"},
	{"MENU_CLASSICGAME_DOOMSKILLA", "I'm too young to die"},
	{"MENU_CLASSICGAME_DOOMSKILLB", "Hey, not too rough"},
	{"MENU_CLASSICGAME_DOOMSKILLC", "Hurt me plenty"},
	{"MENU_CLASSICGAME_DOOMSKILLD", "Ultra-violence"},
	{"MENU_CLASSICGAME_DOOMSKILLE", "Nightmare!"},
	{"MENU_CLASSICGAME_HERETICSKILLA", "Thou needeth a wet-nurse"},
	{"MENU_CLASSICGAME_HERETICSKILLB", "Yellowbellies-r-us"},
	{"MENU_CLASSICGAME_HERETICSKILLC", "Bringest them oneth"},
	{"MENU_CLASSICGAME_HERETICSKILLD", "Thou art a smite-meister"},
	{"MENU_CLASSICGAME_HERETICSKILLE", "Black plague possesses thee"},
	{"MENU_CLASSICGAME_DOOMEPISODEA", "Knee-Deep In The Dead"},
	{"MENU_CLASSICGAME_DOOMEPISODEB", "The Shores of Hell"},
	{"MENU_CLASSICGAME_DOOMEPISODEC", "Inferno"},
	{"MENU_CLASSICGAME_DOOMEPISODED", "Thy Flesh Consumed"},
	{"MENU_CLASSICGAME_DOOMEPISODEE", "Episode 5"},
	{"MENU_CLASSICGAME_DOOMEPISODEF", "Episode 6"},
	{"MENU_CLASSICGAME_HERETICEPISODEA", "City of the Damned"},
	{"MENU_CLASSICGAME_HERETICEPISODEB", "Hell's Maw"},
	{"MENU_CLASSICGAME_HERETICEPISODEC", "The Dome of D'Sparil"},
	{"MENU_CLASSICGAME_HERETICEPISODED", "The Ossuary"},
	{"MENU_CLASSICGAME_HERETICEPISODEE", "The Stagnant Demesne"},
	{"MENU_CLASSICGAME_HERETICEPISODEF", "Fate's Path"},
	{"MENU_CREATEGAME_SOLOTITLE", "Create Game"},
	{"MENU_CREATEGAME_LOCALTITLE", "Create Local Game"},
	{"MENU_CREATEGAME_LEVEL", "Level"},
	{"MENU_CREATEGAME_SKILL", "Skill"},
	{"MENU_CREATEGAME_SPAWNMONSTERS", "Spawn Monsters"},
	{"MENU_CREATEGAME_OPTIONS", "Options"},
	{"MENU_CREATEGAME_SETUPOPTIONS", "Setup Options..."},
	{"MENU_CREATEGAME_STARTGAME", "Start Game!"},
	{"MENU_CREATEGAME_NUMBEROFPLAYERS", "Number of Players"},
	{"MENU_CREATEGAME_DEATHMATCHTYPE", "Deathmatch Type"},
	{"MENU_CREATEGAME_FRAGLIMIT", "Fraglimit"},
	{"MENU_CREATEGAME_TIMELIMIT", "Timelimit"},
	{"MENU_PROFILES_TITLE", "Setup Profiles"},
	{"MENU_PROFILES_CREATEPROFILE", "Create Profile..."},
	{"MENU_PROFILES_CURRENTPROFILE", "Current Profile"},
	{"MENU_PROFILES_NAME", "Name"},
	{"MENU_PROFILES_COLOR", "Color"},
	{"MENU_PROFILES_SKIN", "Skin"},
	{"MENU_PROFILES_AUTOAIM", "Autoaim"},
	{"MENU_CREATEPROFILE_TITLE", "Create Profile"},
	{"MENU_CREATEPROFILE_PLEASENAME", "Please name your profile."},
	{"MENU_CREATEPROFILE_NAME", "Name"},
	{"MENU_CREATEPROFILE_ACCEPT", "Accept"},
	{"MENU_SELECTPROFILE_TITLE", "Select Profile"},
	{"MENU_SELECTPROFILE_PLEASESELECT", "Please select a profile to use"},
	{"MENU_SELECTPROFILE_PLACEHOLDER", "[You]"},
	{"MENU_SELECTPROFILE_FORYOU", "for you."},
	{"MENU_SELECTPROFILE_TWOSPLITA", "for Player 1 (Top Screen)."},
	{"MENU_SELECTPROFILE_TWOSPLITB", "for Player 2 (Bottom Screen)."},
	{"MENU_SELECTPROFILE_FOURSPLITA", "for Player 1 (Top-Left Screen)."},
	{"MENU_SELECTPROFILE_FOURSPLITB", "for Player 2 (Top-Right Screen)."},
	{"MENU_SELECTPROFILE_FOURSPLITC", "for Player 3 (Bottom-Left Screen)."},
	{"MENU_SELECTPROFILE_FOURSPLITD", "for Player 4 (Bottom-Right Screen)."},
	{"MENU_SELECTPROFILE_PROFILE", "Profile"},
	{"MENU_SELECTPROFILE_ACCEPT", "Accept"},
	{"MENU_OTHER_RANDOM", "Random"},
	{"MENU_OTHER_RANDOMEPISODEA", "Random (Episode 1)"},
	{"MENU_OTHER_RANDOMEPISODEB", "Random (Episode 2)"},
	{"MENU_OTHER_RANDOMEPISODEC", "Random (Episode 3)"},
	{"MENU_OTHER_RANDOMEPISODED", "Random (Episode 4)"},
	{"MENU_OTHER_RANDOMEPISODEE", "Random (Episode 5)"},
	{"MENU_OTHER_RANDOMEPISODEF", "Random (Episode 6)"},
	{"MENU_OTHER_CHANGECONTROL", "Hit the new key for\n%s\nESC for Cancel"},
	{"MENU_OTHER_PLAYERACONTROLS", "Player 1's Controls"},
	{"MENU_OTHER_PLAYERBCONTROLS", "Player 2's Controls"},
	{"MENU_OTHER_PLAYERCCONTROLS", "Player 3's Controls"},
	{"MENU_OTHER_PLAYERDCONTROLS", "Player 4's Controls"},
	{"MENU_OPTIONS_BINARYSAVES", "Binary Saves"},
	
	{			   "MENUGAMEVAR_CATNONE", ""},
	{			   "MENUGAMEVAR_CATGAME", "*** GAME ***"},
	{			   "MENUGAMEVAR_CATITEMS", "*** ITEMS ***"},
	{			"MENUGAMEVAR_CATPLAYERS", "*** PLAYERS ***"},
	{		   "MENUGAMEVAR_CATMONSTERS", "*** MONSTERS ***"},
	{			   "MENUGAMEVAR_CATMISC", "*** MISC ***"},
	{				"MENUGAMEVAR_CATFUN", "*** FUN ***"},
	{			"MENUGAMEVAR_CATHERETIC", "*** HERETIC ***"},
	{			 "MENUGAMEVAR_CATCOMPAT", "*** COMPATIBILITY ***"},
	
	{			"MENUGENERAL_HELLOWORLD", "Hello World!"},
	{		   "MENUGENERAL_NEWGAMETYPE", "Connection Type"},
	{		"MENUGENERAL_NEWGAMEISLOCAL", "Local System"},
	{	   "MENUGENERAL_NEWGAMEISSERVER", "Network Server"},
	{		  "MENUGENERAL_NEWGAMELEVEL", "Level"},
	{			  "MENUGENERAL_SOLOGAME", "Single Player"},
	{			  "MENUGENERAL_MAINMENU", "Main Menu"},
	{			 	  "MENUGENERAL_QUIT", "Quit"},
	{		  "MENUGENERAL_CLASSICSTART", "Start Classic Game"},
	{			"MENUGENERAL_SKILLLEVEL", "Choose Skill Level"},
	{			"MENUGENERAL_SELECTEPIS", "Select Episode"},
	{		 "MENUGENERAL_SELECTPROFILE", "Select Profile"},
	{		"MENUGENERAL_SELECTTHISPROF", "Use This Profile"},
	{			"MENUGENERAL_CREATEPROF", "Create Profile"},
	{		   "MENUGENERAL_CURRENTPROF", "Current Profile"},
	{			   "MENUGENERAL_OPTIONS", "Options"},
	{			"MENUGENERAL_SERVERVARS", "Internet/LAN Server Settings"},
	{		   "MENUGENERAL_CONSOLEVARS", "Console Settings"},
	{		 "MENUGENERAL_INTERFACEVARS", "System Interface Settings"},
	{			  "MENUGENERAL_MENUVARS", "Menu Settings"},
	{		  "MENUGENERAL_RENDERERVARS", "Renderer Settings"},
	{			"MENUGENERAL_SCREENVARS", "Video and Screen Settings"},
	{			 "MENUGENERAL_SOUNDVARS", "Sound and Music Settings"},
	
	{		   "MENUGENERAL_CONBACKCOLOR", "Background Color"},
	{				"MENUGENERAL_CONFONT", "Font"},
	{		   "MENUGENERAL_CONFORECOLOR", "Foreground Color"},
	{		   "MENUGENERAL_CONMONOSPACE", "Monospace Characters"},
	{		   "MENUGENERAL_CONPAUSEGAME", "Pause When Active"},
	{			   "MENUGENERAL_CONSCALE", "Scale"},
	{		"MENUGENERAL_CONSCREENHEIGHT", "Height of Console"},
	{		  "MENUGENERAL_CONTESTSTRING", "Test String"},
	{		"MENUGENERAL_IENABLEJOYSTICK", "Enable Joysticks"},
	{		   "MENUGENERAL_IENABLEMOUSE", "Enable Mouse"},
	{			"MENUGENERAL_IOSSMIDIDEV", "OSS MIDI Device Pathname"},
	{			"MENUGENERAL_MENUCOMPACT", "Compact Menus"},
	{			   "MENUGENERAL_MENUFONT", "Font"},
	{		"MENUGENERAL_MENUHEADERCOLOR", "Header Color"},
	{		  "MENUGENERAL_MENUITEMCOLOR", "Item Color"},
	{		   "MENUGENERAL_MENUVALCOLOR", "Value Color"},
	{		 "MENUGENERAL_RCUTWALLDETAIL", "Reduce Texture Detail"},
	{			"MENUGENERAL_RDRAWSPLATS", "Draw Splats"},
	{			 "MENUGENERAL_RFAKESSPAL", "Fake Split-Screen Palette"},
	{			 "MENUGENERAL_RMAXSPLATS", "Max Splats"},
	{			  "MENUGENERAL_RRENDERER", "Renderer"},
	{		  "MENUGENERAL_RTRANSPARENCY", "Enable Transparency"},
	{			  "MENUGENERAL_RVIEWSIZE", "View Size"},
	{		  "MENUGENERAL_SCRFULLSCREEN", "Fullscreen"},
	{			  "MENUGENERAL_SCRHEIGHT", "Screen Height"},
	{			   "MENUGENERAL_SCRWIDTH", "Screen Width"},
	{		  "MENUGENERAL_SNDBUFFERSIZE", "Buffer Size"},
	{			"MENUGENERAL_SNDCHANNELS", "Channels"},
	{			 "MENUGENERAL_SNDDENSITY", "Bit Depth"},
	{		 "MENUGENERAL_SNDMUSICVOLUME", "Music Volume"},
	{			 "MENUGENERAL_SNDQUALITY", "Frequency"},
	{		 "MENUGENERAL_SNDRANDOMPITCH", "Random Pitching"},
	{	"MENUGENERAL_SNDRESERVEDCHANNELS", "Reserved Channels"},
	{		 "MENUGENERAL_SNDSOUNDVOLUME", "Sound Volume"},
	{		"MENUGENERAL_SNDSPEAKERSETUP", "Speaker Setup"},
	{	  "MENUGENERAL_SVCONNECTPASSWORD", "Connect Password"},
	{				"MENUGENERAL_SVEMAIL", "E-Mail Address"},
	{				  "MENUGENERAL_SVIRC", "IRC Channel"},
	{		 "MENUGENERAL_SVJOINPASSWORD", "Join Password"},
	{		   "MENUGENERAL_SVMAXCATCHUP", "Catch-Up Threshold"},
	{		   "MENUGENERAL_SVMAXCLIENTS", "Max Clients"},
	{	   "MENUGENERAL_SVMAXDEMOCATCHUP", "Demo Catch-Up Threshold"},
	{				 "MENUGENERAL_SVMOTD", "Message of the Day"},
	{				 "MENUGENERAL_SVNAME", "Name"},
	{			  "MENUGENERAL_SVREADYBY", "Connection Sync Threshold"},
	{				  "MENUGENERAL_SVURL", "Website URL"},
	{			   "MENUGENERAL_SVWADURL", "WAD Download URL"},
	{			 "MENUGENERAL_VIDDRAWFPS", "Draw Frames per Second"},
	{		  "MENUGENERAL_VIDSCREENLINK", "Screen Link"},
	
	{				"UIGENERAL_DONTCARE", "Don't Care!"},
	{					 "UIGENERAL_YES", "Yes"},
	{					  "UIGENERAL_NO", "No"},
	
	/*** INTERMISSION ***/
	{"INTERMISSION_FINISHED", "Finished"},
	{"INTERMISSION_ENTERING", "Entering"},
	{"INTERMISSION_KILLS", "Kills"},
	{"INTERMISSION_ITEMS", "Items"},
	{"INTERMISSION_SECRETS", "Secrets"},
	{"INTERMISSION_FRAGS", "Frags"},
	{"INTERMISSION_NETKILLS", "Kills"},
	{"INTERMISSION_NETITEMS", "Items"},
	{"INTERMISSION_NETSECRETS", "Secrt"},
	{"INTERMISSION_NETFRAGS", "Frags"},
	{"INTERMISSION_NOWENTERING", "Now Entering:"},
	{"INTERMISSION_TIME", "Time"},
	{"INTERMISSION_PAR", "Par"},
	{"INTERMISSION_NETTIME", "Time"},
	{"INTERMISSION_NETPAR", "Par"},
	
	/*** CONSOLE VARIABLE HINTS ***/
	{           "CVHINT_CONSCREENHEIGHT", "Height of the console."},
	{              "CVHINT_CONBACKCOLOR", "Background color of the console."},
	{              "CVHINT_CONFORECOLOR", "Foreground color of the console."},
	{                   "CVHINT_CONFONT", "Font to draw the console with."},
	{              "CVHINT_CONMONOSPACE", "Draw the console font as monospaced."},
	{                  "CVHINT_CONSCALE", "Scales the text of the console (high resolution)."},
	{             "CVHINT_CONTESTSTRING", "Variable to test strings (escape/unescape)."},
	{			   "CVHINT_CONPAUSEGAME", "Pause the game when the console is open."},
	
	{				   "CVHINT_MENUFONT", "Font used to draw menu items."},
	{			  "CVHINT_MENUITEMCOLOR", "Color used to draw menu items."},
	{			  "CVHINT_MENUVALSCOLOR", "Color used to draw menu values."},
	{			"CVHINT_MENUHEADERCOLOR", "Color used to draw item headers."},
	
	{					 "CVHINT_SVNAME", "The visible name of the server."},
	{					  "CVHINT_SVURL", "The web site for this server."},
	{				   "CVHINT_SVWADURL", "Location where WADs should be downloaded from."},
	{					  "CVHINT_SVIRC", "The IRC Channel for this server."},
	{					 "CVHINT_SVMOTD", "The message displayed to clients when joining."},
	{		  "CVHINT_SVCONNECTPASSWORD", "The password needed to connect to a server."},
	{			 "CVHINT_SVJOINPASSWORD", "The password needed to join the game."},
	{			   "CVHINT_SVMAXCLIENTS", "The maximum amount of clients that may join the game."},
	{				  "CVHINT_SVREADYBY", "Amount of time in milliseconds a client has to be ready by before a forced disconnect."},
	{			   "CVHINT_SVMAXCATCHUP", "Maximum amount of time to catchup when playing as the server, after this threshold catchup stops."},
	{		   "CVHINT_SVMAXDEMOCATCHUP", "Maximum amount of time to catchup when playing a demo, after this threshold catchup stops."},
	
	{				"CVHINT_CLMAXPTRIES", "Maximum amount of client attempts to obtain a client from the server."},
	{			  "CVHINT_CLMAXPTRYTIME", "Time to wait between client requests."},
	
	{			 "CVHINT_RCUTWALLDETAIL", "Lower the detail of wall textures to reduce memory usage at the cost of quality."},
	{			 "CVHINT_RCUTWALLDETAIL", "Lower the detail of wall textures to reduce memory usage at the cost of quality."},
	{				 "CVHINT_RFAKESSPAL", "Fake the palette changing when using split-screen."},
	{				  "CVHINT_RRENDERER", "Which renderer platform to utilize."},
	{				  "CVHINT_RVIEWSIZE", "The size of the screen to draw."},
	{			  "CVHINT_RTRANSPARENCY", "Enables drawing of transparent images."},
	{				"CVHINT_RDRAWSPLATS", "Enables drawing of splats on walls and floors."},
	{				 "CVHINT_RMAXSPLATS", "Maximum amount of splats that can exist at one time."},
	
	{			  "CVHINT_SCRFULLSCREEN", "Use fullscreen video mode."},
	{				   "CVHINT_SCRWIDTH", "Width of the screen to use."},
	{				  "CVHINT_SCRHEIGHT", "Height of the screen to use."},
	
	{			  "CVHINT_VIDSCREENLINK", "Special effect to perform when linking together screens."},
	{				 "CVHINT_VIDDRAWFPS", "Draw frames per second on the screen."},
	
	{			   "CVHINT_IENABLEMOUSE", "Enable mouse input."},
	{			"CVHINT_IENABLEJOYSTICK", "Enable joystick input."},
	{				"CVHINT_IOSSMIDIDEV", "Open Sound System /dev/midi device to use when playing MIDI music."},
	
	{			"CVHINT_SNDSPEAKERSETUP", "Number of speakers to use."},
	{				 "CVHINT_SNDQUALITY", "Quality of sound to be outputted."},
	{				 "CVHINT_SNDDENSITY", "Density of the sound to use, the higher the better."},
	{			  "CVHINT_SNDBUFFERSIZE", "Size of the sound buffer in size."},
	{			 "CVHINT_SNDRANDOMPITCH", "Randomly Pitch Sounds."},
	{				"CVHINT_SNDCHANNELS", "Number of sound channels to play."},
	{		"CVHINT_SNDRESERVEDCHANNELS", "Number of channels to keep in reserve."},
	{			 "CVHINT_SNDSOUNDVOLUME", "The volume at which sounds play at."},
	{			 "CVHINT_SNDMUSICVOLUME", "The volume at which music plays at."},
	
	/*** NETWORK STUFF ***/
	{			"NET_YOUARENOTTHESERVER", "You are not the server."},
	{				 "NET_LEVELNOTFOUND", "Level not found."},
	{				 "NET_EXCEEDEDSPLIT", "Client wants a player, but exceeds split-screen limit."},
	{				  "NET_ATMAXPLAYERS", "Client wants a player, but there are too many players."},
	{					 "NET_BADCLIENT", "Client wants a player, but the client is invalid."},
	{		"NET_CONNECTINGTOSAMESERVER", "You are trying to connect to the same server you are already connected to, disconnect first."},
	{			   "NET_CONNECTNOSOCKET", "Failed to create socket to connect to server."},
	{           "NET_CONNECTINGTOSERVER", "Connecting to server..."},
	{		 "NET_RECONNECTYOUARESERVER", "Cannot reconnect to the server, because you are the server."},
	{				"NET_BADHOSTRESOLVE", "Cannot resolve hostname."},
	{			   "NET_CLIENTCONNECTED", "Client $1{z has connected."},
	{				  "NET_PLAYERJOINED", "Player $1{z has joined the game."},
	{					"NET_CLIENTGONE", "Client $1{z has disconnected ($2)."},
	{					  "NET_NOREASON", "No reason."},
	
	{						"WFGS_TITLE", "Waiting For Game To Start"},
	{				   "WFGS_PLAYERNAME", "Player Name"},
	{						 "WFGS_PING", "Ping"},
	{				   "WFGS_DEMOPLAYER", "Demo"},
	{						 "WFGS_HOST", "Host"},
	{						  "WFGS_BOT", "Bot"},
	
	{						"WFJW_TITLE", "Connecting to Server"},
	{				  "NET_SERVERDISCON", "Server Disconnected"},
	{						"NET_KICKED", "Kicked"},
	
	/*** DEPRECATED STRINGS ***/
	{                              "DEP_D_DEVSTR", "Development mode ON.\n"},
	{                               "DEP_D_CDROM", "CD-ROM Version: default.cfg from c:\\doomdata\n"},
	{                              "DEP_PRESSKEY", "press a key."},
	{                               "DEP_PRESSYN", "press y or n."},
	{                               "DEP_LOADNET", "only the server can do a load net game!\n\npress a key."},
	{                              "DEP_QLOADNET", "you can't quickload during a netgame!\n\npress a key."},
	{                             "DEP_QSAVESPOT", "you haven't picked a quicksave slot yet!\n\npress a key."},
	{                              "DEP_SAVEDEAD", "you can't save if you aren't playing!\n\npress a key."},
	{                              "DEP_QSPROMPT", "quicksave over your game named\n\n'$1'?\n\npress y or n."},
	{                              "DEP_QLPROMPT", "do you want to quickload the game named\n\n'$1'?\n\npress y or n."},
	{                               "DEP_NEWGAME", "you can't start a new game\nwhile in a network game.\n\n"},
	{                             "DEP_NIGHTMARE", "are you sure? this skill level\nisn't even remotely fair.\n\npress y or n."},
	{                              "DEP_SWSTRING", "this is the shareware version of doom.\n\nyou need to order the entire trilogy.\n\npress a key."},
	{                                "DEP_MSGOFF", "Messages OFF"},
	{                                 "DEP_MSGON", "Messages ON"},
	{                                "DEP_NETEND", "you can't end a netgame!\n\npress a key."},
	{                               "DEP_ENDGAME", "are you sure you want to end the game?\n\npress y or n."},
	{                             "DEP_NOENDGAME", "you can't end a game if you aren't even playing.\n\npress a key."},
	{                                  "DEP_DOSY", "$1\n\n(press y to quit.)"},
	{                              "DEP_DETAILHI", "High detail"},
	{                              "DEP_DETAILLO", "Low detail"},
	{                             "DEP_GAMMALVL0", "Gamma correction OFF"},
	{                             "DEP_GAMMALVL1", "Gamma correction level 1"},
	{                             "DEP_GAMMALVL2", "Gamma correction level 2"},
	{                             "DEP_GAMMALVL3", "Gamma correction level 3"},
	{                             "DEP_GAMMALVL4", "Gamma correction level 4"},
	{                           "DEP_EMPTYSTRING", "empty slot"},
	{                              "DEP_GOTARMOR", "Picked up the armor."},
	{                               "DEP_GOTMEGA", "Picked up the MegaArmor!"},
	{                           "DEP_GOTHTHBONUS", "Picked up a health bonus."},
	{                           "DEP_GOTARMBONUS", "Picked up an armor bonus."},
	{                               "DEP_GOTSTIM", "Picked up a stimpack."},
	{                           "DEP_GOTMEDINEED", "Picked up a medikit that you REALLY need!"},
	{                            "DEP_GOTMEDIKIT", "Picked up a medikit."},
	{                              "DEP_GOTSUPER", "Supercharge!"},
	{                           "DEP_GOTBLUECARD", "Picked up a blue keycard."},
	{                           "DEP_GOTYELWCARD", "Picked up a yellow keycard."},
	{                            "DEP_GOTREDCARD", "Picked up a red keycard."},
	{                           "DEP_GOTBLUESKUL", "Picked up a blue skull key."},
	{                           "DEP_GOTYELWSKUL", "Picked up a yellow skull key."},
	{                           "DEP_GOTREDSKULL", "Picked up a red skull key."},
	{                              "DEP_GOTINVUL", "Invulnerability!"},
	{                            "DEP_GOTBERSERK", "Berserk!"},
	{                              "DEP_GOTINVIS", "Partial Invisibility"},
	{                               "DEP_GOTSUIT", "Radiation Shielding Suit"},
	{                                "DEP_GOTMAP", "Computer Area Map"},
	{                              "DEP_GOTVISOR", "Light Amplification Visor"},
	{                            "DEP_GOTMSPHERE", "MegaSphere!"},
	{                               "DEP_GOTCLIP", "Picked up a clip."},
	{                            "DEP_GOTCLIPBOX", "Picked up a box of bullets."},
	{                             "DEP_GOTROCKET", "Picked up a rocket."},
	{                            "DEP_GOTROCKBOX", "Picked up a box of rockets."},
	{                               "DEP_GOTCELL", "Picked up an energy cell."},
	{                            "DEP_GOTCELLBOX", "Picked up an energy cell pack."},
	{                             "DEP_GOTSHELLS", "Picked up 4 shotgun shells."},
	{                           "DEP_GOTSHELLBOX", "Picked up a box of shotgun shells."},
	{                           "DEP_GOTBACKPACK", "Picked up a backpack full of ammo!"},
	{                            "DEP_GOTBFG9000", "You got the BFG9000!  Oh, yes."},
	{                           "DEP_GOTCHAINGUN", "You got the chaingun!"},
	{                           "DEP_GOTCHAINSAW", "A chainsaw!  Find some meat!"},
	{                           "DEP_GOTLAUNCHER", "You got the rocket launcher!"},
	{                             "DEP_GOTPLASMA", "You got the plasma gun!"},
	{                            "DEP_GOTSHOTGUN", "You got the shotgun!"},
	{                           "DEP_GOTSHOTGUN2", "You got the super shotgun!"},
	{                              "DEP_PD_BLUEO", "You need a blue key to activate this object"},
	{                               "DEP_PD_REDO", "You need a red key to activate this object"},
	{                            "DEP_PD_YELLOWO", "You need a yellow key to activate this object"},
	{                              "DEP_PD_BLUEK", "You need a blue key to open this door"},
	{                               "DEP_PD_REDK", "You need a red key to open this door"},
	{                            "DEP_PD_YELLOWK", "You need a yellow key to open this door"},
	{                               "DEP_GGSAVED", "game saved."},
	{                            "DEP_HUSTR_MSGU", "[Message unsent]"},
	{                            "DEP_HUSTR_E1M1", "E1M1: Hangar"},
	{                            "DEP_HUSTR_E1M2", "E1M2: Nuclear Plant"},
	{                            "DEP_HUSTR_E1M3", "E1M3: Toxin Refinery"},
	{                            "DEP_HUSTR_E1M4", "E1M4: Command Control"},
	{                            "DEP_HUSTR_E1M5", "E1M5: Phobos Lab"},
	{                            "DEP_HUSTR_E1M6", "E1M6: Central Processing"},
	{                            "DEP_HUSTR_E1M7", "E1M7: Computer Station"},
	{                            "DEP_HUSTR_E1M8", "E1M8: Phobos Anomaly"},
	{                            "DEP_HUSTR_E1M9", "E1M9: Military Base"},
	{                            "DEP_HUSTR_E2M1", "E2M1: Deimos Anomaly"},
	{                            "DEP_HUSTR_E2M2", "E2M2: Containment Area"},
	{                            "DEP_HUSTR_E2M3", "E2M3: Refinery"},
	{                            "DEP_HUSTR_E2M4", "E2M4: Deimos Lab"},
	{                            "DEP_HUSTR_E2M5", "E2M5: Command Center"},
	{                            "DEP_HUSTR_E2M6", "E2M6: Halls of the Damned"},
	{                            "DEP_HUSTR_E2M7", "E2M7: Spawning Vats"},
	{                            "DEP_HUSTR_E2M8", "E2M8: Tower of Babel"},
	{                            "DEP_HUSTR_E2M9", "E2M9: Fortress of Mystery"},
	{                            "DEP_HUSTR_E3M1", "E3M1: Hell Keep"},
	{                            "DEP_HUSTR_E3M2", "E3M2: Slough of Despair"},
	{                            "DEP_HUSTR_E3M3", "E3M3: Pandemonium"},
	{                            "DEP_HUSTR_E3M4", "E3M4: House of Pain"},
	{                            "DEP_HUSTR_E3M5", "E3M5: Unholy Cathedral"},
	{                            "DEP_HUSTR_E3M6", "E3M6: Mt. Erebus"},
	{                            "DEP_HUSTR_E3M7", "E3M7: Limbo"},
	{                            "DEP_HUSTR_E3M8", "E3M8: Dis"},
	{                            "DEP_HUSTR_E3M9", "E3M9: Warrens"},
	{                            "DEP_HUSTR_E4M1", "E4M1: Hell Beneath"},
	{                            "DEP_HUSTR_E4M2", "E4M2: Perfect Hatred"},
	{                            "DEP_HUSTR_E4M3", "E4M3: Sever The Wicked"},
	{                            "DEP_HUSTR_E4M4", "E4M4: Unruly Evil"},
	{                            "DEP_HUSTR_E4M5", "E4M5: They Will Repent"},
	{                            "DEP_HUSTR_E4M6", "E4M6: Against Thee Wickedly"},
	{                            "DEP_HUSTR_E4M7", "E4M7: And Hell Followed"},
	{                            "DEP_HUSTR_E4M8", "E4M8: Unto The Cruel"},
	{                            "DEP_HUSTR_E4M9", "E4M9: Fear"},
	{                               "DEP_HUSTR_1", "level 1: entryway"},
	{                               "DEP_HUSTR_2", "level 2: underhalls"},
	{                               "DEP_HUSTR_3", "level 3: the gantlet"},
	{                               "DEP_HUSTR_4", "level 4: the focus"},
	{                               "DEP_HUSTR_5", "level 5: the waste tunnels"},
	{                               "DEP_HUSTR_6", "level 6: the crusher"},
	{                               "DEP_HUSTR_7", "level 7: dead simple"},
	{                               "DEP_HUSTR_8", "level 8: tricks and traps"},
	{                               "DEP_HUSTR_9", "level 9: the pit"},
	{                              "DEP_HUSTR_10", "level 10: refueling base"},
	{                              "DEP_HUSTR_11", "level 11: 'o' of destruction!"},
	{                              "DEP_HUSTR_12", "level 12: the factory"},
	{                              "DEP_HUSTR_13", "level 13: downtown"},
	{                              "DEP_HUSTR_14", "level 14: the inmost dens"},
	{                              "DEP_HUSTR_15", "level 15: industrial zone"},
	{                              "DEP_HUSTR_16", "level 16: suburbs"},
	{                              "DEP_HUSTR_17", "level 17: tenements"},
	{                              "DEP_HUSTR_18", "level 18: the courtyard"},
	{                              "DEP_HUSTR_19", "level 19: the citadel"},
	{                              "DEP_HUSTR_20", "level 20: gotcha!"},
	{                              "DEP_HUSTR_21", "level 21: nirvana"},
	{                              "DEP_HUSTR_22", "level 22: the catacombs"},
	{                              "DEP_HUSTR_23", "level 23: barrels o' fun"},
	{                              "DEP_HUSTR_24", "level 24: the chasm"},
	{                              "DEP_HUSTR_25", "level 25: bloodfalls"},
	{                              "DEP_HUSTR_26", "level 26: the abandoned mines"},
	{                              "DEP_HUSTR_27", "level 27: monster condo"},
	{                              "DEP_HUSTR_28", "level 28: the spirit world"},
	{                              "DEP_HUSTR_29", "level 29: the living end"},
	{                              "DEP_HUSTR_30", "level 30: icon of sin"},
	{                              "DEP_HUSTR_31", "level 31: wolfenstein"},
	{                              "DEP_HUSTR_32", "level 32: grosse"},
	{                              "DEP_PHUSTR_1", "level 1: congo"},
	{                              "DEP_PHUSTR_2", "level 2: well of souls"},
	{                              "DEP_PHUSTR_3", "level 3: aztec"},
	{                              "DEP_PHUSTR_4", "level 4: caged"},
	{                              "DEP_PHUSTR_5", "level 5: ghost town"},
	{                              "DEP_PHUSTR_6", "level 6: baron's lair"},
	{                              "DEP_PHUSTR_7", "level 7: caughtyard"},
	{                              "DEP_PHUSTR_8", "level 8: realm"},
	{                              "DEP_PHUSTR_9", "level 9: abattoire"},
	{                             "DEP_PHUSTR_10", "level 10: onslaught"},
	{                             "DEP_PHUSTR_11", "level 11: hunted"},
	{                             "DEP_PHUSTR_12", "level 12: speed"},
	{                             "DEP_PHUSTR_13", "level 13: the crypt"},
	{                             "DEP_PHUSTR_14", "level 14: genesis"},
	{                             "DEP_PHUSTR_15", "level 15: the twilight"},
	{                             "DEP_PHUSTR_16", "level 16: the omen"},
	{                             "DEP_PHUSTR_17", "level 17: compound"},
	{                             "DEP_PHUSTR_18", "level 18: neurosphere"},
	{                             "DEP_PHUSTR_19", "level 19: nme"},
	{                             "DEP_PHUSTR_20", "level 20: the death domain"},
	{                             "DEP_PHUSTR_21", "level 21: slayer"},
	{                             "DEP_PHUSTR_22", "level 22: impossible mission"},
	{                             "DEP_PHUSTR_23", "level 23: tombstone"},
	{                             "DEP_PHUSTR_24", "level 24: the final frontier"},
	{                             "DEP_PHUSTR_25", "level 25: the temple of darkness"},
	{                             "DEP_PHUSTR_26", "level 26: bunker"},
	{                             "DEP_PHUSTR_27", "level 27: anti-christ"},
	{                             "DEP_PHUSTR_28", "level 28: the sewers"},
	{                             "DEP_PHUSTR_29", "level 29: odyssey of noises"},
	{                             "DEP_PHUSTR_30", "level 30: the gateway of hell"},
	{                             "DEP_PHUSTR_31", "level 31: cyberden"},
	{                             "DEP_PHUSTR_32", "level 32: go 2 it"},
	{                              "DEP_THUSTR_1", "level 1: system control"},
	{                              "DEP_THUSTR_2", "level 2: human bbq"},
	{                              "DEP_THUSTR_3", "level 3: power control"},
	{                              "DEP_THUSTR_4", "level 4: wormhole"},
	{                              "DEP_THUSTR_5", "level 5: hanger"},
	{                              "DEP_THUSTR_6", "level 6: open season"},
	{                              "DEP_THUSTR_7", "level 7: prison"},
	{                              "DEP_THUSTR_8", "level 8: metal"},
	{                              "DEP_THUSTR_9", "level 9: stronghold"},
	{                             "DEP_THUSTR_10", "level 10: redemption"},
	{                             "DEP_THUSTR_11", "level 11: storage facility"},
	{                             "DEP_THUSTR_12", "level 12: crater"},
	{                             "DEP_THUSTR_13", "level 13: nukage processing"},
	{                             "DEP_THUSTR_14", "level 14: steel works"},
	{                             "DEP_THUSTR_15", "level 15: dead zone"},
	{                             "DEP_THUSTR_16", "level 16: deepest reaches"},
	{                             "DEP_THUSTR_17", "level 17: processing area"},
	{                             "DEP_THUSTR_18", "level 18: mill"},
	{                             "DEP_THUSTR_19", "level 19: shipping/respawning"},
	{                             "DEP_THUSTR_20", "level 20: central processing"},
	{                             "DEP_THUSTR_21", "level 21: administration center"},
	{                             "DEP_THUSTR_22", "level 22: habitat"},
	{                             "DEP_THUSTR_23", "level 23: lunar mining project"},
	{                             "DEP_THUSTR_24", "level 24: quarry"},
	{                             "DEP_THUSTR_25", "level 25: baron's den"},
	{                             "DEP_THUSTR_26", "level 26: ballistyx"},
	{                             "DEP_THUSTR_27", "level 27: mount pain"},
	{                             "DEP_THUSTR_28", "level 28: heck"},
	{                             "DEP_THUSTR_29", "level 29: river styx"},
	{                             "DEP_THUSTR_30", "level 30: last call"},
	{                             "DEP_THUSTR_31", "level 31: pharaoh"},
	{                             "DEP_THUSTR_32", "level 32: caribbean"},
	{                      "DEP_HUSTR_CHATMACRO1", "I'm ready to kick butt!"},
	{                      "DEP_HUSTR_CHATMACRO2", "I'm OK."},
	{                      "DEP_HUSTR_CHATMACRO3", "I'm not looking too good!"},
	{                      "DEP_HUSTR_CHATMACRO4", "Help!"},
	{                      "DEP_HUSTR_CHATMACRO5", "You suck!"},
	{                      "DEP_HUSTR_CHATMACRO6", "Next time, scumbag..."},
	{                      "DEP_HUSTR_CHATMACRO7", "Come here!"},
	{                      "DEP_HUSTR_CHATMACRO8", "I'll take care of it."},
	{                      "DEP_HUSTR_CHATMACRO9", "Yes"},
	{                      "DEP_HUSTR_CHATMACRO0", "No"},
	{                     "DEP_HUSTR_TALKTOSELF1", "You mumble to yourself"},
	{                     "DEP_HUSTR_TALKTOSELF2", "Who's there?"},
	{                     "DEP_HUSTR_TALKTOSELF3", "You scare yourself"},
	{                     "DEP_HUSTR_TALKTOSELF4", "You start to rave"},
	{                     "DEP_HUSTR_TALKTOSELF5", "You've lost it..."},
	{                     "DEP_HUSTR_MESSAGESENT", "[Message Sent]"},
	{                        "DEP_AMSTR_FOLLOWON", "Follow Mode ON"},
	{                       "DEP_AMSTR_FOLLOWOFF", "Follow Mode OFF"},
	{                          "DEP_AMSTR_GRIDON", "Grid ON"},
	{                         "DEP_AMSTR_GRIDOFF", "Grid OFF"},
	{                      "DEP_AMSTR_MARKEDSPOT", "Marked Spot"},
	{                    "DEP_AMSTR_MARKSCLEARED", "All Marks Cleared"},
	{                             "DEP_STSTR_MUS", "Music Change"},
	{                           "DEP_STSTR_NOMUS", "IMPOSSIBLE SELECTION"},
	{                           "DEP_STSTR_DQDON", "Degreelessness Mode On"},
	{                          "DEP_STSTR_DQDOFF", "Degreelessness Mode Off"},
	{                        "DEP_STSTR_KFAADDED", "Very Happy Ammo Added"},
	{                         "DEP_STSTR_FAADDED", "Ammo (no keys) Added"},
	{                            "DEP_STSTR_NCON", "No Clipping Mode ON"},
	{                           "DEP_STSTR_NCOFF", "No Clipping Mode OFF"},
	{                          "DEP_STSTR_BEHOLD", "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"},
	{                         "DEP_STSTR_BEHOLDX", "Power-up Toggled"},
	{                        "DEP_STSTR_CHOPPERS", "... doesn't suck - GM"},
	{                            "DEP_STSTR_CLEV", "Changing Level..."},
	{                                "DEP_E1TEXT", "Once you beat the big badasses and\nclean out the moon base you're supposed\nto win, aren't you? Aren't you? Where's\nyour fat reward and ticket home? What\nthe hell is this? It's not supposed to\nend this way!\n\nIt stinks like rotten meat, but looks\nlike the lost Deimos base.  Looks like\nyou're stuck on The Shores of Hell.\nThe only way out is through.\n\nTo continue the DOOM experience, play\nThe Shores of Hell and its amazing\nsequel, Inferno!\n"},
	{                                "DEP_E2TEXT", "You've done it! The hideous cyber-\ndemon lord that ruled the lost Deimos\nmoon base has been slain and you\nare triumphant! But ... where are\nyou? You clamber to the edge of the\nmoon and look down to see the awful\ntruth.\n\nDeimos floats above Hell itself!\nYou've never heard of anyone escaping\nfrom Hell, but you'll make the bastards\nsorry they ever heard of you! Quickly,\nyou rappel down to  the surface of\nHell.\n\nNow, it's on to the final chapter of\nDOOM! -- Inferno."},
	{                                "DEP_E3TEXT", "The loathsome spiderdemon that\nmasterminded the invasion of the moon\nbases and caused so much death has had\nits ass kicked for all time.\n\nA hidden doorway opens and you enter.\nYou've proven too tough for Hell to\ncontain, and now Hell at last plays\nfair -- for you emerge from the door\nto see the green fields of Earth!\nHome at last.\n\nYou wonder what's been happening on\nEarth while you were battling evil\nunleashed. It's good that no Hell-\nspawn could have come through that\ndoor with you ..."},
	{                                "DEP_E4TEXT", "the spider mastermind must have sent forth\nits legions of hellspawn before your\nfinal confrontation with that terrible\nbeast from hell.  but you stepped forward\nand brought forth eternal damnation and\nsuffering upon the horde as a true hero\nwould in the face of something so evil.\n\nbesides, someone was gonna pay for what\nhappened to daisy, your pet rabbit.\n\nbut now, you see spread before you more\npotential pain and gibbitude as a nation\nof demons run amok among our cities.\n\nnext stop, hell on earth!"},
	{                                "DEP_C1TEXT", "YOU HAVE ENTERED DEEPLY INTO THE INFESTED\nSTARPORT. BUT SOMETHING IS WRONG. THE\nMONSTERS HAVE BROUGHT THEIR OWN REALITY\nWITH THEM, AND THE STARPORT'S TECHNOLOGY\nIS BEING SUBVERTED BY THEIR PRESENCE.\n\nAHEAD, YOU SEE AN OUTPOST OF HELL, A\nFORTIFIED ZONE. IF YOU CAN GET PAST IT,\nYOU CAN PENETRATE INTO THE HAUNTED HEART\nOF THE STARBASE AND FIND THE CONTROLLING\nSWITCH WHICH HOLDS EARTH'S POPULATION\nHOSTAGE."},
	{                                "DEP_C2TEXT", "YOU HAVE WON! YOUR VICTORY HAS ENABLED\nHUMANKIND TO EVACUATE EARTH AND ESCAPE\nTHE NIGHTMARE.  NOW YOU ARE THE ONLY\nHUMAN LEFT ON THE FACE OF THE PLANET.\nCANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\nAND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\nYOU SIT BACK AND WAIT FOR DEATH, CONTENT\nTHAT YOU HAVE SAVED YOUR SPECIES.\n\nBUT THEN, EARTH CONTROL BEAMS DOWN A\nMESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\nTHE SOURCE OF THE ALIEN INVASION. IF YOU\nGO THERE, YOU MAY BE ABLE TO BLOCK THEIR\nENTRY.  THE ALIEN BASE IS IN THE HEART OF\nYOUR OWN HOME CITY, NOT FAR FROM THE\nSTARPORT.\" SLOWLY AND PAINFULLY YOU GET\nUP AND RETURN TO THE FRAY."},
	{                                "DEP_C3TEXT", "YOU ARE AT THE CORRUPT HEART OF THE CITY,\nSURROUNDED BY THE CORPSES OF YOUR ENEMIES.\nYOU SEE NO WAY TO DESTROY THE CREATURES'\nENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\nTEETH AND PLUNGE THROUGH IT.\n\nTHERE MUST BE A WAY TO CLOSE IT ON THE\nOTHER SIDE. WHAT DO YOU CARE IF YOU'VE\nGOT TO GO THROUGH HELL TO GET TO IT?"},
	{                                "DEP_C4TEXT", "THE HORRENDOUS VISAGE OF THE BIGGEST\nDEMON YOU'VE EVER SEEN CRUMBLES BEFORE\nYOU, AFTER YOU PUMP YOUR ROCKETS INTO\nHIS EXPOSED BRAIN. THE MONSTER SHRIVELS\nUP AND DIES, ITS THRASHING LIMBS\nDEVASTATING UNTOLD MILES OF HELL'S\nSURFACE.\n\nYOU'VE DONE IT. THE INVASION IS OVER.\nEARTH IS SAVED. HELL IS A WRECK. YOU\nWONDER WHERE BAD FOLKS WILL GO WHEN THEY\nDIE, NOW. WIPING THE SWEAT FROM YOUR\nFOREHEAD YOU BEGIN THE LONG TREK BACK\nHOME. REBUILDING EARTH OUGHT TO BE A\nLOT MORE FUN THAN RUINING IT WAS.\n"},
	{                                "DEP_C5TEXT", "CONGRATULATIONS, YOU'VE FOUND THE SECRET\nLEVEL! LOOKS LIKE IT'S BEEN BUILT BY\nHUMANS, RATHER THAN DEMONS. YOU WONDER\nWHO THE INMATES OF THIS CORNER OF HELL\nWILL BE."},
	{                                "DEP_C6TEXT", "CONGRATULATIONS, YOU'VE FOUND THE\nSUPER SECRET LEVEL!  YOU'D BETTER\nBLAZE THROUGH THIS ONE!\n"},
	{                                "DEP_T1TEXT", "You've fought your way out of the infested\nexperimental labs.   It seems that UAC has\nonce again gulped it down.  With their\nhigh turnover, it must be hard for poor\nold UAC to buy corporate health insurance\nnowadays..\n\nAhead lies the military complex, now\nswarming with diseased horrors hot to get\ntheir teeth into you. With luck, the\ncomplex still has some warlike ordnance\nlaying around."},
	{                                "DEP_T2TEXT", "You hear the grinding of heavy machinery\nahead.  You sure hope they're not stamping\nout new hellspawn, but you're ready to\nream out a whole herd if you have to.\nThey might be planning a blood feast, but\nyou feel about as mean as two thousand\nmaniacs packed into one mad killer.\n\nYou don't plan to go down easy."},
	{                                "DEP_T3TEXT", "The vista opening ahead looks real damn\nfamiliar. Smells familiar, too -- like\nfried excrement. You didn't like this\nplace before, and you sure as hell ain't\nplanning to like it now. The more you\nbrood on it, the madder you get.\nHefting your gun, an evil grin trickles\nonto your face. Time to take some names."},
	{                                "DEP_T4TEXT", "Suddenly, all is silent, from one horizon\nto the other. The agonizing echo of Hell\nfades away, the nightmare sky turns to\nblue, the heaps of monster corpses start \nto evaporate along with the evil stench \nthat filled the air. Jeeze, maybe you've\ndone it. Have you really won?\n\nSomething rumbles in the distance.\nA blue light begins to glow inside the\nruined skull of the demon-spitter."},
	{                                "DEP_T5TEXT", "What now? Looks totally different. Kind\nof like King Tut's condo. Well,\nwhatever's here can't be any worse\nthan usual. Can it?  Or maybe it's best\nto let sleeping gods lie.."},
	{                                "DEP_T6TEXT", "Time for a vacation. You've burst the\nbowels of hell and by golly you're ready\nfor a break. You mutter to yourself,\nMaybe someone else can kick Hell's ass\nnext time around. Ahead lies a quiet town,\nwith peaceful flowing water, quaint\nbuildings, and presumably no Hellspawn.\n\nAs you step off the transport, you hear\nthe stomp of a cyberdemon's iron shoe."},
	{                             "DEP_CC_ZOMBIE", "ZOMBIEMAN"},
	{                            "DEP_CC_SHOTGUN", "SHOTGUN GUY"},
	{                              "DEP_CC_HEAVY", "HEAVY WEAPON DUDE"},
	{                                "DEP_CC_IMP", "IMP"},
	{                              "DEP_CC_DEMON", "DEMON"},
	{                               "DEP_CC_LOST", "LOST SOUL"},
	{                               "DEP_CC_CACO", "CACODEMON"},
	{                               "DEP_CC_HELL", "HELL KNIGHT"},
	{                              "DEP_CC_BARON", "BARON OF HELL"},
	{                              "DEP_CC_ARACH", "ARACHNOTRON"},
	{                               "DEP_CC_PAIN", "PAIN ELEMENTAL"},
	{                              "DEP_CC_REVEN", "REVENANT"},
	{                              "DEP_CC_MANCU", "MANCUBUS"},
	{                               "DEP_CC_ARCH", "ARCH-VILE"},
	{                             "DEP_CC_SPIDER", "THE SPIDER MASTERMIND"},
	{                              "DEP_CC_CYBER", "THE CYBERDEMON"},
	{                               "DEP_CC_HERO", "OUR HERO"},
	{                               "DEP_QUITMSG", "are you sure you want to\nquit this great game?"},
	{                              "DEP_QUITMSG1", "please don't leave, there's more\ndemons to toast!"},
	{                              "DEP_QUITMSG2", "let's beat it -- this is turning\ninto a bloodbath!"},
	{                              "DEP_QUITMSG3", "i wouldn't leave if i were you.\nYour OS is much worse."},
	{                              "DEP_QUITMSG4", "you're trying to say you like your OS\nbetter than me, right?"},
	{                              "DEP_QUITMSG5", "don't leave yet -- there's a\ndemon around that corner!"},
	{                              "DEP_QUITMSG6", "ya know, next time you come in here\ni'm gonna toast ya."},
	{                              "DEP_QUITMSG7", "go ahead and leave. see if i care."},
	{                              "DEP_QUIT2MSG", "you want to quit?\nthen, thou hast lost an eighth!"},
	{                             "DEP_QUIT2MSG1", "don't go now, there's a \ndimensional shambler waiting\nin the real world!"},
	{                             "DEP_QUIT2MSG2", "get outta here and go back\nto your boring programs."},
	{                             "DEP_QUIT2MSG3", "if i were your boss, i'd \n deathmatch ya in a minute!"},
	{                             "DEP_QUIT2MSG4", "look, bud. you leave now\nand you forfeit your body count!"},
	{                             "DEP_QUIT2MSG5", "just leave. when you come\nback, i'll be waiting with a bat."},
	{                             "DEP_QUIT2MSG6", "you're lucky i don't smack\nyou for thinking about leaving."},
	{                              "DEP_FLOOR4_8", "FLOOR4_8"},
	{                               "DEP_SFLR6_1", "SFLR6_1"},
	{                               "DEP_MFLR8_4", "MFLR8_4"},
	{                               "DEP_MFLR8_3", "MFLR8_3"},
	{                               "DEP_SLIME16", "SLIME16"},
	{                               "DEP_RROCK14", "RROCK14"},
	{                               "DEP_RROCK07", "RROCK07"},
	{                               "DEP_RROCK17", "RROCK17"},
	{                               "DEP_RROCK13", "RROCK13"},
	{                               "DEP_RROCK19", "RROCK19"},
	{                                "DEP_CREDIT", "CREDIT"},
	{                                 "DEP_HELP2", "HELP2"},
	{                              "DEP_VICTORY2", "VICTORY2"},
	{                                "DEP_ENDPIC", "ENDPIC"},
	{                              "DEP_MODIFIED", "===========================================================================\nATTENTION:  This version of DOOM has been modified.  If you would like to\nget a copy of the original game, call 1-800-IDGAMES or see the readme file.\n You will not receive technical support for modified games.\n   press enter to continue\n===========================================================================\n"},
	{                             "DEP_SHAREWARE", "===========================================================================\n  This program is Free Software!\n===========================================================================\n"},
	{                             "DEP_COMERCIAL", "===========================================================================\n  This program is Free Software!\n  See the terms of the GNU General Public License\n===========================================================================\n"},
	{                                "DEP_AUSTIN", "Austin Virtual Gaming: Levels will end after 20 minutes\n"},
	{                                "DEP_M_LOAD", "M_LoadDefaults: Load system defaults.\n"},
	{                                "DEP_Z_INIT", "Z_Init: Init zone memory allocation daemon. \n"},
	{                                "DEP_W_INIT", "W_Init: Init WADfiles.\n"},
	{                                "DEP_M_INIT", "M_Init: Init miscellaneous info.\n"},
	{                                "DEP_R_INIT", "R_Init: Init DOOM refresh daemon - "},
	{                                "DEP_P_INIT", "\nP_Init: Init Playloop state.\n"},
	{                                "DEP_I_INIT", "I_Init: Setting up machine state.\n"},
	{                            "DEP_D_CHECKNET", "D_CheckNetGame: Checking network game status.\n"},
	{                            "DEP_S_SETSOUND", "S_Init: Setting up sound.\n"},
	{                               "DEP_HU_INIT", "HU_Init: Setting up heads up display.\n"},
	{                               "DEP_ST_INIT", "ST_Init: Init status bar.\n"},
	{                               "DEP_STATREG", "External statistics registered.\n"},
	{                              "DEP_DOOM2WAD", "doom2.wad"},
	{                              "DEP_DOOMUWAD", "doomu.wad"},
	{                               "DEP_DOOMWAD", "doom.wad"},
	{                              "DEP_DOOM1WAD", "doom1.wad"},
	{                             "DEP_CDROM_DIR", "c:\\doomdata"},
	{                             "DEP_CDROM_DEF", "c:/doomdata/default.cfg"},
	{                            "DEP_CDROM_SAVE", "c:\\doomdata\\remoodsv$1.dsg"},
	{                             "DEP_NORM_SAVE", "remoodsv$1.dsg"},
	{                           "DEP_CDROM_SAVEI", "c:\\doomdata\\remoodsv$1.dsg"},
	{                            "DEP_NORM_SAVEI", "remoodsv$1.dsg"},
	{                              "DEP_PD_BLUEC", "You need a blue card to open this door"},
	{                               "DEP_PD_REDC", "You need a red card to open this door"},
	{                            "DEP_PD_YELLOWC", "You need a yellow card to open this door"},
	{                              "DEP_PD_BLUES", "You need a blue skull to open this door"},
	{                               "DEP_PD_REDS", "You need a red skull to open this door"},
	{                            "DEP_PD_YELLOWS", "You need a yellow skull to open this door"},
	{                                "DEP_PD_ANY", "Any key will open this door"},
	{                               "DEP_PD_ALL3", "You need all three keys to open this door"},
	{                               "DEP_PD_ALL6", "You need all six keys to open this door"},
	{                        "DEP_TXT_ARTIHEALTH", "QUARTZ FLASK"},
	{                           "DEP_TXT_ARTIFLY", "WINGS OF WRATH"},
	{               "DEP_TXT_ARTIINVULNERABILITY", "RING OF INVINCIBILITY"},
	{                   "DEP_TXT_ARTITOMEOFPOWER", "TOME OF POWER"},
	{                  "DEP_TXT_ARTIINVISIBILITY", "SHADOWSPHERE"},
	{                           "DEP_TXT_ARTIEGG", "MORPH OVUM"},
	{                   "DEP_TXT_ARTISUPERHEALTH", "MYSTIC URN"},
	{                         "DEP_TXT_ARTITORCH", "TORCH"},
	{                      "DEP_TXT_ARTIFIREBOMB", "TIME BOMB OF THE ANCIENTS"},
	{                      "DEP_TXT_ARTITELEPORT", "CHAOS DEVICE"},
	{                     "DEP_TXT_AMMOGOLDWAND1", "WAND CRYSTAL"},
	{                     "DEP_TXT_AMMOGOLDWAND2", "CRYSTAL GEODE"},
	{                         "DEP_TXT_AMMOMACE1", "MACE SPHERES"},
	{                         "DEP_TXT_AMMOMACE2", "PILE OF MACE SPHERES"},
	{                     "DEP_TXT_AMMOCROSSBOW1", "ETHEREAL ARROWS"},
	{                     "DEP_TXT_AMMOCROSSBOW2", "QUIVER OF ETHEREAL ARROWS"},
	{                      "DEP_TXT_AMMOBLASTER1", "CLAW ORB"},
	{                      "DEP_TXT_AMMOBLASTER2", "ENERGY ORB"},
	{                     "DEP_TXT_AMMOSKULLROD1", "LESSER RUNES"},
	{                     "DEP_TXT_AMMOSKULLROD2", "GREATER RUNES"},
	{                   "DEP_TXT_AMMOPHOENIXROD1", "FLAME ORB"},
	{                   "DEP_TXT_AMMOPHOENIXROD2", "INFERNO ORB"},
	{                           "DEP_TXT_WPNMACE", "FIREMACE"},
	{                       "DEP_TXT_WPNCROSSBOW", "ETHEREAL CROSSBOW"},
	{                        "DEP_TXT_WPNBLASTER", "DRAGON CLAW"},
	{                       "DEP_TXT_WPNSKULLROD", "HELLSTAFF"},
	{                     "DEP_TXT_WPNPHOENIXROD", "PHOENIX ROD"},
	{                      "DEP_TXT_WPNGAUNTLETS", "GAUNTLETS OF THE NECROMANCER"},
	{                  "DEP_TXT_ITEMBAGOFHOLDING", "BAG OF HOLDING"},
	{                        "DEP_TXT_CHEATGODON", "GOD MODE ON"},
	{                       "DEP_TXT_CHEATGODOFF", "GOD MODE OFF"},
	{                     "DEP_TXT_CHEATNOCLIPON", "NO CLIPPING ON"},
	{                    "DEP_TXT_CHEATNOCLIPOFF", "NO CLIPPING OFF"},
	{                      "DEP_TXT_CHEATWEAPONS", "ALL WEAPONS"},
	{                     "DEP_TXT_CHEATFLIGHTON", "FLIGHT ON"},
	{                    "DEP_TXT_CHEATFLIGHTOFF", "FLIGHT OFF"},
	{                      "DEP_TXT_CHEATPOWERON", "POWER ON"},
	{                     "DEP_TXT_CHEATPOWEROFF", "POWER OFF"},
	{                       "DEP_TXT_CHEATHEALTH", "FULL HEALTH"},
	{                         "DEP_TXT_CHEATKEYS", "ALL KEYS"},
	{                      "DEP_TXT_CHEATSOUNDON", "SOUND DEBUG ON"},
	{                     "DEP_TXT_CHEATSOUNDOFF", "SOUND DEBUG OFF"},
	{                     "DEP_TXT_CHEATTICKERON", "TICKER ON"},
	{                    "DEP_TXT_CHEATTICKEROFF", "TICKER OFF"},
	{                   "DEP_TXT_CHEATARTIFACTS1", "CHOOSE AN ARTIFACT ( A - J )"},
	{                   "DEP_TXT_CHEATARTIFACTS2", "HOW MANY ( 1 - 9 )"},
	{                   "DEP_TXT_CHEATARTIFACTS3", "YOU GOT IT"},
	{                "DEP_TXT_CHEATARTIFACTSFAIL", "BAD INPUT"},
	{                         "DEP_TXT_CHEATWARP", "LEVEL WARP"},
	{                   "DEP_TXT_CHEATSCREENSHOT", "SCREENSHOT"},
	{                    "DEP_TXT_CHEATCHICKENON", "CHICKEN ON"},
	{                   "DEP_TXT_CHEATCHICKENOFF", "CHICKEN OFF"},
	{                     "DEP_TXT_CHEATMASSACRE", "MASSACRE"},
	{                        "DEP_TXT_CHEATIDDQD", "TRYING TO CHEAT, EH?  NOW YOU DIE!"},
	{                        "DEP_TXT_CHEATIDKFA", "CHEATER - YOU DON'T DESERVE WEAPONS"},
	{                          "DEP_HERETIC_E1M1", "E1M1: THE DOCKS"},
	{                          "DEP_HERETIC_E1M2", "E1M2: THE DUNGEONS"},
	{                          "DEP_HERETIC_E1M3", "E1M3: THE GATEHOUSE"},
	{                          "DEP_HERETIC_E1M4", "E1M4: THE GUARD TOWER"},
	{                          "DEP_HERETIC_E1M5", "E1M5: THE CITADEL"},
	{                          "DEP_HERETIC_E1M6", "E1M6: THE CATHEDRAL"},
	{                          "DEP_HERETIC_E1M7", "E1M7: THE CRYPTS"},
	{                          "DEP_HERETIC_E1M8", "E1M8: HELL'S MAW"},
	{                          "DEP_HERETIC_E1M9", "E1M9: THE GRAVEYARD"},
	{                          "DEP_HERETIC_E2M1", "E2M1: THE CRATER"},
	{                          "DEP_HERETIC_E2M2", "E2M2: THE LAVA PITS"},
	{                          "DEP_HERETIC_E2M3", "E2M3: THE RIVER OF FIRE"},
	{                          "DEP_HERETIC_E2M4", "E2M4: THE ICE GROTTO"},
	{                          "DEP_HERETIC_E2M5", "E2M5: THE CATACOMBS"},
	{                          "DEP_HERETIC_E2M6", "E2M6: THE LABYRINTH"},
	{                          "DEP_HERETIC_E2M7", "E2M7: THE GREAT HALL"},
	{                          "DEP_HERETIC_E2M8", "E2M8: THE PORTALS OF CHAOS"},
	{                          "DEP_HERETIC_E2M9", "E2M9: THE GLACIER"},
	{                          "DEP_HERETIC_E3M1", "E3M1: THE STOREHOUSE"},
	{                          "DEP_HERETIC_E3M2", "E3M2: THE CESSPOOL"},
	{                          "DEP_HERETIC_E3M3", "E3M3: THE CONFLUENCE"},
	{                          "DEP_HERETIC_E3M4", "E3M4: THE AZURE FORTRESS"},
	{                          "DEP_HERETIC_E3M5", "E3M5: THE OPHIDIAN LAIR"},
	{                          "DEP_HERETIC_E3M6", "E3M6: THE HALLS OF FEAR"},
	{                          "DEP_HERETIC_E3M7", "E3M7: THE CHASM"},
	{                          "DEP_HERETIC_E3M8", "E3M8: D'SPARIL'S KEEP"},
	{                          "DEP_HERETIC_E3M9", "E3M9: THE AQUIFER"},
	{                          "DEP_HERETIC_E4M1", "E4M1: CATAFALQUE"},
	{                          "DEP_HERETIC_E4M2", "E4M2: BLOCKHOUSE"},
	{                          "DEP_HERETIC_E4M3", "E4M3: AMBULATORY"},
	{                          "DEP_HERETIC_E4M4", "E4M4: SEPULCHER"},
	{                          "DEP_HERETIC_E4M5", "E4M5: GREAT STAIR"},
	{                          "DEP_HERETIC_E4M6", "E4M6: HALLS OF THE APOSTATE"},
	{                          "DEP_HERETIC_E4M7", "E4M7: RAMPARTS OF PERDITION"},
	{                          "DEP_HERETIC_E4M8", "E4M8: SHATTERED BRIDGE"},
	{                          "DEP_HERETIC_E4M9", "E4M9: MAUSOLEUM"},
	{                          "DEP_HERETIC_E5M1", "E5M1: OCHRE CLIFFS"},
	{                          "DEP_HERETIC_E5M2", "E5M2: RAPIDS"},
	{                          "DEP_HERETIC_E5M3", "E5M3: QUAY"},
	{                          "DEP_HERETIC_E5M4", "E5M4: COURTYARD"},
	{                          "DEP_HERETIC_E5M5", "E5M5: HYDRATYR"},
	{                          "DEP_HERETIC_E5M6", "E5M6: COLONNADE"},
	{                          "DEP_HERETIC_E5M7", "E5M7: FOETID MANSE"},
	{                          "DEP_HERETIC_E5M8", "E5M8: FIELD OF JUDGEMENT"},
	{                          "DEP_HERETIC_E5M9", "E5M9: SKEIN OF D'SPARIL"},
	{                          "DEP_HERETIC_E6M1", "E6M1: Raven's Lair"},
	{                          "DEP_HERETIC_E6M2", "E6M2: Ruined Temple"},
	{                          "DEP_HERETIC_E6M3", "E6M3: American Legacy"},
	{                        "DEP_HERETIC_E1TEXT", "with the destruction of the iron\nliches and their minions, the last\nof the undead are cleared from this\nplane of existence.\n\nthose creatures had to come from\nsomewhere, though, and you have the\nsneaky suspicion that the fiery\nportal of hell's maw opens onto\ntheir home dimension.\n\nto make sure that more undead\n(or even worse things) don't come\nthrough, you'll have to seal hell's\nmaw from the other side. of course\nthis means you may get stuck in a\nvery unfriendly world, but no one\never said being a Heretic was easy!"},
	{                        "DEP_HERETIC_E2TEXT", "the mighty maulotaurs have proved\nto be no match for you, and as\ntheir steaming corpses slide to the\nground you feel a sense of grim\nsatisfaction that they have been\ndestroyed.\n\nthe gateways which they guarded\nhave opened, revealing what you\nhope is the way home. but as you\nstep through, mocking laughter\nrings in your ears.\n\nwas some other force controlling\nthe maulotaurs? could there be even\nmore horrific beings through this\ngate? the sweep of a crystal dome\noverhead where the sky should be is\ncertainly not a good sign...."},
	{                        "DEP_HERETIC_E3TEXT", "the death of d'sparil has loosed\nthe magical bonds holding his\ncreatures on this plane, their\ndying screams overwhelming his own\ncries of agony.\n\nyour oath of vengeance fulfilled,\nyou enter the portal to your own\nworld, mere moments before the dome\nshatters into a million pieces.\n\nbut if d'sparil's power is broken\nforever, why don't you feel safe?\nwas it that last shout just before\nhis death, the one that sounded\nlike a curse? or a summoning? you\ncan't really be sure, but it might\njust have been a scream.\n\nthen again, what about the other\nserpent riders?"},
	{                        "DEP_HERETIC_E4TEXT", "you thought you would return to your\nown world after d'sparil died, but\nhis final act banished you to his\nown plane. here you entered the\nshattered remnants of lands\nconquered by d'sparil. you defeated\nthe last guardians of these lands,\nbut now you stand before the gates\nto d'sparil's stronghold. until this\nmoment you had no doubts about your\nability to face anything you might\nencounter, but beyond this portal\nlies the very heart of the evil\nwhich invaded your world. d'sparil\nmight be dead, but the pit where he\nwas spawned remains. now you must\nenter that pit in the hopes of\nfinding a way out. and somewhere,\nin the darkest corner of d'sparil's\ndemesne, his personal bodyguards\nawait your arrival ..."},
	{                        "DEP_HERETIC_E5TEXT", "as the final maulotaur bellows his\ndeath-agony, you realize that you\nhave never come so close to your own\ndestruction. not even the fight with\nd'sparil and his disciples had been\nthis desperate. grimly you stare at\nthe gates which open before you,\nwondering if they lead home, or if\nthey open onto some undreamed-of\nhorror. you find yourself wondering\nif you have the strength to go on,\nif nothing but death and pain await\nyou. but what else can you do, if\nthe will to fight is gone? can you\nforce yourself to continue in the\nface of such despair? do you have\nthe courage? you find, in the end,\nthat it is not within you to\nsurrender without a fight. eyes\nwide, you go to meet your fate."},
	{                            "DEP_DOOM2TITLE", "DOOM 2: Hell on Earth"},
	{                            "DEP_DOOMUTITLE", "The Ultimate DOOM Startup"},
	{                             "DEP_DOOMTITLE", "DOOM Registered Startup"},
	{                            "DEP_DOOM1TITLE", "DOOM Shareware Startup"},
	
	/*** OTHER GAME STUFF ***/
	{					"FOUNDSECRET", "You found a secret area!"},
	{              "NETPLAYERRENAMED", "$1 renamed to $2."},
	{					 "GAMEPAUSED", "$1 paused the game."},
	{				   "GAMEUNPAUSED", "$1 resumed the game."},
	
	/*** ITEMS ***/
	{		"ITEM_GOTPRESSRELEASEBFG", "You got the press release BFG9000!"},
	
	/*** DEMO STUFF ***/
	{		  "BADDEMO_LEVELNOTFOUND", "The level named \"$1\" was not found."},
	{  "BADDEMO_LOADGAMENOTSUPPORTED", "Loading save games are not supported."},
	{  "BADDEMO_SAVEGAMENOTSUPPORTED", "Saving games is not supported."},
	{	 "BADDEMO_NETVARNOTSUPPORTED", "The variable $1 is not supported."},
	{		   "BADDEMO_UNKNOWNXDCMD", "Unknown extra command $1."},
	{        "BADDEMO_UNKNOWNFACTORY", "Unknown demo factory."},
	{           "BADDEMO_NONHOSTDEMO", "Demo possibly not recorded by the host, if so then the demo will most likely fail to play back at all."},
	{		  "BADDEMO_ILLEGALHEADER", "Demo contains an illegal header."},
	{		  "BADDEMO_UNHANDLEDDATA", "Unhandled data labeled \"$1\"."},
	{			 "BADDEMO_SKIPPEDTIC", "Demo skipped to tic $1, expected tic $1."},
	{				 "BADDEMO_DESYNC", "Demo desynced at tic $1, expected {$2, $4} but got {$3, $5}."},
	
	/*** D_PROF.C ***/
	{			 "DPROFC_CREATEUSAGE", "Usage: $1 create <Name> (UUID)"},
	{			"DPROFC_ALREADYEXIST", "Profile \"$1\" already exists!"},
	{			"DPROFC_FAILEDCREATE", "Failed to create profile."},
	{			"DPROFC_CONTROLUSAGE", "Usage: $1 control <Name> <Action> <Id> <Key>"},
	{			  "DPROFC_MAXISUSAGE", "Usage: $1 maxis <Name> <Group> <Axis> <Action>"},
	{			  "DPROFC_JAXISUSAGE", "Usage: $1 jaxis <Name> <Group> <Axis> <Action>"},
	{				"DPROFC_NOTFOUND", "Profile $1 not found."},
	{		  "DPROFC_NOTCONTROLNAME", "$1 is not a valid control."},
	{			  "DPROFC_VALUEUSAGE", "Usage: $1 value <Option> <Value>"},
	{		 "DPROFC_INDEXOUTOFRANGE", "Index $1 out of range."},
	
	/*** D_NET.C ***/
	{		 "DNETC_SOCKFAILEDTOOPEN", "The UDPv$1 socket failed to open."},
	{			  "DNETC_BOUNDTOPORT", "UDPv$1 socket bound to port $2."},
	{			  "DNETC_CONNECTFROM", "New connection from \"$1\" (ReMooD $2.$3$4)"},
	{		   "DNETC_CLIENTNOWREADY", "Client $1 is now ready."},
	{			  "DNETC_CONSISTFAIL", "Consistency Failure"},
	{			  "DNETC_PLEASERECON", "Please Reconnect"},
	{			"DNETC_JOININGPLAYER", "Joining player \"$1{z\"."},
	{			"DNETC_PLAYERLISTENT", "$1 $2 $3{z"},
	{				   "DNETC_BADURI", "The URI \"$1\" is not valid."},
	{				 "DNETC_BINDFAIL", "Failed to bind to \"$1\"."},
	{		 "DNETC_SERVERCHANGENAME", "Server changed name to \"$1\"."},
	{				"DNETC_CONNECTTO", "Connecting to \"$1\"."},
	{				"DNETC_SERVERWAD", "Selecting WAD \"$1\"."},
	{			"DNETC_HOSTNORESOLVE", "Host \"$1\" does not resolve."},
	{			  "DNETC_HOSTRESFOUR", "Host \"$1\" resolves to $2.$3.$4.$5 (v4)."},
	{			   "DNETC_HOSTRESSIX", "Host \"$1\" resolves to $2$3:$4:$5:$6:$7%$8 (v6)."},
	
	/*** D_RMOD.C ***/
	{		"DRMOD_NAMESPACENOTINWAD", "Namespace \"$2\" not in WAD \"$1\"."},
	{			"DRMOD_DATASTREAMERR", "Failed to open datastream for \"$1\" in WAD \"$1\"."},
	{			   "DRMOD_PARSEERROR", "Parse error in \"$3\" ($1), at $2:$9->$4:$5 ($7) ($6 [$8])."},
	
	/*** COMMAND.C ***/
	{	  "COMMANDC_WOULDHASHCOLLIDE", "Variable \"$1\" hash collides with \"$2\"."},
	
	/*** ST_STUFF.C ***/
	{			"STSTUFFC_SCOREBOARD", "Scoreboard"},
	
	/*** T_VM.C **/
	{				 "TVMC_COMPILING", "Compiling \"$1\"..."},
	{					 "TVMC_ERROR", "Compile error on line $1 ($2)."},
	{				   "TVMC_SUCCESS", "Successfuly compiled \"$1\"."},
	{		   "TVMC_GLOBALSCRIPTNUM", "WARNING: Script with ID $1 in global namespace, this is not recommended."},
	
	/*** B_GHOST.C ***/
	{			   "BGHOSTC_BASEINIT", "Registering bot registration..."},
	
	/*** D_MAIN.C ***/
	{			 "DMAINC_JOYINSTRUCT", "Hold joystick towards\ndirection to control."},
	{				 "DMAINC_PLAYER1", "{x70P1"},
	{				 "DMAINC_PLAYER2", "{x71P2"},
	{				 "DMAINC_PLAYER3", "{x72P3"},
	{				 "DMAINC_PLAYER4", "{x73P4"},
	
	/*** IP_*.c ***/
	{				 "IPC_CREATECONN", "Creating connection \"$1\" ($2)"},
	{		   "IPC_NOHOSTCANNOTBIND", "Host \"$1:$2\" not found, binding will not be possible."},
	{			  "IPC_SVREQUESTINFO", "Requesting server information..."},
	{					 "IPC_UNKODA", "Unknown message type $1."},
	{				  "IPC_SVGOTINFO", "Obtained server information."},
	{			   "IPC_SVSERVERSYNC", "Synchronizing with server."},
	{			  "IPC_SVUNKENCODING", "Unknown message encoding."},
	{				"IPC_ODAVIRTCVAR", "Odamex CVAR change: \"$1\" to \"$2\"."},
	
	/*** W_WAD.C ***/
	{		   "WWADC_WADSTILLLINKED", "WAD \"$1\" is still linked in."},
	{		   "WWADC_CHECKINGTHESUM", "Found \"$1\", confirming checksum."},
	
	/*** INFO.C ***/
	{					"INFOC_NODEH", "WAD \"$1\" contains no DEHACKED lump."},
	{				  "INFOC_NOTADEH", "WAD \"$1\" contains illegal DEHACKED lump."},
	{				"INFOC_BINARYDEH", "WAD \"$1\" contains unsupported binary DEHACKED lump."},
	{				 "INFOC_OLDPFDEH", "WAD \"$1\" contains old DEHACKED lump, which may fail."},
	{			  "INFOC_DEHNOSPRMAP", "No sprite mappings, DEHACKED not operational."},
	
	/*** D_BLOCK.C ***/
	{		 "DBLOCKC_ZLIBINFLATEERR", "ZLib inflation error $1."},
	
	/*** D_NWLINE.C ***/
	{			 "DNWLINE_LOCKEDDOOR", "This door is locked."},
	
	/*** P_SPEC.C ***/
	{		"PSPECC_TIMELIMITREACHED", "Time limit reached."},
	{		"PSPECC_FIVEMINLEFT", "5 minutes left!"},
	{		"PSPECC_ONEMINLEFT", "1 minute left!"},
	{		"PSPECC_THIRTYSECLEFT", "30 seconds left!"},
	{		"PSPECC_TENSECLEFT", "10 seconds left!"},
	{		"PSPECC_FIVESECLEFT", "5 seconds left!"},
	{		"PSPECC_FOURSECLEFT", "4 seconds left!"},
	{		"PSPECC_THREESECLEFT", "3 seconds left!"},
	{		"PSPECC_TWOSECLEFT", "2 seconds left!"},
	{		"PSPECC_ONESECLEFT", "1 second left!"},
	
	/*** p_INTER.c ***/
	{	   "PINTERC_FRAGLIMITREACHED", "Frag limit reached."},
	
	/*** P_DEMCMP.C ***/
	{               "M_PGS_NOTHINGHERE", "Nothing"},
	{               "D_PGS_NOTHINGHERE", "Nothing is here"},
	{       "M_PGS_COENABLEBLOODSPLATS", "Enable Blood Splats"},
	{       "D_PGS_COENABLEBLOODSPLATS", "Enables blood spats on walls. [Legacy >= 1.29]"},
	{          "M_PGS_CORANDOMLASTLOOK", "Randomize Monster Last Look"},
	{          "D_PGS_CORANDOMLASTLOOK", "Randomize monster's last look (player to target). [Legacy >= 1.29]"},
	{        "M_PGS_COUNSHIFTVILERAISE", "Unshift Arch-Vile Resurrection"},
	{        "D_PGS_COUNSHIFTVILERAISE", "Multiply the corpse height by 4 on resurrects. [Legacy < 1.29]"},
	{            "M_PGS_COMODIFYCORPSE", "Modify Corpse (Solid Corpses)"},
	{            "D_PGS_COMODIFYCORPSE", "Enables correct corpse modification for solid corpses. [Legacy >= 1.31]"},
	{           "M_PGS_CONOSMOKETRAILS", "No Smoke Trails"},
	{           "D_PGS_CONOSMOKETRAILS", "Disable smoke trails on rockets and lost souls [Legacy < 1.11]"},
	{            "M_PGS_COUSEREALSMOKE", "Use Real Smoke For Trails"},
	{            "D_PGS_COUSEREALSMOKE", "Use actual smoke rather than tracers for trails. [Legacy >= 1.25]"},
	{      "M_PGS_COOLDCUTCORPSERADIUS", "Cut Corpse Radius (Solid Corpse)"},
	{      "D_PGS_COOLDCUTCORPSERADIUS", "Reduce corpse radius, when co_modifycorpse is off. [Legacy >= 1.12]"},
	{    "M_PGS_COSPAWNDROPSONMOFLOORZ", "Spawn Drops on Fake-Floor"},
	{    "D_PGS_COSPAWNDROPSONMOFLOORZ", "Item drops on the fake floor not on the sector floor. [Legacy >= 1.32]"},
	{         "M_PGS_CODISABLETEAMPLAY", "Disable Team Play"},
	{         "D_PGS_CODISABLETEAMPLAY", "Disable support for team play. [Legacy < 1.25]"},
	{             "M_PGS_COSLOWINWATER", "Move Slower In Water"},
	{             "D_PGS_COSLOWINWATER", "Move slower when underwater (in 3D Water). [Legacy >= 1.28]"},
	{         "M_PGS_COSLIDEOFFMOFLOOR", "Dead Things Slide Off Fake Floors"},
	{         "D_PGS_COSLIDEOFFMOFLOOR", "Slide off the near floor not the real one (3D floors). [Legacy >= 1.32]"},
	{         "M_PGS_COOLDFRICTIONMOVE", "Old Friction Movement"},
	{         "D_PGS_COOLDFRICTIONMOVE", "Use old friction when moving. [Legacy < 1.32]"},
	{           "M_PGS_COOUCHONCEILING", "Go Ouch When Hitting Ceiling"},
	{           "D_PGS_COOUCHONCEILING", "Go ouch when hitting the ceiling. [Legacy >= 1.12]"},
	{          "M_PGS_COENABLESPLASHES", "Enable Water Splashes"},
	{          "D_PGS_COENABLESPLASHES", "Enable splashes on the water. [Legacy >= 1.25]"},
	{        "M_PGS_COENABLEFLOORSMOKE", "Enable Floor Damage Smoke"},
	{        "D_PGS_COENABLEFLOORSMOKE", "Enable smoke when on a damaging floor. [Legacy >= 1.25]"},
	{             "M_PGS_COENABLESMOKE", "Enable Smoke"},
	{             "D_PGS_COENABLESMOKE", "Enable smoke. [Legacy >= 1.25]"},
	{            "M_PGS_CODAMAGEONLAND", "Instant-Damage On Special Floors"},
	{            "D_PGS_CODAMAGEONLAND", "Damage when landing on a damaging floor. [Legacy >= 1.25]"},
	{           "M_PGS_COABSOLUTEANGLE", "Use Absolute Angles"},
	{           "D_PGS_COABSOLUTEANGLE", "Use absolute angle rather than relative angle. [Legacy >= 1.25]"},
	{             "M_PGS_COOLDJUMPOVER", "Old Jump Over"},
	{             "D_PGS_COOLDJUMPOVER", "Use old jump over code. [Legacy < 1.28]"},
	{            "M_PGS_COENABLESPLATS", "Enable Splats"},
	{            "D_PGS_COENABLESPLATS", "Enable splats. [Legacy >= 1.28]"},
	{       "M_PGS_COOLDFLATPUSHERCODE", "Old Pusher/Puller Code"},
	{       "D_PGS_COOLDFLATPUSHERCODE", "Use pusher/puller code that cannot handle 3D Floors. [Legacy <= 1.40]"},
	{       "M_PGS_COSPAWNPLAYERSEARLY", "Spawn Players Earlier"},
	{       "D_PGS_COSPAWNPLAYERSEARLY", "Spawn players while the map is loading. [Legacy >= 1.28]"},
	{       "M_PGS_COENABLEUPDOWNSHOOT", "Enable Up/Down Aim"},
	{       "D_PGS_COENABLEUPDOWNSHOOT", "Enable shooting up/down when not aiming at something. [Legacy >= 1.28]"},
	{       "M_PGS_CONOUNDERWATERCHECK", "No Underwater Check"},
	{       "D_PGS_CONOUNDERWATERCHECK", "Do not check for an object being underwater. [Legacy < 1.28]"},
	{        "M_PGS_COSPLASHTRANSWATER", "Water Transition Splash"},
	{        "D_PGS_COSPLASHTRANSWATER", "Causes splashes when transitioning from/to water [Legacy >= 1.32]"},
	{            "M_PGS_COUSEOLDZCHECK", "Old Z Check"},
	{            "D_PGS_COUSEOLDZCHECK", "Use old Z checking code rather than Heretic's. [Legacy < 1.31]"},
	{             "M_PGS_COCHECKXYMOVE", "Check X/Y Movement"},
	{             "D_PGS_COCHECKXYMOVE", "Check X/Y Movement (When co_useoldzcheck is enabled). [Legacy >= 1.12]"},
	{          "M_PGS_COWATERZFRICTION", "Water Z Friction"},
	{          "D_PGS_COWATERZFRICTION", "Apply Z friction movement when underwater. [Legacy >= 1.28]"},
	{      "M_PGS_CORANOMLASTLOOKSPAWN", "Randomize Last Look (Spawn)"},
	{      "D_PGS_CORANOMLASTLOOKSPAWN", "Choose random player when object spawns. [Legacy < 1.29]"},
	{"M_PGS_COALWAYSRETURNDEADSPMISSILE", "Return Dead Missiles"},
	{"D_PGS_COALWAYSRETURNDEADSPMISSILE", "Always return the missile spawned even if it dies. [Legacy < 1.31]"},
	{          "M_PGS_COUSEMOUSEAIMING", "Allow Mouse To Aim"},
	{          "D_PGS_COUSEMOUSEAIMING", "Use mouse aiming when not aimed at another object. [Legacy >= 1.28]"},
	{   "M_PGS_COFIXPLAYERMISSILEANGLE", "Fix Player Missiles"},
	{   "D_PGS_COFIXPLAYERMISSILEANGLE", "Fix player missiles being fired and make them more accurate. [Legacy >= 1.28]"},
	{          "M_PGS_COREMOVEMOINSKYZ", "Remove Missiles In Sky"},
	{          "D_PGS_COREMOVEMOINSKYZ", "When Z movement is performed in a sky, do not explode. [Legacy >= 1.29]"},
	{            "M_PGS_COFORCEAUTOAIM", "Force Auto-Aim"},
	{            "D_PGS_COFORCEAUTOAIM", "Always force auto-aim. [Legacy <= 1.11]"},
	{      "M_PGS_COFORCEBERSERKSWITCH", "Force Berserk Switch"},
	{      "D_PGS_COFORCEBERSERKSWITCH", "Force switching to berserk-enabled weapons in slots. [Legacy < 1.28]"},
	{       "M_PGS_CODOUBLEPICKUPCHECK", "Double-Check Pickup"},
	{       "D_PGS_CODOUBLEPICKUPCHECK", "Double check for pickups rather than just once. [Legacy >= 1.32]"},
	{"M_PGS_CODISABLEMISSILEIMPACTCHECK", "No Missile Impact Check"},
	{"D_PGS_CODISABLEMISSILEIMPACTCHECK", "Disable the checking of missile impacts. [Legacy < 1.32]"},
	{      "M_PGS_COMISSILESPLATONWALL", "Enable Missile Splats on Walls"},
	{      "D_PGS_COMISSILESPLATONWALL", "When missiles hit walls, they splat it. [Legacy >= 1.29]"},
	{     "M_PGS_CONEWBLOODHITSCANCODE", "Blood Splat Tracers"},
	{     "D_PGS_CONEWBLOODHITSCANCODE", "Use newer blood spawning code when tracing hitscans. [Legacy >= 1.25]"},
	{           "M_PGS_CONEWAIMINGCODE", "New Aiming Code"},
	{           "D_PGS_CONEWAIMINGCODE", "Use newer aiming code in P_AimLineAttack(). [Legacy >= 1.28]"},
	{          "M_PGS_COMISSILESPECHIT", "Missiles Trigger Special Line Hits"},
	{          "D_PGS_COMISSILESPECHIT", "Missiles can trigger special line hits. [Legacy >= 1.32]"},
	{    "M_PGS_COHITSCANSSLIDEONFLATS", "Hitscans Slide On Floor"},
	{    "D_PGS_COHITSCANSSLIDEONFLATS", "Hitscans slide on flats. [Legacy < 1.12]"},
	{     "M_PGS_CONONSOLIDPASSTHRUOLD", "No Solid Pass-Thru A"},
	{     "D_PGS_CONONSOLIDPASSTHRUOLD", "Non-solid objects pass through others (Old trigger). [Legacy < 1.12]"},
	{     "M_PGS_CONONSOLIDPASSTHRUNEW", "No Solid Pass-Thru B"},
	{     "D_PGS_CONONSOLIDPASSTHRUNEW", "Non-solid objects pass through others (New trigger). [Legacy >= 1.32]"},
	{               "M_PGS_COJUMPCHECK", "Check For Jump Over"},
	{               "D_PGS_COJUMPCHECK", "Allow jump over to take effect. [Legacy >= 1.12]"},
	{       "M_PGS_COLINEARMAPTRAVERSE", "Linear Map Traverse"},
	{       "D_PGS_COLINEARMAPTRAVERSE", "Loads a new map rather than starting from fresh. [Legacy < 1.29]"},
	{       "M_PGS_COONLYTWENTYDMSPOTS", "Limit to 20 DM Starts"},
	{       "D_PGS_COONLYTWENTYDMSPOTS", "Support only 20 DM starts rather than 64. [Legacy < 1.23]"},
	{        "M_PGS_COALLOWSTUCKSPAWNS", "Allow stuck DM spawns"},
	{        "D_PGS_COALLOWSTUCKSPAWNS", "Allow players getting stuck in others in DM spots. [Legacy < 1.13]"},
	{             "M_PGS_COUSEOLDBLOOD", "Use Old Doom Blood"},
	{             "D_PGS_COUSEOLDBLOOD", "Uses standard Doom blood rather than Legacy blood. [Legacy < 130]"},
	{             "M_PGS_FUNMONSTERFFA", "Monster Free For All"},
	{             "D_PGS_FUNMONSTERFFA", "Monsters enter a Free For All and attack anything in sight."},
	{             "M_PGS_FUNINFIGHTING", "Monsters Infight"},
	{             "D_PGS_FUNINFIGHTING", "Monsters attack monsters of the same race."},
	{       "M_PGS_COCORRECTVILETARGET", "Correct Position Of Vile Vire"},
	{       "D_PGS_COCORRECTVILETARGET", "Correct the position of the Arch-Vile target fire. [ReMooD >= 1.0a]"},
	{       "M_PGS_FUNMONSTERSMISSMORE", "Monsters Miss More"},
	{       "D_PGS_FUNMONSTERSMISSMORE", "Monsters miss their target more."},
	{        "M_PGS_COMORECRUSHERBLOOD", "More Crusher Blood"},
	{        "D_PGS_COMORECRUSHERBLOOD", "Make crushers spew more blood. [Legacy < 1.32]"},
	{          "M_PGS_CORANDOMBLOODDIR", "Random Blood Direction"},
	{          "D_PGS_CORANDOMBLOODDIR", "Spew blood in a random direction. [Legacy >= 1.28]"},
	{         "M_PGS_COINFINITEROCKETZ", "Infinite Rocket Z Range"},
	{         "D_PGS_COINFINITEROCKETZ", "Rocket damage distance on Z is infinite. [Legacy < 1.12]"},
	{      "M_PGS_COALLOWROCKETJUMPING", "Allow Rocket Jumping"},
	{      "D_PGS_COALLOWROCKETJUMPING", "Allow support for rocket jumping. [Legacy >= 1.29]"},
	{           "M_PGS_COROCKETZTHRUST", "Rocket Z Thrust"},
	{           "D_PGS_COROCKETZTHRUST", "Allow thrusting on the Z plane from rockets. [Legacy >= 1.24]"},
	{    "M_PGS_COLIMITMONSTERZMATTACK", "Limit Monster Z Melee Range"},
	{    "D_PGS_COLIMITMONSTERZMATTACK", "Limits Z Melee Attack range (stops melee from cliffs). [Legacy > 1.11]"},
	{         "M_PGS_HEREMONSTERTHRESH", "Heretic Monster Threshold"},
	{         "D_PGS_HEREMONSTERTHRESH", "Use Heretic Threshold Logic. [Heretic]"},
	{             "M_PGS_COVOODOODOLLS", "Enable Voodoo Dolls"},
	{             "D_PGS_COVOODOODOLLS", "Enable spawning of Voodoo Dolls. [Legacy < 1.28]"},
	{          "M_PGS_COEXTRATRAILPUFF", "Extra Smoke Trails"},
	{          "D_PGS_COEXTRATRAILPUFF", "Add extra puff for smoke. [Legacy < 1.25]"},
	{          "M_PGS_COLOSTSOULTRAILS", "Lost Soul Trails"},
	{          "D_PGS_COLOSTSOULTRAILS", "Lost souls emit smoke. [Legacy >= 1.25]"},
	{           "M_PGS_COTRANSTWOSIDED", "Transparent 2D Walls"},
	{           "D_PGS_COTRANSTWOSIDED", "Transparent two sided walls. [Legacy = 1.11]"},
	{         "M_PGS_COENABLEBLOODTIME", "Enable Blood Time"},
	{         "D_PGS_COENABLEBLOODTIME", "Enables setting blood time. [Legacy >= 1.23]"},
	{           "M_PGS_PLENABLEJUMPING", "Enable Jumping"},
	{           "D_PGS_PLENABLEJUMPING", "Enables Jumping Support [Legacy >= 1.12]"},
	{                "M_PGS_COMOUSEAIM", "Enable Mouse Aiming"},
	{                "D_PGS_COMOUSEAIM", "Enable mouse aiming [Legacy > 1.11]"},
	{        "M_PGS_MONRESPAWNMONSTERS", "Respawn Monsters"},
	{        "D_PGS_MONRESPAWNMONSTERS", "Monsters come back to life after a short delay."},
	{         "M_PGS_FUNNOTARGETPLAYER", "No Player Targetting"},
	{         "D_PGS_FUNNOTARGETPLAYER", "Monsters are incapable of targetting players."},
	{     "M_PGS_MONARCHVILEANYRESPAWN", "Arch-Viles Respawn Anything"},
	{     "D_PGS_MONARCHVILEANYRESPAWN", "Arch-Viles can ressurect anything regardless if it can be or not."},
	{        "M_PGS_COOLDCHECKPOSITION", "Old Position Checking"},
	{        "D_PGS_COOLDCHECKPOSITION", "Use old P_CheckPosition() Code. [Legacy < 1.42]"},
	{       "M_PGS_COLESSSPAWNSTICKING", "Less Spawn Spot Sticking"},
	{       "D_PGS_COLESSSPAWNSTICKING", "Make players getting stuck inside other players less likely to occur. [ReMooD >= 1.0a]"},
	{           "M_PGS_PLSPAWNTELEFRAG", "Tele-Frag When Spawning"},
	{           "D_PGS_PLSPAWNTELEFRAG", "Tele-frag when a player respawns (to empty the spot). [ReMooD >= 1.0a]"},
	{           "M_PGS_GAMEONEHITKILLS", "One Hit Kills"},
	{           "D_PGS_GAMEONEHITKILLS", "Any recieved damage kills."},
	{   "M_PGS_COBETTERPLCORPSEREMOVAL", "Better Body Management"},
	{   "D_PGS_COBETTERPLCORPSEREMOVAL", "Better management of player bodies so they do not litter everywhere. [ReMooD >= 1.0a]"},
	{         "M_PGS_PLSPAWNCLUSTERING", "Spawn Spot Clustering"},
	{         "D_PGS_PLSPAWNCLUSTERING", "Adds extra spawn spots near other spawn spots for more players. [ReMooD >= 1.0a]"},
	{      "M_PGS_COIMPROVEDMOBJONMOBJ", "Improved Object on Object"},
	{      "D_PGS_COIMPROVEDMOBJONMOBJ", "Improves handling of objects on top of other objects. [ReMooD >= 1.0a]"},
	{     "M_PGS_COIMPROVEPATHTRAVERSE", "Improve Traversing Move"},
	{     "D_PGS_COIMPROVEPATHTRAVERSE", "Smooth out position moving. [ReMooD >= 1.0a]"},
	{             "M_PGS_PLJUMPGRAVITY", "Jump Gravity"},
	{             "D_PGS_PLJUMPGRAVITY", "This is the amount of pushing force used when jumping."},
	{          "M_PGS_FUNNOLOCKEDDOORS", "No Locked Doors"},
	{          "D_PGS_FUNNOLOCKEDDOORS", "All doors are unlocked and do not need keys."},
	{           "M_PGS_GAMEAIRFRICTION", "Friction In Air"},
	{           "D_PGS_GAMEAIRFRICTION", "This modifies the amount of friction in the air, the higher the easier it is to move."},
	{         "M_PGS_GAMEWATERFRICTION", "Friction In Water"},
	{         "D_PGS_GAMEWATERFRICTION", "This modifies the amount of friction in water, the higher the easier it is to move."},
	{      "M_PGS_GAMEMIDWATERFRICTION", "Friction In Mid-Water"},
	{      "D_PGS_GAMEMIDWATERFRICTION", "This modifies the amount of friction in water when not touching the ground, the higher the easier it is to move."},
	{        "M_PGS_GAMEALLOWLEVELEXIT", "Allow Level Exiting"},
	{        "D_PGS_GAMEALLOWLEVELEXIT", "Allows players or monsters to exit the level."},
	{       "M_PGS_GAMEALLOWROCKETJUMP", "Allow Rocket Jumping"},
	{       "D_PGS_GAMEALLOWROCKETJUMP", "Enables the use of rocket jumping."},
	{            "M_PGS_PLALLOWAUTOAIM", "Allow Auto-Aiming"},
	{            "D_PGS_PLALLOWAUTOAIM", "Allows players to aim vertically automatically."},
	{       "M_PGS_PLFORCEWEAPONSWITCH", "Force Weapon Switch"},
	{       "D_PGS_PLFORCEWEAPONSWITCH", "Forces weapon switches on pickup. [Doom <= 1.09]"},
	{             "M_PGS_PLDROPWEAPONS", "Drop Weapons on Death"},
	{             "D_PGS_PLDROPWEAPONS", "Drops the player's weapon when they are killed."},
	{            "M_PGS_PLINFINITEAMMO", "Infinite Ammo"},
	{            "D_PGS_PLINFINITEAMMO", "Ammo is never depleted and lasts forever."},
	{        "M_PGS_GAMEHERETICGIBBING", "Heretic Gibbing"},
	{        "D_PGS_GAMEHERETICGIBBING", "Objects that can be gibbed are much easier to gib."},
	{        "M_PGS_MONPREDICTMISSILES", "Predict Missiles"},
	{        "D_PGS_MONPREDICTMISSILES", "Monsters predict missile targets and aim accordingly."},
	{    "M_PGS_MONRESPAWNMONSTERSTIME", "Monster Respawn Delay"},
	{    "D_PGS_MONRESPAWNMONSTERSTIME", "Time in seconds before monsters are respawned."},
	{        "M_PGS_PLSPAWNWITHMAXGUNS", "Spawn With Non-Super Weapons"},
	{        "D_PGS_PLSPAWNWITHMAXGUNS", "When a player is spawned, they have all non-super weapons."},
	{      "M_PGS_PLSPAWNWITHSUPERGUNS", "Spawn With Super Weapons"},
	{      "D_PGS_PLSPAWNWITHSUPERGUNS", "When a player is spawned, they have all super weapons."},
	{       "M_PGS_PLSPAWNWITHMAXSTATS", "Spawn With Max Stats"},
	{       "D_PGS_PLSPAWNWITHMAXSTATS", "When a player is spawned, they have max health and armor."},
	{         "M_PGS_ITEMSSPAWNPICKUPS", "Spawn Pickups"},
	{         "D_PGS_ITEMSSPAWNPICKUPS", "Spawn pickups on map load."},
	{         "M_PGS_COHERETICFRICTION", "Heretic Friction"},
	{         "D_PGS_COHERETICFRICTION", "Use Heretic Friction"},
	{            "M_PGS_GAMEDEATHMATCH", "Deathmatch"},
	{            "D_PGS_GAMEDEATHMATCH", "Enables Deathmatch Mode (Player vs Player)"},
	{        "M_PGS_PLSPAWNWITHALLKEYS", "Spawn With All Keys"},
	{        "D_PGS_PLSPAWNWITHALLKEYS", "When a player is spawned, they have every key."},
	{          "M_PGS_ITEMSKEEPWEAPONS", "Keep Weapons On Floor"},
	{          "D_PGS_ITEMSKEEPWEAPONS", "Keep weapons on the ground when picked up."},
	{              "M_PGS_GAMETEAMPLAY", "Enable Team Play"},
	{              "D_PGS_GAMETEAMPLAY", "Enable team mode, teams of different colors vs others."},
	{            "M_PGS_GAMETEAMDAMAGE", "Allow Team Damage"},
	{            "D_PGS_GAMETEAMDAMAGE", "Allow players on the same team to hurt each other."},
	{             "M_PGS_GAMEFRAGLIMIT", "Frag Limit"},
	{             "D_PGS_GAMEFRAGLIMIT", "How many frags a player or team must obtain before they achieve victory,"},
	{             "M_PGS_GAMETIMELIMIT", "Time Limit"},
	{             "D_PGS_GAMETIMELIMIT", "Time in minutes before the game automatically ends."},
	{      "M_PGS_MONSTATICRESPAWNTIME", "Static Monster Respawn Time"},
	{      "D_PGS_MONSTATICRESPAWNTIME", "Monsters always come back after respawn delay rather than randomly afterwards."},
	{           "M_PGS_PLFASTERWEAPONS", "Weapons Are Faster"},
	{           "D_PGS_PLFASTERWEAPONS", "Weapons fire and move a bit faster (all speeds are set to 1)."},
	{          "M_PGS_MONSPAWNMONSTERS", "Spawn Monsters"},
	{          "D_PGS_MONSPAWNMONSTERS", "Monsters are spawned when the level loads."},
	{      "M_PGS_GAMESPAWNMULTIPLAYER", "Spawn Multi-Player Objects"},
	{      "D_PGS_GAMESPAWNMULTIPLAYER", "Spawn any items that are marked multiplayer (extra weapons, monsters, etc.)"},
	{          "M_PGS_ITEMRESPAWNITEMS", "Respawn Items"},
	{          "D_PGS_ITEMRESPAWNITEMS", "Items are respawned after they are picked up."},
	{      "M_PGS_ITEMRESPAWNITEMSTIME", "Item Respawn Delay"},
	{      "D_PGS_ITEMRESPAWNITEMSTIME", "Time in seconds before items are respawned."},
	{           "M_PGS_MONFASTMONSTERS", "Fast Monsters"},
	{           "D_PGS_MONFASTMONSTERS", "Monster move faster and their attacks are also faster."},
	{          "M_PGS_GAMESOLIDCORPSES", "Solid Corpses"},
	{          "D_PGS_GAMESOLIDCORPSES", "Corpses on the ground are solidified and could be killed again."},
	{             "M_PGS_GAMEBLOODTIME", "Blood Time"},
	{             "D_PGS_GAMEBLOODTIME", "Time in seconds blood will last on the gound."},
	{               "M_PGS_GAMEGRAVITY", "Gravity"},
	{               "D_PGS_GAMEGRAVITY", "The multiplier to the amount of downward force to apply to players that are in the air."},
	{          "M_PGS_MONENABLECLEANUP", "Enable Corpse Cleanup"},
	{          "D_PGS_MONENABLECLEANUP", "Enable clean up of dead monsters. [ReMooD >= 1.0a]"},
	{        "M_PGS_MONCLEANUPRESPTIME", "Respawnable Cleanup Time"},
	{        "D_PGS_MONCLEANUPRESPTIME", "Time in minutes before dead respawnable monsters are cleaned up."},
	{        "M_PGS_MONCLEANUPNONRTIME", "Non-Respawnable Cleanup Time"},
	{        "D_PGS_MONCLEANUPNONRTIME", "Time in minutes before dead respawnable monsters are cleaned up."},
	{                 "M_PGS_GAMESKILL", "Skill Level"},
	{                 "D_PGS_GAMESKILL", "The current difficulty of the level, the higher the more monsters that appear."},
	{              "M_PGS_PLHALFDAMAGE", "Take Half Damage"},
	{              "D_PGS_PLHALFDAMAGE", "Players recieve only half of normal damage they recieve."},
	{              "M_PGS_PLDOUBLEAMMO", "Get Double Ammo"},
	{              "D_PGS_PLDOUBLEAMMO", "Players recieve double the amount of ammo they pickup."},
	{          "M_PGS_MONKILLCOUNTMODE", "Kill Count Mode"},
	{          "D_PGS_MONKILLCOUNTMODE", "Specifies the mode at which kill totals are calculated."},
	{             "M_PGS_COOLDBFGSPRAY", "Old BFG Spray"},
	{             "D_PGS_COOLDBFGSPRAY", "Use BFG Ball owner as inflictor rather than the ball itself. [Legacy < 1.32]"},
	{         "M_PGS_COEXPLODEHITFLOOR", "Hit Floor When A_Explode"},
	{         "D_PGS_COEXPLODEHITFLOOR", "When a state calls A_Explode() the floor is hit. [Legacy >= 1.32]"},
	{           "M_PGS_COBOMBTHRUFLOOR", "Bomb Through Floor"},
	{           "D_PGS_COBOMBTHRUFLOOR", "Explosions bleed through floors. [Legacy < 1.32]"},
	{           "M_PGS_COOLDEXPLOSIONS", "Use Old Explosions"},
	{           "D_PGS_COOLDEXPLOSIONS", "Use older explosion code which cannot handle certain aspects. [Legacy < 1.32]"},
	{       "M_PGS_COAIMCHECKFAKEFLOOR", "Check 3D Floor When Aiming"},
	{       "D_PGS_COAIMCHECKFAKEFLOOR", "Checks local 3D floors to determine if it is possible to aim through them. [Legacy >= 1.32]"},
	{          "M_PGS_CONEWGUNSHOTCODE", "Use New Gunshot Code"},
	{          "D_PGS_CONEWGUNSHOTCODE", "Use logically incorrect gunshot code. [Legacy >= 1.32]"},
	{     "M_PGS_COSHOOTCHECKFAKEFLOOR", "Check 3D Floor When Shooting"},
	{     "D_PGS_COSHOOTCHECKFAKEFLOOR", "Checks local 3D floors to determine if it is possible to shoot through them. [Legacy >= 1.32]"},
	{      "M_PGS_COSHOOTFLOORCLIPPING", "Clip Tracers To Surface"},
	{      "D_PGS_COSHOOTFLOORCLIPPING", "When tracing clip against the floor rather than going through it. [Legacy >= 1.32]"},
	{            "M_PGS_CONEWSSGSPREAD", "Use New SSG Spread"},
	{            "D_PGS_CONEWSSGSPREAD", "Use a subtle change in the SSG spread code. [Legacy >= 1.32]"},
	{   "M_PGS_COMONSTERLOOKFORMONSTER", "Monsters Look For Other Monsters"},
	{   "D_PGS_COMONSTERLOOKFORMONSTER", "This enables monsters to look for other monsters. [ReMooD >= 1.0a]"},
	{         "M_PGS_COOLDTHINGHEIGHTS", "Use Old Thing Heights"},
	{         "D_PGS_COOLDTHINGHEIGHTS", "Use the older heights of objects that were changed in Legacy. [Legacy < 1.32]"},
	{      "M_PGS_COLASTLOOKMAXPLAYERS", "Last Look MAXPLAYERS"},
	{      "D_PGS_COLASTLOOKMAXPLAYERS", "This is the modulo value when calculating the randomized last look for spawns."},
	{      "M_PGS_COMOVECHECKFAKEFLOOR", "Check 3D Floor When Moving"},
	{      "D_PGS_COMOVECHECKFAKEFLOOR", "Checks 3D floors during movement. [Legacy >= 1.32]"},
	{             "M_PGS_COMULTIPLAYER", "Multi-Player Mode"},
	{             "D_PGS_COMULTIPLAYER", "Enables multiplayer mode checks."},
	{             "M_PGS_COBOOMSUPPORT", "Boom Support"},
	{             "D_PGS_COBOOMSUPPORT", "Allows changes used from Boom in certain areas. [Legacy >= 1.32]"},
	{         "M_PGS_PLSPAWNWITHFAVGUN", "Spawn With Favorite Gun"},
	{         "D_PGS_PLSPAWNWITHFAVGUN", "Allows players to start with their favorite gun when they respawn. [ReMooD >= 1.0a]"},
	{             "M_PGS_CONOSAWFACING", "No Chainsaw Facing"},
	{             "D_PGS_CONOSAWFACING", "When chainsawing enemies you face their direction to continue harming them."},
	{      "M_PGS_COENABLETEAMMONSTERS", "Enable Team Monsters"},
	{      "D_PGS_COENABLETEAMMONSTERS", "Allows monsters to be placed on a team along with players. [ReMooD >= 1.0a]"},
	{       "M_PGS_COMONSTERDEADTARGET", "Monsters Untarget Dead"},
	{       "D_PGS_COMONSTERDEADTARGET", "When the monster's target dies it no longer targets them. [ReMooD >= 1.0a]"},
	{          "M_PGS_COJUMPREGARDLESS", "Ignore Jump Disable"},
	{          "D_PGS_COJUMPREGARDLESS", "This allows jumping regardless if it is enabled or not, this is for old demos. [ReMooD < 1.0a]"},
	{        "M_PGS_COOLDLASTLOOKLOGIC", "Use Old Lastlook Logic"},
	{        "D_PGS_COOLDLASTLOOKLOGIC", "Use older method of computing an object's last look target. [Legacy < 1.30]"},
	{        "M_PGS_CORADIALSPAWNCHECK", "Radial Spawn Check"},
	{        "D_PGS_CORADIALSPAWNCHECK", "Enable radial spawn checking to help prevent stuck spawns. [ReMooD >= 1.0a]"},
	{    "M_PGS_MONENABLEPLAYASMONSTER", "Enable Playing as Monsters"},
	{    "D_PGS_MONENABLEPLAYASMONSTER", "Allows players to take control over monsters."},
	{        "M_PGS_COKILLSTOPLAYERONE", "Give Kills to Player 1"},
	{        "D_PGS_COKILLSTOPLAYERONE", "Gives kills performed by non-players to player 1. [ReMooD < 1.0a]"},
	{            "M_PGS_PLALLOWSUICIDE", "Allow Suicide Pill"},
	{            "D_PGS_PLALLOWSUICIDE", "Allows player to use the suicide pill key to instantly commit suicide [ReMooD >= 1.0a]."},
	{            "M_PGS_PLSUICIDEDELAY", "Suicide Pill Delay"},
	{            "D_PGS_PLSUICIDEDELAY", "Time in seconds after allowing another suicide pill after consuming an existing suicide pill [ReMooD >= 1.0a]."},
	{      "M_PGS_PLSPAWNWITHMELEEONLY", "Spawn With Only Melee Weapons"},
	{      "D_PGS_PLSPAWNWITHMELEEONLY", "When a player is spawned, they are only spawned with melee weapons. Spawn with non-super/super affects this option."},
	{      "M_PGS_PLSPAWNWITHRANDOMGUN", "Spawn With A Random Weapon"},
	{      "D_PGS_PLSPAWNWITHRANDOMGUN", "When a player is spawned, they are spawned with a random weapon. Spawn with non-super/super/melee affects this option."},
	{            "M_PGS_COENABLESLOPES", "Enables Slopes"},
	{            "D_PGS_COENABLESLOPES", "Enables the usage of sloped floors and ceilings. [ReMooD >= 1.0a]"},
	{			  "M_PGS_FUNFLIPLEVELS", "Flip Levels"},
	{			  "D_PGS_FUNFLIPLEVELS", "Levels are horizontaly flipped."},
	{		  "M_PGS_PLREDUCEINVENTORY", "Reduce Inventory on Exit"},
	{		  "D_PGS_PLREDUCEINVENTORY", "All items in the player inventory are reduced to a single item, that is all stacks are lost."},
	{			"M_PGS_CODISPLACESPAWN", "Displawn Spawning"},
	{			"D_PGS_CODISPLACESPAWN", "If the standard spawners and/or cluster spawners fail, a displace spawn will occur. This spawns the failed player next to an adjacent player. Used in cooperative and team modes only."},
	{	   "M_PGS_CORESPAWNCORPSESONLY", "Respawn Corpses Only"},
	{	   "D_PGS_CORESPAWNCORPSESONLY", "Only objects that are dead are respawned when respawn monsters is enabled."},
};

/* DS_NameOfString() -- Returns name of pointer to string */
const char* DS_NameOfString(char** const WCharStr)
{
	size_t i;
	
	/* Check */
	if (!WCharStr)
		return NULL;
		
	/* Look through all */
	for (i = 0; i < NUMUNICODESTRINGS; i++)
		if (WCharStr == &UnicodeStrings[i].wcharstr)
			return UnicodeStrings[i].id;
			
	/* Failure */
	return NULL;
}

/* DS_FindStringRef() -- Finds string reference by name */
const char** DS_FindStringRef(const char* const a_StrID)
{
	static bool_t Booted = false;
	size_t i;
	uint32_t Hash;
	
	/* Not booted? */
	if (!Booted)
	{
		// Hash every string
		for (i = 0; i < NUMUNICODESTRINGS; i++)
			UnicodeStrings[i].Hash = Z_Hash(UnicodeStrings[i].id);
		
		// Now it is booted
		Booted = true;
	}
	
	/* Check */
	if (!a_StrID)
		return NULL;
	
	/* Hash string */
	Hash = Z_Hash(a_StrID);
	
	/* Look through every string */
	for (i = 0; i < NUMUNICODESTRINGS; i++)
		if (Hash == UnicodeStrings[i].Hash)
			if (strcasecmp(a_StrID, UnicodeStrings[i].id) == 0)
				return &UnicodeStrings[i].wcharstr;
	
	/* Not found */
	return NULL;
}

/* D_USPrint() -- Prints internationalized string */
size_t D_USPrint(char* const a_OutBuf, const size_t a_OutSize, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr)
{
#define BUFSIZE 128
#define SMALLBUF 32
#define HACKBUF 128
	char MiniBuf[BUFSIZE];
	char SmallBuf[SMALLBUF];
	char HackBuf[HACKBUF];
	
	char* Points[11];
	char* o, *oe, *f, *Sym, *NextSym, *h;
	const char* i, *iold;
	int SpecialNum, sn, len, zz, hl;
	int HackLeft;
	bool_t SpecialPoint, NewLine;
	va_list ArgPtrCopy;
	
	void* PtrType;
	int IntType;
	long LongType;
	
	/* Check */
	if (a_StrID < 0 || a_StrID >= NUMUNICODESTRINGS)
		return 0;
	
	/* Copy Arguments */
	__REMOOD_VA_COPY(ArgPtrCopy, a_ArgPtr);
	
	/* Clear Mini Buffer */
	memset(MiniBuf, 0, sizeof(MiniBuf));
	memset(Points, 0, sizeof(Points));
	memset(HackBuf, 0, sizeof(HackBuf));
	
	/* Read and parse specials */
	// Clone small buffer
	memset(SmallBuf, 0, sizeof(SmallBuf));
	strncat(SmallBuf, a_Format, SMALLBUF);
	
	// Newline?
	NewLine = !!strchr(a_Format, '\n');
	
	// Go through format
	NewLine = !!(*a_Format == '\n');
	HackLeft = HACKBUF - 1;
	h = HackBuf;
	sn = 0;
	for (Sym = strchr(SmallBuf, '%'); Sym; Sym = NextSym)
	{
		// Get occurance of next symbol
		NextSym = strchr(Sym + 1, '%');
		
		// Does not exist?
		if (!NextSym)
		{
			// Go to \n if it exists
			oe = strchr(Sym + 1, '\n');
			
			// And it that does not exist, go to the end
			if (!oe)
				oe = &Sym[strlen(Sym)];
			else
				NewLine = true;
		}
		
		// Otherwise set holder to it
		else
			oe = NextSym;
		
		// Convert to NUL
		*oe = 0;
		
		// Determine the sequence to print
		f = oe - 1;
		
		// Only if there is room in the albiet small buffer
		if (HackLeft > 0)
		{
			switch (*f)
			{
					// Integers
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
				case 'c':
					// Long
					if (*(f - 1) == 'l')
					{
						LongType = va_arg(a_ArgPtr, long);
						snprintf(h, HackLeft, Sym, LongType);
					}
				
					// Standard
					else
					{
						IntType = va_arg(a_ArgPtr, int);
						snprintf(h, HackLeft, Sym, IntType);
					}
					break;
			
					// Pointers
				case 'p':
				case 's':
					PtrType = va_arg(a_ArgPtr, void*);
					snprintf(h, HackLeft, Sym, PtrType);
					break;
				
					// Unknown
				default:
					break;
			}
		
			if (sn <= 9)
			{
				// Place here
				Points[sn++] = h;
			
				// Move around
				hl = strlen(h);
				h += hl + 2;
				HackLeft -= hl + 2;
			}
		}
		
		// Convert back to whatever, if anything exists
		if (NextSym)
			*oe = '%';
	}
	
	/* Read Input and place in output buffer */
	i = DS_GetString(a_StrID);
	iold = NULL;
	SpecialPoint = false;
	for (o = a_OutBuf, oe = (o + a_OutSize) - 1; o < oe;)
	{
		// Dollar Special
		if (!SpecialPoint && *i == '$')
		{
			// Obtain special
			SpecialNum = *(++i) - '1';
			i++;	// Done later
			
			// Exists?
			if (SpecialNum >= 0 && SpecialNum <= 9)
				if (Points[SpecialNum])
				{
					iold = i;
					i = Points[SpecialNum];
					SpecialPoint = true;
				}
		}
		
		// Normal
		else
		{
			// End of string?
			if (!SpecialPoint && !*i)
				break;
			
			// End of special point
			if (SpecialPoint)
				if (!*i)
				{
					SpecialPoint = false;
					i = iold;
				}
				
			// Copy Character
			*(o++) = *(i++);
		}
	}
	
	/* Append Newline? */
	if (NewLine)
		if (o < oe)
			*o = '\n';
		else
			a_OutBuf[a_OutSize - 1] = '\n';
	
	/* End Arguments */
	__REMOOD_VA_COPYEND(ArgPtrCopy);
	
	/* Return Written Length */
	return o - a_OutBuf;
#undef BUFSIZE
}


