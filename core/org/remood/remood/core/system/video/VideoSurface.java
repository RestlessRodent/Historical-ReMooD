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
	protected final int width;
	
	/** The surface height. */
	protected final int height;
	
	/**
	 * Initializes the video surface.
	 *
	 * @param __w The width.
	 * @param __h The height.
	 * @throws VideoException If the width and/or height are zero
	 * or negative.
	 * @since 2016/06/07
	 */
	public VideoSurface(int __w, int __h)
		throws VideoException
	{
		// Check
		if (__w <= 0 || __h <= 0)
			throw new IllegalArgumentException();
		
		// Set
		this.width = __w;
		this.height = __h;
	}
	
	/**
	 * Destroys this video surface.
	 *
	 * @since 2016/07/17
	 */
	public abstract void destroy();
	
	/**
	 * Returns {@code true} if this is a three dimensional hardware surface.
	 *
	 * @return {@code true} if a hardware surface.
	 * @since 2016/06/07
	 */
	public abstract boolean isHardwareSurface();
	
	/**
	 * Refreshes the video surface.
	 *
	 * @since 2016/07/17
	 */
	public abstract void refresh();
	
	/**
	 * Returns the height of the video surface.
	 *
	 * @return The surface height.
	 * @since 2016/06/07
	 */
	public final int height()
	{
		return this.height;
	}
	
	/**
	 * Returns the width of the video surface.
	 *
	 * @return The surface width.
	 * @since 2016/06/07
	 */
	public final int width()
	{
		return this.width;
	}
}

