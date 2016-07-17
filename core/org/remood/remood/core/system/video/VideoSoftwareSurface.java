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
	 * @param __fb The raw framebuffer, these are used directly, if
	 * {@code null} one is created.
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
		if (__p < __w)
			throw new VideoException();
		
		// Set
		this.pitch = Math.max(__w, __p);
		this.framebuffer = (__fb != null ? __fb : createFramebuffer());
	}
	
	/**
	 * Creates a new framebuffer.
	 *
	 * @return A framebuffer.
	 * @since 2016/07/17
	 */
	protected int[] createFramebuffer()
	{
		return new int[this.pitch * this.height];
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

