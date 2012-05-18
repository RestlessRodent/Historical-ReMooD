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
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
//    Globally defined strings.

#include "dstrings.h"
#include "z_zone.h"

char* text[NUMTEXT] =
{
	"Development mode ON.\n",
	"CD-ROM Version: default.cfg from c:\\doomdata\n",
	"press a key.",
	"press y or n.",
	"only the server can do a load net game!\n\npress a key.",
	"you can't quickload during a netgame!\n\npress a key.",
	"you haven't picked a quicksave slot yet!\n\npress a key.",
	"you can't save if you aren't playing!\n\npress a key.",
	"quicksave over your game named\n\n'%s'?\n\npress y or n.",
	"do you want to quickload the game named\n\n'%s'?\n\npress y or n.",
	"you can't start a new game\n" "while in a network game.\n\n",
	"are you sure? this skill level\nisn't even remotely fair.\n\npress y or n.",
	"this is the shareware version of doom.\n\nyou need to order the entire trilogy.\n\npress a key.",
	"Messages OFF",
	"Messages ON",
	"you can't end a netgame!\n\npress a key.",
	"are you sure you want to end the game?\n\npress y or n.",
	"you can't end a game if you aren't even playing.\n\npress a key.",
	
#if defined(__MSDOS__)
	"%s\n\n(press y to quit to dos.)",
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	"%s\n\n(press y to quit to windows.)",
#elif defined(FREEBSD)
	"%s\n\n(press y to quit to bsd.)",
#else
	"%s\n\n(press y to quit to linux.)",
#endif
	
	"High detail",
	"Low detail",
	"Gamma correction OFF",
	"Gamma correction level 1",
	"Gamma correction level 2",
	"Gamma correction level 3",
	"Gamma correction level 4",
	"empty slot",
	"Picked up the armor.",
	"Picked up the MegaArmor!",
	"Picked up a health bonus.",
	"Picked up an armor bonus.",
	"Picked up a stimpack.",
	"Picked up a medikit that you REALLY need!",
	"Picked up a medikit.",
	"Supercharge!",
	
	"Picked up a blue keycard.",
	"Picked up a yellow keycard.",
	"Picked up a red keycard.",
	"Picked up a blue skull key.",
	"Picked up a yellow skull key.",
	"Picked up a red skull key.",
	
	"Invulnerability!",
	"Berserk!",
	"Partial Invisibility",
	"Radiation Shielding Suit",
	"Computer Area Map",
	"Light Amplification Visor",
	"MegaSphere!",
	
	"Picked up a clip.",
	"Picked up a box of bullets.",
	"Picked up a rocket.",
	"Picked up a box of rockets.",
	"Picked up an energy cell.",
	"Picked up an energy cell pack.",
	"Picked up 4 shotgun shells.",
	"Picked up a box of shotgun shells.",
	"Picked up a backpack full of ammo!",
	
	"You got the BFG9000!  Oh, yes.",
	"You got the chaingun!",
	"A chainsaw!  Find some meat!",
	"You got the rocket launcher!",
	"You got the plasma gun!",
	"You got the shotgun!",
	"You got the super shotgun!",
	
	"You need a blue key to activate this object",
	"You need a red key to activate this object",
	"You need a yellow key to activate this object",
	"You need a blue key to open this door",
	"You need a red key to open this door",
	"You need a yellow key to open this door",
	
	"game saved.",
	"[Message unsent]",
	
	"E1M1: Hangar",
	"E1M2: Nuclear Plant",
	"E1M3: Toxin Refinery",
	"E1M4: Command Control",
	"E1M5: Phobos Lab",
	"E1M6: Central Processing",
	"E1M7: Computer Station",
	"E1M8: Phobos Anomaly",
	"E1M9: Military Base",
	
	"E2M1: Deimos Anomaly",
	"E2M2: Containment Area",
	"E2M3: Refinery",
	"E2M4: Deimos Lab",
	"E2M5: Command Center",
	"E2M6: Halls of the Damned",
	"E2M7: Spawning Vats",
	"E2M8: Tower of Babel",
	"E2M9: Fortress of Mystery",
	
	"E3M1: Hell Keep",
	"E3M2: Slough of Despair",
	"E3M3: Pandemonium",
	"E3M4: House of Pain",
	"E3M5: Unholy Cathedral",
	"E3M6: Mt. Erebus",
	"E3M7: Limbo",
	"E3M8: Dis",
	"E3M9: Warrens",
	
	"E4M1: Hell Beneath",
	"E4M2: Perfect Hatred",
	"E4M3: Sever The Wicked",
	"E4M4: Unruly Evil",
	"E4M5: They Will Repent",
	"E4M6: Against Thee Wickedly",
	"E4M7: And Hell Followed",
	"E4M8: Unto The Cruel",
	"E4M9: Fear",
	"level 1: entryway",
	"level 2: underhalls",
	"level 3: the gantlet",
	"level 4: the focus",
	"level 5: the waste tunnels",
	"level 6: the crusher",
	"level 7: dead simple",
	"level 8: tricks and traps",
	"level 9: the pit",
	"level 10: refueling base",
	"level 11: 'o' of destruction!",
	
	"level 12: the factory",
	"level 13: downtown",
	"level 14: the inmost dens",
	"level 15: industrial zone",
	"level 16: suburbs",
	"level 17: tenements",
	"level 18: the courtyard",
	"level 19: the citadel",
	"level 20: gotcha!",
	
	"level 21: nirvana",
	"level 22: the catacombs",
	"level 23: barrels o' fun",
	"level 24: the chasm",
	"level 25: bloodfalls",
	"level 26: the abandoned mines",
	"level 27: monster condo",
	"level 28: the spirit world",
	"level 29: the living end",
	"level 30: icon of sin",
	
	"level 31: wolfenstein",
	"level 32: grosse",
	
	"level 1: congo",
	"level 2: well of souls",
	"level 3: aztec",
	"level 4: caged",
	"level 5: ghost town",
	"level 6: baron's lair",
	"level 7: caughtyard",
	"level 8: realm",
	"level 9: abattoire",
	"level 10: onslaught",
	"level 11: hunted",
	
	"level 12: speed",
	"level 13: the crypt",
	"level 14: genesis",
	"level 15: the twilight",
	"level 16: the omen",
	"level 17: compound",
	"level 18: neurosphere",
	"level 19: nme",
	"level 20: the death domain",
	
	"level 21: slayer",
	"level 22: impossible mission",
	"level 23: tombstone",
	"level 24: the final frontier",
	"level 25: the temple of darkness",
	"level 26: bunker",
	"level 27: anti-christ",
	"level 28: the sewers",
	"level 29: odyssey of noises",
	"level 30: the gateway of hell",
	
	"level 31: cyberden",
	"level 32: go 2 it",
	
	"level 1: system control",
	"level 2: human bbq",
	"level 3: power control",
	"level 4: wormhole",
	"level 5: hanger",
	"level 6: open season",
	"level 7: prison",
	"level 8: metal",
	"level 9: stronghold",
	"level 10: redemption",
	"level 11: storage facility",
	
	"level 12: crater",
	"level 13: nukage processing",
	"level 14: steel works",
	"level 15: dead zone",
	"level 16: deepest reaches",
	"level 17: processing area",
	"level 18: mill",
	"level 19: shipping/respawning",
	"level 20: central processing",
	
	"level 21: administration center",
	"level 22: habitat",
	"level 23: lunar mining project",
	"level 24: quarry",
	"level 25: baron's den",
	"level 26: ballistyx",
	"level 27: mount pain",
	"level 28: heck",
	"level 29: river styx",
	"level 30: last call",
	
	"level 31: pharaoh",
	"level 32: caribbean",
	"I'm ready to kick butt!",
	"I'm OK.",
	"I'm not looking too good!",
	"Help!",
	"You suck!",
	"Next time, scumbag...",
	"Come here!",
	"I'll take care of it.",
	"Yes",
	"No",
	
	"You mumble to yourself",
	"Who's there?",
	"You scare yourself",
	"You start to rave",
	"You've lost it...",
	
	"[Message Sent]",
	"Follow Mode ON",
	"Follow Mode OFF",
	
	"Grid ON",
	"Grid OFF",
	"Marked Spot",
	"All Marks Cleared",
	
	"Music Change",
	"IMPOSSIBLE SELECTION",
	"Degreelessness Mode On",
	"Degreelessness Mode Off",
	
	"Very Happy Ammo Added",
	"Ammo (no keys) Added",
	
	"No Clipping Mode ON",
	"No Clipping Mode OFF",
	
	"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp",
	"Power-up Toggled",
	
	"... doesn't suck - GM",
	"Changing Level...",
	
	"Once you beat the big badasses and\n"
	"clean out the moon base you're supposed\n"
	"to win, aren't you? Aren't you? Where's\n"
	"your fat reward and ticket home? What\n"
	"the hell is this? It's not supposed to\n" "end this way!\n" "\n"
	"It stinks like rotten meat, but looks\n"
	"like the lost Deimos base.  Looks like\n"
	"you're stuck on The Shores of Hell.\n" "The only way out is through.\n"
	"\n" "To continue the DOOM experience, play\n" "The Shores of Hell and its amazing\n" "sequel, Inferno!\n",
	
	"You've done it! The hideous cyber-\n"
	"demon lord that ruled the lost Deimos\n"
	"moon base has been slain and you\n"
	"are triumphant! But ... where are\n"
	"you? You clamber to the edge of the\n"
	"moon and look down to see the awful\n" "truth.\n" "\n"
	"Deimos floats above Hell itself!\n"
	"You've never heard of anyone escaping\n"
	"from Hell, but you'll make the bastards\n"
	"sorry they ever heard of you! Quickly,\n"
	"you rappel down to  the surface of\n" "Hell.\n" "\n" "Now, it's on to the final chapter of\n" "DOOM! -- Inferno.",
	
	"The loathsome spiderdemon that\n" "masterminded the invasion of the moon\n"
	"bases and caused so much death has had\n"
	"its ass kicked for all time.\n" "\n"
	"A hidden doorway opens and you enter.\n"
	"You've proven too tough for Hell to\n"
	"contain, and now Hell at last plays\n"
	"fair -- for you emerge from the door\n"
	"to see the green fields of Earth!\n" "Home at last.\n" "\n"
	"You wonder what's been happening on\n"
	"Earth while you were battling evil\n" "unleashed. It's good that no Hell-\n" "spawn could have come through that\n" "door with you ...",
	
	"the spider mastermind must have sent forth\n"
	"its legions of hellspawn before your\n"
	"final confrontation with that terrible\n"
	"beast from hell.  but you stepped forward\n"
	"and brought forth eternal damnation and\n"
	"suffering upon the horde as a true hero\n"
	"would in the face of something so evil.\n" "\n"
	"besides, someone was gonna pay for what\n"
	"happened to daisy, your pet rabbit.\n" "\n"
	"but now, you see spread before you more\n"
	"potential pain and gibbitude as a nation\n" "of demons run amok among our cities.\n" "\n" "next stop, hell on earth!",
	
	"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n"
	"STARPORT. BUT SOMETHING IS WRONG. THE\n"
	"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n"
	"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n"
	"IS BEING SUBVERTED BY THEIR PRESENCE.\n" "\n"
	"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n"
	"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n"
	"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" "OF THE STARBASE AND FIND THE CONTROLLING\n" "SWITCH WHICH HOLDS EARTH'S POPULATION\n" "HOSTAGE.",
	
	"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n"
	"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"
	"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"
	"HUMAN LEFT ON THE FACE OF THE PLANET.\n"
	"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"
	"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"
	"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"
	"THAT YOU HAVE SAVED YOUR SPECIES.\n" "\n"
	"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"
	"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"
	"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"
	"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"
	"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"
	"YOUR OWN HOME CITY, NOT FAR FROM THE\n" "STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n" "UP AND RETURN TO THE FRAY.",
	
	"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
	"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
	"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
	"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
	"TEETH AND PLUNGE THROUGH IT.\n" "\n"
	"THERE MUST BE A WAY TO CLOSE IT ON THE\n" "OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n" "GOT TO GO THROUGH HELL TO GET TO IT?",
	
	"THE HORRENDOUS VISAGE OF THE BIGGEST\n"
	"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"
	"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"
	"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"
	"UP AND DIES, ITS THRASHING LIMBS\n"
	"DEVASTATING UNTOLD MILES OF HELL'S\n" "SURFACE.\n" "\n"
	"YOU'VE DONE IT. THE INVASION IS OVER.\n"
	"EARTH IS SAVED. HELL IS A WRECK. YOU\n"
	"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"
	"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"
	"FOREHEAD YOU BEGIN THE LONG TREK BACK\n" "HOME. REBUILDING EARTH OUGHT TO BE A\n" "LOT MORE FUN THAN RUINING IT WAS.\n",
	
	"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
	"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n" "HUMANS, RATHER THAN DEMONS. YOU WONDER\n" "WHO THE INMATES OF THIS CORNER OF HELL\n" "WILL BE.",
	
	"CONGRATULATIONS, YOU'VE FOUND THE\n" "SUPER SECRET LEVEL!  YOU'D BETTER\n" "BLAZE THROUGH THIS ONE!\n",
	
	"You've fought your way out of the infested\n"
	"experimental labs.   It seems that UAC has\n"
	"once again gulped it down.  With their\n"
	"high turnover, it must be hard for poor\n"
	"old UAC to buy corporate health insurance\n" "nowadays..\n" "\n"
	"Ahead lies the military complex, now\n"
	"swarming with diseased horrors hot to get\n" "their teeth into you. With luck, the\n" "complex still has some warlike ordnance\n" "laying around.",
	
	"You hear the grinding of heavy machinery\n"
	"ahead.  You sure hope they're not stamping\n"
	"out new hellspawn, but you're ready to\n"
	"ream out a whole herd if you have to.\n"
	"They might be planning a blood feast, but\n"
	"you feel about as mean as two thousand\n" "maniacs packed into one mad killer.\n" "\n" "You don't plan to go down easy.",
	
	"The vista opening ahead looks real damn\n"
	"familiar. Smells familiar, too -- like\n"
	"fried excrement. You didn't like this\n"
	"place before, and you sure as hell ain't\n"
	"planning to like it now. The more you\n"
	"brood on it, the madder you get.\n" "Hefting your gun, an evil grin trickles\n" "onto your face. Time to take some names.",
	
	"Suddenly, all is silent, from one horizon\n"
	"to the other. The agonizing echo of Hell\n"
	"fades away, the nightmare sky turns to\n"
	"blue, the heaps of monster corpses start \n"
	"to evaporate along with the evil stench \n"
	"that filled the air. Jeeze, maybe you've\n"
	"done it. Have you really won?\n" "\n"
	"Something rumbles in the distance.\n" "A blue light begins to glow inside the\n" "ruined skull of the demon-spitter.",
	
	"What now? Looks totally different. Kind\n"
	"of like King Tut's condo. Well,\n" "whatever's here can't be any worse\n" "than usual. Can it?  Or maybe it's best\n" "to let sleeping gods lie..",
	
	"Time for a vacation. You've burst the\n"
	"bowels of hell and by golly you're ready\n"
	"for a break. You mutter to yourself,\n"
	"Maybe someone else can kick Hell's ass\n"
	"next time around. Ahead lies a quiet town,\n"
	"with peaceful flowing water, quaint\n"
	"buildings, and presumably no Hellspawn.\n" "\n" "As you step off the transport, you hear\n" "the stomp of a cyberdemon's iron shoe.",
	
	"ZOMBIEMAN",
	"SHOTGUN GUY",
	"HEAVY WEAPON DUDE",
	"IMP",
	"DEMON",
	"LOST SOUL",
	"CACODEMON",
	"HELL KNIGHT",
	"BARON OF HELL",
	"ARACHNOTRON",
	"PAIN ELEMENTAL",
	"REVENANT",
	"MANCUBUS",
	"ARCH-VILE",
	"THE SPIDER MASTERMIND",
	"THE CYBERDEMON",
	"OUR HERO",
	
	// DOOM1
	"are you sure you want to\nquit this great game?",
	"please don't leave, there's more\ndemons to toast!",
	"let's beat it -- this is turning\ninto a bloodbath!",
#if defined(__MSDOS__)
	"i wouldn't leave if i were you.\ndos is much worse.",
	"you're trying to say you like dos\nbetter than me, right?",
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	"i wouldn't leave if i were you.\nwindows is much worse.",
	"you're trying to say you like windows\nbetter than me, right?",
#elif defined(FREEBSD)
	"i wouldn't leave if i were you.\nbsd is much worse.",
	"you're trying to say you like bsd\nbetter than me, right?",
#else
	"i wouldn't leave if i were you.\nlinux is much worse.",
	"you're trying to say you like linux\nbetter than me, right?",
#endif
	"don't leave yet -- there's a\ndemon around that corner!",
	"ya know, next time you come in here\ni'm gonna toast ya.",
	"go ahead and leave. see if i care.",
	
	// QuitDOOM II messages
	"you want to quit?\nthen, thou hast lost an eighth!",
#if defined(__MSDOS__)
	"don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	"don't go now, there's a \ndimensional shambler waiting\nat the windows desktop!",
#else
	"don't go now, there's a \ndimensional shambler waiting\nin the real world!",
#endif
	"get outta here and go back\nto your boring programs.",
	"if i were your boss, i'd \n deathmatch ya in a minute!",
	"look, bud. you leave now\nand you forfeit your body count!",
	"just leave. when you come\nback, i'll be waiting with a bat.",
	"you're lucky i don't smack\nyou for thinking about leaving.",
	
	"FLOOR4_8",
	"SFLR6_1",
	"MFLR8_4",
	"MFLR8_3",
	"SLIME16",
	"RROCK14",
	"RROCK07",
	"RROCK17",
	"RROCK13",
	"RROCK19",
	
	"CREDIT",
	"HELP2",
	"VICTORY2",
	"ENDPIC",
	
	"===========================================================================\n"
	"ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
	"get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
	"		You will not receive technical support for modified games.\n"
	"					  press enter to continue\n" "===========================================================================\n",
	
	"===========================================================================\n"
	"					 This program is Free Software!\n" "===========================================================================\n",
	
	"===========================================================================\n"
	"					 This program is Free Software!\n"
	"			 See the terms of the GNU General Public License\n" "===========================================================================\n",
	
	"Austin Virtual Gaming: Levels will end after 20 minutes\n",
	"M_LoadDefaults: Load system defaults.\n",
	"Z_Init: Init zone memory allocation daemon. \n",
	"W_Init: Init WADfiles.\n",
	"M_Init: Init miscellaneous info.\n",
	"R_Init: Init DOOM refresh daemon - ",
	"\nP_Init: Init Playloop state.\n",
	"I_Init: Setting up machine state.\n",
	"D_CheckNetGame: Checking network game status.\n",
	"S_Init: Setting up sound.\n",
	"HU_Init: Setting up heads up display.\n",
	"ST_Init: Init status bar.\n",
	"External statistics registered.\n",
	
	"doom2.wad",
	"doomu.wad",
	"doom.wad",
	"doom1.wad",
	
	"c:\\doomdata",				//UNUSED
	"c:/doomdata/default.cfg",	//UNUSED
	"c:\\doomdata\\" SAVEGAMENAME "%c.dsg",	//UNUSED
	SAVEGAMENAME "%c.dsg",		//UNUSED
	"c:\\doomdata\\" SAVEGAMENAME "%d.dsg",
	SAVEGAMENAME "%d.dsg",
	
	//SoM: 3/9/2000: Boom generic key messages:
	"You need a blue card to open this door",
	"You need a red card to open this door",
	"You need a yellow card to open this door",
	"You need a blue skull to open this door",
	"You need a red skull to open this door",
	"You need a yellow skull to open this door",
	"Any key will open this door",
	"You need all three keys to open this door",
	"You need all six keys to open this door",
	
	// heretic strings
	"QUARTZ FLASK",
	"WINGS OF WRATH",
	"RING OF INVINCIBILITY",
	"TOME OF POWER",
	"SHADOWSPHERE",
	"MORPH OVUM",
	"MYSTIC URN",
	"TORCH",
	"TIME BOMB OF THE ANCIENTS",
	"CHAOS DEVICE",
	
	"WAND CRYSTAL",
	"CRYSTAL GEODE",
	"MACE SPHERES",
	"PILE OF MACE SPHERES",
	"ETHEREAL ARROWS",
	"QUIVER OF ETHEREAL ARROWS",
	"CLAW ORB",
	"ENERGY ORB",
	"LESSER RUNES",
	"GREATER RUNES",
	"FLAME ORB",
	"INFERNO ORB",
	
	"FIREMACE",
	"ETHEREAL CROSSBOW",
	"DRAGON CLAW",
	"HELLSTAFF",
	"PHOENIX ROD",
	"GAUNTLETS OF THE NECROMANCER",
	
	"BAG OF HOLDING",
	
	"GOD MODE ON",
	"GOD MODE OFF",
	"NO CLIPPING ON",
	"NO CLIPPING OFF",
	"ALL WEAPONS",
	"FLIGHT ON",
	"FLIGHT OFF",
	"POWER ON",
	"POWER OFF",
	"FULL HEALTH",
	"ALL KEYS",
	"SOUND DEBUG ON",
	"SOUND DEBUG OFF",
	"TICKER ON",
	"TICKER OFF",
	"CHOOSE AN ARTIFACT ( A - J )",
	"HOW MANY ( 1 - 9 )",
	"YOU GOT IT",
	"BAD INPUT",
	"LEVEL WARP",
	"SCREENSHOT",
	"CHICKEN ON",
	"CHICKEN OFF",
	"MASSACRE",
	"TRYING TO CHEAT, EH?  NOW YOU DIE!",
	"CHEATER - YOU DON'T DESERVE WEAPONS",
	
	// EPISODE 1 - THE CITY OF THE DAMNED
	"E1M1:  THE DOCKS",
	"E1M2:  THE DUNGEONS",
	"E1M3:  THE GATEHOUSE",
	"E1M4:  THE GUARD TOWER",
	"E1M5:  THE CITADEL",
	"E1M6:  THE CATHEDRAL",
	"E1M7:  THE CRYPTS",
	"E1M8:  HELL'S MAW",
	"E1M9:  THE GRAVEYARD",
	// EPISODE 2 - HELL'S MAW
	"E2M1:  THE CRATER",
	"E2M2:  THE LAVA PITS",
	"E2M3:  THE RIVER OF FIRE",
	"E2M4:  THE ICE GROTTO",
	"E2M5:  THE CATACOMBS",
	"E2M6:  THE LABYRINTH",
	"E2M7:  THE GREAT HALL",
	"E2M8:  THE PORTALS OF CHAOS",
	"E2M9:  THE GLACIER",
	// EPISODE 3 - THE DOME OF D'SPARIL
	"E3M1:  THE STOREHOUSE",
	"E3M2:  THE CESSPOOL",
	"E3M3:  THE CONFLUENCE",
	"E3M4:  THE AZURE FORTRESS",
	"E3M5:  THE OPHIDIAN LAIR",
	"E3M6:  THE HALLS OF FEAR",
	"E3M7:  THE CHASM",
	"E3M8:  D'SPARIL'S KEEP",
	"E3M9:  THE AQUIFER",
	// EPISODE 4: THE OSSUARY
	"E4M1:  CATAFALQUE",
	"E4M2:  BLOCKHOUSE",
	"E4M3:  AMBULATORY",
	"E4M4:  SEPULCHER",
	"E4M5:  GREAT STAIR",
	"E4M6:  HALLS OF THE APOSTATE",
	"E4M7:  RAMPARTS OF PERDITION",
	"E4M8:  SHATTERED BRIDGE",
	"E4M9:  MAUSOLEUM",
	// EPISODE 5: THE STAGNANT DEMESNE
	"E5M1:  OCHRE CLIFFS",
	"E5M2:  RAPIDS",
	"E5M3:  QUAY",
	"E5M4:  COURTYARD",
	"E5M5:  HYDRATYR",
	"E5M6:  COLONNADE",
	"E5M7:  FOETID MANSE",
	"E5M8:  FIELD OF JUDGEMENT",
	"E5M9:  SKEIN OF D'SPARIL",
	// EPISODE 6: UNFINISHED
	"E6M1: Raven's Lair",
	"E6M2: Ruined Temple",
	"E6M3: American Legacy",
	
// Heretic E1TEXT
	"with the destruction of the iron\n" "liches and their minions, the last\n"
	"of the undead are cleared from this\n" "plane of existence.\n\n"
	"those creatures had to come from\n" "somewhere, though, and you have the\n"
	"sneaky suspicion that the fiery\n" "portal of hell's maw opens onto\n"
	"their home dimension.\n\n" "to make sure that more undead\n"
	"(or even worse things) don't come\n" "through, you'll have to seal hell's\n"
	"maw from the other side. of course\n" "this means you may get stuck in a\n"
	"very unfriendly world, but no one\n" "ever said being a Heretic was easy!",
	
// Heretic E2TEXT
	"the mighty maulotaurs have proved\n" "to be no match for you, and as\n"
	"their steaming corpses slide to the\n" "ground you feel a sense of grim\n"
	"satisfaction that they have been\n" "destroyed.\n\n" "the gateways which they guarded\n"
	"have opened, revealing what you\n" "hope is the way home. but as you\n"
	"step through, mocking laughter\n" "rings in your ears.\n\n"
	"was some other force controlling\n" "the maulotaurs? could there be even\n"
	"more horrific beings through this\n" "gate? the sweep of a crystal dome\n" "overhead where the sky should be is\n" "certainly not a good sign....",
	
// Heretic E3TEXT
	"the death of d'sparil has loosed\n" "the magical bonds holding his\n"
	"creatures on this plane, their\n" "dying screams overwhelming his own\n"
	"cries of agony.\n\n" "your oath of vengeance fulfilled,\n"
	"you enter the portal to your own\n" "world, mere moments before the dome\n"
	"shatters into a million pieces.\n\n" "but if d'sparil's power is broken\n"
	"forever, why don't you feel safe?\n" "was it that last shout just before\n"
	"his death, the one that sounded\n" "like a curse? or a summoning? you\n"
	"can't really be sure, but it might\n" "just have been a scream.\n\n" "then again, what about the other\n" "serpent riders?",
	
// Heretic E4TEXT
	"you thought you would return to your\n" "own world after d'sparil died, but\n"
	"his final act banished you to his\n" "own plane. here you entered the\n"
	"shattered remnants of lands\n" "conquered by d'sparil. you defeated\n"
	"the last guardians of these lands,\n" "but now you stand before the gates\n"
	"to d'sparil's stronghold. until this\n" "moment you had no doubts about your\n"
	"ability to face anything you might\n" "encounter, but beyond this portal\n"
	"lies the very heart of the evil\n" "which invaded your world. d'sparil\n"
	"might be dead, but the pit where he\n" "was spawned remains. now you must\n"
	"enter that pit in the hopes of\n" "finding a way out. and somewhere,\n"
	"in the darkest corner of d'sparil's\n" "demesne, his personal bodyguards\n" "await your arrival ...",
	
// Heretic E5TEXT
	"as the final maulotaur bellows his\n" "death-agony, you realize that you\n"
	"have never come so close to your own\n" "destruction. not even the fight with\n"
	"d'sparil and his disciples had been\n" "this desperate. grimly you stare at\n"
	"the gates which open before you,\n" "wondering if they lead home, or if\n"
	"they open onto some undreamed-of\n" "horror. you find yourself wondering\n"
	"if you have the strength to go on,\n" "if nothing but death and pain await\n"
	"you. but what else can you do, if\n" "the will to fight is gone? can you\n"
	"force yourself to continue in the\n" "face of such despair? do you have\n"
	"the courage? you find, in the end,\n" "that it is not within you to\n" "surrender without a fight. eyes\n" "wide, you go to meet your fate.",
	
	"%s suicides\n",
	"%s was telefraged by %s\n",
	"%s was beaten to a pulp by %s\n",
	"%s was gunned by %s\n",
	"%s was shot down by %s\n",
	"%s was machine-gunned by %s\n",
	"%s caught %s's rocket\n",
	"%s was gibbed by %s's rocket\n",
	"%s eats %s's toaster\n",
	"%s enjoys %s's big fraggin' gun\n",
	"%s was divided up into little pieces by %s's chainsaw\n",
	"%s ate 2 loads of %s's buckshot\n",
	"%s was killed by %s\n",
	"%s dies in hellslime\n",
	"%s gulped a load of nukage\n",
	"%s dies in super hellslime/strobe hurt\n",
	"%s dies in special sector\n",
	"%s was barrel-fragged by %s\n",
	"%s dies from a barrel explosion\n",
	"%s was shot by a possessed\n",
	"%s was shot down by a shotguy\n",
	"%s was blasted by an Arch-vile\n",
	"%s was exploded by a Mancubus\n",
	"%s was punctured by a Chainguy\n",
	"%s was fried by an Imp\n",
	"%s was eviscerated by a Demon\n",
	"%s was eaten by a Spectre\n",
	"%s was fried by a Cacodemon\n",
	"%s was slain by a Baron of Hell\n",
	"%s was smashed by a Revenant\n",
	"%s was slain by a Hell-Knight\n",
	"%s was ghosted by a Lost Soul\n",
	"%s was chaingunned by The Spider Mastermind\n",
	"%s was plasma'd by a Arachnotron\n",
	"%s was crushed by the Cyberdemon\n",
	"%s was killed by a Pain Elemental\n",
	"%s was killed by a WolfSS\n",
	"%s died\n",
	
	"%s was poked to death by %s's staff\n",	// DEATHMSG_STAFF
	"%s was electrocuted by %s's tomed staff\n",	// DEATHMSG_SUPERSTAFF
	"%s was tickled to death by %s's magical hands\n",	// DEATHMSG_GAUNTLETS
	"%s got soul sucked by %s's proton gloves\n",	// DEATHMSG_SUPERGAUNTLETS
	"%s got zapped by %s's elven wand\n",	// DEATHMSG_WAND
	"%s got vaporized by %s's tomed elven wand\n",	// DEATHMSG_SUPERWAND
	"%s got bolted by %s's crossbow\n",	// DEATHMSG_CROSSBOW
	"%s got shot by %s's tomed crossbow\n",	// DEATHMSG_SUPERCROSSBOW
	"%s felt the claw of %s\n",	// DEATHMSG_DRAGONCLAW
	"%s was shredded by %s's tomed dragon claw\n",	// DEATHMSG_SUPERDRAGONCLAW
	"%s felt the wrath of %s's hellstaff\n",	// DEATHMSG_HELLSTAFF
	"%s was scalded by %s's tomed hellstaff\n",	// DEATHMSG_SUPERHELLSTAFF
	"%s got acid rained on by %s\n",	// DEATHMSG_HELLSTAFFRAIN
	"%s was blasted by %s's phoenix rod\n",	// DEATHMSG_FIREROD
	"%s was burnt to a crisp by %s's tomed phoenix rod\n",	// DEATHMSG_SUPERFIREROD
	"%s felt %s's balls of steel\n",	// DEATHMSG_FIREMACE
	"%s was squished by %s's gigantic mace sphere\n",	// DEATHMSG_SUPERFIREMACE
	"%s's gigantic mace sphere helped cause suicide\n",	// DEATHMSG_SUPERFIREMACESELF
	"%s was pecked to death by %s\n",	// DEATHMSG_BEAK
	"%s was pecked to death by a blessed %s\n",	// DEATHMSG_SUPERBEAK
	
	"%s died\n",				// DEATHMSG_DSPARIL
	"%s died\n",				// DEATHMSG_WIZARD
	"%s died\n",				// DEATHMSG_FIREGARGOYLE
	"%s died\n",				// DEATHMSG_GARGOYLE
	"%s died\n",				// DEATHMSG_GOLEM
	"%s died\n",				// DEATHMSG_GHOSTGOLEM
	"%s died\n",				// DEATHMSG_CHAOSSERPENT
	"%s died\n",				// DEATHMSG_IRONLICH
	"%s died\n",				// DEATHMSG_MAULOTAUR
	"%s died\n",				// DEATHMSG_NITROGOLEM
	"%s died\n",				// DEATHMSG_GHOSTNITROGOLEM
	"%s died\n",				// DEATHMSG_OPHIDIAN
	"%s died\n",				// DEATHMSG_SABRECLAW
	"%s died\n",				// DEATHMSG_UNDEADWARRIOR
	"%s died\n",				// DEATHMSG_GHOSTUNDEADWARRIOR
	"%s died\n",				// DEATHMSG_WEREDRAGON
	
	//BP: here is special dehacked handling, include centring and version
	
	"DOOM 2: Hell on Earth",
	"The Ultimate DOOM Startup",
	"DOOM Registered Startup",
	"DOOM Shareware Startup",
	
};

char savegamename[256];

/******************************************************************************/

/******************************************************************************/

/******************************************************************************/

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
	{"MENU_GRAPHICS_CROSSHAIR", "Crosshair"},
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
	{                   "CVHINT_CONFONT", "Font to draw the console with."},
	{              "CVHINT_CONMONOSPACE", "Draw the console font as monospaced."},
	{                  "CVHINT_CONSCALE", "Scales the text of the console (high resolution)."},
	{             "CVHINT_CONTESTSTRING", "Variable to test strings (escape/unescape)."},
	
	{					 "CVHINT_SVNAME", "The visible name of the server."},
	{					  "CVHINT_SVURL", "The web site for this server."},
	{				   "CVHINT_SVWADURL", "Location where WADs should be downloaded from."},
	{					  "CVHINT_SVIRC", "The IRC Channel for this server."},
	{					 "CVHINT_SVMOTD", "The message displayed to clients when joining."},
	{		  "CVHINT_SVCONNECTPASSWORD", "The password needed to connect to a server."},
	{			 "CVHINT_SVJOINPASSWORD", "The password needed to join the game."},
	
	{			 "CVHINT_RCUTWALLDETAIL", "Lower the detail of wall textures to reduce memory usage at the cost of quality."},
	
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
	{                      "DEP_DEATHMSG_SUICIDE", "$1 suicides\n"},
	{                     "DEP_DEATHMSG_TELEFRAG", "$1 was telefraged by $2\n"},
	{                         "DEP_DEATHMSG_FIST", "$1 was beaten to a pulp by $2\n"},
	{                          "DEP_DEATHMSG_GUN", "$1 was gunned by $2\n"},
	{                      "DEP_DEATHMSG_SHOTGUN", "$1 was shot down by $2\n"},
	{                      "DEP_DEATHMSG_MACHGUN", "$1 was machine-gunned by $2\n"},
	{                       "DEP_DEATHMSG_ROCKET", "$1 caught $2's rocket\n"},
	{                    "DEP_DEATHMSG_GIBROCKET", "$1 was gibbed by $2's rocket\n"},
	{                       "DEP_DEATHMSG_PLASMA", "$1 eats $2's toaster\n"},
	{                      "DEP_DEATHMSG_BFGBALL", "$1 enjoys $2's big fraggin' gun\n"},
	{                     "DEP_DEATHMSG_CHAINSAW", "$1 was divided up into little pieces by $2's chainsaw\n"},
	{                   "DEP_DEATHMSG_SUPSHOTGUN", "$1 ate 2 loads of $2's buckshot\n"},
	{                   "DEP_DEATHMSG_PLAYUNKNOW", "$1 was killed by $2\n"},
	{                    "DEP_DEATHMSG_HELLSLIME", "$1 dies in hellslime\n"},
	{                         "DEP_DEATHMSG_NUKE", "$1 gulped a load of nukage\n"},
	{                 "DEP_DEATHMSG_SUPHELLSLIME", "$1 dies in super hellslime/strobe hurt\n"},
	{                   "DEP_DEATHMSG_SPECUNKNOW", "$1 dies in special sector\n"},
	{                   "DEP_DEATHMSG_BARRELFRAG", "$1 was barrel-fragged by $2\n"},
	{                       "DEP_DEATHMSG_BARREL", "$1 dies from a barrel explosion\n"},
	{                    "DEP_DEATHMSG_POSSESSED", "$1 was shot by a possessed\n"},
	{                      "DEP_DEATHMSG_SHOTGUY", "$1 was shot down by a shotguy\n"},
	{                         "DEP_DEATHMSG_VILE", "$1 was blasted by an Arch-vile\n"},
	{                        "DEP_DEATHMSG_FATSO", "$1 was exploded by a Mancubus\n"},
	{                     "DEP_DEATHMSG_CHAINGUY", "$1 was punctured by a Chainguy\n"},
	{                        "DEP_DEATHMSG_TROOP", "$1 was fried by an Imp\n"},
	{                     "DEP_DEATHMSG_SERGEANT", "$1 was eviscerated by a Demon\n"},
	{                      "DEP_DEATHMSG_SHADOWS", "$1 was eaten by a Spectre\n"},
	{                         "DEP_DEATHMSG_HEAD", "$1 was fried by a Cacodemon\n"},
	{                      "DEP_DEATHMSG_BRUISER", "$1 was slain by a Baron of Hell\n"},
	{                       "DEP_DEATHMSG_UNDEAD", "$1 was smashed by a Revenant\n"},
	{                       "DEP_DEATHMSG_KNIGHT", "$1 was slain by a Hell-Knight\n"},
	{                        "DEP_DEATHMSG_SKULL", "$1 was ghosted by a Lost Soul\n"},
	{                       "DEP_DEATHMSG_SPIDER", "$1 was chaingunned by The Spider Mastermind\n"},
	{                         "DEP_DEATHMSG_BABY", "$1 was plasma'd by a Arachnotron\n"},
	{                       "DEP_DEATHMSG_CYBORG", "$1 was crushed by the Cyberdemon\n"},
	{                         "DEP_DEATHMSG_PAIN", "$1 was killed by a Pain Elemental\n"},
	{                       "DEP_DEATHMSG_WOLFSS", "$1 was killed by a WolfSS\n"},
	{                         "DEP_DEATHMSG_DEAD", "$1 died\n"},
	{                        "DEP_DEATHMSG_STAFF", "$1 was poked to death by $2's staff\n"},
	{                   "DEP_DEATHMSG_SUPERSTAFF", "$1 was electrocuted by $2's tomed staff\n"},
	{                    "DEP_DEATHMSG_GAUNTLETS", "$1 was tickled to death by $2's magical hands\n"},
	{               "DEP_DEATHMSG_SUPERGAUNTLETS", "$1 got soul sucked by $2's proton gloves\n"},
	{                         "DEP_DEATHMSG_WAND", "$1 got zapped by $2's elven wand\n"},
	{                    "DEP_DEATHMSG_SUPERWAND", "$1 got vaporized by $2's tomed elven wand\n"},
	{                     "DEP_DEATHMSG_CROSSBOW", "$1 got bolted by $2's crossbow\n"},
	{                "DEP_DEATHMSG_SUPERCROSSBOW", "$1 got shot by $2's tomed crossbow\n"},
	{                   "DEP_DEATHMSG_DRAGONCLAW", "$1 felt the claw of $2\n"},
	{              "DEP_DEATHMSG_SUPERDRAGONCLAW", "$1 was shredded by $2's tomed dragon claw\n"},
	{                    "DEP_DEATHMSG_HELLSTAFF", "$1 felt the wrath of $2's hellstaff\n"},
	{               "DEP_DEATHMSG_SUPERHELLSTAFF", "$1 was scalded by $2's tomed hellstaff\n"},
	{                "DEP_DEATHMSG_HELLSTAFFRAIN", "$1 got acid rained on by $2\n"},
	{                      "DEP_DEATHMSG_FIREROD", "$1 was blasted by $2's phoenix rod\n"},
	{                 "DEP_DEATHMSG_SUPERFIREROD", "$1 was burnt to a crisp by $2's tomed phoenix rod\n"},
	{                     "DEP_DEATHMSG_FIREMACE", "$1 felt $2's balls of steel\n"},
	{                "DEP_DEATHMSG_SUPERFIREMACE", "$1 was squished by $2's gigantic mace sphere\n"},
	{            "DEP_DEATHMSG_SUPERFIREMACESELF", "$1's gigantic mace sphere helped caused suicide\n"},
	{                         "DEP_DEATHMSG_BEAK", "$1 was pecked to death by $2\n"},
	{                    "DEP_DEATHMSG_SUPERBEAK", "$1 was pecked to death by a blessed $2\n"},
	{                      "DEP_DEATHMSG_DSPARIL", "$1 died\n"},
	{                       "DEP_DEATHMSG_WIZARD", "$1 died\n"},
	{                 "DEP_DEATHMSG_FIREGARGOYLE", "$1 died\n"},
	{                     "DEP_DEATHMSG_GARGOYLE", "$1 died\n"},
	{                        "DEP_DEATHMSG_GOLEM", "$1 died\n"},
	{                   "DEP_DEATHMSG_GHOSTGOLEM", "$1 died\n"},
	{                 "DEP_DEATHMSG_CHAOSSERPENT", "$1 died\n"},
	{                     "DEP_DEATHMSG_IRONLICH", "$1 died\n"},
	{                    "DEP_DEATHMSG_MAULOTAUR", "$1 died\n"},
	{                   "DEP_DEATHMSG_NITROGOLEM", "$1 died\n"},
	{              "DEP_DEATHMSG_GHOSTNITROGOLEM", "$1 died\n"},
	{                     "DEP_DEATHMSG_OPHIDIAN", "$1 died\n"},
	{                    "DEP_DEATHMSG_SABRECLAW", "$1 died\n"},
	{                "DEP_DEATHMSG_UNDEADWARRIOR", "$1 died\n"},
	{           "DEP_DEATHMSG_GHOSTUNDEADWARRIOR", "$1 died\n"},
	{                   "DEP_DEATHMSG_WEREDRAGON", "$1 died\n"},
	{                            "DEP_DOOM2TITLE", "DOOM 2: Hell on Earth"},
	{                            "DEP_DOOMUTITLE", "The Ultimate DOOM Startup"},
	{                             "DEP_DOOMTITLE", "DOOM Registered Startup"},
	{                            "DEP_DOOM1TITLE", "DOOM Shareware Startup"},
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

