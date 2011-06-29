// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
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
// DESCRIPTION: Menu widget stuff, episode selection and such.

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"
#include "command.h"
#include "d_prof.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
boolean M_Responder(event_t * ev);

// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker(void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer(void);

// Called by D_DoomMain,
// loads the config file.
void M_Init(void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel(void);

// Draws a box with a texture inside as background for messages
void M_DrawTextBox(int x, int y, int width, int lines);
// show or hide the setup for player 2 (called at splitscreen change)
void M_SwitchSplitscreen(void);

// the function to show a message box typing with the string inside
// string must be static (not in the stack)
// routine is a function taking a int in parameter
typedef enum
{
	MM_NOTHING = 0,				// is just displayed until the user do someting
	MM_YESNO,					// routine is called with only 'y' or 'n' in param
	MM_EVENTHANDLER				// the same of above but without 'y' or 'n' restriction
		// and routine is void routine(event_t *) (ex: set control)
} menumessagetype_t;
void M_StartMessage(const char *string, void *routine, menumessagetype_t itemtype);

// Called by linux_x/i_video_xshm.c
void M_QuitResponse(int ch);

/* Global Menu Stuff */
// flags for items in the menu
// menu handle (what we do when key is pressed
#define  IT_TYPE             14	// (2+4+8)
#define  IT_CALL              0	// call the function
#define  IT_ARROWS            2	// call function with 0 for left arrow and 1 for right arrow in param
#define  IT_KEYHANDLER        4	// call with the key in param
#define  IT_SUBMENU           6	// go to sub menu
#define  IT_CVAR              8	// hangdle as a cvar
#define  IT_SPACE            10	// no handling
#define  IT_MSGHANDLER       12	// same as key but with event and sometime can handle y/n key (special for message

#define  IT_DISPLAY  (48+64+128)	// 16+32+64
#define  IT_NOTHING           0	// space
#define  IT_PATCH            16	// a patch or a string with big font
#define  IT_STRING           32	// little string (spaced with 10)
#define  IT_WHITESTRING      48	// little string in white
#define  IT_DYBIGSPACE       64	// same as noting
#define  IT_DYLITLSPACE  (16+64)	// little space
#define  IT_STRING2      (32+64)	// a simple string
#define  IT_GRAYPATCH    (16+32+64)	// grayed patch or big font string
#define  IT_BIGSLIDER     (128)	// volume sound use this
#define  IT_DISABLED2   (524288)
#define  IT_CENTERSTRING (1048576)
#define  IT_CVARREADONLY (2097152)	// CVAR is read-only!

//consvar specific
#define  IT_CVARTYPE   (256+512+1024)
#define  IT_CV_NORMAL         0
#define  IT_CV_SLIDER       256
#define  IT_CV_STRING       512
#define  IT_CV_NOPRINT (256+512)
#define  IT_CV_NOMOD       1024

// in short for some common use
#define  IT_BIGSPACE    (IT_SPACE  +IT_DYBIGSPACE)
#define  IT_LITLSPACE   (IT_SPACE  +IT_DYLITLSPACE)
#define  IT_CONTROL     (IT_STRING2+IT_CALL)
#define  IT_CVARMAX     (IT_CVAR   +IT_CV_NOMOD)
#define  IT_DISABLED    (IT_SPACE  +IT_GRAYPATCH)

#define SKULLXOFF       -32
#define LINEHEIGHT       16
#define STRINGHEIGHT     10
#define FONTBHEIGHT      20
#define SMALLLINEHEIGHT   8
#define SLIDER_RANGE     10
#define SLIDER_WIDTH    (8*SLIDER_RANGE+6)
#define MAXSTRINGLENGTH  32

// GhostlyDeath <July 6, 2008> -- Sub menu items
#define ITX_SUBMENUTITLE (IT_WHITESTRING | IT_CENTERSTRING | IT_SPACE)
#define ITX_MAINMENUITEM (IT_STRING | IT_CENTERSTRING)

typedef union
{
	struct menu_s *submenu;		// IT_SUBMENU
	consvar_t *cvar;			// IT_CVAR
	void (*routine) (int choice);	// IT_CALL, IT_KEYHANDLER, IT_ARROWS
} itemaction_t;

//
// MENU TYPEDEFS
//
typedef struct menuitem_s
{
	// show IT_xxx
	int status;

	char *patch;
	char** WItemTextPtr;
	//char *text;					// used when FONTBxx lump is found

	// FIXME: should be itemaction_t !!!
	void *itemaction;

	// hotkey in menu
	// or y of the item 
	byte alphaKey;
} menuitem_t;

#define MENUFLAG_OPTIMALSPACE	1			// Use automatic spacing!
#define MENUFLAG_HIDECURSOR		2			// Don't show the cursor

#define MENUPADDING 10

typedef struct menu_s
{
	int extraflags;				// Flags if we ever need them
	char *menutitlepic;
	char** WMenuTitlePtr;	// Pointer to string specification
	//char *menutitle;			// title as string for display with fontb if present
	short numitems;				// # of menu items
	menuitem_t *menuitems;		// menu items
	struct menu_s *prevMenu;	// previous menu
	void (*drawroutine) (void);	// draw routine
	short x;
	short y;					// x,y of menu (of the draw spot)
	short width;				// if !0, don't use 320
	short height;				// if !0, don't use 200
	short numcolumns;			// Number of columns ((x <= 0) = 1)
	short menutitlex;			// X Position to draw menutitle
	short menutitley;			// Y position to draw menutitle
	short lastOn;				// last item user was on in menu
	 boolean(*quitroutine) (void);	// called before quit a menu return true if we can
	 
	// GhostlyDeath <July 6, 2008> -- For scrolling menus
	short itemsperpage;
	short firstdraw;
} menu_t;

void M_DrawSaveLoadBorder(int x, int y);
void M_SetupNextMenu(menu_t * menudef);

void M_DrawGenericMenu(void);
void M_DrawControl(void);
void M_DrawTextBox(int x, int y, int width, int lines);	//added:06-02-98:
void M_DrawThermo(int x, int y, consvar_t * cv);
void M_DrawEmptyCell(menu_t * menu, int item);
void M_DrawSelCell(menu_t * menu, int item);
void M_DrawSlider(int x, int y, int range, void* extra);
void M_CentreText(int y, char *string);	//added:30-01-98:writetext centered

void M_StartControlPanel(void);
void M_StopMessage(int choice);
void M_ClearMenus(boolean callexitmenufunc);
int M_StringHeight(char *string);
void M_GameOption(int choice);
void M_NetOption(int choice);
//28/08/99: added by Hurdler
void M_OpenGLOption(int choice);
void M_ChangeControl(int choice);

/* From m_menu.c */
extern int quickSaveSlot;
extern boolean menuactive;
extern int saveStringEnter;
extern int saveSlot;
extern int saveCharIndex;
extern char saveOldString[SAVESTRINGSIZE];
extern char savegamestrings[10][SAVESTRINGSIZE];
extern menu_t *currentMenu;
extern short itemOn;
extern short skullAnimCounter;
extern short whichSkull;
extern int SkullBaseLump;
extern char skullName[2][9];
extern consvar_t cv_blinkingrate;
extern consvar_t cv_cons_blinkingrate;

extern menu_t
	MainDef,					// Main Menu
	OptionsDef,					// Options Menu
	DefaultKeyBindDef,			// Key Binds
	GameOptionsDef,				// Game Options
	NewGameDef,					// New Game
	NewGameClassicDef,			// New Game -> Create Game
	SoundsDef,					// Options -> Sound
	VideoDef,					// Options -> Video
	CreateLocalGameDef,			// New Game -> Local
	NewGameOptionsDef,			// New Game -> Options
	NewGameCCSkillDef,
	NewGameCCEpiDef,
	
	ControlSettingsDef,
	GraphicalSettingsDef,
	
	LASTMENU
	;
	
extern menu_t ProfileDef;
	
extern menu_t* MenuPtrList[];
	
// Menu Functions
void M_QuitDOOM(int choice);
void M_QuitResponse(int ch);
void M_EndGame(int choice);

extern consvar_t cv_skill;
extern consvar_t cv_monsters;
extern consvar_t cv_nextmap;
extern consvar_t cv_newdeathmatch;

void M_LockGameCVARS(void);
void M_UnLockGameCVARS(void);

void M_ControlsDoPlayer1(int choice);
void M_ControlsDoPlayer2(int choice);
void M_ControlsDoPlayer3(int choice);
void M_ControlsDoPlayer4(int choice);

void M_DoNewGameClassicClassic(int choice);
void M_SelectSkill(int choice);
void M_SelectEpisode(int choice);

void M_DoNewGameClassic(int choice);
void M_StartClassicGame(int choice);

void M_DoNewGameLocal(int choice);
void M_StartLocalGame(int choice);

void M_ClassicGameOptions(int choice);
void M_LocalGameOptions(int choice);

void M_HandleVideoKey(int choice);
void M_DrawVideoOptions(void);
void M_StartVideoOptions(int choice);

void M_ResetSound(int choice);

void M_MouseModeChange(void);

extern consvar_t cv_ng_map;
extern consvar_t cv_ng_skill;
extern consvar_t cv_ng_options;
extern consvar_t cv_ng_splitscreen;
extern consvar_t cv_ng_map;
extern consvar_t cv_ng_skill;
extern consvar_t cv_ng_options;
extern consvar_t cv_ng_deathmatch;
extern consvar_t cv_ng_teamplay;
extern consvar_t cv_ng_teamdamage;
extern consvar_t cv_ng_fraglimit;
extern consvar_t cv_ng_timelimit;
extern consvar_t cv_ng_allowexitlevel;
extern consvar_t cv_ng_allowjump;
extern consvar_t cv_ng_allowautoaim;
extern consvar_t cv_ng_forceautoaim;
extern consvar_t cv_ng_allowrocketjump;
extern consvar_t cv_ng_classicrocketblast;
extern consvar_t cv_ng_allowturbo;
extern consvar_t cv_ng_itemrespawntime;
extern consvar_t cv_ng_itemrespawn;
extern consvar_t cv_ng_spawnmonsters;
extern consvar_t cv_ng_respawnmonsters;
extern consvar_t cv_ng_respawnmonsterstime;
extern consvar_t cv_ng_solidcorpse;
extern consvar_t cv_ng_fastmonsters;
extern consvar_t cv_ng_predictingmonsters;
extern consvar_t cv_ng_classicmonsterlogic;
extern consvar_t cv_ng_gravity;
extern consvar_t cv_ng_classicblood;
extern consvar_t cv_ng_classicmeleerange;
extern consvar_t cv_ng_fragsweaponfalling;
extern consvar_t cv_ng_bloodtime;
extern consvar_t cv_ng_infiniteammo;

#endif

