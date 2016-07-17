// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.javaseremood;

import org.remood.remood.core.config.GameConfiguration;
import org.remood.remood.core.console.GameConsole;
import org.remood.remood.core.system.video.VideoDriver;
import org.remood.remood.core.system.video.VideoException;
import org.remood.remood.core.system.video.VideoSurface;

/**
 * This provides the capability of creating video surfaces which utilize
 * Swing.
 *
 * @since 2016/07/17
 */
public class JavaSEVideoDriver
	implements VideoDriver
{
	/** The game console. */
	protected final GameConsole console;
	
	/** The game configuration. */
	protected final GameConfiguration config;
	
	/**
	 * Initializes the video driver.
	 *
	 * @param __gc The game console.
	 * @param __conf The game configuration.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/17
	 */
	public JavaSEVideoDriver(GameConsole __gc, GameConfiguration __conf)
		throws NullPointerException
	{
		// Check
		if (__gc == null || __conf == null)
			throw new NullPointerException();
		
		// Set
		this.console = __gc;
		this.config = __conf;
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	public VideoSurface selectVideoMode(boolean __hw, int __w, int __h)
		throws VideoException
	{
		return new JavaSEVideoSurface(__w, __h);
	}
}

