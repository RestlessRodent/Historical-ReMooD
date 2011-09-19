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
// DESCRIPTION:
//      handle mouse/keyboard/joystick inputs,
//      maps inputs to game controls (forward,use,open...)

#include "doomdef.h"
#include "doomstat.h"
#include "g_input.h"
#include "keys.h"
#include "hu_stuff.h"			//need HUFONT start & end
#include "keys.h"
#include "d_net.h"
#include "console.h"

CV_PossibleValue_t mousesens_cons_t[] = { {1, "MIN"}
	, {MAXMOUSESENSITIVITY, "MAXCURSOR"}
	, {INT_MAX, "MAX"}
	, {0, NULL}
};
CV_PossibleValue_t onecontrolperkey_cons_t[] = { {1, "One"}
	, {2, "Several"}
	, {0, NULL}
};

void M_MouseModeChange(void);

// mouse values are used once
consvar_t cv_mousesens = { "mousesens", "10", CV_ALIAS | CV_DEPRECATED, mousesens_cons_t, NULL, "m_xsensitivity" };
consvar_t cv_mousesensy = { "mousesensy", "10", CV_ALIAS | CV_DEPRECATED, mousesens_cons_t, NULL, "m_ysensitivity" };
consvar_t cv_mlooksens = { "mlooksens", "10", CV_SAVE, mousesens_cons_t };
consvar_t cv_mousesens2 = { "mousesens2", "10", CV_SAVE, mousesens_cons_t };
consvar_t cv_mlooksens2 = { "mlooksens2", "10", CV_SAVE, mousesens_cons_t };
consvar_t cv_legacymouse = { "legacymouse", "1", CV_ALIAS | CV_DEPRECATED, CV_YesNo, NULL, "m_legacymouse" };

consvar_t cv_m_xsensitivity = { "m_xsensitivity", "10", CV_SAVE, mousesens_cons_t };
consvar_t cv_m_ysensitivity = { "m_ysensitivity", "10", CV_SAVE, mousesens_cons_t };
consvar_t cv_m_legacymouse = { "m_legacymouse", "1", CV_SAVE | CV_CALL, CV_YesNo, M_MouseModeChange };

// GhostlyDeath <December 11, 2008> -- Fun Stuff :)
CV_PossibleValue_t mouseaxismode_cons_t[] =
{
	// Standard
	{0, "xmove"},				// Strafing
	{1, "xlook"},				// Turning left and right
	{2, "ymove"},				// Moving Forward and Backwards
	{3, "ylook"},				// Looking up and down
	{4, "zmove"},				// Vertical Movement
	//{5, "zlook"},         // Rolling the view
	
	// Inverted
	{6, "invertedxmove"},		// Strafing
	{7, "invertedxlook"},		// Turning left and right
	{8, "invertedymove"},		// Moving Forward and Backwards
	{9, "invertedylook"},		// Looking up and down
	{10, "invertedzmove"},		// Vertical Movement
	//{11, "invertedzlook"},// Rolling the view
	
	{12, "disabled"},			// Does Nothing
	{0, NULL}
};
consvar_t cv_m_xaxismode = { "m_xaxismode", "xlook", CV_SAVE, mouseaxismode_cons_t };
consvar_t cv_m_yaxismode = { "m_yaxismode", "ymove", CV_SAVE, mouseaxismode_cons_t };
consvar_t cv_m_xaxissecmode = { "m_xaxissecmode", "xlook", CV_SAVE, mouseaxismode_cons_t };
consvar_t cv_m_yaxissecmode = { "m_yaxissecmode", "ylook", CV_SAVE, mouseaxismode_cons_t };
consvar_t cv_m_classicalt = { "m_classicalt", "1", CV_SAVE, CV_YesNo };

consvar_t cv_allowjump = { "allowjump", "1", CV_NETVAR, CV_YesNo };
consvar_t cv_allowautoaim = { "allowautoaim", "1", CV_NETVAR, CV_YesNo };
consvar_t cv_forceautoaim = { "forceautoaim", "1", CV_NETVAR, CV_YesNo };
consvar_t cv_controlperkey = { "controlperkey", "1", CV_SAVE, onecontrolperkey_cons_t };

//SoM: 3/28/2000: Working rocket jumping.
consvar_t cv_allowrocketjump = { "allowrocketjump", "0", CV_NETVAR, CV_YesNo };
consvar_t cv_classicrocketblast = { "classicrocketblast", "0", CV_NETVAR, CV_YesNo };

int mousex;
int mousey;
int mlooky;						//like mousey but with a custom sensitivity

//for mlook
int mouse2x;
int mouse2y;
int mlook2y;

// joystick values are repeated
int joyxmove;
int joyymove;

// current state of the keys : true if pushed
uint8_t gamekeydown[NUMINPUTS];

// two key codes (or virtual key) per game control
int gamecontrol[MAXSPLITSCREENPLAYERS][num_gamecontrols][2];

typedef struct
{
	int time;
	int state;
	int clicks;
} dclick_t;
dclick_t mousedclicks[MAXSPLITSCREENPLAYERS][MOUSEBUTTONS];
dclick_t joydclicks[MAXSPLITSCREENPLAYERS][JOYBUTTONS];

//
//  General double-click detection routine for any kind of input.
//
static bool_t G_CheckDoubleClick(int state, dclick_t* dt)
{
	if (state != dt->state && dt->time > 1)
	{
		dt->state = state;
		if (state)
			dt->clicks++;
		if (dt->clicks == 2)
		{
			dt->clicks = 0;
			return true;
		}
		else
			dt->time = 0;
	}
	else
	{
		dt->time++;
		if (dt->time > 20)
		{
			dt->clicks = 0;
			dt->state = 0;
		}
	}
	
	return false;
}

//
//  Remaps the inputs to game controls.
//
//  A game control can be triggered by one or more keys/buttons.
//
//  Each key/mousebutton/joybutton triggers ONLY ONE game control.
//
void G_MapEventsToControls(event_t* ev)
{
	int i = 0;
	int j = 0;
	int flag = 0;
	
	switch (ev->type)
	{
		case ev_keydown:
			if (ev->data1 < NUMINPUTS)
				gamekeydown[ev->data1] = 1;
			break;
			
		case ev_keyup:
			if (ev->data1 < NUMINPUTS)
				gamekeydown[ev->data1] = 0;
			break;
			
		case ev_mouse:			// buttons are virtual keys
			if (cv_m_legacymouse.value)
			{
				mousex += ev->data2 * ((cv_m_xsensitivity.value * cv_m_xsensitivity.value) / 110.0 + 0.1);
				mousey += ev->data3 * ((cv_m_ysensitivity.value * cv_m_ysensitivity.value) / 110.0 + 0.1);
				
				//added:10-02-98:
				// for now I use the mlook sensitivity just for mlook,
				// instead of having a general mouse y sensitivity.
				mlooky = ev->data3 * ((cv_mlooksens.value * cv_mlooksens.value) / 110.0 + 0.1);
			}
			else
			{
				mousex += ev->data2 * ((cv_m_xsensitivity.value * cv_m_xsensitivity.value) / 110.0 + 0.1);
				mousey += ev->data3 * ((cv_m_ysensitivity.value * cv_m_ysensitivity.value) / 110.0 + 0.1);
			}
			break;
			
		case ev_joystick:		// buttons are virtual keys
			joyxmove = ev->data2;
			joyymove = ev->data3;
			break;
			
		case ev_mouse2:		// buttons hare virtual keys
			mouse2x = ev->data2 * ((cv_mousesens2.value * cv_mousesens2.value) / 110.0f + 0.1);
			mouse2y = ev->data3 * ((cv_mousesens2.value * cv_mousesens2.value) / 110.0f + 0.1);
			
			//added:10-02-98:
			// for now I use the mlook sensitivity just for mlook,
			// instead of having a general mouse y sensitivity.
			mlook2y = ev->data3 * ((cv_mlooksens.value * cv_mlooksens.value) / 110.0f + 0.1);
			break;
			
		default:
			break;
			
	}
	
	// ALWAYS check for mouse & joystick double-clicks
	// even if no mouse event
#if 0
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < MOUSEBUTTONS; i++)
		{
			flag = G_CheckDoubleClick(gamekeydown[KEY_MOUSE1B1 + (MOUSEBUTTONRANGE * j) + i], &mousedclicks[j][i]);
			gamekeydown[KEY_MOUSE1DBL1 + (MOUSEBUTTONRANGE * j) + i] = flag;
		}
	}
	
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < JOYBUTTONS; i++)
		{
			flag = G_CheckDoubleClick(gamekeydown[KEY_JOY1B1 + (JOYBUTTONRANGE * j) + i], &joydclicks[j][i]);
			gamekeydown[KEY_JOY1DBL1 + (JOYBUTTONRANGE * j) + i] = flag;
		}
	}
#endif
}

typedef struct
{
	int keynum;
	char name[15];
} keyname_t;

#define KEYNAMEEXTRAS (((((((MOUSEBUTTONS << 1) + 2) * 2) + ((JOYBUTTONS << 1))) * 4)) + (KEY_WORLDSTART - KEY_WORLDEND))

static keyname_t keynames[54 + KEYNAMEEXTRAS + 90] =  	// FIXME: +90 because there's an overflow somewhere
{
	// FIXME: Overflows in G_InitKeys(), 441 keys are named but this array is only 359 in size
	
	{KEY_SPACE, "SPACE"},
	{KEY_CAPSLOCK, "CAPS LOCK"},
	{KEY_ENTER, "ENTER"},
	{KEY_TAB, "TAB"},
	{KEY_ESCAPE, "ESCAPE"},
	{KEY_BACKSPACE, "BACKSPACE"},
	
	{KEY_NUMLOCK, "NUMLOCK"},
	{KEY_SCROLLLOCK, "SCROLLLOCK"},
	
	// bill gates keys
	
	{KEY_LEFTWIN, "LEFTWIN"},
	{KEY_RIGHTWIN, "RIGHTWIN"},
	{KEY_MENU, "MENU"},
	
	// shift,ctrl,alt are not distinguished between left & right
	
	{KEY_SHIFT, "SHIFT"},
	{KEY_CTRL, "CTRL"},
	{KEY_ALT, "ALT"},
	
	// keypad keys
	
	{KEY_KPADSLASH, "KEYPAD /"},
	
	{KEY_KEYPAD7, "KEYPAD 7"},
	{KEY_KEYPAD8, "KEYPAD 8"},
	{KEY_KEYPAD9, "KEYPAD 9"},
	{KEY_MINUSPAD, "KEYPAD -"},
	{KEY_KEYPAD4, "KEYPAD 4"},
	{KEY_KEYPAD5, "KEYPAD 5"},
	{KEY_KEYPAD6, "KEYPAD 6"},
	{KEY_PLUSPAD, "KEYPAD +"},
	{KEY_KEYPAD1, "KEYPAD 1"},
	{KEY_KEYPAD2, "KEYPAD 2"},
	{KEY_KEYPAD3, "KEYPAD 3"},
	{KEY_KEYPAD0, "KEYPAD 0"},
	{KEY_KPADDEL, "KEYPAD ."},
	
	// extended keys (not keypad)
	
	{KEY_HOME, "HOME"},
	{KEY_UPARROW, "UP ARROW"},
	{KEY_PGUP, "PGUP"},
	{KEY_LEFTARROW, "LEFT ARROW"},
	{KEY_RIGHTARROW, "RIGHT ARROW"},
	{KEY_END, "END"},
	{KEY_DOWNARROW, "DOWN ARROW"},
	{KEY_PGDN, "PGDN"},
	{KEY_INS, "INS"},
	{KEY_DEL, "DEL"},
	
	// other keys
	
	{KEY_F1, "F1"},
	{KEY_F2, "F2"},
	{KEY_F3, "F3"},
	{KEY_F4, "F4"},
	{KEY_F5, "F5"},
	{KEY_F6, "F6"},
	{KEY_F7, "F7"},
	{KEY_F8, "F8"},
	{KEY_F9, "F9"},
	{KEY_F10, "F10"},
	{KEY_F11, "F11"},
	{KEY_F12, "F12"},
	{KEY_F13, "F13"},
	{KEY_F14, "F14"},
	{KEY_F15, "F15"},
	
	/* "World" Keys */
	{KEY_EVILEURO, "EURO"},
	
};

char* gamecontrolname[num_gamecontrols] =
{
	"nothing",					//a key/button mapped to gc_null has no effect
	"forward",
	"backward",
	"strafe",
	"straferight",
	"strafeleft",
	"speed",
	"turnleft",
	"turnright",
	"fire",
	"use",
	"lookup",
	"lookdown",
	"centerview",
	"mouseaiming",
	"weapon1",
	"weapon2",
	"weapon3",
	"weapon4",
	"weapon5",
	"weapon6",
	"weapon7",
	"weapon8",
	"talkkey",
	"scores",
	"jump",
	"console",
	"nextweapon",
	"prevweapon",
	"bestweapon",
	"inventorynext",
	"inventoryprev",
	"inventoryuse",
	"down"
};

#define NUMKEYNAMES (sizeof(keynames)/sizeof(keyname_t))

/* G_InitKeys() -- Add stuff to our static array instead of going crazy! */
// FIXME: Overflow somewhere
void G_InitKeys(void)
{
	size_t i = 54;
	size_t j, k;
	
	// World Keys
	for (j = KEY_WORLDSTART; j <= KEY_WORLDEND; j++, i++)
		sprintf(keynames[i].name, "WORLD%02d", j - KEY_WORLDSTART);
		
	// Mouse First
	for (j = 0; j < 2; j++)
	{
		for (k = 0; k < MOUSEBUTTONS; k++, i++)
			sprintf(keynames[i].name, "MOUSE%dB%01d", j + 1, k + 1);
		for (k = 0; k < MOUSEBUTTONS; k++, i++)
			sprintf(keynames[i].name, "MOUSE%dDBL%01d", j + 1, k + 1);
		sprintf(keynames[i].name, "MOUSE%dWHEELUP", j + 1);
		i++;
		sprintf(keynames[i].name, "MOUSE%dWHEELDOWN", j + 1);
		i++;
	}
	
	// Joystick Second
	for (j = 0; j < 4; j++)
	{
		for (k = 0; k < JOYBUTTONS; k++, i++)
			sprintf(keynames[i].name, "JOY%dB%02d", j + 1, k + 1);
		for (k = 0; k < JOYBUTTONS; k++, i++)
			sprintf(keynames[i].name, "JOY%dDBL%02d", j + 1, k + 1);
	}
	
	for (i = 54; i < NUMKEYNAMES; i++)
		keynames[i].keynum = NUMKEYS + (i - 54);
}

//
//  Detach any keys associated to the given game control
//  - pass the pointer to the gamecontrol table for the player being edited
void G_ClearControlKeys(int (*setupcontrols)[2], int control)
{
	setupcontrols[control][0] = KEY_NULL;
	setupcontrols[control][1] = KEY_NULL;
}

//
//  Returns the name of a key (or virtual key for mouse and joy)
//  the input value being an keynum
//
char* G_KeynumToString(int keynum)
{
	static char keynamestr[8];
	
	int j;
	
	// return a string with the ascii char if displayable
	if (keynum > ' ' && keynum <= 'z' && keynum != KEY_CONSOLE)
	{
		keynamestr[0] = keynum;
		keynamestr[1] = '\0';
		return keynamestr;
	}
	// find a description for special keys
	for (j = 0; j < NUMKEYNAMES; j++)
		if (keynames[j].keynum == keynum)
			return keynames[j].name;
			
	// create a name for Unknown key
	sprintf(keynamestr, "KEY%d", keynum);
	return keynamestr;
}

int G_KeyStringtoNum(char* keystr)
{
	int j;
	
//    C_strupr(keystr);

	if (keystr[1] == 0 && keystr[0] > ' ' && keystr[0] <= 'z')
		return keystr[0];
		
	for (j = 0; j < NUMKEYNAMES; j++)
		if (strcasecmp(keynames[j].name, keystr) == 0)
			return keynames[j].keynum;
			
	if (strlen(keystr) > 3)
		return atoi(&keystr[3]);
		
	return 0;
}

void G_Controldefault(void)
{
	gamecontrol[0][gc_forward][0] = KEY_UPARROW;
	gamecontrol[0][gc_forward][1] = KEY_MOUSE1B1 + 2;
	gamecontrol[0][gc_backward][0] = KEY_DOWNARROW;
	gamecontrol[0][gc_strafe][0] = KEY_ALT;
	gamecontrol[0][gc_strafe][1] = KEY_MOUSE1B1 + 1;
	gamecontrol[0][gc_straferight][0] = '.';
	gamecontrol[0][gc_strafeleft][0] = ',';
	gamecontrol[0][gc_speed][0] = KEY_SHIFT;
	gamecontrol[0][gc_turnleft][0] = KEY_LEFTARROW;
	gamecontrol[0][gc_turnright][0] = KEY_RIGHTARROW;
	gamecontrol[0][gc_fire][0] = KEY_CTRL;
	gamecontrol[0][gc_fire][1] = KEY_MOUSE1B1;
	gamecontrol[0][gc_use][0] = KEY_SPACE;
	gamecontrol[0][gc_lookup][0] = KEY_PGUP;
	gamecontrol[0][gc_lookdown][0] = KEY_PGDN;
	gamecontrol[0][gc_centerview][0] = KEY_END;
	gamecontrol[0][gc_mouseaiming][0] = 's';
	gamecontrol[0][gc_weapon1][0] = '1';
	gamecontrol[0][gc_weapon2][0] = '2';
	gamecontrol[0][gc_weapon3][0] = '3';
	gamecontrol[0][gc_weapon4][0] = '4';
	gamecontrol[0][gc_weapon5][0] = '5';
	gamecontrol[0][gc_weapon6][0] = '6';
	gamecontrol[0][gc_weapon7][0] = '7';
	gamecontrol[0][gc_weapon8][0] = '8';
	gamecontrol[0][gc_talkkey][0] = 't';
	gamecontrol[0][gc_scores][0] = 'f';
	gamecontrol[0][gc_jump][0] = '/';
	gamecontrol[0][gc_console][0] = KEY_CONSOLE;
	gamecontrol[0][gc_nextweapon][1] = KEY_JOY1B1 + 4;
	gamecontrol[0][gc_prevweapon][1] = KEY_JOY1B1 + 5;
	
	gamecontrol[0][gc_invnext][0] = '\'';
	gamecontrol[0][gc_invprev][0] = ';';
	gamecontrol[0][gc_nextweapon][0] = ']';
	gamecontrol[0][gc_prevweapon][0] = '[';
	gamecontrol[0][gc_invuse][0] = KEY_ENTER;
	gamecontrol[0][gc_flydown][0] = KEY_DEL;
}

void G_SaveKeySetting(FILE* f)
{
	int i;
	int j;
	
	for (j = 0; j < MAXSPLITSCREENPLAYERS; j++)
		for (i = 1; i < num_gamecontrols; i++)
		{
			fprintf(f, "setcontrol%i \"%s\" \"%s\"", j + 1, gamecontrolname[i], G_KeynumToString(gamecontrol[j][i][0]));
			
			if (gamecontrol[j][i][1])
				fprintf(f, " \"%s\"\n", G_KeynumToString(gamecontrol[j][i][1]));
			else
				fprintf(f, "\n");
		}
}

void G_CheckDoubleUsage(int keynum)
{
	if (cv_controlperkey.value == 1)
	{
		int i;
		int j;
		
		for (j = 0; j < MAXSPLITSCREENPLAYERS; j++)
			for (i = 0; i < num_gamecontrols; i++)
			{
				if (gamecontrol[j][i][0] == keynum)
					gamecontrol[j][i][0] = KEY_NULL;
				if (gamecontrol[j][i][1] == keynum)
					gamecontrol[j][i][1] = KEY_NULL;
			}
	}
}

void setcontrol(int (*gc)[2], int na)
{
	int numctrl;
	char* namectrl;
	int keynum;
	
	namectrl = COM_Argv(1);
	for (numctrl = 0; numctrl < num_gamecontrols && strcasecmp(namectrl, gamecontrolname[numctrl]); numctrl++);
	if (numctrl == num_gamecontrols)
	{
		CONS_Printf("Control '%s' unknown\n", namectrl);
		return;
	}
	keynum = G_KeyStringtoNum(COM_Argv(2));
	G_CheckDoubleUsage(keynum);
	gc[numctrl][0] = keynum;
	
	if (na == 4)
		gc[numctrl][1] = G_KeyStringtoNum(COM_Argv(3));
	else
		gc[numctrl][1] = 0;
}

void Command_Setcontrol_f(void)
{
	int na;
	int j;
	
	na = COM_Argc();
	
	j = ((COM_Argv(0))[10]) - '1';
	
	if (na != 3 && na != 4)
	{
		CONS_Printf("%s <controlname> <keyname> [<2nd keyname>]\n", COM_Argv(0));
		CONS_Printf("# is a value between 1 and %i.\n", MAXSPLITSCREENPLAYERS + 1);
		return;
	}
	
	setcontrol(gamecontrol[j], na);
}
