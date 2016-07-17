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

/**
 * This is the main ReMooD entry point.
 *
 * @since 2016/07/16
 */
public class Main
{
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
		// Parse the command line
		CommandLineArguments cla = new CommandLineArguments(__args);
		
		throw new Error("TODO");
	}
}

