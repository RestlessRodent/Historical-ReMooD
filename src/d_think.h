// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: MapObj data. Map Objects or mobjs are actors, entities,

#ifndef __D_THINK__
#define __D_THINK__

#include "doomtype.h"

//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//
typedef void (*actionf_v) ();
typedef void (*actionf_p1) (void*);
typedef void (*actionf_p2) (void*, void*);
typedef void (*actionf_p3) (void*, void*, void*);
typedef void (*actionf_p4) (void*, void*, void*, void*);
typedef void (*actionf_p5) (void*, void*, void*, uint8_t, void*);

typedef union
{
	actionf_v acv;
	actionf_p1 acp1;
	actionf_p2 acp2;
	actionf_p3 acp3;
	actionf_p4 acp4;
	actionf_p5 acp5;
} actionf_t;

// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
typedef actionf_t think_t;

/* Define P_ThinkerType_t */
#if !defined(__REMOOD_PTT_DEFINED)
	typedef int P_ThinkerType_t;
	#define __REMOOD_PTT_DEFINED
#endif

/* P_ThinkerType_t -- Type of thinker this is */
enum P_ThinkerType_e
{
	PTT_CAP,									// Thinker Cap
	PTT_VERTICALDOOR,							// T_VerticalDoor/vldoor_t
	PTT_FIREFLICKER,							// T_FireFlicker/fireflicker_t
	PTT_LIGHTFLASH,								// T_LightFlash/lightflash_t
	PTT_STROBEFLASH,							// T_StrobeFlash/strobe_t
	PTT_GLOW,									// T_Glow/glow_t
	PTT_LIGHTFADE,								// T_LightFade/lightlevel_t
	PTT_MOVEFLOOR,								// T_MoveFloor/floormove_t
	PTT_MOVECEILING,							// T_MoveCeiling/ceiling_t
	PTT_PLATRAISE,								// T_PlatRaise/plat_t
	PTT_MOVEELEVATOR,							// T_MoveElevator/elevator_t
	PTT_SCROLL,									// T_Scroll/scroll_t
	PTT_FRICTION,								// T_Friction/friction_t
	PTT_PUSHER,									// T_Pusher/pusher_t
	PTT_MOBJ,									// P_MobjThinker/mobj_t
	PTT_DEFUNCT,								// Defunct Object
	PTT_DELETEME,								// Deletes this thing
	
	NUMPTHINKERTYPES
}; 

/* Define thinker_t */
#if !defined(__REMOOD_THINKERT_DEFINED)
	typedef struct thinker_s thinker_t;
	#define __REMOOD_THINKERT_DEFINED
#endif

// Doubly linked list of actors.
struct thinker_s
{
	P_ThinkerType_t Type;
	thinker_t* prev;
	thinker_t* next;
	think_t function;
};

extern thinker_t thinkercap;

extern thinker_t** g_ThinkerList;				// List of thinkers
extern size_t g_NumThinkerList;					// Thinkers in list

/* G_ThinkerInfo_t -- Thinker info (save games) */
typedef struct G_ThinkerInfo_s
{
	size_t Size;
	actionf_t Func;
} G_ThinkerInfo_t;

extern const G_ThinkerInfo_t g_ThinkerData[NUMPTHINKERTYPES];	// Thinker Data

actionf_t G_ThinkTypeToFunc(const P_ThinkerType_t a_Type);
P_ThinkerType_t G_ThinkFuncToType(actionf_t a_Func);

/* Define S_NoiseThinker_t */
#if !defined(__REMOOD_SNOISETHNK_DEFINED)
	typedef struct S_NoiseThinker_s S_NoiseThinker_t;
	#define __REMOOD_SNOISETHNK_DEFINED
#endif

/* S_NoiseThinker_t -- A thinker that makes noise */
struct S_NoiseThinker_s
{
	uint32_t Flags;				// Sound flags
	
	/* World Position */
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	/* Momenntum */
	// This is for doppler and such
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	
	/* Other things */
	fixed_t Pitch;				// Pitch modification
	fixed_t Volume;				// Volume modification
	angle_t Angle;				// Angle
};

#endif

