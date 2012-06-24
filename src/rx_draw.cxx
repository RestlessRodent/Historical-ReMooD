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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Renderer -- Draws Sky

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "rx_main.h"
#include "doomstat.h"

/****************
*** FUNCTIONS ***
****************/

/* RXS_ProjectPoint() -- Project points */
// cos(y) * (sin(z) * (ay - cy) + cos(z) * (az - cx)) - sin(y) * (az - cz)
// sin(x) * (cos(y) * (az - cx) + sin(y) * (sin(z) * (ay - cy) + cos(z) * (ax - cx))) + cos(x) * (cos(z) * (ay - cy) - sin(z) * (ax - cy))
// cos(x) * (cos(y) * (az - cx) + sin(y) * (sin(z) * (ay - cy) + cos(z) * (ax - cx))) - sin(x) * (cos(z) * (ay - cy) - sin(z) * (ax - cy))
static void RXS_ProjectPoint(RX_RenderSpec_t* const a_Spec, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z)
{
	fixed_t daycy, dazcx, dazcz, daxcy, daxcx;
	fixed_t bitA, bitB, bitC, bitD, bitE;
	
	/* Pre-Calc */
	daycy = a_Y - a_Spec->CamD[1];
	dazcx = a_Z - a_Spec->CamD[0];
	dazcz = a_Z - a_Spec->CamD[2];
	daxcy = a_X - a_Spec->CamD[1];
	daxcx = a_X - a_Spec->CamD[0];
	
	/* Bits */
	// cos(y) * dazcx
	bitA = FixedMul(a_Spec->ViewCos[1], dazcx);
	
	// sin(z) * daycy
	bitB = FixedMul(a_Spec->ViewSin[2], daycy);
	
	// cos(z) * daxcx
	bitC = FixedMul(a_Spec->ViewCos[2], daxcx);
	
	// bitA + sin(y) * (bitB + bitC)
	bitD = bitA + FixedMul(a_Spec->ViewSin[1], bitB + bitC);
	
	// cos(z) * daycy - sin(z) * daxcy
	bitE = FixedMul(a_Spec->ViewCos[2], daycy) - FixedMul(a_Spec->ViewSin[2], daxcy);
	
	/* Project Final Points */
	// cos(y) * (bitB + cos(z) * dazcx) - sin(y) * dazcz
	a_Spec->ProjD[0] = FixedMul(a_Spec->ViewCos[1], (bitB + FixedMul(a_Spec->ViewCos[2], dazcx))) - FixedMul(a_Spec->ViewSin[1], dazcz);
	
	// sin(x) * (bitD) + cos(x) * (bitE)
	a_Spec->ProjD[1] = FixedMul(a_Spec->ViewSin[0], bitD) + FixedMul(a_Spec->ViewCos[0], bitE);
	
	// cos(x) * (bitD) - sin(x) * (bitE)
	a_Spec->ProjD[2] = FixedMul(a_Spec->ViewCos[0], bitD) - FixedMul(a_Spec->ViewSin[0], bitE);
}

/* RX_DrawPoly() -- Draws Polygon */
void RX_DrawPoly(RX_RenderSpec_t* const a_Spec, const uint8_t a_Color, const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_z1, const fixed_t a_x2, const fixed_t a_y2, const fixed_t a_z2, const fixed_t a_x3, const fixed_t a_y3, const fixed_t a_z3)
{
}

