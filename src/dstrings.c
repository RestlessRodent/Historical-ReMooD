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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:
//	  Globally defined strings.

#include "dstrings.h"
#include "z_zone.h"
#include "m_swap.h"

char *text[NUMTEXT] = {
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
		"\n" "To continue the DOOM experience, play\n"
		"The Shores of Hell and its amazing\n" "sequel, Inferno!\n",

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
		"you rappel down to  the surface of\n" "Hell.\n" "\n"
		"Now, it's on to the final chapter of\n" "DOOM! -- Inferno.",

	"The loathsome spiderdemon that\n" "masterminded the invasion of the moon\n"
		"bases and caused so much death has had\n"
		"its ass kicked for all time.\n" "\n"
		"A hidden doorway opens and you enter.\n"
		"You've proven too tough for Hell to\n"
		"contain, and now Hell at last plays\n"
		"fair -- for you emerge from the door\n"
		"to see the green fields of Earth!\n" "Home at last.\n" "\n"
		"You wonder what's been happening on\n"
		"Earth while you were battling evil\n"
		"unleashed. It's good that no Hell-\n"
		"spawn could have come through that\n" "door with you ...",

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
		"potential pain and gibbitude as a nation\n"
		"of demons run amok among our cities.\n" "\n" "next stop, hell on earth!",

	"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n"
		"STARPORT. BUT SOMETHING IS WRONG. THE\n"
		"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n"
		"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n"
		"IS BEING SUBVERTED BY THEIR PRESENCE.\n" "\n"
		"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n"
		"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n"
		"YOU CAN PENETRATE INTO THE HAUNTED HEART\n"
		"OF THE STARBASE AND FIND THE CONTROLLING\n"
		"SWITCH WHICH HOLDS EARTH'S POPULATION\n" "HOSTAGE.",

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
		"YOUR OWN HOME CITY, NOT FAR FROM THE\n"
		"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n" "UP AND RETURN TO THE FRAY.",

	"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
		"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
		"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
		"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
		"TEETH AND PLUNGE THROUGH IT.\n" "\n"
		"THERE MUST BE A WAY TO CLOSE IT ON THE\n"
		"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n" "GOT TO GO THROUGH HELL TO GET TO IT?",

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
		"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"
		"HOME. REBUILDING EARTH OUGHT TO BE A\n" "LOT MORE FUN THAN RUINING IT WAS.\n",

	"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
		"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"
		"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"
		"WHO THE INMATES OF THIS CORNER OF HELL\n" "WILL BE.",

	"CONGRATULATIONS, YOU'VE FOUND THE\n" "SUPER SECRET LEVEL!  YOU'D BETTER\n"
		"BLAZE THROUGH THIS ONE!\n",

	"You've fought your way out of the infested\n"
		"experimental labs.   It seems that UAC has\n"
		"once again gulped it down.  With their\n"
		"high turnover, it must be hard for poor\n"
		"old UAC to buy corporate health insurance\n" "nowadays..\n" "\n"
		"Ahead lies the military complex, now\n"
		"swarming with diseased horrors hot to get\n"
		"their teeth into you. With luck, the\n"
		"complex still has some warlike ordnance\n" "laying around.",

	"You hear the grinding of heavy machinery\n"
		"ahead.  You sure hope they're not stamping\n"
		"out new hellspawn, but you're ready to\n"
		"ream out a whole herd if you have to.\n"
		"They might be planning a blood feast, but\n"
		"you feel about as mean as two thousand\n"
		"maniacs packed into one mad killer.\n" "\n" "You don't plan to go down easy.",

	"The vista opening ahead looks real damn\n"
		"familiar. Smells familiar, too -- like\n"
		"fried excrement. You didn't like this\n"
		"place before, and you sure as hell ain't\n"
		"planning to like it now. The more you\n"
		"brood on it, the madder you get.\n"
		"Hefting your gun, an evil grin trickles\n" "onto your face. Time to take some names.",

	"Suddenly, all is silent, from one horizon\n"
		"to the other. The agonizing echo of Hell\n"
		"fades away, the nightmare sky turns to\n"
		"blue, the heaps of monster corpses start \n"
		"to evaporate along with the evil stench \n"
		"that filled the air. Jeeze, maybe you've\n"
		"done it. Have you really won?\n" "\n"
		"Something rumbles in the distance.\n"
		"A blue light begins to glow inside the\n" "ruined skull of the demon-spitter.",

	"What now? Looks totally different. Kind\n"
		"of like King Tut's condo. Well,\n"
		"whatever's here can't be any worse\n"
		"than usual. Can it?  Or maybe it's best\n" "to let sleeping gods lie..",

	"Time for a vacation. You've burst the\n"
		"bowels of hell and by golly you're ready\n"
		"for a break. You mutter to yourself,\n"
		"Maybe someone else can kick Hell's ass\n"
		"next time around. Ahead lies a quiet town,\n"
		"with peaceful flowing water, quaint\n"
		"buildings, and presumably no Hellspawn.\n" "\n"
		"As you step off the transport, you hear\n" "the stomp of a cyberdemon's iron shoe.",

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
		"					  press enter to continue\n"
		"===========================================================================\n",

	"===========================================================================\n"
		"					 This program is Free Software!\n"
		"===========================================================================\n",

	"===========================================================================\n"
		"					 This program is Free Software!\n"
		"			 See the terms of the GNU General Public License\n"
		"===========================================================================\n",

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
		"more horrific beings through this\n" "gate? the sweep of a crystal dome\n"
		"overhead where the sky should be is\n" "certainly not a good sign....",

// Heretic E3TEXT
	"the death of d'sparil has loosed\n" "the magical bonds holding his\n"
		"creatures on this plane, their\n" "dying screams overwhelming his own\n"
		"cries of agony.\n\n" "your oath of vengeance fulfilled,\n"
		"you enter the portal to your own\n" "world, mere moments before the dome\n"
		"shatters into a million pieces.\n\n" "but if d'sparil's power is broken\n"
		"forever, why don't you feel safe?\n" "was it that last shout just before\n"
		"his death, the one that sounded\n" "like a curse? or a summoning? you\n"
		"can't really be sure, but it might\n" "just have been a scream.\n\n"
		"then again, what about the other\n" "serpent riders?",

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
		"in the darkest corner of d'sparil's\n" "demesne, his personal bodyguards\n"
		"await your arrival ...",

// Heretic E5TEXT
	"as the final maulotaur bellows his\n" "death-agony, you realize that you\n"
		"have never come so close to your own\n" "destruction. not even the fight with\n"
		"d'sparil and his disciples had been\n" "this desperate. grimly you stare at\n"
		"the gates which open before you,\n" "wondering if they lead home, or if\n"
		"they open onto some undreamed-of\n" "horror. you find yourself wondering\n"
		"if you have the strength to go on,\n" "if nothing but death and pain await\n"
		"you. but what else can you do, if\n" "the will to fight is gone? can you\n"
		"force yourself to continue in the\n" "face of such despair? do you have\n"
		"the courage? you find, in the end,\n" "that it is not within you to\n"
		"surrender without a fight. eyes\n" "wide, you go to meet your fate.",

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
	
	"%s was poked to death by %s's staff\n",				// DEATHMSG_STAFF
	"%s was electrocuted by %s's tomed staff\n",			// DEATHMSG_SUPERSTAFF
	"%s was tickled to death by %s's magical hands\n",		// DEATHMSG_GAUNTLETS
	"%s got soul sucked by %s's proton gloves\n",			// DEATHMSG_SUPERGAUNTLETS
	"%s got zapped by %s's elven wand\n",					// DEATHMSG_WAND
	"%s got vaporized by %s's tomed elven wand\n",			// DEATHMSG_SUPERWAND
	"%s got bolted by %s's crossbow\n",						// DEATHMSG_CROSSBOW
	"%s got shot by %s's tomed crossbow\n",					// DEATHMSG_SUPERCROSSBOW
	"%s felt the claw of %s\n",								// DEATHMSG_DRAGONCLAW
	"%s was shredded by %s's tomed dragon claw\n",			// DEATHMSG_SUPERDRAGONCLAW
	"%s felt the wrath of %s's hellstaff\n",				// DEATHMSG_HELLSTAFF
	"%s was scalded by %s's tomed hellstaff\n",				// DEATHMSG_SUPERHELLSTAFF
	"%s got acid rained on by %s\n",						// DEATHMSG_HELLSTAFFRAIN
	"%s was blasted by %s's phoenix rod\n",					// DEATHMSG_FIREROD
	"%s was burnt to a crisp by %s's tomed phoenix rod\n",	// DEATHMSG_SUPERFIREROD
	"%s felt %s's balls of steel\n",						// DEATHMSG_FIREMACE
	"%s was squished by %s's gigantic mace sphere\n",		// DEATHMSG_SUPERFIREMACE
	"%s's gigantic mace sphere helped cause suicide\n",		// DEATHMSG_SUPERFIREMACESELF
	"%s was pecked to death by %s\n",						// DEATHMSG_BEAK
	"%s was pecked to death by a blessed %s\n",				// DEATHMSG_SUPERBEAK
	
	"%s died\n",											// DEATHMSG_DSPARIL
	"%s died\n",											// DEATHMSG_WIZARD
	"%s died\n",											// DEATHMSG_FIREGARGOYLE
	"%s died\n",											// DEATHMSG_GARGOYLE
	"%s died\n",											// DEATHMSG_GOLEM
	"%s died\n",											// DEATHMSG_GHOSTGOLEM
	"%s died\n",											// DEATHMSG_CHAOSSERPENT
	"%s died\n",											// DEATHMSG_IRONLICH
	"%s died\n",											// DEATHMSG_MAULOTAUR
	"%s died\n",											// DEATHMSG_NITROGOLEM
	"%s died\n",											// DEATHMSG_GHOSTNITROGOLEM
	"%s died\n",											// DEATHMSG_OPHIDIAN
	"%s died\n",											// DEATHMSG_SABRECLAW
	"%s died\n",											// DEATHMSG_UNDEADWARRIOR
	"%s died\n",											// DEATHMSG_GHOSTUNDEADWARRIOR
	"%s died\n",											// DEATHMSG_WEREDRAGON

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
	{							 "MENU_NULLSPACE", L" "},
	{						  "MENU_MAIN_NEWGAME", L"New Game"},
	{						  "MENU_MAIN_ENDGAME", L"End Game"},
	{						 "MENU_MAIN_LOADGAME", L"Load Game"},
	{						 "MENU_MAIN_SAVEGAME", L"Save Game"},
	{						  "MENU_MAIN_OPTIONS", L"Options"},
	{						 "MENU_MAIN_PROFILES", L"Profiles"},
	{						 "MENU_MAIN_QUITGAME", L"Quit Game"},
	{						 "MENU_OPTIONS_TITLE", L"Options"},
	{				  "MENU_OPTIONS_GAMESETTINGS", L"Game Settings..."},
	{			   "MENU_OPTIONS_CONTROLSETTINGS", L"Control Settings..."},
	{			 "MENU_OPTIONS_GRAPHICALSETTINGS", L"Graphical Settings..."},
	{				 "MENU_OPTIONS_AUDIOSETTINGS", L"Audio Settings..."},
	{			  "MENU_OPTIONS_ADVANCEDSETTINGS", L"Advanced Settings..."},
	{	   "MENU_OPTIONS_DISABLETITLESCREENDEMOS", L"Disable Title Screen Demos"},
	{						"MENU_CONTROLS_TITLE", L"Control Settings"},
	{			  "MENU_CONTROLS_CONTROLSUBTITLE", L"*** CONTROLS ***"},
	{				"MENU_CONTROLS_ACTIONSPERKEY", L"Max actions per key"},
	{			"MENU_CONTROLS_PLAYERONECONTROLS", L"Default Controls (Player 1)..."},
	{			"MENU_CONTROLS_PLAYERTWOCONTROLS", L"Default Controls (Player 2)..."},
	{		  "MENU_CONTROLS_PLAYERTHREECONTROLS", L"Default Controls (Player 3)..."},
	{		   "MENU_CONTROLS_PLAYERFOURCONTROLS", L"Default Controls (Player 4)..."},
	{				"MENU_CONTROLS_MOUSESUBTITLE", L"*** MOUSE ***"},
	{				  "MENU_CONTROLS_ENABLEMOUSE", L"Enable Mouse"},
	{				"MENU_CONTROLS_ADVANCEDMOUSE", L"Use advanced mouse settings"},
	{	 "MENU_CONTROLS_BASICSETTINGSSUBSUBTITLE", L">> Basic Settings"},
	{			 "MENU_CONTROLS_USEMOUSEFREELOOK", L"Use mouse to freelook"},
	{				 "MENU_CONTROLS_USEMOUSEMOVE", L"Use mouse to move"},
	{				  "MENU_CONTROLS_INVERTYAXIS", L"Invert Y-Axis"},
	{				 "MENU_CONTROLS_MOVETURNSENS", L"Move/Turn sensitivity"},
	{			   "MENU_CONTROLS_LOOKUPDOWNSENS", L"Look Up/Down sensitivity"},
	{   "MENU_CONTROLS_ADVANCEDSETTINGSUBSUBTITLE", L">> Advanced Settings"},
	{					"MENU_CONTROLS_XAXISSENS", L"X-Axis sensitivity"},
	{					"MENU_CONTROLS_YAXISSENS", L"Y-Axis sensitivity"},
	{					"MENU_CONTROLS_XAXISMOVE", L"X-Axis Movement"},
	{					"MENU_CONTROLS_YAXISMOVE", L"Y-Axis Movement"},
	{		   "MENU_CONTROLS_XAXISMOVESECONDARY", L"X-Axis Movement (Secondary)"},
	{		   "MENU_CONTROLS_YAXISMOVESECONDARY", L"Y-Axis Movement (Secondary)"},
	{		"MENU_CONTROLS_STRAFEKEYUSESECONDARY", L"Strafe key uses secondary movement"},
	{						"MENU_GRAPHICS_TITLE", L"Graphical Settings"},
	{			   "MENU_GRAPHICS_SCREENSUBTITLE", L"*** SCREEN ***"},
	{				"MENU_GRAPHICS_SETRESOLUTION", L"Set Resolution..."},
	{				   "MENU_GRAPHICS_FULLSCREEN", L"Fullscreen"},
	{				   "MENU_GRAPHICS_BRIGHTNESS", L"Brightness"},
	{				   "MENU_GRAPHICS_SCREENSIZE", L"Screen Size"},
	{				   "MENU_GRAPHICS_SCREENLINK", L"Screen Link"},
	{			 "MENU_GRAPHICS_RENDERERSUBTITLE", L"*** RENDERER ***"},
	{				 "MENU_GRAPHICS_TRANSLUCENCY", L"Translucency"},
	{				 "MENU_GRAPHICS_ENABLEDECALS", L"Enable decals"},
	{					"MENU_GRAPHICS_MAXDECALS", L"Max decales"},
	{			  "MENU_GRAPHICS_CONSOLESUBTITLE", L"*** CONSOLE ***"},
	{				 "MENU_GRAPHICS_CONSOLESPEED", L"Console Speed"},
	{				"MENU_GRAPHICS_CONSOLEHEIGHT", L"Console Height"},
	{			"MENU_GRAPHICS_CONSOLEBACKGROUND", L"Console Background"},
	{			  "MENU_GRAPHICS_MESSAGEDURATION", L"Message Duration"},
	{				 "MENU_GRAPHICS_ECHOMESSAGES", L"Echo Messages"},
	{				 "MENU_GRAPHICS_MENUSUBTITLE", L"*** MENU ***"},
	{		  "MENU_GRAPHICS_CURSORBLINKDURATION", L"Cursor Blink Duration"},
	{				  "MENU_GRAPHICS_HUDSUBTITLE", L"*** HUD ***"},
	{			   "MENU_GRAPHICS_SCALESTATUSBAR", L"Scale Status Bar"},
	{		 "MENU_GRAPHICS_TRANSPARENTSTATUSBAR", L"Transparent Status Bar"},
	{  "MENU_GRAPHICS_STATUSBARTRANSPARENCYAMOUNT", L"Status Bar Transparency Level"},
	{					"MENU_GRAPHICS_CROSSHAIR", L"Crosshair"},
	{						"MENU_KEYBINDS_TITLE", L"Setup Controls"},
	{			 "MENU_KEYBINDS_MOVEMENTSUBTITLE", L"*** MOVEMENT ***"},
	{						 "MENU_KEYBINDS_FIRE", L"Fire"},
	{					 "MENU_KEYBINDS_ACTIVATE", L"Activate"},
	{				 "MENU_KEYBINDS_MOVEFORWARDS", L"Move Forwards"},
	{				"MENU_KEYBINDS_MOVEBACKWARDS", L"Move Backwards"},
	{					 "MENU_KEYBINDS_TURNLEFT", L"Turn Left"},
	{					"MENU_KEYBINDS_TURNRIGHT", L"Turn Right"},
	{						  "MENU_KEYBINDS_RUN", L"Run"},
	{					 "MENU_KEYBINDS_STRAFEON", L"Strafe On"},
	{				   "MENU_KEYBINDS_STRAFELEFT", L"Strafe Left"},
	{				  "MENU_KEYBINDS_STRAFERIGHT", L"Strafe Right"},
	{					   "MENU_KEYBINDS_LOOKUP", L"Look Up"},
	{					 "MENU_KEYBINDS_LOOKDOWN", L"Look Down"},
	{				   "MENU_KEYBINDS_CENTERVIEW", L"Center View"},
	{					"MENU_KEYBINDS_MOUSELOOK", L"Mouselook"},
	{					"MENU_KEYBINDS_JUMPFLYUP", L"Jump/Fly Up"},
	{					  "MENU_KEYBINDS_FLYDOWN", L"Fly down"},
	{	  "MENU_KEYBINDS_WEAPONSANDITEMSSUBTITLE", L"*** WEAPONS & ITEMS ***"},
	{					  "MENU_KEYBINDS_SLOTONE", L"Fist/Chainsaw/Staff/Gauntlets"},
	{					  "MENU_KEYBINDS_SLOTTWO", L"Pistol/Wand"},
	{					"MENU_KEYBINDS_SLOTTHREE", L"Shotgun/Super Shotgun/Crossbow"},
	{					 "MENU_KEYBINDS_SLOTFOUR", L"Chaingun/Dragon Claw"},
	{					 "MENU_KEYBINDS_SLOTFIVE", L"Rocket Launcher/Hellstaff"},
	{					  "MENU_KEYBINDS_SLOTSIX", L"Plasma rifle/Phoenix Rod"},
	{					"MENU_KEYBINDS_SLOTSEVEN", L"BFG/Firemace"},
	{					"MENU_KEYBINDS_SLOTEIGHT", L"Chainsaw"},
	{			   "MENU_KEYBINDS_PREVIOUSWEAPON", L"Previous Weapon"},
	{				   "MENU_KEYBINDS_NEXTWEAPON", L"Next Weapon"},
	{				   "MENU_KEYBINDS_BESTWEAPON", L"Best Weapon"},
	{				"MENU_KEYBINDS_INVENTORYLEFT", L"Inventory Left"},
	{			   "MENU_KEYBINDS_INVENTORYRIGHT", L"Inventory Right"},
	{				 "MENU_KEYBINDS_INVENTORYUSE", L"Inventory Use"},
	{				 "MENU_KEYBINDS_MISCSUBTITLE", L"*** MISC ***"},
	{					  "MENU_KEYBINDS_TALKKEY", L"Talk key"},
	{			"MENU_KEYBINDS_RANKINGSANDSCORES", L"Rankings/Scores"},
	{				"MENU_KEYBINDS_TOGGLECONSOLE", L"Toggle Console"},
	{						   "MENU_AUDIO_TITLE", L"Audio Settings"},
	{				  "MENU_AUDIO_OUTPUTSUBTITLE", L"*** OUTPUT ***"},
	{					 "MENU_AUDIO_SOUNDOUTPUT", L"Sound Output"},
	{					 "MENU_AUDIO_SOUNDDEVICE", L"Sound Device"},
	{					 "MENU_AUDIO_MUSICOUTPUT", L"Music Output"},
	{					 "MENU_AUDIO_MUSICDEVICE", L"Music Device"},
	{				 "MENU_AUDIO_QUALITYSUBTITLE", L"*** QUALITY ***"},
	{					"MENU_AUDIO_SPEAKERSETUP", L"Speaker Setup"},
	{				"MENU_AUDIO_SAMPLESPERSECOND", L"Samples per second"},
	{				   "MENU_AUDIO_BITSPERSAMPLE", L"Bits per sample"},
	{		   "MENU_AUDIO_FAKEPCSPEAKERWAVEFORM", L"Simulated PC Speaker Waveform"},
	{				  "MENU_AUDIO_VOLUMESUBTITLE", L"*** VOLUME ***"},
	{					 "MENU_AUDIO_SOUNDVOLUME", L"Sound Volume"},
	{					 "MENU_AUDIO_MUSICVOLUME", L"Music Volume"},
	{					"MENU_AUDIO_MISCSUBTITLE", L"*** MISC SETTINGS ***"},
	{				  "MENU_AUDIO_PRECACHESOUNDS", L"Precache Sounds"},
	{				"MENU_AUDIO_RANDOMSOUNDPITCH", L"Random Sound Pitch"},
	{				   "MENU_AUDIO_SOUNDCHANNELS", L"Sound Channels"},
	{		   "MENU_AUDIO_RESERVEDSOUNDCHANNELS", L"Reserved Sound Channels"},
	{			  "MENU_AUDIO_MULTITHREADEDSOUND", L"Multi-Threaded Sound"},
	{			  "MENU_AUDIO_MULTITHREADEDMUSIC", L"Multi-Threaded Music"},
	{				   "MENU_AUDIO_RESETSUBTITLE", L"*** RESET ***"},
	{					  "MENU_AUDIO_RESETSOUND", L"Reset Sound"},
	{					  "MENU_AUDIO_RESETMUSIC", L"Reset Music"},
	{						   "MENU_VIDEO_TITLE", L"Video Resolution"},
	{					  "MENU_VIDEO_MODESELECT", L"Select Mode (Press 'd' to set as default)"},
	{							"MENU_GAME_TITLE", L"Game Options"},
	{			  "MENU_GAME_MULTIPLAYERSUBTITLE", L"*** MULTIPLAYER ***"},
	{				   "MENU_GAME_DEATHMATCHTYPE", L"Deathmatch Type"},
	{						"MENU_GAME_FRAGLIMIT", L"Fraglimit"},
	{						"MENU_GAME_TIMELIMIT", L"Timelimit"},
	{					 "MENU_GAME_TEAMSUBTITLE", L"*** TEAM ***"},
	{				   "MENU_GAME_ENABLETEAMPLAY", L"Enable Teamplay"},
	{					 "MENU_GAME_FRIENDLYFIRE", L"Friendly Fire"},
	{			 "MENU_GAME_RESTRICTIONSSUBTITLE", L"*** RESTRICTIONS ***"},
	{						"MENU_GAME_ALLOWJUMP", L"Allow Jump"},
	{				  "MENU_GAME_ALLOWROCKETJUMP", L"Allow Rocket Jump"},
	{					 "MENU_GAME_ALLOWAUTOAIM", L"Allow autoaim"},
	{					   "MENU_GAME_ALLOWTURBO", L"Allow turbo"},
	{				   "MENU_GAME_ALLOWEXITLEVEL", L"Allow exitlevel"},
	{					 "MENU_GAME_FORCEAUTOAIM", L"Force Autoaim"},
	{		  "MENU_GAME_WEAPONSANDITEMSSUBTITLE", L"*** WEAPONS & ITEMS ***"},
	{				"MENU_GAME_ENABLEITEMRESPAWN", L"Enable Item Respawn"},
	{				  "MENU_GAME_ITEMRESPAWNTIME", L"Item Respawn time"},
	{			"MENU_GAME_DROPWEAPONSWHENYOUDIE", L"Drop Weapons when you die"},
	{					 "MENU_GAME_INFINITEAMMO", L"Infinite Ammo"},
	{				 "MENU_GAME_MONSTERSSUBTITLE", L"*** MONSTERS ***"},
	{					"MENU_GAME_SPAWNMONSTERS", L"Spawn Monsters"},
	{			 "MENU_GAME_ENABLEMONSTERRESPAWN", L"Enable Monster Respawn"},
	{			   "MENU_GAME_MONSTERRESPAWNTIME", L"Monster Respawn time"},
	{					 "MENU_GAME_FASTMONSTERS", L"Fast Monsters"},
	{			   "MENU_GAME_PREDICTINGMONSTERS", L"Predicting Monsters"},
	{					 "MENU_GAME_MISCSUBTITLE", L"*** MISC ***"},
	{						  "MENU_GAME_GRAVITY", L"Gravity"},
	{					 "MENU_GAME_SOLIDCORPSES", L"Solid corpses"},
	{						"MENU_GAME_BLOODTIME", L"BloodTime"},
	{			"MENU_GAME_COMPATIBILITYSUBTITLE", L"*** COMPATIBILITY ***"},
	{					 "MENU_GAME_CLASSICBLOOD", L"Classic Blood"},
	{		  "MENU_GAME_CLASSICROCKETEXPLOSIONS", L"Classic Rocket Explosions"},
	{		 "MENU_GAME_CLASSICMONSTERMELEERANGE", L"Classic Monster Melee Range"},
	{			  "MENU_GAME_CLASSICMONSTERLOGIC", L"Classic Monster Logic"},
	{						 "MENU_NEWGAME_TITLE", L"New Game"},
	{		  "MENU_NEWGAME_SINGLEPLAYERSUBTITLE", L"*** SINGLE PLAYER ***"},
	{					   "MENU_NEWGAME_CLASSIC", L"Classic..."},
	{					"MENU_NEWGAME_CREATEGAME", L"Create Game..."},
	{					"MENU_NEWGAME_QUICKSTART", L"Quick Start..."},
	{		   "MENU_NEWGAME_MULTIPLAYERSUBTITLE", L"*** MULTI PLAYER ***"},
	{			   "MENU_NEWGAME_SPLITSCREENGAME", L"Split Screen Game..."},
	{			"MENU_NEWGAME_UDPLANINTERNETGAME", L"UDP LAN/Internet Game..."},
	{			"MENU_NEWGAME_TCPLANINTERNETGAME", L"TCP LAN/Internet Game..."},
	{					 "MENU_NEWGAME_MODEMGAME", L"Modem Game..."},
	{		   "MENU_NEWGAME_SERIALNULLMODEMGAME", L"Serial/Null-Modem Game..."},
	{					  "MENU_NEWGAME_FORKGAME", L"Fork Game..."},
	{					 "MENU_CLASSICGAME_TITLE", L"Classic Game"},
	{				"MENU_CLASSICGAME_DOOMSKILLA", L"I'm too young to die"},
	{				"MENU_CLASSICGAME_DOOMSKILLB", L"Hey, not too rough"},
	{				"MENU_CLASSICGAME_DOOMSKILLC", L"Hurt me plenty"},
	{				"MENU_CLASSICGAME_DOOMSKILLD", L"Ultra-violence"},
	{				"MENU_CLASSICGAME_DOOMSKILLE", L"Nightmare!"},
	{			 "MENU_CLASSICGAME_HERETICSKILLA", L"Thou needeth a wet-nurse"},
	{			 "MENU_CLASSICGAME_HERETICSKILLB", L"Yellowbellies-r-us"},
	{			 "MENU_CLASSICGAME_HERETICSKILLC", L"Bringest them oneth"},
	{			 "MENU_CLASSICGAME_HERETICSKILLD", L"Thou art a smite-meister"},
	{			 "MENU_CLASSICGAME_HERETICSKILLE", L"Black plague possesses thee"},
	{			  "MENU_CLASSICGAME_DOOMEPISODEA", L"Knee-Deep In The Dead"},
	{			  "MENU_CLASSICGAME_DOOMEPISODEB", L"The Shores of Hell"},
	{			  "MENU_CLASSICGAME_DOOMEPISODEC", L"Inferno"},
	{			  "MENU_CLASSICGAME_DOOMEPISODED", L"Thy Flesh Consumed"},
	{			  "MENU_CLASSICGAME_DOOMEPISODEE", L"Episode 5"},
	{			  "MENU_CLASSICGAME_DOOMEPISODEF", L"Episode 6"},
	{		   "MENU_CLASSICGAME_HERETICEPISODEA", L"City of the Damned"},
	{		   "MENU_CLASSICGAME_HERETICEPISODEB", L"Hell's Maw"},
	{		   "MENU_CLASSICGAME_HERETICEPISODEC", L"The Dome of D'Sparil"},
	{		   "MENU_CLASSICGAME_HERETICEPISODED", L"The Ossuary"},
	{		   "MENU_CLASSICGAME_HERETICEPISODEE", L"The Stagnant Demesne"},
	{		   "MENU_CLASSICGAME_HERETICEPISODEF", L"Fate's Path"},
	{				  "MENU_CREATEGAME_SOLOTITLE", L"Create Game"},
	{				 "MENU_CREATEGAME_LOCALTITLE", L"Create Local Game"},
	{					  "MENU_CREATEGAME_LEVEL", L"Level"},
	{					  "MENU_CREATEGAME_SKILL", L"Skill"},
	{			  "MENU_CREATEGAME_SPAWNMONSTERS", L"Spawn Monsters"},
	{					"MENU_CREATEGAME_OPTIONS", L"Options"},
	{			   "MENU_CREATEGAME_SETUPOPTIONS", L"Setup Options..."},
	{				  "MENU_CREATEGAME_STARTGAME", L"Start Game!"},
	{			"MENU_CREATEGAME_NUMBEROFPLAYERS", L"Number of Players"},
	{			 "MENU_CREATEGAME_DEATHMATCHTYPE", L"Deathmatch Type"},
	{				  "MENU_CREATEGAME_FRAGLIMIT", L"Fraglimit"},
	{				  "MENU_CREATEGAME_TIMELIMIT", L"Timelimit"},
	{						"MENU_PROFILES_TITLE", L"Setup Profiles"},
	{				"MENU_PROFILES_CREATEPROFILE", L"Create Profile..."},
	{			   "MENU_PROFILES_CURRENTPROFILE", L"Current Profile"},
	{						 "MENU_PROFILES_NAME", L"Name"},
	{						"MENU_PROFILES_COLOR", L"Color"},
	{						 "MENU_PROFILES_SKIN", L"Skin"},
	{					  "MENU_PROFILES_AUTOAIM", L"Autoaim"},
	{				   "MENU_CREATEPROFILE_TITLE", L"Create Profile"},
	{			  "MENU_CREATEPROFILE_PLEASENAME", L"Please name your profile."},
	{					"MENU_CREATEPROFILE_NAME", L"Name"},
	{				  "MENU_CREATEPROFILE_ACCEPT", L"Accept"},
	{				   "MENU_SELECTPROFILE_TITLE", L"Select Profile"},
	{			"MENU_SELECTPROFILE_PLEASESELECT", L"Please select a profile to use"},
	{			 "MENU_SELECTPROFILE_PLACEHOLDER", L"[You]"},
	{				  "MENU_SELECTPROFILE_FORYOU", L"for you."},
	{			   "MENU_SELECTPROFILE_TWOSPLITA", L"for Player 1 (Top Screen)."},
	{			   "MENU_SELECTPROFILE_TWOSPLITB", L"for Player 2 (Bottom Screen)."},
	{			  "MENU_SELECTPROFILE_FOURSPLITA", L"for Player 1 (Top-Left Screen)."},
	{			  "MENU_SELECTPROFILE_FOURSPLITB", L"for Player 2 (Top-Right Screen)."},
	{			  "MENU_SELECTPROFILE_FOURSPLITC", L"for Player 3 (Bottom-Left Screen)."},
	{			  "MENU_SELECTPROFILE_FOURSPLITD", L"for Player 4 (Bottom-Right Screen)."},
	{				 "MENU_SELECTPROFILE_PROFILE", L"Profile"},
	{				  "MENU_SELECTPROFILE_ACCEPT", L"Accept"},
	{						  "MENU_OTHER_RANDOM", L"Random"},
	{				  "MENU_OTHER_RANDOMEPISODEA", L"Random (Episode 1)"},
	{				  "MENU_OTHER_RANDOMEPISODEB", L"Random (Episode 2)"},
	{				  "MENU_OTHER_RANDOMEPISODEC", L"Random (Episode 3)"},
	{				  "MENU_OTHER_RANDOMEPISODED", L"Random (Episode 4)"},
	{				  "MENU_OTHER_RANDOMEPISODEE", L"Random (Episode 5)"},
	{				  "MENU_OTHER_RANDOMEPISODEF", L"Random (Episode 6)"},
	{				   "MENU_OTHER_CHANGECONTROL", L"Hit the new key for\n%s\nESC for Cancel"},
	{				 "MENU_OTHER_PLAYERACONTROLS", L"Player 1's Controls"},
	{				 "MENU_OTHER_PLAYERBCONTROLS", L"Player 2's Controls"},
	{				 "MENU_OTHER_PLAYERCCONTROLS", L"Player 3's Controls"},
	{				 "MENU_OTHER_PLAYERDCONTROLS", L"Player 4's Controls"},
	{				   "MENU_OPTIONS_BINARYSAVES", L"Binary Saves"},
};

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* Unicode/ASCII Control code controlling */
// When converting to/from ASCII and Unicode and to self, convert control codes
// if necessary (ASCII configs can't have Unicode in them for example)
// When sourcing from Unicode, prefix \ with ^ ("^\n"L)
// When sourcing from ASCII, just use \ ("\n"L)
//   Convert ASCII "\a" to Nothing						-- Ignore
//   Convert ASCII "\b" to Nothing						-- Ignore
//   Convert ASCII "\f" to Nothing						-- Ignore
//   Convert ASCII "\v" to Nothing						-- Ignore
//   Convert ASCII "\xhhhh" to Unicode [0xhhhh]			-- Unicode Character
//   Convert ASCII "\chhhh" to Unicode [0x0012 0xhhhh]	-- Color and Formatting (Reserved)
//   Convert ASCII "\n" to Unicode [LF]					-- New line
//   Convert ASCII "\r" to Unicode [LF]					-- Drop CR if last character was \n
//   Convert ASCII "\t" to Unicode [TAB]				-- Tabulation
//   Convert ASCII "\yhh" to Unicode [0x0012 0x????]	-- Simple Color (Reserved)
//			Just \xHHHH for now

/*** UNICODE ***/
#if defined(EMULATEWCHARFUNCTIONS)
/* UNICODE_StringLength() -- Emulated UTF-16 string length */
size_t UNICODE_StringLength(wchar_t* String)
{
	size_t i = 0;
	wchar_t* Cur = String;
	
	while (*Cur)
	{
		i++;
		Cur++;
	}
	
	return i;
}
#endif

static int QuickCheckA(char in)
{
	if (in >= '0' && in <= '9')
		return (in - '0');
	else if (in >= 'A' && in <= 'F')
		return (in - 'A') + 10;
	else if (in >= 'a' && in <= 'f')
		return (in - 'a') + 10;
	else
		return 0;
}

/* UNICODE_ASCIILengthForUnicode() -- Checks out ASCII String, returns correct size (in div 2s) of Unicode variant */
size_t UNICODE_ASCIILengthForUnicode(uint8_t* InData, size_t InSize)
{
	size_t retval = 0;
	int i;
	char* x = NULL;
	wchar_t wc;
	
	if (!InData || !InSize)
		return 0;
	
	for (i = 0, x = InData; i < InSize && *x; i++, x++)
		switch (*x)
		{
			case '\\':		/* Control Specifier */
				if ((i + 1) < InSize)
					switch (*(x+1))
					{
						case 'x':
							if ((i + 5) < InSize)
							{
								//wc = QuickCheckA(*(x+2)) | QuickCheckA(*(x+3)) | QuickCheckA(*(x+4)) | QuickCheckA(*x(x+5));
								
								i += 5;
								x += 5;
							}
							else
								break;
							break;
						
						case '\\':
							if ((i + 1) < InSize)
							{
								retval++;
								i++;
								x++;
							}
							
							retval++;
						default:
							break;
					}
				retval++;
				break;
			
			default:
				retval++;
				break;
		}
	
	return retval;
}

/* UNICODE_UnicodeLengthForASCII() -- Checks out Unicode size and returns correct size of ASCII variant */
size_t UNICODE_UnicodeLengthForASCII(wchar_t* InData, size_t InSize)
{
	size_t retval = 0;
	int i;
	wchar_t* x = NULL;
	
	if (!InData || !InSize)
		return 0;
	
	for (i = 0, x = InData; i < InSize && *x; i++, x++)
		switch (*x)
		{
			default:
				if (*x <= 255)	// Normal Character
					retval++;
				else			// Unicode Character	"\xHHHH"
					retval += 6;
				break;
		}
	
	return retval;
}

/* UNICODE_ASCIIToUnicode() -- Converts ASCII to Unicode */
boolean UNICODE_ASCIIToUnicode(char* InData, size_t InSize, wchar_t** OutData, size_t* OutSize)
{
	int i;
	wchar_t wc;
	char* x = NULL;
	wchar_t* y = NULL;
	
	if (!InData || !OutData || !OutSize || InSize == 0)
		return false;
		
	// Set OutSize and allocate
	*OutSize = UNICODE_ASCIILengthForUnicode(InData, InSize) + 1;
	*OutData = Z_Malloc(sizeof(wchar_t) * *OutSize, PU_STATIC, NULL);
	
	// Convert
	for (i = 0, x = InData, y = *OutData; i < InSize && *x; i++, x++, y++)
		switch (*x)
		{
			case '\\':		/* Control Specifier */
				if ((i + 1) < (InSize - 1) && *(x+1))
				{
					switch (*(x+1))
					{
						case 'x':
							if ((i + 5) < InSize)
							{
								wc = (QuickCheckA(*(x+2)) << 12) |
									(QuickCheckA(*(x+3)) << 8) |
									(QuickCheckA(*(x+4)) << 4) |
									QuickCheckA(*(x+5));
								
								i += 5;
								x += 5;
								
								*y = wc;
							}
							else
								*y = *x;
							break;
						
						case '\\':
							if ((i + 1) < InSize)
							{
								*y = *x;
								i++;
								x++;
								y++;
							}
							
							*y = *x;
							break;
							
						default:
							*y = *x;
							break;
					}
				}
				else
					*y = *x;
				break;

			
			default:
				*y = *x;
				break;
		}
	
	*y = 0;
		
	return true;
}

/* UNICODE_UnicodeToASCII() -- Converts Unicode to ASCII */
boolean UNICODE_UnicodeToASCII(wchar_t* InData, size_t InSize, char** OutData, size_t* OutSize)
{
	if (!InData || !OutData || !OutSize || InSize == 0)
		return false;
		
	return true;
}

/* UNICODE_ConvertFile() -- Converts an entire file of one format to an entire file of another format */
// UT_AUTO (in only)
// UT_ASCII, UT_UTF8, UT_UTF16, UT_UTF32 (in, out)
// UT_WCHART (out only)
// When converting from ASCII and UTF8: Convert ReMooD \xhhhh to a Unicode Character
// When converting UTF* to ASCII: Create ReMooD \xhhhh in ASCII
boolean UNICODE_ConvertFile(UTFType_t InType, uint8_t* InData, size_t InSize,
	UTFType_t OutType, uint8_t** OutData, size_t* OutSize)
{
	// Pointers
	uint8_t* x8;
	uint16_t* x16;
	uint32_t* x32;
	size_t InitOffset = 0;
	size_t CorrectedSize = 0;
	boolean StrippedOut = false;
	boolean SwappedIn = false;
	boolean SwappedOut = false;
	boolean DontAdd = false;
	uint32_t In;
	uint8_t* y8;
	uint16_t* y16;
	uint32_t* y32;
	size_t NumChars;
	
	/* Check Validity */
	if (!InData || !InSize || !OutData || !OutSize || OutType == UT_AUTO || InType == UT_WCHART)
		return false;
		
	/* Check for stripping */
	if (OutType & UT_REMOODSTRIP)
		StrippedOut = true;
	
	InType = InType & ~UT_REMOODSTRIP;
	OutType = OutType & ~UT_REMOODSTRIP;
	
	/* wchar_t based */
	if (OutType == UT_WCHART)
	{
		if (sizeof(wchar_t) == 4)
			OutType = UT_UTF32;
		else if (sizeof(wchar_t) == 2)
			OutType = UT_UTF16;
		else
			OutType = UT_UTF8;
	}
		
	/* Normalize BE and LE */
#if defined(__BIG_ENDIAN__)
	if (InType == UT_UTF16BE)
		InType = UT_UTF16;
	else if (InType == UT_UTF32BE)
		InType = UT_UTF32;
	if (OutType == UT_UTF16BE)
		OutType = UT_UTF16;
	else if (OutType == UT_UTF32BE)
		OutType = UT_UTF32;
#else
	if (InType == UT_UTF16LE)
		InType = UT_UTF16;
	else if (InType == UT_UTF32LE)
		InType = UT_UTF32;
	if (OutType == UT_UTF16LE)
		OutType = UT_UTF16;
	else if (OutType == UT_UTF32LE)
		OutType = UT_UTF32;
#endif
		
	/* If the In Type is auto then guess the internal type and set it */
	// Never goes to ASCII
	x8 = InData;
	x16 = InData;
	x32 = InData;
	
	if (InType == UT_AUTO)
	{
		// First check for UTF-32...
		if (InSize >= 4 && *x32 == 0xFEFF)			// Native UTF-32
		{
			InType = UT_UTF32;
			InitOffset = 4;
		}
		else if (InSize >= 4 && *x32 == 0xFFFE0000)	// Non-Native UTF-32
		{
#if defined(__BIG_ENDIAN__)
			InType = UT_UTF32LE;
#else
			InType = UT_UTF32BE;
#endif
			InitOffset = 4;
		}
		else if (InSize >= 2 && *x16 == 0xFFFE)		// Native UTF-16
		{
			InType = UT_UTF16;
			InitOffset = 2;
		}
		else if (InSize >= 2 && *x16 == 0xFEFF)		// Non-Native UTF-32
		{
#if defined(__BIG_ENDIAN__)
			InType = UT_UTF16LE;
#else
			InType = UT_UTF16BE;
#endif
			InitOffset = 2;
		}
		else if (InSize >= 3 && *x8 == 0xEF && *(x8+1) == 0xBB && *(x8+2) == 0xBF)	// Microsoft Decorated UTF-8
		{
			InType = UT_UTF8;
			InitOffset = 3;
		}
		else
			InType = UT_UTF8;
	}
	
	/* Set Opposite to BS, since it's simpler for this function */
#if defined(__BIG_ENDIAN__)
	if (InType == UT_UTF32LE)
		InType = UT_UTF32BS;
	else if (InType == UT_UTF16LE)
		InType = UT_UTF16BS;
	if (OutType == UT_UTF32LE)
		OutType = UT_UTF32BS;
	else if (OutType == UT_UTF16LE)
		OutType = UT_UTF16BS;
#else
	if (InType == UT_UTF32BE)
		InType = UT_UTF32BS;
	else if (InType == UT_UTF16BE)
		InType = UT_UTF16BS;
	if (OutType == UT_UTF32BE)
		OutType = UT_UTF32BS;
	else if (OutType == UT_UTF16BE)
		OutType = UT_UTF16BS;
#endif

	// Swapped?
	if (InType == UT_UTF16BS || InType == UT_UTF32BS)
		SwappedIn = true;
	if (OutType == UT_UTF16BS || OutType == UT_UTF32BS)
		SwappedOut = true;

	// Reset
	x8 = InData + InitOffset;
	x16 = InData + InitOffset;
	x32 = InData + InitOffset;
	
	if (InType == UT_UTF8)
		CorrectedSize = (InSize - InitOffset);
	else if (InType == UT_UTF16 || InType == UT_UTF16BS)
		CorrectedSize = (InSize - InitOffset) / 2;
	else if (InType == UT_UTF32 || InType == UT_UTF32BS)
		CorrectedSize = (InSize - InitOffset) / 4;
		
	/* Get Corrected Length */
	NumChars = 0;
	
	switch (InType)
	{
		case UT_ASCII:
		case UT_UTF8:
			while (*x8)
			{
				DontAdd = false;
				
				// Get character
				In = *x8;
				
				// Huge switches compacted into a basic if
				if ((In == '\\') && (x8 < (InData + InSize)) && (*(x8+1)))
				{
					// Skip characters
					if ((*(x8+1) == 'x') && ((x8 + 5) < (InData + InSize)))
						x8 += 5;
					else if ((*(x8+1) == '\\') && ((x8 + 1) < (InData + InSize)))
						x8 += 1;
				}
				else if (InType == UT_UTF8 && (In & 0x80))
				{
					if (((In & 0xE0) == 0xC0) && ((x8 + 1) < (InData + InSize)))	// Two-byte
						x8 += 1;
					else if (((In & 0xF0) == 0xE0) && ((x8 + 2) < (InData + InSize)))	// Three-byte
						x8 += 2;
					else if (((In & 0xF8) == 0xF0) && ((x8 + 3) < (InData + InSize)))	// four-byte
						x8 += 3;
				}
				
				// Increment
				if (!DontAdd)
					NumChars++;
				
				// Next Char
				x8++;
			}
			break;
		
		case UT_UTF16:
		case UT_UTF16BS:
#define DOWESWAP16(n) (SwappedIn ? SWAP16((n)) : (n))
			while (*x16)
			{
				DontAdd = false;
				
				// Get character
				if (SwappedIn)
					In = SWAP16(*x16);
				else
					In = *x16;
				
				// Escape sequences and multibyte
				if ((In == '\\') && (x16 < (uint16_t*)(InData + InSize)) && (*(x16+1)))
				{
					// Skip characters
					if ((*(x16+1) == DOWESWAP16('x')) && ((x16 + 5) < (uint16_t*)(InData + InSize)))
						x16 += 5;
					else if ((*(x16+1) == DOWESWAP16('\\')) && ((x16 + 1) < (uint16_t*)(InData + InSize)))
						x16 += 1;
				}
				else if (In >= 0xD800 && In <= 0xDB7F && ((x16 + 1) < (uint16_t*)(InData + InSize)))	// 4-byte
					x16 += 1;
				
				// Increment
				if (!DontAdd)
					NumChars++;
				
				// Next Char
				x16++;
			}
			break;
			
		case UT_UTF32:
		case UT_UTF32BS:
#define DOWESWAP32(n) (SwappedIn ? SWAP32((n)) : (n))
			while (*x32)
			{
				DontAdd = false;
				
				// Get character
				if (SwappedIn)
					In = SWAP32(*x32);
				else
					In = *x32;
					
				// Escape sequences
				if ((In == '\\') && (x32 < (uint32_t*)(InData + InSize)) && (*(x32+1)))
				{
					// Skip characters
					if ((*(x32+1) == DOWESWAP32('x')) && ((x32 + 5) < (uint32_t*)(InData + InSize)))
						x32 += 5;
					else if ((*(x16+1) == DOWESWAP32('\\')) && ((x32 + 1) < (uint32_t*)(InData + InSize)))
						x32 += 1;
				}
				
				// Increment
				if (!DontAdd)
					NumChars++;
				
				// Next Char
				x32++;
			}
			break;
		
		default:
			return false;
	};
	
	/* Now Convert */
	
	return false;
}

/* UNICODE_Localize() -- Convert non-native Unicode file and ASCII file to native Unicode file */
// ASCII	--> UTF-16 (Native Endieness)
// UTF-8	--> UTF-16 (Native Endieness)
// UTF-16BE --> UTF-16 (Native Endieness)
// UTF-16LE --> UTF-16 (Native Endieness)
// UTF-32BE --> UTF-16 (Native Endieness)
// UTF-32LE --> UTF-16 (Native Endienness
boolean UNICODE_Localize(uint8_t* InData, size_t InSize, wchar_t** OutData, size_t* OutSize)
{
	return UNICODE_ConvertFile(UT_AUTO, InData, InSize, UT_WCHART, OutData, OutSize);
}


