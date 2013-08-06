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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Hexen Line Types

/***************
*** INCLUDES ***
***************/

//#include "doomtype.h"
//#include "doomdef.h"
//#include "p_mobj.h"
//#include "r_local.h"
//#include "p_spec.h"
//#include "console.h"

/****************
*** FUNCTIONS ***
****************/

/* PHL_Line_SetIdentification() -- Set line ID */
static bool_t PHL_Line_SetIdentification(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain)
{
	/* Setting ID */
	if (a_Line->ACSArgs[0])
		EV_TagACSLine(a_Line, a_Line->ACSArgs[0]);
	
	/* Clear */
	a_Line->special = 0;
	a_Line->HexenSpecial = 0;
	return true;
}

/* c_HexenEVs -- Hexen line table */
static const struct
{
	uint8_t ID;
	bool_t StartOnly;
	const char* Name;
	EV_Action_t Func;
} c_HexenEVs[256] =
{
	{0,		false,	"Unknown",	NULL},
	{1,		false,	"Unknown",	NULL},
	{2,		false,	"Unknown",	NULL},
	{3,		false,	"Unknown",	NULL},
	{4,		false,	"Unknown",	NULL},
	{5,		false,	"Unknown",	NULL},
	{6,		false,	"Unknown",	NULL},
	{7,		false,	"Unknown",	NULL},
	{8,		false,	"Unknown",	NULL},
	{9,		false,	"Unknown",	NULL},
	{10,	false,	"Unknown",	NULL},
	{11,	false,	"Unknown",	NULL},
	{12,	false,	"Unknown",	NULL},
	{13,	false,	"Unknown",	NULL},
	{14,	false,	"Unknown",	NULL},
	{15,	false,	"Unknown",	NULL},
	{16,	false,	"Unknown",	NULL},
	{17,	false,	"Unknown",	NULL},
	{18,	false,	"Unknown",	NULL},
	{19,	false,	"Unknown",	NULL},
	{20,	false,	"Unknown",	NULL},
	{21,	false,	"Unknown",	NULL},
	{22,	false,	"Unknown",	NULL},
	{23,	false,	"Unknown",	NULL},
	{24,	false,	"Unknown",	NULL},
	{25,	false,	"Unknown",	NULL},
	{26,	false,	"Unknown",	NULL},
	{27,	false,	"Unknown",	NULL},
	{28,	false,	"Unknown",	NULL},
	{29,	false,	"Unknown",	NULL},
	{30,	false,	"Unknown",	NULL},
	{31,	false,	"Unknown",	NULL},
	{32,	false,	"Unknown",	NULL},
	{33,	false,	"Unknown",	NULL},
	{34,	false,	"Unknown",	NULL},
	{35,	false,	"Unknown",	NULL},
	{36,	false,	"Unknown",	NULL},
	{37,	false,	"Unknown",	NULL},
	{38,	false,	"Unknown",	NULL},
	{39,	false,	"Unknown",	NULL},
	{40,	false,	"Unknown",	NULL},
	{41,	false,	"Unknown",	NULL},
	{42,	false,	"Unknown",	NULL},
	{43,	false,	"Unknown",	NULL},
	{44,	false,	"Unknown",	NULL},
	{45,	false,	"Unknown",	NULL},
	{46,	false,	"Unknown",	NULL},
	{47,	false,	"Unknown",	NULL},
	{48,	false,	"Unknown",	NULL},
	{49,	false,	"Unknown",	NULL},
	{50,	false,	"Unknown",	NULL},
	{51,	false,	"Unknown",	NULL},
	{52,	false,	"Unknown",	NULL},
	{53,	false,	"Unknown",	NULL},
	{54,	false,	"Unknown",	NULL},
	{55,	false,	"Unknown",	NULL},
	{56,	false,	"Unknown",	NULL},
	{57,	false,	"Unknown",	NULL},
	{58,	false,	"Unknown",	NULL},
	{59,	false,	"Unknown",	NULL},
	{60,	false,	"Unknown",	NULL},
	{61,	false,	"Unknown",	NULL},
	{62,	false,	"Unknown",	NULL},
	{63,	false,	"Unknown",	NULL},
	{64,	false,	"Unknown",	NULL},
	{65,	false,	"Unknown",	NULL},
	{66,	false,	"Unknown",	NULL},
	{67,	false,	"Unknown",	NULL},
	{68,	false,	"Unknown",	NULL},
	{69,	false,	"Unknown",	NULL},
	{70,	false,	"Unknown",	NULL},
	{71,	false,	"Unknown",	NULL},
	{72,	false,	"Unknown",	NULL},
	{73,	false,	"Unknown",	NULL},
	{74,	false,	"Unknown",	NULL},
	{75,	false,	"Unknown",	NULL},
	{76,	false,	"Unknown",	NULL},
	{77,	false,	"Unknown",	NULL},
	{78,	false,	"Unknown",	NULL},
	{79,	false,	"Unknown",	NULL},
	{80,	false,	"Unknown",	NULL},
	{81,	false,	"Unknown",	NULL},
	{82,	false,	"Unknown",	NULL},
	{83,	false,	"Unknown",	NULL},
	{84,	false,	"Unknown",	NULL},
	{85,	false,	"Unknown",	NULL},
	{86,	false,	"Unknown",	NULL},
	{87,	false,	"Unknown",	NULL},
	{88,	false,	"Unknown",	NULL},
	{89,	false,	"Unknown",	NULL},
	{90,	false,	"Unknown",	NULL},
	{91,	false,	"Unknown",	NULL},
	{92,	false,	"Unknown",	NULL},
	{93,	false,	"Unknown",	NULL},
	{94,	false,	"Unknown",	NULL},
	{95,	false,	"Unknown",	NULL},
	{96,	false,	"Unknown",	NULL},
	{97,	false,	"Unknown",	NULL},
	{98,	false,	"Unknown",	NULL},
	{99,	false,	"Unknown",	NULL},
	{100,	false,	"Unknown",	NULL},
	{101,	false,	"Unknown",	NULL},
	{102,	false,	"Unknown",	NULL},
	{103,	false,	"Unknown",	NULL},
	{104,	false,	"Unknown",	NULL},
	{105,	false,	"Unknown",	NULL},
	{106,	false,	"Unknown",	NULL},
	{107,	false,	"Unknown",	NULL},
	{108,	false,	"Unknown",	NULL},
	{109,	false,	"Unknown",	NULL},
	{110,	false,	"Unknown",	NULL},
	{111,	false,	"Unknown",	NULL},
	{112,	true,	"Line_SetIdentification",	PHL_Line_SetIdentification},
	{113,	false,	"Unknown",	NULL},
	{114,	false,	"Unknown",	NULL},
	{115,	false,	"Unknown",	NULL},
	{116,	false,	"Unknown",	NULL},
	{117,	false,	"Unknown",	NULL},
	{118,	false,	"Unknown",	NULL},
	{119,	false,	"Unknown",	NULL},
	{120,	false,	"Unknown",	NULL},
	{121,	false,	"Unknown",	NULL},
	{122,	false,	"Unknown",	NULL},
	{123,	false,	"Unknown",	NULL},
	{124,	false,	"Unknown",	NULL},
	{125,	false,	"Unknown",	NULL},
	{126,	false,	"Unknown",	NULL},
	{127,	false,	"Unknown",	NULL},
	{128,	false,	"Unknown",	NULL},
	{129,	false,	"Unknown",	NULL},
	{130,	false,	"Unknown",	NULL},
	{131,	false,	"Unknown",	NULL},
	{132,	false,	"Unknown",	NULL},
	{133,	false,	"Unknown",	NULL},
	{134,	false,	"Unknown",	NULL},
	{135,	false,	"Unknown",	NULL},
	{136,	false,	"Unknown",	NULL},
	{137,	false,	"Unknown",	NULL},
	{138,	false,	"Unknown",	NULL},
	{139,	false,	"Unknown",	NULL},
	{140,	false,	"Unknown",	NULL},
	{141,	false,	"Unknown",	NULL},
	{142,	false,	"Unknown",	NULL},
	{143,	false,	"Unknown",	NULL},
	{144,	false,	"Unknown",	NULL},
	{145,	false,	"Unknown",	NULL},
	{146,	false,	"Unknown",	NULL},
	{147,	false,	"Unknown",	NULL},
	{148,	false,	"Unknown",	NULL},
	{149,	false,	"Unknown",	NULL},
	{150,	false,	"Unknown",	NULL},
	{151,	false,	"Unknown",	NULL},
	{152,	false,	"Unknown",	NULL},
	{153,	false,	"Unknown",	NULL},
	{154,	false,	"Unknown",	NULL},
	{155,	false,	"Unknown",	NULL},
	{156,	false,	"Unknown",	NULL},
	{157,	false,	"Unknown",	NULL},
	{158,	false,	"Unknown",	NULL},
	{159,	false,	"Unknown",	NULL},
	{160,	false,	"Unknown",	NULL},
	{161,	false,	"Unknown",	NULL},
	{162,	false,	"Unknown",	NULL},
	{163,	false,	"Unknown",	NULL},
	{164,	false,	"Unknown",	NULL},
	{165,	false,	"Unknown",	NULL},
	{166,	false,	"Unknown",	NULL},
	{167,	false,	"Unknown",	NULL},
	{168,	false,	"Unknown",	NULL},
	{169,	false,	"Unknown",	NULL},
	{170,	false,	"Unknown",	NULL},
	{171,	false,	"Unknown",	NULL},
	{172,	false,	"Unknown",	NULL},
	{173,	false,	"Unknown",	NULL},
	{174,	false,	"Unknown",	NULL},
	{175,	false,	"Unknown",	NULL},
	{176,	false,	"Unknown",	NULL},
	{177,	false,	"Unknown",	NULL},
	{178,	false,	"Unknown",	NULL},
	{179,	false,	"Unknown",	NULL},
	{180,	false,	"Unknown",	NULL},
	{181,	false,	"Unknown",	NULL},
	{182,	false,	"Unknown",	NULL},
	{183,	false,	"Unknown",	NULL},
	{184,	false,	"Unknown",	NULL},
	{185,	false,	"Unknown",	NULL},
	{186,	false,	"Unknown",	NULL},
	{187,	false,	"Unknown",	NULL},
	{188,	false,	"Unknown",	NULL},
	{189,	false,	"Unknown",	NULL},
	{190,	false,	"Unknown",	NULL},
	{191,	false,	"Unknown",	NULL},
	{192,	false,	"Unknown",	NULL},
	{193,	false,	"Unknown",	NULL},
	{194,	false,	"Unknown",	NULL},
	{195,	false,	"Unknown",	NULL},
	{196,	false,	"Unknown",	NULL},
	{197,	false,	"Unknown",	NULL},
	{198,	false,	"Unknown",	NULL},
	{199,	false,	"Unknown",	NULL},
	{200,	false,	"Unknown",	NULL},
	{201,	false,	"Unknown",	NULL},
	{202,	false,	"Unknown",	NULL},
	{203,	false,	"Unknown",	NULL},
	{204,	false,	"Unknown",	NULL},
	{205,	false,	"Unknown",	NULL},
	{206,	false,	"Unknown",	NULL},
	{207,	false,	"Unknown",	NULL},
	{208,	false,	"Unknown",	NULL},
	{209,	false,	"Unknown",	NULL},
	{210,	false,	"Unknown",	NULL},
	{211,	false,	"Unknown",	NULL},
	{212,	false,	"Unknown",	NULL},
	{213,	false,	"Unknown",	NULL},
	{214,	false,	"Unknown",	NULL},
	{215,	false,	"Unknown",	NULL},
	{216,	false,	"Unknown",	NULL},
	{217,	false,	"Unknown",	NULL},
	{218,	false,	"Unknown",	NULL},
	{219,	false,	"Unknown",	NULL},
	{220,	false,	"Unknown",	NULL},
	{221,	false,	"Unknown",	NULL},
	{222,	false,	"Unknown",	NULL},
	{223,	false,	"Unknown",	NULL},
	{224,	false,	"Unknown",	NULL},
	{225,	false,	"Unknown",	NULL},
	{226,	false,	"Unknown",	NULL},
	{227,	false,	"Unknown",	NULL},
	{228,	false,	"Unknown",	NULL},
	{229,	false,	"Unknown",	NULL},
	{230,	false,	"Unknown",	NULL},
	{231,	false,	"Unknown",	NULL},
	{232,	false,	"Unknown",	NULL},
	{233,	false,	"Unknown",	NULL},
	{234,	false,	"Unknown",	NULL},
	{235,	false,	"Unknown",	NULL},
	{236,	false,	"Unknown",	NULL},
	{237,	false,	"Unknown",	NULL},
	{238,	false,	"Unknown",	NULL},
	{239,	false,	"Unknown",	NULL},
	{240,	false,	"Unknown",	NULL},
	{241,	false,	"Unknown",	NULL},
	{242,	false,	"Unknown",	NULL},
	{243,	false,	"Unknown",	NULL},
	{244,	false,	"Unknown",	NULL},
	{245,	false,	"Unknown",	NULL},
	{246,	false,	"Unknown",	NULL},
	{247,	false,	"Unknown",	NULL},
	{248,	false,	"Unknown",	NULL},
	{249,	false,	"Unknown",	NULL},
	{250,	false,	"Unknown",	NULL},
	{251,	false,	"Unknown",	NULL},
	{252,	false,	"Unknown",	NULL},
	{253,	false,	"Unknown",	NULL},
	{254,	false,	"Unknown",	NULL},
	{255,	false,	"Unknown",	NULL}
};

/* EV_DoHexenLine() -- Triggers hexen line */
bool_t EV_DoHexenLine(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain)
{
	/* Debug */
	if (devparm)
		CONL_PrintF("HexT %p by %p (side %+1i): Via %c, %3u [%02x, %02x, %02x, %02x, %02x]\n", a_Line, a_Object, a_Side, (a_Type == LAT_WALK ? 'W' : (a_Type == LAT_SHOOT ? 'G' : 'S')), a_Line->HexenSpecial, a_Line->ACSArgs[0], a_Line->ACSArgs[1], a_Line->ACSArgs[2], a_Line->ACSArgs[3], a_Line->ACSArgs[4]);
		
	/* Out of range? */
	if (a_Line->HexenSpecial < 0 || a_Line->HexenSpecial > 255)
		return false;
	
	/* No defined function? */
	if (!c_HexenEVs[a_Line->HexenSpecial].Func)
		return false;
	
	/* Mismatch level start? */
	if ((a_Type == LAT_MAPSTART && !c_HexenEVs[a_Line->HexenSpecial].StartOnly) ||
		(a_Type != LAT_MAPSTART && c_HexenEVs[a_Line->HexenSpecial].StartOnly))
		return false;
	
	/* Call function */
	return c_HexenEVs[a_Line->HexenSpecial].Func(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain);
}

