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
// DESCRIPTION: Printed strings for translation.
//              English language support (default).

#ifndef __D_TEXT__
#define __D_TEXT__

typedef enum
{
	D_DEVSTR_NUM,				//Development mode ON.
	D_CDROM_NUM,				//CD-ROM Version: default.cfg from c:\\doomdata
	PRESSKEY_NUM,				//press a key.
	PRESSYN_NUM,				//press y or n.
	LOADNET_NUM,				//only the server can do a load net game! press a key.
	QLOADNET_NUM,				//you can't quickload during a netgame! press a key.
	QSAVESPOT_NUM,				//you haven't picked a quicksave slot yet! press a key.
	SAVEDEAD_NUM,				//you can't save if you aren't playing! press a key.
	QSPROMPT_NUM,				//quicksave over your game named ""? press y or n.
	QLPROMPT_NUM,				//do you want to quickload the game named ""? press y or n.
	NEWGAME_NUM,				//you can't start a new game while in a network game.
	NIGHTMARE_NUM,				//are you sure? this skill level isn't even remotely fair. press y or n.
	SWSTRING_NUM,				//this is the shareware version of doom. you need to order the entire trilogy. press a key.
	MSGOFF_NUM,					//Messages OFF
	MSGON_NUM,					//Messages ON
	NETEND_NUM,					//you can't end a netgame! press a key.
	ENDGAME_NUM,				//are you sure you want to end the game? press y or n.
	NOENDGAME_NUM,				//you can't end the game if you aren't even playing. press a key.
	DOSY_NUM,					//(press y to quit to dos.)
	DETAILHI_NUM,				//High Detail
	DETAILLO_NUM,				//Low Detail
	GAMMALVL0_NUM,				//Gamma correction OFF
	GAMMALVL1_NUM,				//Gamma correction level 1
	GAMMALVL2_NUM,				//Gamma correction level 2
	GAMMALVL3_NUM,				//Gamma correction level 3
	GAMMALVL4_NUM,				//Gamma correction level 4
	EMPTYSTRING_NUM,			//empty slot

	/* Powerups/armor/health/keys */
	GOTARMOR_NUM,				//Picked up the armor.
	GOTMEGA_NUM,				//Picked up the MegaArmor!
	GOTHTHBONUS_NUM,			//Picked up a health bonus.
	GOTARMBONUS_NUM,			//Picked up a armor bonus.
	GOTSTIM_NUM,				//Picked up a stimpack.
	GOTMEDINEED_NUM,			//Picked up a medikit that you REALLY need!
	GOTMEDIKIT_NUM,				//Picked up a medikit.
	GOTSUPER_NUM,				//Supercharge!
	GOTBLUECARD_NUM,			//Picked up a blue keycard.
	GOTYELWCARD_NUM,			//Picked up a yellow keycard.
	GOTREDCARD_NUM,				//Picked up a red keycard.
	GOTBLUESKUL_NUM,			//Picked up a blue skull key.
	GOTYELWSKUL_NUM,			//Picked up a yellow skull key.
	GOTREDSKULL_NUM,			//Picked up a red skull key.
	GOTINVUL_NUM,				//Invulnerability!
	GOTBERSERK_NUM,				//Berserk!
	GOTINVIS_NUM,				//Partial Invisibility
	GOTSUIT_NUM,				//Radiation Shielding Suit
	GOTMAP_NUM,					//Computer Area Map
	GOTVISOR_NUM,				//Light Amplification Visor
	GOTMSPHERE_NUM,				//MegaSphere!

	/* Ammo */
	GOTCLIP_NUM,				//Picked up a clip.
	GOTCLIPBOX_NUM,				//Picked up a box of bullets.
	GOTROCKET_NUM,				//Picked up a rocket.
	GOTROCKBOX_NUM,				//Picked up a box of rockets.
	GOTCELL_NUM,				//Picked up an energy cell.
	GOTCELLBOX_NUM,				//Picked up an energy cell pack.
	GOTSHELLS_NUM,				//Picked up 4 shotgun shells.
	GOTSHELLBOX_NUM,			//Picked up a box of shotgun shells.
	GOTBACKPACK_NUM,			//Picked up a backpack full of ammo!

	/* Weapons */
	GOTBFG9000_NUM,				//You got the BFG9000!  Oh, yes.
	GOTCHAINGUN_NUM,			//You got the chaingun!
	GOTCHAINSAW_NUM,			//A chainsaw!  Find some meat!
	GOTLAUNCHER_NUM,			//You got the rocket launcher!
	GOTPLASMA_NUM,				//You got the plasma gun!
	GOTSHOTGUN_NUM,				//You got the shotgun!
	GOTSHOTGUN2_NUM,			//You got the super shotgun!

	/* Misc */
	PD_BLUEO_NUM,				//You need a blue key to activate this object
	PD_REDO_NUM,				//You need a red key to activate this object
	PD_YELLOWO_NUM,				//You need a yellow key to activate this object
	PD_BLUEK_NUM,				//You need a blue key to open this door
	PD_REDK_NUM,				//You need a red key to open this door
	PD_YELLOWK_NUM,				//You need a yellow key to open this door
	GGSAVED_NUM,				//game saved.
	HUSTR_MSGU_NUM,				//[Message unsent]

	/* Ultimate Doom Maps */
	HUSTR_E1M1_NUM,				//E1M1: Hangar
	HUSTR_E1M2_NUM,				//E1M2: Nuclear Plant
	HUSTR_E1M3_NUM,				//E1M3: Toxin Refinery
	HUSTR_E1M4_NUM,				//E1M4: Command Control
	HUSTR_E1M5_NUM,				//E1M5: Phobos Lab
	HUSTR_E1M6_NUM,				//E1M6: Central Processing
	HUSTR_E1M7_NUM,				//E1M7: Computer Station
	HUSTR_E1M8_NUM,				//E1M8: Phobos Anomaly
	HUSTR_E1M9_NUM,				//E1M9: Military Base
	HUSTR_E2M1_NUM,				//E2M1: Deimos Anomaly
	HUSTR_E2M2_NUM,				//E2M2: Containment Area
	HUSTR_E2M3_NUM,				//E2M3: Refinery
	HUSTR_E2M4_NUM,				//E2M4: Deimos Lab
	HUSTR_E2M5_NUM,				//E2M5: Command Center
	HUSTR_E2M6_NUM,				//E2M6: Halls of the Damned
	HUSTR_E2M7_NUM,				//E2M7: Spawning Vats
	HUSTR_E2M8_NUM,				//E2M8: Tower of Babel
	HUSTR_E2M9_NUM,				//E2M9: Fortress of Mystery
	HUSTR_E3M1_NUM,				//E3M1: Hell Keep
	HUSTR_E3M2_NUM,				//E3M2: Slough of Despair
	HUSTR_E3M3_NUM,				//E3M3: Pandemonium
	HUSTR_E3M4_NUM,				//E3M4: House of Pain
	HUSTR_E3M5_NUM,				//E3M5: Unholy Cathedral
	HUSTR_E3M6_NUM,				//E3M6: Mt. Erebus
	HUSTR_E3M7_NUM,				//E3M7: Limbo
	HUSTR_E3M8_NUM,				//E3M8: Dis
	HUSTR_E3M9_NUM,				//E3M9: Warrens
	HUSTR_E4M1_NUM,				//E4M1: Hell Beneath
	HUSTR_E4M2_NUM,				//E4M2: Perfect Hatred
	HUSTR_E4M3_NUM,				//E4M3: Sever The Wicked
	HUSTR_E4M4_NUM,				//E4M4: Unruly Evil
	HUSTR_E4M5_NUM,				//E4M5: They Will Repent
	HUSTR_E4M6_NUM,				//E4M6: Against Thee Wickedly
	HUSTR_E4M7_NUM,				//E4M7: And Hell Followed
	HUSTR_E4M8_NUM,				//E4M8: Unto The Cruel
	HUSTR_E4M9_NUM,				//E4M9: Fear

	/* Doom2 Maps */
	HUSTR_1_NUM,				//level 1: entryway
	HUSTR_2_NUM,				//level 2: underhalls
	HUSTR_3_NUM,				//level 3: the gantlet
	HUSTR_4_NUM,				//level 4: the focus
	HUSTR_5_NUM,				//level 5: the waste tunnels
	HUSTR_6_NUM,				//level 6: the crusher
	HUSTR_7_NUM,				//level 7: dead simple
	HUSTR_8_NUM,				//level 8: tricks and traps
	HUSTR_9_NUM,				//level 9: the pit
	HUSTR_10_NUM,				//level 10: refueling base
	HUSTR_11_NUM,				//level 11: 'o' of destruction!
	HUSTR_12_NUM,				//level 12: the factory
	HUSTR_13_NUM,				//level 13: downtown
	HUSTR_14_NUM,				//level 14: the inmost dens
	HUSTR_15_NUM,				//level 15: industrial zone
	HUSTR_16_NUM,				//level 16: suburbs
	HUSTR_17_NUM,				//level 17: tenements
	HUSTR_18_NUM,				//level 18: the courtyard
	HUSTR_19_NUM,				//level 19: the citadel
	HUSTR_20_NUM,				//level 20: gotcha!
	HUSTR_21_NUM,				//level 21: nirvana
	HUSTR_22_NUM,				//level 22: the catacombs
	HUSTR_23_NUM,				//level 23: barrels o' fun
	HUSTR_24_NUM,				//level 24: the chasm
	HUSTR_25_NUM,				//level 25: bloodfalls
	HUSTR_26_NUM,				//level 26: the abandoned mines
	HUSTR_27_NUM,				//level 27: monster condo
	HUSTR_28_NUM,				//level 28: the spirit world
	HUSTR_29_NUM,				//level 29: the living end
	HUSTR_30_NUM,				//level 30: icon of sin
	HUSTR_31_NUM,				//level 31: wolfenstein
	HUSTR_32_NUM,				//level 32: grosse

	/* Plutonia Maps */
	PHUSTR_1_NUM,				//level 1: congo
	PHUSTR_2_NUM,				//level 2: well of souls
	PHUSTR_3_NUM,				//level 3: aztec
	PHUSTR_4_NUM,				//level 4: caged
	PHUSTR_5_NUM,				//level 5: ghost town
	PHUSTR_6_NUM,				//level 6: baron's lair
	PHUSTR_7_NUM,				//level 7: caughtyard
	PHUSTR_8_NUM,				//level 8: realm
	PHUSTR_9_NUM,				//level 9: abattoire
	PHUSTR_10_NUM,				//level 10: onslaught
	PHUSTR_11_NUM,				//level 11: hunted
	PHUSTR_12_NUM,				//level 12: speed
	PHUSTR_13_NUM,				//level 13: the crypt
	PHUSTR_14_NUM,				//level 14: genesis
	PHUSTR_15_NUM,				//level 15: the twilight
	PHUSTR_16_NUM,				//level 16: the omen
	PHUSTR_17_NUM,				//level 17: compound
	PHUSTR_18_NUM,				//level 18: neurosphere
	PHUSTR_19_NUM,				//level 19: nme
	PHUSTR_20_NUM,				//level 20: the death domain
	PHUSTR_21_NUM,				//level 21: slayer
	PHUSTR_22_NUM,				//level 22: impossible mission
	PHUSTR_23_NUM,				//level 23: tombstone
	PHUSTR_24_NUM,				//level 24: the final frontier
	PHUSTR_25_NUM,				//level 25: the temple of darkness
	PHUSTR_26_NUM,				//level 26: bunker
	PHUSTR_27_NUM,				//level 27: anti-christ
	PHUSTR_28_NUM,				//level 28: the sewers
	PHUSTR_29_NUM,				//level 29: odyssey of noises
	PHUSTR_30_NUM,				//level 30: the gateway of hell
	PHUSTR_31_NUM,				//level 31: cyberden
	PHUSTR_32_NUM,				//level 32: go 2 it

	/* TNT Maps */
	THUSTR_1_NUM,				//level 1: system control
	THUSTR_2_NUM,				//level 2: human bbq
	THUSTR_3_NUM,				//level 3: power control
	THUSTR_4_NUM,				//level 4: wormhole
	THUSTR_5_NUM,				//level 5: hanger
	THUSTR_6_NUM,				//level 6: open season
	THUSTR_7_NUM,				//level 7: prison
	THUSTR_8_NUM,				//level 8: metal
	THUSTR_9_NUM,				//level 9: stronghold
	THUSTR_10_NUM,				//level 10: redemption
	THUSTR_11_NUM,				//level 11: storage facility
	THUSTR_12_NUM,				//level 12: crater
	THUSTR_13_NUM,				//level 13: nukage processing
	THUSTR_14_NUM,				//level 14: steel works
	THUSTR_15_NUM,				//level 15: dead zone
	THUSTR_16_NUM,				//level 16: deepest reaches
	THUSTR_17_NUM,				//level 17: processing area
	THUSTR_18_NUM,				//level 18: mill
	THUSTR_19_NUM,				//level 19: shipping/respawning
	THUSTR_20_NUM,				//level 20: central processing
	THUSTR_21_NUM,				//level 21: administration center
	THUSTR_22_NUM,				//level 22: habitat
	THUSTR_23_NUM,				//level 23: lunar mining project
	THUSTR_24_NUM,				//level 24: quarry
	THUSTR_25_NUM,				//level 25: baron's den
	THUSTR_26_NUM,				//level 26: ballistyx
	THUSTR_27_NUM,				//level 27: mount pain
	THUSTR_28_NUM,				//level 28: heck
	THUSTR_29_NUM,				//level 29: river styx
	THUSTR_30_NUM,				//level 30: last call
	THUSTR_31_NUM,				//level 31: pharaoh
	THUSTR_32_NUM,				//level 32: caribbean

	/* Chat Macros */
	HUSTR_CHATMACRO1_NUM,		//I'm ready to kick butt!
	HUSTR_CHATMACRO2_NUM,		//I'm OK.
	HUSTR_CHATMACRO3_NUM,		//I'm not looking too good!
	HUSTR_CHATMACRO4_NUM,		//Help!
	HUSTR_CHATMACRO5_NUM,		//You suck!
	HUSTR_CHATMACRO6_NUM,		//Next time, scumbag...
	HUSTR_CHATMACRO7_NUM,		//Come here!
	HUSTR_CHATMACRO8_NUM,		//I'll take care of it.
	HUSTR_CHATMACRO9_NUM,		//Yes
	HUSTR_CHATMACRO0_NUM,		//No
	HUSTR_TALKTOSELF1_NUM,		//You mumble to yourself
	HUSTR_TALKTOSELF2_NUM,		//Who's there?
	HUSTR_TALKTOSELF3_NUM,		//You scare yourself
	HUSTR_TALKTOSELF4_NUM,		//You start to rave
	HUSTR_TALKTOSELF5_NUM,		//You've lost it...
	HUSTR_MESSAGESENT_NUM,		//[Message Sent]

	/* Automap */
	AMSTR_FOLLOWON_NUM,			//Follow Mode ON
	AMSTR_FOLLOWOFF_NUM,		//Follow Mode OFF
	AMSTR_GRIDON_NUM,			//Grid ON
	AMSTR_GRIDOFF_NUM,			//Grid OFF
	AMSTR_MARKEDSPOT_NUM,		//Marked Spot
	AMSTR_MARKSCLEARED_NUM,		//All Marks Cleared

	/* Cheats */
	STSTR_MUS_NUM,				//Music Change
	STSTR_NOMUS_NUM,			//IMPOSSIBLE SELECTION
	STSTR_DQDON_NUM,			//Degreelessness Mode On
	STSTR_DQDOFF_NUM,			//Degreelessness Mode Off
	STSTR_KFAADDED_NUM,			//Very Happy Ammo Added
	STSTR_FAADDED_NUM,			//Ammo (no keys) Added
	STSTR_NCON_NUM,				//No Clipping Mode ON
	STSTR_NCOFF_NUM,			//No Clipping Mode OFF
	STSTR_BEHOLD_NUM,			//inVuln, Str, Inviso, Rad, Allmap, or Lite-amp
	STSTR_BEHOLDX_NUM,			//Power-up Toggled
	STSTR_CHOPPERS_NUM,			//... doesn't suck - GM
	STSTR_CLEV_NUM,				//Changing Level...

	/* End Text */
	E1TEXT_NUM,
	E2TEXT_NUM,
	E3TEXT_NUM,
	E4TEXT_NUM,
	C1TEXT_NUM,
	C2TEXT_NUM,
	C3TEXT_NUM,
	C4TEXT_NUM,
	C5TEXT_NUM,
	C6TEXT_NUM,
	T1TEXT_NUM,
	T2TEXT_NUM,
	T3TEXT_NUM,
	T4TEXT_NUM,
	T5TEXT_NUM,
	T6TEXT_NUM,

	/* Finale Cast */
	CC_ZOMBIE_NUM,				//ZOMBIEMAN
	CC_SHOTGUN_NUM,				//SHOTGUN GUY
	CC_HEAVY_NUM,				//HEAVY WEAPON DUDE
	CC_IMP_NUM,					//IMP
	CC_DEMON_NUM,				//DEMON
	CC_LOST_NUM,				//LOST SOUL
	CC_CACO_NUM,				//CACODEMON
	CC_HELL_NUM,				//HELL KNIGHT
	CC_BARON_NUM,				//BARON OF HELL
	CC_ARACH_NUM,				//ARACHNOTRON
	CC_PAIN_NUM,				//PAIN ELEMENTAL
	CC_REVEN_NUM,				//REVENANT
	CC_MANCU_NUM,				//MANCUBUS
	CC_ARCH_NUM,				//ARCH-VILE
	CC_SPIDER_NUM,				//THE SPIDER MASTERMIND
	CC_CYBER_NUM,				//THE CYBERDEMON
	CC_HERO_NUM,				//OUR HERO

	/* Quit Messages - Doom1 */
	QUITMSG_NUM,				//are you sure you want to quit this great game?
	QUITMSG1_NUM,				//please don't leave, there's more demons to toast!
	QUITMSG2_NUM,				//let's beat it -- this is turning into a bloodbath!
	QUITMSG3_NUM,				//i wouldn't leave if i were you. dos is much worse.
	QUITMSG4_NUM,				//you're trying to say you like dos better than me, right?
	QUITMSG5_NUM,				//don't leave yet -- there's a demon around that corner!
	QUITMSG6_NUM,				//ya know, next time you come in here i'm gonna toast ya.
	QUITMSG7_NUM,				//go ahead and leave. see if i care.

	/* Quit Messages - Doom2 */
	QUIT2MSG_NUM,				//you want to quit? then, thou hast lost an eighth!
	QUIT2MSG1_NUM,				//don't go now, there's a dimensional shambler waiting at the dos prompt!
	QUIT2MSG2_NUM,				//get outta here and go back to your boring programs.
	QUIT2MSG3_NUM,				//if i were your boss, i'd deathmatch ya in a minute!
	QUIT2MSG4_NUM,				//look, bud. you leave now and you forfeit your body count!
	QUIT2MSG5_NUM,				//just leave. when you come back, i'll be waiting with a bat.
	QUIT2MSG6_NUM,				//you're lucky i don't smack you for thinking about leaving.

	/* Flats */
	FLOOR4_8_NUM,				//FLOOR4_8
	SFLR6_1_NUM,				//SFLR6_1
	MFLR8_4_NUM,				//MFLR8_4
	MFLR8_3_NUM,				//MFLR8_3
	SLIME16_NUM,				//SLIME16
	RROCK14_NUM,				//RROCK14
	RROCK07_NUM,				//RROCK07
	RROCK17_NUM,				//RROCK17
	RROCK13_NUM,				//RROCK13
	RROCK19_NUM,				//RROCK19

	CREDIT_NUM,					//CREDIT
	HELP2_NUM,					//HELP2
	VICTORY2_NUM,				//VICTORY2
	ENDPIC_NUM,					//ENDPIC

	/* Dos screen */
	MODIFIED_NUM,
	SHAREWARE_NUM,
	COMERCIAL_NUM,

	/* Startup messages */
	AUSTIN_NUM,					//Austin Virtual Gaming: Levels will end after 20 minutes
	M_LOAD_NUM,					//M_LoadDefaults: Load system defaults.
	Z_INIT_NUM,					//Z_Init: Init zone memory allocation daemon.
	W_INIT_NUM,					//W_Init: Init WADfiles.
	M_INIT_NUM,					//M_Init: Init miscellaneous info.
	R_INIT_NUM,					//R_Init: Init DOOM refresh daemon - 
	P_INIT_NUM,					//P_Init: Init Playloop state.
	I_INIT_NUM,					//I_Init: Setting up machine state.
	D_CHECKNET_NUM,				//D_CheckNetGame: Checking network game status.
	S_SETSOUND_NUM,				//S_Init: Setting up sound.
	HU_INIT_NUM,				//HU_Init: Setting up heads up display.
	ST_INIT_NUM,				//ST_Init: Init status bar.
	STATREG_NUM,				//External statistics registered.

	/* IWADS */
	DOOM2WAD_NUM,				//doom2.wad
	DOOMUWAD_NUM,				//doomu.wad
	DOOMWAD_NUM,				//doom.wad
	DOOM1WAD_NUM,				//doom1.wad

	/* Unused */
	CDROM_DIR_NUM,				//c:\\doomdata
	CDROM_DEF_NUM,				//c:/doomdata/default.cfg
	CDROM_SAVE_NUM,				//c:\\doomdata\\" SAVEGAMENAME "%c.dsg
	NORM_SAVE_NUM,				//SAVEGAMENAME "%c.dsg"

	/* CDROM misc */
	CDROM_SAVEI_NUM,			//c:\\doomdata\\" SAVEGAMENAME "%d.dsg
	NORM_SAVEI_NUM,				//SAVEGAMENAME "%d.dsg"

	//SoM: 3/9/2000: Add boom messages.
	PD_BLUEC_NUM,				//You need a blue card to open this door
	PD_REDC_NUM,				//You need a red card to open this door
	PD_YELLOWC_NUM,				//You need a yellow card to open this door
	PD_BLUES_NUM,				//You need a blue skull to open this door
	PD_REDS_NUM,				//You need a red skull to open this door
	PD_YELLOWS_NUM,				//You need a yellow skull to open this door
	PD_ANY_NUM,					//Any key will open this door
	PD_ALL3_NUM,				//You need all three keys to open this door
	PD_ALL6_NUM,				//You need all six keys to open this door
	
	// heretic stuff

	TXT_ARTIHEALTH_NUM,
	TXT_ARTIFLY_NUM,
	TXT_ARTIINVULNERABILITY_NUM,
	TXT_ARTITOMEOFPOWER_NUM,
	TXT_ARTIINVISIBILITY_NUM,
	TXT_ARTIEGG_NUM,
	TXT_ARTISUPERHEALTH_NUM,
	TXT_ARTITORCH_NUM,
	TXT_ARTIFIREBOMB_NUM,
	TXT_ARTITELEPORT_NUM,

	TXT_AMMOGOLDWAND1_NUM,
	TXT_AMMOGOLDWAND2_NUM,
	TXT_AMMOMACE1_NUM,
	TXT_AMMOMACE2_NUM,
	TXT_AMMOCROSSBOW1_NUM,
	TXT_AMMOCROSSBOW2_NUM,
	TXT_AMMOBLASTER1_NUM,
	TXT_AMMOBLASTER2_NUM,
	TXT_AMMOSKULLROD1_NUM,
	TXT_AMMOSKULLROD2_NUM,
	TXT_AMMOPHOENIXROD1_NUM,
	TXT_AMMOPHOENIXROD2_NUM,

	TXT_WPNMACE_NUM,
	TXT_WPNCROSSBOW_NUM,
	TXT_WPNBLASTER_NUM,
	TXT_WPNSKULLROD_NUM,
	TXT_WPNPHOENIXROD_NUM,
	TXT_WPNGAUNTLETS_NUM,

	TXT_ITEMBAGOFHOLDING_NUM,

	TXT_CHEATGODON_NUM,
	TXT_CHEATGODOFF_NUM,
	TXT_CHEATNOCLIPON_NUM,
	TXT_CHEATNOCLIPOFF_NUM,
	TXT_CHEATWEAPONS_NUM,
	TXT_CHEATFLIGHTON_NUM,
	TXT_CHEATFLIGHTOFF_NUM,
	TXT_CHEATPOWERON_NUM,
	TXT_CHEATPOWEROFF_NUM,
	TXT_CHEATHEALTH_NUM,
	TXT_CHEATKEYS_NUM,
	TXT_CHEATSOUNDON_NUM,
	TXT_CHEATSOUNDOFF_NUM,
	TXT_CHEATTICKERON_NUM,
	TXT_CHEATTICKEROFF_NUM,
	TXT_CHEATARTIFACTS1_NUM,
	TXT_CHEATARTIFACTS2_NUM,
	TXT_CHEATARTIFACTS3_NUM,
	TXT_CHEATARTIFACTSFAIL_NUM,
	TXT_CHEATWARP_NUM,
	TXT_CHEATSCREENSHOT_NUM,
	TXT_CHEATCHICKENON_NUM,
	TXT_CHEATCHICKENOFF_NUM,
	TXT_CHEATMASSACRE_NUM,
	TXT_CHEATIDDQD_NUM,
	TXT_CHEATIDKFA_NUM,

	HERETIC_E1M1_NUM,
	HERETIC_E1M2_NUM,
	HERETIC_E1M3_NUM,
	HERETIC_E1M4_NUM,
	HERETIC_E1M5_NUM,
	HERETIC_E1M6_NUM,
	HERETIC_E1M7_NUM,
	HERETIC_E1M8_NUM,
	HERETIC_E1M9_NUM,
	HERETIC_E2M1_NUM,
	HERETIC_E2M2_NUM,
	HERETIC_E2M3_NUM,
	HERETIC_E2M4_NUM,
	HERETIC_E2M5_NUM,
	HERETIC_E2M6_NUM,
	HERETIC_E2M7_NUM,
	HERETIC_E2M8_NUM,
	HERETIC_E2M9_NUM,
	HERETIC_E3M1_NUM,
	HERETIC_E3M2_NUM,
	HERETIC_E3M3_NUM,
	HERETIC_E3M4_NUM,
	HERETIC_E3M5_NUM,
	HERETIC_E3M6_NUM,
	HERETIC_E3M7_NUM,
	HERETIC_E3M8_NUM,
	HERETIC_E3M9_NUM,
	HERETIC_E4M1_NUM,
	HERETIC_E4M2_NUM,
	HERETIC_E4M3_NUM,
	HERETIC_E4M4_NUM,
	HERETIC_E4M5_NUM,
	HERETIC_E4M6_NUM,
	HERETIC_E4M7_NUM,
	HERETIC_E4M8_NUM,
	HERETIC_E4M9_NUM,
	HERETIC_E5M1_NUM,
	HERETIC_E5M2_NUM,
	HERETIC_E5M3_NUM,
	HERETIC_E5M4_NUM,
	HERETIC_E5M5_NUM,
	HERETIC_E5M6_NUM,
	HERETIC_E5M7_NUM,
	HERETIC_E5M8_NUM,
	HERETIC_E5M9_NUM,
	HERETIC_E6M1_NUM,
	HERETIC_E6M2_NUM,
	HERETIC_E6M3_NUM,

	HERETIC_E1TEXT,
	HERETIC_E2TEXT,
	HERETIC_E3TEXT,
	HERETIC_E4TEXT,
	HERETIC_E5TEXT,

	DEATHMSG_SUICIDE,
	DEATHMSG_TELEFRAG,
	DEATHMSG_FIST,
	DEATHMSG_GUN,
	DEATHMSG_SHOTGUN,
	DEATHMSG_MACHGUN,
	DEATHMSG_ROCKET,
	DEATHMSG_GIBROCKET,
	DEATHMSG_PLASMA,
	DEATHMSG_BFGBALL,
	DEATHMSG_CHAINSAW,
	DEATHMSG_SUPSHOTGUN,
	DEATHMSG_PLAYUNKNOW,
	DEATHMSG_HELLSLIME,
	DEATHMSG_NUKE,
	DEATHMSG_SUPHELLSLIME,
	DEATHMSG_SPECUNKNOW,
	DEATHMSG_BARRELFRAG,
	DEATHMSG_BARREL,
	DEATHMSG_POSSESSED,
	DEATHMSG_SHOTGUY,
	DEATHMSG_VILE,
	DEATHMSG_FATSO,
	DEATHMSG_CHAINGUY,
	DEATHMSG_TROOP,
	DEATHMSG_SERGEANT,
	DEATHMSG_SHADOWS,
	DEATHMSG_HEAD,
	DEATHMSG_BRUISER,
	DEATHMSG_UNDEAD,
	DEATHMSG_KNIGHT,
	DEATHMSG_SKULL,
	DEATHMSG_SPIDER,
	DEATHMSG_BABY,
	DEATHMSG_CYBORG,
	DEATHMSG_PAIN,
	DEATHMSG_WOLFSS,
	DEATHMSG_DEAD,
	
	// Heretic Death messages
	DEATHMSG_STAFF,
	DEATHMSG_SUPERSTAFF,
	DEATHMSG_GAUNTLETS,
	DEATHMSG_SUPERGAUNTLETS,
	DEATHMSG_WAND,
	DEATHMSG_SUPERWAND,
	DEATHMSG_CROSSBOW,
	DEATHMSG_SUPERCROSSBOW,
	DEATHMSG_DRAGONCLAW,
	DEATHMSG_SUPERDRAGONCLAW,
	DEATHMSG_HELLSTAFF,
	DEATHMSG_SUPERHELLSTAFF,
	DEATHMSG_HELLSTAFFRAIN,
	DEATHMSG_FIREROD,
	DEATHMSG_SUPERFIREROD,
	DEATHMSG_FIREMACE,
	DEATHMSG_SUPERFIREMACE,
	DEATHMSG_SUPERFIREMACESELF,
	DEATHMSG_BEAK,
	DEATHMSG_SUPERBEAK,
	
	DEATHMSG_DSPARIL,
	DEATHMSG_WIZARD,
	DEATHMSG_FIREGARGOYLE,
	DEATHMSG_GARGOYLE,
	DEATHMSG_GOLEM,
	DEATHMSG_GHOSTGOLEM,
	DEATHMSG_CHAOSSERPENT,
	DEATHMSG_IRONLICH,
	DEATHMSG_MAULOTAUR,
	DEATHMSG_NITROGOLEM,
	DEATHMSG_GHOSTNITROGOLEM,
	DEATHMSG_OPHIDIAN,
	DEATHMSG_SABRECLAW,
	DEATHMSG_UNDEADWARRIOR,
	DEATHMSG_GHOSTUNDEADWARRIOR,
	DEATHMSG_WEREDRAGON,

	SPECIALDEHACKED,

	DOOM2TITLE_NUM = SPECIALDEHACKED,	//DOOM 2: Hell on Earth
	DOOMUTITLE_NUM,				//The Ultimate DOOM Startup
	DOOMTITLE_NUM,				//DOOM Registered Startup
	DOOM1TITLE_NUM,				//DOOM Shareware Startup

	NUMTEXT
} text_enum;

extern char *text[];

//
//      Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR          text[D_DEVSTR_NUM]
#define D_CDROM           text[D_CDROM_NUM]

//
//      M_Menu.C
//
#define PRESSKEY          text[PRESSKEY_NUM]
#define PRESSYN           text[PRESSYN_NUM]
#define LOADNET           text[LOADNET_NUM]
#define QLOADNET          text[QLOADNET_NUM]
#define QSAVESPOT         text[QSAVESPOT_NUM]
#define SAVEDEAD          text[SAVEDEAD_NUM]
#define QSPROMPT          text[QSPROMPT_NUM]
#define QLPROMPT          text[QLPROMPT_NUM]
#define NEWGAME           text[NEWGAME_NUM]
#define NIGHTMARE         text[NIGHTMARE_NUM]
#define SWSTRING          text[SWSTRING_NUM]
#define MSGOFF            text[MSGOFF_NUM]
#define MSGON             text[MSGON_NUM]
#define NETEND            text[NETEND_NUM]
#define ENDGAME           text[ENDGAME_NUM]
#define NOENDGAME         text[NOENDGAME_NUM]
#define DOSY              text[DOSY_NUM]
#define DETAILHI          text[DETAILHI_NUM]
#define DETAILLO          text[DETAILLO_NUM]
#define GAMMALVL0         text[GAMMALVL0_NUM]
#define GAMMALVL1         text[GAMMALVL1_NUM]
#define GAMMALVL2         text[GAMMALVL2_NUM]
#define GAMMALVL3         text[GAMMALVL3_NUM]
#define GAMMALVL4         text[GAMMALVL4_NUM]
#define EMPTYSTRING       text[EMPTYSTRING_NUM]

//
//      P_inter.C
//
#define GOTARMOR          text[GOTARMOR_NUM]
#define GOTMEGA           text[GOTMEGA_NUM]
#define GOTHTHBONUS       text[GOTHTHBONUS_NUM]
#define GOTARMBONUS       text[GOTARMBONUS_NUM]
#define GOTSTIM           text[GOTSTIM_NUM]
#define GOTMEDINEED       text[GOTMEDINEED_NUM]
#define GOTMEDIKIT        text[GOTMEDIKIT_NUM]
#define GOTSUPER          text[GOTSUPER_NUM]
#define GOTBLUECARD       text[GOTBLUECARD_NUM]
#define GOTYELWCARD       text[GOTYELWCARD_NUM]
#define GOTREDCARD        text[GOTREDCARD_NUM]
#define GOTBLUESKUL       text[GOTBLUESKUL_NUM]
#define GOTYELWSKUL       text[GOTYELWSKUL_NUM]
#define GOTREDSKULL       text[GOTREDSKULL_NUM]
#define GOTINVUL          text[GOTINVUL_NUM]
#define GOTBERSERK        text[GOTBERSERK_NUM]
#define GOTINVIS          text[GOTINVIS_NUM]
#define GOTSUIT           text[GOTSUIT_NUM]
#define GOTMAP            text[GOTMAP_NUM]
#define GOTVISOR          text[GOTVISOR_NUM]
#define GOTMSPHERE        text[GOTMSPHERE_NUM]
#define GOTCLIP           text[GOTCLIP_NUM]
#define GOTCLIPBOX        text[GOTCLIPBOX_NUM]
#define GOTROCKET         text[GOTROCKET_NUM]
#define GOTROCKBOX        text[GOTROCKBOX_NUM]
#define GOTCELL           text[GOTCELL_NUM]
#define GOTCELLBOX        text[GOTCELLBOX_NUM]
#define GOTSHELLS         text[GOTSHELLS_NUM]
#define GOTSHELLBOX       text[GOTSHELLBOX_NUM]
#define GOTBACKPACK       text[GOTBACKPACK_NUM]
#define GOTBFG9000        text[GOTBFG9000_NUM]
#define GOTCHAINGUN       text[GOTCHAINGUN_NUM]
#define GOTCHAINSAW       text[GOTCHAINSAW_NUM]
#define GOTLAUNCHER       text[GOTLAUNCHER_NUM]
#define GOTPLASMA         text[GOTPLASMA_NUM]
#define GOTSHOTGUN        text[GOTSHOTGUN_NUM]
#define GOTSHOTGUN2       text[GOTSHOTGUN2_NUM]

//
// P_Doors.C
//
#define PD_BLUEO          text[PD_BLUEO_NUM]
#define PD_REDO           text[PD_REDO_NUM]
#define PD_YELLOWO        text[PD_YELLOWO_NUM]
#define PD_BLUEK          text[PD_BLUEK_NUM]
#define PD_REDK           text[PD_REDK_NUM]
#define PD_YELLOWK        text[PD_YELLOWK_NUM]

//SoM: 3/9/2000: Add new key messages.
#define PD_BLUEC          text[PD_BLUEC_NUM]
#define PD_REDC           text[PD_REDC_NUM]
#define PD_YELLOWC        text[PD_YELLOWC_NUM]
#define PD_BLUES          text[PD_BLUES_NUM]
#define PD_REDS           text[PD_REDS_NUM]
#define PD_YELLOWS        text[PD_YELLOWS_NUM]
#define PD_ANY            text[PD_ANY_NUM]
#define PD_ALL3           text[PD_ALL3_NUM]
#define PD_ALL6           text[PD_ALL6_NUM]

//
//      G_game.C
//
#define GGSAVED           text[GGSAVED_NUM]

//
//      HU_stuff.C
//
#define HUSTR_MSGU        text[HUSTR_MSGU_NUM]
#define HUSTR_E1M1        text[HUSTR_E1M1_NUM]
#define HUSTR_E1M2        text[HUSTR_E1M2_NUM]
#define HUSTR_E1M3        text[HUSTR_E1M3_NUM]
#define HUSTR_E1M4        text[HUSTR_E1M4_NUM]
#define HUSTR_E1M5        text[HUSTR_E1M5_NUM]
#define HUSTR_E1M6        text[HUSTR_E1M6_NUM]
#define HUSTR_E1M7        text[HUSTR_E1M7_NUM]
#define HUSTR_E1M8        text[HUSTR_E1M8_NUM]
#define HUSTR_E1M9        text[HUSTR_E1M9_NUM]
#define HUSTR_E2M1        text[HUSTR_E2M1_NUM]
#define HUSTR_E2M2        text[HUSTR_E2M2_NUM]
#define HUSTR_E2M3        text[HUSTR_E2M3_NUM]
#define HUSTR_E2M4        text[HUSTR_E2M4_NUM]
#define HUSTR_E2M5        text[HUSTR_E2M5_NUM]
#define HUSTR_E2M6        text[HUSTR_E2M6_NUM]
#define HUSTR_E2M7        text[HUSTR_E2M7_NUM]
#define HUSTR_E2M8        text[HUSTR_E2M8_NUM]
#define HUSTR_E2M9        text[HUSTR_E2M9_NUM]
#define HUSTR_E3M1        text[HUSTR_E3M1_NUM]
#define HUSTR_E3M2        text[HUSTR_E3M2_NUM]
#define HUSTR_E3M3        text[HUSTR_E3M3_NUM]
#define HUSTR_E3M4        text[HUSTR_E3M4_NUM]
#define HUSTR_E3M5        text[HUSTR_E3M5_NUM]
#define HUSTR_E3M6        text[HUSTR_E3M6_NUM]
#define HUSTR_E3M7        text[HUSTR_E3M7_NUM]
#define HUSTR_E3M8        text[HUSTR_E3M8_NUM]
#define HUSTR_E3M9        text[HUSTR_E3M9_NUM]
#define HUSTR_E4M1        text[HUSTR_E4M1_NUM]
#define HUSTR_E4M2        text[HUSTR_E4M2_NUM]
#define HUSTR_E4M3        text[HUSTR_E4M3_NUM]
#define HUSTR_E4M4        text[HUSTR_E4M4_NUM]
#define HUSTR_E4M5        text[HUSTR_E4M5_NUM]
#define HUSTR_E4M6        text[HUSTR_E4M6_NUM]
#define HUSTR_E4M7        text[HUSTR_E4M7_NUM]
#define HUSTR_E4M8        text[HUSTR_E4M8_NUM]
#define HUSTR_E4M9        text[HUSTR_E4M9_NUM]
#define HUSTR_1           text[HUSTR_1_NUM]
#define HUSTR_2           text[HUSTR_2_NUM]
#define HUSTR_3           text[HUSTR_3_NUM]
#define HUSTR_4           text[HUSTR_4_NUM]
#define HUSTR_5           text[HUSTR_5_NUM]
#define HUSTR_6           text[HUSTR_6_NUM]
#define HUSTR_7           text[HUSTR_7_NUM]
#define HUSTR_8           text[HUSTR_8_NUM]
#define HUSTR_9           text[HUSTR_9 _NUM]
#define HUSTR_10          text[HUSTR_10_NUM]
#define HUSTR_11          text[HUSTR_11_NUM]
#define HUSTR_12          text[HUSTR_12_NUM]
#define HUSTR_13          text[HUSTR_13_NUM]
#define HUSTR_14          text[HUSTR_14_NUM]
#define HUSTR_15          text[HUSTR_15_NUM]
#define HUSTR_16          text[HUSTR_16_NUM]
#define HUSTR_17          text[HUSTR_17_NUM]
#define HUSTR_18          text[HUSTR_18_NUM]
#define HUSTR_19          text[HUSTR_19_NUM]
#define HUSTR_20          text[HUSTR_20_NUM]
#define HUSTR_21          text[HUSTR_21_NUM]
#define HUSTR_22          text[HUSTR_22_NUM]
#define HUSTR_23          text[HUSTR_23_NUM]
#define HUSTR_24          text[HUSTR_24_NUM]
#define HUSTR_25          text[HUSTR_25_NUM]
#define HUSTR_26          text[HUSTR_26_NUM]
#define HUSTR_27          text[HUSTR_27_NUM]
#define HUSTR_28          text[HUSTR_28_NUM]
#define HUSTR_29          text[HUSTR_29_NUM]
#define HUSTR_30          text[HUSTR_30_NUM]
#define HUSTR_31          text[HUSTR_31_NUM]
#define HUSTR_32          text[HUSTR_32_NUM]
#define PHUSTR_1          text[PHUSTR_1_NUM]
#define PHUSTR_2          text[PHUSTR_2_NUM]
#define PHUSTR_3          text[PHUSTR_3_NUM]
#define PHUSTR_4          text[PHUSTR_4_NUM]
#define PHUSTR_5          text[PHUSTR_5_NUM]
#define PHUSTR_6          text[PHUSTR_6_NUM]
#define PHUSTR_7          text[PHUSTR_7_NUM]
#define PHUSTR_8          text[PHUSTR_8_NUM]
#define PHUSTR_9          text[PHUSTR_9_NUM]
#define PHUSTR_10         text[PHUSTR_10_NUM]
#define PHUSTR_11         text[PHUSTR_11_NUM]
#define PHUSTR_12         text[PHUSTR_12_NUM]
#define PHUSTR_13         text[PHUSTR_13_NUM]
#define PHUSTR_14         text[PHUSTR_14_NUM]
#define PHUSTR_15         text[PHUSTR_15_NUM]
#define PHUSTR_16         text[PHUSTR_16_NUM]
#define PHUSTR_17         text[PHUSTR_17_NUM]
#define PHUSTR_18         text[PHUSTR_18_NUM]
#define PHUSTR_19         text[PHUSTR_19_NUM]
#define PHUSTR_20         text[PHUSTR_20_NUM]
#define PHUSTR_21         text[PHUSTR_21_NUM]
#define PHUSTR_22         text[PHUSTR_22_NUM]
#define PHUSTR_23         text[PHUSTR_23_NUM]
#define PHUSTR_24         text[PHUSTR_24_NUM]
#define PHUSTR_25         text[PHUSTR_25_NUM]
#define PHUSTR_26         text[PHUSTR_26_NUM]
#define PHUSTR_27         text[PHUSTR_27_NUM]
#define PHUSTR_28         text[PHUSTR_28_NUM]
#define PHUSTR_29         text[PHUSTR_29_NUM]
#define PHUSTR_30         text[PHUSTR_30_NUM]
#define PHUSTR_31         text[PHUSTR_31_NUM]
#define PHUSTR_32         text[PHUSTR_32_NUM]
#define THUSTR_1          text[THUSTR_1_NUM]
#define THUSTR_2          text[THUSTR_2_NUM]
#define THUSTR_3          text[THUSTR_3_NUM]
#define THUSTR_4          text[THUSTR_4_NUM]
#define THUSTR_5          text[THUSTR_5_NUM]
#define THUSTR_6          text[THUSTR_6_NUM]
#define THUSTR_7          text[THUSTR_7_NUM]
#define THUSTR_8          text[THUSTR_8_NUM]
#define THUSTR_9          text[THUSTR_9_NUM]
#define THUSTR_10         text[THUSTR_10_NUM]
#define THUSTR_11         text[THUSTR_11_NUM]
#define THUSTR_12         text[THUSTR_12_NUM]
#define THUSTR_13         text[THUSTR_13_NUM]
#define THUSTR_14         text[THUSTR_14_NUM]
#define THUSTR_15         text[THUSTR_15_NUM]
#define THUSTR_16         text[THUSTR_16_NUM]
#define THUSTR_17         text[THUSTR_17_NUM]
#define THUSTR_18         text[THUSTR_18_NUM]
#define THUSTR_19         text[THUSTR_19_NUM]
#define THUSTR_20         text[THUSTR_20_NUM]
#define THUSTR_21         text[THUSTR_21_NUM]
#define THUSTR_22         text[THUSTR_22_NUM]
#define THUSTR_23         text[THUSTR_23_NUM]
#define THUSTR_24         text[THUSTR_24_NUM]
#define THUSTR_25         text[THUSTR_25_NUM]
#define THUSTR_26         text[THUSTR_26_NUM]
#define THUSTR_27         text[THUSTR_27_NUM]
#define THUSTR_28         text[THUSTR_28_NUM]
#define THUSTR_29         text[THUSTR_29_NUM]
#define THUSTR_30         text[THUSTR_30_NUM]
#define THUSTR_31         text[THUSTR_31_NUM]
#define THUSTR_32         text[THUSTR_32_NUM]
#define HUSTR_CHATMACRO1  text[HUSTR_CHATMACRO1_NUM]
#define HUSTR_CHATMACRO2  text[HUSTR_CHATMACRO2_NUM]
#define HUSTR_CHATMACRO3  text[HUSTR_CHATMACRO3_NUM]
#define HUSTR_CHATMACRO4  text[HUSTR_CHATMACRO4_NUM]
#define HUSTR_CHATMACRO5  text[HUSTR_CHATMACRO5_NUM]
#define HUSTR_CHATMACRO6  text[HUSTR_CHATMACRO6_NUM]
#define HUSTR_CHATMACRO7  text[HUSTR_CHATMACRO7_NUM]
#define HUSTR_CHATMACRO8  text[HUSTR_CHATMACRO8_NUM]
#define HUSTR_CHATMACRO9  text[HUSTR_CHATMACRO9_NUM]
#define HUSTR_CHATMACRO0  text[HUSTR_CHATMACRO0_NUM]
#define HUSTR_TALKTOSELF1 text[HUSTR_TALKTOSELF1_NUM]
#define HUSTR_TALKTOSELF2 text[HUSTR_TALKTOSELF2_NUM]
#define HUSTR_TALKTOSELF3 text[HUSTR_TALKTOSELF3_NUM]
#define HUSTR_TALKTOSELF4 text[HUSTR_TALKTOSELF4_NUM]
#define HUSTR_TALKTOSELF5 text[HUSTR_TALKTOSELF5_NUM]
#define HUSTR_MESSAGESENT text[HUSTR_MESSAGESENT_NUM]

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define HUSTR_KEYGREEN  'g'
#define HUSTR_KEYINDIGO 'i'
#define HUSTR_KEYBROWN  'b'
#define HUSTR_KEYRED    'r'

//
//      AM_map.C
//

#define AMSTR_FOLLOWON     text[AMSTR_FOLLOWON_NUM]
#define AMSTR_FOLLOWOFF    text[AMSTR_FOLLOWOFF_NUM]
#define AMSTR_GRIDON       text[AMSTR_GRIDON_NUM]
#define AMSTR_GRIDOFF      text[AMSTR_GRIDOFF_NUM]
#define AMSTR_MARKEDSPOT   text[AMSTR_MARKEDSPOT_NUM]
#define AMSTR_MARKSCLEARED text[AMSTR_MARKSCLEARED_NUM]

//
//      ST_stuff.C
//

#define STSTR_MUS          text[STSTR_MUS_NUM]
#define STSTR_NOMUS        text[STSTR_NOMUS_NUM]
#define STSTR_DQDON        text[STSTR_DQDON_NUM]
#define STSTR_DQDOFF       text[STSTR_DQDOFF_NUM]
#define STSTR_KFAADDED     text[STSTR_KFAADDED_NUM]
#define STSTR_FAADDED      text[STSTR_FAADDED_NUM]
#define STSTR_NCON         text[STSTR_NCON_NUM]
#define STSTR_NCOFF        text[STSTR_NCOFF_NUM]
#define STSTR_BEHOLD       text[STSTR_BEHOLD_NUM]
#define STSTR_BEHOLDX      text[STSTR_BEHOLDX_NUM]
#define STSTR_CHOPPERS     text[STSTR_CHOPPERS_NUM]
#define STSTR_CLEV         text[STSTR_CLEV_NUM]

//
//      F_Finale.C
//
#define E1TEXT             text[E1TEXT_NUM]
#define E2TEXT             text[E2TEXT_NUM]
#define E3TEXT             text[E3TEXT_NUM]
#define E4TEXT             text[E4TEXT_NUM]

// after level 6] put this:
#define C1TEXT             text[C1TEXT_NUM]

// After level 11] put this:
#define C2TEXT             text[C2TEXT_NUM]

// After level 20] put this:
#define C3TEXT             text[C3TEXT_NUM]

// After level 29] put this:
#define C4TEXT             text[C4TEXT_NUM]

// Before level 31] put this:
#define C5TEXT             text[C5TEXT_NUM]

// Before level 32] put this:

#define C6TEXT             text[C6TEXT_NUM]

#define T1TEXT             text[T1TEXT_NUM]
#define T2TEXT             text[T2TEXT_NUM]
#define T3TEXT             text[T3TEXT_NUM]
#define T4TEXT             text[T4TEXT_NUM]
#define T5TEXT             text[T5TEXT_NUM]
#define T6TEXT             text[T6TEXT_NUM]

//
// Character cast strings F_FINALE.C
//
#define CC_ZOMBIE          text[CC_ZOMBIE_NUM]
#define CC_SHOTGUN         text[CC_SHOTGUN_NUM]
#define CC_HEAVY           text[CC_HEAVY_NUM]
#define CC_IMP             text[CC_IMP_NUM]
#define CC_DEMON           text[CC_DEMON_NUM]
#define CC_LOST            text[CC_LOST_NUM]
#define CC_CACO            text[CC_CACO_NUM]
#define CC_HELL            text[CC_HELL _NUM]
#define CC_BARON           text[CC_BARON_NUM]
#define CC_ARACH           text[CC_ARACH_NUM]
#define CC_PAIN            text[CC_PAIN_NUM]
#define CC_REVEN           text[CC_REVEN_NUM]
#define CC_MANCU           text[CC_MANCU_NUM]
#define CC_ARCH            text[CC_ARCH_NUM]
#define CC_SPIDER          text[CC_SPIDER_NUM]
#define CC_CYBER           text[CC_CYBER_NUM]
#define CC_HERO            text[CC_HERO_NUM]

// heretic
#define TXT_ARTIHEALTH           text[TXT_ARTIHEALTH_NUM]
#define TXT_ARTIFLY              text[TXT_ARTIFLY_NUM]
#define TXT_ARTIINVULNERABILITY  text[TXT_ARTIINVULNERABILITY_NUM]
#define TXT_ARTITOMEOFPOWER      text[TXT_ARTITOMEOFPOWER_NUM]
#define TXT_ARTIINVISIBILITY     text[TXT_ARTIINVISIBILITY_NUM]
#define TXT_ARTIEGG              text[TXT_ARTIEGG_NUM]
#define TXT_ARTISUPERHEALTH      text[TXT_ARTISUPERHEALTH_NUM]
#define TXT_ARTITORCH            text[TXT_ARTITORCH_NUM]
#define TXT_ARTIFIREBOMB         text[TXT_ARTIFIREBOMB_NUM]
#define TXT_ARTITELEPORT         text[TXT_ARTITELEPORT_NUM]

#define TXT_AMMOGOLDWAND1        text[TXT_AMMOGOLDWAND1_NUM]
#define TXT_AMMOGOLDWAND2        text[TXT_AMMOGOLDWAND2_NUM]
#define TXT_AMMOMACE1            text[TXT_AMMOMACE1_NUM]
#define TXT_AMMOMACE2            text[TXT_AMMOMACE2_NUM]
#define TXT_AMMOCROSSBOW1        text[TXT_AMMOCROSSBOW1_NUM]
#define TXT_AMMOCROSSBOW2        text[TXT_AMMOCROSSBOW2_NUM]
#define TXT_AMMOBLASTER1         text[TXT_AMMOBLASTER1_NUM]
#define TXT_AMMOBLASTER2         text[TXT_AMMOBLASTER2_NUM]
#define TXT_AMMOSKULLROD1        text[TXT_AMMOSKULLROD1_NUM]
#define TXT_AMMOSKULLROD2        text[TXT_AMMOSKULLROD2_NUM]
#define TXT_AMMOPHOENIXROD1      text[TXT_AMMOPHOENIXROD1_NUM]
#define TXT_AMMOPHOENIXROD2      text[TXT_AMMOPHOENIXROD2_NUM]

#define TXT_WPNMACE              text[TXT_WPNMACE_NUM]
#define TXT_WPNCROSSBOW          text[TXT_WPNCROSSBOW_NUM]
#define TXT_WPNBLASTER           text[TXT_WPNBLASTER_NUM]
#define TXT_WPNSKULLROD          text[TXT_WPNSKULLROD_NUM]
#define TXT_WPNPHOENIXROD        text[TXT_WPNPHOENIXROD_NUM]
#define TXT_WPNGAUNTLETS         text[TXT_WPNGAUNTLETS_NUM]

#define TXT_ITEMBAGOFHOLDING     text[TXT_ITEMBAGOFHOLDING_NUM]

#define TXT_CHEATGODON           text[TXT_CHEATGODON_NUM]
#define TXT_CHEATGODOFF          text[TXT_CHEATGODOFF_NUM]
#define TXT_CHEATNOCLIPON        text[TXT_CHEATNOCLIPON_NUM]
#define TXT_CHEATNOCLIPOFF       text[TXT_CHEATNOCLIPOFF_NUM]
#define TXT_CHEATWEAPONS         text[TXT_CHEATWEAPONS_NUM]
#define TXT_CHEATFLIGHTON        text[TXT_CHEATFLIGHTON_NUM]
#define TXT_CHEATFLIGHTOFF       text[TXT_CHEATFLIGHTOFF_NUM]
#define TXT_CHEATPOWERON         text[TXT_CHEATPOWERON_NUM]
#define TXT_CHEATPOWEROFF        text[TXT_CHEATPOWEROFF_NUM]
#define TXT_CHEATHEALTH          text[TXT_CHEATHEALTH_NUM]
#define TXT_CHEATKEYS            text[TXT_CHEATKEYS_NUM]
#define TXT_CHEATSOUNDON         text[TXT_CHEATSOUNDON_NUM]
#define TXT_CHEATSOUNDOFF        text[TXT_CHEATSOUNDOFF_NUM]
#define TXT_CHEATTICKERON        text[TXT_CHEATTICKERON_NUM]
#define TXT_CHEATTICKEROFF       text[TXT_CHEATTICKEROFF_NUM]
#define TXT_CHEATARTIFACTS1      text[TXT_CHEATARTIFACTS1_NUM]
#define TXT_CHEATARTIFACTS2      text[TXT_CHEATARTIFACTS2_NUM]
#define TXT_CHEATARTIFACTS3      text[TXT_CHEATARTIFACTS3_NUM]
#define TXT_CHEATARTIFACTSFAIL   text[TXT_CHEATARTIFACTSFAIL_NUM]
#define TXT_CHEATWARP            text[TXT_CHEATWARP_NUM]
#define TXT_CHEATSCREENSHOT      text[TXT_CHEATSCREENSHOT_NUM]
#define TXT_CHEATCHICKENON       text[TXT_CHEATCHICKENON_NUM]
#define TXT_CHEATCHICKENOFF      text[TXT_CHEATCHICKENOFF_NUM]
#define TXT_CHEATMASSACRE        text[TXT_CHEATMASSACRE_NUM]
#define TXT_CHEATIDDQD           text[TXT_CHEATIDDQD_NUM]
#define TXT_CHEATIDKFA           text[TXT_CHEATIDKFA_NUM]

#endif

