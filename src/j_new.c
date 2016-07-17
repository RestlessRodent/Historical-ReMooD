// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

#include "j.h"

#include "z_zone.h"

/**
 * Prepares the video mode list.
 *
 * Calls VID_AddMode.
 *
 * @since 2016/07/17
 */
void VID_PrepareModeList(void)
{
	jobject vd;
	jintArray modes;
	int i, n;
	jint* cmodes;
	jclass vdc = J_FindClass(
		"org/remood/remood/core/system/video/VideoDriver");
	jmethodID vmm = J_GetMethodID(g_MainClass, "videoDriver",
		"()Lorg/remood/remood/core/system/video/VideoDriver;");
	jmethodID lvm = J_GetMethodID(vdc, "listModes",
		"()[I");
	
	// Get the video driver
	vd = J_CallObjectMethod(g_MainObject, vmm);
	
	// Get video modes
	modes = (jintArray)J_CallObjectMethod(vd, lvm);
	
	// Setup target array
	n = J_GetArrayLength(modes);
	cmodes = Z_Malloc(sizeof(int) * n, PU_STATIC, NULL);
	
	// Load elements
	J_GetIntArrayRegion(modes, 0, n, cmodes);
	
	for (i = 0; i < n; i += 2)
		VID_AddMode(cmodes[i], cmodes[i + 1]);
	
	Z_Free(cmodes);
}

/* I_StartupGraphics() -- Initializes graphics */
void I_StartupGraphics(void)
{
	/* Pre-initialize video */
	if (!I_VideoPreInit())
		return;
	
	/* Generic Init */
	I_VideoGenericInit();
}

/**
 * Sets the video mode.
 *
 * @sicne 2016/07/17
 */
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen, const uint8_t a_Depth)
{
	jobject vss;
	jintArray fbd;
	jmethodID svmm = J_GetMethodID(g_MainClass, "selectVideoMode",
		"(ZII)Lorg/remood/remood/core/system/video/VideoSurface;");
	jmethodID gfbm = J_GetMethodID(J_FindClass(
		"org/remood/remood/core/system/video/VideoSoftwareSurface"),
		"framebuffer", "()[I");
	
	// Change video mode
	vss = J_CallObjectMethod(g_MainObject, svmm, false, a_Width, a_Height);
	
	// Get framebuffer
	fbd = (jintArray)J_CallObjectMethod(vss, gfbm);
	
	// Setup video surface
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL, false, false, 4);
}

