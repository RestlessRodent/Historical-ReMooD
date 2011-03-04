// Emacs style mode select   -*- C++ -*- 
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
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//

#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>

#include "am_map.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"
#include "t_script.h"

#include "d_net.h"
#include "p_inter.h"
#include "dstrings.h"
#include "v_video.h"

// Demyx -- Only here temporary, this needs a new home!
CV_PossibleValue_t blinkingrate_cons_t[] = {
	//{1, "1 (No cursor)"},
	{2, "2 (Very Fast)"},
	{3, "3 (Fast)"},
	{4, "4 (Default)"},
	{5, "5 (Slow)"},
	{6, "6 (Slower)"},
	{7, "7 (Very Slow)"},
	{8, "8 (Slowest)"},
	{9, "9 (No blinking)"},
	{0, NULL}
};

consvar_t cv_blinkingrate = {"blinkingrate", "4", CV_ALIAS | CV_DEPRECATED, blinkingrate_cons_t, NULL, "cons_blinkingrate"};
consvar_t cv_cons_blinkingrate = {"cons_blinkingrate", "4", CV_SAVE, blinkingrate_cons_t};

//===========================================================================
//Generic Stuffs (more easy to create menus :))
//===========================================================================

void M_DrawMenuTitle(void)
{
	int xtitle = 0;
	int ytitle = 0;
	
	if (CharacterGroups[VFONT_LARGE] && currentMenu->WMenuTitlePtr)
	{
		xtitle = (BASEVIDWIDTH - V_StringWidthW(VFONT_LARGE, 0, *(currentMenu->WMenuTitlePtr))) / 2;
		ytitle = (currentMenu->y - V_StringHeightW(VFONT_LARGE, 0, *(currentMenu->WMenuTitlePtr))) / 2;
		if (xtitle < 0)
			xtitle = 0;
		if (ytitle < 0)
			ytitle = 0;

		V_DrawStringW(VFONT_LARGE, 0, *(currentMenu->WMenuTitlePtr), xtitle, ytitle);
	}
	else if (currentMenu->menutitlepic)
	{
		patch_t *p = W_CachePatchName(currentMenu->menutitlepic, PU_CACHE);
		
		if (currentMenu->menutitlex <= -1)
			xtitle = 94;
		else
			xtitle = currentMenu->menutitlex;
			
		if (currentMenu->menutitley <= -1)
			ytitle = 2;
		else
			ytitle = currentMenu->menutitley;

		if (xtitle < 0)
			xtitle = 0;
		if (ytitle < 0)
			ytitle = 0;
			
		V_DrawScaledPatch(xtitle, ytitle, 0, p);
	}
}

void M_DrawGenericMenu(void)
{
	int x, y, i, cursory = 0;
	int fixedx, fixedy;
	int cursorx1;
	int cursorx2;
	int USSizeW = V_StringWidthW(VFONT_SMALL, 0, L"_");
	int USSizeH = V_StringHeightW(VFONT_SMALL, 0, L"_");

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;

	// draw title (or big pic)
	M_DrawMenuTitle();
	
	// Check itemsperpage
	if (!currentMenu->itemsperpage)
		currentMenu->itemsperpage = 500;

	for (i = currentMenu->firstdraw; i < currentMenu->numitems && i < (currentMenu->firstdraw + currentMenu->itemsperpage); i++)
	{
		if (i == itemOn)
			cursory = y;
		switch (currentMenu->menuitems[i].status & IT_DISPLAY)
		{
			case IT_PATCH:
				if (CharacterGroups[VFONT_LARGE] && currentMenu->menuitems[i].WItemTextPtr)
				{
					V_DrawStringW(VFONT_LARGE, 0, *(currentMenu->menuitems[i].WItemTextPtr), x, y);
					y += FONTBHEIGHT - LINEHEIGHT;
				}
				else if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
					V_DrawScaledPatch(x, y, 0,
									  W_CachePatchName(currentMenu->menuitems[i].patch, PU_CACHE));
			case IT_NOTHING:
			case IT_DYBIGSPACE:
				y += LINEHEIGHT;
				break;
			case IT_BIGSLIDER:
				M_DrawThermo(x, y, (consvar_t *) currentMenu->menuitems[i].itemaction);
				y += LINEHEIGHT;
				break;
			case IT_STRING:
			case IT_WHITESTRING:
				if (currentMenu->menuitems[i].alphaKey)
					y = currentMenu->y + currentMenu->menuitems[i].alphaKey;
				if (i == itemOn)
					cursory = y;
					
				if (currentMenu->menuitems[i].status & IT_CENTERSTRING)
				{
					fixedx = ((currentMenu->width + currentMenu->x) >> 1) -
						(V_StringWidthW(VFONT_SMALL, 0, *(currentMenu->menuitems[i].WItemTextPtr)) >> 1);
						
					if (i == itemOn)
					{
						cursorx1 = fixedx - 10;
						cursorx2 = fixedx + V_StringWidthW(VFONT_SMALL, 0, *(currentMenu->menuitems[i].WItemTextPtr)) + 4;
					}
				}
				else
					fixedx = x;
				fixedy = y;

				if (currentMenu->menuitems[i].status & IT_DISABLED2)
					V_DrawStringW(VFONT_SMALL, VFONTOPTION_GRAY, *(currentMenu->menuitems[i].WItemTextPtr), fixedx, fixedy);
				else
				{
					if ((currentMenu->menuitems[i].status & IT_DISPLAY) == IT_STRING)
						V_DrawStringW(VFONT_SMALL, 0, *(currentMenu->menuitems[i].WItemTextPtr), fixedx, fixedy);
					else
						V_DrawStringW(VFONT_SMALL, VFONTOPTION_WHITE, *(currentMenu->menuitems[i].WItemTextPtr), fixedx, fixedy);
				}

				// Cvar specific handling
				switch (currentMenu->menuitems[i].status & IT_TYPE)
					case IT_CVAR:
					{
						consvar_t *cv = (consvar_t *) currentMenu->menuitems[i].itemaction;
						switch (currentMenu->menuitems[i].status & IT_CVARTYPE)
						{
							case IT_CV_SLIDER:
								M_DrawSlider((currentMenu->x + currentMenu->width) - SLIDER_WIDTH,
											 y,
											 ((cv->value -
											   cv->PossibleValue[0].value) *
											  100 /
											  (cv->PossibleValue[1].value -
											   cv->PossibleValue[0].value)),
											   (currentMenu->menuitems[i].status & IT_DISABLED2 || currentMenu->menuitems[i].status &  IT_CVARREADONLY ? graymap : 0));
								//y += STRINGHEIGHT;
							case IT_CV_NOPRINT:	// color use this 
								//y += STRINGHEIGHT;
								break;
							case IT_CV_STRING:
									/* TODO: WHY constantly check the size of a single char? */
#if 0
								V_DrawFill((currentMenu->x + currentMenu->width) -
									V_StringWidthW(VFONT_SMALL, 0, cv->string) - (i == itemOn ? USSizeW : 0) - 1,
									y - 1, V_StringWidthW(VFONT_SMALL, 0, cv->string) + (i == itemOn ? USSizeW : 0) + 2,
									USSizeH + 2, 0);
								V_DrawStringW(VFONT_SMALL, VFONTOPTION_WHITE, cv->wstring,
									V_StringWidth(cv->string) - (i == itemOn ? USSizeW : 0),
									y);
								
#else
								V_DrawFill((currentMenu->x + currentMenu->width) -
									V_StringWidth(cv->string) - (i == itemOn ? USSizeW : 0) - 1,
									y - 1, V_StringWidth(cv->string) + (i == itemOn ? USSizeW : 0) + 2,
									USSizeH + 2, 0);
								V_DrawString(
									(currentMenu->x + currentMenu->width) -
									V_StringWidth(cv->string) - (i == itemOn ? USSizeW : 0),
									y/* + 12*/, V_WHITEMAP, cv->string);
#endif
								if (skullAnimCounter < 4 && i == itemOn)
									V_DrawCharacter((currentMenu->x + currentMenu->width) -
										USSizeW, y, '_' | 0x80);
								//y += STRINGHEIGHT;
								//y += LINEHEIGHT;
								break;
							default:
#if 0
								V_DrawStringW(VFONT_SMALL, (currentMenu->menuitems[i].status & IT_DISABLED2 || currentMenu->menuitems[i].status & IT_CVARREADONLY? VFONTOPTION_GRAY : VFONTOPTION_WHITE),
									cv->wstring,
									((currentMenu->x + currentMenu->width)) - V_StringWidthW(VFONT_SMALL, 0, cv->wstring),
									y);
#else
								V_DrawString(
									((currentMenu->x + currentMenu->width)) - V_StringWidth(cv->string),
									y,
									(currentMenu->menuitems[i].status & IT_DISABLED2 || currentMenu->menuitems[i].status &  IT_CVARREADONLY? V_GRAYMAP : V_WHITEMAP),
									cv->string);
									
#endif
								//V_DrawString(BASEVIDWIDTH - x -
								//			 V_StringWidth(cv->string), y, V_WHITEMAP, cv->string);
								//y += STRINGHEIGHT;
								break;
						}
						break;
					}
				y += STRINGHEIGHT;
				break;
			case IT_STRING2:
				V_DrawStringW(VFONT_SMALL, 0, *(currentMenu->menuitems[i].WItemTextPtr), x, y);
			case IT_DYLITLSPACE:
				y += STRINGHEIGHT;
				break;
			case IT_GRAYPATCH:
				if (currentMenu->menuitems[i].status & IT_PATCH)
				{
					if (CharacterGroups[VFONT_LARGE] && *(currentMenu->menuitems[i].WItemTextPtr))
					{
						V_DrawStringW(VFONT_LARGE, VFONTOPTION_GRAY, *(currentMenu->menuitems[i].WItemTextPtr), x, y);
						y += FONTBHEIGHT - LINEHEIGHT;
					}
					else if (currentMenu->menuitems[i].patch && currentMenu->menuitems[i].patch[0])
						V_DrawMappedPatch(x, y, 0,
										  W_CachePatchName(currentMenu->
														   menuitems[i].patch, PU_CACHE), graymap);
					y += LINEHEIGHT;
				}
				break;

		}
	}

	// DRAW THE SKULL CURSOR
	if (!(currentMenu->extraflags & MENUFLAG_HIDECURSOR))
	{
		if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_PATCH)
			|| ((currentMenu->menuitems[itemOn].status & IT_DISPLAY) == IT_NOTHING))
		{
			V_DrawScaledPatch(currentMenu->x + SKULLXOFF,
							  cursory - 5, 0, W_CachePatchName(skullName[whichSkull], PU_CACHE));
		}
		else if (skullAnimCounter < cv_cons_blinkingrate.value * NEWTICRATERATIO)	//blink cursor
		{
			if (currentMenu->menuitems[itemOn].status & IT_CENTERSTRING)
			{
				V_DrawCharacter(cursorx1, cursory, '*' | 0x80);
				V_DrawCharacter(cursorx2, cursory, '*' | 0x80);
			}
			else
				V_DrawCharacter(currentMenu->x - 10, cursory, '*' | 0x80);
		}
	}
}

//===========================================================================
//                              Some Draw routine
//===========================================================================

//
//      Menu Functions
//
void M_DrawThermo(int x, int y, consvar_t * cv)
{
	int xx, i;
	int leftlump, rightlump, centerlump[2], cursorlump;
	patch_t *p;

	xx = x;
	leftlump = W_GetNumForName("M_THERML");
	rightlump = W_GetNumForName("M_THERMR");
	centerlump[0] = W_GetNumForName("M_THERMM");
	centerlump[1] = W_GetNumForName("M_THERMM");
	cursorlump = W_GetNumForName("M_THERMO");
	V_DrawScaledPatch(xx, y, 0, p = W_CachePatchNum(leftlump, PU_CACHE));
	xx += SHORT(p->width) - SHORT(p->leftoffset);
	for (i = 0; i < 16; i++)
	{
		V_DrawScaledPatch(xx, y, 0, W_CachePatchNum(centerlump[i & 1], PU_CACHE));
		xx += 8;
	}
	V_DrawScaledPatch(xx, y, 0, W_CachePatchNum(rightlump, PU_CACHE));

	xx = (cv->value - cv->PossibleValue[0].value) * (15 * 8) /
		(cv->PossibleValue[1].value - cv->PossibleValue[0].value);

	V_DrawScaledPatch((x + 8) + xx, y, 0, W_CachePatchNum(cursorlump, PU_CACHE));
}

void M_DrawEmptyCell(menu_t * menu, int item)
{
	V_DrawScaledPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0,
					  W_CachePatchName("M_CELL1", PU_CACHE));
}

void M_DrawSelCell(menu_t * menu, int item)
{
	V_DrawScaledPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0,
					  W_CachePatchName("M_CELL2", PU_CACHE));
}

//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
//added:06-02-98:
extern int st_borderpatchnum;	//st_stuff.c (for Glide)
void M_DrawTextBox(int x, int y, int width, int lines)
{
	patch_t *p;
	int cx, cy;
	int n;
	int step, boff;
	
	fixed_t x1, y1, x2, y2;

	step = 8;
	boff = 8;

#if 1
	// GhostlyDeath <November 3, 2010> -- Banded message (gray band)
	x1 = 1;
	y1 = y + 8;
	x2 = vid.width - 1;
	y2 = y1 + (lines * 8);
	
	V_DrawFadeConsBackEx(VEX_COLORMAPWHITE, x1, y1, x2, y2);
#else
	// draw left side
	cx = x;
	cy = y;
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TL], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_L], PU_CACHE);
	for (n = 0; n < lines; n++)
	{
		V_DrawScaledPatch(cx, cy, 0, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BL], PU_CACHE));

	// draw middle
	V_DrawFlatFill(x + boff, y + boff, width * step, lines * step, st_borderpatchnum);

	cx += boff;
	cy = y;
	while (width > 0)
	{
		V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_T], PU_CACHE));

		V_DrawScaledPatch(cx, y + boff + lines * step, 0,
						  W_CachePatchNum(viewborderlump[BRDR_B], PU_CACHE));
		width--;
		cx += step;
	}

	// draw right side
	cy = y;
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_TR], PU_CACHE));
	cy += boff;
	p = W_CachePatchNum(viewborderlump[BRDR_R], PU_CACHE);
	for (n = 0; n < lines; n++)
	{
		V_DrawScaledPatch(cx, cy, 0, p);
		cy += step;
	}
	V_DrawScaledPatch(cx, cy, 0, W_CachePatchNum(viewborderlump[BRDR_BR], PU_CACHE));
#endif
}

//==========================================================================
//                        Message is now a (hackeble) Menu
//==========================================================================
void M_DrawMessageMenu(void);

wchar_t* MessageWStuff = NULL;
wchar_t* MessageItemWStuff = NULL;

menuitem_t MessageMenu[] = {
	// TO HACK
	{0, NULL, &MessageItemWStuff, NULL, 0}
};

menu_t MessageDef = {
	0,
	NULL,						// titlepic
	&MessageWStuff,				// title text
	1,							// # of menu items
	MessageMenu,				// menuitem_t ->
	NULL,						// previous menu       (TO HACK)
	M_DrawMessageMenu,			// drawing routine ->
	0, 0,						// x,y                 (TO HACK)
	0							// lastOn, flags       (TO HACK)
};

void M_StartMessage(const char *string, void *routine, menumessagetype_t itemtype)
{
	wchar_t* WStr = NULL;
	size_t WSz = 0;
	
	UNICODE_ASCIIToUnicode(string, strlen(string), &WStr, &WSz);
	M_StartMessageW(WStr, routine, itemtype);
	
	Z_Free(WStr);
}

void M_StartMessageW(wchar_t *string, void *routine, menumessagetype_t itemtype)
{
	int max, start, i, lines;
#define message (*(MessageDef.menuitems[0].WItemTextPtr))
	if (message)
		Z_Free(message);
	message = Z_StrDupW(string);
	//DEBFILE(message);
	
	M_StartControlPanel();		// can't put menuactiv to true
	
	// GhostlyDeath <November 3, 2010> -- Do not infinite loop in message windows
	// As long as the previous menu wasn't a message
	if (MessageDef.prevMenu && MessageDef.prevMenu != &MessageDef)
		MessageDef.prevMenu = currentMenu;
	
	*(MessageDef.menuitems[0].WItemTextPtr) = message;
	MessageDef.menuitems[0].alphaKey = itemtype;
	switch (itemtype)
	{
		case MM_NOTHING:
			MessageDef.menuitems[0].status = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = M_StopMessage;
			break;
		case MM_YESNO:
			MessageDef.menuitems[0].status = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
		case MM_EVENTHANDLER:
			MessageDef.menuitems[0].status = IT_MSGHANDLER;
			MessageDef.menuitems[0].itemaction = routine;
			break;
	}
	//added:06-02-98: now draw a textbox around the message
	// compute lenght max and the numbers of lines
	max = 0;
	start = 0;
	for (lines = 0; *(message + start); lines++)
	{
		for (i = 0; i < (int)UNICODE_StringLength(message + start); i++)
		{
			if (*(message + start + i) == '\n')
			{
				if (i > max)
					max = i;
				start += i + 1;
				i = -1;			//added:07-02-98:damned!
				break;
			}
		}

		if (i == (int)UNICODE_StringLength(message + start))
			start += i;
	}

	MessageDef.x = (BASEVIDWIDTH - 8 * max - 16) / 2;
	MessageDef.y = (BASEVIDHEIGHT - M_StringHeightW(message)) / 2;

	MessageDef.lastOn = (lines << 8) + max;

//    M_SetupNextMenu();
	currentMenu = &MessageDef;
	itemOn = 0;
}

#define MAXMSGLINELEN 256

void M_DrawMessageMenu(void)
{
	int y;
	short i, max;
	wchar_t string[MAXMSGLINELEN];
	int start, lines;
	wchar_t *msg = *(currentMenu->menuitems[0].WItemTextPtr);

	y = currentMenu->y;
	start = 0;
	lines = currentMenu->lastOn >> 8;
	max = (currentMenu->lastOn & 0xFF) * 8;
	M_DrawTextBox(currentMenu->x, y - 8, (max + 7) >> 3, lines);

	while (*(msg + start))
	{
		for (i = 0; i < (int)UNICODE_StringLength(msg + start); i++)
		{
			if (*(msg + start + i) == '\n')
			{
				memset(string, 0, sizeof(string));
				if (i >= MAXMSGLINELEN)
				{
					CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
					return;
				}
				else
				{
					UNICODE_CopyN(string, msg + start, i);
					start += i + 1;
					i = -1;		//added:07-02-98:damned!
				}

				break;
			}
		}

		if (i == (int)UNICODE_StringLength(msg + start))
		{
			if (i >= MAXMSGLINELEN)
			{
				CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
				return;
			}
			else
			{
				UNICODE_Copy(string, msg + start);
				start += i;
			}
		}

		V_DrawStringW(VFONT_SMALL, 0, string, (BASEVIDWIDTH - V_StringWidthW(VFONT_SMALL, 0, string)) / 2, y);
		y += 8;					//SHORT(hu_font[0]->height);
	}
}

// default message handler
void M_StopMessage(int choice)
{
	M_SetupNextMenu(MessageDef.prevMenu);
	S_StartSound(NULL, sfx_swtchx);
}

//==========================================================================
//                        Menu stuffs
//==========================================================================

//added:30-01-98:
//
//  Write a string centered using the hu_font
//
void M_CentreText(int y, char *string)
{
	int x;
	//added:02-02-98:centre on 320, because V_DrawString centers on vid.width...
	x = (BASEVIDWIDTH - V_StringWidth(string)) >> 1;
	V_DrawString(x, y, 0, string);
}

//
//      Find string height from hu_font chars
//
int M_StringHeight(char *string)
{
	int i;
	int h;
	int height = 8;				//(hu_font[0]->height);

	h = height;
	for (i = 0; i < (int)strlen(string); i++)
		if (string[i] == '\n')
			h += height;

	return h;
}

int M_StringHeightW(wchar_t* string)
{
	int i;
	int h;
	int height = 8;				//(hu_font[0]->height);

	h = height;
	for (i = 0; i < (int)UNICODE_StringLength(string); i++)
		if (string[i] == '\n')
			h += height;

	return h;
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
	if (!menuactive)
		return;

	//added:18-02-98:
	// center the scaled graphics for the menu,
	//  set it 0 again before return!!!
	//scaledofs = vid.centerofs;
	
	if (currentMenu->drawroutine)
	{
		if (currentMenu->drawroutine != M_DrawMessageMenu)
			V_DrawFadeScreen();
		currentMenu->drawroutine();	// call current menu Draw routine
	}

	//added:18-02-98: it should always be 0 for non-menu scaled graphics.
	scaledofs = 0;
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
	// intro might call this repeatedly
	if (menuactive)
		return;

	menuactive = 1;
	currentMenu = &MainDef;		// JDC
	itemOn = currentMenu->lastOn;	// JDC

	CON_ToggleOff();			// dirty hack : move away console
}

//
// M_ClearMenus
//
void M_ClearMenus(boolean callexitmenufunc)
{
	if (!menuactive)
		return;

	if (currentMenu->quitroutine && callexitmenufunc)
	{
		if (!currentMenu->quitroutine())
			return;				// we can't quit this menu (also used to set parameter from the menu)
	}

	menuactive = 0;
}

//
//  A smaller 'Thermo', with range given as percents (0-100)
//
void M_DrawSlider(int x, int y, int range, void* extra)
{
	int i;

	if (range < 0)
		range = 0;
	if (range > 100)
		range = 100;

	if (extra)
	{
		V_DrawMappedPatch(x - 8, y, 0, W_CachePatchName("M_SLIDEL", PU_CACHE), extra);

		for (i = 0; i < SLIDER_RANGE; i++)
			V_DrawMappedPatch(x + i * 8, y, 0, W_CachePatchName("M_SLIDEM", PU_CACHE), extra);

		V_DrawMappedPatch(x + SLIDER_RANGE * 8, y, 0, W_CachePatchName("M_SLIDER", PU_CACHE), extra);

		// draw the slider cursor
		V_DrawMappedPatch(x + ((SLIDER_RANGE - 1) * 8 * range) / 100, y, 0,
						  W_CachePatchName("M_SLIDEC", PU_CACHE), extra);
	}
	else
	{
		V_DrawScaledPatch(x - 8, y, 0, W_CachePatchName("M_SLIDEL", PU_CACHE));

		for (i = 0; i < SLIDER_RANGE; i++)
			V_DrawScaledPatch(x + i * 8, y, 0, W_CachePatchName("M_SLIDEM", PU_CACHE));

		V_DrawScaledPatch(x + SLIDER_RANGE * 8, y, 0, W_CachePatchName("M_SLIDER", PU_CACHE));

		// draw the slider cursor
		V_DrawScaledPatch(x + ((SLIDER_RANGE - 1) * 8 * range) / 100, y, 0,
						  W_CachePatchName("M_SLIDEC", PU_CACHE));
	}
}

int (*setupcontrols)[2] = gamecontrol;

//
//  Draws the Customise Controls menu
//
void M_DrawControl(void)
{
	char tmp[50];
	int i;
	int keys[2];

	// draw title, strings and submenu
	M_DrawGenericMenu();

	for (i = currentMenu->firstdraw; i < currentMenu->numitems && i < (currentMenu->firstdraw + currentMenu->itemsperpage); i++)
	{
		if (currentMenu->menuitems[i].status != IT_CONTROL)
			continue;

		keys[0] = setupcontrols[currentMenu->menuitems[i].alphaKey][0];
		keys[1] = setupcontrols[currentMenu->menuitems[i].alphaKey][1];

		tmp[0] = '\0';
		if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
		{
			strcpy(tmp, "---");
		}
		else
		{
			if (keys[0] != KEY_NULL)
				strcat(tmp, G_KeynumToString(keys[0]));

			if (keys[0] != KEY_NULL && keys[1] != KEY_NULL)
				strcat(tmp, " or ");

			if (keys[1] != KEY_NULL)
				strcat(tmp, G_KeynumToString(keys[1]));

		}
		V_DrawString((DefaultKeyBindDef.x + DefaultKeyBindDef.width) - V_StringWidth(tmp),
					 DefaultKeyBindDef.y + (i - currentMenu->firstdraw) * STRINGHEIGHT, V_WHITEMAP, tmp);
	}
}

