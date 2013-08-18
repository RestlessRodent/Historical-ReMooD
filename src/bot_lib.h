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
	
	// Doom Angles
	typedef uint32_t angle_t;
	
	#define ANG45		UINT32_C(0x20000000)
	#define ANG90		UINT32_C(0x40000000)
	#define ANG180		UINT32_C(0x80000000)
	#define ANG270		UINT32_C(0xc0000000)
	
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
#define MAXBOTINFOFIELDLEN		128
#define MAXBLVENDORDATAS		128
#define MAXBLTCFIELDLENGTH		256

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
	BLT_PORT03,									// Port Specific 3
	BLT_PORT04,									// Port Specific 4
	
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

/* BL_BotInfo_t -- Bot Info */
typedef struct BL_BotInfo_s
{
	uint32_t Commit;							// Commit structure data
	uint8_t Name[MAXBOTINFOFIELDLEN];			// Name of Bot
	uint8_t Skin[MAXBOTINFOFIELDLEN];			// Skin being worn
	uint8_t HexenClass[MAXBOTINFOFIELDLEN];		// Current Hexen Class (ReMooD Namespace)
	tic_t JoinTime[2];							// Gametic at join time
	uint32_t Color;								// ReMooD Skin Color (0-15)
	uint32_t RGBColor;							// Color (R << 24, G << 16, B << 8)
	
	/* Port Specific Reserved Area */
	uint32_t PSVendorCode;						// Vendor code of reserved area
												// The implementation of the VM
												// in the source port shall not
												// use vendor specific data
												// unless the vendor code of
												// the port matches the value
												// in this field.
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
	uint32_t PSVendorCode;						// Vendor code of reserved area
												// The implementation of the VM
												// in the source port shall not
												// use vendor specific data
												// unless the vendor code of
												// the port matches the value
												// in this field.
	uint32_t PSVendorData[MAXBLVENDORDATAS];	// Vendor specific
} BL_TicCmd_t;

/* BL_GameInfo_t -- Game Information */
typedef struct BL_GameInfo_s
{
} BL_GameInfo_t;

/**************
*** GLOBALS ***
**************/

#define EXTADDRPORTINFO			UINT32_C(0x00050000)
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

/****************
*** FUNCTIONS ***
****************/

/* Not Included: Functions */
#if !defined(__REMOOD_INCLUDED)
	static inline fixed_t FixedMul(fixed_t a, fixed_t b)
	{
		return ((int64_t)a * (int64_t)b) >> _FIXED_FRACBITS;
	}
	
	static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
	{
		if (b == 0)
			return 0x7FFFFFFF | (a & 0x80000000);
		else
			return (fixed_t)((((((int64_t)a) << (int64_t)FRACBITS) / ((int64_t)b)) & INT64_C(0xFFFFFFFF)));
	}

	static inline fixed_t FixedMod(fixed_t a, fixed_t b)
	{
		fixed_t dVal = FixedDiv(a, b);
		fixed_t iVal = dVal & 0xFFFF0000;
		return a - FixedMul(dVal, iVal); 
	}
	
	static inline fixed_t TBL_BAMToDeg(const angle_t a_Angle)
	{
		return ((int64_t)a_Angle << ((int64_t)(FRACBITS + FRACBITS))) / (UINT64_C(0xB60B60) << ((int64_t)FRACBITS));
	}
#endif

/*****************************************************************************/

#endif /* __BOT_LIB_H__ */

