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
enum UnicodeStringID_e
{
	/* Menus */
	DSTR_MENU_NULLSPACE,
	
	DSTR_MENUNEWGAME_TITLE,
	DSTR_MENUNEWGAME_CLASSIC,
	DSTR_MENUNEWGAME_CREATEGAME,
	DSTR_MENUNEWGAME_UNLISTEDIP,
	
	DSTR_MENUUNLISTED_CONNECT,
	DSTR_MENUUNLISTED_ADDRESS,
	
	DSTR_MENUGAMEVAR_CATNONE,
	
	DSTR_MENUGENERAL_HELLOWORLD,
	DSTR_MENUGENERAL_NEWGAMETYPE,
	DSTR_MENUGENERAL_NEWGAMEISLOCAL,
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
	
	DSTR_MENUQUIT_DISCONNECT,
	DSTR_MENUQUIT_PDISCONNECT,
	DSTR_MENUQUIT_STOPWATCHING,
	DSTR_MENUQUIT_STOPRECORDING,
	DSTR_MENUQUIT_LOGOFF,
	DSTR_MENUQUIT_EXITREMOOD,
	
	DSTR_MENUCREATEGAME_IWADTITLE,
	DSTR_MENUCREATEGAME_STARTGAME,
	
	DSTR_MENUOPTION_MANAGEPROF,
	DSTR_MENUOPTION_CREATEPROF,
	
	DSTR_UIGENERAL_DONTCARE,
	DSTR_UIGENERAL_YES,
	DSTR_UIGENERAL_NO,
	
	/* Intermission Screen */
	
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
	DSTR_CVHINT_SVIRC,
	DSTR_CVHINT_SVMOTD,
	DSTR_CVHINT_SVCONNECTPASSWORD,
	DSTR_CVHINT_SVJOINPASSWORD,
	DSTR_CVHINT_SVMAXCLIENTS,
	DSTR_CVHINT_SVJOINWINDOW,
	DSTR_CVHINT_SVLAGSTAT,
	DSTR_CVHINT_SVREADYBY,
	DSTR_CVHINT_SVMAXCATCHUP,
	DSTR_CVHINT_SVMAXDEMOCATCHUP,
	DSTR_CVHINT_SVLAGTHRESHEXPIRE,
	
	DSTR_CVHINT_CLMAXPTRIES,
	DSTR_CVHINT_CLMAXPTRYTIME,
	DSTR_CVHINT_CLREQTICDELAY,
	
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
	DSTR_CVHINT_INOFAKEDOUBLEBUFFER,
	DSTR_CVHINT_IENABLEJOYSTICK,
	DSTR_CVHINT_IOSSMIDIDEV,
	DSTR_CVHINT_IALSAMIDIDEV,
	DSTR_CVHINT_IWINMIDIDEV,
	
	DSTR_CVHINT_SNDSPEAKERSETUP,
	DSTR_CVHINT_SNDQUALITY,
	DSTR_CVHINT_SNDDENSITY,
	DSTR_CVHINT_SNDBUFFERSIZE,
	DSTR_CVHINT_SNDRANDOMPITCH,
	DSTR_CVHINT_SNDPCSPEAKER,
	DSTR_CVHINT_SNDCHANNELS,
	DSTR_CVHINT_SNDRESERVEDCHANNELS,
	DSTR_CVHINT_SNDSOUNDVOLUME,
	DSTR_CVHINT_SNDMUSICVOLUME,
	
	/* Network Stuff */
	DSTR_NET_LEVELNOTFOUND,
	DSTR_NET_CLIENTCONNECTED,
	DSTR_NET_PORTCONNECTED,
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
	DSTR_DEP_GOTARMOR,
	DSTR_DEP_GOTMEGA,
	DSTR_DEP_GOTHTHBONUS,
	DSTR_DEP_GOTARMBONUS,
	DSTR_DEP_GOTSTIM,
	DSTR_DEP_GOTMEDIKIT,
	DSTR_DEP_GOTSUPER,
	DSTR_DEP_GOTBLUECARD,
	DSTR_DEP_GOTYELWCARD,
	DSTR_DEP_GOTREDCARD,
	DSTR_DEP_GOTBLUESKUL,
	DSTR_DEP_GOTYELWSKUL,
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
	DSTR_DEP_PD_BLUEK,
	DSTR_DEP_PD_REDK,
	DSTR_DEP_PD_YELLOWK,
	DSTR_DEP_QUITMSG,
	DSTR_DEP_QUIT2MSG,
	DSTR_DEP_QUIT2MSG6,
	
	/* Other Game Stuff */
	DSTR_FOUNDSECRET,
	DSTR_NETPLAYERRENAMED,
	DSTR_GAMEPAUSED,
	DSTR_GAMEUNPAUSED,
	
	/* Items */
	DSTR_ITEM_GOTPRESSRELEASEBFG,
	
	/* Demo Stuff */
	DSTR_BADDEMO_LEVELNOTFOUND,
	DSTR_BADDEMO_SAVEGAMENOTSUPPORTED,
	DSTR_BADDEMO_NETVARNOTSUPPORTED,
	DSTR_BADDEMO_UNKNOWNXDCMD,
	DSTR_BADDEMO_UNKNOWNFACTORY,
	DSTR_BADDEMO_NONHOSTDEMO,
	DSTR_BADDEMO_ILLEGALHEADER,
	DSTR_BADDEMO_UNHANDLEDDATA,
	DSTR_BADDEMO_OLDREMOODFORMAT,
	DSTR_BADDEMO_TICDECODEPROBLEM,
	
	/* d_prof.c */
	DSTR_DPROFC_ALREADYEXIST,
	DSTR_DPROFC_FAILEDCREATE,
	DSTR_DPROFC_CONTROLUSAGE,
	DSTR_DPROFC_MAXISUSAGE,
	DSTR_DPROFC_NOTFOUND,
	DSTR_DPROFC_NOTCONTROLNAME,
	DSTR_DPROFC_VALUEUSAGE,
	DSTR_DPROFC_INDEXOUTOFRANGE,
	
	/* d_net.c */
	DSTR_DNETC_CONNECTUSAGE,
	DSTR_DNETC_ADDPLAYERUSAGE,
	DSTR_DNETC_PLAYERLISTENT,
	DSTR_DNETC_BINDFAIL,
	DSTR_DNETC_SERVERCHANGENAME,
	DSTR_DNETC_HOSTNORESOLVE,
	DSTR_DNETC_HOSTRESFOUR,
	DSTR_DNETC_HOSTRESSIX,
	DSTR_DNETC_INVALIDTIC,
	
	/* d_trans.c */
	DSTR_DNETC_PARTIALDISC,
	DSTR_DTRANSC_GETWAD,
	DSTR_DTRANSC_BLACKLIST,
	
	/* d_netdraw.c */
	DSTR_DNETDRAWC_PDWARN,
	
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
	DSTR_DXP_CONNECTING,
	DSTR_DXP_CLCONNECT,
	DSTR_DXP_DISCONNED,
	DSTR_DXP_WADHEADER,
	DSTR_DXP_WADENTRY,
	DSTR_DXP_BADWADEXT,
	DSTR_DXP_CLIENTREADYWAIT,
	DSTR_DXP_WAITINGFORCONN,
	DSTR_DXP_SENDFILE,
	DSTR_DXP_RECVFILE,
	DSTR_DXP_BADSAVELOAD,
	DSTR_DXP_PLAYERISPLAYING,
	
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
	DSTR_PSPECC_PLDEFECTED,
	
	/* p_inter.c */
	DSTR_PINTERC_FRAGLIMITREACHED,
	
	/* p_saveg.c */
	DSTR_PSAVEGC_ENDOFSTREAM,
	DSTR_PSAVEGC_WRONGHEADER,
	DSTR_PSAVEGC_UNKNOWNLEVEL,
	DSTR_PSAVEGC_LEVELLOADFAIL,
	DSTR_PSAVEGC_ILLEGALTHINKER,
	DSTR_PSAVEGC_UNHANDLEDTHINKER,
	DSTR_PSAVEGC_ILLEGALHOST,
	
	/* i_utlnet.c */
	DSTR_IUTLNET_BADUNIXBIND,
	
	/* g_game.c */
	DSTR_GGAMEC_CHATALL,
	DSTR_GGAMEC_CHATTEAM,
	DSTR_GGAMEC_CHATSPEC,
	DSTR_GGAMEC_CHATINDIV,
	DSTR_GGAMEC_CHATPRIV,
	
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
	DSTR_M_PGS_CORANDOMLASTLOOKSPAWN,
	DSTR_D_PGS_CORANDOMLASTLOOKSPAWN,
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
	DSTR_M_PGS_COVARIABLEFRICTION,
	DSTR_D_PGS_COVARIABLEFRICTION,
	DSTR_M_PGS_COALLOWPUSHERS,
	DSTR_D_PGS_COALLOWPUSHERS,
	DSTR_M_PGS_PLMAXTEAMS,
	DSTR_D_PGS_PLMAXTEAMS,
	DSTR_M_PGS_PLMAXPLAYERS,
	DSTR_D_PGS_PLMAXPLAYERS,
	
	NUMUNICODESTRINGS
};

/* Define UnicodeStringID_t */
#if !defined(__REMOOD_UNICSTRID_DEFINED)
	typedef int UnicodeStringID_t;
	#define __REMOOD_UNICSTRID_DEFINED
#endif

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

