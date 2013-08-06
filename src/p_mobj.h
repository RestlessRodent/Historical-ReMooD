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
// DESCRIPTION: Map Objects, MObj, definition and handling.

#ifndef __P_MOBJ__
#define __P_MOBJ__

#include "d_think.h"

// Basics.
//#include "tables.h"
//#include "m_fixed.h"

// We need the thinker_t stuff.
//#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
//#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
//#include "info.h"

//#include "s_sound.h"
//#include "d_items.h"

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
// from PI_state_t structures.
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
	MF2_BOUNCES =           0x00200000,	// Bounces off walls and floors
	MF2_FRIENDLY =          0x00400000,	// On the players' side
	MF2_FORCETRANSPARENCY = 0x00800000,
	
	MF2_FLOORHUGGER 	  = 0x01000000,	// Always on the floor
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

/* mobjflagrexa_t -- Extended flag group A  */
// GhostlyDeath <March 4, 2012> -- Extended flags here
typedef enum mobjflagrexa_e
{
	MFREXA_ENABLEFASTSPEED		= 0x00000001U,	// Allow RFastSpeed
	MFREXA_NOFORCEALLTRIGGERC	= 0x00000002U,	// Can't trigger cross-line even if ML_ALLTRIGGER
	MFREXA_NOCROSSTRIGGER		= 0x00000004U,	// Does not trigger crossing lines
	MFREXA_ISPUSHPULL			= 0x00000008U,	// Returnable by P_GetPushThing()
	MFREXA_DOPUSHAWAY			= 0x00000010U,	// Push instead of pull by PIT_PushThing()
	MFREXA_ISTELEPORTMAN		= 0x00000020U,	// Is teleport destination? [EV_Teleport()]
	MFREXA_ALWAYSTELEPORT		= 0x00000040U,	// Always teleport? no check [EV_Teleport()]
	MFREXA_ISMONSTER			= 0x00000080U,	// Treat as monster regardless of !MF_COUNTKILL
	MFREXA_ISTELEFOG			= 0x00000100U,	// Spin triangle in automap
	MFREXA_ISPOWERUP			= 0x00000200U,	// Glow triangle in automap
	MFREXA_HALFMISSILERANGE		= 0x00000400U,	// Half missile range [P_CheckMissileRange]
	MFREXA_SOUNDEVERYWHERE		= 0x00000800U,	// Play wake up/death sound everywhere
	MFREXA_NOWATERSPLASH		= 0x00001000U,	// Do not splash water [P_MobjCheckWater]
	MFREXA_NOCHECKWATER			= 0x00002000U,	// Do not perform water check []
	MFREXA_USENULLMOTHINKER		= 0x00004000U,	// Use MobjNullThinker
	MFREXA_NOPLAYERWALK			= 0x00008000U,	// Don't use player walking animation [P_XYFriction]
	MFREXA_NOSMOOTHSTEPUP		= 0x00010000U,	// Don't smoothly step up the player view [P_ZMovement]
	MFREXA_NOALTDMRESPAWN		= 0x00020000U,	// Do not respawn in altdeath mode
	MFREXA_CARRYKILLER			= 0x00040000U,	// Carries object killer (Barrels) [P_KillMobj]
	MFREXA_MARKRESTOREWEAPON	= 0x00080000U,	// Mark mapthing_t to restore weapon [P_RespawnWeapons]
	MFREXA_NORANDOMPLAYERLOOK	= 0x00100000U,	// Don't randomize the last target player when spawning the new object [P_SpawnMapObject]
	MFREXA_ALLOWNOCROSSCROSS	= 0x00200000U,	// Allow crossing of uncrossable lines [PIT_CheckLine]
	MFREXA_NEVERCROSSTRIGGER	= 0x00400000U,	// Similar to MFREXA_NOCROSSTRIGGER however it never triggers cross specials regardless if ML_ALLTRIGGER or not [P_TryMove]
	MFREXA_CANCEILINGSTEP		= 0x00800000U,	// Can step from the ceiling (similar to stairs but for ceilings instead)
	MFREXA_NOTHRESHOLD			= 0x01000000U,	// No threshold (instantly change target) [P_DamageMobj]
	MFREXA_NOTRETAILIATETARGET	= 0x02000000U,	// Object cannot be targetted by monster even if hurt by it [P_DamageMobj]
	MFREXA_RADIUSATTACKPROOF	= 0x04000000U,	// Immune to radial attacks [PIT_RadiusAttack]
	MFREXA_RANDOMPUFFTIME		= 0x08000000U,	// Randomize the puff time a bit [P_SpawnPuff]
	MFREXA_KEEPGRAVONDEATH		= 0x10000000U,	// Keep gravity when object is killed [P_KillMobj]
	MFREXA_ISPLAYEROBJECT		= 0x20000000U,	// Is player object?
	MFREXA_ISBRAINTARGET		= 0x40000000U,	// Brain shoots here [P_InitBrainTarget]
} mobjflagrexa_t;

/* mobjflagrexb_t -- Extended flag group B  */
// GhostlyDeath <March 7, 2012> -- Extended flags here
typedef enum mobjflagrexb_e
{
	MFREXB_DOMAPSEVENSPECA		= 0x00000001U,	// DOOM2.WAD MAP07 (666) [A_BossDeath]
	MFREXB_DOMAPSEVENSPECB		= 0x00000002U,	// DOOM2.WAD MAP07 (667) [A_BossDeath]
	MFREXB_DOBARONSPECIAL		= 0x00000004U,	// Triggers baron special [A_BossDeath]
	MFREXB_DOCYBERSPECIAL		= 0x00000008U,	// Triggers cyber special [A_BossDeath]
	MFREXB_DOSPIDERSPECIAL		= 0x00000010U,	// Triggers spider special [A_BossDeath]
	MFREXB_DODOORSIXTHREEOPEN	= 0x00000020U,	// When this thing dies, blaze a door open [A_BossDeath]
	MFREXB_INITBOTNODES			= 0x00000040U,	// Initialize With bot nodes
	MFREXB_DONTTAKEDAMAGE		= 0x00000080U,	// Do not actually take damage
	MFREXB_ISHIGHBOUNCER		= 0x00000100U,	// Bounces High
	MFREXB_NONMISSILEFLBOUNCE	= 0x00000200U,	// Bounce on floor without being MF_MISSILE
	MFREXB_IGNOREBLOCKMONS		= 0x00000400U,	// Ignore monster blocking lines
	MFREXB_USEPLAYERMOVEMENT	= 0x00000800U,	// Use player movement
	MFREXB_CANUSEWEAPONS		= 0x00001000U,	// Can use player weapons
	MFREXB_NOFLOORDAMAGE		= 0x00002000U,	// No damage on the floor
	MFREXB_ISDOOMPALETTE		= 0x00004000U,	// Uses Doom Palette
	MFREXB_SPITBIT				= 0x00008000U,	// Brain Spit Something (easy shifter)
	MFREXB_NONMRESPAWN			= 0x00010000U,	// Cannot be respawned
	MFREXB_FREEZEDEMO			= 0x00020000U,	// Freezes Demo Playback
	MFREXB_NOHERETICFRICT		= 0x00040000U,	// No Heretic Friction
} mobjflagrexb_t;

/* P_MobjRefType_t -- Reference type */
typedef enum P_MobjRefType_e
{
	PMRT_TARGET,								// ->target
	PMRT_TRACER,								// ->tracer
	PMRT_FOLLOWPLAYER,							// ->FollowPlayer
	
	NUMPMOBJREFTYPES
} P_MobjRefType_t;

/* Define PI_stateid_t */
#if !defined(__REMOOD_STSPIDS_DEFINED)
	typedef int32_t PI_spriteid_t;
	typedef int32_t PI_stateid_t;
	#define __REMOOD_STSPIDS_DEFINED
#endif

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

/* Define mobj_t */
#if !defined(__REMOOD_MOBJT_DEFINED)
	typedef struct mobj_s mobj_t;
	#define __REMOOD_MOBJT_DEFINED
#endif

/* Define PI_state_t */
#if !defined(__REMOOD_PISTATE_DEFINED)
	typedef struct PI_state_s PI_state_t;
	#define __REMOOD_PISTATE_DEFINED
#endif

/* Define mapthing_t */
#if !defined(__REMOOD_MAPTHINGT_DEFINED)
	typedef struct mapthing_s mapthing_t;
	#define __REMOOD_MAPTHINGT_DEFINED
#endif

/* Define PI_mobjid_t */
#if !defined(__REMOOD_PIMOID_DEFINED)
	typedef int32_t PI_mobjid_t;
	#define __REMOOD_PIMOID_DEFINED
#endif

/* Define PI_mobj_t */
#if !defined(__REMOOD_PIMOBJT_DEFINED)
	typedef struct PI_mobj_s PI_mobj_t;
	#define __REMOOD_PIMOBJT_DEFINED
#endif

/* Define PI_wepid_t */
#if !defined(__REMOOD_PIWEPIDT_DEFINED)
	typedef int32_t PI_wepid_t;
	#define __REMOOD_PIWEPIDT_DEFINED
#endif

/* P_MobjRefLog_t -- Map object reference log */
typedef struct P_MobjRefLog_s
{
	mobj_t* Source;
	mobj_t* Ref;
	bool_t IsSource;
	char* File;
	int Line;
} P_MobjRefLog_t;

#if MAXSKINCOLOR > 16

#error MAXSKINCOLOR have changed Change MF_TRANSLATION to take effect of the change
#endif

/* P_EXAttackType_t -- Attack type */
typedef enum P_RXAttackType_e
{
	PRXAT_UNKNOWN,								// Unknown
	PRXAT_MELEE,								// Melee Attack
	PRXAT_RANGED,								// Ranged Attack
	PRXAT_TELEFRAG,								// Telefrag
	PRXAT_SUICIDE,								// Suicide Pill
	
	NUMPRXATTACKTYPES
} P_RXAttackType_t;

typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
} dirtype_t;

/* KidList_t -- Linked list of kids */
typedef struct KidList_s
{
	/* Link to kid */
	void* Kid;
	
	/* Links */
	struct KidList_s* Prev;
	struct KidList_s* Next;
} KidList_t;

/* mapthing_t -- Map Thing */
struct mapthing_s
{
	int16_t x;
	int16_t y;
	int16_t z;					// Z support for objects SSNTails 07-24-2002
	int16_t angle;
	uint32_t type;
	uint32_t options;
	
	mobj_t* mobj;
	
	// Hexen Stuff
	bool_t IsHexen;								// Hexen Defined
	int16_t HeightOffset;						// Height offset
	uint16_t ID;								// Hexen Thing ID
	uint8_t Special;							// Hexen Special
	uint8_t Args[5];							// Hexen arguments
	
	// Other Stuff
	PI_mobjid_t MoType;							// Type of spawned object
	bool_t MarkedWeapon;						// Marked as a weapon to respawn
};

/* mobj_t -- Map Object definition */
struct mobj_s
{
	// List: thinker links.
	thinker_t thinker;
	
	S_NoiseThinker_t NoiseThinker;	// Info for noise generation
	
	// Info for drawing: position.
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	// More list: links in sector (if needed)
	mobj_t* snext;
	mobj_t* sprev;
	
	//More drawing info: to determine current sprite.
	angle_t angle;				// orientation
	PI_spriteid_t sprite;			// used to find patch_t and flip value
	int32_t frame;					// frame number, plus bits see p_pspr.h
	
	//Fab:02-08-98
	int32_t skin;					// GhostlyDeath <Jult 16, 2011> -- Make this an integer instead
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
	mobj_t* bnext;
	mobj_t* bprev;
	
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
	
	PI_mobjid_t type;
	PI_mobj_t* info;			// &mobjinfo[mobj->type]
	
	int32_t tics;					// state tic counter
	PI_state_t* state;
	
	/*** DEPRECATED ***/
	int32_t flags;
	int32_t eflags;					//added:28-02-98: extra flags see above
	int32_t flags2;					// heretic stuff
	
	/******************/
	
	int32_t special1;
	int32_t special2;
	int32_t health;
	
	// Movement direction, movement generation (zig-zagging).
	int32_t movedir;				// 0-7
	int32_t movecount;				// when 0, select a new dir
	
	// Thing being chased/attacked (or NULL),
	// also the originator for missiles.
	mobj_t* target;
	
	// Reaction time: if non 0, don't attack yet.
	// Used by player to freeze a bit after teleporting.
	int32_t reactiontime;
	
	// If >0, the target will be chased
	// no matter what (even if shot)
	int32_t threshold;
	
	// Additional info record for player avatars only.
	player_t* player;
	
	// Player number last looked for.
	int lastlook;
	
	// For nightmare and itemrespawn respawn.
	mapthing_t* spawnpoint;
	
	// Thing being chased/attacked for tracers.
	mobj_t* tracer;
	
	//SoM: Friction.
	int32_t friction;
	int32_t movefactor;
	
	// a linked list of sectors where this object appears
	struct msecnode_s* touching_sectorlist;
	
	// Support for Frag Weapon Falling
	// This field valid only for MF_DROPPED ammo and weapn objects
	int32_t dropped_ammo_count;
	
	/*** RMOD EXTENDED SUPPORT ***/
	// New Flags
	uint32_t RXFlags[NUMINFORXFIELDS];			// ReMooD Extended Flags
	
	// Obituary helpers
	PI_wepid_t RXShotWithWeapon;				// Weapon that fired this
	P_RXAttackType_t RXAttackAttackType;		// Attack type
	
	// Reference Counts
	int32_t RefCount[NUMPMOBJREFTYPES];			// Objects referencing this object
	mobj_t** RefList[NUMPMOBJREFTYPES];	// Reference List
	size_t RefListSz[NUMPMOBJREFTYPES];			// Size of reference list

#if defined(_DEBUG)
	char* RefFile[NUMPMOBJREFTYPES];			// Reference From File
	int32_t RefLine[NUMPMOBJREFTYPES];				// Reference From Line
#endif

	bool_t RemoveMo;							// Remove Map Object
	PI_mobjid_t RemType;							// Type removed
	
	// Properties
	fixed_t MaxZObtained;						// Max Z Obtained while in air
	uint32_t SpawnOrder;						// Object Spawn Order
	
	// GhostlyDeath <April 26, 2012> -- Improved mobj on mobj
	mobj_t** MoOn[2];					// Objects on top/bottom
	size_t MoOnCount[2];						// Count of top/bottom
	
	// GhostlyDeath <April 29, 2012> -- Teams
	int32_t SkinTeamColor;						// player skincolor + 1 Team
	
	// GhostlyDeath <June 6, 2012> -- Follow Player (Friendlies)
	mobj_t* FollowPlayer;				// Following Player
	
	// GhostlyDeath <June 12, 2012> -- Object Cleanup
	uint32_t TimeThinking[2];					// Time spent thinking
	uint32_t TimeFromDead[2];					// Time being dead'
	
	// GhostlyDeath <June 15, 2012> -- Player who killed this thing
	int32_t KillerPlayer;						// Player Killer
	uint32_t FraggerID;							// Fragger ID of Player
	
	// GhostlyDeath <June 22, 2012> -- Interpolation
	fixed_t DrawPos[3];							// Interpolated Draw Position
	
	// GhostlyDeath <February 15, 2013> -- Team/CTF Related Stuff
	int32_t FakeColor;							// Draw as this color
	int8_t CTFTeam;								// Team Flag is on?
	bool_t (*AltTouchFunc)(mobj_t* const a_Special, mobj_t* const a_Toucher);
};

/* Converts natural flags to/from extended flags */
int P_MobjFlagsNaturalToExtended(mobj_t* MObj);
int P_MobjFlagsExtendedToNatural(mobj_t* MObj);

// check mobj against water content, before movement code
void P_MobjCheckWater(mobj_t* mobj);

void P_SpawnMapThing(mapthing_t* mthing);
void P_SpawnPlayerBackup(int32_t const a_PlayerNum);
void P_SpawnPlayer(mapthing_t* mthing);
int P_HitFloor(mobj_t* thing);

mobj_t* P_RefMobjReal(const P_MobjRefType_t a_Type, mobj_t* const a_SourceRef, mobj_t* const a_RefThis
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif 
	);

void P_FindMobjRef(const P_MobjRefType_t a_Type, mobj_t* const a_SourceRef);
void P_SetMobjToCrash(mobj_t* const a_Mo);
void P_ClearMobjRefs(mobj_t* const a_Mo);

#if defined(_DEBUG)
	#define P_RefMobj(t,s,r) P_RefMobjReal((t), (s), (r), __FILE__, __LINE__)
#else
	#define P_RefMobj(t,s,r) P_RefMobjReal((t), (s), (r))
#endif

void P_NightmareRespawn(mobj_t* mobj, const bool_t a_ForceRespawn);

void P_RemoveFromBodyQueue(mobj_t* const a_Mo);
void P_MorphObjectClass(mobj_t* const a_Mo, const PI_mobjid_t a_NewClass);


bool_t P_MobjIsPlayer(mobj_t* const a_Mo);
bool_t P_MobjOnSameFamily(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo);
int32_t P_GetPlayerTeam(player_t* const a_Player);
int32_t P_GetMobjTeam(mobj_t* const a_Mo);
bool_t P_MobjOnSameTeam(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo);
bool_t P_MobjDamageTeam(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo, mobj_t* const a_Inflictor);

void P_ControlNewMonster(player_t* const a_Player);

#endif

