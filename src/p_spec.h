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
// DESCRIPTION: Implements special effects:
//              Texture animation, height or lighting changes
//              according to adjacent sectors, respective
//              utility functions, etc.

#ifndef __P_SPEC__
#define __P_SPEC__

#include "d_rmod.h"

//      Define values for map objects
#define MO_TELEPORTMAN          14

// at game start
void P_InitPicAnims(void);

// at map load (sectors)
void P_SetupLevelFlatAnims(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
bool_t P_UseSpecialLine(mobj_t* thing, line_t* line, int side);

void P_ShootSpecialLine(mobj_t* thing, line_t* line);

void P_CrossSpecialLine(int linenum, int side, mobj_t* thing);

void P_ActivateCrossedLine(line_t* line, int side, mobj_t* thing);

void P_PlayerInSpecialSector(player_t* player);

int twoSided(int sector, int line);

sector_t* getSector(int currentSector, int line, int side);

side_t* getSide(int currentSector, int line, int side);

fixed_t P_FindLowestFloorSurrounding(sector_t* sec);
fixed_t P_FindHighestFloorSurrounding(sector_t* sec);

fixed_t P_FindNextHighestFloor(sector_t* sec, int currentheight);

//SoM: 3/6/2000
fixed_t P_FindNextLowestFloor(sector_t* sec, int currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t* sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec);

int P_FindSectorFromLineTag(line_t* line, int start);

int P_FindSectorFromTag(int tag, int start);

//DarkWolf95:July 23, 2003: Needed for SF_SetLineTexture
int P_FindLineFromTag(int tag, int start);

int P_FindLineFromLineTag(const line_t* line, int start);	//SoM: 3/16/2000

int P_FindMinSurroundingLight(sector_t* sector, int max);

sector_t* getNextSector(line_t* line, sector_t* sec);

//SoM: 3/6/2000
sector_t* P_FindModelFloorSector(fixed_t floordestheight, int secnum);

//SoM: 3/15/2000
fixed_t P_FindNextHighestCeiling(sector_t* sec, int currentheight);
fixed_t P_FindNextLowestCeiling(sector_t* sec, int currentheight);
fixed_t P_FindShortestUpperAround(int secnum);
fixed_t P_FindShortestTextureAround(int secnum);
sector_t* P_FindModelCeilingSector(fixed_t ceildestheight, int secnum);
bool_t P_CanUnlockGenDoor(line_t* line, player_t* player);
int P_CheckTag(line_t* line);

//
// SPECIAL
//
int EV_DoDonut(line_t* line);

//
// P_LIGHTS
//
typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int maxlight;
	int minlight;
	
} fireflicker_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int maxlight;
	int minlight;
	int maxtime;
	int mintime;
	
} lightflash_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int count;
	int minlight;
	int maxlight;
	int darktime;
	int brighttime;
	
} strobe_t;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int minlight;
	int maxlight;
	int direction;
	
} glow_t;

//SoM: thinker struct for fading lights. ToDo: Add effects for light
// transition
typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	int destlevel;
	int speed;
} lightlevel_t;

#define GLOWSPEED               8
#define STROBEBRIGHT            5
#define FASTDARK                15
#define SLOWDARK                35

void T_FireFlicker(fireflicker_t* flick);
void P_SpawnFireFlicker(sector_t* sector);
void T_LightFlash(lightflash_t* flash);
void P_SpawnLightFlash(sector_t* sector);
void T_StrobeFlash(strobe_t* flash);

void P_SpawnStrobeFlash(sector_t* sector, int fastOrSlow, int inSync);

int EV_StartLightStrobing(line_t* line);
int EV_TurnTagLightsOff(line_t* line);

int EV_LightTurnOn(line_t* line, int bright);

void T_Glow(glow_t* g);
void P_SpawnGlowingLight(sector_t* sector);

void P_FadeLight(int tag, int destvalue, int speed);
void T_LightFade(lightlevel_t* ll);

//
// P_SWITCH
//
#pragma pack(1)					//Hurdler: 04/04/2000: I think pragma is more portable
typedef struct
{
	char name1[9];
	char name2[9];
	short episode;
} switchlist_t;

//} __attribute__ ((packed)) switchlist_t; //SoM: 3/22/2000: Packed to read from memory.
#pragma pack()

typedef enum
{
	top,
	middle,
	bottom
} bwhere_e;

typedef struct
{
	line_t* line;
	bwhere_e where;
	int btexture;
	int btimer;
	S_NoiseThinker_t* soundorg;
	
} button_t;

// max # of wall switches in a level
#define MAXSWITCHES             50

// 4 players, 4 buttons each at once, max.
// added 19-1-98 16->MAXPLAYERS*4
#define MAXBUTTONS           (MAXPLAYERS*4)	//16

// 1 second, in ticks.
#define BUTTONTIME      35

extern button_t buttonlist[MAXBUTTONS];

void P_ChangeSwitchTexture(line_t* line, int useAgain);

void P_InitSwitchList(void);

// SoM: 3/4/2000: Misc Boom stuff for thinkers that can share sectors, and some other stuff

typedef enum
{
	floor_special,
	ceiling_special,
	lighting_special,
} special_e;

//SoM: 3/6/2000
size_t P_SectorActive(special_e t, sector_t* s);

typedef enum
{
	trigChangeOnly,
	numChangeOnly,
} change_e;

//
// P_PLATS
//
typedef enum
{
	up,
	down,
	waiting,
	in_stasis
} plat_e;

typedef enum
{
	PPT_PERPRAISE,
	downWaitUpStay,
	raiseAndChange,
	raiseToNearestAndChange,
	blazeDWUS,
	//SoM:3/4/2000: Added boom stuffs
	genLift,					//General stuff
	genPerpetual,
	toggleUpDn,					//Instant toggle of stuff.
	
} plattype_e;

typedef struct
{
	thinker_t thinker;
	sector_t* sector;
	fixed_t speed;
	fixed_t low;
	fixed_t high;
	int wait;
	int count;
	plat_e status;
	plat_e oldstatus;
	bool_t crush;
	int tag;
	plattype_e type;
	
	struct platlist* list;		//SoM: 3/6/2000: Boom's improved code without limits.
} plat_t;

//SoM: 3/6/2000: Boom's improved code without limits.
typedef struct platlist
{
	plat_t* plat;
	struct platlist* next, **prev;
} platlist_t;

void P_RemoveAllActivePlats(void);	//SoM: 3/9/2000

#define PLATWAIT                3
#define PLATSPEED               (FRACUNIT)
#define MAXPLATS                30

extern platlist_t* activeplats;

void T_PlatRaise(plat_t* plat);

void P_AddActivePlat(plat_t* plat);
void P_RemoveActivePlat(plat_t* plat);
int EV_StopPlat(line_t* line);
void P_ActivateInStasis(int tag);

//
// P_DOORS
//
typedef enum
{
	normalDoor,
	close30ThenOpen,
	doorclose,
	dooropen,
	raiseIn5Mins,
	blazeRaise,
	blazeOpen,
	blazeClose,
	
	//SoM: 3/4/2000: General door types...
	genRaise,
	genBlazeRaise,
	genOpen,
	genBlazeOpen,
	genClose,
	genBlazeClose,
	genCdO,
	genBlazeCdO,
} vldoor_e;

typedef struct
{
	thinker_t thinker;
	vldoor_e type;
	sector_t* sector;
	fixed_t topheight;
	fixed_t speed;
	
	// 1 = up, 0 = waiting at top, -1 = down
	int direction;
	
	// tics to wait at the top
	int topwait;
	// (keep in case a door going down is reset)
	// when it reaches 0, start going down
	int topcountdown;
	
	//SoM: 3/6/2000: the line that triggered the door.
	line_t* line;
} vldoor_t;

#define VDOORSPEED              (FRACUNIT*2)
#define VDOORWAIT               150

void EV_OpenDoor(int sectag, int speed, int wait_time);
void EV_CloseDoor(int sectag, int speed);

void T_VerticalDoor(vldoor_t* door);
vldoor_t* P_SpawnDoorCloseIn(sector_t* sec, const uint32_t a_Tics, const uint32_t a_Type);
void P_SpawnDoorCloseIn30(sector_t* sec);

vldoor_t* P_SpawnDoorRaiseIn(sector_t* sec, const bool_t a_InitWait, const uint32_t a_Tics, const uint32_t a_Type);
void P_SpawnDoorRaiseIn5Mins(sector_t* sec, int secnum);

#if 0							// UNUSED
//
//      Sliding doors...
//
typedef enum
{
	sd_opening,
	sd_waiting,
	sd_closing
} sd_e;

typedef enum
{
	sdt_openOnly,
	sdt_closeOnly,
	sdt_openAndClose
} sdt_e;

typedef struct
{
	thinker_t thinker;
	sdt_e type;
	line_t* line;
	int frame;
	int whichDoorIndex;
	int timer;
	sector_t* frontsector;
	sector_t* backsector;
	sd_e status;
	
} slidedoor_t;

typedef struct
{
	char frontFrame1[9];
	char frontFrame2[9];
	char frontFrame3[9];
	char frontFrame4[9];
	char backFrame1[9];
	char backFrame2[9];
	char backFrame3[9];
	char backFrame4[9];
	
} slidename_t;

typedef struct
{
	int frontFrames[4];
	int backFrames[4];
	
} slideframe_t;

// how many frames of animation
#define SNUMFRAMES              4

#define SDOORWAIT               (35*3)
#define SWAITTICS               4

// how many diff. types of anims
#define MAXSLIDEDOORS   5

void P_InitSlidingDoorFrames(void);

void EV_SlidingDoor(line_t* line, mobj_t* thing);
#endif

//
// P_CEILNG
//
typedef enum
{
	lowerToFloor,
	raiseToHighest,
	//SoM:3/4/2000: Extra boom stuffs that tricked me...
	lowerToLowest,
	lowerToMaxFloor,
	
	lowerAndCrush,
	crushAndRaise,
	fastCrushAndRaise,
	silentCrushAndRaise,
	instantRaise,				// Insantly raises SSNTails 06-13-2002
	
	//SoM:3/4/2000
	//jff 02/04/98 add types for generalized ceiling mover
	genCeiling,
	genCeilingChg,
	genCeilingChg0,
	genCeilingChgT,
	
	//jff 02/05/98 add types for generalized ceiling mover
	genCrusher,
	genSilentCrusher,
	
} ceiling_e;

typedef struct
{
	thinker_t thinker;
	ceiling_e type;
	sector_t* sector;
	fixed_t bottomheight;
	fixed_t topheight;
	fixed_t speed;
	fixed_t oldspeed;			//SoM: 3/6/2000
	bool_t crush;
	
	//SoM: 3/6/2000: Support ceiling changers
	int newspecial;
	int oldspecial;
	short texture;
	
	// 1 = up, 0 = waiting, -1 = down
	int direction;
	
	// ID
	int tag;
	int olddirection;
	
	struct ceilinglist* list;	// SoM: 3/6/2000: by jff: copied from killough's plats
} ceiling_t;

//SoM: 3/6/2000: Boom's improved ceiling list.
typedef struct ceilinglist
{
	ceiling_t* ceiling;
	struct ceilinglist* next, **prev;
} ceilinglist_t;

void P_RemoveAllActiveCeilings(void);	//SoM: 3/9/2000

#define CEILSPEED               (FRACUNIT)
#define CEILWAIT                150
#define MAXCEILINGS             30

extern ceilinglist_t* activeceilings;	//SoM: 3/6/2000: New improved boom code.

int EV_DoCeiling(line_t* line, ceiling_e type);

void T_MoveCeiling(ceiling_t* ceiling);
void P_AddActiveCeiling(ceiling_t* ceiling);
void P_RemoveActiveCeiling(ceiling_t* ceiling);
int EV_CeilingCrushStop(line_t* line);
int P_ActivateInStasisCeiling(line_t* line);

//
// P_FLOOR
//
typedef enum
{
	// lower floor to highest surrounding floor
	lowerFloor,
	
	// lower floor to lowest surrounding floor
	lowerFloorToLowest,
	
	// lower floor to highest surrounding floor VERY FAST
	turboLower,
	
	// raise floor to lowest surrounding CEILING
	raiseFloor,
	
	// raise floor to next highest surrounding floor
	raiseFloorToNearest,
	
	// lower floor to lowest surrounding floor
	lowerFloorToNearest,
	
	// lower floor 24
	lowerFloor24,
	
	// lower floor 32
	lowerFloor32Turbo,
	
	// raise floor to shortest height texture around it
	raiseToTexture,
	
	// lower floor to lowest surrounding floor
	//  and change floorpic
	lowerAndChange,
	
	raiseFloor24,
	
	//raise floor 32
	raiseFloor32Turbo,
	
	raiseFloor24AndChange,
	raiseFloorCrush,
	
	// raise to next highest floor, turbo-speed
	raiseFloorTurbo,
	donutRaise,
	raiseFloor512,
	instantLower,				// Instantly lowers SSNTails 06-13-2002
	
	//SoM: 3/4/2000 Boom copy YEAH YEAH
	genFloor,
	genFloorChg,
	genFloorChg0,
	genFloorChgT,
	
	//new types for stair builders
	buildStair,
	genBuildStair,
	
} floor_e;

//SoM:3/4/2000: Anothe boom code copy.
typedef enum
{
	elevateUp,
	elevateDown,
	elevateCurrent,
} elevator_e;

typedef enum
{
	build8,						// slowly build by 8
	turbo16						// quickly build by 16
} stair_e;

typedef struct
{
	thinker_t thinker;
	floor_e type;
	bool_t crush;
	sector_t* sector;
	int direction;
	int newspecial;
	int oldspecial;				//SoM: 3/6/2000
	short texture;
	fixed_t floordestheight;
	fixed_t speed;
	
} floormove_t;

typedef struct					//SoM: 3/6/2000: Elevator struct.
{
	thinker_t thinker;
	uint32_t type;								// ReMooD Enhanced
	sector_t* sector;
	int direction;
	fixed_t floordestheight;
	fixed_t ceilingdestheight;
	fixed_t speed;
	bool_t Silent;								// Silent
	fixed_t PerpWait;							// Perp Wait Amount
	fixed_t PerpTicsLeft;						// Time left until perp moves
	line_t* CallLine;							// Calling Line
	fixed_t PDoorSpeed;							// Elevator Door speed
	int OldDirection;							// Old movement direction
	bool_t Dinged;								// Elevator Dinged
} elevator_t;

#define ELEVATORSPEED (FRACUNIT*4)	//SoM: 3/6/2000
#define FLOORSPEED    (FRACUNIT)

typedef enum
{
	ok,
	crushed,
	pastdest
} result_e;

result_e T_MovePlane(sector_t* sector, fixed_t speed, fixed_t dest, bool_t crush, int floorOrCeiling, int direction);

int EV_BuildStairs(line_t* line, stair_e type);

int EV_DoChange(line_t* line, change_e changetype);	//SoM: 3/16/2000

void T_MoveFloor(floormove_t* floor);

//SoM: New thinker functions.
void T_MoveElevator(elevator_t* elevator);

//
// P_TELEPT
//

//SoM: 3/15/2000: Boom silent teleport functions

int EV_PortalTeleport(line_t* line, mobj_t* thing, int side);

/* SoM: 3/4/2000: This is a large section of copied code. Sorry if this offends people, but
   I really don't want to read, understand and rewrite all the changes to the source and entire
   team made! Anyway, this is for the generalized linedef types. */

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80
#define SECRET_SHIFT    7
#define FRICTION_MASK   0x100
#define FRICTION_SHIFT  8
#define PUSH_MASK       0x200
#define PUSH_SHIFT      9

/* REXWindPushDir_t -- Wind push direction */
typedef enum REXWindPushDir_s
{
	RWXWPD_EAST,
	RWXWPD_NORTHEAST,
	RWXWPD_NORTH,
	RWXWPD_NORTHWEST,
	RWXWPD_WEST,
	RWXWPD_SOUTHWEST,
	RWXWPD_SOUTH,
	RWXWPD_SOUTHEAST,
	
	NUMREXWINDPUSHDIRS
} REXWindPushDir_t;

#define REXEXIT_MASK		UINT32_C(0x1000)

#define REXS_DIRMASK		UINT32_C(0xE000)
#define REXS_DIRSHIFT		UINT32_C(13)

#define REXS_SCROLLMASK		UINT32_C(0x10000)
#define REXS_SCROLLSHIFT	UINT32_C(16)

#define REXS_WINDMASK		UINT32_C(0x20000)
#define REXS_WINDSHIFT		UINT32_C(17)

#define REXS_SPEEDMASK		UINT32_C(0x1C0000)
#define REXS_SPEEDSHIFT		UINT32_C(18)

#define REXS_HFRICTMASK		UINT32_C(0x200000)
#define REXS_HFRICTSHIFT	UINT32_C(21)

//jff 02/04/98 Define masks, shifts, for fields in
// generalized linedef types

#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3c00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift      0

// define masks and shifts for the floor type fields

#define FloorCrush            0x1000
#define FloorChange           0x0c00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3

// define masks and shifts for the ceiling type fields

#define CeilingCrush          0x1000
#define CeilingChange         0x0c00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields

/* PGL_GenLiftDelay_t -- General Lift Delay */
typedef enum PGL_GenLiftDelay_e
{
	PGLGLD_WAITONE,
	PGLGLD_WAITTHREE,
	PGLGLD_WAITFIVE,
	PGLGLD_WAITTEN,
} PGL_GenLiftDelay_t;

#define LiftTarget            0x0300
#define LiftDelay             0x00c0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftMonsterShift           5
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields

/* PGL_GenStairStep_t -- General Stair Step */
typedef enum PGL_GenStairStep_e
{
	PGLGSS_STEPFOUR,
	PGLGSS_STEPEIGHT,
	PGLGSS_STEPSIXTEEN,
	PGLGSS_STEPTWENTYFOUR,
} PGL_GenStairStep_t;

#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00c0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairMonsterShift          5
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields

#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherMonsterShift        5
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields

#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorMonsterShift           7
#define DoorKindShift              5
#define DoorSpeedShift             3

/* PGL_GenDoorDelay_t -- General Door Delay */
typedef enum PGL_GenDoorDelay_e
{
	PGLGOD_WAITONE,
	PGLGOD_WAITFOUR,
	PGLGOD_WAITNINE,
	PGLGOD_WAITTHIRTY,
} PGL_GenDoorDelay_t;

/* PGL_GenDoorKind_t -- General Door Kind */
typedef enum PGL_GenDoorKind_e
{
	PGLGDK_ODC,
	PGLGDK_O,
	PGLGDK_CDO,
	PGLGDK_C,
} PGL_GenDoorKind_t;

/* PGL_GenLockedDoorKind_t -- Kind of locked door */
typedef enum PGL_GenLockedDoorKind_e
{
	PGLGLDK_ODC,
	PGLGLDK_O,
} PGL_GenLockedDoorKind_t;

// define masks and shifts for the locked door type fields

#define LockedNKeys           0x0200
#define LockedKey             0x01c0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

/* EV_TryGenType_t -- Trigger attempt type */
typedef enum EV_TryGenType_e
{
	LAT_WALK,									// Attempt to walk through
	LAT_SWITCH,								// Attempt to switch
	LAT_SHOOT,								// Attempt to shoot
	LAT_MAPSTART,								// Map Initialization
		
	NUMEVTRYGENTYPES
} EV_TryGenType_t;

/* EV_MapStartSides_t -- Side IDs for map start */
typedef enum EV_MapStartSides_e
{
	PMSS_BASE = -5,								// Base	
	
	PMSS_SCROLLERS = -4,						// Line scrollers
	PMSS_FRICTION = -3,							// Friction
	PMSS_PUSHERS = -2,							// Pushers
	
	PMSS_GENERAL = -1,							// General Stuff
} EV_MapStartSides_t;

/* EV_TryGenTypeFlags_t -- Flags for TryGenType */
typedef enum EV_TryGenTypeFlags_e
{
	EVTGTF_FORCEUSE					= 0x0001,	// Force trigger
} EV_TryGenTypeFlags_t;

//SoM: 3/9/2000: p_genlin

int EV_DoGenFloor(line_t* line, mobj_t* const a_Object);
int EV_DoGenCeiling(line_t* line, mobj_t* const a_Object);
int EV_DoGenLift(line_t* line, mobj_t* const a_Object);
int EV_DoGenStairs(line_t* line, mobj_t* const a_Object);
int EV_DoGenCrusher(line_t* line, mobj_t* const a_Object);
int EV_DoGenDoor(line_t* line, mobj_t* const a_Object);
int EV_DoGenLockedDoor(line_t* line, mobj_t* const a_Object);

void EV_ClearACSTags(void);
void EV_TagACSLine(line_t* const a_Line, const int32_t a_ID);
line_t* EV_SearchACSTags(const int32_t a_ID, int32_t* const a_SearchPoint);


typedef bool_t (*EV_Action_t)(line_t* const, const int, mobj_t* const, const EV_TryGenType_t, const uint32_t, bool_t* const);

bool_t EV_DoHexenLine(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain);
bool_t EV_TryGenTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain);

void P_ProcessSpecialSectorEx(const EV_TryGenType_t a_Type, mobj_t* const a_Mo, player_t* const a_Player, sector_t* const a_Sector, const bool_t a_InstaDamage);

/****** EXTENDED HIGH GENERALIZATION ******/

/*** CONSTANTS ***/

/* EV_GenHEType_t -- Generalized High Extended Type */
// Consumes 5 of 32 bits, 27 left
typedef enum EV_GenHEType_e
{
	EVGHET_NULL,								//  0 Do not use
	
	EVGHET_XDOOR,								//  1 XR Door
	EVGHET_YFAKEFLOORS,							//  2 MM 3D Floors
	EVGHET_XLOCKEDDOOR,							//  3 XR Locked Door
	EVGHET_YTRANSFER,							//  4 MM Transfers
	EVGHET_XFLOOR,								//  5 XR Floor
	EVGHET_YSCROLLER,							//  6 MM Scrollers
	EVGHET_XCEILING,							//  7 XR Ceiling
	EVGHET_YEFFECTS,							//  8 MM Line Effects
	EVGHET_XPLAT,								//  9 XR Platform [Replaces EV_DoPlat]
	EVGHET_YTEAMSTARTS,							// 10 MM Team Starts
	EVGHET_XCRUSHER,							// 11 XR Crusher
	EVGHET_YUNUSED12,							// 12 MM Unused
	EVGHET_XSTAIR,								// 13 XR Stair
	EVGHET_YUNUSED14,							// 14 MM Unused
	EVGHET_XELEVATOR,							// 15 XR Elevator
	EVGHET_YUNUSED16,							// 16 MM Unused
	EVGHET_XLIGHTS,								// 17 XR Lights
	EVGHET_YUNUSED18,							// 18 MM Unused
	EVGHET_XEXIT,								// 19 XR Exit
	EVGHET_YUNUSED20,							// 20 MM Unused
	EVGHET_XTELEPORT,							// 21 XR Teleporter
	EVGHET_YUNUSED22,							// 22 MM Unused
	EVGHET_XDONUT,								// 23 XR Donut
	EVGHET_YUNUSED24,							// 24 MM Unused
	EVGHET_XSCRIPTS,							// 25 XR Scripts
	EVGHET_YSCRIPTS,							// 26 MM Scripts
	EVGHET_XDAMAGE,								// 27 XR Damage Objects
} EV_GenHEType_t;

/* EV_GenHETrig_t -- Generalized High Extended Trigger */
// Consumes 3 of 32 bits, 24 left
typedef uint32_t EV_GenHEActivator_t;

/* EV_GenHESpeed_t -- Generalized Speed */
// Consumes 3 of 32 bits
typedef enum EV_GenHESpeed_e
{
	EVGHES_SLOWEST,								// 000 Speed / 8
	EVGHES_SLOWER,								// 001 Speed / 4
	EVGHES_SLOW,								// 010 Speed / 2
	EVGHES_NORMAL,								// 011 Speed
	EVGHES_FAST,								// 100 Speed * 2
	EVGHES_FASTER,								// 101 Speed * 4
	EVGHES_FASTEST,								// 110 Speed * 8
	EVGHES_INSTANT,								// 111 Instant
} EV_GenHESpeed_t;

/* EV_GenHEFCDWait_t -- Wait Delay */
// Consumes 3 of 32 bits
typedef enum EV_GenHEFCDWait_e
{
	EVGHEFCDW_WAIT1S,							// 000 Wait 1 Seconds
	EVGHEFCDW_WAIT3S,							// 001 Wait 3 Seconds
	EVGHEFCDW_WAIT4S,							// 010 Wait 4 Seconds
	EVGHEFCDW_WAIT5S,							// 011 Wait 5 Seconds
	EVGHEFCDW_WAIT9S,							// 100 Wait 9 Seconds
	EVGHEFCDW_WAIT10S,							// 101 Wait 10 Seconds
	EVGHEFCDW_WAIT30S,							// 110 Wait 30 Seconds
	EVGHEFCDW_WAIT60S,							// 111 Wait 60 Seconds
} EV_GenHEFCDWait_t;

/* EV_GenHEFType_t -- Floor/Ceiling/Plat Type */
// Uses 4 bits of 32
typedef enum EV_GenHEFCPType_e
{
	EVGHEPLATT_LHFPERP,							// Perpetual lower to higher
	EVGHEPLATT_CEILTOGGLE,						// Ceiling Toggle
	EVGHEPLATT_DWUS,								// Down wait up stay
	EVGHEPLATT_RAISE24,							// Raise and Change by 24
	EVGHEPLATT_RAISE32,							// Raise and Change by 32
	EVGHEPLATT_NNF,								// Next to Next Floor
} EV_GenHEFType_t;

/* EV_GenHEXFerType_t -- Generalized transfer type */
typedef enum EV_GenHEXFerType_e
{
	EVGHEXFT_FLIGHT,
	EVGHEXFT_FRICTION,
	EVGHEXFT_WIND,
	EVGHEXFT_CURRENT,
	EVGHEXFT_POINTFORCE,
	EVGHEXFT_HEIGHTS,
	EVGHEXFT_TRANSLUCENCY,
	EVGHEXFT_CLIGHT,
	EVGHEXFT_SKY,
	EVGHEXFT_DRAWHEIGHTS,
	EVGHEXFT_CREATECOLORMAP,
	EVGHEXFT_TRANSID1,
	EVGHEXFT_TRANSID2,
	EVGHEXFT_TRANSID3,
	EVGHEXFT_TRANSID4,
	EVGHEXFT_TRANSID5,
	EVGHEXFT_UPCURRENT,
	EVGHEXFT_DOWNCURRENT,
	EVGHEXFT_UPWIND,
	EVGHEXFT_DOWNWIND,
	EVGHEXFT_SKYFLIPPED,
} EV_GenHEXFerType_t;

/* EV_GenHEElevType_t -- Generic Elevator Type */
typedef enum EV_GenHEElevType_e
{
	EVGHEELEVT_UP,
	EVGHEELEVT_DOWN,
	EVGHEELEVT_CURRENT,
	EVGHEELEVT_PERPUP,
	EVGHEELEVT_PERPDOWN,
	EVGHEELEVT_CALLPERP,
	EVGHEELEVT_STOPPERP,
} EV_GenHEElevType_t;

/*** SHIFTS ***/

// BASE
#define EVGENHE_TYPEMASK			UINT32_C(0xF8000000)
#define EVGENHE_TYPESHIFT			UINT32_C(27)
#define EVGENHE_TYPEBASE(n)			((((uint32_t)(n)) << EVGENHE_TYPESHIFT) & EVGENHE_TYPEMASK)

#define EVGENGE_TRIGMASK			UINT32_C(0x00000007)
#define EVGENGE_TRIGSHIFT			UINT32_C(0)

#define EVGENGE_MONSTERMASK			UINT32_C(0x00000008)
#define EVGENGE_MONSTERSHIFT		UINT32_C(3)

#define EVGENGE_PLAYERMASK			UINT32_C(0x00000010)
#define EVGENGE_PLAYERSHIFT			UINT32_C(4)

#define EVGENGE_SPEEDMASK			UINT32_C(0x000000E0)
#define EVGENGE_SPEEDSHIFT			UINT32_C(5)

// FLOOR/CEILING/DOOR Wait Delay
#define EVGENGE_FCDWAITMASK			UINT32_C(0x00000700)
#define EVGENGE_FCDWAITSHIFT		UINT32_C(8)

// XFLOOR Stuff
#define EVGENGE_FCFMODEMASK			UINT32_C(0x00003800)
#define EVGENGE_FCFMODESHIFT		UINT32_C(11)

// XPLAT Stuff
#define EVGENGE_FCPTYPEMASK			UINT32_C(0x00003800)
#define EVGENGE_FCPTYPESHIFT		UINT32_C(11)
#define EVGENGE_FCPMODEMASK			UINT32_C(0x0001C000)
#define EVGENGE_FCPMODESHIFT		UINT32_C(14)

// XDAMAGE Stuff
#define EVGENHE_XDAMGMASK			UINT32_C(0x00FFFF00)
#define EVGENHE_XDAMGSHIFT			UINT32_C(8)

// XEXIT Stuff
#define EVGENGE_EXITSECRETMASK		UINT32_C(0x00000020)
#define EVGENGE_EXITSECRETSHIFT		UINT32_C(5)

#define EVGENGE_EXITHUBMASK			UINT32_C(0x00000040)
#define EVGENGE_EXITHUBSHIFT		UINT32_C(6)

// XTELEPORT Stuff
#define EVGENGE_TELESILENTMASK		UINT32_C(0x00000020)
#define EVGENGE_TELESILENTSHIFT		UINT32_C(5)
#define EVGENGE_TELEREVERSEMASK		UINT32_C(0x00000040)
#define EVGENGE_TELEREVERSESHIFT	UINT32_C(6)
#define EVGENGE_TELELWSTMASK		UINT32_C(0x00000080)
#define EVGENGE_TELELWSTSHIFT		UINT32_C(7)

// XTRANSFER
#define EVGENGE_TRANSFERMASK		UINT32_C(0x0000FFFF)
#define EVGENGE_TRANSFERSHIFT		UINT32_C(0)

// XELEVATOR
#define EVGENGE_ELEVSILENTMASK		UINT32_C(0x00000100)
#define EVGENGE_ELEVSILENTSHIFT		UINT32_C(8)
#define EVGENGE_ELEVTYPEMASK		UINT32_C(0x00000E00)
#define EVGENGE_ELEVTYPESHIFT		UINT32_C(9)
#define EVGENGE_ELEVWAITMASK		UINT32_C(0x00007000)
#define EVGENGE_ELEVWAITSHIFT		UINT32_C(12)
#define EVGENGE_ELEVDOORMASK		UINT32_C(0x00008000)
#define EVGENGE_ELEVDOORSHIFT		UINT32_C(15)
#define EVGENGE_ELEVDSPEEDMASK		UINT32_C(0x00070000)
#define EVGENGE_ELEVDSPEEDSHIFT		UINT32_C(16)

/******************************************/

// define names for the TriggerType field of the general linedefs

typedef enum
{
	WalkOnce,	// 000
	WalkMany,	// 001
	SwitchOnce, // 010
	SwitchMany, // 011
	GunOnce,    // 100
	GunMany,    // 101
	PushOnce,   // 110
	PushMany,   // 111
} triggertype_e;

// define names for the Speed field of the general linedefs

typedef enum
{
	SpeedSlow,
	SpeedNormal,
	SpeedFast,
	SpeedTurbo,
} motionspeed_e;

// define names for the Target field of the general floor

typedef enum
{
	FtoHnF,
	FtoLnF,
	FtoNnF,
	FtoLnC,
	FtoC,
	FbyST,
	Fby24,
	Fby32,
} floortarget_e;

// define names for the Changer Type field of the general floor

typedef enum
{
	FNoChg,
	FChgZero,
	FChgTxt,
	FChgTyp,
} floorchange_e;

// define names for the Change Model field of the general floor

typedef enum
{
	FTriggerModel,
	FNumericModel,
} floormodel_t;

// define names for the Target field of the general ceiling

typedef enum
{
	CtoHnC,
	CtoLnC,
	CtoNnC,
	CtoHnF,
	CtoF,
	CbyST,
	Cby24,
	Cby32,
} ceilingtarget_e;

// define names for the Changer Type field of the general ceiling

typedef enum
{
	CNoChg,
	CChgZero,
	CChgTxt,
	CChgTyp,
} ceilingchange_e;

// define names for the Change Model field of the general ceiling

typedef enum
{
	CTriggerModel,
	CNumericModel,
} ceilingmodel_t;

// define names for the Target field of the general lift

typedef enum
{
	F2LnF,
	F2NnF,
	F2LnC,
	LnF2HnF,
} lifttarget_e;

// define names for the door Kind field of the general ceiling

typedef enum
{
	OdCDoor,
	ODoor,
	CdODoor,
	CDoor,
} doorkind_e;

// define names for the locked door Kind field of the general ceiling

typedef enum
{
	AnyKey_,
	RCard,
	BCard,
	YCard,
	RSkull,
	BSkull,
	YSkull,
	AllKeys,
} keykind_e;

/* SoM: End generalized linedef code */

//SoM: 3/8/2000: Add generalized scroller code
typedef struct
{
	thinker_t thinker;			// Thinker structure for scrolling
	fixed_t dx, dy;				// (dx,dy) scroll speeds
	int affectee;				// Number of affected sidedef, sector, tag, or whatever
	int control;				// Control sector (-1 if none) used to control scrolling
	fixed_t last_height;		// Last known height of control sector
	fixed_t vdx, vdy;			// Accumulated velocity if accelerative
	int accel;					// Whether it's accelerative
	enum
	{
		sc_side,
		sc_floor,
		sc_ceiling,
		sc_carry,
		sc_carry_ceiling,
	} type;
} scroll_t;

void T_Scroll(scroll_t* s);

//SoM: 3/8/2000: added new model of friction for ice/sludge effects

typedef struct
{
	thinker_t thinker;			// Thinker structure for friction
	int friction;				// friction value (E800 = normal)
	int movefactor;				// inertia factor when adding to momentum
	int affectee;				// Number of affected sector
} friction_t;

//SoM: Friction defines.
#define MORE_FRICTION_MOMENTUM 15000	// mud factor based on momentum
#define ORIG_FRICTION          0xE800	// original value
#define ORIG_FRICTION_FACTOR   2048	// original value
#define PUSH_FACTOR 7

//SoM: 3/9/2000: Otherwise, the compiler whines!
void T_Friction(friction_t* f);

//SoM: 3/8/2000: Model for Pushers for push/pull effects

typedef struct
{
	thinker_t thinker;			// Thinker structure for Pusher
	enum
	{
		p_push,
		p_pull,
		p_wind,
		p_current,
		p_upcurrent,			// SSNTails 06-10-2002
		p_downcurrent,			// SSNTails 06-10-2002
		p_upwind,				// SSNTails 06-10-2003 WOAH! EXACTLY ONE YEAR LATER! FREAKY!
		p_downwind,				// SSNTails 06-10-2003
	} type;
	mobj_t* source;				// Point source if point pusher
	int x_mag;					// X Strength
	int y_mag;					// Y Strength
	int magnitude;				// Vector strength for point pusher
	int radius;					// Effective radius for point pusher
	int x;						// X of point source if point pusher
	int y;						// Y of point source if point pusher
	int affectee;				// Number of affected sector
} pusher_t;

//SoM: 3/9/2000: Prototype functions for pushers
bool_t PIT_PushThing(mobj_t* thing, void* a_Arg);
void T_Pusher(pusher_t* p);
mobj_t* P_GetPushThing(int s);

bool_t P_Teleport(mobj_t* thing, fixed_t x, fixed_t y, angle_t angle);

//SoM: 3/16/2000
void P_CalcHeight(player_t* player);

// heretic stuff
void P_InitLava(void);
void P_AmbientSound(void);
void P_AddAmbientSfx(int sequence);
void P_InitAmbientSound(void);

/*****************************************************************************/

extern ffloor_t** g_PFakeFloors;				// Fake Floors
extern size_t g_NumPFakeFloors;					// Number of them

int32_t P_GetIDFromFFloor(ffloor_t* const a_FFloor);
ffloor_t* P_GetFFloorFromID(const int32_t a_ID);

/*****************************************************************************/

bool_t EV_VerticalDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_DoDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_DoLockedDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_DoFloor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_DoPlat(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_Teleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_SilentTeleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

bool_t EV_SilentLineTeleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

/*****************************************************************************/

/*** CONSTANTS ***/

/* P_EXSLineTrigger_t -- Trigger for line */
typedef enum P_EXSLineTrigger_s
{
	PWXSLT_WALK,								// Walk
	PWXSLT_SWITCH,								// Press button
	PWXSLT_GUN,									// Shoot
	PWXSLT_PUSH,								// Push on it
	PWXSLT_MAP,									// Done at map start
	
	NUMPEXSLINETRIGGERS
} P_EXSLineTrigger_t;

/*** STRUCTURES ***/

/*** FUNCTIONS ***/

void P_ExtraSpecialStuff(void);

/*****************************************************************************/

#endif

