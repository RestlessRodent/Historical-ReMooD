// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: User Interface Drawer

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "g_state.h"

/*****************
*** STRUCTURES ***
*****************/

/*****************
*** PROTOTYPES ***
*****************/

void D_UITitle(UI_BufferSpec_t* const a_Spec);

/****************
*** FUNCTIONS ***
****************/

/* UI_DrawBGLayer() -- Single Instance Drawing Specification */
void UI_DrawBGLayer(UI_BufferSpec_t* const a_Spec)
{
	/* Drawing is based on the current game state */
	switch (gamestate)
	{
			// Title Screen
		case GS_DEMOSCREEN:
			D_UITitle(a_Spec);
			break;
		
			// Unknown
		default:
			break;
	}
}

/* UI_DrawFGLayer() -- Draws the foreground layer */
void UI_DrawFGLayer(UI_BufferSpec_t* const a_Spec)
{
}

/* UI_DrawLoop() -- UI Drawing Loop */
// This replaces D_Display()!
void UI_DrawLoop(void)
{
	UI_BufferSpec_t Spec;
	
	/* Obtain screen spec */
	// Screen is locked by soft buffer, if needed
	Spec.Data = I_VideoSoftBuffer(&Spec.w, &Spec.h, &Spec.d, &Spec.p);
	Spec.pd = Spec.p * Spec.d;
	
	/* Draw single loop */
	// Background
	UI_DrawBGLayer(&Spec);
	
	// Foreground
	UI_DrawFGLayer(&Spec);
	
	/* Update the screen */
	I_FinishUpdate();
	
	/* Unlock the screen */
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/


