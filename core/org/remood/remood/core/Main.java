// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core;

import java.io.PrintStream;
import java.util.Arrays;
import org.remood.remood.core.config.CommandLineArguments;
import org.remood.remood.core.config.GameConfiguration;
import org.remood.remood.core.console.GameConsole;
import org.remood.remood.core.system.video.VideoDriverManager;
import org.remood.remood.core.system.video.VideoSurface;

/**
 * This is the main ReMooD entry point.
 *
 * @since 2016/07/16
 */
public class Main
{
	/** The primary game console. */
	protected final GameConsole console;
	
	/** The default command line arguments. */
	protected final CommandLineArguments commandline;
	
	/** The user's configuration. */
	protected final GameConfiguration config;
	
	/** Manager for video drivers. */
	protected final VideoDriverManager videomanager;
	
	/**
	 * Initialize some game details.
	 *
	 * @param __args Program arguments.
	 * @since 2016/07/17
	 */
	public Main(String... __args)
	{
		// Setup the core console that every instance will use (for debugging
		// and game usage)
		GameConsole gc = new GameConsole();
		this.console = gc;
		
		// Parse the command line
		CommandLineArguments cla = new CommandLineArguments(gc, __args);
		this.commandline = cla;
		
		// Load the user configuration
		GameConfiguration config = new GameConfiguration(gc, cla);
		this.config = config;
		
		// Setup video manager
		VideoDriverManager videomanager = new VideoDriverManager(gc, config);
		this.videomanager = videomanager;
	}
	
	/**
	 * This is called by the C code because it does not know anything about
	 * Java exception handling.
	 *
	 * @param __args Program arguments.
	 * @return Zero on failure, any other value on success.
	 * @since 2016/07/16
	 */
	public static int catchMain(String... __args)
	{
		// Could fail
		try
		{
			// Call other method
			main(__args);
			
			// Ok
			return 1;
		}
		
		// Caught some exception
		catch (Throwable t)
		{
			// Print it
			PrintStream err = System.err;
			err.println();
			err.println("*** UNCAUGHT EXCEPTION ***");
			t.printStackTrace(err);
			err.println();
			
			// Fail so I_Error it called
			return 0;
		}
	}
	
	/**
	 * Main entry point for the game.
	 *
	 * @param __args Program arguments.
	 * @since 2016/07/16
	 */
	public static void main(String... __args)
	{
		// Create main class, which does everything
		new Main(__args);
	}
}

