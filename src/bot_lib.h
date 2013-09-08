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
// Copyright (C) 2013-2013 ReMooD       <http://remood.org/>
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// THERE ARE TWO LICENSES AVAILABLE FOR THIS FILE ONLY WITH ADDITIONAL TERMS:
//
// * THIS HEADER MUST NOT BE REMOVED, REGARDLESS OF WHICH LICENSE YOU CHOOSE.
// * KEEPING THESE LICENSE DESCRIPTIONS IN THE HEADER WILL NOT AFFECT NOR FORCE
// * THE CHOICE OF THE LICENSE AS LONG AS IT IS ONE OF THE FOLLOWING LISTED.
//
//  GNU GENERAL PUBLIC LICENSE 3 OR LATER
//  * This program is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU General Public License
//  * as published by the Free Software Foundation; either version 3
//  * of the License, or (at your option) any later version.
//  * 
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
// 
//  SIMPLIFIED BSD LICENSE:
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are
//  * met: 
//  * 
//  * 1. Redistributions of source code must retain the above copyright notice,
//  *    this list of conditions and the following disclaimer. 
//  * 2. Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in the
//  *    documentation and/or other materials provided with the distribution. 
//  * 
//  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//  * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//  * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
//  * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  * 
//  * The views and conclusions contained in the software and documentation are
//  * those of the authors and should not be interpreted as representing
//  * official policies, either expressed or implied, of the ReMooD Project.
// ----------------------------------------------------------------------------
// DESCRIPTION: Bot Library Header

#ifndef __BOT_LIB_H__
#define __BOT_LIB_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

/* Not Included: Define Fixed Size Types */
#if !defined(__REMOOD_INCLUDED)
	#if defined(__GNUC__)
		typedef __INT8_TYPE__ int8_t;
		typedef __INT16_TYPE__ int16_t;
		typedef __INT32_TYPE__ int32_t;
		typedef __INT64_TYPE__ int64_t;
		typedef __UINT8_TYPE__ uint8_t;
		typedef __UINT16_TYPE__ uint16_t;
		typedef __UINT32_TYPE__ uint32_t;
		typedef __UINT64_TYPE__ uint64_t;
	
		#define INT8_C(x) __INT8_C(x)
		#define INT16_C(x) __INT16_C(x)
		#define INT32_C(x) __INT32_C(x)
		#define INT64_C(x) __INT64_C(x)
		#define UINT8_C(x) __UINT8_C(x)
		#define UINT16_C(x) __UINT16_C(x)
		#define UINT32_C(x) __UINT32_C(x)
		#define UINT64_C(x) __UINT64_C(x)
	#endif
	
	// ReMooD uses 64-bit Tics
	typedef uint64_t tic_t;
	#define TICRATE UINT64_C(35)
	
	// Doom Angles
	typedef uint32_t angle_t;
	
	#define ANG45		UINT32_C(0x20000000)
	#define ANG90		UINT32_C(0x40000000)
	#define ANG135		UINT32_C(0x60000000)
	#define ANG180		UINT32_C(0x80000000)
	#define ANG225		UINT32_C(0xA0000000)
	#define ANG270		UINT32_C(0xC0000000)
	#define ANG315		UINT32_C(0xE0000000)
	
	#define ANGMAX		UINT32_C(0xffffffff)
	#define ANG1		(ANG45/UINT32_C(45))
	#define ANG60		(ANG180/UINT32_C(3))

	#define ANGLEX(x)	((angle_t)(((angle_t)ANG1) * ((angle_t)(x))))
	
	// Doom Fixed
	typedef int32_t fixed_t;
	
	#define _FIXED_FRACBITS INT32_C(16)
	#define _FIXED_ONE (INT32_C(1) << _FIXED_FRACBITS)
	#define _FIXED_TWO (INT32_C(2) << _FIXED_FRACBITS)
	#define _FIXED_NEGONE (INT32_C(-1) << _FIXED_FRACBITS)
	#define _FIXED_SIGN		INT32_C(0x80000000)
	#define _FIXED_INT		INT32_C(0xFFFF0000)
	#define _FIXED_FRAC		INT32_C(0x0000FFFF)
	#define _FIXED_ROUND	INT32_C(0x00008000)

	// Compatibility
	#define FRACBITS _FIXED_FRACBITS
	#define FRACUNIT _FIXED_ONE
#endif

/****************
*** CONSTANTS ***
****************/

#define MAXPORTINFOFIELDLEN		128
#define MAXBOTINFOFIELDLEN		64
#define MAXBLVENDORDATAS		128
#define MAXBLTCFIELDLENGTH		64

#define MAXNAMELENGTH			64
#define MAXSKINLENGTH			64
#define MAXHEXENCLASSLENGTH		64
#define MAXWEAPONLENGTH			64
#define MAXCHATLENGTH			128

#define MAXWALKSPEED	25
#define MAXRUNSPEED		50

/* BL_Button_t -- Buttons for the tic command structure */
// ** = VM shall clear these flags when they are acknowledged. If the flag is
//      never cleared. Assume port did not yet ACK or does not know of it.
typedef enum BL_Button_e
{
	BLT_ATTACK			= UINT32_C(0x00000001),	// Fire Gun
	BLT_USE				= UINT32_C(0x00000002),	// Use Wall
	BLT_CHANGE			= UINT32_C(0x00000004),	// Change Weapon**
	BLT_JUMP			= UINT32_C(0x00000008),	// Jumps into the air**
	BLT_SUICIDE			= UINT32_C(0x00000010),	// Ends ones own life**
	BLT_FLYLAND			= UINT32_C(0x00000020),	// Land after flying**
	BLT_RESETAIM		= UINT32_C(0x00000040),	// Center view angle (useless?)**
	BLT_RELOAD			= UINT32_C(0x00000080),	// Reloads current weapon**
	BLT_SPECTATE		= UINT32_C(0x00000100),	// Spectates the bot**
												// After spectating, the exec
												// may stop running for the bot.
	BLT_DROPFLAG		= UINT32_C(0x00000200),	// Drops CTF Flags (if any)**
	BLT_DROPITEM		= UINT32_C(0x00000400),	// Drops inventory item**
	BLT_AUTOHEALTHITEM	= UINT32_C(0x00000800),	// Use Health Item**	
	BLT_TAUNT			= UINT32_C(0x00001000),	// Taunt**
} BL_Button_t;

/* BL_BotInfoFlag_t -- Bot information flag */
typedef enum BL_BotInfoFlag_e
{
	BLBIF_DEAD			= UINT32_C(0x00000001),	// Bot is currently dead
} BL_BotInfoFlag_t;

/* BL_ChatCommand_t -- Chat Command Code */
typedef enum BL_ChatCommand_e
{
	BLCC_NONE,									// Not talking
	BLCC_ALL,									// Talk to everyone
	BLCC_TEAM,									// Current Team (if possible)
	BLCC_SPECTATOR,								// Only to spectators
	BLCC_INDIV,									// Private Message (to port ID)
} BL_ChatCommand_t;

/* BL_VendorCode_t -- Vendor Codes for Source Ports */
typedef enum BL_VendorCode_e
{
	BLVC_NONE			= UINT32_C(0x00000000),	// No vendor??
	BLVC_REMOOD			= UINT32_C(0xA1101337),	// ReMooD
	
	BLVC_BOOM			= UINT32_C(0x0B001717),	// Boom (Generic)
	BLVC_CHOCOLATEDOOM	= UINT32_C(0xC40C07A7),	// Chocolate Doom
	BLVC_DOOMSDAY		= UINT32_C(0xD001717D),	// Doomsday/JDoom
	BLVC_ETERNITYENGINE	= UINT32_C(0x45654565),	// Eternity Engine
	BLVC_LEGACY			= UINT32_C(0x7E67AC79),	// Doom Legacy
	BLVC_ODAMEX			= UINT32_C(0x0DA53711),	// Odamex
	BLVC_PRBOOM			= UINT32_C(0x7072B017),	// PrBoom
	BLVC_PRBOOMPLUS		= UINT32_C(0x710B2707),	// PrBoom+
	BLVC_SKULLTAG		= UINT32_C(0x6a554e6b),	// Skulltag
	BLVC_STRAWBERRY		= UINT32_C(0xDEADF00D),	// Strawberru Doom
	BLVC_VANILLA		= UINT32_C(0x1CEC4EA7),	// Vanilla Doom
	BLVC_VAVOOM			= UINT32_C(0x5641766D),	// Vavoom
	BLVC_ZANDRONUM		= UINT32_C(0x7A414E44),	// Zandronum
	BLVC_ZDAEMON		= UINT32_C(0x4556494C),	// ZDaemon
	BLVC_ZDOOM			= UINT32_C(0x7A646F6F),	// ZDoom
} BL_VendorCode_t;

/* BL_Time_t -- Time field */
typedef enum BL_Time_e
{
	BLT_GAME,									// Gametic
	BLT_PROGRAM,								// Program tic (localtic)
	BLT_RESERVED01,								// ?????
	BLT_RESERVED02,								// ?????
	BLT_PORT01,									// Port Specific 1
	BLT_PORT02,									// Port Specific 2
	
	MAXBLT
} BL_Time_t;

/*****************
*** STRUCTURES ***
*****************/

/* BL_PortInfo_t -- Source Port Info */
typedef struct BL_PortInfo_s
{
	uint32_t VendorID;							// Vendor ID of source port
	uint8_t Name[MAXPORTINFOFIELDLEN];			// Name of sourceport
	uint32_t Version;							// Version of Port
	uint8_t VerString[MAXPORTINFOFIELDLEN];		// Version as string
	
	uint32_t __REMOOD_RESERVEDPI01;				// ?????
	uint32_t __REMOOD_RESERVEDPI02;				// ?????
	uint32_t __REMOOD_RESERVEDPI03;				// ?????
	uint32_t __REMOOD_RESERVEDPI04;				// ?????
	uint32_t __REMOOD_RESERVEDPI05;				// ?????
	uint32_t __REMOOD_RESERVEDPI06;				// ?????
	uint32_t __REMOOD_RESERVEDPI07;				// ?????
	uint32_t __REMOOD_RESERVEDPI08;				// ?????
	
	/* Port Specific Reserved Area */
	uint32_t PSVendorData[MAXBLVENDORDATAS];	// Vendor specific
} BL_PortInfo_t;

/* BL_BotAccount_t -- Bot Account */
typedef struct BL_BotAccount_s
{
	uint32_t Commit;							// Commit Account Data
	uint8_t Name[MAXNAMELENGTH];				// Name of Bot
	uint8_t Skin[MAXSKINLENGTH];				// Skin being worn
	uint8_t HexenClass[MAXHEXENCLASSLENGTH];	// Current Hexen Class (ReMooD Namespace)
	uint32_t Color;								// ReMooD Skin Color (0-15)
	uint32_t RGBColor;							// Color (R << 24, G << 16, B << 8)
	uint32_t Team;								// Team bot is on
		
	/* Port Specific Reserved Area */
	uint32_t PSVendorCode;						// Vendor code of reserved area
												// The implementation of the VM
												// in the source port shall not
												// use vendor specific data
												// unless the vendor code of
												// the port matches the value
												// in this field.
	uint32_t PSVendorData[MAXBLVENDORDATAS];	// Vendor specific
} BL_BotAccount_t;

/* BL_BotInfo_t -- Bot Info */
typedef struct BL_BotInfo_s
{
	/* General */
	tic_t JoinTime;								// Gametic at join time
	uint32_t Flags;								// Flags for bot
	int32_t PlayerID;							// Player ID (if in game)
	void* InternalPlayer;						// Internal player reference
	void* InternalMobj;							// Bot's internal mobj rep ID
	fixed_t Pos[3];								// Position of object
	uint8_t Weapon[MAXWEAPONLENGTH];			// Weapon bot is using
	uint32_t WeaponID;							// Weapon by class ID number
	
	/* Port Specific Reserved Area */
	uint32_t PSVendorData[MAXBLVENDORDATAS];	// Vendor specific
} BL_BotInfo_t;

/* BL_TicCmd_t -- Tic Command (controls bot) */
// NOTE THAT ALL MEMBERS ARE TO BE 32-BIT ALIGNED!
typedef struct BL_TicCmd_s
{
	/* Core ReMooD Commands */
	int32_t ForwardMove;						// -128 to 127
	int32_t SideMove;							// -128 to 127
	uint32_t LookAngle;							// BAM: Absolute Left/Right Look
	uint32_t Aiming;							// BAM: Absolute Up/Down Look
	uint32_t Buttons;							// BL_Button_t above
	uint32_t __REMOOD_RESERVEDBLTC01;			// ????? (Artifact)
	uint32_t StatFlags;							// ReMooD Status Flags
	uint32_t __REMOOD_RESERVEDBLTC02;			// ????? (Inventory Bits)
	int32_t FlySwim;							// > 0 = yup, < 0 = down
	uint8_t Weapon[MAXBLTCFIELDLENGTH];			// Weapon to switch to
												// This is ONLY registered when
												// the BLT_CHANGE is pressed.
												// Once it is registered, the
												// contents of this buffer are
												// erased. So always set this
												// before flagging.
	uint8_t Chat[MAXBLTCFIELDLENGTH];			// Chat Command
												// Similar to Weapon, but
												// registered when ChatCommand
												// is != 0
	uint32_t ChatTarget;						// Target for chat (privmsg)
	uint32_t ChatCommand;						// Command for Chat
	
	/* Reserved by ReMooD */
	uint32_t __REMOOD_RESERVEDBLTC03;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC04;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC05;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC06;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC07;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC08;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC09;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0A;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0B;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0C;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0D;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0E;			// ?????
	uint32_t __REMOOD_RESERVEDBLTC0F;			// ?????
	
	/* Port Specific Reserved Area */
	uint32_t PSVendorData[MAXBLVENDORDATAS];	// Vendor specific
} BL_TicCmd_t;

/* BL_GameInfo_t -- Game Information */
typedef struct BL_GameInfo_s
{
	tic_t GameTic;								// Current Gametic
	uint32_t __REMOOD_RESERVED01;				// ????
	uint32_t __REMOOD_RESERVED02;				// ????
	uint32_t __REMOOD_RESERVED03;				// ????
} BL_GameInfo_t;

/**************
*** GLOBALS ***
**************/

#define EXTADDRPORTINFO			UINT32_C(0x00050000)
#define EXTADDRACCTINFO			UINT32_C(0x00054000)
#define EXTADDRBOTINFO			UINT32_C(0x00058000)
#define EXTADDRTICCMD			UINT32_C(0x00060000)
#define EXTADDRFINESINE			UINT32_C(0x00068000)
#define EXTADDRFINECOSINE		UINT32_C(0x00070000)
#define EXTADDRFINETANGENT		UINT32_C(0x00078000)
#define EXTADDRTANTOANGLE		UINT32_C(0x00080000)
#define EXTADDRANGLUT			UINT32_C(0x00088000)
#define EXTADDRGAMEINFO			UINT32_C(0x00090000)

/* Not Included: Globals */
#if !defined(__REMOOD_INCLUDED)
	extern volatile const BL_PortInfo_t g_PortInfo;
	extern volatile const BL_BotInfo_t g_BotInfo;
	extern volatile BL_BotAccount_t g_Account;
	extern volatile BL_TicCmd_t g_TicCmd;
	extern volatile const BL_GameInfo_t g_GameInfo;
#endif

/********************
*** MINI TABLES.H ***
********************/

/* Not Included: Globals */
#if !defined(__REMOOD_INCLUDED)
	// From tables.h
	#define FINEANGLES              8192
	#define FINEMASK                (FINEANGLES-1)
	#define ANGLETOFINESHIFT        19	// 0x100000000 to 0x2000
	#define SLOPERANGE  2048
	#define SLOPEBITS   11
	#define DBITS       (FRACBITS-SLOPEBITS)
	
	extern fixed_t finesine[5 * FINEANGLES / 4];
	extern fixed_t* finecosine;
	extern fixed_t finetangent[FINEANGLES / 2];
	extern angle_t tantoangle[SLOPERANGE + 1];
	extern const int16_t c_AngLUT[8192];
#endif

/******************************************************************************
******************************* EVEN BETTER API *******************************
******************************************************************************/

/****************
*** CONSTANTS ***
****************/

#define BOTSYSCALL_FMUL			UINT32_C(0x00000001)
#define BOTSYSCALL_BSLEEP		UINT32_C(0x00000002)
#define BOTSYSCALL_DISTTO		UINT32_C(0x00000003)
#define BOTSYSCALL_SIGHTTO		UINT32_C(0x00000004)
#define BOTSYSCALL_CHAT			UINT32_C(0x00000005)
#define BOTSYSCALL_RANDLTZGT	UINT32_C(0x00000006)

#if !defined(__REMOOD_INCLUDED)
	#define VOIDP(x) x
	#define PCAST(s,x) ((s)(x))
	#define AOF(s,b,z,g,i) &(((s)(g))[(i)])
#else
	#define VOIDP(x) uint32_t
	#define PCAST(s,x) ((uint32_t)(x))
	#define AOF(s,b,z,g,i) ((uint32_t)((b) + ((z) * (i))))
#endif

/**************************
*** MAP DATA STRUCTURES ***
**************************/

// Base address for data structures, everything after this point is some kind
// of data structure. Least important ones have a lower address, while more
// important ones have a higher address.
#define MDSBASEADDR UINT32_C(0x80000000)
#define MDSENDADDR UINT32_C(0x80370A00)

// Current Mapping of Structures...

// What?			SizeOf	StartAddr	EndAddr		Total
// ----------------	-------	----------	----------	----------
// Vertexes			8		0x80000000	0x8007FFFF	65535
// Fake Floors		20		0x80080000	0x800809FF	127
// Sector Nodes		24		0x80080A00	0x800B09FF	8191
// Seg				40		0x800B0A00	0x803309FF	65535
// SideDef			4		0x80330A00	0x803709FF	65535
// LineDef			108		0x80370A00	0x80A309FF	65535
// SubSector		36		0x80A30A00	0x80C709FF	65535
// Nodes			40		0x80C70A00	0x80EF09FF	65535
// Map Thing		72		0x80EF0A00	0x813709FF	65535
// Sectors			452		0x81370A00	0x82FB09FF	65535
// Objects			236		0x82FB0A00	0x83E709FF	65535
// Players			1876	0x83E70A00	0x83EE5DFF	256
// ???				???		0xE3EE5E00	0x????????	???

/*** PREDEFINE ***/

typedef struct MVertex_s MVertex_t;
typedef struct MFakeFloor_s MFakeFloor_t;
typedef struct MSecNode_s MSecNode_t;
typedef struct MSeg_s MSeg_t;
typedef struct MSide_s MSide_t;
typedef struct MLine_s MLine_t;
typedef struct MSubS_s MSubS_t;
typedef struct MNode_s MNode_t;
typedef struct MThing_s MThing_t;
typedef struct MSector_s MSector_t;
typedef struct MObject_s MObject_t;
typedef struct MPlayer_s MPlayer_t;
typedef struct MObjInfo_s MObjInfo_t;
typedef struct MState_s MState_t;
typedef struct MWeapon_s MWeapon_t;

/*** VERTEX ***/

#define MVERTEXBASEADDR (MDSBASEADDR)
#define MVERTEXMAX		UINT32_C(65535)
#define MVERTEXSIZE		UINT32_C(8)
#define MVERTEXIDTOP(x)	AOF(MVertex_t,MVERTEXBASEADDR,MVERTEXSIZE,MVertexes,(x))

/* MVertex_t -- Vertex in map */
struct MVertex_s
{
	fixed_t x;									// X Position of vertex
	fixed_t y;									// Y Position of vertex
};

/*** FAKE FLOORS (A.K.A. 3D FLOORS) ***/

#define MFFBASEADDR (MDSBASEADDR + UINT32_C(0x80000))
#define MFFMAX		UINT32_C(127)
#define MFFSIZE		UINT32_C(20)
#define MFFIDTOP(x)	AOF(MFakeFloor_t,MFFBASEADDR,MFFSIZE,MFFloors,(x))

typedef enum MFakeFloorFlag_e
{
	MFFF_EXISTS			= UINT32_C(0x00000001),	//MAKE SURE IT'S VALID
	MFFF_SOLID			= UINT32_C(0x00000002),	//Does it clip things?
	MFFF_RENDERSIDES	= UINT32_C(0x00000004),	//Render the sides?
	MFFF_RENDERPLANES	= UINT32_C(0x00000008),	//Render the floor/ceiling?
	MFFF_RENDERALL		= UINT32_C(0x0000000C),	//Render everything?
	MFFF_SWIMMABLE		= UINT32_C(0x00000010),	//Can we swim?
	MFFF_NOSHADE		= UINT32_C(0x00000020),	//Does it mess with the lighting?
	MFFF_CUTSOLIDS		= UINT32_C(0x00000040),	//Does it cut out hidden solid pixles?
	MFFF_CUTEXTRA		= UINT32_C(0x00000080),	//Does it cut out hidden translucent pixles?
	MFFF_CUTLEVEL		= UINT32_C(0x000000C0),	//Does it cut out all hidden pixles?
	MFFF_CUTSPRITES		= UINT32_C(0x00000100),	//Final Step in 3D water
	MFFF_BOTHPLANES		= UINT32_C(0x00000200),	//Render both planes all the time?
	MFFF_EXTRA			= UINT32_C(0x00000400),	//Does it get cut by FF_CUTEXTRAS?
	MFFF_TRANSLUCENT	= UINT32_C(0x00000800),	//See through!
	MFFF_FOG			= UINT32_C(0x00001000),	//Fog "brush"?
	MFFF_INVERTPLANES	= UINT32_C(0x00002000),	//Reverse the plane visibility rules?
	MFFF_ALLSIDES		= UINT32_C(0x00004000),	//Render inside and outside sides?
	MFFF_INVERTSIDES	= UINT32_C(0x00008000),	//Only render inside sides?
	MFFF_DOUBLESHADOW	= UINT32_C(0x00010000),	//Make two lightlist entries to reset light?
} MFakeFloorFlag_t;

/* MFakeFloor_t -- Fake Floor */
struct MFakeFloor_s
{
	VOIDP(MSector_t*) Target;					// Target Sector
	VOIDP(MSector_t*) Ref;						// Reference Sector
	uint32_t Flags;								// Flags of floor (as above)
	VOIDP(MFakeFloor_t*) Prev;					// Previous floor
	VOIDP(MFakeFloor_t*) Next;					// Next floor
};

/*** SECTOR NODES ***/

#define MSECNODEBASEADDR	(MDSBASEADDR + UINT32_C(0x80A00))
#define MSECNODEMAX			UINT32_C(8191)
#define MSECNODESIZE		UINT32_C(24)
#define MSECNODEIDTOP(x)	AOF(MSecNode_t,MSECNODEBASEADDR,MSECNODESIZE,MSecNodes,(x))

/* MSecNode_t -- Sector Node */
struct MSecNode_s
{
	VOIDP(MSector_t*) Sector;					// Sector containing this thing
	VOIDP(MObject_t*) Thing;					// Specific Thing
	VOIDP(MSecNode_t*) TPrev;					// Previous thing
	VOIDP(MSecNode_t*) TNext;					// Next Thing
	VOIDP(MSecNode_t*) SPrev;					// Previous Sector
	VOIDP(MSecNode_t*) SNext;					// Next Sector
};

/*** SEGS ***/

#define MSEGBASEADDR	(MDSBASEADDR + UINT32_C(0xB0A00))
#define MSEGMAX			UINT32_C(65535)
#define MSEGSIZE		UINT32_C(40)
#define MSEGIDTOP(x)	AOF(MSeg_t,MSEGBASEADDR,MSEGSIZE,MSegs,(x))

/* MSeg_t -- Seg */
struct MSeg_s
{
	VOIDP(MVertex_t*) Start;					// Starting point
	VOIDP(MVertex_t*) End;						// Ending point
	int32_t Side;								// Side
	fixed_t Offset;								// Offset
	angle_t Angle;								// Angle
	VOIDP(MSide_t*) SideDef;					// SideDef used on
	VOIDP(MLine_t*) LineDef;					// LineDef used on
	VOIDP(MSector_t*) FrontSector;				// Sector in front of
	VOIDP(MSector_t*) BackSector;				// Sector behind
	fixed_t Length;								// Length of line
};

/*** SIDEDEF ***/

#define MSIDEBASEADDR	(MDSBASEADDR + UINT32_C(0x330A00))
#define MSIDEMAX		UINT32_C(65535)
#define MSIDESIZE		UINT32_C(4)
#define MSIDEIDTOP(x)	AOF(MSide_t,MSIDEBASEADDR,MSIDESIZE,MSides,(x))

/* MSide_t -- Side */
struct MSide_t
{
	VOIDP(MSector_t*) Sector;					// Sector side faces
};

/*** LINEDEF ***/

#define MLINEBASEADDR	(MDSBASEADDR + UINT32_C(0x370A00))
#define MLINEMAX		UINT32_C(65535)
#define MLINESIZE		UINT32_C(108)
#define MLINEIDTOP(x)	AOF(MLine_t,MLINEBASEADDR,MLINESIZE,MLines,(x))

/* MLine_t -- LineDef */ 
struct MLine_s
{
	VOIDP(MVertex_t*) Start;					// Start of line
	VOIDP(MVertex_t*) End;						// End of line
	fixed_t DiffX;								// Start-End Difference
	fixed_t DiffY;
	uint32_t Flags;								// Flags
	uint32_t Special;							// Special
	uint32_t Tag;								// Tag
	uint32_t Side[2];							// Side in front/back
	VOIDP(MSector_t*) Sector[2];				// Sector in front/back
	uint32_t HexenSpecial;						// Hexen Special
	uint32_t HexenArgs[5];						// Hexen Arguments
	int32_t FirstTag;							// First Tag
	int32_t NextTag;							// Next Tag
	uint32_t Reserved1;							// Reserved
	uint32_t Reserved2;							// Reserved
	uint32_t Reserved3;							// Reserved
	uint32_t Reserved4;							// Reserved
	uint32_t Reserved5;							// Reserved
	uint32_t Reserved6;							// Reserved
	uint32_t Reserved7;							// Reserved
	uint32_t Reserved8;							// Reserved
};

/*** SUBSECTORS ***/

#define MSUBSBASEADDR	(MDSBASEADDR + UINT32_C(0xA30A00))
#define MSUBSMAX		UINT32_C(65535)
#define MSUBSSIZE		UINT32_C(36)
#define MSUBSIDTOP(x)	AOF(MSubS_t,MSUBSBASEADDR,MSUBSSIZE,MSubSectors,(x))

/* MSubS_t -- Subsector */
struct MSubS_s
{
	VOIDP(MSector_t*) Sector;					// Sector this belongs to
	uint32_t NumSegs;							// Number of segs
	uint32_t FirstSeg;							// First seg reference
	fixed_t CenterX;							// Center Position (!0 if valid)
	fixed_t CenterY;							//  "  "   "    "
	uint32_t Reserved1;							// Reserved
	uint32_t Reserved2;							// Reserved
	uint32_t Reserved3;							// Reserved
	uint32_t Reserved4;							// Reserved
};

/*** NODES ***/

#define MNODEBASEADDR	(MDSBASEADDR + UINT32_C(0xC70A00))
#define MNODEMAX		UINT32_C(65535)
#define MNODESIZE		UINT32_C(40)
#define MNODEIDTOP(x)	AOF(MNode_t,MNODEBASEADDR,MNODESIZE,MNodes,(x))

/* MNode_t -- Subsector */
struct MNode_s
{
	fixed_t Splice[4];							// Partition Line
	fixed_t BBox[4];							// Bounding box
	uint32_t Kids[2];							// Kids
};

/*** MAP THINGS ***/

#define MTHINGBASEADDR	(MDSBASEADDR + UINT32_C(0xEF0A00))
#define MTHINGMAX		UINT32_C(65535)
#define MTHINGSIZE		UINT32_C(72)
#define MTHINGIDTOP(x)	AOF(MThing_t,MTHINGBASEADDR,MTHINGSIZE,MThings,(x))

/* MThing_t -- Map Thing */
struct MThing_s
{
	fixed_t Pos[3];								// Spawn Position
	angle_t Angle;								// Angle
	uint32_t Flags;								// Flags
	VOIDP(MObject_t*) Object;					// Object last spawned
	uint32_t HexenID;							// Hexen ID
	fixed_t HexenHeightOff;						// Hexen Height Offset
	uint32_t HexenSpecial;						// Hexen Special
	uint32_t HexenArgs[5];						// Hexen Arguments
	uint32_t Reserved1;							// Reserved
	uint32_t Reserved2;							// Reserved
	uint32_t Reserved3;							// Reserved
	uint32_t Reserved4;							// Reserved
};

/*** SECTORS ***/

#define MSECTORBASEADDR	(MDSBASEADDR + UINT32_C(0x1370A00))
#define MSECTORMAX		UINT32_C(65535)
#define MSECTORSIZE		UINT32_C(452)
#define MSECTORIDTOP(x)	AOF(MSector_t,MSECTORBASEADDR,MSECTORSIZE,MSectors,(x))

#define MAXMSECTORLINES UINT32_C(32)
#define MAXMSECTORATTACHED UINT32_C(32)
#define MAXMSECTORADJ UINT32_C(32)

/* MSector_t -- Sector */
struct MSector_s
{
	fixed_t Z[2];								// Floor/Ceiling Height
	int32_t LightLevel;							// Light Level
	uint32_t Special;							// Special Action
	int32_t Tag;								// Tag
	int32_t NextTag;							// Next in tag list
	int32_t FirstTag;							// First in tag list
	VOIDP(MObject_t*) SoundTarget;				// Thing which made a noise
	fixed_t BlockBox[4];						// Sector blocking box
	uint32_t NumLines;							// Number of lines
	VOIDP(MLine_t*) Lines[MAXMSECTORLINES];		// Lines of sector
	VOIDP(MFakeFloor_t*) FFloor;				// Fake Floor
	uint32_t NumAttached;						// Number of attached??
	uint32_t Attached[MAXMSECTORATTACHED];		// Attached Lines??
	uint32_t NumAdj;							// Number of adjacent sectors
	VOIDP(MSector_t*) Adj[MAXMSECTORADJ];		// Adjacent Sectors
	VOIDP(MSecNode_t*) TouchThingList;			// Touching thing list
};

/*** OBJECTS ***/

#define MOBJECTBASEADDR	(MDSBASEADDR + UINT32_C(0x2FB0A00))
#define MOBJECTMAX		UINT32_C(65535)
#define MOBJECTSIZE		UINT32_C(236)
#define MOBJECTIDTOP(x)	AOF(MObject_t,MOBJECTBASEADDR,MOBJECTSIZE,MObjects,(x))

/* MObject_t -- Object */
struct MObject_s
{
	uint32_t InUse;								// This object slot is in use
	uint32_t SpawnID;							// Object Spawn ID
	int32_t Health;								// Health
	fixed_t Pos[3];								// Position
	fixed_t Mom[3];								// Momentum
	angle_t Angle;								// Angle
	fixed_t SurfaceZ[2];						// floorz/ceilingz
	fixed_t Radius;								// Radius
	fixed_t Height;								// Height
	uint32_t Flags[12];							// Object Flags
	uint32_t Type;								// Type of thing
	VOIDP(MSubS_t*) SubS;						// Subsector object is in
	VOIDP(MObjInfo_t*) Info;					// Object Information
	int32_t Tic;								// Tics of current state
	VOIDP(MState_t*) State;						// Current State
	VOIDP(MObject_t*) Target;					// Target Object
	VOIDP(MObject_t*) Tracer;					// Tracer
	VOIDP(MPlayer_t*) Player;					// Player
	int32_t Move[2];							// Move direction/count
	int32_t ReactionTime;						// Reaction Time
	int32_t LastLook;							// Last Look
	int32_t Threshold;							// Threshold
	VOIDP(MThing_t*) SpawnThing;				// Thing spawned at
	fixed_t MaxZ;								// Max Z of object
	int32_t Team;								// Team Object is on
	uint32_t SameTeam;							// Same team as you
	int32_t DroppedAmmo;						// Dropped Ammo
	int32_t Special[2];							// Special Property
	int32_t KilledByPlayer;						// Player that killed this
	uint32_t FraggerID;							// ID of killer
	VOIDP(MObject_t*) SPrev;					// Previous in sector
	VOIDP(MObject_t*) SNext;					// Next in sector
	VOIDP(MObject_t*) BPrev;					// Previous in blockmap
	VOIDP(MObject_t*) BNext;					// Next in blockmap
	uint32_t Reserved[7];						// Reserved
};

/*** PLAYERS ***/

#define MPLAYERBASEADDR	(MDSBASEADDR + UINT32_C(0x3E70A00))
#define MPLAYERMAX		UINT32_C(256)
#define MPLAYERSIZE		UINT32_C(1876)
#define MPLAYERIDTOP(x)	AOF(MPlayer_t,MPLAYERBASEADDR,MPLAYERSIZE,MPlayers,(x))

#define MPLAYERMAXWEAPON	128
#define MPLAYERMAXAMMO		128

/* MPlayer_t -- Player */
struct MPlayer_s
{
	uint32_t InGame;							// Player is in game
	VOIDP(MObject_t*) Obj;						// Object of player
	VOIDP(MObject_t*) Attacker;					// Attacker of player
	VOIDP(MObject_t*) Attackee;					// Player is attacking this
	int32_t Health;								// Health
	int32_t Armor;								// Armor
	uint32_t ArmorType;							// Type of armor
	int32_t Team;								// Player's Team
	uint32_t SameTeam;							// Same Team as you
	uint32_t MaxHealth[2];						// Max Health (normal/super)
	uint32_t MaxArmor[2];						// Max Armor (normal/super)
	uint32_t Cards;								// Cards (bitfield)
	int32_t PendingWeapon;						// Weapon to change to
	int32_t ReadyWeapon;						// Weapon to ready
	VOIDP(MWeapon_t*) PendingWeaponP;			// Pointer to pending weapon
	VOIDP(MWeapon_t*) ReadyWeaponP;				// Pointer to ready weapon
	uint32_t WeaponOwned[MPLAYERMAXWEAPON];		// Weapons owned by player
	struct
	{
		int32_t Now;							// Current Ammo
		int32_t Max;							// Max Ammo
	} Ammo[MPLAYERMAXAMMO];
	uint32_t Backpack;							// Has a backpack
	angle_t Aiming;								// Up/Down Angle
	int32_t DamageCount;						// How red the screen is
	int32_t BonusCount;							// Palette of items picked up
	int32_t Frags;								// Frags
	int32_t Deaths;								// Number of deaths
	int32_t Kills;								// Monster Kills
	int32_t Items;								// Items picked up
	int32_t Secrets;							// Secrets Found
	VOIDP(MObject_t*) Rain[2];					// Heretic Rain
	uint32_t Reserved[56];						// Reserved
};

/******************
*** INFORMATION ***
******************/

typedef struct BInfo_s BInfo_t;
typedef struct BReal_s BReal_t;

#define BCOREINFOBASEADDR UINT32_C(0x60000000)

/*** BOT INFORMATION ***/
// This structure contains the information on the current bot, relevant for the
// current bot being executed.

#define BBINFOBASEADDR (BCOREINFOBASEADDR + UINT32_C(0x10000))

/* BInfo_t -- Bot information structure */
struct BInfo_s
{
	/* Player */
	VOIDP(MPlayer_t*) Player;					// Player Pointer
	int32_t Health;								// Current Health
	int32_t Armor;								// Current Armor
	int32_t ArmorType;							// Type of armor being worn
	VOIDP(MObject_t*) Attacker;					// Being attacked by
	VOIDP(MObject_t*) Attackee;					// Thing attacking
	VOIDP(MWeapon_t*) Weapon;					// Weapon bot is using
	
	/* Map Object */
	VOIDP(MObject_t*) Mo;						// Map Object Pointer
	fixed_t x;									// X position
	fixed_t y;									// Y position
	fixed_t z;									// Z position
	VOIDP(MSubS_t*) SubS;						// Subsector bot is in
};

/*** REAL TIME INFORMATION ***/

#define BBREALBASEADDR (BCOREINFOBASEADDR + UINT32_C(0x20000))

/* BGameState_t -- Current game state */
typedef enum BGameState_e
{
	BGS_LEVEL,									// Playing Level
	BGS_INTER,									// Intermission
	BGS_STORY,									// Story Text
	BGS_FINALE,									// Finale Event
	BGS_UNKNOWN,								// Unknown game state
} BGameState_t;

/* BReal_t -- Realtime information (per tic) */
struct BReal_s
{
	tic_t GameTic;								// Current game tic
	tic_t LocalTic;								// Current local tic
	tic_t LevelTic;								// Current level time
	uint32_t State;								// Game State
};

/*******************
*** SYSTEM CALLS ***
*******************/

#if !defined(__REMOOD_INCLUDED)

#define ____SYSCALL_INTRO \
volatile register unsigned int ko asm("k0");\
volatile register unsigned int kl asm("k1");

// System calls require a memory barrier because variable in locals and memory
// can change without the compiler knowing it.
#define ____SYSCALL_DO(t,a) \
ko = (t);\
kl = (unsigned int)(a);\
asm __volatile__("syscall");\
asm __volatile__("nop");\
asm __volatile__("" : : : "memory");

/* BSleep() -- Sleeps for the specified number of tics */
static inline void BSleep(const uint32_t a_Len)
{
	____SYSCALL_INTRO;
	volatile uint32_t CS[1];
	
	/* Initialize the Call Stack */
	CS[0] = a_Len;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_BSLEEP, CS);
}

/* FMul() -- Performs fixed multiplication */
static inline fixed_t FMul(const fixed_t a_A, const fixed_t a_B)
{
	____SYSCALL_INTRO;
	volatile fixed_t CS[2];
	
	/* Initialize the Call Stack */
	CS[0] = a_A;
	CS[1] = a_B;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_FMUL, &CS);
	
	/* Return result */
	return kl;
}

/* DistTo() -- Calculates distance to remote point */
static inline fixed_t DistTo(const fixed_t a_X, const fixed_t a_Y)
{
	____SYSCALL_INTRO;
	volatile fixed_t CS[2];
	
	/* Initialize the Call Stack */
	CS[0] = a_X;
	CS[1] = a_Y;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_DISTTO, &CS);
	
	/* Return result */
	return kl;
}

/* SightTo() -- Determines if remote point is visible */
static inline int32_t SightTo(const fixed_t a_X, const fixed_t a_Y)
{
	____SYSCALL_INTRO;
	volatile fixed_t CS[2];
	
	/* Initialize the Call Stack */
	CS[0] = a_X;
	CS[1] = a_Y;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_SIGHTTO, &CS);
	
	/* Return result */
	return kl;
}

/* GeneralChat() -- Sends general chat message */
static inline void GeneralChat(const char* const a_Msg)
{
	____SYSCALL_INTRO;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_CHAT, a_Msg);
}

/* RandomLTZGT() -- Returns either -1, 0, or 1 */
static inline int32_t RandomLTZGT(void)
{
	____SYSCALL_INTRO;
	
	/* Call Handler */
	____SYSCALL_DO(BOTSYSCALL_RANDLTZGT, 0);
	
	/* Return result */
	return kl;
}

#undef ____SYSCALL_INTRO
#undef ____SYSCALL_DO

#endif

/**************
*** GLOBALS ***
**************/

#if !defined(__REMOOD_INCLUDED)

extern volatile BInfo_t g_Info;
extern volatile const BReal_t g_Real;

#endif

/******************************************************************************
*******************************************************************************
******************************************************************************/

/*****************************************************************************/

#endif /* __BOT_LIB_H__ */

