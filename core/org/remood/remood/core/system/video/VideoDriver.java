// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system.video;

import org.remood.remood.core.system.DeviceDriver;

/**
 * This interface represents video drivers which are used to show the game to
 * the user.
 *
 * @since 2016/07/17
 */
public interface VideoDriver
	extends DeviceDriver
{
	/**
	 * Lists all of the video modes which are available.
	 *
	 * @return The list of video modes, must be a multiple of two where the
	 * width is followed by the height.
	 * @since 2016/07/17
	 */
	public abstract int[] listModes();
	
	/**
	 * Creates a new video surface for displaying video.
	 *
	 * @param __hw Should hardware acceleration (3D) graphics be used?
	 * @param __w The width of the screen.
	 * @param __h The height of the screen.
	 * @return The video surface.
	 * @throws VideoException If the surface could not be created.
	 * @since 2016/07/17
	 */
	public abstract VideoSurface selectVideoMode(boolean __hw, int __w,
		int __h)
		throws VideoException;
}

