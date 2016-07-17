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
import org.remood.remood.core.system.DeviceDriverIdentity;

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
	
	/** The service manager. */
	private final ServiceLoader<VideoDriverFactory> _services =
		ServiceLoader.<VideoDriverFactory>load(VideoDriverFactory.class);
	
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
		
		// Locate the next driver to use
		this._driver = switchDriver(null);
		
		// Setup service loader to find video driver factories
		ServiceLoader<VideoDriverFactory> services =
			ServiceLoader.<VideoDriverFactory>load(VideoDriverFactory.class);
	}
	
	/**
	 * Selects a new video mode.
	 *
	 * @param __hw Use 3D mode?
	 * @param __w The width.
	 * @param __h The height.
	 * @return The video surface.
	 * @throws VideoException If the surface could not be created.
	 * @since 2016/07/17
	 */
	public VideoSurface selectVideoMode(boolean __hw, int __w, int __h)
		throws VideoException
	{
		return this._driver.selectVideoMode(__hw, __w, __h);
	}
	
	/**
	 * Switches to another video driver.
	 *
	 * @param __name The name of the video driver to switch to, may be
	 * {@code null} to choose any driver.
	 * @return The video driver to use.
	 * @throws IllegalStateException If no video driver was found.
	 * @since 2016/07/17
	 */
	public VideoDriver switchDriver(String __name)
	{
		// Lock
		ServiceLoader<VideoDriverFactory> services = this._services;
		synchronized (services)
		{
			VideoDriverFactory best = null;
			for (VideoDriverFactory f : services)
			{
				// Get the identity
				DeviceDriverIdentity id = f.identity();
			
				// For now just choose any driver
				System.err.println("TODO -- Choose a better driver.");
				best = f;
				break;
			}
			
			// Fail
			if (best == null)
				throw new IllegalStateException("No video driver found.");
			
			// Create the driver
			VideoDriver rv = best.createDriver(this.console, this.config);
			this._driver = rv;
			return rv;
		}
	}
}

