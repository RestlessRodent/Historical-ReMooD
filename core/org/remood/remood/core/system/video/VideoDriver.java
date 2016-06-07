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
 * This package contains the video driver interface which is implemented by
 * libraries so that the user can actually see and interact with the game.
 *
 * @since 2016/05/19
 */
public interface VideoDriver
	extends DeviceDriver
{
	/**
	 * Attempts to create a video surface.
	 *
	 * @param __hw If {@code true} then the surface is a hardware accelerated
	 * three dimensional surface. If a 3D accelerated surface is not possible
	 * then a two dimensional software surface is created instead.
	 * @param __w The framebuffer width.
	 * @param __h The framebuffer height.
	 * @return The created video surface.
	 * @throws VideoException If the surface could not be created.,
	 * @since 2016/06/07
	 */
	public abstract VideoSurface createSurface(boolean __hw, int __w, int __h)
		throws VideoException;
}

