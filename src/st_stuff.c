// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "st_stuff.h"
#include "screen.h"
#include "d_player.h"
#include "g_state.h"
#include "d_netcmd.h"
#include "v_video.h"
#include "d_prof.h"
#include "info.h"
#include "p_demcmp.h"
#include "p_mobj.h"
#include "r_state.h"
#include "m_bbox.h"
#include "st_doom.h"
#include "sn.h"
#include "vhw_wrap.h"
#include "m_menu.h"
#include "p_info.h"
#include "p_local.h"


























//protos
void ST_createWidgets(void);

extern fixed_t waterheight;

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            (1 * VPALSMOOTHCOUNT)
#define STARTBONUSPALS          (9 * VPALSMOOTHCOUNT)
#define NUMREDPALS              (8 * VPALSMOOTHCOUNT)
#define NUMBONUSPALS            (4 * VPALSMOOTHCOUNT)
// Radiation suit, green shift.
#define RADIATIONPAL            (13 * VPALSMOOTHCOUNT)

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY              96

// For Responder
#define ST_TOGGLECHAT           KEY_ENTER

// Location of status bar
//added:08-01-98:status bar position changes according to resolution.
#define ST_FX                     143

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

#define ST_FACESX               143
#define ST_FACESY               (ST_Y+0)

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                44
#define ST_AMMOY                (ST_Y+3)

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              90
#define ST_HEALTHY              (ST_Y+3)

// Weapon pos.
#define ST_ARMSX                111
#define ST_ARMSY                (ST_Y+4)
#define ST_ARMSBGX              104
#define ST_ARMSBGY              (ST_Y)
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
#define ST_FRAGSX               138
#define ST_FRAGSY               (ST_Y+3)
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
#define ST_ARMORX               221
#define ST_ARMORY               (ST_Y+3)

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
#define ST_KEY0X                239
#define ST_KEY0Y                (ST_Y+3)
#define ST_KEY1WIDTH            ST_KEY0WIDTH
#define ST_KEY1X                239
#define ST_KEY1Y                (ST_Y+13)
#define ST_KEY2WIDTH            ST_KEY0WIDTH
#define ST_KEY2X                239
#define ST_KEY2Y                (ST_Y+23)

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
#define ST_AMMO0X               288
#define ST_AMMO0Y               (ST_Y+5)
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
#define ST_AMMO1X               288
#define ST_AMMO1Y               (ST_Y+11)
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
#define ST_AMMO2X               288
#define ST_AMMO2Y               (ST_Y+23)
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
#define ST_AMMO3X               288
#define ST_AMMO3Y               (ST_Y+17)

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
#define ST_MAXAMMO0X            314
#define ST_MAXAMMO0Y            (ST_Y+5)
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X            314
#define ST_MAXAMMO1Y            (ST_Y+11)
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X            314
#define ST_MAXAMMO2Y            (ST_Y+23)
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X            314
#define ST_MAXAMMO3Y            (ST_Y+17)

//faB: unused stuff from the Doom alpha version ?
// pistol
//#define ST_WEAPON0X           110
//#define ST_WEAPON0Y           (ST_Y+4)
// shotgun
//#define ST_WEAPON1X           122
//#define ST_WEAPON1Y           (ST_Y+4)
// chain gun
//#define ST_WEAPON2X           134
//#define ST_WEAPON2Y           (ST_Y+4)
// missile launcher
//#define ST_WEAPON3X           110
//#define ST_WEAPON3Y           (ST_Y+13)
// plasma gun
//#define ST_WEAPON4X           122
//#define ST_WEAPON4Y           (ST_Y+13)
// bfg
//#define ST_WEAPON5X           134
//#define ST_WEAPON5Y           (ST_Y+13)

// WPNS title
//#define ST_WPNSX              109
//#define ST_WPNSY              (ST_Y+23)

// DETH title
//#define ST_DETHX              109
//#define ST_DETHY              (ST_Y+23)

//Incoming messages window location
// #define ST_MSGTEXTX     (viewwindowx)
// #define ST_MSGTEXTY     (viewwindowy+viewheight-18)
//#define ST_MSGTEXTX             0
//#define ST_MSGTEXTY             0     //added:08-01-98:unused
// Dimensions given in characters.
#define ST_MSGWIDTH             52
// Or shall I say, in lines?
#define ST_MSGHEIGHT            1

#define ST_OUTTEXTX             0
#define ST_OUTTEXTY             6

// Width, in characters again.
#define ST_OUTWIDTH             52
// Height, in lines.
#define ST_OUTHEIGHT            1

#define ST_MAPWIDTH     \
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

//added:24-01-98:unused ?
//#define ST_MAPTITLEX  (vid.width - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY            0
#define ST_MAPHEIGHT            1

//added:02-02-98: set true if widgets coords need to be recalculated
bool_t st_recalc;

// main player in game
//Hurdler: no not static!
player_t* plyr;

// ST_Start() has just been called
bool_t st_firsttime;

// used to execute ST_Init() only once
static int veryfirsttime = 1;

// used for timing
static unsigned int st_clock;

int stbarheight = ST_HEIGHT;
int ST_Y = BASEVIDHEIGHT - ST_HEIGHT;
int st_x = 0;
float st_scalex, st_scaley;

// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
static int sbohealth;
static int sbofrags;
static int sboarmor;

void ST_TransSTChange(void)
{
	R_ExecuteSetViewSize();
}

void ST_ExternrefreshBackground(void)
{
}

static bool_t st_stopped = true;

void ST_Ticker(void)
{
}

static int st_palette = 0;

/* ST_doPaletteStuff() -- Changes the current palette */
// GhostlyDeath <July 30, 2011> -- Redone for smoothing
void ST_doPaletteStuff(void)
{
	int ChosePal = 0;
	int BaseDam = 0, BzFade;
	
	/* Berserker fade */
	BaseDam = plyr->damagecount;
	
	if (plyr->powers[pw_strength])
	{
		BzFade = 12 - (plyr->powers[pw_strength] >> 6);
		
		if (BzFade > BaseDam)
			BaseDam = BzFade;
	}
	
	/* Player is hurt? */
	if (BaseDam)
	{
		// Division is number of palettes
		ChosePal = FixedMul(NUMREDPALS << FRACBITS, FixedDiv((BaseDam) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
		ChosePal++;				// +7
		//ChosePal = (plyr->damagecount * (10000 / NUMREDPALS) ) / 100;
		
		// Don't exceed
		if (ChosePal >= NUMREDPALS)
			ChosePal = NUMREDPALS - 1;
			
		// Offset
		ChosePal += STARTREDPALS;
	}
	
	/* Player got an item */
	else if (plyr->bonuscount)
	{
		// Division is number of palettes
		ChosePal = FixedMul(NUMBONUSPALS << FRACBITS, FixedDiv((plyr->bonuscount) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
		ChosePal++;				// +7
		
		// Don't exceed
		if (ChosePal >= NUMBONUSPALS)
			ChosePal = NUMBONUSPALS - 1;
			
		// Offset
		ChosePal += STARTBONUSPALS;
	}
	
	/* Player has radiation suit */
	else if (plyr->powers[pw_ironfeet] > 4 * 32 || plyr->powers[pw_ironfeet] & 8)
	{
		ChosePal = RADIATIONPAL;
	}
	
	/* Set palette */
	if ((g_SplitScreen <= 0))
		V_SetPalette(ChosePal);
}

static void ST_diffDraw(void)
{
}

void ST_Invalidate(void)
{
	st_firsttime = true;
}

void ST_overlayDrawer(void);

void ST_Drawer(bool_t refresh)
{
}

static void ST_loadGraphics(void)
{
}

// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics(char* facestr)
{
}

static void ST_loadData(void)
{
}

void ST_unloadGraphics(void)
{
}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics(void)
{
}

void ST_unloadData(void)
{
}

void ST_initData(void)
{
}

static void ST_Stop(void)
{
}

void ST_Start(void)
{
}

//
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

//faB: used by Glide mode, holds lumpnum of flat used to fill space around the viewwindow
int st_borderpatchnum;

void ST_Init(void)
{
}

//added:16-01-98: change the status bar too, when pressing F12 while viewing
//                 a demo.
void ST_changeDemoView(void)
{
}

/*****************************************************************************/

/*** CONSTANTS ***/

// c_DefMapColors -- Default automap colors
static const uint32_t c_DefMapColors[NUMPROFAUTOMAPCOLORS][3] =
{
	{0, 0, 0},									// DPAMC_BACKGROUND
	{0, 0, 0},									// DPAMC_YOURPLAYER
	{63, 255, 63},								// DPAMC_THING
	{63, 255, 255},								// DPAMC_ALLYTHING
	{255, 63, 63},								// DPAMC_ENEMYTHING
	{255, 255, 0},								// DPAMC_PICKUP
	{255, 0, 0},								// DPAMC_SOLIDWALL
	{168, 168, 0},								// DPAMC_FLOORSTEP
	{255, 127, 0},								// DPAMC_CEILSTEP
	{255, 255, 0},								// DPAMC_TRIGGER
	{96, 96, 127},								// DPAMC_UNMAPPED
	{63, 63, 63},								// DPAMC_GRID
	{127, 127, 127},							// DPAMC_INVISODWALL
	{255, 255, 255},							// DPAMC_DEFAULT
};

/*** GLOBALS ***/

extern fixed_t g_GlobalBoundBox[4];				// Global bounding box

/*** STRUCTURES ***/

/* ST_MapDrawInfo_t -- Automap drawing info */
typedef struct ST_MapDrawInfo_s
{
	int32_t Scr;								// Screen being drawn
	fixed_t BaseCo[2];							// Base coordinate offsets
	D_SplitInfo_t* Split;						// Player Split (view window)
	D_Prof_t* Profile;						// Profile of player
	int32_t Rect[4];							// Screen rectangle
	int32_t Size[2];							// Size of screen
	uint32_t (*Color)[NUMPROFAUTOMAPCOLORS][3];	// Automap colors
	player_t* POV;								// Point of view
	mobj_t* POVMo;								// Object of POV
	fixed_t CenterAt[2];						// Center coordinates at
	fixed_t Scale;								// Scale
	bool_t DoRot;								// Do rotation
	angle_t RotAngle;							// Rotation angle
} ST_MapDrawInfo_t;

/*** PRIVATE FUNCTIONS ***/

/* STS_SBX() -- Status Bar X */
static int32_t STS_SBX(D_Prof_t* const a_Profile, const int32_t a_Coord, int32_t a_W, const int32_t a_H)
{
	int c = a_Coord;
	
	// a_W    1
	// --- * --- = ???
	//  1    320
	return FixedMul(c << FRACBITS, FixedMul(204, a_W << FRACBITS)) >> FRACBITS;
}

/* STS_SBY() -- Status Bar Y */
static int32_t STS_SBY(D_Prof_t* const a_Profile, const int32_t a_Coord, int32_t a_W, const int32_t a_H)
{
	int c = a_Coord;
	
	// a_H    1
	// --- * --- = ???
	//  1    200
	return FixedMul(c << FRACBITS, FixedMul(327, a_H << FRACBITS)) >> FRACBITS;
}

/* STS_Rotate() -- Rotates something? */
static void STS_Rotate(const fixed_t a_X, const fixed_t a_Y, const angle_t a_Angle, fixed_t* const a_OX, fixed_t* const a_OY)
{
	angle_t AnS;

	/* Get LUT value */
	AnS = a_Angle >> ANGLETOFINESHIFT;
	
	/* Perform matrix rotation */
	*a_OX = FixedMul(a_X, finecosine[AnS]) - FixedMul(a_Y, finesine[AnS]);
	*a_OY = FixedMul(a_X, finesine[AnS]) + FixedMul(a_Y, finecosine[AnS]);
}

/* STS_MapToScreen() -- Translate map coordinates to screen */
static void STS_MapToScreen(ST_MapDrawInfo_t* const a_Info, const fixed_t a_X, const fixed_t a_Y, fixed_t* const a_OX, fixed_t* const a_OY)
{
	fixed_t CenterX, CenterY;
	
	/* Calculate center of screen */
	CenterX = (a_Info->Size[0] >> 1) << FRACBITS;
	CenterY = (a_Info->Size[1] >> 1) << FRACBITS;
	
	/* Project */
	*a_OX = FixedMul(a_X - a_Info->CenterAt[0], a_Info->Scale);
	*a_OY = FixedMul(a_Info->CenterAt[1] - a_Y, a_Info->Scale);
	
	/* Rotate? */
	if (a_Info->DoRot)
		STS_Rotate(*a_OX, *a_OY, a_Info->RotAngle - ANG90, a_OX, a_OY);
	
	/* Modify to map to center of screen */
	*a_OX += CenterX;
	*a_OY += CenterY;
}

/* STS_DrawMapLine() -- Draws map line */
static void STS_DrawMapLine(ST_MapDrawInfo_t* const a_Info, const fixed_t a_Xa, const fixed_t a_Ya, const fixed_t a_Xb, const fixed_t a_Yb, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B)
{
	fixed_t p[2][2], m, xx, yy, c, b;
	register int i, j;
	int xDiff, yDiff;
	uint32_t Mask;
		
	/* Project Coordinates */
	// Map to screen
	STS_MapToScreen(a_Info, a_Xa, a_Ya, &p[0][0], &p[0][1]);
	STS_MapToScreen(a_Info, a_Xb, a_Yb, &p[1][0], &p[1][1]);
	
	/* Place onto screen */
	// Convert to int
	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			p[i][j] = p[i][j] >> FRACBITS;
	
	// Translate
	for (i = 0; i < 2; i++)
	{
		p[i][0] += a_Info->Rect[0];
		p[i][1] += a_Info->Rect[1];
	}
	
	// If any coordinate is out of bounds on the screen it needs to be snipped
	Mask = 0;
	if (p[0][0] < a_Info->Rect[0])	Mask |= 0x01;
	if (p[0][0] >= a_Info->Rect[2]) Mask |= 0x02;
	if (p[1][0] < a_Info->Rect[0])	Mask |= 0x04;
	if (p[1][0] >= a_Info->Rect[2]) Mask |= 0x08;
	if (p[0][1] < a_Info->Rect[1])	Mask |= 0x10;
	if (p[0][1] >= a_Info->Rect[3]) Mask |= 0x20;
	if (p[1][1] < a_Info->Rect[1])	Mask |= 0x40;
	if (p[1][1] >= a_Info->Rect[3]) Mask |= 0x80;
	
	// Something was off screen
	if (Mask)
	{
		// Both ends off bottom
		if ((Mask & 0xA0) == 0xA0)
			return;
		
		// Both ends off top
		if ((Mask & 0x50) == 0x50)
			return;
		
		// Both ends off right
		if ((Mask & 0x0A) == 0x0A)
			return;
		
		// Both ends off left
		if ((Mask & 0x05) == 0x05)
			return;
		
		// Get X difference
		xDiff = p[1][0] - p[0][0];
		
		// Vertical line
		if (!xDiff)
		{
			if (Mask & 0x10)
				p[0][1] = a_Info->Rect[1];
			if (Mask & 0x20)
				p[0][1] = a_Info->Rect[3] - 1;
			if (Mask & 0x40)
				p[1][1] = a_Info->Rect[1];
			if (Mask & 0x80)
				p[1][1] = a_Info->Rect[3] - 1;
		}
		
		// Non-Vertical Line
		else
		{
			// Get y difference
			yDiff = (p[1][1] - p[0][1]);
			
			// Get slope
			m = FixedDiv(yDiff << FRACBITS, xDiff << FRACBITS);
			
			// Horizontal line
			if (m == 0)
			{
				if (Mask & 0x01)
					p[0][0] = a_Info->Rect[0];
				if (Mask & 0x02)
					p[0][0] = a_Info->Rect[2] - 1;
				if (Mask & 0x04)
					p[1][0] = a_Info->Rect[0];
				if (Mask & 0x08)
					p[1][0] = a_Info->Rect[2] - 1;
			}
			
			// Diagonal line
			else
			{
				// Calculate for b = y - mx
				b = (p[0][1] << FRACBITS) - FixedMul(m, (p[0][0] << FRACBITS));
				
				// Calculate for y = mx + b
				// Calculate for x = (y - b) / m;
				
			}
		}
	}
	
	// Scale to screen duplication count
	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			p[i][j] = FixedMul(p[i][j] << FRACBITS, (j ? vid.fxdupy : vid.fxdupx)) >> FRACBITS;
	
	// Draw it
	VHW_HUDDrawLine(VHWRGB(a_R,a_G,a_B), p[0][0], p[0][1], p[1][0], p[1][1]);
}

/* STS_DrawMapThing() -- Draws map thing */
static void STS_DrawMapThing(ST_MapDrawInfo_t* const a_Info, mobj_t* a_Mo, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Radius, const angle_t a_Angle)
{
	int8_t Shape;
	fixed_t x, y, c, d, l, m;
	uint32_t (*rgb)[3];
	int32_t PNum;
	
	/* Base Colors */
	rgb = (*a_Info->Color)[DPAMC_THING];
	PNum = a_Info->POV - players;
	
	/* If object is shootable or a missile, make an arrow */
	Shape = 0;		// Square by default
	if ((a_Mo->flags & MF_SHOOTABLE) || (a_Mo->flags & MF_MISSILE))
		Shape = 1;	// Arrow
	else if (a_Mo->flags & MF_PICKUP)
	{
		// Based on pickup type, change the shape
	}
	
	/* Recolorize */
	// Pickupable
	if (a_Mo->flags & MF_SPECIAL)
		rgb = (*a_Info->Color)[DPAMC_PICKUP];
	
	// Non-Pickupable
	else
	{
		// Can be shot, so teamify
		if (a_Mo->flags & MF_SHOOTABLE)
		{
			// This is not from a spectator (specs don't really care for allies, etc.)
			if (PNum >= 0 && PNum < MAXPLAYERS)
			{
				if (P_MobjOnSameTeam(a_Info->POVMo, a_Mo))
					rgb = (*a_Info->Color)[DPAMC_ALLYTHING];
				else
					rgb = (*a_Info->Color)[DPAMC_ENEMYTHING];
			}
		}
	}
	
	/* How is the object represented? */
	// An arrow
	if (Shape == 1)
	{
		STS_Rotate(-(a_Radius >> 1), -a_Radius, a_Angle - ANG90, &l, &m);
		l += a_X;
		m += a_Y;
		STS_Rotate((a_Radius >> 1), -a_Radius, a_Angle - ANG90, &c, &d);
		c += a_X;
		d += a_Y;
		STS_Rotate(0, a_Radius, a_Angle - ANG90, &x, &y);
		x += a_X;
		y += a_Y;
		
		// Back line
		STS_DrawMapLine(a_Info, l, m, c, d, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
		
		// Right Side
		STS_DrawMapLine(a_Info, c, d, x, y, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
		
		// Left Side
		STS_DrawMapLine(a_Info, x, y, l, m, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
	}
	
	// A nice box
	else
	{
		STS_DrawMapLine(a_Info, a_X - a_Radius, a_Y - a_Radius, a_X + a_Radius, a_Y - a_Radius, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
		STS_DrawMapLine(a_Info, a_X + a_Radius, a_Y - a_Radius, a_X + a_Radius, a_Y + a_Radius, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
		STS_DrawMapLine(a_Info, a_X + a_Radius, a_Y + a_Radius, a_X - a_Radius, a_Y + a_Radius, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
		STS_DrawMapLine(a_Info, a_X - a_Radius, a_Y + a_Radius, a_X - a_Radius, a_Y - a_Radius, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
	}
}

/* STS_DrawPlayerMap() -- Draws player automap */
static void STS_DrawPlayerMap(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP)
{
	ST_MapDrawInfo_t Info;
	int32_t i;
	line_t* Line;
	uint32_t (*rgb)[3];
	sector_t* Front, *Back;
	thinker_t* Thinker;
	mobj_t* Mo;
	subsector_t* SubS;
	fixed_t XSize, YSize;
	
	/* Fill Info */
	memset(&Info, 0, sizeof(Info));
	
	Info.Scr = a_PID;
	Info.BaseCo[0] = g_GlobalBoundBox[BOXLEFT];
	Info.BaseCo[1] = g_GlobalBoundBox[BOXBOTTOM];
	Info.Split = &g_Splits[a_PID];
	Info.Profile = Info.Split->Profile;
	Info.Rect[0] = a_X;
	Info.Rect[1] = a_Y;
	Info.Rect[2] = a_X + a_W;
	Info.Rect[3] = a_Y + a_H;
	Info.Size[0] = a_W;
	Info.Size[1] = a_H;
	Info.Color = c_DefMapColors;
	Info.POV = P_SpecGetPOV(Info.Scr);
	Info.POVMo = Info.POV->mo;
	Info.DoRot = true;
	Info.RotAngle = (Info.POVMo ? Info.POVMo->angle : 0);
	
	/* Initialize Map Zoom? */
	if (!Info.Split->MapZoom)
	{
		// Size of entire map
		XSize = g_GlobalBoundBox[BOXRIGHT] - g_GlobalBoundBox[BOXLEFT];
		YSize = g_GlobalBoundBox[BOXTOP] - g_GlobalBoundBox[BOXBOTTOM];
		
		// Use larger size
		if (YSize > XSize)
			XSize = YSize;
		
		// Default so that 1/4th of the map is visible at once
		Info.Split->MapZoom = 1 << (FRACBITS - 2);
	}
	
	// Set scale
	Info.Scale = Info.Split->MapZoom;
	
	// Free movement mode
	if (Info.Split->MapFreeMode)
	{
		Info.CenterAt[0] = Info.Split->MapPos[0];
		Info.CenterAt[1] = Info.Split->MapPos[1];
	}
	
	// Follow POV
	else
	{
		if (Info.POV->mo)
		{
			Info.CenterAt[0] = Info.Split->MapPos[0] = Info.POV->mo->x;
			Info.CenterAt[1] = Info.Split->MapPos[1] = Info.POV->mo->y;
		}
	}
	
	/* Draw the map */
	for (i = 0; i < numlines; i++)
	{
		Line = &lines[i];
		
		// Base color
		rgb = (*Info.Color)[DPAMC_DEFAULT];
		
		// Not visible?
		
		// Change color or don't draw?
			// Trigger line
		if (Line->special)
		{
			rgb = (*Info.Color)[DPAMC_TRIGGER];
		}
		
			// Inert Wall
		else
		{
			// Get sector sides
			Front = Line->frontsector;
			Back = Line->backsector;
			
				// Double sided and not blocking
			if (Front && Back && !(Line->flags & ML_BLOCKING))
			{
				
				// Floor difference?
				if (Front->floorheight != Back->floorheight)
					rgb = (*Info.Color)[DPAMC_FLOORSTEP];
				
				// Ceiling diff?
				else if (Front->ceilingheight != Back->ceilingheight)
					rgb = (*Info.Color)[DPAMC_CEILSTEP];
				
				// Normal
				else
					rgb = (*Info.Color)[DPAMC_INVISODWALL];
			}
		
			else
				rgb = (*Info.Color)[DPAMC_SOLIDWALL];
		}
		
		// Draw it
		STS_DrawMapLine(&Info, Line->v1->x, Line->v1->y, Line->v2->x, Line->v2->y, (*rgb)[0], (*rgb)[1], (*rgb)[2]);
	}
	
	/* Bot Debug Stuff */
	//if (g_BotDebug)
	//	B_DrawBotLines(&Info, STS_DrawMapLine);
	
	/* Draw things on it */
	for (i = 0; i < numsectors; i++)
	{
		Front = &sectors[i];
		
		// Get object list and draw all objects
		for (Mo = Front->thinglist; Mo; Mo = Mo->snext)
			STS_DrawMapThing(&Info, Mo, Mo->x, Mo->y, Mo->radius, Mo->angle);
	}
	
	/* Draw players */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Skip non playing or missing object
		if (!playeringame[i] || !players[i].mo)
			continue;
	}
	
	/* Current Level Name */
	V_DrawStringA(VFONT_SMALL, 0, P_LevelNameEx(), a_X + STS_SBX(Info.Profile, 20, a_W, a_H), a_Y + (a_H - V_FontHeight(VFONT_SMALL)));
}

//extern bool_t g_NetBoardDown;

/* STS_DrawPlayerBarEx() -- Draws a player's status bar, and a few other things */
static void STS_DrawPlayerBarEx(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];
	player_t* ConsoleP, *DisplayP;
	D_Prof_t* Profile;
	V_Image_t* vi;
	VideoFont_t Font;
	PI_wepid_t ReadyWeapon;
	PI_ammoid_t AmmoType;
	bool_t BigLetters, IsMonster;
	bool_t IsFake, OK;
	uint32_t i, j, k;
	PI_key_t* DrawKey;
	int32_t Right;
	
	/* Init */
	// Font to use
	if (a_W < 320)
	{
		BigLetters = false;
		Font = VFONT_SMALL;
	}
	else
	{
		BigLetters = true;
		
		if (g_CoreGame == CG_HERETIC)
			Font = VFONT_LARGE_HERETIC;
		else
			Font = VFONT_STATUSBARLARGE;
	}
	
	/* Get players to draw for */
	ConsoleP = a_ConsoleP;
	DisplayP = a_DisplayP;
	
	// Fake player?
	IsFake = false;
	if (DisplayP == P_SpecGet(a_PID))
		IsFake = true;
	
	// Net player
	//XPlay = ConsoleP->XPlayer;
	
	/* Get profile of player */
	Profile = ConsoleP->ProfileEx;
	
	/* Obtain some info */
	if (!IsFake)
	{
		ReadyWeapon = DisplayP->readyweapon;
		AmmoType = NUMAMMO;
		if (DisplayP->weaponinfo)
			if (ReadyWeapon >= 0 && ReadyWeapon < NUMWEAPONS)
				AmmoType = DisplayP->weaponinfo[DisplayP->readyweapon]->ammo;
	}
	
	/* Monster? */
	IsMonster = false;
	if (!IsFake)
		if (DisplayP->mo && ((DisplayP->mo->flags & MF_COUNTKILL) || !P_MobjIsPlayer(DisplayP->mo)))
			IsMonster = true;
	
	/* Draw the automap */
	if (g_Splits[a_PID].AutomapActive)
	{
		// Draw black box?
		if (!g_Splits[a_PID].OverlayMap)
			VHW_HUDDrawBox(0, 0, 0, 0, a_X, a_Y, a_X + a_W, a_Y + a_H);
		
		// Used subroutine
		STS_DrawPlayerMap(a_PID, a_X, a_Y, a_W, a_H, a_ConsoleP, a_DisplayP);
	}
	
	/* Which status bar type to draw? */
	// Overlay
	if (true && !IsFake)
	{
		//// HEALTH
		// Draw Health Icon
		vi = V_ImageFindA((IsMonster ? "sbohealg" : "sbohealt"), VCP_DOOM);
		if (vi)
			V_ImageDraw(
					0, vi,
					a_X + STS_SBX(Profile, 8, a_W, a_H),
					a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
					NULL
				);
		
		// Draw Health Text
		if (IsMonster)
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->mo->health);
		else
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->health);
		V_DrawStringA(
				Font, 0, Buf,
				a_X + STS_SBX(Profile, 8, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
				a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
			);
		
		//// ARMOR
		if (!IsMonster)
		{
			// Draw Armor Icon
			if (!DisplayP->armortype)
				vi = V_ImageFindA("sboempty", VCP_DOOM);
			else if (DisplayP->armortype == 1)
				vi = V_ImageFindA("sboarmwk", VCP_DOOM);
			else
				vi = V_ImageFindA("sboarmor", VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 96, a_W, a_H),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
						NULL
					);
		
			// Draw Armor Text
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->armorpoints);
			V_DrawStringA(
					Font, 0, Buf,
					a_X + STS_SBX(Profile, 96, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
					a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
				);
		}
		
		//// WEAPON/AMMO
		if (!IsMonster && (ReadyWeapon >= 0 && ReadyWeapon < NUMWEAPONS))
		{
			// Draw Icon
			vi = V_ImageFindA((DisplayP->weaponinfo && DisplayP->weaponinfo[ReadyWeapon]->SBOGraphic ? DisplayP->weaponinfo[ReadyWeapon]->SBOGraphic : "sboempty"), VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 240, a_W, a_H),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
						NULL
					);
		
			// Draw Ammo Text
			if (DisplayP->ammo)
			{
				if (AmmoType < 0 || AmmoType >= NUMAMMO || P_XGSVal(PGS_PLINFINITEAMMO))
					snprintf(Buf, BUFSIZE - 1, "-");
				else
					snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->ammo[AmmoType]);
				V_DrawStringA(
						Font, 0, Buf,
						a_X + STS_SBX(Profile, 240, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
					);
			}
		}
		
		//// FRAGS
		if (P_GMIsDM())
		{
			// Draw Icon
			vi = V_ImageFindA("sbofrags", VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 240, a_W, a_H),
						a_Y + STS_SBY(Profile, 8, a_W, a_H),
						NULL
					);
		
			// Draw Frags Text
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->TotalFrags);
			V_DrawStringA(
					Font, 0, Buf,
					a_X + STS_SBX(Profile, 240, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
					a_Y + STS_SBY(Profile, 8, a_W, a_H) - (BigLetters ? 4 : 0)
				);
		}
		
		//// KEYS
		Right = 8;
		for (k = 0; k < 2; k++)
			for (i = 0; i < 2; i++)
				for (j = 0; j < 32; j++)
				{
					OK = false;
				
					// We actually own the key
					if (DisplayP->KeyCards[i] & (UINT32_C(1) << j))
					{
						if (!k)
							OK = true;
					}
				
					// Do not own key, see if flashing
					else if (k == 1)
					{
						// Flash about every half second
						if (DisplayP->KeyFlash[i][j] & 0x10)
							OK = true;
					}
				
					// OK to draw?
					if (OK)
					{
						// Too many keys in view?
						if (Right > 300)
							break;
					
						// Try to find the key
						DrawKey = INFO_KeyByGroupBit(i, j);
					
						// No key exists here?
						if (!DrawKey)
							continue;
					
						// No image supplied?
						if (!DrawKey->ImageName)
							vi = V_ImageFindA("RMD_UKEY", VCP_DOOM);
					
						// Otherwise, use the key image possibly
						else
							vi = V_ImageFindA(DrawKey->ImageName, VCP_NONE);
					
						// Draw the key
						if (vi)
						{
							V_ImageDraw(0, vi, a_X + STS_SBX(Profile, Right, a_W, a_H), a_Y + STS_SBY(Profile, 165, a_W, a_H), NULL);
					
							// Shift
							Right += vi->Width + 1;
						}
					}
				}
	}
	
	/* Classic Doom */
	else
	{
	}
	
	/* Draw Object Overlays */
	
	/* We are looking at another player */
	if (ConsoleP != DisplayP)
	{
		// Put warning if the player is under attack
			// TODO
	}
	
	/* Scoreboard */
#if 0
	if (/*(XPlay && XPlay->Scores) ||*/ (a_PID == 0 && g_NetBoardDown))
	{
		WI_DrawScoreBoard(false, DS_GetString(DSTR_STSTUFFC_SCOREBOARD), NULL);
	}
#endif
	
	/* Chatting */
	SN_ChatDrawer(a_PID, a_X, a_Y, a_W, a_H);
#undef BUFSIZE
}

/*** FUNCTIONS ***/

// c_STBars -- Status bar implementations
static const struct
{
	ST_BarFunc_t Bar;
	ST_ModShapeFunc_t Shape;
} c_STBars[NUMPROFBARS] =
{
	{ST_DoomBar, ST_DoomModShape},
	{STS_DrawPlayerBarEx, NULL},
};

/* ST_GetDefaultBar() -- Returns the default status bar */
D_ProfBarType_t ST_GetDefaultBar(void)
{
	/*if (g_SplitScreen < 0)
		return DPBT_DEFAULT;
	else*/
		return DPBT_REMOOD;
}

/* ST_GetScreenCDP() -- Get console, display, and profile */
void ST_GetScreenCDP(const int32_t a_Split, player_t** const a_ConsolePP, player_t** const a_DisplayPP, D_Prof_t** const a_ProfP)
{
	int i;
	
	/* Check */
	if (a_Split < 0 || a_Split >= MAXSPLITS)
		return;
	
	// Get players to draw for
	if (!g_Splits[a_Split].Port)
	{
		if (g_Splits[a_Split].Console >= 0  && g_Splits[a_Split].Console < MAXPLAYERS && playeringame[g_Splits[a_Split].Console])
			(*a_ConsolePP) = &players[g_Splits[a_Split].Console];
		else
			(*a_ConsolePP) = NULL;
	}
	else
		(*a_ConsolePP) = g_Splits[a_Split].Port->Player;//&players[g_Splits[a_Split].Console];
	(*a_DisplayPP) = P_SpecGetPOV(a_Split);//&players[g_Splits[a_Split].Display];
	
	// Missing player?
	if (!(*a_ConsolePP))
		(*a_ConsolePP) = P_SpecGet(a_Split);
	
	if (!(*a_DisplayPP))
		(*a_DisplayPP) = (*a_ConsolePP);
	
	/* Find profile */
	(*a_ProfP) = NULL;
	for (i = 0; i < 2 && !(*a_ProfP); i++)
	{
		if (i == 0 && g_Splits[a_Split].Port && g_Splits[a_Split].Profile)
			(*a_ProfP) = g_Splits[a_Split].Profile;
		else if (i == 1 && (*a_ConsolePP)->ProfileEx)
			(*a_ProfP) = (*a_ConsolePP)->ProfileEx;
	}
}

/* ST_DrawPlayerBarsEx() -- Draw player status bars */
void ST_DrawPlayerBarsEx(void)
{
	player_t* ConsoleP, *DisplayP;
	int p, x, y, w, h, i;
	bool_t BigLetters;
	static uint32_t LastPal;	// Lowers palette change (faster drawing)
	D_Prof_t* Prof;
	D_SplitInfo_t* Split;
	
	/* Screen division? */
	// Initial
	x = y = 0;
	w = 320;
	h = 200;
	
	// 2+ split
	if (g_SplitScreen >= 1)
		h /= 2;
	
	// 3+ split
	if (g_SplitScreen >= 2)
		w /= 2;
		
	/* Use standard palette */
	if (g_SplitScreen > 0)
	{
		if (LastPal != 0)
			V_SetPalette(0);
		LastPal = 0;
	}
	
	/* Draw each player */
	for (p = 0; p < (g_SplitScreen < 0 ? 1 : g_SplitScreen + 1) && p < MAXSPLITS; p++)
	{
		// Reference split
		Split = &g_Splits[p];
		
		// Split player active
		if (D_ScrSplitVisible(p) || (demoplayback && g_Splits[p].Active))
		{
			ST_GetScreenCDP(p, &ConsoleP, &DisplayP, &Prof);
			
			// Modify palette?
			if (g_SplitScreen <= 0)	// Only 1 player inside
			{
				if (LastPal != DisplayP->PalChoice)
				{
					V_SetPalette(DisplayP->PalChoice);
					LastPal = DisplayP->PalChoice;
				}
			}
			
			// If profile was found and bar type is legal
			if (Prof && Prof->BarType >= 0 && Prof->BarType < NUMPROFBARS)
				c_STBars[Prof->BarType].Bar(p, x, y, w, h, ConsoleP, DisplayP, Prof);
			
			// Otherwise, draw the standard bar
			else
				c_STBars[ST_GetDefaultBar()].Bar(p, x, y, w, h, ConsoleP, DisplayP, Prof);
		}
		
		// Profile selection
		if (Split->SelProfile)
		{
			// Text
			V_DrawStringA(
					VFONT_LARGE, 0, "Select Profile",
					x + 5,
					y + 5
				);
			
			if (Split->AtProf)
				V_DrawStringA(
						VFONT_SMALL, 0, Split->AtProf->DisplayName,
						x + 5,
						y + 10 + V_FontHeight(VFONT_LARGE)
					);
		}
	
		// Add to coords (finished drawing everything, or not drawn at all)
		if (g_SplitScreen == 1)
			y += h;
		else if (g_SplitScreen > 1)
		{
			x += w;
		
			if (x == (w * 2))
			{
				x = 0;
				y += h;
			}
		}
	}
}

/* ST_InitEx() -- Initializes the extended status bar */
void ST_InitEx(void)
{
}

/* ST_TickerEx() -- Extended Ticker */
void ST_TickerEx(void)
{
	player_t* Player;
	int ChosePal = 0;
	int BaseDam = 0, BzFade;
	size_t p;
	bool_t ActiveMenu;
	
	/* Update for all players */
	for (p = 0; p < MAXPLAYERS; p++)
	{
		// Get player
		Player = &players[p];
		
		// No player here?
		if (!playeringame[p])
			continue;
		
		// Player has a menu open?
		ActiveMenu = M_SMPlayerMenuVisible(p);
		
		// Player Palette
			// Reset variables -- Otherwise palettes "stick"
		ChosePal = BaseDam = BzFade = 0;
		
			// Berserker fade
		BaseDam = Player->damagecount;
	
		if (Player->powers[pw_strength])
		{
			BzFade = 12 - (Player->powers[pw_strength] >> 6);
		
			if (BzFade > BaseDam)
				BaseDam = BzFade;
		}
	
			// Player is hurt?
		if (BaseDam)
		{
			// Division is number of palettes
			ChosePal = FixedMul(NUMREDPALS << FRACBITS, FixedDiv((BaseDam) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
			ChosePal++;				// +7
			//ChosePal = (plyr->damagecount * (10000 / NUMREDPALS) ) / 100;
			
			// Menu Cutdown
			if (ActiveMenu)
				ChosePal >>= 1;
			
			// Don't exceed
			if (ChosePal >= NUMREDPALS)
				ChosePal = NUMREDPALS - 1;
			
			// Offset
			ChosePal += STARTREDPALS;
		}
	
			// Player got an item
		else if (Player->bonuscount)
		{
			// Division is number of palettes
			ChosePal = FixedMul(NUMBONUSPALS << FRACBITS, FixedDiv((Player->bonuscount) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
			ChosePal++;				// +7
			
			// Menu Cutdown
			if (ActiveMenu)
				ChosePal >>= 1;
		
			// Don't exceed
			if (ChosePal >= NUMBONUSPALS)
				ChosePal = NUMBONUSPALS - 1;
			
			// Offset
			ChosePal += STARTBONUSPALS;
		}
	
			// Player has radiation suit
		else if (Player->powers[pw_ironfeet] > 4 * 32 || Player->powers[pw_ironfeet] & 8)
		{
			ChosePal = RADIATIONPAL;
		}
		
		// Set palette to what was chosen
		Player->PalChoice = ChosePal;
	}
}

/* ST_ExSoloViewTransSBar() -- Transparent status bar for single view */
bool_t ST_ExSoloViewTransSBar(void)
{
	return false;
}

/* ST_ExSoloViewScaledSBar() -- Scaled status bar for single view */
bool_t ST_ExSoloViewScaledSBar(void)
{
	return false;
}

/* ST_ExViewBarHeight() -- Status Bar Height */
int32_t ST_ExViewBarHeight(void)
{
	return 0;
}

/* ST_CalcScreen() -- Calculates render screen size for local player */
void ST_CalcScreen(const int32_t a_ThisPlayer, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H)
{
	player_t* ConsoleP, *DisplayP;
	D_Prof_t* Prof;
	int right, bottom;
	
	/* Check */
	if (a_ThisPlayer < 0 || a_ThisPlayer >= MAXSPLITS)
		return;	
	
	/* Get players */
	ConsoleP = DisplayP = NULL;
	Prof = NULL;
	ST_GetScreenCDP(a_ThisPlayer, &ConsoleP, &DisplayP, &Prof);
	
	/* How many splits? */
	switch (g_SplitScreen)
	{
			// 3/4 Player
		case 2:
		case 3:
			right = a_ThisPlayer & 1;
			bottom = (a_ThisPlayer >> 1) & 1;
		
			*a_X = (vid.width >> 1) * right;
			*a_Y = (vid.height >> 1) * bottom;
			*a_W = vid.width >> 1;
			*a_H = vid.height >> 1;
			break;
			
			// 2 Player
		case 1:
			bottom = a_ThisPlayer & 1;
			
			*a_X = 0;
			*a_Y = (vid.height >> 1) * bottom;
			*a_W = vid.width;
			*a_H = vid.height >> 1;
			break;
			
			// 1 Player
		default:
			*a_X = 0;
			*a_Y = 0;
			*a_W = vid.width;
			*a_H = vid.height;
			break;
	}
	
	/* Reshape window, if only 1 player is playing */
	if (g_SplitScreen < 0)
	{
		// Viewsize rescale
		
		// Status bar reshape
		if (Prof && Prof->BarType >= 0 && Prof->BarType < NUMPROFBARS)
		{
			if (c_STBars[Prof->BarType].Shape)
				c_STBars[Prof->BarType].Shape(a_ThisPlayer, a_X, a_Y, a_W, a_H, ConsoleP, DisplayP, Prof);
		}
	
		// Otherwise, use the standard bar
		else
		{
			if (c_STBars[ST_GetDefaultBar()].Shape)
				c_STBars[ST_GetDefaultBar()].Shape(a_ThisPlayer, a_X, a_Y, a_W, a_H, ConsoleP, DisplayP, Prof);
		}
	}
}

/* ST_CheckDrawGameView() -- Checks if game view can be drawn */
bool_t ST_CheckDrawGameView(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITS)
		return false;	
	
	/* Automap disables view */
	if (g_Splits[a_Screen].AutomapActive)
		// Overlay automap draws the view
		if (g_Splits[a_Screen].OverlayMap)
			return true;
		else
			return false;
	
	/* Otherwise, draw it */
	return true;
}

