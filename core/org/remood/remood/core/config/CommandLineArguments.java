// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.config;

import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedList;
import java.util.List;
import org.remood.remood.core.console.GameConsole;

/**
 * This class contains some default command line arguments which may be used
 * to initialize the game or setup some other state (such as playing a demo).
 *
 * Arguments are either switches which start with a dash, or console commands
 * to be executed which start with a plus. Unlike previously some arguments
 * now accept multiple options (such as demo playback).
 *
 * @since 2016/07/16
 */
public class CommandLineArguments
{
	/** Console commands to execute in sequence. */
	private final String[] _console;
	
	/**
	 * This parses the command line arguments which may be used when parsing
	 * the configuration file.
	 *
	 * @param __gc The console to print messages to.
	 * @param __args Program arguments.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/16
	 */
	public CommandLineArguments(GameConsole __gc, String... __args)
		throws NullPointerException
	{
		// Check
		if (__gc == null || __args == null)
			throw new NullPointerException();
		
		// Go through all arguments
		int n = __args.length;
		List<String> sub = new ArrayList<>();
		for (int i = 0; i < n; i += (1 + sub.size()))
		{
			// Get
			String arg = __args[i];
			int an = arg.length();
			char fc = (an > 0 ? arg.charAt(0) : 0);
			
			// Determine the sub-arguments to this argument
			sub.clear();
			for (int j = i + 1; i < n; i++)
			{
				String sx = __args[j];
				
				// If it starts with a minus or plus then it is a new argument
				// group
				int sxn = sx.length();
				char qq = (sxn > 0 ? sx.charAt(0) : 0);
				if (qq == '-' || qq == '+')
					break;
				
				// Add to list
				sub.add(sx);
			}
			
			// A normal command line switch?
			switch (arg)
			{
					// Unknown, either an illegal switch or a console command
				default:
					// Unknown switch
					if (fc == '-')
					{
						throw new Error("TODO");
					}
					
					// Console command
					else if (fc == '+')
					{
						throw new Error("TODO");
					}
					
					// Assume -file
					else
					{
						if (true)
							throw new Error("TODO");
					}
					break;
			}
		}
		
		throw new Error("TODO");
	}
}

