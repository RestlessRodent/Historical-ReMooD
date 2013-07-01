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
	{"MENU_NULLSPACE", ""},
	
	{"MENUNEWGAME_TITLE", "New Game"},
	{"MENUNEWGAME_CLASSIC", "Classic..."},
	{"MENUNEWGAME_CREATEGAME", "Create Game..."},
	{			"MENUNEWGAME_UNLISTEDIP", "Connect to Unlisted Server..."},
	
	{			  "MENUUNLISTED_CONNECT", "Connect!"},
	{			  "MENUUNLISTED_ADDRESS", "Remote Address:"},
	
	{			   "MENUGAMEVAR_CATNONE", ""},
	
	{			"MENUGENERAL_HELLOWORLD", "Hello World!"},
	{		   "MENUGENERAL_NEWGAMETYPE", "Connection Type"},
	{		"MENUGENERAL_NEWGAMEISLOCAL", "Local System"},
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
	
	{				"MENUQUIT_DISCONNECT", "Disconnect"},
	{			  "MENUQUIT_STOPWATCHING", "Stop Watching Demo"},
	{			 "MENUQUIT_STOPRECORDING", "Stop Recording Demo"},
	{					"MENUQUIT_LOGOFF", "Log Off"},
	{				"MENUQUIT_EXITREMOOD", "Exit ReMooD"},
	
	{		   "MENUCREATEGAME_IWADTITLE", "IWAD"},
	{		   "MENUCREATEGAME_STARTGAME", "Start Game!"},
	
	{			  "MENUOPTION_MANAGEPROF", "Manage Profiles"},
	{			  "MENUOPTION_CREATEPROF", "Create Profile..."},
	
	{				"UIGENERAL_DONTCARE", "Don't Care!"},
	{					 "UIGENERAL_YES", "Yes"},
	{					  "UIGENERAL_NO", "No"},
	
	/*** INTERMISSION ***/
	
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
	{					  "CVHINT_SVIRC", "The IRC Channel for this server."},
	{					 "CVHINT_SVMOTD", "The message displayed to clients when joining."},
	{		  "CVHINT_SVCONNECTPASSWORD", "The password needed to connect to a server."},
	{			 "CVHINT_SVJOINPASSWORD", "The password needed to join the game."},
	{			   "CVHINT_SVMAXCLIENTS", "The maximum amount of clients that may join the game."},
	{			   "CVHINT_SVJOINWINDOW", "Amount of time in seconds between each join window interval."},
	{				  "CVHINT_SVLAGSTAT", "Amount of time in minutes between each lag check for a client."},
	{				  "CVHINT_SVREADYBY", "Amount of time in seconds before a client is disconnected for lagging out."},
	{			   "CVHINT_SVMAXCATCHUP", "Maximum amount of time to catchup when playing as the server, after this threshold catchup stops and waiting for players appears."},
	{		   "CVHINT_SVMAXDEMOCATCHUP", "Maximum amount of time to catchup when playing a demo, after this threshold catchup stops."},
	{		  "CVHINT_SVLAGTHRESHEXPIRE", "Time in seconds before the lag threshold expires."},
	
	{				"CVHINT_CLMAXPTRIES", "Maximum amount of client attempts to obtain a client from the server."},
	{			  "CVHINT_CLMAXPTRYTIME", "Time to wait between client requests."},
	{			  "CVHINT_CLREQTICDELAY", "Amount in time to tics to wait when requesting more tics if none are available."},
	
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
	{		"CVHINT_INOFAKEDOUBLEBUFFER", "Do not fake double buffering (will cause tearing if your hardware is not capable of double buffering)."},
	{			"CVHINT_IENABLEJOYSTICK", "Enable joystick input."},
	{				"CVHINT_IOSSMIDIDEV", "OSS device to use when playing MIDI music. (Pathname to MIDI device such as /dev/midi)"},
	{			   "CVHINT_IALSAMIDIDEV", "ALSA device to use when playing MIDI music. (Client:Port to MIDI device such as 128:0)"},
	{				"CVHINT_IWINMIDIDEV", "Win32 device to use when playing MIDI music. (Name or number of device such as 0 or \"midimapper\")"},
	
	{			"CVHINT_SNDSPEAKERSETUP", "Number of speakers to use."},
	{				 "CVHINT_SNDQUALITY", "Quality of sound to be outputted."},
	{				 "CVHINT_SNDDENSITY", "Density of the sound to use, the higher the better."},
	{			  "CVHINT_SNDBUFFERSIZE", "Size of the sound buffer in size."},
	{			 "CVHINT_SNDRANDOMPITCH", "Randomly Pitch Sounds."},
	{			   "CVHINT_SNDPCSPEAKER", "Simulate Doom PC Speaker Effects."},
	{				"CVHINT_SNDCHANNELS", "Number of sound channels to play."},
	{		"CVHINT_SNDRESERVEDCHANNELS", "Number of channels to keep in reserve."},
	{			 "CVHINT_SNDSOUNDVOLUME", "The volume at which sounds play at."},
	{			 "CVHINT_SNDMUSICVOLUME", "The volume at which music plays at."},
	
	/*** NETWORK STUFF ***/
	{				 "NET_LEVELNOTFOUND", "Level not found."},
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
	{                              "DEP_GOTARMOR", "Picked up the armor."},
	{                               "DEP_GOTMEGA", "Picked up the MegaArmor!"},
	{                           "DEP_GOTHTHBONUS", "Picked up a health bonus."},
	{                           "DEP_GOTARMBONUS", "Picked up an armor bonus."},
	{                               "DEP_GOTSTIM", "Picked up a stimpack."},
	{                            "DEP_GOTMEDIKIT", "Picked up a medikit."},
	{                              "DEP_GOTSUPER", "Supercharge!"},
	{                           "DEP_GOTBLUECARD", "Picked up a blue keycard."},
	{                           "DEP_GOTYELWCARD", "Picked up a yellow keycard."},
	{                            "DEP_GOTREDCARD", "Picked up a red keycard."},
	{                           "DEP_GOTBLUESKUL", "Picked up a blue skull key."},
	{                           "DEP_GOTYELWSKUL", "Picked up a yellow skull key."},
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
	{                              "DEP_PD_BLUEK", "You need a blue key to open this door"},
	{                               "DEP_PD_REDK", "You need a red key to open this door"},
	{                            "DEP_PD_YELLOWK", "You need a yellow key to open this door"},
	{                               "DEP_QUITMSG", "are you sure you want to\nquit this great game?"},
	{                              "DEP_QUIT2MSG", "you want to quit?\nthen, thou hast lost an eighth!"},
	{                             "DEP_QUIT2MSG6", "you're lucky i don't smack\nyou for thinking about leaving."},
	
	/*** OTHER GAME STUFF ***/
	{					"FOUNDSECRET", "You found a secret area!"},
	{              "NETPLAYERRENAMED", "$1 renamed to $2."},
	{					 "GAMEPAUSED", "$1 paused the game."},
	{				   "GAMEUNPAUSED", "$1 resumed the game."},
	
	/*** ITEMS ***/
	{		"ITEM_GOTPRESSRELEASEBFG", "You got the press release BFG9000!"},
	
	/*** DEMO STUFF ***/
	{		  "BADDEMO_LEVELNOTFOUND", "The level named \"$1\" was not found."},
	{  "BADDEMO_SAVEGAMENOTSUPPORTED", "Saving games is not supported."},
	{	 "BADDEMO_NETVARNOTSUPPORTED", "The variable $1 is not supported."},
	{		   "BADDEMO_UNKNOWNXDCMD", "Unknown extra command $1."},
	{        "BADDEMO_UNKNOWNFACTORY", "Unknown demo factory."},
	{           "BADDEMO_NONHOSTDEMO", "Demo possibly not recorded by the host, if so then the demo will most likely fail to play back at all."},
	{		  "BADDEMO_ILLEGALHEADER", "Demo contains an illegal header."},
	{		  "BADDEMO_UNHANDLEDDATA", "Unhandled data labeled \"$1\"."},
	{		"BADDEMO_OLDREMOODFORMAT", "Ancient ReMooD Demos not supported!"},
	{	   "BADDEMO_TICDECODEPROBLEM", "Tic decoding error."},
	
	/*** D_PROF.C ***/
	{			"DPROFC_ALREADYEXIST", "Profile \"$1\" already exists!"},
	{			"DPROFC_FAILEDCREATE", "Failed to create profile."},
	{			"DPROFC_CONTROLUSAGE", "Usage: $1 control <Name> <Action> <Id> <Key>"},
	{			  "DPROFC_MAXISUSAGE", "Usage: $1 maxis <Name> <Group> <Axis> <Action>"},
	{				"DPROFC_NOTFOUND", "Profile $1 not found."},
	{		  "DPROFC_NOTCONTROLNAME", "$1 is not a valid control."},
	{			  "DPROFC_VALUEUSAGE", "Usage: $1 value <Option> <Value>"},
	{		 "DPROFC_INDEXOUTOFRANGE", "Index $1 out of range."},
	
	/*** D_NET.C ***/
	{			 "DNETC_CONNECTUSAGE", "Usage: $1 <hostname> (password)"},
	{		   "DNETC_ADDPLAYERUSAGE", "Usage: $1 <profile> (screen) (joystick id)"},
	{			"DNETC_PLAYERLISTENT", "$1 $2 $3{z"},
	{				 "DNETC_BINDFAIL", "Failed to bind to \"$1\"."},
	{		 "DNETC_SERVERCHANGENAME", "Server changed name to \"$1\"."},
	{			"DNETC_HOSTNORESOLVE", "Host \"$1\" does not resolve."},
	{			  "DNETC_HOSTRESFOUR", "Host \"$1\" resolves to $2.$3.$4.$5 (v4)."},
	{			   "DNETC_HOSTRESSIX", "Host \"$1\" resolves to $2$3:$4:$5:$6:$7%$8 (v6)."},
	{			   "DNETC_INVALIDTIC", "Attempted to gather tic $1 but it was not found."},
	
	/*** D_TRANS.C ***/
	{			  "DNETC_PARTIALDISC", "Partial Disconnect ($1{z)."},
	{				 "DTRANSC_GETWAD", "Will download WAD \"$1\"."},
	{			  "DTRANSC_BLACKLIST", "WAD \"$1\" is on blacklist, not downloading.\n"},
	
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
	{			     "DXP_CONNECTING", "{zConnecting to {9$1{z..."},
	{				  "DXP_CLCONNECT", "{z{9$1{z has connected!"},
	{				  "DXP_DISCONNED", "{zDisconnected, reason: \"{9$1\"."},
	{				  "DXP_WADHEADER", "{zWADs used by server:"},
	{				   "DXP_WADENTRY", "{z [{4$3{z] {9$1 {z({3$2{z)"},
	{				  "DXP_BADWADEXT", "{zWAD {9$1{z/{3$2{z has an invalid extension, due to security measures."},
	{			"DXP_CLIENTREADYWAIT", "{z{9$1{z is waiting for next join window."},
	{			 "DXP_WAITINGFORCONN", "{zWaiting for connection..."},
	{				   "DXP_SENDFILE", "{zSending file \"{3$1{z\" to {9$2{z."},
	{				   "DXP_RECVFILE", "{zReceiving file \"{3$1{z\" from {9$2{z."},
	{				"DXP_BADSAVELOAD", "{zFailed to load savegame \"{3$1{z\"."},
	{			"DXP_PLAYERISPLAYING", "{9$1{z ({9$2{z) is now playing."},
	
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
	{		"PSPECC_PLDEFECTED", "{z$1{z changed to team $2"},
	
	/*** P_INTER.c ***/
	{	   "PINTERC_FRAGLIMITREACHED", "Frag limit reached."},
	
	/*** P_SAVEG.C ***/
	{			"PSAVEGC_ENDOFSTREAM", "End of stream reached too early."},
	{			"PSAVEGC_WRONGHEADER", "Incorrect header."},
	{		   "PSAVEGC_UNKNOWNLEVEL", "The level the game was saved on is not currently known."},
	{		  "PSAVEGC_LEVELLOADFAIL", "The level failed to load correctly."},
	{		 "PSAVEGC_ILLEGALTHINKER", "An illegal thinker exists."},
	{	   "PSAVEGC_UNHANDLEDTHINKER", "A valid thinker is not currently handled."},
	
	/*** I_UTLNET.C ***/
	{			"IUTLNET_BADUNIXBIND", "Socket bind failed: $2 ($1)"},
	
	/*** G_GAME.C ***/
	{				 "GGAMEC_CHATALL", "$1{1<{z$2{1> {z"},
	{				"GGAMEC_CHATTEAM", "$1{x7$3<{z$2{z{x7$3 to team> {z"},
	{				"GGAMEC_CHATSPEC", "$1{a<{z$2{z{a to spec> {z"},
	{			   "GGAMEC_CHATINDIV", "$1{3<{z$2{z{3 to $3{z{3> {z"},
	
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
	{			 "M_PGS_CONEWGAMEMODES", "Enable New Game Modes"},
	{			 "D_PGS_CONEWGAMEMODES", "Enables the usage of the new game mode settings."},
	{				   "M_PGS_GAMEMODE", "Game Mode"},
	{			       "D_PGS_GAMEMODE", "The type of game currently being played."},
	{		  "M_PGS_CTFNEEDFLAGATHOME", "Flag At Home To Score"},
	{		  "D_PGS_CTFNEEDFLAGATHOME", "To score a flag, your team flag must be at home."},
	{		 "M_PGS_COVARIABLEFRICTION", "Boom Variable Friction"},
	{		 "D_PGS_COVARIABLEFRICTION", "Friction is able to be varied."},
	{			 "M_PGS_COALLOWPUSHERS", "Allow Boom Pushers"},
	{			 "D_PGS_COALLOWPUSHERS", "Enables push/puller objects to work."},
	{			 	 "M_PGS_PLMAXTEAMS", "Max Teams"},
	{			 	 "D_PGS_PLMAXTEAMS", "Maximum amount of teams that can exist at once."},
	{			   "M_PGS_PLMAXPLAYERS", "Max Players"},
	{			   "D_PGS_PLMAXPLAYERS", "Maximum amount of players that can be playing at one time."},
	
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


