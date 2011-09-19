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
// DESCRIPTION: Map Objects, MObj, definition and handling.

#ifndef __P_MOBJ__
#define __P_MOBJ__

// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

#include "s_sound.h"

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
	// Call P_SpecialThing when touched.
	MF_SPECIAL = 0x0001,
	// Blocks.
	MF_SOLID = 0x0002,
	// Can be hit.
	MF_SHOOTABLE = 0x0004,
	// Don't use the sector links (invisible but touchable).
	MF_NOSECTOR = 0x0008,
	// Don't use the blocklinks (inert but displayable)
	MF_NOBLOCKMAP = 0x0010,
	
	// Not to be activated by sound, deaf monster.
	MF_AMBUSH = 0x0020,
	// Will try to attack right back.
	MF_JUSTHIT = 0x0040,
	// Will take at least one step before attacking.
	MF_JUSTATTACKED = 0x0080,
	// On level spawning (initial position),
	//  hang from ceiling instead of stand on floor.
	MF_SPAWNCEILING = 0x0100,
	// Don't apply gravity (every tic),
	//  that is, object will float, keeping current height
	//  or changing it actively.
	MF_NOGRAVITY = 0x0200,
	
	// Movement flags.
	// This allows jumps from high places.
	MF_DROPOFF = 0x0400,
	// For players, will pick up items.
	MF_PICKUP = 0x0800,
	// Player cheat. ???
	MF_NOCLIP = 0x1000,
	// Player: keep info about sliding along walls.
	MF_SLIDE = 0x2000,
	// Allow moves to any height, no gravity.
	// For active floaters, e.g. cacodemons, pain elementals.
	MF_FLOAT = 0x4000,
	// Don't cross lines
	//   ??? or look at heights on teleport.
	MF_TELEPORT = 0x8000,
	// Don't hit same species, explode on block.
	// Player missiles as well as fireballs of various kinds.
	MF_MISSILE = 0x10000,
	// Dropped by a demon, not level spawned.
	// E.g. ammo clips dropped by dying former humans.
	MF_DROPPED = 0x20000,
	// DOOM2: Use fuzzy draw (shadow demons or spectres),
	//  temporary player invisibility powerup.
	// LEGACY: no more for translucency, but still makes targeting harder
	MF_SHADOW = 0x40000,
	// Flag: don't bleed when shot (use puff),
	//  barrels and shootable furniture shall not bleed.
	MF_NOBLOOD = 0x80000,
	// Don't stop moving halfway off a step,
	//  that is, have dead bodies slide down all the way.
	MF_CORPSE = 0x100000,
	// Floating to a height for a move, ???
	//  don't auto float to target's height.
	MF_INFLOAT = 0x200000,
	
	// On kill, count this enemy object
	//  towards intermission kill total.
	// Happy gathering.
	MF_COUNTKILL = 0x400000,
	
	// On picking up, count this item object
	//  towards intermission item total.
	MF_COUNTITEM = 0x800000,
	
	// Special handling: skull in flight.
	// Neither a cacodemon nor a missile.
	MF_SKULLFLY = 0x1000000,
	
	// Don't spawn this object
	//  in death match mode (e.g. key cards).
	MF_NOTDMATCH = 0x2000000,
	
	// Player sprites in multiplayer modes are modified
	//  using an internal color lookup table for re-indexing.
	// If 0x4 0x8 or 0xc,
	//  use a translation table for player colormaps
	MF_TRANSLATION = 0x3C000000,	// 0xc000000, original 4color
	
	// Hmm ???.
	MF_TRANSSHIFT = 26,
	
	// for chase camera, don't be blocked by things (parsial cliping)
	MF_NOCLIPTHING = 0x40000000,
	
	MF_FLOORHUGGER = 0x80000000
} mobjflag_t;

typedef enum
{
	MF2_LOGRAV = 0x00000001,	// alternate gravity setting
	MF2_WINDTHRUST = 0x00000002,	// gets pushed around by the wind
	// specials
	MF2_FLOORBOUNCE = 0x00000004,	// bounces off the floor
	MF2_THRUGHOST = 0x00000008,	// missile will pass through ghosts
	MF2_FLY = 0x00000010,		// fly mode is active
	MF2_FOOTCLIP = 0x00000020,	// if feet are allowed to be clipped
	MF2_SPAWNFLOAT = 0x00000040,	// spawn random float z
	MF2_NOTELEPORT = 0x00000080,	// does not teleport
	MF2_RIP = 0x00000100,		// missile rips through solid
	// targets
	MF2_PUSHABLE = 0x00000200,	// can be pushed by other moving
	// mobjs
	MF2_SLIDE = 0x00000400,		// slides against walls
	MF2_ONMOBJ = 0x00000800,	// mobj is resting on top of another
	// mobj
	MF2_PASSMOBJ = 0x00001000,	// Enable z block checking.  If on,
	// this flag will allow the mobj to
	// pass over/under other mobjs.
	MF2_CANNOTPUSH = 0x00002000,	// cannot push other pushable mobjs
	MF2_FEETARECLIPPED = 0x00004000,	// a mobj's feet are now being cut
	MF2_BOSS = 0x00008000,		// mobj is a major boss
	MF2_FIREDAMAGE = 0x00010000,	// does fire damage
	MF2_NODMGTHRUST = 0x00020000,	// does not thrust target when
	// damaging
	MF2_TELESTOMP = 0x00040000,	// mobj can stomp another
	MF2_FLOATBOB = 0x00080000,	// use float bobbing z movement
	MF2_DONTDRAW = 0X00100000,	// don't generate a vissprite
	
	// GhostlyDeath
	MF2_BOUNCES = 0x00200000,	// Bounces off walls and floors
	MF2_FRIENDLY = 0x00400000,	// On the players' side
	MF2_FORCETRANSPARENCY = 0x00800000,
} mobjflag2_t;

//
//  New mobj extra flags
//
//added:28-02-98:
typedef enum
{
	// The mobj stands on solid floor (not on another mobj or in air)
	MF_ONGROUND = 1,
	// The mobj just hit the floor while falling, this is cleared on next frame
	// (instant damage in lava/slime sectors to prevent jump cheat..)
	MF_JUSTHITFLOOR = 2,
	// The mobj stands in a sector with water, and touches the surface
	// this bit is set once and for all at the start of mobjthinker
	MF_TOUCHWATER = 4,
	// The mobj stands in a sector with water, and his waist is BELOW the water surface
	// (for player, allows swimming up/down)
	MF_UNDERWATER = 8,
	// Set by P_MovePlayer() to disable gravity add in P_MobjThinker() ( for gameplay )
	MF_SWIMMING = 16,
	// used for client prediction code, player can't be blocked in z by walls
	// it is set temporarely when player follow the spirit
	MF_NOZCHECKING = 32,
} mobjeflag_t;

typedef enum
{
	MFXA_AREFEETCLIPPED = 0x00000001,
	MFXA_AUDIBLEPICKUPSOUND = 0x00000002,
	MFXA_CANBOUNCE = 0x00000004,
	MFXA_CANDROPOFF = 0x00000008,
	MFXA_CANFEETCLIP = 0x00000010,
	MFXA_CANFLOORBOUNCE = 0x00000020,
	MFXA_CANGATHER = 0x00000040,
	MFXA_CANJUMP = 0x00000080,
	MFXA_CANMINOTAURSLAM = 0x00000100,
	MFXA_CANOPENDOORS = 0x00000200,
	MFXA_CANSLIDE = 0x00000400,
	MFXA_CANSWIM = 0x00000800,
	MFXA_CANTELEPORTSTOMP = 0x00001000,
	MFXA_CANWALLBOUNCE = 0x00002000,
	MFXA_CARRYKILLER = 0x00004000,
	MFXA_ENABLEZCHECK = 0x00008000,
	MFXA_FLOATBOB = 0x00010000,
	MFXA_FORCEDSPARILTELEPORT = 0x00020000,
	MFXA_ISBOSS = 0x00040000,
	MFXA_ISBOSSCUBESPAWNABLE = 0x00080000,
	MFXA_ISCORPSE = 0x00100000,
	MFXA_ISDEAF = 0x00200000,
	MFXA_ISDOOMITEMFOG = 0x00400000,
	MFXA_ISDOOMPLAYER = 0x00800000,
	MFXA_ISDOOMTELEPORTFOG = 0x01000000,
	MFXA_ISDROPPED = 0x02000000,
	MFXA_ISDSPARIL = 0x04000000,
	MFXA_ISEXPLOSIONIMMUNE = 0x08000000,
	MFXA_ISFLOATABLE = 0x10000000,
	MFXA_ISFLOORHUGGER = 0x20000000,
	MFXA_ISFLYING = 0x40000000,
	MFXA_ISFLYINGSKULL = 0x80000000,
} mobjflagexa_t;

typedef enum
{
	MFXB_ISFRIENDLY = 0x00000001,
	MFXB_ISGATHERABLE = 0x00000002,
	MFXB_ISHERETICITEMFOG = 0x00000004,
	MFXB_ISHERETICPLAYER = 0x00000008,
	MFXB_ISHERETICTELEPORTFOG = 0x00000010,
	MFXB_ISINSTANTKILLIMMUNE = 0x00000020,
	MFXB_ISITEMCOUNTABLE = 0x00000040,
	MFXB_ISKILLCOUNTABLE = 0x00000080,
	MFXB_ISLOWGRAVITY = 0x00000100,
	MFXB_ISMISSILE = 0x00000200,
	MFXB_ISMISSILEINSTANTKILL = 0x00000400,
	MFXB_ISMONSTER = 0x00000800,
	MFXB_ISONGROUND = 0x00001000,
	MFXB_ISONMOBJ = 0x00002000,
	MFXB_ISPUSHABLE = 0x00004000,
	MFXB_ISSHADOW = 0x00008000,
	MFXB_ISSHOOTABLE = 0x00010000,
	MFXB_ISSOLID = 0x00020000,
	MFXB_ISSWIMMING = 0x00040000,
	MFXB_ISTRANSPARENT = 0x00080000,
	MFXB_ISTOUCHINGWATER = 0x00100000,
	MFXB_ISUNDERWATER = 0x00200000,
	MFXB_ISUNIQUE = 0x00400000,
	MFXB_ISWINDPUSHABLE = 0x00800000,
	MFXB_JUSTATTACKED = 0x01000000,
	MFXB_JUSTHIT = 0x02000000,
	MFXB_JUSTHITFLOOR = 0x04000000,
	MFXB_KEEPSLIDE = 0x08000000,
	MFXB_NOALTDEATHMATCH = 0x10000000,
	MFXB_NOAUTOFLOAT = 0x20000000,
	MFXB_NOBLOCKMAP = 0x40000000,
	MFXB_NOBLOOD = 0x80000000,
} mobjflagexb_t;

typedef enum
{
	MFXC_NOCHICKENMORPH = 0x00000001,
	MFXC_NOCLIP = 0x00000002,
	MFXC_NOCOOP = 0x00000004,
	MFXC_NOCTF = 0x00000008,
	MFXC_NODAMAGETHRUST = 0x00000010,
	MFXC_NODEATHMATCH = 0x00000020,
	MFXC_NODRAW = 0x00000040,
	MFXC_NOGRAVITY = 0x00000080,
	MFXC_NOHITGHOST = 0x00000100,
	MFXC_NOHITSOLID = 0x00000200,
	MFXC_NOMISSILEHURTSAMETYPE = 0x00000400,
	MFXC_NOLINEACTIVATE = 0x00000800,
	MFXC_NOLINECLIPPING = 0x00001000,
	MFXC_NOMOVEOVERSAMETYPE = 0x00002000,
	MFXC_NONEWDEATHMATCH = 0x00004000,
	MFXC_NOPUSHING = 0x00008000,
	MFXC_NOSINGLEPLAYER = 0x00010000,
	MFXC_NOSECTORLINKS = 0x00020000,
	MFXC_NOTARGET = 0x00040000,
	MFXC_NOTELEPORT = 0x00080000,
	MFXC_NOTHINGCLIPPING = 0x00100000,
	MFXC_NOZCHECKING = 0x00200000,
	MFXC_REDUCEDBOSSDAMAGE = 0x00400000,
	MFXC_SLOWSPLAYER = 0x00800000,
	MFXC_SPAWNATRANDOMZ = 0x01000000,
	MFXC_SPAWNONCEILING = 0x02000000,
	MFXC_FULLBLASTWAKESOUND = 0x04000000,
	MFXC_FULLBLASTDEATHSOUND = 0x08000000,
	MFXC_VILEMISSILERANGE = 0x10000000,
	MFXC_REVENANTMISSILERANGE = 0x20000000,
	MFXC_HALFMISSILERANGE = 0x40000000,
	MFXC_CYBERDEMONMISSILERANGE = 0x80000000,
} mobjflagexc_t;

typedef enum
{
	MFXD_ROAMSOUNDISSEESOUND = 0x00000001,
	MFXD_FULLBLASTROAMSOUND = 0x00000002,
	MFXD_MINOTAUREXPLOSION = 0x00000004,
	MFXD_FIREBOMBEXPLOSION = 0x00000008,
	MFXD_DSPARILEXPLOSION = 0x00000010,
	MFXD_MAPSEVENSIXSIXSIX = 0x00000020,
	MFXD_MAPSEVENSIXSIXSEVEN = 0x00000040,
	MFXD_COMMERCIALSIXSIXSIX = 0x00000080,
	MFXD_EPISODEONESIXSIXSIX = 0x00000100,
	MFXD_EPISODETWOEXITLEVEL = 0x00000200,
	MFXD_EPISODETHREEEXITLEVEL = 0x00000400,
	MFXD_EPISODEFOURASIXSIXSIX = 0x00000800,
	MFXD_EPISODEFOURBSIXSIXSIX = 0x00001000,
	MFXD_BOSSBRAINTARGET = 0x00002000,
	MFXD_HERETICEPISODEONESPECIAL = 0x00004000,
	MFXD_HERETICEPISODETWOSPECIAL = 0x00008000,
	MFXD_HERETICEPISODETHREESPECIAL = 0x00010000,
	MFXD_HERETICEPISODEFOURSPECIAL = 0x00020000,
	MFXD_HERETICEPISODEFIVESPECIAL = 0x00040000,
	MFXD_SLIDESALONGWALLS = 0x00080000,
	
	MFXD_LEGACYCOMPATIBILITY = 0x80000000,
} mobjflagexd_t;

#if MAXSKINCOLOR > 16

#error MAXSKINCOLOR have changed Change MF_TRANSLATION to take effect of the change
#endif

/* KidList_t -- Linked list of kids */
typedef struct KidList_s
{
	/* Link to kid */
	void* Kid;
	
	/* Links */
	struct KidList_s* Prev;
	struct KidList_s* Next;
} KidList_t;

/* mobj_t -- Map Object definition */
typedef struct mobj_s
{
	// List: thinker links.
	thinker_t thinker;
	
	S_NoiseThinker_t NoiseThinker;	// Info for noise generation
	
	// Info for drawing: position.
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	// More list: links in sector (if needed)
	struct mobj_s* snext;
	struct mobj_s* sprev;
	
	//More drawing info: to determine current sprite.
	angle_t angle;				// orientation
	spritenum_t sprite;			// used to find patch_t and flip value
	int frame;					// frame number, plus bits see p_pspr.h
	
	//Fab:02-08-98
	int skin;					// GhostlyDeath <Jult 16, 2011> -- Make this an integer instead
	// this one overrides 'sprite' when
	// non NULL (currently hack for player
	// bodies so they 'remember' the skin)
	//
	// secondary used when player die and
	// play the die sound problem is he is
	// already respawn and the corps play
	// the sound !!! (he yeah it happens :\)
	
	// Interaction info, by BLOCKMAP.
	// Links in blocks (if needed).
	struct mobj_s* bnext;
	struct mobj_s* bprev;
	
	struct subsector_s* subsector;
	
	// The closest interval over all contacted Sectors (or Things).
	fixed_t floorz;
	fixed_t ceilingz;
	
	// For movement checking.
	fixed_t radius;
	fixed_t height;
	
	// Momentums, used to update position.
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	
	// If == validcount, already checked.
	//int                 validcount;
	
	mobjtype_t type;
	mobjinfo_t* info;			// &mobjinfo[mobj->type]
	
	int tics;					// state tic counter
	state_t* state;
	
	/*** DEPRECATED ***/
	int flags;
	int eflags;					//added:28-02-98: extra flags see above
	int flags2;					// heretic stuff
	
	/******************/
	
	int special1;
	int special2;
	int health;
	
	// Movement direction, movement generation (zig-zagging).
	int movedir;				// 0-7
	int movecount;				// when 0, select a new dir
	
	// Thing being chased/attacked (or NULL),
	// also the originator for missiles.
	struct mobj_s* target;
	
	// Reaction time: if non 0, don't attack yet.
	// Used by player to freeze a bit after teleporting.
	int reactiontime;
	
	// If >0, the target will be chased
	// no matter what (even if shot)
	int threshold;
	
	// Additional info record for player avatars only.
	// Only valid if type == MT_PLAYER
	struct player_s* player;
	
	// Player number last looked for.
	int lastlook;
	
	// For nightmare and itemrespawn respawn.
	mapthing_t* spawnpoint;
	
	// Thing being chased/attacked for tracers.
	struct mobj_s* tracer;
	
	//SoM: Friction.
	int friction;
	int movefactor;
	
	// a linked list of sectors where this object appears
	struct msecnode_s* touching_sectorlist;
	
	// Support for Frag Weapon Falling
	// This field valid only for MF_DROPPED ammo and weapn objects
	int dropped_ammo_count;
	
	// WARNING : new field are not automaticely added to save game
	struct ffloor_s* ChildFloor;
	
	/*** EXTENDED OBJECT DATA ***/
	// Flags
	uint32_t XFlagsA;
	uint32_t XFlagsB;
	uint32_t XFlagsC;
	uint32_t XFlagsD;
	
	// Owners
#if 0
	struct mobj_s* RootOwner;	// Root owner of this object (first mobj in owner chain)
	struct mobj_s* Owner;		// Owner of this object
	KidList_t* Kids;			// Kid objects
#endif
} mobj_t;

/* Converts natural flags to/from extended flags */
int P_MobjFlagsNaturalToExtended(mobj_t* MObj);
int P_MobjFlagsExtendedToNatural(mobj_t* MObj);

// check mobj against water content, before movement code
void P_MobjCheckWater(mobj_t* mobj);

void P_SpawnMapThing(mapthing_t* mthing);
void P_SpawnPlayer(mapthing_t* mthing);
int P_HitFloor(mobj_t* thing);

#endif
