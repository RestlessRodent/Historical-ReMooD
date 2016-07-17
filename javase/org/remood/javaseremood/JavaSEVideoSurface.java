// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.javaseremood;

import org.remood.remood.core.system.video.VideoDriver;
import org.remood.remood.core.system.video.VideoException;
import org.remood.remood.core.system.video.VideoSoftwareSurface;
import org.remood.remood.core.system.video.VideoSurface;

/**
 * This class provides a video surface which uses Swing.
 *
 * @since 2016/07/17
 */
public class JavaSEVideoSurface
	extends VideoSoftwareSurface
{
	/**
	 * Initializes the video surface.
	 *
	 * @param __w The surface width.
	 * @param __h The surface height.
	 * @since 2016/07/17
	 */
	public JavaSEVideoSurface(int __w, int __h)
		throws VideoException
	{
		super(__w, __h, null, __w);
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	protected int[] createFramebuffer()
	{
		throw new Error("TODO");
	}
}

