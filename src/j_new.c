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
	jclass vsc = J_FindClass(
		"org/remood/remood/core/system/video/VideoDriver");
	jmethodID vmm = J_GetMethodID(g_MainClass, "videoDriver",
		"()Lorg/remood/remood/core/system/video/VideoDriver;");
	jmethodID lvm = J_GetMethodID(vsc, "listModes",
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

