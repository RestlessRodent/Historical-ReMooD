// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system.video;

import java.util.ServiceLoader;
import org.remood.remood.core.config.GameConfiguration;
import org.remood.remood.core.console.GameConsole;

/**
 * This class manages video drivers which are used to create game display
 * surfaces.
 *
 * @since 2016/07/17
 */
public class VideoDriverManager
{
	/** The game console. */
	protected final GameConsole console;
	
	/** The game configuration. */
	protected final GameConfiguration config;
	
	/** The currently selected driver. */
	private volatile VideoDriver _driver;
	
	/**
	 * Initializes the video driver manager.
	 *
	 * @param __gc The game console.
	 * @param __conf The game configuration.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/17
	 */
	public VideoDriverManager(GameConsole __gc, GameConfiguration __conf)
		throws NullPointerException
	{
		// Check
		if (__gc == null || __conf == null)
			throw new NullPointerException();
		
		// Set
		this.console = __gc;
		this.config = __conf;
		
		// Setup service loader to find video driver factories
		ServiceLoader<VideoDriverFactory> services =
			ServiceLoader.<VideoDriverFactory>load(VideoDriverFactory.class);
		for (VideoDriverFactory f : services)
		{
			throw new Error("TODO");
		}
	}
}

