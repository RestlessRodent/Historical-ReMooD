// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Interface Drawing Control

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "i_video.h"
#include "ui_dloc.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define UIPROT(d) \
void UI_##d##_DrawImg(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y);\
void UI_##d##_DrawImgScale(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const fixed_t a_sW, const fixed_t a_sH);

#define UISET(d) \
UI_DrawImg = UI_##d##_DrawImg;\
UI_DrawImgScale = UI_##d##_DrawImgScale

UIPROT(d8)
UIPROT(d16)
UIPROT(d32)
UIPROT(dgl)

/**************
*** GLOBALS ***
**************/

void (*UI_DrawImg)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y) = NULL;
void (*UI_DrawImgScale)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const fixed_t a_sW, const fixed_t a_sH) = NULL;

/*****************
*** PROTOTYPES ***
*****************/

void D_UILoadTitles(void);

/****************
*** FUNCTIONS ***
****************/

/* UI_ColorRelated() -- Color related changes */
static void UI_ColorRelated(void)
{
	/* Clear image list */
	UI_ImgClearList();
	
	/* Load the default palette (PLAYPAL) */
	UI_ImgLoadPalette();
	
	/* Indicate depth changes */
	D_UITitleDepthChange();
}

/* UI_WLOrder() -- WAD order list */
bool_t UI_WLOrder(const bool_t a_Pushed, const WL_WADFile_t* const a_WAD)
{
	/* Color change related */
	UI_ColorRelated();
	
	/* Initialize the title screen */
	D_UILoadTitles();
	
	/* Always works */
	return true;
}

/* UI_Init() -- Initializes the UI */
void UI_Init(void)
{
	if (!WL_RegisterOCCB(UI_WLOrder, WLDCO_UIIMAGES))
		I_Error("Failed to register UI_WLOrder");
}

/* UI_SetBitDepth() -- Sets bit depth of rendering */
void UI_SetBitDepth(const uint32_t a_Depth)
{
	static bool_t FirstDepth;
	
	/* Remove boot logo */
	if (FirstDepth)
		UI_ConBootClear();
	
	/* Color change related */
	UI_ColorRelated();
	
	/* Initialize the boot logo */
	if (!FirstDepth)
	{
		FirstDepth = true;
		
		UI_ConBootInit();
	}
	
	/* Set drawer functions */
	switch (a_Depth)
	{
			// 8-bit
		case 1:
			UISET(d8);
			break;
			
			// 16-bit
		case 2:
			UISET(d16);
			break;
			
			// 32-bit
		case 4:
			UISET(d32);
			break;
			
			// OpenGL
		case I_VIDEOGLMODECONST:
			UISET(dgl);
			break;
	}
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

