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
#include "v_video.h"
#include "i_util.h"

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
		VID_AddMode(cmodes[i], cmodes[i + 1], false);
	
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
	jmethodID svmm = J_GetMethodID(g_MainClass, "selectVideoMode",
		"(ZII)Lorg/remood/remood/core/system/video/VideoSurface;");
	
	// Change video mode
	vss = J_CallObjectMethod(g_MainObject, svmm, false, a_Width, a_Height);
	
	// Setup video surface, use a software surface because the Java native
	// one is 32-bit RGB.
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL, false, false, 1);
}

jclass __VSSClass;
jclass __VSClass;
jmethodID __VSRefreshM;
jmethodID __MainVSGetM;
jmethodID __VSFBGetM;

void J_VideoQuickInit(void)
{
	__VSSClass = J_FindClass(
		"org/remood/remood/core/system/video/VideoSoftwareSurface");
	__VSClass = J_FindClass(
		"org/remood/remood/core/system/video/VideoSurface");
	__VSRefreshM = J_GetMethodID(__VSClass, "refresh", "()V");
	__MainVSGetM = J_GetMethodID(g_MainClass, "videoSurface",
		"()Lorg/remood/remood/core/system/video/VideoSurface;");
	__VSFBGetM = J_GetMethodID(__VSSClass,
		"framebuffer", "()[I");
}

/**
 * Blits the graphics to the screen.
 *
 * @since 2016/07/17
 */
void I_FinishUpdate(void)
{
	jobject vss;
	jintArray fbd;
	uint32_t w, h, b, i, n;
	jint* rfb;
	uint8_t* buffer;
	uint8_t* pal;
	
	// Get the 8-bit buffer
	buffer = I_VideoSoftBuffer(&w, &h, &b, NULL);
	w *= b;
	
	// Get framebuffer
	vss = J_CallObjectMethod(g_MainObject, __MainVSGetM);
	fbd = (jintArray)J_CallObjectMethod(vss, __VSFBGetM);
	
	// Get raw array
	rfb = J_GetIntArrayElements(fbd, NULL);
	
	// Get the palette
	pal = V_GetPalette(0);
	
	// Copy image data
	n = w * h;
	for (i = 0; i < n; i++)
	{
		// Get index
		int pdx = buffer[i] * 3;
		
		// Set colors
		rfb[i] = (((int)pal[pdx]) << 16) |
			(((int)pal[pdx + 1]) << 8) |
			(((int)pal[pdx + 2]));
	}
	
	// Commit the array
	J_ReleaseIntArrayElements(fbd, rfb, 0);
	
	// Tell the video surface to refresh
	J_CallVoidMethod(vss, __VSRefreshM);
}

