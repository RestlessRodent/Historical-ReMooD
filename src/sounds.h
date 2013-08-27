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
// DESCRIPTION: Created by the sound utility written by Dave Taylor.
//              Kept as a sample, DOOM2  sounds. Frozen.

#ifndef __SOUNDS__
#define __SOUNDS__

/* Define WadIndex_t */
#if !defined(__REMOOD_WADINDEXT_DEFINED)
	typedef int WadIndex_t;
	#define __REMOOD_WADINDEXT_DEFINED
#endif



// 10 customisable sounds for Skins
typedef enum
{
	SKSPLPAIN,
	SKSSLOP,
	SKSOOF,
	SKSPLDETH,
	SKSPDIEHI,
	SKSNOWAY,
	SKSPUNCH,
	SKSRADIO,
	SKSJUMP,
	SKSOUCH,
	NUMSKINSOUNDS
} skinsound_t;

// free sfx for S_AddSoundFx()
//MAXSKINS
#define NUMSFXFREESLOTS    (32*NUMSKINSOUNDS)
#define NUMMUSICFREESLOTS  64

//
// SoundFX struct.
//
typedef struct sfxinfo_struct sfxinfo_t;

struct sfxinfo_struct
{
	// up to 6-character name
	char* name;
	
	// Sfx singularity (only one at a time)
	int singularity;
	
	// Sfx priority
	int priority;
	
	// referenced sound if a link
	sfxinfo_t* link;
	
	// pitch if a link
	int pitch;
	
	// volume if a link
	int volume;
	
	// sound data
	void* data;
	
	// sound that can be remapped for a skin, indexes skins[].skinsounds
	// 0 up to (NUMSKINSOUNDS-1), -1 = not skin specifc
	int skinsound;
	
	// this is checked every second to see if sound
	// can be thrown out (if 0, then decrement, if -1,
	// then throw out, if > 0, then it is in use)
	int usefulness;
	
	// lump number of sfx
	int lumpnum;
	
};

//
// MusicInfo struct.
//
typedef struct
{
	// up to 6-character name
	char* name;
	
	// lump number of music
	WadIndex_t lumpnum;
	
	// music data
	void* data;
	
	// music handle once registered
	int handle;
	
} musicinfo_t;

// the complete set of sound effects
extern sfxinfo_t S_sfx[];

// the complete set of music
extern musicinfo_t S_music[];

//
// Identifiers for all music in game.
//

typedef enum
{
	mus_None,
	mus_e1m1,
	mus_e1m2,
	mus_e1m3,
	mus_e1m4,
	mus_e1m5,
	mus_e1m6,
	mus_e1m7,
	mus_e1m8,
	mus_e1m9,
	mus_e2m1,
	mus_e2m2,
	mus_e2m3,
	mus_e2m4,
	mus_e2m5,
	mus_e2m6,
	mus_e2m7,
	mus_e2m8,
	mus_e2m9,
	mus_e3m1,
	mus_e3m2,
	mus_e3m3,
	mus_e3m4,
	mus_e3m5,
	mus_e3m6,
	mus_e3m7,
	mus_e3m8,
	mus_e3m9,
	mus_inter,
	mus_intro,
	mus_bunny,
	mus_victor,
	mus_introa,
	mus_runnin,
	mus_stalks,
	mus_countd,
	mus_betwee,
	mus_doom,
	mus_the_da,
	mus_shawn,
	mus_ddtblu,
	mus_in_cit,
	mus_dead,
	mus_stlks2,
	mus_theda2,
	mus_doom2,
	mus_ddtbl2,
	mus_runni2,
	mus_dead2,
	mus_stlks3,
	mus_romero,
	mus_shawn2,
	mus_messag,
	mus_count2,
	mus_ddtbl3,
	mus_ampie,
	mus_theda3,
	mus_adrian,
	mus_messg2,
	mus_romer2,
	mus_tense,
	mus_shawn3,
	mus_openin,
	mus_evil,
	mus_ultima,
	mus_read_m,
	mus_dm2ttl,
	mus_dm2int,
	
	mus_waitin,
	
	mus_firstfreeslot,
	// 64 free slots here
	mus_lastfreeslot = mus_firstfreeslot + NUMMUSICFREESLOTS - 1,
	NUMMUSIC
} musicenum_t;

//
// Identifiers for all sfx in game.
//

typedef enum
{
	sfx_None,
	sfx_pistol,
	sfx_shotgn,
	sfx_sgcock,
	sfx_dshtgn,
	sfx_dbopn,
	sfx_dbcls,
	sfx_dbload,
	sfx_plasma,
	sfx_bfg,
	sfx_sawup,
	sfx_sawidl,
	sfx_sawful,
	sfx_sawhit,
	sfx_rlaunc,
	sfx_rxplod,
	sfx_firsht,
	sfx_firxpl,
	sfx_pstart,
	sfx_pstop,
	sfx_doropn,
	sfx_dorcls,
	sfx_stnmov,
	sfx_swtchn,
	sfx_swtchx,
	sfx_plpain,
	sfx_dmpain,
	sfx_popain,
	sfx_vipain,
	sfx_mnpain,
	sfx_pepain,
	sfx_slop,
	sfx_itemup,
	sfx_wpnup,
	sfx_oof,
	sfx_telept,
	sfx_posit1,
	sfx_posit2,
	sfx_posit3,
	sfx_bgsit1,
	sfx_bgsit2,
	sfx_sgtsit,
	sfx_cacsit,
	sfx_brssit,
	sfx_cybsit,
	sfx_spisit,
	sfx_bspsit,
	sfx_kntsit,
	sfx_vilsit,
	sfx_mansit,
	sfx_pesit,
	sfx_sklatk,
	sfx_sgtatk,
	sfx_skepch,
	sfx_vilatk,
	sfx_claw,
	sfx_skeswg,
	sfx_pldeth,
	sfx_pdiehi,
	sfx_podth1,
	sfx_podth2,
	sfx_podth3,
	sfx_bgdth1,
	sfx_bgdth2,
	sfx_sgtdth,
	sfx_cacdth,
	sfx_skldth,
	sfx_brsdth,
	sfx_cybdth,
	sfx_spidth,
	sfx_bspdth,
	sfx_vildth,
	sfx_kntdth,
	sfx_pedth,
	sfx_skedth,
	sfx_posact,
	sfx_bgact,
	sfx_dmact,
	sfx_bspact,
	sfx_bspwlk,
	sfx_vilact,
	sfx_noway,
	sfx_barexp,
	sfx_punch,
	sfx_hoof,
	sfx_metal,
	sfx_chgun,
	sfx_tink,
	sfx_bdopn,
	sfx_bdcls,
	sfx_itmbk,
	sfx_flame,
	sfx_flamst,
	sfx_getpow,
	sfx_bospit,
	sfx_boscub,
	sfx_bossit,
	sfx_bospn,
	sfx_bosdth,
	sfx_manatk,
	sfx_mandth,
	sfx_sssit,
	sfx_ssdth,
	sfx_keenpn,
	sfx_keendt,
	sfx_skeact,
	sfx_skesit,
	sfx_skeatk,
	sfx_radio,
	//added:22-02-98: player avatar jumps
	sfx_jump,
	//added:22-02-98: player hits something hard and says 'ouch!'
	sfx_ouch,
	//test water
	sfx_gloop,
	sfx_splash,
	sfx_floush,
	
	// killough 11/98: dog sounds
	sfx_dgsit,
	sfx_dgatk,
	sfx_dgact,
	sfx_dgdth,
	sfx_dgpain,
	
	// GhostlyDeath -- ReMooD Sounds
	sfx_zzzzza,
	sfx_oskmov,
	sfx_osktyp,
	sfx_lotime,
	sfx_secret,
	sfx_elvcal,
	sfx_gerror,
	sfx_zzsqwv,
	sfx_dialup,
	
	// GhostlyDeath <August 31, 2012> -- Heretic Sounds
	sfx_gldhit,
	sfx_gntful,
	sfx_gnthit,
	sfx_gntpow,
    sfx_gntact,
	sfx_gntuse,
	sfx_phosht,
	sfx_phohit,
	sfx_phopow,
	sfx_lobsht,
	sfx_lobhit,
	sfx_lobpow,
	sfx_hrnsht,
	sfx_hrnhit,
	sfx_hrnpow,
	sfx_ramphit,
	sfx_ramrain,
	sfx_bowsht,
	sfx_stfhit,
	sfx_stfpow,
	sfx_stfcrk,
	sfx_impsit,
	sfx_impat1,
	sfx_impat2,
	sfx_impdth,
	sfx_impact,
	sfx_imppai,
	sfx_mumsit,
	sfx_mumat1,
	sfx_mumat2,
	sfx_mumdth,
	sfx_mumact,
	sfx_mumpai,
	sfx_mumhed,
	sfx_bstsit,
	sfx_bstatk,
	sfx_bstdth,
	sfx_bstact,
	sfx_bstpai,
	sfx_clksit,
	sfx_clkatk,
	sfx_clkdth,
	sfx_clkact,
	sfx_clkpai,
	sfx_snksit,
	sfx_snkatk,
	sfx_snkdth,
	sfx_snkact,
	sfx_snkpai,
	sfx_kgtsit,
	sfx_kgtatk,
	sfx_kgtat2,
	sfx_kgtdth,
	sfx_kgtact,
	sfx_kgtpai,
	sfx_wizsit,
	sfx_wizatk,
	sfx_wizdth,
	sfx_wizact,
	sfx_wizpai,
	sfx_minsit,
	sfx_minat1,
	sfx_minat2,
	sfx_minat3,
	sfx_mindth,
	sfx_minact,
	sfx_minpai,
	sfx_hedsit,
	sfx_hedat1,
	sfx_hedat2,
	sfx_hedat3,
	sfx_heddth,
	sfx_hedact,
	sfx_hedpai,
	sfx_sorzap,
	sfx_sorrise,
	sfx_sorsit,
	sfx_soratk,
	sfx_soract,
	sfx_sorpai,
	sfx_sordsph,
	sfx_sordexp,
	sfx_sordbon,
	sfx_sbtsit,
	sfx_sbtatk,
	sfx_sbtdth,
	sfx_sbtact,
	sfx_sbtpai,
	sfx_hplroof,
	sfx_plrpai,
	sfx_plrdth,					// Normal
	sfx_gibdth,					// Extreme
	sfx_plrwdth,				// Wimpy
	sfx_plrcdth,				// Crazy
	sfx_hitemup,
	sfx_hwpnup,
	sfx_htelept,
	sfx_hdoropn,
	sfx_hdorcls,
	sfx_dormov,
	sfx_artiup,
    sfx_switch,
	sfx_hpstart,
	sfx_hpstop,
	sfx_hstnmov,
	sfx_chicpai,
	sfx_chicatk,
	sfx_chicdth,
	sfx_chicact,
	sfx_chicpk1,
	sfx_chicpk2,
	sfx_chicpk3,
	sfx_keyup,
	sfx_ripslop,
	sfx_newpod,
	sfx_podexp,
	sfx_bounce,
	sfx_volsht,
	sfx_volhit,
	sfx_burn,
	sfx_hsplash,
	sfx_hgloop,
	sfx_hrespawn,
	sfx_blssht,
	sfx_blshit,
	sfx_hchat,
	sfx_artiuse,
	sfx_gfrag,
	sfx_waterfl,
	sfx_wind,
	sfx_amb1,
	sfx_amb2,
	sfx_amb3,
	sfx_amb4,
	sfx_amb5,
	sfx_amb6,
	sfx_amb7,
	sfx_amb8,
	sfx_amb9,
	sfx_amb10,
	sfx_amb11,
	
	// GhostlyDeath <September 12, 2012> -- Generic Sounds
	sfx_generic_switchon,
	sfx_generic_switchoff,
	sfx_generic_menupress,
	sfx_generic_menumove,
	sfx_generic_menuslide,
	sfx_generic_menufail,
	sfx_generic_chat,
	
	// free slots for S_AddSoundFx() at run-time --------------------
	sfx_freeslot0,
	//
	// ... 60 free sounds here ...
	//
	sfx_lastfreeslot = (sfx_freeslot0 + NUMSFXFREESLOTS - 1),
	// end of freeslots ---------------------------------------------
	
	NUMSFX
} sfxenum_t;

void S_InitRuntimeSounds(void);
int S_AddSoundFx(char* name, int singularity);
void S_RemoveSoundFx(int id);

#endif

