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
 * This class should be described in the future.
 *
 * @since 2016/06/07
 */
public abstract class VideoSoftwareSurface
	extends VideoSurface
{
	/** The surface pitch. */
	private final int pitch;
	
	/** The framebuffer used (32-bit ARGB). */
	private final int[] framebuffer;
	
	/**
	 * Initializes the video surface.
	 *
	 * @param __w The width.
	 * @param __h The height.
	 * @param __fb The raw framebuffer, these are used directly.
	 * @param __p The framebuffer pitch, must be equal or greater than the
	 * width.
	 * @throws VideoException If the pitch is lower than the width.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/06/07
	 */
	public VideoSoftwareSurface(int __w, int __h, int[] __fb, int __p)
		throws VideoException, NullPointerException
	{
		super(__w, __h);
		
		// Check
		if (__fb == null)
			throw new NullPointerException();
		if (__p < __w)
			throw new VideoException();
		
		// Set
		this.pitch = __p;
		this.framebuffer = __fb;
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/06/07
	 */
	@Override
	public final boolean isHardwareSurface()
	{
		return false;
	}
}

