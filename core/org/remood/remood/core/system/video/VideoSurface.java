// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system.video;

/**
 * This class is used by the video driver to provide a display surface to
 * render the game on.
 *
 * @since 2016/06/07
 */
public abstract class VideoSurface
{
	/** The surface width. */
	private final int width;
	
	/** The surface height. */
	private final int height;
	
	/**
	 * Initializes the video surface.
	 *
	 * @param __w The width.
	 * @param __h The height.
	 * @throws IllegalArgumentException If the width and/or height are zero
	 * or negative.
	 * @since 2016/06/07
	 */
	public VideoSurface(int __w, int __h)
		throws IllegalArgumentException
	{
		// Check
		if (__w <= 0 || __h <= 0)
			throw new IllegalArgumentException("");
		
		// Set
		this.width = __w;
		this.height = __h;
	}
}

