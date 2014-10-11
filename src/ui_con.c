// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Console Drawer

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "g_state.h"
#include "screen.h"

#include "bootdata.h"

/*************
*** LOCALS ***
*************/

static UI_Img_t* l_BootLogo = NULL;

/****************
*** FUNCTIONS ***
****************/

/* UI_ConBootInit() -- Initialization of the boot console */
void UI_ConBootInit(void)
{
	/* Load boot logo */
	if (!(l_BootLogo = UI_ImgLoadBootLogo(c_BootLogo, CBOOTLOGOSIZE)))
		I_Error("Failed to load boot logo!");
	
	/* Reference it */
	UI_ImgCount(l_BootLogo, 1);
}

/* UI_ConBootClear() -- Clear the boot logo */
void UI_ConBootClear(void)
{
	/* If image does not exist, do not bother */
	if (!l_BootLogo)
		return;	
	
	/* Dereference logo */
	UI_ImgCount(l_BootLogo, -1);
	
	// No longer reference it
	l_BootLogo = NULL;
}

/* UI_ConPassLine() -- Passes line of text to the console */
void UI_ConPassLine(const char* const a_Line)
{
	/* Boot Console? */
	if (l_BootLogo)
	{
	}
	
	/* Normal Console */
	else
	{
	}
}

/* UI_ConDraw() -- Draws the console */
void UI_ConDraw(UI_BufferSpec_t* const a_Spec)
{
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

