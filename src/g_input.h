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
// DESCRIPTION: handle mouse/keyboard/joystick inputs,
//              maps inputs to game controls (forward,use,open...)

#ifndef __G_INPUT_H__
#define __G_INPUT_H__

#include "doomstat.h"
#include "d_event.h"
#include "keys.h"
#include "command.h"

#define MAXMOUSESENSITIVITY   160	// sensitivity steps

// number of total 'button' inputs, include keyboard keys, plus virtual
// keys (mousebuttons and joybuttons becomes keys)
#define NUMKEYS         256

#define MOUSEBUTTONS    8

// In i_util.h and g_input.h
#ifndef JOYBUTTONS
#define JOYBUTTONS      32
#endif

//
// mouse and joystick buttons are handled as 'virtual' keys
//
#if (MAXSPLITSCREENPLAYERS > 4)
#error MODIFY INPUTS (Max Splitscreen players is now > 4)!
#endif

#define MOUSEBUTTONRANGE ((MOUSEBUTTONS << 1) + 2)
#define JOYBUTTONRANGE (JOYBUTTONS << 1)

typedef enum
{

	/*** World ***/
	KEY_WORLDSTART = NUMKEYS,
	KEY_WORLDEND = KEY_WORLDSTART + 96,
	
	/*** Mouse ***/
	// Mouse 1
	KEY_MOUSE1B1,
	KEY_MOUSE1DBL1 = KEY_MOUSE1B1 + MOUSEBUTTONS,
	KEY_MOUSE1WHEELUP = KEY_MOUSE1DBL1 + MOUSEBUTTONS,
	KEY_MOUSE1WHEELDOWN,
	
	// Mouse 2
	KEY_MOUSE2B1,
	KEY_MOUSE2DBL1 = KEY_MOUSE2B1 + MOUSEBUTTONS,
	KEY_MOUSE2WHEELUP = KEY_MOUSE2DBL1 + MOUSEBUTTONS,
	KEY_MOUSE2WHEELDOWN,
	
	// Mouse 3
	/*KEY_MOUSE3B1,
	   KEY_MOUSE3DBL1 = KEY_MOUSE3B1 + MOUSEBUTTONS,
	   KEY_MOUSE3WHEELUP = KEY_MOUSE3DBL1 + MOUSEBUTTONS,
	   KEY_MOUSE3WHEELDOWN,
	
	   // Mouse 4
	   KEY_MOUSE4B1,
	   KEY_MOUSE4DBL1 = KEY_MOUSE4B1 + MOUSEBUTTONS,
	   KEY_MOUSE4WHEELUP = KEY_MOUSE4DBL1 + MOUSEBUTTONS,
	   KEY_MOUSE4WHEELDOWN, */
	
	/*** Joystick ***/
	// Joy 1
	KEY_JOY1B1,
	KEY_JOY1DBL1 = KEY_JOY1B1 + JOYBUTTONS,
	
	// Joy 2
	KEY_JOY2B1 = KEY_JOY1DBL1 + JOYBUTTONS,
	KEY_JOY2DBL1 = KEY_JOY2B1 + JOYBUTTONS,
	
	// Joy 3
	KEY_JOY3B1 = KEY_JOY2DBL1 + JOYBUTTONS,
	KEY_JOY3DBL1 = KEY_JOY3B1 + JOYBUTTONS,
	
	// Joy 4
	KEY_JOY4B1 = KEY_JOY3DBL1 + JOYBUTTONS,
	KEY_JOY4DBL1 = KEY_JOY4B1 + JOYBUTTONS,
	
	NUMINPUTS
} key_input_e;

typedef enum
{
	gc_null = 0,				//a key/button mapped to gc_null has no effect
	gc_forward,
	gc_backward,
	gc_strafe,
	gc_straferight,
	gc_strafeleft,
	gc_speed,
	gc_turnleft,
	gc_turnright,
	gc_fire,
	gc_use,
	gc_lookup,
	gc_lookdown,
	gc_centerview,
	gc_mouseaiming,				// mouse aiming is momentary (toggleable in the menu)
	gc_weapon1,
	gc_weapon2,
	gc_weapon3,
	gc_weapon4,
	gc_weapon5,
	gc_weapon6,
	gc_weapon7,
	gc_weapon8,
	gc_talkkey,
	gc_scores,
	gc_jump,
	gc_console,
	gc_nextweapon,
	gc_prevweapon,
	gc_bestweapon,
	gc_invnext,
	gc_invprev,
	gc_invuse,
	gc_flydown,					// flyup is jump !
	num_gamecontrols
} gamecontrols_e;

// mouse values are used once
extern consvar_t cv_mousesens;
extern consvar_t cv_mousesensy;
extern consvar_t cv_m_xsensitivity;
extern consvar_t cv_m_ysensitivity;
extern consvar_t cv_mlooksens;
extern consvar_t cv_allowjump;
extern consvar_t cv_allowrocketjump;
extern consvar_t cv_classicrocketblast;
extern consvar_t cv_allowautoaim;
extern consvar_t cv_forceautoaim;
extern consvar_t cv_m_xaxismode;
extern consvar_t cv_m_yaxismode;
extern consvar_t cv_m_xaxissecmode;
extern consvar_t cv_m_yaxissecmode;
extern consvar_t cv_legacymouse;	// DEPRECATED
extern consvar_t cv_m_legacymouse;
extern consvar_t cv_m_classicalt;

extern int mousex;
extern int mousey;
extern int mlooky;				//mousey with mlookSensitivity
extern int mouse2x;
extern int mouse2y;
extern int mlook2y;

extern int dclicktime;
extern int dclickstate;
extern int dclicks;
extern int dclicktime2;
extern int dclickstate2;
extern int dclicks2;

extern int joyxmove;
extern int joyymove;

// current state of the keys : true if pushed
extern uint8_t gamekeydown[NUMINPUTS];

// two key codes (or virtual key) per game control
extern int gamecontrol[MAXSPLITSCREENPLAYERS][num_gamecontrols][2];

// peace to my little coder fingers!
// check a gamecontrol being active or not

// remaps the input event to a game control.
void G_MapEventsToControls(event_t* ev);

// returns the name of a key
char* G_KeynumToString(int keynum);
int G_KeyStringtoNum(char* keystr);

// detach any keys associated to the given game control
void G_ClearControlKeys(int (*setupcontrols)[2], int control);
void Command_Setcontrol_f(void);
void Command_Setcontrol2_f(void);
void G_Controldefault(void);
void G_SaveKeySetting(FILE* f);
void G_CheckDoubleUsage(int keynum);

void G_InitKeys(void);

extern bool_t shiftdown;

/* GhostlyDeath <December 11, 2008> -- Joysticks */
// For Axes Binding:
//  a?#, a = axis, ? = axis usage, # = bound player
//  If there are 2 Axises (X and Y for example)
//  Then there are 4 possible values for a?#
//  Axis 0 = 0, Axis 0 (Inverted) = 2
//  Axis 1 = 1, Axis 1 (Inverted) = 3

extern consvar_t cv_joy_enable;	// Enable Joysticks

/*** Player 1 ***/
// Basic
extern consvar_t cv_joy_bind1;	// Joystick ID bound to Player 1

// Looking
extern consvar_t cv_joy_xlook1;	// X Axis Binding
extern consvar_t cv_joy_ylook1;	// Y Axis Binding
extern consvar_t cv_joy_zlook1;	// Z Axis Binding

// Moving
extern consvar_t cv_joy_xmove1;	// X Axis Movement (Strafing)
extern consvar_t cv_joy_ymove1;	// Y Axis Movement (Forward/Backwards)
extern consvar_t cv_joy_zmove1;	// Z Axis Movement (Swimming/Flying)

#endif							/* __G_INPUT_H__ */
