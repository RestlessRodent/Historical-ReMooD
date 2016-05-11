// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Profiles

#ifndef __D_PROF_H__
#define __D_PROF_H__

#include "doomtype.h"

#include "j.h"

/** Compatibility to specify the max profiles in the menu. */
#define MAXPROFCONST 24

typedef struct D_Prof_s
{
	/** The associated Java object for the profile. */
	jobject java;
	
	/** The profile display name and cache. */
	jstring displayname;
	const char* displaynamecache;
	
	/** The account name and cache. */
	jstring accountname;
	const char* accountnamecache;
	
	/** The UUID. */
	jstring uuid;
	const char* uuidcache;
} D_Prof_t;


/* Key bits for profiles */
// This gives extra keys and such for each action performed
#define PRFKBIT_MASK	UINT32_C(0xFFFFF000)
#define PRFKBIT_VMASK	UINT32_C(0x00000FFF)
#define PRFKBIT_KEY		UINT32_C(0x00000000)
#define PRFKBIT_JOY		UINT32_C(0x00001000)
#define PRFKBIT_MOUSE	UINT32_C(0x00002000)
#define PRFKBIT_DMOUSE	UINT32_C(0x00003000)
#define PRFKBIT_JOYP	UINT32_C(0x00004000)
#define PRFKBIT_MOUSEP	UINT32_C(0x00005000)
#define PRFKBIT_DMOUSEP	UINT32_C(0x00006000)
#define PRFKBIT_KEYP	UINT32_C(0x00007000)
#define PRFKBIT_JOYX	UINT32_C(0x00008000)
#define PRFKBIT_MOUSEX	UINT32_C(0x00009000)
#define PRFKBIT_DMOUSEX	UINT32_C(0x0000A000)
#define PRFKBIT_KEYX	UINT32_C(0x0000B000)

/* D_ProfileExFlags_t -- Extended profile flags */
typedef enum D_ProfileExFlags_e
{
	DPEXF_GOTMOUSE				= 0x00000001,	// Has control of the mouse
	DPEXF_GOTJOY				= 0x00000002,	// Controls a joystick
	DPEXF_PLAYING				= 0x00000004,	// Is playing the game
	DPEXF_DONTSAVE				= 0x00000010,	// Don't save in configs
	DPEXF_DEFAULTKEYS			= 0x00000020,	// Default Keys here!
	DPEXF_DUMPALL				= 0x00000040,	// Dump everything
} D_ProfileExFlags_t;

/* D_ProfileExBotFlags_t -- Bot Flags */
typedef enum D_ProfileExBotFlags_e
{
	DPEXBOTF_MOUSE				= 0x00000001,	// Bot "uses" a mouse
	DPEXBOTF_TURNAROUND			= 0x00000002,	// Bot can do 180 degree turns
} D_ProfileExBotFlags_t;

/* D_ProfileExInputCtrl_t -- Input control */
typedef enum D_ProfileExInputCtrl_e
{
	/* Modifiers */
	DPEXIC_SPEED,
	DPEXIC_MOVEMENT,
	DPEXIC_LOOKING,
	
	/* Movement */
	DPEXIC_FORWARDS,
	DPEXIC_BACKWARDS,
	DPEXIC_STRAFELEFT,
	DPEXIC_STRAFERIGHT,
	DPEXIC_FLYUP,
	DPEXIC_FLYDOWN,
	DPEXIC_LAND,
	DPEXIC_JUMP,
	
	/* Looking */
	DPEXIC_TURNLEFT,
	DPEXIC_TURNRIGHT,
	DPEXIC_TURNSEMICIRCLE,
	DPEXIC_LOOKUP,
	DPEXIC_LOOKDOWN,
	DPEXIC_LOOKCENTER,
	
	/* Actions */
	DPEXIC_USE,
	DPEXIC_SUICIDE,
	DPEXIC_TAUNT,
	DPEXIC_CHAT,
	DPEXIC_TEAMCHAT,
	
	/* Weapons */
	DPEXIC_ATTACK,
	DPEXIC_ALTATTACK,
	DPEXIC_RELOAD,
	DPEXIC_SWITCHFIREMODE,
	DPEXIC_SLOT1,
	DPEXIC_SLOT2,
	DPEXIC_SLOT3,
	DPEXIC_SLOT4,
	DPEXIC_SLOT5,
	DPEXIC_SLOT6,
	DPEXIC_SLOT7,
	DPEXIC_SLOT8,
	DPEXIC_SLOT9,
	DPEXIC_SLOT10,
	DPEXIC_NEXTWEAPON,
	DPEXIC_PREVWEAPON,
	DPEXIC_BESTWEAPON,
	DPEXIC_WORSTWEAPON,
	
	/* Inventory */
	DPEXIC_NEXTINVENTORY,
	DPEXIC_PREVINVENTORY,
	DPEXIC_USEINVENTORY,
	DPEXIC_CANCELINVENTORY,
	
	/* General */
	DPEXIC_TOPSCORES,							// Show the best players
	DPEXIC_BOTTOMSCORES,						// Show the worst players
	DPEXIC_COOPSPY,								// Coop Spy
	DPEXIC_AUTOMAP,								// Toggle Automap
	DPEXIC_CHATMODE,							// Chat Mode
	DPEXIC_POPUPMENU,							// Popup the menu
	DPEXIC_MORESTUFF,							// Access to more stuff
	DPEXIC_QUICKMENU,							// Perfect Dark-like Quick Menu
	DPEXIC_MOREMORESTUFF,						// Access to even more stuff
	
	DPEXIC_NULL,
	
	NUMDPROFILEEXINPUTCTRLS
} D_ProfileExInputCtrl_t;

/* D_ProfileExCtrlMA_t -- Controlled mouse axis */
typedef enum D_ProfileExCtrlMA_e
{
	DPEXCMA_NULL,								// Do Nothing
	DPEXCMA_MOVEX,								// Move on axis
	DPEXCMA_MOVEY,
	DPEXCMA_LOOKX,								// Look on axis
	DPEXCMA_LOOKY,
	
	DPEXCMA_NEGMOVEX,							// Negative Move on axis
	DPEXCMA_NEGMOVEY,
	DPEXCMA_NEGLOOKX,							// Negative Look on axis
	DPEXCMA_NEGLOOKY,
	
	DPEXCMA_PANY,								// Pan on Y Axis (joys only)
	DPEXCMA_NEGPANY,
	
	DPEXCMA_ANGPANY,							// Log Pan on Y Axis (joys only)
	DPEXCMA_NEGANGPANY,
	
	NUMDPROFILEEXCTRLMAS
} D_ProfileExCtrlMA_t;

#define MAXJOYAXIS 8

/** Get the display name. */
const char* D_ProfileDisplayName(D_Prof_t* __prof);

/** Get the account name. */
const char* D_ProfileAccountName(D_Prof_t* __prof);

/** Get profile by the appearance order index. */
D_Prof_t* D_ProfileGetIndex(int __i);

/** Rename profile to the given string. */
void D_ProfileRename(D_Prof_t* __prof, const char* __new);

/** Returns the view height of the profile. */
fixed_t D_ProfileViewHeight(D_Prof_t* __prof);

/** The account UUID. */
const char* D_ProfileUUID(D_Prof_t* __prof);

/** The profile's bob mode. */
int D_ProfileBobMode(D_Prof_t* __prof);

/** Draw gun sprite? */
bool_t D_ProfileDrawGunSprite(D_Prof_t* __prof);

/** Returns raw control information. */
int D_ProfileRawControl(D_Prof_t* __prof, int __key, int __i);

/** Obtains profile flags. */
int D_ProfileFlags(D_Prof_t* __prof);

/** Convert enumeration to input control. */
D_ProfileExInputCtrl_t D_ProfileEnumToInputCtrl(jobject __jo);

/** Convert input control to enumeration. */
jobject D_ProfileInputCtrlToControl(D_ProfileExInputCtrl_t __ctrl);

/** Use slow turning. */
bool_t D_ProfileUseSlowTurn(D_Prof_t* __prof);

/** Time to slow turn for. */
int D_ProfileSlowTurnTime(D_Prof_t* __prof);

/** Use auto-run? */
bool_t D_ProfileUseAutoRun(D_Prof_t* __prof);

/** Use look spring? */
bool_t D_ProfileUseLookSpring(D_Prof_t* __prof);

/** Look up/down speed. */
int D_ProfileLookUpDownSpeed(D_Prof_t* __prof);

#if 0

/************************
*** EXTENDED PROFILES ***
************************/

/*** CONSTANTS ***/

#define MAXPROFCONST 12							// Max quick access profiles

/* D_ProfAutoMapColors_t -- Automap colors */
typedef enum D_ProfAutoMapColors_e
{
	DPAMC_BACKGROUND,							// Background Color
	DPAMC_YOURPLAYER,							// Your Color
	DPAMC_THING,								// Thing Color
	DPAMC_ALLYTHING,							// Ally Thing Color
	DPAMC_ENEMYTHING,							// Enemy Thing Color
	DPAMC_PICKUP,								// Pickup Item
	DPAMC_SOLIDWALL,							// Wall
	DPAMC_FLOORSTEP,							// Floor Step
	DPAMC_CEILSTEP,								// Ceiling Step
	DPAMC_TRIGGER,								// Trigger line
	DPAMC_UNMAPPED,								// Unmapped line
	DPAMC_GRID,									// Blockmap grid color
	DPAMC_DEFAULT,								// Default Color
	DPAMC_INVISODWALL,							// Invisible double wall
	
	NUMPROFAUTOMAPCOLORS
} D_ProfAutoMapColors_t;

/* D_ProfBarType_t -- Status bar type */
typedef enum D_ProfBarType_e
{
	DPBT_DOOM,
	DPBT_REMOOD,
	
	NUMPROFBARS,
	
	DPBT_DEFAULT = DPBT_REMOOD,
} D_ProfBarType_t;

#define MAXALTAXIS		3
#define MAXMOUSEAXIS	2
#define MAXJOYAXIS		8

#if !defined(MAXUUIDLENGTH)
	#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)	// Length of UUIDs
#endif

/*** STRUCTURES ***/

struct mobj_s;

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* D_Prof_t -- Extended Profile */
struct D_Prof_s
{
	/* Profile Related */
	uint32_t Flags;								// Flags for profile controller
	char DisplayName[MAXPLAYERNAME];			// Name to show in network games
	char AccountName[MAXPLAYERNAME];			// Local account name (selection limited)
	uint8_t Color;								// Color
	uint8_t JoyControl;							// Which joystick player controls
	char UUID[MAXUUIDLENGTH + 1];				// Player Unique ID
	tic_t SlowTurnTime;							// Time to slow turn
	uint32_t InstanceID;						// Instance ID
	
	/* Controls For Player */
	uint32_t Ctrls[NUMDPROFILEEXINPUTCTRLS][4];	// Player Controls
	D_ProfileExCtrlMA_t MouseAxis[MAXALTAXIS][MAXMOUSEAXIS];	// Mouse Axis Movement
	D_ProfileExCtrlMA_t JoyAxis[MAXALTAXIS][MAXJOYAXIS];		// Joy Axis Movement
	int32_t MouseSens[2];						// Mouse Sensitivity
	int32_t JoySens[2];							// Joystick Sensitivity
	int32_t LookUpDownSpeed;					// Looking Up/Down Speed
	int32_t AutoGrabJoy;						// Auto-Grab Joystick
	
	/* Profile Chains */
	D_Prof_t* Prev;					// Previous link
	D_Prof_t* Next;					// Next link
	
	/* Other stuff */
	uint8_t ColorPickup;						// Color for pickups
	uint8_t ColorSecret;						// Secret Found Color
	char SoundSecret[MAXPLAYERNAME];			// Sound to play when Secret Found
	bool_t DrawPSprites;						// Draw Player Sprites
	int8_t BobMode;								// Bobbing Mode (Doom, Mid, Effort)
	fixed_t ViewHeight;							// View Height
	fixed_t CamDist, CamHeight, CamSpeed;		// Camera Properties
	bool_t ChaseCam;							// Enable chase camera
	bool_t TransSBar;							// Transparent Status Bar
	bool_t ScaledSBar;							// Scaled Status Bar
	char HexenClass[MAXPLAYERNAME];				// Hexen Class
	bool_t AutoRun;								// Autorun
	bool_t SlowTurn;							// Perform slow turning
	bool_t LookSpring;							// Spring back to center on move
	bool_t JoyAutoRun;							// Joystick auto-run
	uint8_t ColorLock[4];						// Door Lock Colors
	uint8_t VTeam;								// Virtual Team
	bool_t CounterOp;							// CounterOp player
	char* AccountRef, *DisplayRef;				// References to own name
	uint8_t BarType;							// Type of status bar
};

/*** GLOBALS ***/

extern D_Prof_t* g_KeyDefaultProfile;
extern D_Prof_t* g_ProfList[MAXPROFCONST];

/*** FUNCTIONS ***/

void D_ProfFixAccountName(char* const a_Buffer);

D_Prof_t* D_ProfFirst(void);

bool_t D_ProfRename(D_Prof_t* a_Prof, const char* const a_NewName);
D_Prof_t* D_CreateProfileEx(const char* const a_Name);

D_Prof_t* D_FindProfileEx(const char* const a_Name);
D_Prof_t* D_FindProfileExByInstance(const uint32_t a_ID);

void D_SaveProfileData(void (*a_WriteBack)(const char* const a_Buf, void* const a_Data), void* const a_Data);
int CLC_Profile(const uint32_t a_ArgC, const char** const a_ArgV);

#endif

#endif							/* __D_PROF_H__ */

