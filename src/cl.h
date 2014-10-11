// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Local Game Client

#ifndef __CL_H__
#define __CL_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "i_util.h"

/****************
*** CONSTANTS ***
****************/

/* CL_SockFlags_t -- Socket Flags */
typedef enum CL_SockFlags_e
{
	CLSF_JOYSTICK		= UINT32_C(0x00000001),	// Using joystick
} CL_SockFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/* Define CL_View_t */
#if !defined(__REMOOD_CLVIEW_DEFINED)
	typedef struct CL_View_s CL_View_t;
	#define __REMOOD_CLVIEW_DEFINED
#endif

/* Define CL_Socket_t */
#if !defined(__REMOOD_CLSOCKET_DEFINED)
	typedef struct CL_Socket_s CL_Socket_t;
	#define __REMOOD_CLSOCKET_DEFINED
#endif

/* Define CL_Client_t */
#if !defined(__REMOOD_CLCLIENT_DEFINED)
	typedef struct CL_Client_s CL_Client_t;
	#define __REMOOD_CLCLIENT_DEFINED
#endif

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

/* CL_View_t -- Client viewport */
struct CL_View_s
{
	CL_Socket_t* Socket;						// Control Socket
};

/* CL_Socket_t -- Socket which recieves controller input */
// Sockets are attached to viewports
struct CL_Socket_s
{
	SN_Port_t* Port;							// Port to control
	CL_View_t* View;							// Viewer
	uint32_t Flags;								// Flags
	int8_t JoyID;								// JoyStick ID
	
	int32_t Console;							// Console Player
	int32_t Display;							// Display Player
	
	angle_t VRot;								// Left/Right Look
	angle_t VPitch;								// Up/Down Look
	
	// TO BE REMOVED IN THE FUTURE
	D_Prof_t* Profile;							// Profile to use
};

/**************
*** GLOBALS ***
**************/

extern CL_View_t g_CLViews[MAXSPLITS];			// Viewports
extern int32_t g_CLBinds;						// Number of bound viewports

/****************
*** FUNCTIONS ***
****************/

/*** CL_SOCK.C **/

int32_t CL_InitSocks(void);
CL_View_t* CL_BindSocket(CL_Socket_t* const a_Sock, const int8_t a_JoyID);

bool_t CL_SockEvent(const I_EventEx_t* const a_Event);
void CL_SockDrawer(void);

void CL_InitLevelSocks(void);
void CL_ClearLevelSocks(void);
void CL_DoResetMapZoom(void);
void CL_DoAngleSync(void);
void CL_DoSetYawP(player_t* const a_Player, const angle_t a_Yaw);
void CL_DoSetAnglesP(player_t* const a_Player, const angle_t a_Yaw, const angle_t a_Pitch);
void CL_DoDeathViewP(player_t* const a_Player);
void CL_DoTactileP(player_t* const a_Player, const int32_t a_On, const int32_t a_Off, const int32_t a_Total);

void CL_SpecTicker(void);// Move to cl_spec.c

/*****************************************************************************/

#endif /* __CL_H__ */


