// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.javaseremood;

import org.remood.remood.core.system.input.InputDriver;
import org.remood.remood.core.system.video.VideoDriver;
import org.remood.remood.core.system.video.VideoException;
import org.remood.remood.core.system.video.VideoSoftwareSurface;
import org.remood.remood.core.system.video.VideoSurface;

/**
 * This class implements a video driver which relies on standard Java SE
 * classes.
 *
 * @since 2016/07/16
 */
public class JavaSEVideoDriver
	implements InputDriver, VideoDriver
{
	/**
	 * {@inheritDoc}
	 * @since 2016/07/16
	 */
	@Override
	public VideoSurface createSurface(boolean __hw, int __w, int __h)
		throws VideoException
	{
		throw new Error("TODO");
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/16
	 */
	@Override
	public String name()
	{
		return "javase-video";
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/16
	 */
	@Override
	public int priority()
	{
		return Integer.MAX_VALUE;
	}
}

