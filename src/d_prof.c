// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Profiles

#include "d_prof.h"
#include "z_zone.h"
#include "i_util.h"
#include "v_video.h"
#include "dstrings.h"
#include "g_state.h"
#include "console.h"
#include "d_netcmd.h"

#include "j.h"

static jclass __profman_class = NULL;
static jobject __profman = NULL;

extern char g_ConfigDir[PATH_MAX];

void J_InitProfiles(void)
{
	jmethodID con;
	
	// Find and construct the profile manager
	__profman_class =
		J_FindClass("org/remood/remood/core/profile/ProfileManager");
	
	// Get the constructor
	con = J_GetMethodID(__profman_class, "<init>", "(Ljava/lang/String;)V");
	
	// Construct profile manager
	__profman = J_NewObject(__profman_class, con, J_NewStringUTF(g_ConfigDir));
}

const char* D_ProfileDisplayName(D_Prof_t* __prof)
{
	I_Error("TODO");
	return NULL;
}

const char* D_ProfileAccountName(D_Prof_t* __prof)
{
	I_Error("TODO");
	return NULL;
}

D_Prof_t* D_ProfileGetIndex(int __i)
{
	I_Error("TODO");
	return NULL;
}

void D_ProfileRename(D_Prof_t* __prof, const char* __new)
{
	I_Error("TODO");
}

fixed_t D_ProfileViewHeight(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 0;
}

const char* D_ProfileUUID(D_Prof_t* __prof)
{
	I_Error("TODO");
	return NULL;
}

int D_ProfileBobMode(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 0;
}

bool_t D_ProfileDrawGunSprite(D_Prof_t* __prof)
{
	I_Error("TODO");
	return true;
}

int D_ProfileRawControl(D_Prof_t* __prof, int __key, int __i)
{
	I_Error("TODO");
	return 0;
}

D_ProfileExInputCtrl_t D_ProfileEnumToInputCtrl(jobject __jo)
{
	I_Error("TODO");
	return 0;
}

jobject D_ProfileInputCtrlToControl(D_ProfileExInputCtrl_t __ctrl)
{
	I_Error("TODO");
	return NULL;
}

bool_t D_ProfileUseSlowTurn(D_Prof_t* __prof)
{
	I_Error("TODO");
	return false;
}

int D_ProfileSlowTurnTime(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 0;
}

bool_t D_ProfileUseAutoRun(D_Prof_t* __prof)
{
	I_Error("TODO");
	return false;
}

bool_t D_ProfileUseLookSpring(D_Prof_t* __prof)
{
	I_Error("TODO");
	return false;
}

int D_ProfileLookUpDownSpeed(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 1;
}

int D_ProfileVTeam(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 0;
}

int D_ProfileColor(D_Prof_t* __prof)
{
	I_Error("TODO");
	return 0;
}

bool_t D_ProfileIsCounterOp(D_Prof_t* __prof)
{
	I_Error("TODO");
	return false;
}

/****************************************************************************/

#if 0


/************************
*** EXTENDED PROFILES ***
************************/

/*** CONSTANTS ***/
static const struct
{
	const char ShortName[16];					// Short Name
	const char LongName[32];					// Long Name (menus)
	D_ProfileExInputCtrl_t ID;					// For Reference
} c_ControlMapper[NUMDPROFILEEXINPUTCTRLS] =
{
	{"null", "Nothing", DPEXIC_NULL},
	
	{"modspeed", "Speed Modifier", DPEXIC_SPEED},
	{"modmove", "Movement Modifier", DPEXIC_MOVEMENT},
	{"modlook", "Look Modifier", DPEXIC_LOOKING},
	
	{"forwards", "Move Forwards", DPEXIC_FORWARDS},
	{"backwards", "Move Backwards", DPEXIC_BACKWARDS},
	{"strafeleft", "Strafe Left", DPEXIC_STRAFELEFT},
	{"straferight", "Strafe Right", DPEXIC_STRAFERIGHT},
	{"flyup", "Fly/Swim Up", DPEXIC_FLYUP},
	{"flydown", "Fly/Swim Down", DPEXIC_FLYDOWN},
	{"land", "Land", DPEXIC_LAND},
	{"jump", "Jump", DPEXIC_JUMP},
	
	/* Looking */
	{"turnleft", "Turn Left", DPEXIC_TURNLEFT},
	{"turnright", "Turn Right", DPEXIC_TURNRIGHT},
	{"turn180", "Turn 180\xC2\xb0", DPEXIC_TURNSEMICIRCLE},
	{"lookup", "Look Up", DPEXIC_LOOKUP},
	{"lookdown", "Look Down", DPEXIC_LOOKDOWN},
	{"lookcenter", "Center View", DPEXIC_LOOKCENTER},
	
	/* Actions */
	{"use", "Use Action", DPEXIC_USE},
	{"suicide", "Commit Suicide", DPEXIC_SUICIDE},
	{"taunt", "Taunt", DPEXIC_TAUNT},
	{"chat", "Chat", DPEXIC_CHAT},
	{"teamchat", "Chat With Team", DPEXIC_TEAMCHAT},
	
	/* Weapons */
	{"attack", "Attack", DPEXIC_ATTACK},
	{"altattack", "Secondary Attack", DPEXIC_ALTATTACK},
	{"reload", "Reload Weapon", DPEXIC_RELOAD},
	{"switchfire", "Switch Firing Mode", DPEXIC_SWITCHFIREMODE},
	{"slot1", "Weapon Slot 1", DPEXIC_SLOT1},
	{"slot2", "Weapon Slot 2", DPEXIC_SLOT2},
	{"slot3", "Weapon Slot 3", DPEXIC_SLOT3},
	{"slot4", "Weapon Slot 4", DPEXIC_SLOT4},
	{"slot5", "Weapon Slot 5", DPEXIC_SLOT5},
	{"slot6", "Weapon Slot 6", DPEXIC_SLOT6},
	{"slot7", "Weapon Slot 7", DPEXIC_SLOT7},
	{"slot8", "Weapon Slot 8", DPEXIC_SLOT8},
	{"slot9", "Weapon Slot 9", DPEXIC_SLOT9},
	{"slot10", "Weapon Slot 10", DPEXIC_SLOT10},
	{"nextweapon", "Next Weapon", DPEXIC_NEXTWEAPON},
	{"prevweapon", "Previous Weapon", DPEXIC_PREVWEAPON},
	{"bestweapon", "Best Weapon", DPEXIC_BESTWEAPON},
	{"worstweapon", "Worst Weapon", DPEXIC_WORSTWEAPON},
	
	/* Inventory */
	{"nextinventory", "Inventory Cursor Next", DPEXIC_NEXTINVENTORY},
	{"previnventory", "Inventory Cursor Previous", DPEXIC_PREVINVENTORY},
	{"useinventory", "Use Inventory Item", DPEXIC_USEINVENTORY},
	{"cancelinventory", "Cancel Inventory Selection", DPEXIC_CANCELINVENTORY},
	
	/* General */
	{"topscores", "Show Top Scores", DPEXIC_TOPSCORES},
	{"worstscores", "Show Worst Scores", DPEXIC_BOTTOMSCORES},
	{"coopspy", "Switch Cooperative Spy Player", DPEXIC_COOPSPY},
	{"automap",	"Toggle the Automap", DPEXIC_AUTOMAP},
	{"chatmode", "Enter Chat Mode", DPEXIC_CHATMODE},
	{"popupmenu", "Show the Menu", DPEXIC_POPUPMENU},
	{"morestuff", "More Commands Modifier", DPEXIC_MORESTUFF},
	{"quickmenu", "Quick Selection Menu", DPEXIC_QUICKMENU},
	{"moremorestuff", "Extra More Commands Modifier", DPEXIC_MOREMORESTUFF},
};

/* c_AxisMap -- Map of axis names */
static const char* const c_AxisMap[NUMDPROFILEEXCTRLMAS] =
{
	"null",										// DPEXCMA_NULL
	"movex",									// DPEXCMA_MOVEX
	"movey",									// DPEXCMA_MOVEY,
	"lookx",									// DPEXCMA_LOOKX
	"looky",									// DPEXCMA_LOOKY,
	
	"negmovex",									// DPEXCMA_NEGMOVEX
	"negmovey",									// DPEXCMA_NEGMOVEY,
	"neglookx",									// DPEXCMA_NEGLOOKX,
	"neglooky",									// DPEXCMA_NEGLOOKY,
	
	"pany",										// DPEXCMA_PANY
	"negpany",									// DPEXCMA_NEGPANY
	
	"angpany",									// DPEXCMA_ANGPANY
	"negangpany",								// DPEXCMA_NEGANGPANY
};

/* D_PDST_t -- Profile Data Stat Type */
typedef enum D_PDST_e
{
	PDST_UINT8,
	PDST_UINT32,
	PDST_INT32,
	PDST_FIXED,
	PDST_BOOL,
	PDST_STRING,
	PDST_TIC,
} D_PDST_t;

/* c_ProfDataStat -- Simplified config space */
static const struct
{
	const char ArgName[16];
	size_t Offset;
	D_PDST_t Type;
} c_ProfDataStat[] =
{
#define QUICKDS(x,s) {#x, offsetof(D_Prof_t, x), s}
	QUICKDS(DisplayName, PDST_STRING),
	QUICKDS(Color, PDST_UINT8),
	QUICKDS(VTeam, PDST_UINT8),
	QUICKDS(JoyControl, PDST_UINT8),
	QUICKDS(SlowTurnTime, PDST_TIC),
	QUICKDS(MouseSens[0], PDST_INT32),
	QUICKDS(MouseSens[1], PDST_INT32),
	QUICKDS(JoySens[0], PDST_INT32),
	QUICKDS(JoySens[1], PDST_INT32),
	QUICKDS(LookUpDownSpeed, PDST_INT32),
	QUICKDS(AutoGrabJoy, PDST_INT32),
	QUICKDS(ColorPickup, PDST_UINT8),
	QUICKDS(ColorSecret, PDST_UINT8),
	QUICKDS(ColorLock[0], PDST_UINT8),
	QUICKDS(ColorLock[1], PDST_UINT8),
	QUICKDS(ColorLock[2], PDST_UINT8),
	QUICKDS(ColorLock[3], PDST_UINT8),
	QUICKDS(SoundSecret, PDST_STRING),
	QUICKDS(DrawPSprites, PDST_BOOL),
	QUICKDS(BobMode, PDST_UINT8),
	QUICKDS(ViewHeight, PDST_FIXED),
	QUICKDS(CamDist, PDST_FIXED),
	QUICKDS(CamHeight, PDST_FIXED),
	QUICKDS(CamSpeed, PDST_FIXED),
	QUICKDS(ChaseCam, PDST_BOOL),
	QUICKDS(TransSBar, PDST_BOOL),
	QUICKDS(ScaledSBar, PDST_BOOL),
	QUICKDS(HexenClass, PDST_STRING),
	QUICKDS(AutoRun, PDST_BOOL),
	QUICKDS(SlowTurn, PDST_BOOL),
	QUICKDS(LookSpring, PDST_BOOL),
	QUICKDS(JoyAutoRun, PDST_BOOL),
	QUICKDS(CounterOp, PDST_BOOL),
	QUICKDS(BarType, PDST_UINT8),
	
	{"", 0, 0},
#undef QUICKDS
};

/*** GLOBALS ***/

D_Prof_t* g_KeyDefaultProfile = NULL;			// Profile with our key defaults
D_Prof_t* g_ProfList[MAXPROFCONST];				// Constant profile list

/*** LOCALS ***/

static D_Prof_t* l_FirstProfile = NULL;	// First in chain

static bool_t l_DefaultCtrlsMapped = false;
static D_ProfileExCtrlMA_t l_DefaultMouseAxis[MAXALTAXIS][MAXMOUSEAXIS];		// Mouse Axis Movement
static D_ProfileExCtrlMA_t l_DefaultJoyAxis[MAXALTAXIS][MAXJOYAXIS];	// Joy Axis Movement
static uint32_t l_DefaultCtrls[NUMDPROFILEEXINPUTCTRLS][4];

/*** FUNCTIONS ***/

/* D_ProfFixAccountName() -- Fixes account name */
void D_ProfFixAccountName(char* const a_Buffer)
{
#define BUFSIZE MAXPLAYERNAME
	char Buf[BUFSIZE];
	char* d, *s, c;
	
	/* Check */
	if (!a_Buffer)
		return;
	
	/* Copy old name to buffer */
	memset(Buf, 0, sizeof(Buf));
	strncpy(Buf, a_Buffer, BUFSIZE - 1);
	
	/* Slowly Copy */
	for (d = a_Buffer, s = Buf; *s; s++)
	{
		// Get current character
		c = tolower(*s);
		
		// Legal Character
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
			*(d++) = c;
	}
	
	// No characters?
	if (d == a_Buffer)
		*(d++) = 'x';
	
	// Always place ending zero
	*(d++) = 0;

#undef BUFSIZE
}

/* D_ProfFirst() -- Returns the first profile */
D_Prof_t* D_ProfFirst(void)
{
	return l_FirstProfile;
}

/* D_ProfRename() -- Rename profile account */
bool_t D_ProfRename(D_Prof_t* a_Prof, const char* const a_NewName)
{
	char FixedName[MAXPLAYERNAME];
	
	/* Check */
	if (!a_Prof || !a_NewName)
		return false;
	
	/* Fix name */
	strncpy(FixedName, a_NewName, MAXPLAYERNAME - 1);
	FixedName[MAXPLAYERNAME - 1] = 0;
	
	// Fixup
	D_ProfFixAccountName(FixedName);
	
	// Already exists?
	if (D_FindProfileEx(FixedName))
		return false;
	
	/* Change Name */
	memmove(a_Prof->AccountName, FixedName, sizeof(char) * MAXPLAYERNAME);
	return true;
}

/* D_CreateProfileEx() -- Create Profile */
D_Prof_t* D_CreateProfileEx(const char* const a_Name)
{
	char FixedName[MAXPLAYERNAME];
	D_Prof_t* New;
	size_t i;
	char Char;
	
	/* Check */
	if (!a_Name)
		return NULL;
		
	/* Correct Name */
	// Copy
	strncpy(FixedName, a_Name, MAXPLAYERNAME - 1);
	FixedName[MAXPLAYERNAME - 1] = 0;
	
	// Fix
	D_ProfFixAccountName(FixedName);
	
	// Already exists?
	if (D_FindProfileEx(FixedName))
		return NULL;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// find new slot in list
	for (i = 0; i < MAXPROFCONST; i++)
		if (!g_ProfList[i])
		{
			g_ProfList[i] = New;
			break;
		}
	
	/* Set properties */
	// UUID (hopefully random)
	D_CMakeUUID(New->UUID);
	
	// First character is never random
	New->UUID[0] = a_Name[0];
	
	// Instance ID (multiplayer)
	New->InstanceID = D_CMakePureRandom();
	
	/* Copy Name */
	strncpy(New->AccountName, FixedName, MAXPLAYERNAME - 1);
	strncpy(New->DisplayName, a_Name, MAXPLAYERNAME - 1);
	
	/* Set Default Options */
	New->Flags |= DPEXF_GOTMOUSE | DPEXF_GOTJOY;
	New->SlowTurnTime = 6;
	New->SlowTurn = true;
	New->JoyAutoRun = true;
	
	// Default Controls (First Time)
	if (!l_DefaultCtrlsMapped)
	{
#define SETKEY_M(a,b) a##b
#define SETKEY(x,c,k) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_KEY | (SETKEY_M(IKBK_,k))
#define SETJOY(x,c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_JOY | ((b) - 1)
#define SETMOUSE(x,c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_MOUSE | ((b) - 1)
#define SETDBLMOUSE(x,c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_DMOUSE | ((b) - 1)

#define SETJOYMORE(x,c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_JOYP | ((b) - 1)
#define SETKEYMORE(x,c,k) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_KEYP | (SETKEY_M(IKBK_,k))

#define SETJOYXTRA(x,c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_JOYX | ((b) - 1)
#define SETKEYXTRA(x,c,k) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][x] = PRFKBIT_KEYX | (SETKEY_M(IKBK_,k))
		
		// Default Controls
		if (g_ModelMode == DMM_DEFAULT)
		{
			SETMOUSE(1, ATTACK, 1);
			SETMOUSE(1, MOVEMENT, 2);
			SETMOUSE(1, PREVWEAPON, 5);
			SETMOUSE(1, NEXTWEAPON, 4);
			SETDBLMOUSE(1, USE, 2);
			
			SETKEY(0, SPEED, SHIFT);
			SETKEY(0, MOVEMENT, ALT);
			SETKEY(0, LOOKING, S);
			SETKEY(0, FORWARDS, UP);
			SETKEY(0, BACKWARDS, DOWN);
			SETKEY(0, STRAFELEFT, COMMA);
			SETKEY(0, STRAFERIGHT, PERIOD);
			SETKEY(0, JUMP, FORWARDSLASH);
			SETKEY(0, LAND, HOME);
			SETKEY(0, TURNSEMICIRCLE, BACKSLASH);
			SETKEY(0, TURNLEFT, LEFT);
			SETKEY(0, TURNRIGHT, RIGHT);
			SETKEY(0, LOOKUP, PAGEUP);
			SETKEY(0, LOOKDOWN, PAGEDOWN);
			SETKEY(0, LOOKCENTER, END);
			SETKEY(0, USE, SPACE);
			SETKEY(0, TAUNT, U);
			SETKEY(0, CHAT, T);
			SETKEY(0, TEAMCHAT, Y);
			SETKEY(0, ATTACK, CTRL);
			SETKEY(0, RELOAD, R);
			SETKEY(0, SUICIDE, K);
			SETKEY(0, SLOT1, 1);
			SETKEY(0, SLOT2, 2);
			SETKEY(0, SLOT3, 3);
			SETKEY(0, SLOT4, 4);
			SETKEY(0, SLOT5, 5);
			SETKEY(0, SLOT6, 6);
			SETKEY(0, SLOT7, 7);
			SETKEY(0, SLOT8, 8);
			SETKEY(0, SLOT9, 9);
			SETKEY(0, SLOT10, 0);
			SETKEY(0, PREVWEAPON, LEFTBRACKET);
			SETKEY(0, NEXTWEAPON, RIGHTBRACKET);
			SETKEY(0, PREVINVENTORY, SEMICOLON);
			SETKEY(0, NEXTINVENTORY, COLON);
			SETKEY(0, USEINVENTORY, RETURN);
			SETKEY(0, FLYUP, P);
			SETKEY(1, FLYUP, INSERT);
			SETKEY(0, FLYDOWN, L);
			SETKEY(1, FLYDOWN, KDELETE);
			SETKEY(0, TOPSCORES, F);
			SETKEY(0, COOPSPY, F12);
			SETKEY(0, AUTOMAP, TAB);
	
			// Joystick Buttons
			SETJOY(3, ATTACK, 1);
			SETJOY(3, USE, 2);
			SETJOY(3, MOVEMENT, 3);
			SETJOY(3, SPEED, 4);
		
			SETJOY(3, STRAFELEFT, 5);
			SETJOY(3, STRAFERIGHT, 6);
		
			SETJOY(3, PREVWEAPON, 7);
			SETJOY(3, NEXTWEAPON, 8);
		
			SETJOY(3, MORESTUFF, 9);
			SETJOY(3, POPUPMENU, 10);
		
			SETJOY(3, JUMP, 11);
		
			// More Joystick Buttons (with more key)
			SETJOYMORE(3, TOPSCORES, 1);
			SETJOYMORE(3, COOPSPY, 2);
			SETJOYMORE(3, AUTOMAP, 3);
			SETJOYMORE(3, USEINVENTORY, 4);
		
			SETJOYMORE(3, RELOAD, 5);
			SETJOYMORE(3, SWITCHFIREMODE, 6);
		
			SETJOYMORE(3, PREVINVENTORY, 7);
			SETJOYMORE(3, NEXTINVENTORY, 8);
		
			SETJOYMORE(3, CHATMODE, 10);
			SETJOYMORE(3, SUICIDE, 11);
		}
		
		// GCW Zero
		else if (g_ModelMode == DMM_GCW)
		{
			// Standard Keys
			SETKEY(0, FORWARDS, UP);
			SETKEY(0, BACKWARDS, DOWN);
			SETKEY(0, TURNLEFT, LEFT);
			SETKEY(0, TURNRIGHT, RIGHT);
			
			SETKEY(0, ATTACK, GCWZEROY);
			SETKEY(0, USE, GCWZEROA);
			SETKEY(0, MORESTUFF, GCWZEROB);
			SETKEY(0, SPEED, GCWZEROX);
		
			SETKEY(0, STRAFELEFT, GCWZEROL);
			SETKEY(0, STRAFERIGHT, GCWZEROR);
			
			SETKEY(0, USEINVENTORY, GCWZEROSTART);
			
			// More Keys (B is pressed)
			SETKEYMORE(1, FORWARDS, UP);
			SETKEYMORE(1, BACKWARDS, DOWN);
			SETKEYMORE(1, TURNLEFT, LEFT);
			SETKEYMORE(1, TURNRIGHT, RIGHT);
			
			SETKEYMORE(1, LAND, GCWZEROY);
			SETKEYMORE(1, JUMP, GCWZEROA);
			SETKEYMORE(1, MOREMORESTUFF, GCWZEROX);
		
			SETKEYMORE(1, PREVWEAPON, GCWZEROL);
			SETKEYMORE(1, NEXTWEAPON, GCWZEROR);
			
			SETKEYMORE(1, CHAT, GCWZEROSTART);
			
			// Extra Keys (B+X is pressed)
			SETKEYXTRA(2, LOOKDOWN, UP);
			SETKEYXTRA(2, LOOKUP, DOWN);
			SETKEYXTRA(2, TURNLEFT, LEFT);
			SETKEYXTRA(2, TURNRIGHT, RIGHT);
			
			SETKEYXTRA(2, AUTOMAP, GCWZEROY);
			SETKEYXTRA(2, COOPSPY, GCWZEROA);
		
			SETKEYXTRA(2, PREVINVENTORY, GCWZEROL);
			SETKEYXTRA(2, NEXTINVENTORY, GCWZEROR);
			
			SETKEYXTRA(2, SUICIDE, GCWZEROSTART);
		}
		
#undef SETJOY
#undef SETKEY_M
#undef SETKEY
#undef SETKEYMORE
#undef SETMOUSE
#undef SETDBLMOUSE
#undef SETJOY
#undef SETJOYMORE
		
		// Mouse Axis
			// Not ALT
		l_DefaultMouseAxis[0][0] = DPEXCMA_LOOKX;
		l_DefaultMouseAxis[0][1] = DPEXCMA_MOVEY;
			// ALT
		l_DefaultMouseAxis[1][0] = DPEXCMA_MOVEX;
		l_DefaultMouseAxis[1][1] = DPEXCMA_MOVEY;
			// Mouse Look (Default 'S')
		l_DefaultMouseAxis[2][0] = DPEXCMA_LOOKX;
		l_DefaultMouseAxis[2][1] = DPEXCMA_LOOKY;
	
		// Joystick Axis
			// Not ALT
		l_DefaultJoyAxis[0][0] = DPEXCMA_LOOKX;
		l_DefaultJoyAxis[0][1] = DPEXCMA_MOVEY;
			// ALT
		l_DefaultJoyAxis[1][0] = DPEXCMA_MOVEX;
		l_DefaultJoyAxis[1][1] = DPEXCMA_MOVEY;
			// Mouse Look (Default 'S')
		l_DefaultJoyAxis[2][0] = DPEXCMA_LOOKX;
		l_DefaultJoyAxis[2][1] = DPEXCMA_NEGLOOKY;
	
		// Now set
		l_DefaultCtrlsMapped = true;
	}
	
	// Copy directly from defaults
	memmove(New->Ctrls, l_DefaultCtrls, sizeof(l_DefaultCtrls));
	memmove(New->MouseAxis, l_DefaultMouseAxis, sizeof(l_DefaultMouseAxis));
	memmove(New->JoyAxis, l_DefaultJoyAxis, sizeof(l_DefaultJoyAxis));

	// Default Sensitivities
	New->MouseSens[0] = New->MouseSens[1] = 10;
	New->JoySens[0] = New->JoySens[1] = 100;
	New->LookUpDownSpeed = (1 << 25);
	
	// Default Colors
	New->ColorPickup = VEX_MAP_WHITE;
	New->ColorSecret = VEX_MAP_BRIGHTWHITE;
	New->ColorLock[0] = VEX_MAP_RED;
	New->ColorLock[1] = VEX_MAP_YELLOW;
	New->ColorLock[2] = VEX_MAP_BLUE;
	New->ColorLock[3] = VEX_MAP_GRAY;
	
	// Default Sounds
	strncpy(New->SoundSecret, "secret", MAXPLAYERNAME - 1);
	
	// Default other options
	New->DrawPSprites = true;
	New->BobMode = 1;							// Middle bobbing mode
	New->ViewHeight = VIEWHEIGHT << FRACBITS;	// Player View Height
	New->ChaseCam = false;						// Enable chase cam
	New->CamDist = 128 << FRACBITS;				// Camera Distance (default)
	New->CamHeight = 20 << FRACBITS;			// Camera Height
	New->CamSpeed = 16384;						// Camera Speed
	New->TransSBar = false;						// Transparent status bar
	New->ScaledSBar = false;					// Scaled status bar
	New->BarType = DPBT_REMOOD;
	
	// Autorun on GCW
	if (g_ModelMode == DMM_GCW)
		New->AutoRun = true;					// Hurts thumb to hold run all the time
	
	/* Link */
	if (!l_FirstProfile)
		l_FirstProfile = New;
	else
	{
		New->Next = l_FirstProfile;
		l_FirstProfile->Prev = New;
		l_FirstProfile = New;
	}
	
	/* Key defaults unset? */
	if (!g_KeyDefaultProfile)
		g_KeyDefaultProfile = New;
	
	/* Return the new one */
	return New;
}

/* D_FindProfileEx() -- Locates a profile */
D_Prof_t* D_FindProfileEx(const char* const a_Name)
{
	D_Prof_t* Rover;
	
	D_Prof_t* Best;
	int32_t BestN;
	char* a, *b;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Rove */
	// First by UUID Match
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
		if (strcmp(Rover->UUID, a_Name) == 0)
			return Rover;
	
	// Second by account name match
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
		if (strcasecmp(Rover->AccountName, a_Name) == 0)
			return Rover;
	
	// Third my partial UUID
	Best = NULL;
	BestN = 0;
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
	{
		// Prepare for comparison
		a = a_Name;
		b = Rover->UUID;
	}
	
	/* Not found */
	return NULL;
}

/* D_FindProfileExByInstance() -- Find profile instance ID */
D_Prof_t* D_FindProfileExByInstance(const uint32_t a_ID)
{
	D_Prof_t* Rover;
	
	/* Rove */
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
		if (Rover->InstanceID == a_ID)
			return Rover;
	
	/* Not found */
	return NULL;
}

/* DS_KeyCodeToStr() -- Converts a key code to a string */
static void DS_KeyCodeToStr(char* const a_Dest, const size_t a_Size, const uint32_t a_Code)
{
	/* Check */
	if (!a_Dest || !a_Size)
		return;
	
	/* Nothing */
	if (!a_Code)
		snprintf(a_Dest, a_Size, "---");
	
	/* Joystick */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_JOY)
		snprintf(a_Dest, a_Size, "joyb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_MOUSE)
		snprintf(a_Dest, a_Size, "mouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* Double Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_DMOUSE)
		snprintf(a_Dest, a_Size, "dblmouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
		
	/* More Joystick */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_JOYP)
		snprintf(a_Dest, a_Size, "morejoyb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* More Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_MOUSEP)
		snprintf(a_Dest, a_Size, "moremouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* More Double Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_DMOUSEP)
		snprintf(a_Dest, a_Size, "moredblmouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* More Keyboard */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_KEYP)
		snprintf(a_Dest, a_Size, "morekey%s", c_KeyNames[a_Code][0]);
	
	/* Extra Joystick */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_JOYX)
		snprintf(a_Dest, a_Size, "xtrajoyb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* Extra Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_MOUSEX)
		snprintf(a_Dest, a_Size, "xtramouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* Extra Double Mouse */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_DMOUSEX)
		snprintf(a_Dest, a_Size, "xtradblmouseb%02i", (int)((a_Code & PRFKBIT_VMASK) + 1));
	
	/* Extra Keyboard */
	else if ((a_Code & PRFKBIT_MASK) == PRFKBIT_KEYX)
		snprintf(a_Dest, a_Size, "xtrakey%s", c_KeyNames[a_Code][0]);
		
	/* Keyboard */
	else if (a_Code >= 0 && a_Code < NUMIKEYBOARDKEYS)
		snprintf(a_Dest, a_Size, "%s", c_KeyNames[a_Code][0]);
	
	/* Illegal */
	else
		snprintf(a_Dest, a_Size, "---");
}

/* DS_KeyStrToCode() -- Converts string to key code */
static uint32_t DS_KeyStrToCode(const char* const a_Str)
{
	int i;
	const char* p;
	uint32_t ExtraMask;
	
	/* Check */
	if (!a_Str)
		return 0;
	
	/* Illegal/NULL Key */
	if (strcasecmp(a_Str, "---") == 0)
		return 0;
	
	/* Joystick Buttons */
	else if (strncasecmp(a_Str, "joyb", 4) == 0)
		return PRFKBIT_JOY | (C_strtou32(a_Str + 4, NULL, 10) - 1);
	
	/* Mouse Buttons */
	else if (strncasecmp(a_Str, "mouseb", 6) == 0)
		return PRFKBIT_MOUSE | (C_strtou32(a_Str + 6, NULL, 10) - 1);
	
	/* Double Mouse Buttons */
	else if (strncasecmp(a_Str, "dblmouseb", 9) == 0)
		return PRFKBIT_DMOUSE | (C_strtou32(a_Str + 9, NULL, 10) - 1);
		
	/* More Joystick Buttons */
	else if (strncasecmp(a_Str, "morejoyb", 8) == 0)
		return PRFKBIT_JOYP | (C_strtou32(a_Str + 4, NULL, 10) - 1);
	
	/* More Mouse Buttons */
	else if (strncasecmp(a_Str, "moremouseb", 10) == 0)
		return PRFKBIT_MOUSEP | (C_strtou32(a_Str + 6, NULL, 10) - 1);
	
	/* More Double Mouse Buttons */
	else if (strncasecmp(a_Str, "moredblmouseb", 13) == 0)
		return PRFKBIT_DMOUSEP | (C_strtou32(a_Str + 9, NULL, 10) - 1);
	
	/* Extra Joystick Buttons */
	else if (strncasecmp(a_Str, "xtrajoyb", 8) == 0)
		return PRFKBIT_JOYX | (C_strtou32(a_Str + 4, NULL, 10) - 1);
	
	/* Extra Mouse Buttons */
	else if (strncasecmp(a_Str, "xtramouseb", 10) == 0)
		return PRFKBIT_MOUSEX | (C_strtou32(a_Str + 6, NULL, 10) - 1);
	
	/* Extra Double Mouse Buttons */
	else if (strncasecmp(a_Str, "xtradblmouseb", 13) == 0)
		return PRFKBIT_DMOUSEX| (C_strtou32(a_Str + 9, NULL, 10) - 1);
	
	/* Keyboard Keys */
	else
	{
		// Get base
		p = a_Str;
		ExtraMask = PRFKBIT_KEY;
		
		// If string starts with morekey, is a more modifier code
		if (!strncasecmp(p, "morekey", 7))
		{
			p += 7;
			ExtraMask = PRFKBIT_KEYP;
		}
		
		// Or if it starts with xtrakey, it is a more more modifier
		else if (!strncasecmp(p, "xtrakey", 7))
		{
			p += 7;
			ExtraMask = PRFKBIT_KEYX;
		}
		
		// Find key name
		for (i = 0; i < NUMIKEYBOARDKEYS; i++)
			if (strcasecmp(p, c_KeyNames[i][0]) == 0)
				return ExtraMask | i;
	}
}

/* DS_ReloadValue() -- Reloads value into profile */
static void DS_ReloadValue(D_Prof_t* const a_Profile, const char* const a_Option, const char* const a_Value)
{
	int i;
	void* Ptr;
	
	/* Check */
	if (!a_Profile || !a_Option || !a_Value)
		return;
	
	/* Find Named Option */
	for (i = 0; c_ProfDataStat[i].ArgName[0]; i++)
		if (strcasecmp(a_Option, c_ProfDataStat[i].ArgName) == 0)
			break;
	
	// Not found?
	if (!c_ProfDataStat[i].ArgName[0])
		return;
	
	/* Get offset */
	Ptr = (void*)((uintptr_t)a_Profile + c_ProfDataStat[i].Offset);
	
	/* Based on type */
	switch (c_ProfDataStat[i].Type)
	{
			// uint8_t
		case PDST_UINT8:
			*((uint8_t*)Ptr) = C_strtou32(a_Value, NULL, 10);
			break;
			
			// uint32_t
		case PDST_UINT32:
			*((uint32_t*)Ptr) = C_strtou32(a_Value, NULL, 10);
			break;
			
			// int32_t
		case PDST_INT32:
			*((int32_t*)Ptr) = C_strtoi32(a_Value, NULL, 10);
			break;
			
			// fixed_t
		case PDST_FIXED:
			*((fixed_t*)Ptr) = FLOAT_TO_FIXED(atof(a_Value));
			break;
			
			// bool_t
		case PDST_BOOL:
			*((bool_t*)Ptr) = false;
			if (!strcasecmp("true", a_Value))
				*((bool_t*)Ptr) = true;
			break;
			
			// char*
		case PDST_STRING:
			CONL_UnEscapeString(Ptr, MAXPLAYERNAME, a_Value);
			break;
			
			// tic_t
		case PDST_TIC:
			*((tic_t*)Ptr) = C_strtou32(a_Value, NULL, 10);
			break;
			
			// Unknown?
		default:
			break;
	}
}

/* DS_SizeToStr() -- Converts sized argument to a string */
static void DS_SizeToStr(void* const a_Ptr, const D_PDST_t a_Type, char* const a_Buf, const size_t a_BufSize)
{
	switch (a_Type)
	{
			// uint8_t
		case PDST_UINT8:
			snprintf(a_Buf, a_BufSize - 1, "%hhu", (int)(*((uint8_t*)a_Ptr)));
			break;
			
			// uint32_t
		case PDST_UINT32:
			snprintf(a_Buf, a_BufSize - 1, "%u", (int)(*((uint32_t*)a_Ptr)));
			break;
			
			// int32_t
		case PDST_INT32:
			snprintf(a_Buf, a_BufSize - 1, "%i", (int)(*((int32_t*)a_Ptr)));
			break;
			
			// fixed_t
		case PDST_FIXED:
			snprintf(a_Buf, a_BufSize - 1, "%f", FIXED_TO_FLOAT(*((fixed_t*)a_Ptr)));
			break;
			
			// bool_t
		case PDST_BOOL:
			if (*((bool_t*)a_Ptr))
				strncpy(a_Buf, "true", a_BufSize - 1);
			else
				strncpy(a_Buf, "false", a_BufSize - 1);
			break;
			
			// char*
		case PDST_STRING:
			CONL_EscapeString(a_Buf, a_BufSize, ((char**)a_Ptr));
			break;
			
			// tic_t
		case PDST_TIC:
			snprintf(a_Buf, a_BufSize - 1, "%u", (int)(*((tic_t*)a_Ptr)));
			break;
			
			// Unknown?
		default:
			strncpy(a_Buf, "???", a_BufSize - 1);
			break;
	}
}

/* D_SaveProfileData() -- Saves profile data */
void D_SaveProfileData(void (*a_WriteBack)(const char* const a_Buf, void* const a_Data), void* const a_Data)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	char BufB[BUFSIZE];
	char EscapeUUID[BUFSIZE];
	D_Prof_t* Rover;
	int i, j, k;
	
	/* Check */
	if (!a_WriteBack)
		return;
	
	/* Start Header */
	a_WriteBack("\n// Begin Profiles (edit at your own risk)\n", a_Data);
	
	/* Go through every profile */
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
	{
		// Skip ones marked DO NOT SAVE
		if (Rover->Flags & DPEXF_DONTSAVE)
			continue;
		
		// Escape the Profile Name
		memset(EscapeUUID, 0, sizeof(EscapeUUID));
		CONL_EscapeString(EscapeUUID, BUFSIZE, Rover->AccountName);
		
		// Mark profile creation
		memset(BufB, 0, sizeof(BufB));
		CONL_EscapeString(BufB, BUFSIZE, Rover->UUID);
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE, "profile create \"%s\" \"%s\"\n", EscapeUUID, BufB);
		a_WriteBack(Buf, a_Data);
		
		// Write Profile Data
		for (i = 0; c_ProfDataStat[i].ArgName[0]; i++)
		{
			// Value
			memset(BufB, 0, sizeof(BufB));
			DS_SizeToStr((void*)((uintptr_t)Rover + c_ProfDataStat[i].Offset), c_ProfDataStat[i].Type, BufB, BUFSIZE);
			
			// Write
			snprintf(Buf, BUFSIZE, "profile value \"%s\" \"%s\" \"%s\"\n", EscapeUUID, c_ProfDataStat[i].ArgName, BufB);
			a_WriteBack(Buf, a_Data);
		}
		
		// Write Mouse/Joy Axis
		for (i = 0; i < MAXALTAXIS; i++)
		{
			// Mouse
			for (j = 0; j < MAXMOUSEAXIS; j++)
			{
				// If not the default, change
				if (!(Rover->Flags & DPEXF_DUMPALL) && Rover->MouseAxis[i][j] == l_DefaultMouseAxis[i][j])
					continue;
					
				// Write Axis
				snprintf(Buf, BUFSIZE, "profile maxis \"%s\" %i %i \"%s\"\n",
						EscapeUUID,
						i,
						j,
						c_AxisMap[Rover->MouseAxis[i][j]]
					);
				a_WriteBack(Buf, a_Data);
			}
			
			// Joystick
			for (j = 0; j < MAXJOYAXIS; j++)
			{
				// If not the default, change
				if (!(Rover->Flags & DPEXF_DUMPALL) && Rover->JoyAxis[i][j] == l_DefaultJoyAxis[i][j])
					continue;
					
				// Write Axis
				snprintf(Buf, BUFSIZE, "profile jaxis \"%s\" %i %i \"%s\"\n",
						EscapeUUID,
						i,
						j,
						c_AxisMap[Rover->JoyAxis[i][j]]
					);
				a_WriteBack(Buf, a_Data);
			}
		}
		
		// Write Controls
		for (i = 0; i < NUMDPROFILEEXINPUTCTRLS; i++)
			for (j = 0; j < 4; j++)
			{
				// If the key does not match the default then save it.
				// Otherwise don't save it (since this fills the config
				// file up to insane proportions.
				if (!(Rover->Flags & DPEXF_DUMPALL) && Rover->Ctrls[i][j] == l_DefaultCtrls[i][j])
					continue;
				
				// Convert Key to String
				DS_KeyCodeToStr(BufB, BUFSIZE, Rover->Ctrls[i][j]);
				
				// Write Key
				snprintf(Buf, BUFSIZE, "profile control \"%s\" \"%s\" %i \"%s\"\n",
						EscapeUUID,
						c_ControlMapper[i].ShortName,
						j,
						BufB
					);
				a_WriteBack(Buf, a_Data);
			}
		
		// Spacer
		a_WriteBack("\n", a_Data);
	}
	
	/* End Header */
	a_WriteBack("// End Profiles\n", a_Data);
#undef BUFSIZE
}

/* CLC_Profile() -- Profile command handler */
int CLC_Profile(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 256
	char BufA[BUFSIZE];
	char BufB[BUFSIZE];
	D_Prof_t* New;
	int i, j, k;
	D_ProfileExCtrlMA_t* TMA;
	
	/* Not enough arguments? */
	if (a_ArgC < 3)
		return 1;
		
	/* Clear Buffers */
	memset(BufA, 0, sizeof(BufA));
	memset(BufB, 0, sizeof(BufB));
	
	/* Which Sub Command */
	// Dump all to config
	if (!strcasecmp(a_ArgV[1], "dumpall"))
	{
		// Find profile
		New = D_FindProfileEx(a_ArgV[2]);
		
		// Not found?
		if (!New)
			return 1;
		
		// Set dump flag
		New->Flags |= DPEXF_DUMPALL;
		return 0;
	}
	
	// Create Profile
	else if (strcasecmp(a_ArgV[1], "create") == 0)
	{
		// Read Name
		CONL_UnEscapeString(BufA, BUFSIZE, a_ArgV[2]);
		
		// Possibly read UUID
		New = NULL;
		if (a_ArgC >= 4)
		{
			CONL_UnEscapeString(BufB, BUFSIZE, a_ArgV[3]);
			
			// Check UUID existence
			New = D_FindProfileEx(BufB);
		}
		
		// See if it already exists
		if (!New)
			New = D_FindProfileEx(BufA);
		
		// Exists?
		if (New)
		{
			CONL_OutputU(DSTR_DPROFC_ALREADYEXIST, "%s\n", BufA);
			return 1;
		}
		
		// Create Profile
		New = D_CreateProfileEx(BufA);
		
		// Failed??
		if (!New)
		{
			CONL_OutputU(DSTR_DPROFC_FAILEDCREATE, "\n");
			return 1;
		}
		
		// Set UUID if preformed
		if (BufB[0])
			strncpy(New->UUID, BufB, MAXUUIDLENGTH);
		
		// Done
		return 0;
	}
	
	/* After this all values are mostly the same */
	// Read Name
	CONL_UnEscapeString(BufA, BUFSIZE, a_ArgV[2]);
	
	// Find profile
	New = D_FindProfileEx(BufA);
	
	// Not found?
	if (!New)
	{
		CONL_OutputU(DSTR_DPROFC_NOTFOUND, "%s\n", BufA);
		return 1;
	}
	
	// Change Value
	if (strcasecmp(a_ArgV[1], "value") == 0)
	{
		// Usage?
		if (a_ArgC < 5)
		{
			CONL_OutputU(DSTR_DPROFC_VALUEUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		DS_ReloadValue(New, a_ArgV[3], a_ArgV[4]);
	}
	
	// Control
	else if (strcasecmp(a_ArgV[1], "control") == 0)
	{
		// Usage?
		if (a_ArgC < 6)
		{
			CONL_OutputU(DSTR_DPROFC_CONTROLUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		// Load Index
		i = C_strtou32(a_ArgV[4], NULL, 10);
		
		// Out of bounds?
		if (i < 0 || i >= 4)
		{
			CONL_OutputU(DSTR_DPROFC_INDEXOUTOFRANGE, "%i\n", i);
			return 1;
		}
		
		// Load Control Name
		for (k = 0; k < NUMDPROFILEEXINPUTCTRLS; k++)
			if (strcasecmp(a_ArgV[3], c_ControlMapper[k].ShortName) == 0)
				break;
		
		// Not found?
		if (k >= NUMDPROFILEEXINPUTCTRLS)
		{
			CONL_OutputU(DSTR_DPROFC_NOTCONTROLNAME, "%s\n", a_ArgV[3]);
			return 1;
		}
		
		// Back convert string to ID
		New->Ctrls[k][i] = DS_KeyStrToCode(a_ArgV[5]);
	}
	
	// Mouse/Joy Axis
	else if ((strcasecmp(a_ArgV[1], "maxis") == 0) || (strcasecmp(a_ArgV[1], "jaxis") == 0))
	{
		// Usage?
		if (a_ArgC < 6)
		{
			CONL_OutputU(DSTR_DPROFC_MAXISUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		// Obtain current grouping and control set
		i = C_strtou32(a_ArgV[3], NULL, 10);
		j = C_strtou32(a_ArgV[4], NULL, 10);
		
		// Alternate overflow?
		if (i < 0 || i >= MAXALTAXIS)
			return 1;
		
		// Clear
		TMA = NULL;
		
		// Modding Joy?
		if (a_ArgV[1][0] == 'j')
		{
			// Out of range?
			if (j < 0 || j >= MAXJOYAXIS)
				return 1;
			
			// Set
			TMA = &New->JoyAxis[i][j];
		}
		
		// Modding Mouse?
		else
		{
			// Out of range?
			if (j < 0 || j >= MAXMOUSEAXIS)
				return 1;
			
			// Set
			TMA = &New->MouseAxis[i][j];
		}
		
		// Alias for null
		if (strcasecmp(a_ArgV[5], "---") == 0)
		{
			*TMA = 0;
			return 0;
		}
		
		// Find in list
		for (k = 0; k < NUMDPROFILEEXCTRLMAS; k++)
			if (strcasecmp(a_ArgV[5], c_AxisMap[k]) == 0)
			{
				*TMA = k;
				return 0;
			}
		
		// Not found
		return 1;
	}
	
	return 0;
#undef BUFSIZE
}

#endif

